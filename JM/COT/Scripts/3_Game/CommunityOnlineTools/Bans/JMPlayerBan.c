// =============================================================================
//  JMPlayerBan.c
//
//  Persistent ban record. All public fields are reflected automatically by
//  JsonFileLoader<JMPlayerBanStore> - no manual serialisation needed.
//
//  NOTE: BanDuration stores an absolute Unix expiry timestamp (not a relative
//  duration). This matches how JMPlayerModule.SendBanMessage() works:
//      duration = nowUTC.GetTimestamp() + duration;
//  -1 means permanent.
// =============================================================================

class JMPlayerBan
{
    // Canonical identifier - Steam 64-bit ID (GetIdentity().GetId()).
    // Never use PlayerName for lookups; names change.
    string SteamID;

    // Display name at time of ban. Informational only.
    string PlayerName;

    // Reason / message shown to the player on kick.
    string Message;

    // Unix timestamp when the ban was issued.
    int BannedAt;

    // Absolute expiry timestamp. -1 = permanent.
    int BanDuration;

    // SteamID of the admin who issued the ban.
    string IssuedBy;

    // Display name of the admin who issued the ban.
    string IssuedByName;

    // -------------------------------------------------------------------------
    //  Static helpers - called by MissionServer.IsCOTBanned at connect time.
    //  These read/write Bans.json directly so they work before the module loads.
    // -------------------------------------------------------------------------

    static const string BAN_FILE = "$profile:CommunityOnlineTools\\Bans\\Bans.json";
    static const string BAN_DIR  = "$profile:CommunityOnlineTools\\Bans\\";

    // "3d 2h 15m" / "Permanent" / "Expired"
    string GetExpiryString()
    {
        if ( IsPermanent() )
            return "Permanent";

        if ( BanDuration <= 0 )
            return "Expired";

        CF_Date now = CF_Date.Now( true );
        int rem = BanDuration - now.GetTimestamp();

        if ( rem <= 0 )
            return "Expired";

        int d = rem / 86400;
        int h = ( rem % 86400 ) / 3600;
        int m = ( rem % 3600  ) / 60;

        string s = "";
        if ( d > 0 ) s += d.ToString() + "d ";
        if ( h > 0 ) s += h.ToString() + "h ";
        if ( m > 0 ) s += m.ToString() + "m";

        if ( s == "" )
            s = "< 1m";

        return s;
    }

    bool IsActive()
    {
        if ( IsPermanent() )
            return true;

        if ( BanDuration <= 0 )
            return false;

        CF_Date now = CF_Date.Now( true );
        return now.GetTimestamp() < BanDuration;
    }

    // -------------------------------------------------------------------------
    //  Helpers
    // -------------------------------------------------------------------------

    bool IsPermanent()
    {
        return BanDuration == -1;
    }

    // Return the active ban record for this player, or NULL if not banned.
    static JMPlayerBan Load( string guid, string steamID )
    {
        if ( !FileExist( BAN_FILE ) )
            return NULL;

        JMPlayerBanStore store = new JMPlayerBanStore();
        string err;
        if ( !JsonFileLoader<JMPlayerBanStore>.LoadFile( BAN_FILE, store, err ) )
        {
            Print( "[COT/Ban] Load error: " + err );
            return NULL;
        }

        foreach ( JMPlayerBan ban : store.Bans )
        {
            if ( ban && ( ban.SteamID == steamID || ban.SteamID == guid ) && ban.IsActive() )
                return ban;
        }

        return NULL;
    }

    //! DEPRECATED - use CF_Modules<JMBanModule>.Get().Ban( steamID, name, message, seconds, issuedBy, issuedByName ).
    //!
    //! Bans used to be one JSON file per player, written with this. They are one Bans.json owned by
    //! JMBanModule now, so the record is handed to the module instead of written behind its back (the
    //! module would overwrite the file on its next save). ban.BanDuration is still the old absolute expiry:
    //! -1 stays permanent, an expiry already in the past lifts the ban. The key is ban.SteamID, or guid when
    //! the record carries none. Server only; a no-op when the module is not loaded.
    //!
    //! The module is looked up by name and called by name because JMBanModule lives in 5_Mission, above
    //! this file, and a `modded class` cannot add this from a higher layer.
    static void Save( JMPlayerBan ban, string guid )
    {
        JMDeprecated.WarnOnce( null, "JMPlayerBan.Save() is deprecated. Please use CF_Modules<JMBanModule>.Get().Ban() / Unban()." );

        if ( !ban )
            return;

        string key = ban.SteamID;
        if ( key == "" )
            key = guid;

        if ( key == "" )
            return;

        CF_ModuleCore module;
        if ( !CF_ModuleCoreManager.s_ModulesMap.Find( "JMBanModule".ToType(), module ) || !module )
            return;

        int seconds = -1;
        if ( ban.BanDuration != -1 )
        {
            seconds = ban.BanDuration - CF_Date.Now( true ).GetTimestamp();
            if ( seconds <= 0 )
            {
                g_Game.GameScript.CallFunctionParams( module, "Unban", NULL, new Param1< string >( key ) );
                return;
            }
        }

        g_Game.GameScript.CallFunctionParams( module, "Ban", NULL, new Param6< string, string, string, int, string, string >( key, ban.PlayerName, ban.Message, seconds, ban.IssuedBy, ban.IssuedByName ) );
    }

    // Mark a timed ban as expired in the store (called when expiry is detected at connect).
    //! Returns true if a timed ban was marked expired (the old per-file version
    //! returned whether it removed the file; third-party callers branch on it).
    static bool DeleteBanFile( string guid, string steamID )
    {
        if ( !FileExist( BAN_FILE ) )
            return false;

        JMPlayerBanStore store = new JMPlayerBanStore();
        string err;
        if ( !JsonFileLoader<JMPlayerBanStore>.LoadFile( BAN_FILE, store, err ) )
            return false;

        bool changed = false;
        foreach ( JMPlayerBan ban : store.Bans )
        {
            if ( ban && ( ban.SteamID == steamID || ban.SteamID == guid ) && !ban.IsPermanent() )
            {
                ban.BanDuration = 0;
                changed = true;
            }
        }

        if ( changed )
            JsonFileLoader<JMPlayerBanStore>.SaveFile( BAN_FILE, store, err );

        return changed;
    }
}