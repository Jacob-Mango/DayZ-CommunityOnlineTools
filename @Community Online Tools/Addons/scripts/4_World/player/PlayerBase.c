
modded class PlayerBase
{
    //protected bool m_HasGodeMode;

    override void Init()
    {
        super.Init();

        //m_HasGodeMode = false;

        //RegisterNetSyncVariableBool( "m_HasGodeMode" );
    }

    /*
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
    */

    /*
    override void EOnFrame( IEntity other, float timeSlice )
    {
        super.EOnFrame( other, timeSlice );

        if ( m_HasGodeMode )
        {
            SetAllowDamage( false );
            SetHealth( "", "Health", GetMaxHealth( "", "Health" ) );
            SetHealth( "", "Blood", GetMaxHealth( "", "Blood" ) );
            SetHealth( "", "Shock", GetMaxHealth( "", "Shock" ) );
        }
    }

    override void EOnPostFrame( IEntity other, int extra )
    {
        super.EOnPostFrame( other, extra );

        if ( m_HasGodeMode )
        {
            SetAllowDamage( false );
            SetHealth( "", "Health", GetMaxHealth( "", "Health" ) );
            SetHealth( "", "Blood", GetMaxHealth( "", "Blood" ) );
            SetHealth( "", "Shock", GetMaxHealth( "", "Shock" ) );
        }
    }

    override void EEHitBy( TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos )
	{
        Print( "PlayerBase::EEHitBy" );
        if ( m_HasGodeMode ) 
        {
            Print( "Godmode should've prevented this!" );

            SetHealth( "", "Health", GetMaxHealth( "", "Health" ) );
            SetHealth( "", "Blood", GetMaxHealth( "", "Blood" ) );
            SetHealth( "", "Shock", GetMaxHealth( "", "Shock" ) );
        } else 
        {
		    super.EEHitBy( damageResult, damageType, source, component, dmgZone, ammo, modelPos );
        }
    }

    override void EEKilled( Object killer )
    {
        Print( "PlayerBase::EEKilled" );
        if ( m_HasGodeMode ) 
        {
            Print( "Godmode should've prevented this!" );

            SetHealth( "", "Health", GetMaxHealth( "", "Health" ) );
            SetHealth( "", "Blood", GetMaxHealth( "", "Blood" ) );
            SetHealth( "", "Shock", GetMaxHealth( "", "Shock" ) );
        } else 
        {
            super.EEKilled( killer );
        }
    }

    override void CheckDeath()
    {
        if ( m_HasGodeMode ) 
        {
            Print( "Godmode should've prevented this!" );

            SetHealth( "", "Health", GetMaxHealth( "", "Health" ) );
            SetHealth( "", "Blood", GetMaxHealth( "", "Blood" ) );
            SetHealth( "", "Shock", GetMaxHealth( "", "Shock" ) );
        } else
        {
            super.CheckDeath();
        }
    }
    
    override void SimulateDeath( bool state )
    {
        if ( m_HasGodeMode ) 
        {
            Print( "Godmode should've prevented this!" );

            SetHealth( "", "Health", GetMaxHealth( "", "Health" ) );
            SetHealth( "", "Blood", GetMaxHealth( "", "Blood" ) );
            SetHealth( "", "Shock", GetMaxHealth( "", "Shock" ) );
            super.SimulateDeath( false );
        } else
        {
            super.SimulateDeath( state );
        }
    }

    void SetGodMode( bool mode )
    {
        m_HasGodeMode = mode;

        if ( m_HasGodeMode )
        {
            SetAllowDamage( false );
            Notify( authenticatedPlayer, "You now have god mode." );
        } else
        {
            SetAllowDamage( true );
            Notify( authenticatedPlayer, "You no longer have god mode." );
        }
    }
    */ 
}