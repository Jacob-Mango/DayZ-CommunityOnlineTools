modded class PluginAdminLog
{
	protected JMWebhookModule m_Webhook;
	protected string m_LoggingType;

	//! How long a hit still counts as "what killed them" for the death webhook.
	static const float LAST_HIT_WINDOW = 15.0;

	//! The last hit each player took, by their identity id (AI have none and are not tracked).
	protected ref map< string, ref JMWebhookLastHit > m_LastHits;

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
			msg.GetEmbed().AddField( "Player", PBplayer.FormatSteamWebhook(), true );
			msg.GetEmbed().AddField( "Item", item.GetType(), true );
			msg.GetEmbed().AddField( "Position", JMWebhookModule.FormatPosition( item.GetPosition() ), true );

			m_Webhook.PostAs( "TerritoryDeployed", msg, m_Webhook.InstanceOf( PBplayer ) );
		}
		else
		{
			auto msg1 = m_Webhook.CreateDiscordMessageColored( JMConstants.WEBHOOK_COLOR_INFO );

			msg1.GetEmbed().SetTitle( "Item Placed" );
			msg1.GetEmbed().SetDescription( PBplayer.FormatSteamWebhook() + " placed **" + item.GetDisplayName() + "**" );
			msg1.GetEmbed().AddField( "Player", PBplayer.FormatSteamWebhook(), true );
			msg1.GetEmbed().AddField( "Item", item.GetType(), true );
			msg1.GetEmbed().AddField( "Position", JMWebhookModule.FormatPosition( item.GetPosition() ), true );

			m_Webhook.PostAs( "ItemDeployed", msg1, m_Webhook.InstanceOf( PBplayer ) );
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
		COT_WebHookKillfeed( player, source );
	}

	//! Who dealt the killing blow and with what. `weaponName` is the weapon, "Fists", or the
	//! thing that did it (zombie, grenade) when no player was behind it.
	protected PlayerBase ResolveKiller( PlayerBase victim, Object source, out string weaponName )
	{
		weaponName = "";

		PlayerBase killer = NULL;

		if ( !source || victim == source )
			return NULL;

		if ( source.IsWeapon() || source.IsMeleeWeapon() )
		{
			weaponName = source.GetDisplayName();

			EntityAI weaponEntity = EntityAI.Cast( source );
			if ( weaponEntity )
				Class.CastTo( killer, weaponEntity.GetHierarchyParent() );
		}
		else if ( PlayerBase.Cast( source ) )
		{
			Class.CastTo( killer, source );
			weaponName = "Fists";
		}
		else
		{
			weaponName = source.GetDisplayName();
		}

		return killer;
	}

	protected string LastHitKey( PlayerBase player )
	{
		PlayerIdentity identity = player.GetIdentity();
		if ( !identity )
			return "";

		return identity.GetId();
	}

	//! The hit the player took last, if it was recent enough to be what killed them.
	protected JMWebhookLastHit GetRecentHit( PlayerBase player )
	{
		if ( !m_LastHits )
			return NULL;

		string key = LastHitKey( player );
		if ( key == "" )
			return NULL;

		JMWebhookLastHit hit = m_LastHits.Get( key );
		if ( !hit )
			return NULL;

		if ( g_Game.GetTickTime() - hit.HitTime > LAST_HIT_WINDOW )
			return NULL;

		return hit;
	}

	//! The embed's "who / where" pair: the victim ("To") and the one who did it ("From"), each
	//! with where they stood when the position is being shown.
	protected void AddFromTo( JMWebhookDiscordEmbed embed, PlayerBase attacker, string attackerText, PlayerBase victim, bool showPos )
	{
		string fromText = attackerText;
		if ( attacker )
		{
			fromText = attacker.FormatSteamWebhook();

			if ( showPos )
				fromText += "\n" + JMWebhookModule.FormatPosition( attacker.GetPosition() );
		}

		if ( fromText != "" )
			embed.AddField( "From:", fromText, true );

		string toText = victim.FormatSteamWebhook();
		if ( showPos )
			toText += "\n" + JMWebhookModule.FormatPosition( victim.GetPosition() );

		embed.AddField( "To:", toText, true );
	}

	void COT_WebHookPlayerKilled(PlayerBase player, Object source, bool showPos = true)
	{
		auto message = m_Webhook.CreateDiscordMessageColored( JMConstants.WEBHOOK_COLOR_DANGER );
		auto embed = message.GetEmbed();

		string weaponName;
		PlayerBase pbKiller = ResolveKiller( player, source, weaponName );

		JMWebhookLastHit lastHit = GetRecentHit( player );

		string deathBreakdown;

		float distance = -1;
		if ( pbKiller && !source.IsMeleeWeapon() && pbKiller != player )
			distance = vector.Distance( player.GetPosition(), pbKiller.GetPosition() );

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

			embed.SetTitle( "Player Died" );

			if ( deathBreakdown == "" )
				embed.SetDescription( player.FormatSteamWebhook() + " died of unnatural causes." );
			else
				embed.SetDescription( player.FormatSteamWebhook() + " died of natural causes." );
		}
		else if ( pbKiller )
		{
			string killedBy = player.FormatSteamWebhook() + " was killed by " + pbKiller.FormatSteamWebhook() + " with **" + weaponName + "**";

			if ( distance >= 0 )
				killedBy += " (" + Math.Round( distance ).ToString() + "m)";

			embed.SetTitle( "Player Killed" );
			embed.SetDescription( killedBy );
		}
		else
		{
			embed.SetTitle( "Player Died" );
			embed.SetDescription( player.FormatSteamWebhook() + " was killed by **" + weaponName + "**" );
		}

		string attackerText = "";
		if ( !pbKiller && player != source )
			attackerText = "**" + weaponName + "**";

		AddFromTo( embed, pbKiller, attackerText, player, showPos );

		if ( weaponName != "" )
			embed.AddField( "Weapon:", weaponName, true );

		if ( distance >= 0 )
			embed.AddField( "Distance:", Math.Round( distance ).ToString() + " m", true );

		if ( lastHit )
		{
			if ( lastHit.HitAmmo != "" )
				embed.AddField( "Ammo:", lastHit.HitAmmo, true );

			if ( lastHit.HitZone != "" )
				embed.AddField( "Hit zone:", lastHit.HitZone, true );

			embed.AddField( "Last hit:", lastHit.HitKind + ", " + JMWebhookModule.FormatNumber( lastHit.HealthDamage ) + " damage", true );
		}

		if ( deathBreakdown != "" )
			embed.AddField( "Breakdown", deathBreakdown, false );

		//! Tracked through the victim and, when there is one, the killer.
		JMPlayerInstance victimInstance = m_Webhook.InstanceOf( player );
		JMPlayerInstance killerInstance = m_Webhook.InstanceOf( pbKiller );

		if (showPos)
			m_Webhook.PostAs( "PlayerDeath", message, victimInstance, killerInstance );
		else
			m_Webhook.PostAs( "PlayerDeathNoPos", message, victimInstance, killerInstance );
	}

	//! The killfeed: one line, "X killed Y", nothing else. Its own webhook type so a public
	//! channel can have just this without the admin detail of PlayerDeath.
	void COT_WebHookKillfeed( PlayerBase player, Object source )
	{
		string weaponName;
		PlayerBase pbKiller = ResolveKiller( player, source, weaponName );

		JMWebhookLastHit lastHit = GetRecentHit( player );

		string line;

		if ( player == source )
		{
			line = player.FormatSteamWebhook() + " died";
		}
		else if ( pbKiller )
		{
			line = pbKiller.FormatSteamWebhook() + " killed " + player.FormatSteamWebhook();

			if ( weaponName != "" )
				line += " with **" + weaponName + "**";

			if ( !source.IsMeleeWeapon() )
				line += " (" + Math.Round( vector.Distance( player.GetPosition(), pbKiller.GetPosition() ) ).ToString() + "m)";

			if ( lastHit && lastHit.HitZone == "Head" )
				line += " - headshot";
		}
		else
		{
			line = player.FormatSteamWebhook() + " was killed by **" + weaponName + "**";
		}

		auto message = m_Webhook.CreateDiscordMessageColored( JMConstants.WEBHOOK_COLOR_DANGER );
		message.GetEmbed().SetDescription( line );

		m_Webhook.PostAs( "PlayerKillfeed", message, m_Webhook.InstanceOf( player ), m_Webhook.InstanceOf( pbKiller ) );
	}

	protected string FormatKind( int damageType )
	{
		switch ( damageType )
		{
		case DT_CLOSE_COMBAT:
			return "Melee";
		case DT_FIRE_ARM:
			return "Firearm";
		case DT_EXPLOSION:
			return "Explosion";
		case DT_STUN:
			return "Stun";
		}

		return "Environment";
	}

	//! "Health: 80 -> 62 (-18)" - what the stat was before the hit, after it, and the difference.
	protected string FormatDamageLine( string stat, float after, float damage )
	{
		return stat + ": " + JMWebhookModule.FormatNumber( after + damage ) + " -> " + JMWebhookModule.FormatNumber( after ) + " (-" + JMWebhookModule.FormatNumber( damage ) + ")\n";
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

		bool isFallDamage = false;

		if ( damageType == DT_CUSTOM && ammo.IndexOf("FallDamage") == 0 )
		{
			if (healthDmg == 0 && bloodDmg == 0 && shockDmg == 0)
				return;

			isFallDamage = true;
		}

		//! Who hit them and with what: a player behind a weapon or fists, otherwise the thing
		//! itself (zombie, animal, grenade).
		PlayerBase pbKiller = NULL;
		string weaponName = source.GetDisplayName();

		if ( damageType == DT_CLOSE_COMBAT )
		{
			if ( source.IsPlayer() )
			{
				Class.CastTo( pbKiller, source );
				weaponName = "Fists";
			}
			else if ( source.IsMeleeWeapon() )
			{
				Class.CastTo( pbKiller, source.GetHierarchyParent() );
			}
		}
		else if ( damageType == DT_FIRE_ARM && source.IsWeapon() )
		{
			Class.CastTo( pbKiller, source.GetHierarchyParent() );
		}

		float distance = -1;
		if ( pbKiller && damageType == DT_FIRE_ARM )
			distance = vector.Distance( player.GetPosition(), pbKiller.GetPosition() );

		string kind = FormatKind( damageType );
		if ( isFallDamage )
			kind = "Fall";

		string description = player.FormatSteamWebhook();

		if ( isFallDamage )
		{
			description += " took fall damage";
		}
		else if ( damageType == DT_EXPLOSION )
		{
			description += " was hit by an **explosion**";
		}
		else if ( damageType == DT_STUN )
		{
			description += " was stunned";
		}
		else
		{
			if ( damageType == DT_FIRE_ARM )
				description += " was shot by ";
			else
				description += " was hit by ";

			if ( pbKiller )
				description += pbKiller.FormatSteamWebhook() + " with **" + weaponName + "**";
			else
				description += "**" + weaponName + "**";

			if ( distance >= 0 )
				description += " (" + Math.Round( distance ).ToString() + "m)";
		}

		int color = JMConstants.WEBHOOK_COLOR_DANGER;
		if ( !pbKiller )
			color = JMConstants.WEBHOOK_COLOR_WARNING;

		auto message = m_Webhook.CreateDiscordMessageColored( color );
		auto embed = message.GetEmbed();

		embed.SetTitle( "Player Damaged" );
		embed.SetDescription( description );

		string attackerText = "";
		if ( !pbKiller && !isFallDamage )
			attackerText = "**" + weaponName + "**";

		AddFromTo( embed, pbKiller, attackerText, player, true );

		if ( !isFallDamage && weaponName != "" )
			embed.AddField( "Weapon:", weaponName, true );

		if ( ammo != "" && !isFallDamage )
			embed.AddField( "Ammo:", ammo, true );

		if ( distance >= 0 )
			embed.AddField( "Distance:", Math.Round( distance ).ToString() + " m", true );

		if ( dmgZone != "" )
			embed.AddField( "Hit zone:", dmgZone, true );

		embed.AddField( "Type:", kind, true );

		string damageText = "";
		if ( damageResult )
		{
			if ( healthDmg > 0 )
				damageText += FormatDamageLine( "Health", player.GetHealth( "", "Health" ), healthDmg );
			if ( bloodDmg > 0 )
				damageText += FormatDamageLine( "Blood", player.GetHealth( "", "Blood" ), bloodDmg );
			if ( shockDmg > 0 )
				damageText += FormatDamageLine( "Shock", player.GetHealth( "", "Shock" ), shockDmg );
		}

		if ( damageText != "" )
			embed.AddField( "Damage:", damageText, false );

		//! Remembered for the death webhook if this is the hit that kills them.
		string hitKey = LastHitKey( player );
		if ( hitKey != "" )
		{
			if ( !m_LastHits )
				m_LastHits = new map< string, ref JMWebhookLastHit >;

			JMWebhookLastHit lastHit = new JMWebhookLastHit();
			lastHit.WeaponName   = weaponName;
			lastHit.HitAmmo     = ammo;
			lastHit.HitZone     = dmgZone;
			lastHit.HitKind     = kind;
			lastHit.Metres = distance;
			lastHit.HealthDamage   = healthDmg;
			lastHit.HitTime     = g_Game.GetTickTime();

			if ( pbKiller )
				lastHit.AttackerText = pbKiller.FormatSteamWebhook();

			m_LastHits.Set( hitKey, lastHit );
		}

		m_Webhook.PostAs( "PlayerDamage", message, m_Webhook.InstanceOf( player ), m_Webhook.InstanceOf( pbKiller ) );
	}
}
