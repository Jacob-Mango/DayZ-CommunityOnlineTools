modded class Weapon_Base
{
	override void EEFired( int muzzleType, int mode, string ammoType )
	{
		super.EEFired( muzzleType, mode, ammoType );

		PlayerBase player;
		if ( Class.CastTo( player, GetHierarchyRootPlayer() ) && player.COTHasUnlimitedAmmo() )
		{
			Magazine magazine = GetMagazine( GetCurrentMuzzle() );
			if ( magazine )
			{
				if ( GetGame().IsServer() )
				{
					if ( GetGame().IsMultiplayer() && magazine )
						GetGame().RemoteObjectDelete( magazine );

					magazine.ServerSetAmmoMax();

					if ( GetGame().IsMultiplayer() && magazine )
						GetGame().RemoteObjectCreate( magazine );
				} else
				{
					magazine.LocalSetAmmoMax();
				}
			}
		}
	}
};