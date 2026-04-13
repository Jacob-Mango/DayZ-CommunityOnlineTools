// =============================================================================
//  JMPlayerBanStore.c
//
//  Container class whose single public member is the ban array.
//  JsonFileLoader<JMPlayerBanStore> reflects this automatically, producing:
//
//    {
//      "Bans": [
//        { "SteamID": "...", "PlayerName": "...", ... },
//        ...
//      ]
//    }
//
//  This is exactly the same pattern COT uses for its settings / loadout
//  classes — one wrapper class, one array member, JsonFileLoader does the rest.
// =============================================================================

class JMPlayerBanStore
{
    ref array<ref JMPlayerBan> Bans = new array<ref JMPlayerBan>();
}