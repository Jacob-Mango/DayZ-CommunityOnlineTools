// =============================================================================
//  JMEventsAdapter
//
//  Aggregates dynamic + static world events into a single admin list:
//    - Helicopter crashes      (CrashBase, Expansion-tracked when available)
//    - Contaminated zones      (ExpansionContaminatedArea)
//    - Airdrops                (live ExpansionAirdropContainerBase, modded)
//    - Static loot containers  (Land_ContainerLocked_*_DE, Sakhal map)
//    - Static map characters   (StaticObj_FrozenScientist_DE, Sakhal map)
//
//  Static objects are gathered with a one-shot world scan and cached. Dynamic
//  events use static tracking arrays populated by modded class hooks.
// =============================================================================

enum JMEventsCategory
{
	HELI_CRASH       = 1,
	CONTAMINATED     = 2,
	AIRDROP          = 3,
	STATIC_CONTAINER = 4,
	STATIC_OBJECT    = 5,
}

class JMEventsAdapter: JMEntityManagerAdapter
{
	// One-shot cache for static map objects. Filled on first CollectEntities().
	protected ref array<ref JMEntityMetaData> m_StaticCache;
	protected bool m_StaticCacheBuilt;

	void JMEventsAdapter()
	{
		m_StaticCache = new array<ref JMEntityMetaData>;
	}

	override string GetPermissionPrefix()  { return "Events"; }
	override string GetTitle()              { return "#STR_COT_EVENTS_MODULE_NAME"; }
	override string GetIcon()               { return JMConstants.Lucide( "siren" ); }
	override string GetCategory()           { return "Events"; }
	override string GetWebhookTitle()       { return "Events Module"; }

	override void GetWebhookTypes( out array<string> types )
	{
		types.Insert( "Teleport" );
		types.Insert( "Delete"   );
		types.Insert( "Spawn"    );
	}

	override void GetFilterLabels( out array<string> labels )
	{
		labels.Insert( "Heli Crashes" );
		labels.Insert( "Contaminated" );
		labels.Insert( "Airdrops" );
		labels.Insert( "Containers" );
		labels.Insert( "Static Objects" );
	}

	override bool MatchesFilter( JMEntityMetaData entity, int filterIndex )
	{
		if ( filterIndex == 0 ) return true;
		if ( filterIndex == 1 ) return entity.m_Category == JMEventsCategory.HELI_CRASH;
		if ( filterIndex == 2 ) return entity.m_Category == JMEventsCategory.CONTAMINATED;
		if ( filterIndex == 3 ) return entity.m_Category == JMEventsCategory.AIRDROP;
		if ( filterIndex == 4 ) return entity.m_Category == JMEventsCategory.STATIC_CONTAINER;
		if ( filterIndex == 5 ) return entity.m_Category == JMEventsCategory.STATIC_OBJECT;
		return true;
	}

	override void GetActions( out array<ref JMEntityAction> outActions )
	{
		outActions.Insert( new JMEntityAction( "teleport_to", "Teleport To", "Events.Teleport", false, false, "Teleport yourself to this event's location" ) );
		outActions.Insert( new JMEntityAction( "delete",      "Delete",       "Events.Delete",   true,  false, "Remove this event from the world" ) );

		outActions.Insert( new JMEntityAction( "delete_all_crashes",      "Delete Heli Crashes", "Events.Delete", true,  true, "Delete every active helicopter crash site" ) );
		outActions.Insert( new JMEntityAction( "delete_all_contaminated", "Delete Contaminated", "Events.Delete", true,  true, "Delete every active contaminated zone" ) );
		outActions.Insert( new JMEntityAction( "rescan_static",           "Rescan Static",        "Events.View",  false, true, "Re-scan the map for static containers and objects" ) );
		outActions.Insert( new JMEntityAction( "spawn_airdrop",           "Spawn Airdrop",        "Events.Spawn", false, true, "Drop a new airdrop container at your position" ) );
	}

	override void CollectEntities( out array<ref JMEntityMetaData> outEntities )
	{
		if ( !GetGame().IsServer() )
			return;

		CollectHeliCrashes( outEntities );
		CollectContaminatedAreas( outEntities );
		CollectAirdrops( outEntities );

		if ( !m_StaticCacheBuilt )
			BuildStaticCache();

		foreach ( JMEntityMetaData s: m_StaticCache )
			outEntities.Insert( s );
	}

	// ---------------------------------------------------------------------------
	//  Dynamic event collectors
	// ---------------------------------------------------------------------------

	protected void CollectHeliCrashes( out array<ref JMEntityMetaData> outEntities )
	{
#ifdef EXPANSIONMODAI
		CF_DoublyLinkedNode_WeakRef<CrashBase> node = CrashBase.s_Expansion_HeliCrashes.m_Head;
		while ( node )
		{
			CrashBase crash = node.m_Value;
			if ( crash )
				outEntities.Insert( BuildEntityMeta( crash, JMEventsCategory.HELI_CRASH, "Heli Crash", JMConstants.ICON_HELICOPTER, ARGB( 255, 243, 18, 156 ) ) );
			node = node.m_Next;
		}
#else
		// Fallback: use the self-registration list from CrashBase_COT.c.
		CF_DoublyLinkedNode_WeakRef<CrashBase> node = CrashBase.s_JM_COT_AllCrashes.m_Head;
		while ( node )
		{
			CrashBase crash = node.m_Value;
			if ( crash )
				outEntities.Insert( BuildEntityMeta( crash, JMEventsCategory.HELI_CRASH, "Heli Crash", JMConstants.ICON_HELICOPTER, ARGB( 255, 243, 18, 156 ) ) );
			node = node.m_Next;
		}
#endif
	}

	protected void CollectContaminatedAreas( out array<ref JMEntityMetaData> outEntities )
	{
#ifdef EXPANSIONMODMISSIONS
		// Use our server-side mirror (modded ExpansionContaminatedArea_COT.c).
		// Vanilla s_ContaminatedAreas is client-only and would always be empty here.
		for ( int i = 0; i < ExpansionContaminatedArea.s_JM_COT_AllAreas.Count(); i++ )
		{
			ExpansionContaminatedArea area = ExpansionContaminatedArea.s_JM_COT_AllAreas[i];
			if ( !area )
				continue;

			JMEntityMetaData m = BuildEntityMeta( area, JMEventsCategory.CONTAMINATED, "Contaminated Zone", JMConstants.ICON_CANCEL, ARGB( 255, 80, 180, 40 ) );
			m.SetExtra( "Radius", area.m_Radius.ToString() + "m" );
			outEntities.Insert( m );
		}
#else
		// Vanilla EffectArea tracking (modded EffectArea_COT.c).
		for ( int vi = 0; vi < EffectArea.s_JM_COT_AllAreas.Count(); vi++ )
		{
			EffectArea area = EffectArea.s_JM_COT_AllAreas[vi];
			if ( !area )
				continue;

			// Skip areas whose InitZone hasn't fired yet - position is unreliable.
			ContaminatedArea_DynamicBase dynArea;
			if ( Class.CastTo( dynArea, area ) && dynArea.COT_GetDecayState() < eAreaDecayStage.LIVE )
				continue;

			string label = area.m_Name;
			if ( label == "" || label == "Default setup" )
				label = "Contaminated Zone";

			JMEntityMetaData m = BuildEntityMeta( area, JMEventsCategory.CONTAMINATED, label, JMConstants.ICON_CANCEL, ARGB( 255, 80, 180, 40 ) );
			m.SetExtra( "Radius", area.m_Radius.ToString() + "m" );
			string zoneType;
			if ( area.m_Type == eZoneType.DYNAMIC )
				zoneType = "Dynamic";
			else
				zoneType = "Static";
			m.SetExtra( "Type", zoneType );
			outEntities.Insert( m );
		}
#endif
	}

	protected void CollectAirdrops( out array<ref JMEntityMetaData> outEntities )
	{
#ifdef EXPANSIONMODMISSIONS
		if ( !ExpansionAirdropContainerBase.s_JM_COT_Active )
			return;

		for ( int i = 0; i < ExpansionAirdropContainerBase.s_JM_COT_Active.Count(); i++ )
		{
			ExpansionAirdropContainerBase drop = ExpansionAirdropContainerBase.s_JM_COT_Active[i];
			if ( !drop )
				continue;
			outEntities.Insert( BuildEntityMeta( drop, JMEventsCategory.AIRDROP, "Airdrop", JMConstants.ICON_AIRPLANE, ARGB( 255, 243, 220, 40 ) ) );
		}
#endif
	}

	// ---------------------------------------------------------------------------
	//  Static map object cache - one-shot world scan, runs only on "Rescan" or
	//  first open. All target classnames are checked in a single pass over the
	//  result set. On non-Sakhal maps every IsKindOf check fails immediately,
	//  so the loop body is never entered.
	// ---------------------------------------------------------------------------

	protected void BuildStaticCache()
	{
		m_StaticCache.Clear();

		// Single sweep from map centre with a radius that covers all DayZ maps.
		// On non-Sakhal maps every IsKindOf check fails, so nothing is inserted.
		vector centre = "10000 0 10000";
		array<Object>   found = new array<Object>;
		array<CargoBase> dummy = new array<CargoBase>;
		GetGame().GetObjectsAtPosition3D( centre, 20000, found, dummy );

		foreach ( Object obj: found )
		{
			if ( !obj )
				continue;

			if ( obj.IsKindOf( "Land_ContainerLocked_Blue_DE" ) || obj.IsKindOf( "Land_ContainerLocked_Yellow_DE" ) || obj.IsKindOf( "Land_ContainerLocked_Orange_DE" ) || obj.IsKindOf( "Land_ContainerLocked_Red_DE" ) )
			{
				m_StaticCache.Insert( BuildEntityMeta( obj, JMEventsCategory.STATIC_CONTAINER, "Locked Container", JMConstants.ICON_CAMPING_TENT, ARGB( 255, 200, 130, 40 ) ) );
				continue;
			}

			if ( obj.IsKindOf( "StaticObj_FrozenScientist_DE" ) )
				m_StaticCache.Insert( BuildEntityMeta( obj, JMEventsCategory.STATIC_OBJECT, "Frozen Scientist", JMConstants.ICON_CANCEL, ARGB( 255, 100, 200, 220 ) ) );
		}

		m_StaticCacheBuilt = true;
	}

	// ---------------------------------------------------------------------------
	//  Common metadata builder
	// ---------------------------------------------------------------------------

	protected JMEntityMetaData BuildEntityMeta( Object obj, int category, string displayName, string icon, int color )
	{
		JMEntityMetaData m = new JMEntityMetaData();

		int low, high;
		obj.GetNetworkID( low, high );
		m.m_NetworkIDLow  = low;
		m.m_NetworkIDHigh = high;
		m.m_Id            = "evt_" + low.ToString() + "_" + high.ToString();
		m.m_Category      = category;

		m.m_DisplayName   = displayName;
		m.m_ClassName     = obj.GetType();
		m.m_StatusText    = "Active";
		m.m_Position      = obj.GetPosition();
		m.m_Orientation   = obj.GetOrientation();

		m.m_MarkerColor   = color;
		m.m_MarkerIcon    = icon;

		m.SetExtra( "Position", obj.GetPosition().ToString() );

		return m;
	}

	// ---------------------------------------------------------------------------
	//  Action handlers
	// ---------------------------------------------------------------------------

	override void HandleServerAction( string actionId, JMEntityMetaData entity, PlayerIdentity sender )
	{
		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( sender.GetId() );

		if ( actionId == "teleport_to" && entity )
		{
			HandleTeleportTo( entity, sender );
			LogAction( sender, "Teleported to event at " + entity.m_Position.ToString() );
			SendAdapterWebhookColored( "Teleport", inst, "Teleported to event (" + entity.m_DisplayName + ") at " + entity.m_Position.ToString(), JMConstants.WEBHOOK_COLOR_TELEPORT );
		}
		else if ( actionId == "delete" && entity )
		{
			LogAction( sender, "Deleted event: " + entity.m_DisplayName );
			SendAdapterWebhookColored( "Delete", inst, "Deleted event: " + entity.m_DisplayName, JMConstants.WEBHOOK_COLOR_DANGER );
			HandleDelete( entity );
			ReportRemove();
		}
		else if ( actionId == "delete_all_crashes" || actionId == "delete_all_contaminated" || actionId == "rescan_static" || actionId == "spawn_airdrop" )
		{
			if ( actionId == "delete_all_crashes" )
			{
				LogAction( sender, "Deleted all helicopter crashes" );
				SendAdapterWebhookColored( "Delete", inst, "Deleted all helicopter crashes", JMConstants.WEBHOOK_COLOR_CRITICAL );
				HandleDeleteAll( JMEventsCategory.HELI_CRASH );
			}
			else if ( actionId == "delete_all_contaminated" )
			{
				LogAction( sender, "Deleted all contaminated zones" );
				SendAdapterWebhookColored( "Delete", inst, "Deleted all contaminated zones", JMConstants.WEBHOOK_COLOR_CRITICAL );
				HandleDeleteAll( JMEventsCategory.CONTAMINATED );
			}
			else if ( actionId == "rescan_static" )
			{
				m_StaticCacheBuilt = false;
				InvalidateCache();
			}
			else
			{
				LogAction( sender, "Spawned airdrop" );
				SendAdapterWebhookColored( "Spawn", inst, "Spawned airdrop", JMConstants.WEBHOOK_COLOR_SPAWN );
				HandleSpawnAirdrop( sender );
			}
		}
	}

	protected void HandleTeleportTo( JMEntityMetaData entity, PlayerIdentity sender )
	{
		PlayerBase player = GetPlayerObjectByIdentity( sender );
		if ( !player )
			return;
		player.SetWorldPosition( entity.m_Position );
	}

	protected void HandleDelete( JMEntityMetaData entity )
	{
		Object obj = GetGame().GetObjectByNetworkId( entity.m_NetworkIDLow, entity.m_NetworkIDHigh );
		if ( obj )
		{
			GetGame().ObjectDelete( obj );
			// If this was a cached static, invalidate the static cache too.
			if ( entity.m_Category == JMEventsCategory.STATIC_CONTAINER || entity.m_Category == JMEventsCategory.STATIC_OBJECT )
				m_StaticCacheBuilt = false;
			// (Entity manager base also calls InvalidateCache() after every action.)
		}
	}

	protected void HandleDeleteAll( int category )
	{
		if ( category == JMEventsCategory.HELI_CRASH )
		{
#ifdef EXPANSIONMODAI
			CF_DoublyLinkedNode_WeakRef<CrashBase> node = CrashBase.s_Expansion_HeliCrashes.m_Head;
			array<Object> toDelete = new array<Object>;
			while ( node )
			{
				if ( node.m_Value )
					toDelete.Insert( node.m_Value );
				node = node.m_Next;
			}
			foreach ( Object o: toDelete )
				GetGame().ObjectDelete( o );
#endif
		}
		else if ( category == JMEventsCategory.CONTAMINATED )
		{
#ifdef EXPANSIONMODMISSIONS
			array<Object> toDelete2 = new array<Object>;
			for ( int i = 0; i < ExpansionContaminatedArea.s_JM_COT_AllAreas.Count(); i++ )
			{
				if ( ExpansionContaminatedArea.s_JM_COT_AllAreas[i] )
					toDelete2.Insert( ExpansionContaminatedArea.s_JM_COT_AllAreas[i] );
			}
			foreach ( Object o2: toDelete2 )
				GetGame().ObjectDelete( o2 );
#else
			array<Object> toDeleteV = new array<Object>;
			for ( int vi = 0; vi < EffectArea.s_JM_COT_AllAreas.Count(); vi++ )
			{
				if ( EffectArea.s_JM_COT_AllAreas[vi] )
					toDeleteV.Insert( EffectArea.s_JM_COT_AllAreas[vi] );
			}
			foreach ( Object ov: toDeleteV )
				GetGame().ObjectDelete( ov );
#endif
		}
	}

	protected void HandleSpawnAirdrop( PlayerIdentity sender )
	{
#ifdef EXPANSIONMODMISSIONS
		PlayerBase player = GetPlayerObjectByIdentity( sender );
		if ( !player )
			return;
		vector pos = player.GetWorldPosition();
		pos[1] = pos[1] + 400;
		GetGame().CreateObjectEx( "ExpansionAirdropContainer_Basic", pos, ECE_PLACE_ON_SURFACE );
#endif
	}
}
