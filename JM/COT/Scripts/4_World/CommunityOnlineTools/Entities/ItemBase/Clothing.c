modded class Clothing
{
	override bool CanHaveWetness()
	{
		PlayerBase player;
		if ( Class.CastTo( player, GetHierarchyRootPlayer() ) && player.COTHasGodMode() )
			return false;

		return super.CanHaveWetness();
	}
}
