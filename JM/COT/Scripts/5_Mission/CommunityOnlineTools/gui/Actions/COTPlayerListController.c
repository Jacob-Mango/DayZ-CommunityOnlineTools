//! COTFilteredListController for JMPlayerInstance rows. The base class matches
//! on Class.ToString(), which for an object is its pointer, so a name search
//! could never hit; this matches on what the admin actually types.
class COTPlayerListController: COTFilteredListController
{
	override protected bool MatchesFilter( Class item, string query )
	{
		JMPlayerInstance player;
		if ( !Class.CastTo( player, item ) )
			return false;

		string name = player.GetName();
		name.ToLower();
		if ( name.Contains( query ) )
			return true;

		string guid = player.GetGUID();
		guid.ToLower();
		if ( guid.Contains( query ) )
			return true;

		string steam = player.GetSteam64ID();
		steam.ToLower();
		return steam.Contains( query );
	}
}
