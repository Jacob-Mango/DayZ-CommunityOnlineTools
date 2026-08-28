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
			auto msg = m_Webhook.CreateDiscordMessageColored( JMConstants.WEBHOOK_COLOR_WARNING );

			msg.GetEmbed().SetTitle( "Territory Flag Placed" );
			msg.GetEmbed().SetDescription( PBplayer.FormatSteamWebhook() + " placed **" + item.GetDisplayName() + "**" );
			msg.GetEmbed().AddField( "Position", item.GetPosition().ToString(), false );

			m_Webhook.Post( "TerritoryDeployed", msg );
		}
		else
		{
			auto msg1 = m_Webhook.CreateDiscordMessageColored( JMConstants.WEBHOOK_COLOR_INFO );

			msg1.GetEmbed().SetTitle( "Item Placed" );
			msg1.GetEmbed().SetDescription( PBplayer.FormatSteamWebhook() + " placed **" + item.GetDisplayName() + "**" );
			msg1.GetEmbed().AddField( "Position", item.GetPosition().ToString(), false );

			m_Webhook.Post( "ItemDeployed", msg1 );
		}
	}

	override void PlayerList()
	{
		super.PlayerList();

		auto msg = m_Webhook.CreateDiscordMessageColored( JMConstants.WEBHOOK_COLOR_INFO );

		msg.GetEmbed().SetTitle( "Server Population" );
		msg.GetEmbed().SetDescription( "" + m_PlayerArray.Count() + " players are currently online." );

		m_Webhook.Post( "PlayerCount", msg );
	}

	override void PlayerKilled( PlayerBase player, Object source )
	{
		super.PlayerKilled( player, source );

		if ( !player || !source || !m_Webhook )
			return;

		// Create compensation backup before processing webhooks
		JMCompensationHelper.CreateCompensationBackup( player );

		COT_WebHookPlayerKilled(player, source);		// meant for admins mostly
		COT_WebHookPlayerKilled(player, source, false); // for public killfeed stuff
	}

	void COT_WebHookPlayerKilled(PlayerBase player, Object source, bool showPos = true)
	{
		auto message = m_Webhook.CreateDiscordMessageColored( JMConstants.WEBHOOK_COLOR_DANGER );
		auto embed = message.GetEmbed();

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
				embed.SetTitle( "Player Died" );
				embed.SetDescription( player.FormatSteamWebhook() + " died of unnatural causes." );
			} else
			{
				embed.SetTitle( "Player Died" );
				embed.SetDescription( player.FormatSteamWebhook() + " died of natural causes." );
			}

		} else if ( source.IsWeapon() || source.IsMeleeWeapon() )
		{
			Class.CastTo( pbKiller, EntityAI.Cast( source ).GetHierarchyParent() );

			if ( pbKiller )
			{
				string distanceWeapon = "";
				if ( !source.IsMeleeWeapon() )
					distanceWeapon = " (" + vector.Distance( player.GetPosition(), pbKiller.GetPosition() ) + "m)";

				if (showPos)
					deathBreakdown += "Killer Position: " + pbKiller.GetPosition() + "\n";

				embed.SetTitle( "Player Killed" );
				embed.SetDescription( player.FormatSteamWebhook() + " was killed by " + pbKiller.FormatSteamWebhook() + " with **" + source.GetDisplayName() + "**" + distanceWeapon );
			}
		} else
		{
			embed.SetTitle( "Player Died" );
			embed.SetDescription( player.FormatSteamWebhook() + " was killed by **" + source.GetDisplayName() + "**" );
		}

		if ( deathBreakdown != "" )
			embed.AddField( "Breakdown", deathBreakdown, false );

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


		float healthDmg;
		float shockDmg;
		float bloodDmg;

		if (damageResult)
		{
			healthDmg = damageResult.GetHighestDamage("Health");
			shockDmg = damageResult.GetHighestDamage("Shock");
			bloodDmg = damageResult.GetHighestDamage("Blood");
		}

		bool isFallDamage;

		auto message = m_Webhook.CreateDiscordMessageColored( JMConstants.WEBHOOK_COLOR_DANGER );
		auto embed = message.GetEmbed();

		PlayerBase pbKiller = NULL;
		string damageDescription;

		switch ( damageType )
		{
		case DT_CLOSE_COMBAT:
			if ( source.IsZombie() || source.IsAnimal() )
			{
				damageDescription = player.FormatSteamWebhook() + " was hit by **" + source.GetDisplayName() + "**";
			} else if ( source.IsPlayer() )
			{
				Class.CastTo( pbKiller, source );
				damageDescription = player.FormatSteamWebhook() + " was hit by " + pbKiller.FormatSteamWebhook() + " using fists";
			} else if ( source.IsMeleeWeapon() )
			{
				Class.CastTo( pbKiller, source.GetHierarchyParent() );
				damageDescription = player.FormatSteamWebhook() + " was hit by " + pbKiller.FormatSteamWebhook() + " with **" + source.GetDisplayName() + "**";
			} else
			{
				damageDescription = player.FormatSteamWebhook() + " was hit by **" + source.GetDisplayName() + "**";
			}
			break;
		case DT_FIRE_ARM:
			if ( source.IsWeapon() )
			{
				Class.CastTo( pbKiller, source.GetHierarchyParent() );
				float distance = vector.Distance( player.GetPosition(), pbKiller.GetPosition() );
				damageDescription = player.FormatSteamWebhook() + " was shot by " + pbKiller.FormatSteamWebhook() + " with **" + source.GetDisplayName() + "** (" + distance + "m)";
			} else
			{
				damageDescription = player.FormatSteamWebhook() + " was hit by **" + source.GetDisplayName() + "**";
			}
			break;
		case DT_EXPLOSION:
			damageDescription = player.FormatSteamWebhook() + " was hit by an **explosion**";
			break;
		case DT_STUN:
			damageDescription = player.FormatSteamWebhook() + " was stunned";
			break;
		case DT_CUSTOM:
			if (ammo.IndexOf("FallDamage") == 0)
			{
				if (healthDmg == 0 && bloodDmg == 0 && shockDmg == 0)
					return;

				isFallDamage = true;
				damageDescription = player.FormatSteamWebhook() + " took fall damage";
			} else
			{
				if ( source )
					damageDescription = player.FormatSteamWebhook() + " was hit by **" + source.GetDisplayName() + "**";
			}
			break;
		default:
			damageDescription = player.FormatSteamWebhook() + " was hit by an unknown damage type";
			break;
		}

		embed.SetTitle( "Player Damaged" );
		embed.SetDescription( damageDescription );

		string hitMessage = "";
		if ( damageResult )
		{
			if (healthDmg > 0)
				hitMessage += "Damage: " + healthDmg + "\n";
			if (bloodDmg > 0)
				hitMessage += "Blood loss: " + bloodDmg + "\n";
			if (shockDmg > 0)
				hitMessage += "Shock: " + shockDmg + "\n";
		}

		if (dmgZone)
			hitMessage += "Zone: " + dmgZone + "\n";

		if (!isFallDamage)
			hitMessage += "Type: " + ammo + "\n";

		if (hitMessage != "")
			embed.AddField( "Damage Breakdown", hitMessage, false );

		m_Webhook.Post( "PlayerDamage", message );
	}
}