modded class ItemBase
{
	//! Last player to throw or place this item, for kill attribution.
	//! ItemBase is the common ancestor of both ExplosivesBase and TrapBase, so
	//! tagging here covers grenades, mines, claymores and bear traps in one
	//! place. Only set for trackable items -- see COT_IsKillTrackable().
	//!
	//! Needed because the thrower is long gone (and the explosive itself is
	//! deleted) by the time EEKilled fires on the victim.
	protected PlayerBase m_COT_KillOwner;

	//! Only explosives and traps can kill someone after leaving the owner's
	//! hands, so everything else skips tagging entirely.
	bool COT_IsKillTrackable()
	{
		return IsExplosive() || IsInherited( TrapBase );
	}

	void COT_SetKillOwner( PlayerBase owner )
	{
		m_COT_KillOwner = owner;
	}

	PlayerBase COT_GetKillOwner()
	{
		return m_COT_KillOwner;
	}

	//! Tracker hooks live on EEInit/EEDelete, not the ctor/dtor. Enforce
	//! destructors run on GC, not on deletion, so a dtor-based unregister lags
	//! and GetByClassname hands out entities that are already gone. EEDelete is
	//! deterministic. See docs/audits/2026-07-15-todo-mods-audit.md.
	override void EEInit()
	{
		super.EEInit();

		if (g_Game.IsServer())
			JMEntityTracker.Register(this);
	}

	//! Thrown grenade: tag the player whose hands it just left.
	override void OnInventoryExit( Man player )
	{
		super.OnInventoryExit( player );

		if ( !g_Game.IsServer() || !COT_IsKillTrackable() )
			return;

		PlayerBase pb = PlayerBase.Cast( player );
		if ( pb )
			m_COT_KillOwner = pb;
	}

	//! Placed mine/claymore: tag whoever finished the placement.
	override void OnPlacementComplete( Man player, vector position = "0 0 0", vector orientation = "0 0 0" )
	{
		super.OnPlacementComplete( player, position, orientation );

		if ( !g_Game.IsServer() || !COT_IsKillTrackable() )
			return;

		PlayerBase pbp = PlayerBase.Cast( player );
		if ( pbp )
			m_COT_KillOwner = pbp;
	}

	override void EEDelete(EntityAI parent)
	{
		if (g_Game && g_Game.IsServer())
			JMEntityTracker.Unregister(this);

		super.EEDelete(parent);
	}

	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;

		PlayerBase sourcePlayer;
		if (source && Class.CastTo(sourcePlayer, source.GetHierarchyRootPlayer()) && sourcePlayer.COTGetReceiveDamageDealt() && !GetHierarchyRootPlayer())
		{
			sourcePlayer.ProcessDirectDamage(damageType, source, "Torso", ammo, "0 0 0", speedCoef);
			return false;
		}

		Transport transport;
		if (Class.CastTo(transport, GetHierarchyRoot()))
		{
			Human driver = transport.CrewMember(DayZPlayerConstants.VEHICLESEAT_DRIVER);
			if (driver && !driver.GetAllowDamage())
				return false;
		}

		return true;
	}

	override void OnEnergyConsumed()
	{
		PlayerBase player =  PlayerBase.Cast(GetHierarchyRootPlayer());
		if (player && player.COTHasGodMode())  //! UNLIMITED POWER
		{
			GetCompEM().SetEnergy0To1(1.0);
			return;
		}

		super.OnEnergyConsumed();
	}

	//! Prevent calling vanilla EntityAI::OnDebugSpawn
	//! Logic will use COT to spawn attachments, so it'll work like vanilla EXCEPT not have possibility of crashes due to bad items
	override void OnDebugSpawn()
	{
		if (!IsInherited(TentBase))
			GetCommunityOnlineToolsBase().SpawnCompatibleAttachments(this, null, 0);
	}

	void COT_OnDebugSpawn(PlayerBase player)
	{
		OnDebugSpawnEx(DebugSpawnParams.WithPlayer(player));
	}
}

