//  JMPlayerBanStore.c
//  Wrapper whose single public member is the ban array; JsonFileLoader reflects it
//  as { "Bans": [ { "SteamID": "...", ... } ] }, same pattern as COT settings classes.

class JMPlayerBanStore
{
    ref array<ref JMPlayerBan> Bans = new array<ref JMPlayerBan>();
}