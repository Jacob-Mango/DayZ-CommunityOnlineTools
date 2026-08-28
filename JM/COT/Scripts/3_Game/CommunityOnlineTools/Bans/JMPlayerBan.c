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
    //  Helpers
    // -------------------------------------------------------------------------

    bool IsPermanent()
    {
        return BanDuration == -1;
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

    // -------------------------------------------------------------------------
    //  Static helpers - called by MissionServer.IsCOTBanned at connect time.
    //  These read/write Bans.json directly so they work before the module loads.
    // -------------------------------------------------------------------------

    static const string BAN_FILE = "$profile:CommunityOnlineTools\\Bans\\Bans.json";
    static const string BAN_DIR  = "$profile:CommunityOnlineTools\\Bans\\";

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

    // Mark a timed ban as expired in the store (called when expiry is detected at connect).
    static void DeleteBanFile( string guid, string steamID )
    {
        if ( !FileExist( BAN_FILE ) )
            return;

        JMPlayerBanStore store = new JMPlayerBanStore();
        string err;
        if ( !JsonFileLoader<JMPlayerBanStore>.LoadFile( BAN_FILE, store, err ) )
            return;

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
    }
}