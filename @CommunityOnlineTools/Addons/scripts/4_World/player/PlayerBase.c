/*
modded class PlayerBase
{	
	void HandleDeath( Object killerItem ) 
	{
        PlayerBase killer = PlayerBase.Cast( killerItem );

		if ( !authenticatedPlayer ) return;
        
        if ( !killer )
        {
			killer = PlayerBase.Cast( EntityAI.Cast( killerItem ).GetHierarchyParent() );

            if ( !killer )
            {
                return;
            }
        }

		if ( !killer.authenticatedPlayer ) return;

		if ( killer.authenticatedPlayer.GetGUID() == authenticatedPlayer.GetGUID() )
		{
			killer.RPCSingleParam(ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( killer.authenticatedPlayer.GetName() + " has killed themselves." ), false, NULL);
			return;
		}

        authenticatedPlayer.Data.Kills = 0;

        killer.authenticatedPlayer.Data.Kills++;
        killer.authenticatedPlayer.Data.TotalKills++;

		killer.RPCSingleParam(ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "You have " + killer.authenticatedPlayer.Data.Kills.ToString() + " kill(s)." ), false, killer.GetIdentity());
		
		string message = "";

		switch ( killer.authenticatedPlayer.Data.Kills )
		{
			case 5:
				message = " is on a killing spree!";
			    break;
			case 10:
				message = " is dominating!";
			    break;
			case 15:
				message = " is Unstoppable!";
			    break;
			case 20:
				message = " is Unstoppable!";
			    break;
			case 25:
				message = " is Godlike!";
			    break;
		}

		if ( killer.authenticatedPlayer.Data.Kills > 25 ) 
		{
			message = " is Godlike!";
		}
		
		int distance = Math.Round( vector.Distance( killer.GetPosition(), GetPosition()) );

		string weapon = killerItem.GetType();

		killer.RPCSingleParam(ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( killer.authenticatedPlayer.GetName() + " has killed " + authenticatedPlayer.GetName() + " (" + weapon + ")" + "(" + distance + "m)" ), false, NULL);
		
		if ( message.Length() > 0 )
		{
			killer.RPCSingleParam(ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( killer.authenticatedPlayer.GetName() + message ), false, NULL );
		}
	}
	
	override void EEKilled( Object killer )
	{		
		HandleDeath( killer );

        super.EEKilled( killer );
    }
}
*/