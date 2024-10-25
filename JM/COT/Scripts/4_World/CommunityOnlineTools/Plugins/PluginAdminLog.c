modded class PluginAdminLog
{
	private JMWebhookModule m_Webhook;

	private string m_LoggingType;

	void SetWebhook( JMWebhookModule webhook )
	{
		m_Webhook = webhook;
	}
	
	override void OnPlacementComplete( Man player, ItemBase item )
	{
		super.OnPlacementComplete(player, item);

		PlayerBase PBplayer = PlayerBase.Cast(player);
		if (!PBplayer)
			return;

		TerritoryFlag territory;
		if (Class.CastTo( territory, item ))
		{
			auto msg = m_Webhook.CreateDiscordMessage();

			msg.GetEmbed().AddField( "Item Placement", "" + PBplayer.FormatSteamWebhook() + " Placed " + item.GetDisplayName() + " at "+ item.GetPosition(), false );

			m_Webhook.Post( "TerritoryDeployed", msg );
		}
		else
		{
			auto msg1 = m_Webhook.CreateDiscordMessage();

			msg1.GetEmbed().AddField( "Item Placement", "" + PBplayer.FormatSteamWebhook() + " Placed " + item.GetDisplayName() + " at "+ item.GetPosition(), false );

			m_Webhook.Post( "ItemDeployed", msg1 );
		}
	}
	
	override void PlayerList()
	{
		super.PlayerList();

		auto msg = m_Webhook.CreateDiscordMessage();

		msg.GetEmbed().AddField( "Server Population", "" + m_PlayerArray.Count() + " Players are currently on the server.", false );

		m_Webhook.Post( "PlayerCount", msg );
	}

	override void PlayerKilled( PlayerBase player, Object source )  
	{
		super.PlayerKilled( player, source );

		if ( !player || !source || !m_Webhook )
			return;

		COT_WebHookPlayerKilled(player, source);		// meant for admins mostly
		COT_WebHookPlayerKilled(player, source, false); // for public killfeed stuff
	}

	void COT_WebHookPlayerKilled(PlayerBase player, Object source, bool showPos = true)
	{
		auto message = m_Webhook.CreateDiscordMessage();
		auto embed = message.GetEmbed();
		embed.SetColor( 16711680 ); // 0xFF0000

		PlayerBase pbKiller = NULL;

		string deathBreakdown;

		if (showPos)
			deathBreakdown += "Position: " + player.GetPosition() + "\n";

		if ( player == source )
		{
			m_StatWater  = player.GetStatWater();
			m_StatEnergy = player.GetStatEnergy();
			m_BleedMgr   = player.GetBleedingManagerServer();

			if ( m_StatWater )
				deathBreakdown += "Water: " + m_StatWater.Get().ToString() + "\n";
			if ( m_StatEnergy )
				deathBreakdown += "Energy: " + m_StatEnergy.Get().ToString() + "\n";
			if ( m_BleedMgr )
				deathBreakdown += "Cuts: " + m_BleedMgr.GetBleedingSourcesCount().ToString() + "\n";

			if ( deathBreakdown == "" )
			{
				deathBreakdown = "Unknown cause of death";

				embed.AddField( "Player Death", "" + player.FormatSteamWebhook() + " died of unnatural causes." );
			} else
			{
				embed.AddField( "Player Death", "" + player.FormatSteamWebhook() + " died of natural causes." );
			}

		} else if ( source.IsWeapon() || source.IsMeleeWeapon() )
		{
			Class.CastTo( pbKiller, EntityAI.Cast( source ).GetHierarchyParent() );

			if ( pbKiller )
			{
				string distanceWeapon = "";
				if ( !source.IsMeleeWeapon() )
					distanceWeapon = " from " + vector.Distance( player.GetPosition(), pbKiller.GetPosition() ) + " meters.";
				
				if (showPos)
					deathBreakdown += "Killer Position: " + pbKiller.GetPosition() + "\n";

				embed.AddField( "Player Death", "" + player.FormatSteamWebhook() + " was killed by " + pbKiller.FormatSteamWebhook() + " with " + source.GetDisplayName() + distanceWeapon );
			}
		} else
		{
			embed.AddField( "Player Death", "" + player.FormatSteamWebhook() + " was killed by " + source.GetDisplayName() );
		}

		embed.AddField( "Breakdown", deathBreakdown );

		if (showPos)
			m_Webhook.Post( "PlayerDeath", message );
		else
			m_Webhook.Post( "PlayerDeathNoPos", message );
	}
	
	override void PlayerHitBy( TotalDamageResult damageResult, int damageType, PlayerBase player, EntityAI source, int component, string dmgZone, string ammo ) // PlayerBase.c 
	{
		super.PlayerHitBy( damageResult, damageType, player, source, component, dmgZone, ammo );

		if ( !player || !source || !m_Webhook )
			return;

		auto message = m_Webhook.CreateDiscordMessage();
		auto embed = message.GetEmbed();
		embed.SetColor( 16711680 ); // 0xFF0000

		PlayerBase pbKiller = NULL;

		switch ( damageType )
		{
		case DT_CLOSE_COMBAT:
			if ( source.IsZombie() || source.IsAnimal() )
			{
				embed.AddField( "Player Damaged", "" + player.FormatSteamWebhook() + " was hit by " + source.GetDisplayName() );
			} else if ( source.IsPlayer() )
			{
				Class.CastTo( pbKiller, source );
				embed.AddField( "Player Damaged", "" + player.FormatSteamWebhook() + " was hit by " + pbKiller.FormatSteamWebhook() + " using fists" );
			} else if ( source.IsMeleeWeapon() )
			{
				Class.CastTo( pbKiller, source.GetHierarchyParent() );
				embed.AddField( "Player Damaged", "" + player.FormatSteamWebhook() + " was hit by " + pbKiller.FormatSteamWebhook() + " with " + source.GetDisplayName() );
			} else
			{
				embed.AddField( "Player Damaged", "" + player.FormatSteamWebhook() + " was hit by " + source.GetDisplayName() );
			}
			break;
		case DT_FIRE_ARM:
			if ( source.IsWeapon() )
			{
				Class.CastTo( pbKiller, source.GetHierarchyParent() );
				float distance = vector.Distance( player.GetPosition(), pbKiller.GetPosition() );

				embed.AddField( "Player Damaged", "" + player.FormatSteamWebhook() + " was hit by " + pbKiller.FormatSteamWebhook() + " with " + source.GetDisplayName() + " from " + distance + " meters." );
			} else
			{
				embed.AddField( "Player Damaged", "" + player.FormatSteamWebhook() + " was hit by " + source.GetDisplayName() );
			}
			break;
		case DT_EXPLOSION:
			embed.AddField( "Player Damaged", "" + player.FormatSteamWebhook() + " was hit by an explosion." );
			break;
		case DT_STUN:
			embed.AddField( "Player Damaged", "" + player.FormatSteamWebhook() + " was stunned." );
			break;
		case DT_CUSTOM:
			if ( ammo == "FallDamage" )
			{
				embed.AddField( "Player Damaged", "" + player.FormatSteamWebhook() + " fell." );
			} else
			{
				if ( source )
				{
					embed.AddField( "Player Damaged", "" + player.FormatSteamWebhook() + " was hit by " + source.GetDisplayName() );
				}
			}

			break;
		default:
			embed.AddField( "Player Damaged", "WARNING: " + player.FormatSteamWebhook() + " was hit by an unknown damage type!" );
			break;
		}

		string hitMessage = "";
		if ( damageResult )
		{
			hitMessage += "Damage: " + damageResult.GetHighestDamage( "Health" ) + "\n";
		}

		hitMessage += "Zone: " + dmgZone + "\n";
		hitMessage += "Component: " + component + "\n";
		hitMessage += "Ammo: " + ammo + "\n";

		embed.AddField( "Damage Breakdown", hitMessage );

		m_Webhook.Post( "PlayerDamage", message );
	}
};