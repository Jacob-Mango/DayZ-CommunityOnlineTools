modded class DrowningMdfr
{
	override bool CheckIsDrowning( PlayerBase player )
	{
		if ( player.COTHasGodMode() )
			return false;

		return super.CheckIsDrowning( player );
	}
};

