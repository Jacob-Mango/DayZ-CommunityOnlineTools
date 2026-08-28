// =============================================================================
//  JMBanModule.c
//
//  Standalone ban management module for COT.
//
//  Structure confirmed from real source:
//    * Extends JMRenderableModuleBase
//    * Constructor registers permissions via GetPermissionsManager()
//    * GetLayoutRoot / GetTitle / GetIconName / ImageIsIcon / GetWebhookTitle
//    * GetRPCMin / GetRPCMax bracket the enum
//    * OnRPC with internal switch (matching JMPlayerModule / JMLoadoutModule)
//    * OnMissionLoaded for server-side init (matching JMLoadoutModule)
//    * OnUpdate( float timeslice ) for the prune sweep (matching JMESPModule)
//    * EnableUpdate() overridden empty to disable client-side ticking
//    * GetForm() cast to JMBanForm to push data to the open panel
//    * Registered in JMModuleConstructor via modules.Insert( JMBanModule )
//
//  Ban-on-connect enforcement: JMPlayerBan.Save() writes a per-player file
//  that COT's mission connect code checks automatically - we don't need a
//  separate connect hook here.
// =============================================================================

enum JMBanModuleRPC
{
    INVALID = 10800,

    // Client -> Server
    RequestBanList,
    RequestBan,
    UnbanPlayer,
    EditBanDuration,

    // Server -> Client
    BanList,

    COUNT
}

class JMBanModule : JMRenderableModuleBase
{
    static const string BAN_DIR  = "$profile:CommunityOnlineTools\\Bans\\";
    static const string BAN_FILE = "$profile:CommunityOnlineTools\\Bans\\Bans.json";

    static const float PRUNE_INTERVAL = 60.0;

    protected ref JMPlayerBanStore             m_Store;
    protected ref map<string, ref JMPlayerBan> m_ActiveIndex;
    protected float                            m_PruneTimer;

    // -------------------------------------------------------------------------
    //  Constructor - register permissions, matching JMPlayerModule style
    // -------------------------------------------------------------------------

    void JMBanModule()
    {
        GetPermissionsManager().RegisterPermission( "Admin.Ban.View"  );
        GetPermissionsManager().RegisterPermission( "Admin.Ban.Unban" );
    }

    // -------------------------------------------------------------------------
    //  JMRenderableModuleBase overrides
    // -------------------------------------------------------------------------

    override bool HasAccess()
    {
        return GetPermissionsManager().HasPermission( "Admin.Ban.View" );
    }

    override string GetLayoutRoot()
    {
        return "JM/COT/GUI/layouts/ban_form.layout";
    }

    override string GetCategory()
    {
        return "Players";
    }

    override string GetTitle()
    {
        return "Ban Management";
    }

    override string GetIconName()
    {
        return JMConstants.Lucide( "gavel" );
    }

    override bool ImageIsIcon()
    {
        return true;
    }

    override bool ImageHasPath()
    {
        return true;
    }

    override string GetWebhookTitle()
    {
        return "Ban Management Module";
    }

    override void GetWebhookTypes( out array<string> types )
    {
        types.Insert( "Ban"     );
        types.Insert( "Unban"   );
        types.Insert( "EditBan" );
    }

    // -------------------------------------------------------------------------
    //  RPC range
    // -------------------------------------------------------------------------

    override int GetRPCMin()
    {
        return JMBanModuleRPC.INVALID;
    }

    override int GetRPCMax()
    {
        return JMBanModuleRPC.COUNT;
    }

    // -------------------------------------------------------------------------
    //  Update - only needed server-side for expiry pruning.
    //  Override EnableUpdate empty on client (matching JMESPModule #ifdef SERVER)
    // -------------------------------------------------------------------------

#ifndef SERVER
    override void EnableUpdate()
    {
    }
#endif

    // -------------------------------------------------------------------------
    //  OnMissionLoaded - load ban store on server (matching JMLoadoutModule)
    // -------------------------------------------------------------------------

    override void OnMissionLoaded()
    {
        super.OnMissionLoaded();

        m_Store       = new JMPlayerBanStore();
        m_ActiveIndex = new map<string, ref JMPlayerBan>();
        m_PruneTimer  = 0;

        if ( g_Game.IsServer() )
            Load();
    }

    // -------------------------------------------------------------------------
    //  OnUpdate - sweep expired timed bans (matching JMESPModule signature)
    // -------------------------------------------------------------------------

    override void OnUpdate( float timeslice )
    {
        super.OnUpdate( timeslice );

        if ( !g_Game.IsServer() )
            return;

        m_PruneTimer += timeslice;
        if ( m_PruneTimer < PRUNE_INTERVAL )
            return;

        m_PruneTimer = 0;
        PruneExpired();
    }

    // -------------------------------------------------------------------------
    //  OnRPC - single override with internal switch (matching JMPlayerModule)
    // -------------------------------------------------------------------------

    override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
    {
        switch ( rpc_type )
        {
        case JMBanModuleRPC.RequestBanList:
            RPC_RequestBanList( ctx, sender, target );
            break;
        case JMBanModuleRPC.RequestBan:
            RPC_RequestBan( ctx, sender, target );
            break;
        case JMBanModuleRPC.UnbanPlayer:
            RPC_UnbanPlayer( ctx, sender, target );
            break;
        case JMBanModuleRPC.EditBanDuration:
            RPC_EditBanDuration( ctx, sender, target );
            break;
        case JMBanModuleRPC.BanList:
            RPC_BanList( ctx, sender, target );
            break;
        }
    }

    // -------------------------------------------------------------------------
    //  Public API - called by JMBanForm
    // -------------------------------------------------------------------------

    // Issue a ban for an online or offline player.
    // durationSeconds: -1 = permanent, else relative seconds from now.
    void Ban( string steamID, string playerName, string message,
              int durationSeconds, string issuedBy, string issuedByName )
    {
        if ( IsMissionHost() )
        {
            Exec_Ban( steamID, playerName, message, durationSeconds, issuedBy, issuedByName, NULL );
        }
        else
        {
            ScriptRPC rpc = new ScriptRPC();
            rpc.Write( steamID         );
            rpc.Write( playerName      );
            rpc.Write( message         );
            rpc.Write( durationSeconds );
            rpc.Send( NULL, JMBanModuleRPC.RequestBan, true, NULL );
        }
    }

    // Ask the server to send the active ban list to this client.
    void RequestBanList()
    {
        ScriptRPC rpc = new ScriptRPC();
        rpc.Send( NULL, JMBanModuleRPC.RequestBanList, true, NULL );
    }

    // -------------------------------------------------------------------------
    //  Store queries
    // -------------------------------------------------------------------------

    JMPlayerBan GetActiveBan( string steamID )
    {
        if ( !m_ActiveIndex )
            return NULL;
        return m_ActiveIndex.Get( steamID );
    }

    bool IsBanned( string steamID )
    {
        return m_ActiveIndex && m_ActiveIndex.Contains( steamID );
    }

    // -------------------------------------------------------------------------
    //  Server: execute a ban
    // -------------------------------------------------------------------------

    private void Exec_Ban( string steamID, string playerName, string message, int durationSeconds, string issuedBy, string issuedByName, PlayerIdentity adminIdent )
    {
        ref JMPlayerBan ban = new JMPlayerBan();
        ban.SteamID       = steamID;
        ban.PlayerName    = playerName;
        ban.Message       = message;
        ban.IssuedBy      = issuedBy;
        ban.IssuedByName  = issuedByName;

        CF_Date now = CF_Date.Now( true );
        ban.BannedAt = now.GetTimestamp();
        if ( durationSeconds == -1 )
            ban.BanDuration = -1;
        else
            ban.BanDuration = now.GetTimestamp() + durationSeconds;

        m_Store.Bans.Insert( ban );
        if ( ban.IsActive() )
            m_ActiveIndex.Set( steamID, ban );

        Save();

        GetCommunityOnlineToolsBase().Log( adminIdent, "Banned [steamID=" + steamID + "] | " + message );
        SendWebhook( "Ban", NULL, "Banned " + playerName + " (" + steamID + ") | " + message );
    }

    // -------------------------------------------------------------------------
    //  Server RPC: RequestBanList
    // -------------------------------------------------------------------------

    private void RPC_RequestBanList( ParamsReadContext ctx, PlayerIdentity sender, Object target )
    {
        if ( !IsMissionHost() )
            return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Ban.View", sender ) )
            return;

        PruneExpired();

        ScriptRPC rpc = new ScriptRPC();
        rpc.Write( m_ActiveIndex.Count() );

        foreach ( string id, JMPlayerBan ban : m_ActiveIndex )
        {
            rpc.Write( ban.SteamID      );
            rpc.Write( ban.PlayerName   );
            rpc.Write( ban.Message      );
            rpc.Write( ban.BannedAt     );
            rpc.Write( ban.BanDuration  );
            rpc.Write( ban.IssuedByName );
        }

        // Append known player list (all ever-connected players) for "Ban Offline" dropdown
        array< JMPlayerInstance > knownPlayers = GetPermissionsManager().GetPlayers();
        rpc.Write( knownPlayers.Count() );
        foreach ( JMPlayerInstance pi : knownPlayers )
        {
            rpc.Write( pi.GetGUID() );
            rpc.Write( pi.GetName() );
        }

        rpc.Send( NULL, JMBanModuleRPC.BanList, true, sender );
    }

    // -------------------------------------------------------------------------
    //  Server RPC: RequestBan - client asks server to ban a player
    // -------------------------------------------------------------------------

    private void RPC_RequestBan( ParamsReadContext ctx, PlayerIdentity sender, Object target )
    {
        if ( !IsMissionHost() )
            return;

        JMPlayerInstance instance;
        if ( !GetPermissionsManager().HasPermission( "Admin.Ban.View", sender, instance ) )
            return;

        string steamID;
        string playerName;
        string message;
        int    durationSeconds;

        if ( !ctx.Read( steamID )         ) return;
        if ( !ctx.Read( playerName )      ) return;
        if ( !ctx.Read( message )         ) return;
        if ( !ctx.Read( durationSeconds ) ) return;

        if ( steamID == "" || message == "" )
            return;

        Exec_Ban( steamID, playerName, message, durationSeconds, sender.GetId(), sender.GetName(), sender );

        // Refresh requester's panel
        RPC_RequestBanList( ctx, sender, target );
    }

    // -------------------------------------------------------------------------
    //  Server RPC: UnbanPlayer
    // -------------------------------------------------------------------------

    private void RPC_UnbanPlayer( ParamsReadContext ctx, PlayerIdentity sender, Object target )
    {
        if ( !IsMissionHost() )
            return;

        JMPlayerInstance instance;
        if ( !GetPermissionsManager().HasPermission( "Admin.Ban.Unban", sender, instance ) )
            return;

        string steamID;
        if ( !ctx.Read( steamID ) || steamID == "" )
            return;

        if ( !m_ActiveIndex.Contains( steamID ) )
        {
            COTCreateNotification( sender, new StringLocaliser( "No active ban found for: " + steamID ) );
            return;
        }

        JMPlayerBan ban = m_ActiveIndex.Get( steamID );
        if ( ban )
            ban.BanDuration = 0;   // zero = intentionally lifted; record stays for audit

        m_ActiveIndex.Remove( steamID );
        Save();

        GetCommunityOnlineToolsBase().Log( sender, "Unbanned [steamID=" + steamID + "]" );
        SendWebhook( "Unban", NULL, sender.GetName() + " unbanned SteamID: " + steamID );

        COTCreateNotification( sender, new StringLocaliser( "Unbanned SteamID: " + steamID ) );

        // Refresh the admin's panel
        RPC_RequestBanList( ctx, sender, target );
    }

    // -------------------------------------------------------------------------
    //  Public API: edit duration of an existing ban
    //  durationSeconds: -1 = permanent, else relative seconds from now
    // -------------------------------------------------------------------------

    void EditBanDuration( string steamID, int durationSeconds )
    {
        if ( IsMissionHost() )
        {
            Exec_EditBanDuration( steamID, durationSeconds, NULL );
        }
        else
        {
            ScriptRPC rpc = new ScriptRPC();
            rpc.Write( steamID         );
            rpc.Write( durationSeconds );
            rpc.Send( NULL, JMBanModuleRPC.EditBanDuration, true, NULL );
        }
    }

    private void Exec_EditBanDuration( string steamID, int durationSeconds, PlayerIdentity adminIdent )
    {
        JMPlayerBan ban = m_ActiveIndex.Get( steamID );
        if ( !ban )
        {
            if ( adminIdent )
                COTCreateNotification( adminIdent, new StringLocaliser( "No active ban found for: " + steamID ) );
            return;
        }

        if ( durationSeconds == -1 )
        {
            ban.BanDuration = -1;
        }
        else
        {
            CF_Date now = CF_Date.Now( true );
            ban.BanDuration = now.GetTimestamp() + durationSeconds;
        }

        Save();

        GetCommunityOnlineToolsBase().Log( adminIdent, "Edited ban duration [steamID=" + steamID + "] -> " + durationSeconds );
        SendWebhook( "EditBan", NULL, "Edited ban duration for SteamID: " + steamID + " -> " + durationSeconds + "s" );
    }

    private void RPC_EditBanDuration( ParamsReadContext ctx, PlayerIdentity sender, Object target )
    {
        if ( !IsMissionHost() )
            return;

        JMPlayerInstance instance;
        if ( !GetPermissionsManager().HasPermission( "Admin.Ban.Unban", sender, instance ) )
            return;

        string steamID;
        int    durationSeconds;

        if ( !ctx.Read( steamID )         ) return;
        if ( !ctx.Read( durationSeconds ) ) return;

        if ( steamID == "" )
            return;

        Exec_EditBanDuration( steamID, durationSeconds, sender );

        RPC_RequestBanList( ctx, sender, target );
    }

    // -------------------------------------------------------------------------
    //  Client RPC: receive ban list, push to open form (matching JMESPModule pattern)
    // -------------------------------------------------------------------------

    private void RPC_BanList( ParamsReadContext ctx, PlayerIdentity sender, Object target )
    {
        if ( IsMissionHost() )
            return;

        int count;
        if ( !ctx.Read( count ) )
            return;

        array<ref JMPlayerBan> bans = new array<ref JMPlayerBan>();

        for ( int i = 0; i < count; i++ )
        {
            ref JMPlayerBan ban = new JMPlayerBan();
            if ( !ctx.Read( ban.SteamID ) || !ctx.Read( ban.PlayerName ) || !ctx.Read( ban.Message ) || !ctx.Read( ban.BannedAt ) || !ctx.Read( ban.BanDuration ) || !ctx.Read( ban.IssuedByName ) )
                break;

            bans.Insert( ban );
        }

        // Read known player list for "Ban Offline" dropdown
        array<string> playerGuids = new array<string>();
        array<string> playerNames = new array<string>();
        int playerCount;
        if ( ctx.Read( playerCount ) )
        {
            for ( int j = 0; j < playerCount; j++ )
            {
                string guid, name;
                if ( !ctx.Read( guid ) || !ctx.Read( name ) )
                    break;
                playerGuids.Insert( guid );
                playerNames.Insert( name );
            }
        }

        // Push to the open form - GetForm() pattern confirmed from JMESPModule
        JMBanForm form;
        if ( Class.CastTo( form, GetForm() ) )
            form.PopulateBanList( bans, playerGuids, playerNames );
    }

    // -------------------------------------------------------------------------
    //  Persistence - JsonFileLoader<JMPlayerBanStore>
    // -------------------------------------------------------------------------

    private void Load()
    {
        m_Store = new JMPlayerBanStore();
        m_ActiveIndex.Clear();

        if ( !FileExist( BAN_FILE ) )
        {
            MakeDirectory( BAN_DIR );
            return;
        }

        string err;
        if ( !JsonFileLoader<JMPlayerBanStore>.LoadFile( BAN_FILE, m_Store, err ) )
        {
            Print( "[COT/BanModule] ERROR loading " + BAN_FILE + ": " + err );
            m_Store = new JMPlayerBanStore();
            return;
        }

        foreach ( JMPlayerBan ban : m_Store.Bans )
        {
            if ( ban && ban.SteamID != "" && ban.IsActive() )
                m_ActiveIndex.Set( ban.SteamID, ban );
        }

        Print( "[COT/BanModule] Loaded " + m_Store.Bans.Count() + " records, " + m_ActiveIndex.Count() + " active." );
    }

    private void Save()
    {
        MakeDirectory( BAN_DIR );

        string err;
        if ( !JsonFileLoader<JMPlayerBanStore>.SaveFile( BAN_FILE, m_Store, err ) )
            Print( "[COT/BanModule] ERROR saving " + BAN_FILE + ": " + err );
    }

    private void PruneExpired()
    {
        array<string> toRemove = new array<string>();

        foreach ( string id, JMPlayerBan ban : m_ActiveIndex )
        {
            if ( !ban.IsActive() )
                toRemove.Insert( id );
        }

        foreach ( string removeID : toRemove )
            m_ActiveIndex.Remove( removeID );
    }
}