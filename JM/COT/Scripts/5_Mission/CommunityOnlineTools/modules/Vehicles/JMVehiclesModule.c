const int JMVT_NONE = 0;
const int JMVT_CAR = 2;
const int JMVT_BOAT = 4;
const int JMVT_HELICOPTER = 8;
const int JMVT_PLANE = 32;
const int JMVT_BIKE = 512;
//! Every type the module knows how to classify. A vehicle carrying none of
//! these bits is an unclassified mod vehicle, which the UI filters separately.
//!
//! WRITTEN OUT AS A LITERAL ON PURPOSE. This was
//!     JMVT_CAR | JMVT_BOAT | JMVT_HELICOPTER | JMVT_PLANE | JMVT_BIKE
//! which Enforce does not fold - a global const initialised from other global
//! consts comes out 0. The failure is quiet and reads as something else
//! entirely: every row of the type filter draws greyed, while the map still
//! shows everything, because PassesTypeFilter's "(type & JMVT_ALL) == 0" test
//! then matches EVERY vehicle and hands them all to the Unclassified toggle.
//!
//! 2 | 4 | 8 | 32 | 512 = 558. Keep this in step with the bits above.
const int JMVT_ALL = 558;

const int JMDT_NONE = 0;
const int JMDT_EXPLODED = 2;
const int JMDT_DESTROYED = 4;

class JMVehicleMetaData
{
	int m_NetworkIDLow;
	int m_NetworkIDHigh;

	int m_PersistentIDA;
	int m_PersistentIDB;
	int m_PersistentIDC;
	int m_PersistentIDD;

	string m_ClassName;
	vector m_Position;
	vector m_Orientation;

	int m_VehicleType;
	int m_DestructionType;

	bool m_HasKeys;
	bool m_IsCover;

	//! Live state, rebuilt every time the metadata is built. The map's hover
	//! panel reads these, so they are serialised unconditionally - unlike the
	//! key/owner block above, which only exists when Expansion is loaded.
	float  m_HealthPct;   //!< 0..1 global health
	float  m_FuelPct;     //!< 0..1, or -1 when the vehicle carries no fuel tank
	float  m_CoolantPct;  //!< 0..1, or -1 when the vehicle carries no coolant tank
	float  m_SpeedKph;
	int    m_CrewCount;
	string m_CrewNames;   //!< comma-joined occupant names, driver seat first
	bool   m_IsLocked;

	string m_OwnerName;
	string m_OwnerUID;
	string m_LastDriverUID;
	string m_LastDriverSteam;
	string m_LastDriverGUID;

	[NonSerialized()]
	string m_DisplayName;

	[NonSerialized()]
	EntityAI m_Entity;
	
	static JMVehicleMetaData Create( EntityAI entity, string type = string.Empty )
	{
		JMVehicleMetaData meta = new JMVehicleMetaData();
		meta.AcquireFrom( entity, type );
		return meta;
	}

	void AcquireFrom( EntityAI entity, string type = string.Empty )
	{
		m_Entity = entity;

		entity.GetNetworkID( m_NetworkIDLow, m_NetworkIDHigh );
		entity.GetPersistentID( m_PersistentIDA, m_PersistentIDB, m_PersistentIDC, m_PersistentIDD );

		if ( type == string.Empty )
			type = entity.GetType();

		m_ClassName = type;
		m_Position = entity.GetPosition();
		m_Orientation = entity.GetOrientation();

		if ( entity.IsDamageDestroyed() )
			m_DestructionType |= JMDT_DESTROYED;

		AcquireLiveState( entity );

	#ifdef EXPANSIONMODVEHICLE
		ExpansionVehicle vehicle;
		if ( ExpansionVehicle.Get( vehicle, entity ) )
		{
			if ( vehicle.IsCar() )
				m_VehicleType |= JMVT_CAR;

			if ( vehicle.IsBoat() )
				m_VehicleType |= JMVT_BOAT;

			if ( vehicle.IsBike() )
				m_VehicleType |= JMVT_BIKE;

			if ( vehicle.IsHelicopter() )
				m_VehicleType |= JMVT_HELICOPTER;

			if ( vehicle.IsPlane() )
				m_VehicleType |= JMVT_PLANE;

			if ( vehicle.IsExploded() )
				m_DestructionType |= JMDT_EXPLODED;

			m_HasKeys = vehicle.HasKey();
			m_IsLocked = vehicle.IsLocked();

			m_OwnerName = vehicle.GetOwnerName();
			m_OwnerUID = vehicle.GetOwnerUID();
			m_LastDriverUID = vehicle.GetLastDriverUID();
		}
	#else
		if ( entity.IsInherited( CarScript ) )
		{
			m_VehicleType = JMVT_CAR;
			CarScript car = CarScript.Cast( entity );
			if ( car && car.m_JM_LastDriverUID != "" )
				m_LastDriverUID = car.m_JM_LastDriverUID;
		}
		else if ( entity.IsInherited( BoatScript ) )
		{
			m_VehicleType = JMVT_BOAT;
			BoatScript boat = BoatScript.Cast( entity );
			if ( boat && boat.m_JM_LastDriverUID != "" )
				m_LastDriverUID = boat.m_JM_LastDriverUID;
		}
	#ifndef DAYZ_1_29
		else if ( entity.IsInherited( Motorbike ) )
		{
			m_VehicleType = JMVT_BIKE;
			MotorbikeScript bike = MotorbikeScript.Cast( entity );
			if ( bike && bike.m_JM_LastDriverUID != "" )
				m_LastDriverUID = bike.m_JM_LastDriverUID;
		}
	#endif
	#endif
	}

	//! Health / fuel / speed / crew, read straight off the entity. Server side
	//! only - the client never has the vehicle object for a marker outside its
	//! network bubble, which is exactly why these travel as metadata.
	//!
	//! Every read is behind a cast rather than a type check: a vehicle cover
	//! and an unclassified mod vehicle both land here, and neither is a
	//! Transport. Enforce has no block scope, so each cast gets its own name.
	void AcquireLiveState( EntityAI entity )
	{
		m_HealthPct = entity.GetHealth01( "", "" );

		//! -1, not 0: "no tank" and "empty tank" are different readouts.
		m_FuelPct = -1;
		m_CoolantPct = -1;

		CarScript liveCar;
		if ( Class.CastTo( liveCar, entity ) )
		{
			m_FuelPct = liveCar.GetFluidFraction( CarFluid.FUEL );
			m_CoolantPct = liveCar.GetFluidFraction( CarFluid.COOLANT );
		}

		BoatScript liveBoat;
		if ( Class.CastTo( liveBoat, entity ) )
			m_FuelPct = liveBoat.GetFluidFraction( BoatFluid.FUEL );

	#ifndef DAYZ_1_29
		//! No coolant read here - Motorbike has no coolant tank (see
		//! MotorbikeScript.c's COT_Refuel note), so m_CoolantPct stays -1.
		MotorbikeScript liveBike;
		if ( Class.CastTo( liveBike, entity ) )
			m_FuelPct = liveBike.GetFluidFraction( MotorbikeFluid.FUEL );
	#endif

		//! GetSpeedometer only exists on Car, and there is no equivalent on
		//! Boat / Helicopter / a modded base. The rigid body's velocity is the
		//! one figure every vehicle has. m/s -> km/h.
		m_SpeedKph = GetVelocity( entity ).Length() * 3.6;

		m_CrewCount = 0;
		m_CrewNames = "";

		Transport liveTransport;
		if ( Class.CastTo( liveTransport, entity ) )
			AcquireCrew( liveTransport );
	}

	//! Occupants in seat order, so the driver reads first.
	protected void AcquireCrew( Transport transport )
	{
		int seats = transport.CrewSize();

		for ( int i = 0; i < seats; i++ )
		{
			Human crewHuman = transport.CrewMember( i );
			if ( !crewHuman )
				continue;

			m_CrewCount++;

			//! An AI or an infected in a seat has no identity - name it by what
			//! it is rather than dropping it from a count that already includes it.
			string crewName = "Unknown";

			PlayerBase crewPlayer;
			if ( Class.CastTo( crewPlayer, crewHuman ) && crewPlayer.GetIdentity() )
				crewName = crewPlayer.GetIdentity().GetName();

			if ( m_CrewNames != "" )
				m_CrewNames += ", ";

			m_CrewNames += crewName;
		}
	}

	#ifdef EXPANSIONMODVEHICLE
	static JMVehicleMetaData CreateCover( ExpansionVehicleCover cover )
	{
		string type = cover.Expansion_GetStoredEntityType();

		JMVehicleMetaData meta = Create( cover, type );

		if ( g_Game.IsKindOf( type, "ExpansionHelicopterScript" ) )
			meta.m_VehicleType |= JMVT_HELICOPTER;
		else if ( g_Game.IsKindOf( type, "ExpansionBoatScript" ) || g_Game.IsKindOf( type, "BoatScript" ) )
			meta.m_VehicleType |= JMVT_BOAT;
		else if ( g_Game.IsKindOf( type, "Motorbike" ) )
			meta.m_VehicleType |= JMVT_BIKE;
		else
			meta.m_VehicleType |= JMVT_CAR;

		ExpansionKeyChainBase keychain = ExpansionKeyChainBase.Cast( cover.GetAttachmentByType( ExpansionKeyChainBase ) );
		if ( keychain && keychain.Expansion_HasOwner() )
		{
			meta.m_HasKeys = true;

			meta.m_OwnerName = keychain.Expansion_GetOwnerName();
			meta.m_OwnerUID = keychain.Expansion_GetOwnerUID();
		}

		meta.m_IsCover = true;

		return meta;
	}
	#endif

	string GetVehicleType()
	{
		string type = "";

		if ( m_VehicleType & JMVT_CAR )
			type += "Car ";
		if ( m_VehicleType & JMVT_BOAT )
			type += "Boat ";
		if ( m_VehicleType & JMVT_HELICOPTER )
			type += "Helicopter ";
		if ( m_VehicleType & JMVT_PLANE )
			type += "Plane ";
		if ( m_VehicleType & JMVT_BIKE )
			type += "Motorbike ";

		return type;
	}

	string GetVehicleDestructionState()
	{
		string type = "";

		if ( m_DestructionType != JMDT_NONE )
		{
			if ( m_DestructionType & JMDT_DESTROYED )
			{
				type += "Destroyed ";
			}
			if ( m_DestructionType & JMDT_EXPLODED )
			{
				type += "Exploded ";
			}
		} else
		{
			type = "None";
		}

		return type;
	}

	bool IsDestroyed()
	{
		return m_DestructionType & JMDT_DESTROYED;
	}

	bool IsExploded()
	{
		return m_DestructionType & JMDT_EXPLODED;
	}

	void SetDisplayName()
	{
		if ( g_Game.ConfigIsExisting( "cfgVehicles " + m_ClassName + " displayName" ) )
			g_Game.ConfigGetText( "cfgVehicles " + m_ClassName + " displayName", m_DisplayName );
		else
			m_DisplayName = m_ClassName;
	}

	void Write( ParamsWriteContext ctx )
	{
		ctx.Write( m_NetworkIDLow );
		ctx.Write( m_NetworkIDHigh );

		ctx.Write( m_PersistentIDA );
		ctx.Write( m_PersistentIDB );
		ctx.Write( m_PersistentIDC );
		ctx.Write( m_PersistentIDD );

		ctx.Write( m_ClassName );
		ctx.Write( m_Position );
		ctx.Write( m_Orientation );

		ctx.Write( m_VehicleType );
		ctx.Write( m_DestructionType );

	#ifdef EXPANSIONMODVEHICLE
		ctx.Write( m_HasKeys );
		ctx.Write( m_IsCover );

		ctx.Write( m_OwnerName );
		ctx.Write( m_OwnerUID );
	#endif

	#ifdef EXPANSIONMODCORE
		ctx.Write( m_LastDriverUID );
		ctx.Write( m_LastDriverSteam );
		ctx.Write( m_LastDriverGUID );
	#endif

		//! Live state travels for every build, Expansion or not - the hover
		//! panel is not an Expansion feature. Appended last so the ordering of
		//! everything above is untouched.
		ctx.Write( m_HealthPct );
		ctx.Write( m_FuelPct );
		ctx.Write( m_CoolantPct );
		ctx.Write( m_SpeedKph );
		ctx.Write( m_CrewCount );
		ctx.Write( m_CrewNames );
		ctx.Write( m_IsLocked );
	}

	bool Read( ParamsReadContext ctx )
	{
		if ( !ctx.Read( m_NetworkIDLow ) ) return false;
		if ( !ctx.Read( m_NetworkIDHigh ) ) return false;

		if ( !ctx.Read( m_PersistentIDA ) ) return false;
		if ( !ctx.Read( m_PersistentIDB ) ) return false;
		if ( !ctx.Read( m_PersistentIDC ) ) return false;
		if ( !ctx.Read( m_PersistentIDD ) ) return false;

		if ( !ctx.Read( m_ClassName ) ) return false;
		if ( !ctx.Read( m_Position ) ) return false;
		if ( !ctx.Read( m_Orientation ) ) return false;

		if ( !ctx.Read( m_VehicleType ) ) return false;
		if ( !ctx.Read( m_DestructionType ) ) return false;

	#ifdef EXPANSIONMODVEHICLE
		if ( !ctx.Read( m_HasKeys ) ) return false;
		if ( !ctx.Read( m_IsCover ) ) return false;

		if ( !ctx.Read( m_OwnerName ) ) return false;
		if ( !ctx.Read( m_OwnerUID ) ) return false;
	#endif

	#ifdef EXPANSIONMODCORE
		if ( !ctx.Read( m_LastDriverUID ) ) return false;
		if ( !ctx.Read( m_LastDriverSteam ) ) return false;
		if ( !ctx.Read( m_LastDriverGUID ) ) return false;
	#endif

		if ( !ctx.Read( m_HealthPct ) ) return false;
		if ( !ctx.Read( m_FuelPct ) ) return false;
		if ( !ctx.Read( m_CoolantPct ) ) return false;
		if ( !ctx.Read( m_SpeedKph ) ) return false;
		if ( !ctx.Read( m_CrewCount ) ) return false;
		if ( !ctx.Read( m_CrewNames ) ) return false;
		if ( !ctx.Read( m_IsLocked ) ) return false;

		return true;
	}
}

class JMVehiclesModule: JMRenderableModuleBase
{
	private ref array<ref JMVehicleMetaData> m_Vehicles;

	void JMVehiclesModule()
	{
		JMPermissions.Register( JMConstants.PERM_VEHICLES_VIEW );

		JMPermissions.Register( JMConstants.PERM_VEHICLES_DELETE );
		JMPermissions.Register( JMConstants.PERM_VEHICLES_DELETE_ALL );
		JMPermissions.Register( JMConstants.PERM_VEHICLES_DELETE_DESTROYED );
		#ifdef EXPANSIONMODVEHICLE
		JMPermissions.Register( JMConstants.PERM_VEHICLES_DELETE_UNCLAIMED );
		#endif
		JMPermissions.Register( JMConstants.PERM_VEHICLES_TELEPORT );
		JMPermissions.Register( JMConstants.PERM_VEHICLES_REPAIR );
		JMPermissions.Register( JMConstants.PERM_VEHICLES_REFUEL );
		JMPermissions.Register( JMConstants.PERM_VEHICLES_UNSTUCK );
		#ifdef EXPANSIONMODVEHICLE
		JMPermissions.Register( JMConstants.PERM_VEHICLES_COVER );
		JMPermissions.Register( JMConstants.PERM_VEHICLES_LOCK );
		JMPermissions.Register( JMConstants.PERM_VEHICLES_UNPAIR );
		JMPermissions.Register( JMConstants.PERM_VEHICLES_CLEARCARGO );
		JMPermissions.Register( JMConstants.PERM_VEHICLES_SPAWNKEY );
		#endif

		m_Vehicles = new array<ref JMVehicleMetaData>;
	}

	override bool HasAccess()
	{
		return JMPermissions.Has( JMConstants.PERM_VEHICLES_VIEW );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/vehicles/Vehicles_Menu.layout";
	}

	override string GetInputToggle()
	{
		return "UACOTToggleVehicle";
	}

	override string GetCategory()
	{
		return "Vehicles";
	}

	override string GetTitle()
	{
		return "#STR_COT_VEHICLE_MANAGEMENT_MODULE_NAME";
	}

	override string GetIconName()
	{
		return JMConstants.Lucide( "car" );
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
	}

	override string GetWebhookTitle()
	{
		return "Vehicles Module";
	}

	override void GetWebhookTypes( out array<string> types )
	{
		types.Insert( "Delete"   );
		types.Insert( "Teleport" );
		types.Insert( "Repair"   );
		types.Insert( "Refuel"   );
		types.Insert( "Unstuck"  );
		types.Insert( "Cover"    );
		types.Insert( "Lock"     );
		types.Insert( "Unpair"   );
	}

	override void RegisterKeyMouseBindings()
	{
		super.RegisterKeyMouseBindings();
		Bind( new JMModuleBinding( "Input_TeleportToSelected", "UAVehicleModuleTeleportTo", true ) );
		Bind( new JMModuleBinding( "Input_RepairSelected",     "UAVehicleModuleRepair",     true ) );
	}

	void Input_TeleportToSelected()
	{
		if ( !JMPermissions.Has( JMConstants.PERM_VEHICLES_TELEPORT ) ) return;
		JMVehiclesForm form;
		if ( !Class.CastTo( form, GetForm() ) ) return;
		JMVehicleMetaData meta = form.GetCurrentVehicle();
		if ( meta ) RequestTeleportToVehicle( meta );
	}

	void Input_RepairSelected()
	{
		if ( !JMPermissions.Has( JMConstants.PERM_VEHICLES_REPAIR ) ) return;
		JMVehiclesForm form;
		if ( !Class.CastTo( form, GetForm() ) ) return;
		JMVehicleMetaData meta = form.GetCurrentVehicle();
		if ( meta ) RequestRepairVehicle( meta );
	}

	private void UpdateVehiclesMetaData()
	{
		if ( m_Vehicles.Count() > 0 )
			m_Vehicles.Clear();

		CF_DoublyLinkedNode_WeakRef<CarScript> node = CarScript.s_JM_AllCars.m_Head;
		while ( node )
		{
			if ( !node.m_Value.IsSetForDeletion() )
				m_Vehicles.Insert( JMVehicleMetaData.Create( node.m_Value ) );
			node = node.m_Next;
		}

		CF_DoublyLinkedNode_WeakRef<BoatScript> boat = BoatScript.s_JM_AllBoats.m_Head;
		while ( boat )
		{
			if ( !boat.m_Value.IsSetForDeletion() )
				m_Vehicles.Insert( JMVehicleMetaData.Create( boat.m_Value ) );
			boat = boat.m_Next;
		}

	#ifndef DAYZ_1_29
		CF_DoublyLinkedNode_WeakRef<MotorbikeScript> bike = MotorbikeScript.s_JM_AllBikes.m_Head;
		while ( bike )
		{
			if ( !bike.m_Value.IsSetForDeletion() )
				m_Vehicles.Insert( JMVehicleMetaData.Create( bike.m_Value ) );
			bike = bike.m_Next;
		}
	#endif

		#ifdef EXPANSIONMODVEHICLE
		set<ExpansionVehicleBase> vehicles = ExpansionVehicleBase.GetAll();
		foreach ( ExpansionVehicleBase vehicle: vehicles )
		{
			if ( !vehicle )
				continue;

			if ( !vehicle.IsSetForDeletion() )
				m_Vehicles.Insert( JMVehicleMetaData.Create( vehicle ) );
		}

		CF_DoublyLinkedNode_WeakRef<ExpansionVehicleCover> cover = ExpansionVehicleCover.s_JM_AllCovers.m_Head;
		while ( cover )
		{
			if ( !cover.m_Value.IsSetForDeletion() )
				m_Vehicles.Insert( JMVehicleMetaData.CreateCover( cover.m_Value ) );
			cover = cover.m_Next;
		}
		#endif

		// Resolve last driver Steam/GUID from online players where possible.
		// A single session-scoped cache keeps the cost at O(unique_drivers)
		// instead of O(vehicles). On a 200-vehicle server where most drivers
		// repeat (or are null), this is typically ~5-10 map lookups instead
		// of 200.
		map<string, JMPlayerInstance> driverCache = new map<string, JMPlayerInstance>;

		foreach ( JMVehicleMetaData meta: m_Vehicles )
		{
			if ( meta.m_LastDriverUID == "" )
				continue;

			JMPlayerInstance driverInst;
			if ( !driverCache.Find( meta.m_LastDriverUID, driverInst ) )
			{
				driverInst = GetPermissionsManager().GetPlayer( meta.m_LastDriverUID );
				driverCache.Set( meta.m_LastDriverUID, driverInst );
			}

			if ( driverInst )
			{
				meta.m_LastDriverSteam = driverInst.GetSteam64ID();
				meta.m_LastDriverGUID  = driverInst.GetGUID();
			}
		}
	}

	void UpdateVehiclesMetaData_SP()
	{
		UpdateVehiclesMetaData();

		foreach ( auto meta: m_Vehicles )
		{
			meta.SetDisplayName();
		}

		JMVehiclesForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.LoadVehicles();
	}

	override int GetRPCMin()
	{
		return JMVehiclesModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMVehiclesModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMVehiclesModuleRPC.RequestServerVehicles:
			RPC_RequestServerVehicles( ctx, sender, target );
			break;	
		case JMVehiclesModuleRPC.SendServerVehicles:
			RPC_SendServerVehicles( ctx, sender, target );
			break;
		case JMVehiclesModuleRPC.DeleteVehicle:
			RPC_DeleteVehicle( ctx, sender, target );
			break;
		#ifdef EXPANSIONMODVEHICLE
		case JMVehiclesModuleRPC.DeleteVehicleUnclaimed:
			RPC_DeleteVehicleUnclaimed( ctx, sender, target );
			break;
		#endif
		case JMVehiclesModuleRPC.DeleteVehicleDestroyed:
			RPC_DeleteVehicleDestroyed( ctx, sender, target );
			break;
		case JMVehiclesModuleRPC.DeleteVehicleAll:
			RPC_DeleteVehicleAll( ctx, sender, target );
			break;
		case JMVehiclesModuleRPC.TeleportToVehicle:
			RPC_TeleportToVehicle( ctx, sender, target );
			break;
		case JMVehiclesModuleRPC.TeleportVehicleToMe:
			RPC_TeleportVehicleToMe( ctx, sender, target );
			break;
		case JMVehiclesModuleRPC.TeleportVehicleTo:
			RPC_TeleportVehicleTo( ctx, sender, target );
			break;
		case JMVehiclesModuleRPC.ClearVehicleCargo:
			RPC_ClearVehicleCargo( ctx, sender, target );
			break;
		case JMVehiclesModuleRPC.SpawnVehicleKey:
			RPC_SpawnVehicleKey( ctx, sender, target );
			break;
		case JMVehiclesModuleRPC.RepairVehicle:
			RPC_RepairVehicle( ctx, sender, target );
			break;
		case JMVehiclesModuleRPC.RefuelVehicle:
			RPC_RefuelVehicle( ctx, sender, target );
			break;
		case JMVehiclesModuleRPC.UnstuckVehicle:
			RPC_UnstuckVehicle( ctx, sender, target );
			break;
		#ifdef EXPANSIONMODVEHICLE
		case JMVehiclesModuleRPC.CoverVehicle:
			RPC_CoverVehicle( ctx, sender, target );
			break;
		case JMVehiclesModuleRPC.LockVehicle:
			RPC_LockVehicle( ctx, sender, target );
			break;
		case JMVehiclesModuleRPC.UnPairVehicle:
			RPC_UnPairVehicle( ctx, sender, target );
			break;
		#endif
		case JMVehiclesModuleRPC.SendVehicleUpsert:
			RPC_SendVehicleUpsert( ctx, sender, target );
			break;
		case JMVehiclesModuleRPC.SendVehicleRemove:
			RPC_SendVehicleRemove( ctx, sender, target );
			break;
		case JMVehiclesModuleRPC.RequestVehicleUpsert:
			RPC_RequestVehicleUpsert( ctx, sender, target );
			break;
		}
	}

	// --- Single-vehicle refresh ----------------------------------------------

	//! Ask the server to re-send ONE vehicle's metadata. The map's hover panel
	//! shows health, fuel, speed and crew, all of which would otherwise be as
	//! stale as the last full roster sync - and a full sync on hover would
	//! resend every vehicle on the server to read one of them.
	void RequestVehicleUpsert( int netLow, int netHigh )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( netLow );
		rpc.Write( netHigh );
		rpc.Send( NULL, JMVehiclesModuleRPC.RequestVehicleUpsert, true );
	}

	private void RPC_RequestVehicleUpsert( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		//! Read-only: the same permission that lets the caller see the roster
		//! at all. Nothing here mutates the vehicle.
		if ( !senderRPC ) return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_VEHICLES_VIEW, senderRPC ) )
			return;

		int netLow;
		if ( !ctx.Read( netLow ) )
			return;

		int netHigh;
		if ( !ctx.Read( netHigh ) )
			return;

		SendVehicleUpsert( netLow, netHigh, senderRPC );
	}

	// --- Delta send helpers (server) -----------------------------------------

	//! Rebuild metadata for ONE vehicle identified by its network id and push
	//! it to the requester as an Upsert. Much cheaper than the old
	//! "re-walk every linked list and resend 200 vehicles" after each action.
	protected void SendVehicleUpsert( int netLow, int netHigh, PlayerIdentity sender )
	{
		if ( !sender )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
		{
			// Object is gone - caller meant to mutate it but it no longer
			// exists. Send a Remove so the client drops its stale row.
			SendVehicleRemove( netLow, netHigh, sender );
			return;
		}

		JMVehicleMetaData meta = BuildMetaForObject( obj );
		if ( !meta )
			return;

		// Resolve last-driver Steam/GUID the same way the full refresh does.
		if ( meta.m_LastDriverUID != "" )
		{
			JMPlayerInstance driverInst = GetPermissionsManager().GetPlayer( meta.m_LastDriverUID );
			if ( driverInst )
			{
				meta.m_LastDriverSteam = driverInst.GetSteam64ID();
				meta.m_LastDriverGUID  = driverInst.GetGUID();
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		meta.Write( rpc );
		rpc.Send( NULL, JMVehiclesModuleRPC.SendVehicleUpsert, true, sender );
	}

	protected void SendVehicleRemove( int netLow, int netHigh, PlayerIdentity sender )
	{
		if ( !sender )
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( netLow );
		rpc.Write( netHigh );
		rpc.Send( NULL, JMVehiclesModuleRPC.SendVehicleRemove, true, sender );
	}

	//! Construct a fresh JMVehicleMetaData for the given world object.
	//! Mirrors the branches in UpdateVehiclesMetaData for all vehicle subtypes.
	protected JMVehicleMetaData BuildMetaForObject( Object obj )
	{
		if ( !obj )
			return null;

		CarScript car = CarScript.Cast( obj );
		if ( car )
			return JMVehicleMetaData.Create( car );

		BoatScript boat = BoatScript.Cast( obj );
		if ( boat )
			return JMVehicleMetaData.Create( boat );

	#ifndef DAYZ_1_29
		MotorbikeScript bike = MotorbikeScript.Cast( obj );
		if ( bike )
			return JMVehicleMetaData.Create( bike );
	#endif

	#ifdef EXPANSIONMODVEHICLE
		ExpansionVehicleBase expVeh = ExpansionVehicleBase.Cast( obj );
		if ( expVeh )
			return JMVehicleMetaData.Create( expVeh );

		ExpansionVehicleCover cover = ExpansionVehicleCover.Cast( obj );
		if ( cover )
			return JMVehicleMetaData.CreateCover( cover );
	#endif

		return null;
	}

	// --- Delta receive handlers (client) -------------------------------------

	protected void RPC_SendVehicleUpsert( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionClient() )
			return;

		JMVehicleMetaData fresh = new JMVehicleMetaData();
		if ( !fresh.Read( ctx ) )
			return;
		fresh.SetDisplayName();

		bool replaced = false;
		for ( int i = 0; i < m_Vehicles.Count(); i++ )
		{
			JMVehicleMetaData v = m_Vehicles[i];
			if ( v && v.m_NetworkIDLow == fresh.m_NetworkIDLow && v.m_NetworkIDHigh == fresh.m_NetworkIDHigh )
			{
				m_Vehicles.Set( i, fresh );
				replaced = true;
				break;
			}
		}
		if ( !replaced )
			m_Vehicles.Insert( fresh );

		JMVehiclesForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.OnDeltaUpsert( fresh );
	}

	protected void RPC_SendVehicleRemove( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionClient() )
			return;

		int netLow, netHigh;
		if ( !ctx.Read( netLow ) )  return;
		if ( !ctx.Read( netHigh ) ) return;

		for ( int i = m_Vehicles.Count() - 1; i >= 0; i-- )
		{
			JMVehicleMetaData v = m_Vehicles[i];
			if ( v && v.m_NetworkIDLow == netLow && v.m_NetworkIDHigh == netHigh )
				m_Vehicles.Remove( i );
		}

		JMVehiclesForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.OnDeltaRemove( netLow, netHigh );
	}

	void RequestServerVehicles()
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMVehiclesModuleRPC.RequestServerVehicles, true );
	}

	void RPC_RequestServerVehicles( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		if ( !senderRPC ) return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_VEHICLES_VIEW, senderRPC ) )
			return;

		// In SP / listen-server the admin IS the host - update the form directly
		if ( IsMissionHost() && !g_Game.IsDedicatedServer() )
		{
			UpdateVehiclesMetaData_SP();
			return;
		}

		UpdateVehiclesMetaData();

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( m_Vehicles.Count() );
		foreach ( auto vehicle: m_Vehicles )
		{
			vehicle.Write( rpc );
		}
		rpc.Send( NULL, JMVehiclesModuleRPC.SendServerVehicles, true, senderRPC );
	}

	private void RPC_SendServerVehicles( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionClient() )
			return;

		int count;
		if ( !ctx.Read( count ) )
		{
			Error("Couldn't read vehicles count");
			return;
		}

		m_Vehicles.Clear();

		while ( count )
		{
			JMVehicleMetaData vehicle = new JMVehicleMetaData();

			if ( !vehicle.Read( ctx ) )
			{
				Error("Couldn't read vehicle");
				return;
			}

			vehicle.SetDisplayName();

			m_Vehicles.Insert( vehicle );

			count--;
		}

		JMVehiclesForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.LoadVehicles();
	}

	void DeleteVehicleUnclaimed()
	{
	#ifdef EXPANSIONMODVEHICLE
		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMVehiclesModuleRPC.DeleteVehicleUnclaimed, true );
	#endif
	}

	void DeleteVehicleDestroyed()
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMVehiclesModuleRPC.DeleteVehicleDestroyed, true );
	}

	void DeleteVehicleAll()
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMVehiclesModuleRPC.DeleteVehicleAll, true );
	}

	void DeleteVehicle( int netLow, int netHigh )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( netLow );
		rpc.Write( netHigh );
		rpc.Send( NULL, JMVehiclesModuleRPC.DeleteVehicle, true );
	}

	private void RPC_DeleteVehicle( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !GetPermissionsManager().HasPermissionRPC( JMConstants.PERM_VEHICLES_DELETE, senderRPC, instance ) )
			return;

		int netLow;
		if ( !ctx.Read( netLow ) )
			return;

		int netHigh;
		if ( !ctx.Read( netHigh ) )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );

		if ( !obj )
			return;

		string className = obj.GetType();
		GetCommunityOnlineToolsBase().Log( senderRPC, "Deleted vehicle [netId=" + netLow + " " + netHigh + "] " + className );
		SendWebhookColored( "Delete", instance, "Deleted vehicle [netId=" + netLow + " " + netHigh + "] " + className, JMConstants.WEBHOOK_COLOR_CRITICAL );

		g_Game.ObjectDelete( obj );

		SendVehicleRemove( netLow, netHigh, senderRPC );
	}

	#ifdef EXPANSIONMODVEHICLE
	private void RPC_DeleteVehicleUnclaimed( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !GetPermissionsManager().HasPermissionRPC( JMConstants.PERM_VEHICLES_DELETE_UNCLAIMED, senderRPC, instance ) )
			return;

		GetCommunityOnlineToolsBase().Log( senderRPC, "Deleted all UNCLAIMED vehicles" );
		SendWebhookColored( "Delete", instance, "Deleted all UNCLAIMED vehicles", JMConstants.WEBHOOK_COLOR_CRITICAL );

		Exec_DeleteVehicleUnclaimed();

		RPC_RequestServerVehicles( ctx, senderRPC, target );
	}

	void Exec_DeleteVehicleUnclaimed()
	{
		CF_DoublyLinkedNode_WeakRef<CarScript> node = CarScript.s_JM_AllCars.m_Head;
		while ( node )
		{
			CF_DoublyLinkedNode_WeakRef<CarScript> next = node.m_Next;
			if ( !node.m_Value.GetExpansionVehicle().HasKey() )
				node.m_Value.Delete();

			node = next;
		}

		CF_DoublyLinkedNode_WeakRef<BoatScript> boats = BoatScript.s_JM_AllBoats.m_Head;
		while ( boats )
		{
			CF_DoublyLinkedNode_WeakRef<BoatScript> boatNext = boats.m_Next;
			if ( !boats.m_Value.GetExpansionVehicle().HasKey() )
				boats.m_Value.Delete();

			boats = boatNext;
		}

		set<ExpansionVehicleBase> vehicles = ExpansionVehicleBase.GetAll();
		foreach ( ExpansionVehicleBase vehicle: vehicles )
		{
			if ( !vehicle )
				continue;

			if ( vehicle.GetExpansionVehicle().HasKey() )
				continue;

			g_Game.ObjectDelete( vehicle );
		}

		CF_DoublyLinkedNode_WeakRef<ExpansionVehicleCover> cover = ExpansionVehicleCover.s_JM_AllCovers.m_Head;
		while ( cover )
		{
			CF_DoublyLinkedNode_WeakRef<ExpansionVehicleCover> coverNext = cover.m_Next;
			auto keychain = ExpansionKeyChainBase.Cast(cover.m_Value.GetAttachmentByType(ExpansionKeyChainBase));
			if (!keychain || !keychain.Expansion_HasOwner())
				cover.m_Value.Delete();

			cover = coverNext;
		}
	}
	#endif

	private void RPC_DeleteVehicleDestroyed( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !GetPermissionsManager().HasPermissionRPC( JMConstants.PERM_VEHICLES_DELETE_DESTROYED, senderRPC, instance ) )
			return;

		GetCommunityOnlineToolsBase().Log( senderRPC, "Deleted all DESTROYED vehicles" );
		SendWebhookColored( "Delete", instance, "Deleted all DESTROYED vehicles", JMConstants.WEBHOOK_COLOR_CRITICAL );

		Exec_DeleteVehicleDestroyed();

		RPC_RequestServerVehicles( ctx, senderRPC, target );
	}

	void Exec_DeleteVehicleDestroyed()
	{
		CF_DoublyLinkedNode_WeakRef<CarScript> node = CarScript.s_JM_AllCars.m_Head;
		while ( node )
		{
			CF_DoublyLinkedNode_WeakRef<CarScript> next = node.m_Next;
			if ( node.m_Value.IsDamageDestroyed() )
				node.m_Value.Delete();

			node = next;
		}

		CF_DoublyLinkedNode_WeakRef<BoatScript> boats = BoatScript.s_JM_AllBoats.m_Head;
		while ( boats )
		{
			CF_DoublyLinkedNode_WeakRef<BoatScript> boatNext = boats.m_Next;
			if ( boats.m_Value.IsDamageDestroyed() )
				boats.m_Value.Delete();

			boats = boatNext;
		}

	#ifndef DAYZ_1_29
		CF_DoublyLinkedNode_WeakRef<MotorbikeScript> bikes = MotorbikeScript.s_JM_AllBikes.m_Head;
		while ( bikes )
		{
			CF_DoublyLinkedNode_WeakRef<MotorbikeScript> bikeNext = bikes.m_Next;
			if ( bikes.m_Value.IsDamageDestroyed() )
				bikes.m_Value.Delete();

			bikes = bikeNext;
		}
	#endif

		#ifdef EXPANSIONMODVEHICLE
		set<ExpansionVehicleBase> vehicles = ExpansionVehicleBase.GetAll();
		foreach ( ExpansionVehicleBase vehicle: vehicles )
		{
			if ( !vehicle )
				continue;

			if ( !vehicle.IsDamageDestroyed() )
				continue;

			g_Game.ObjectDelete( vehicle );
		}

		CF_DoublyLinkedNode_WeakRef<ExpansionVehicleCover> cover = ExpansionVehicleCover.s_JM_AllCovers.m_Head;
		while ( cover )
		{
			CF_DoublyLinkedNode_WeakRef<ExpansionVehicleCover> coverNext = cover.m_Next;
			if ( cover.m_Value.IsDamageDestroyed() )
				cover.m_Value.Delete();

			cover = coverNext;
		}
		#endif
	}

	private void RPC_DeleteVehicleAll( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !GetPermissionsManager().HasPermissionRPC( JMConstants.PERM_VEHICLES_DELETE_ALL, senderRPC, instance ) )
			return;

		GetCommunityOnlineToolsBase().Log( senderRPC, "Deleted ALL vehicles" );
		SendWebhookColored( "Delete", instance, "Deleted ALL vehicles", JMConstants.WEBHOOK_COLOR_CRITICAL );

		Exec_DeleteVehicleAll();

		RPC_RequestServerVehicles( ctx, senderRPC, target );
	}

	void Exec_DeleteVehicleAll()
	{
		CF_DoublyLinkedNode_WeakRef<CarScript> node = CarScript.s_JM_AllCars.m_Head;
		while ( node )
		{
			CF_DoublyLinkedNode_WeakRef<CarScript> next = node.m_Next;
			node.m_Value.Delete();
			node = next;
		}

		CF_DoublyLinkedNode_WeakRef<BoatScript> boats = BoatScript.s_JM_AllBoats.m_Head;
		while ( boats )
		{
			CF_DoublyLinkedNode_WeakRef<BoatScript> boatNext = boats.m_Next;
			boats.m_Value.Delete();
			boats = boatNext;
		}

	#ifndef DAYZ_1_29
		CF_DoublyLinkedNode_WeakRef<MotorbikeScript> bikes = MotorbikeScript.s_JM_AllBikes.m_Head;
		while ( bikes )
		{
			CF_DoublyLinkedNode_WeakRef<MotorbikeScript> bikeNext = bikes.m_Next;
			bikes.m_Value.Delete();
			bikes = bikeNext;
		}
	#endif

		#ifdef EXPANSIONMODVEHICLE
		set<ExpansionVehicleBase> vehicles = ExpansionVehicleBase.GetAll();
		foreach ( ExpansionVehicleBase vehicle: vehicles )
		{
			if ( !vehicle )
				continue;

			g_Game.ObjectDelete( vehicle );
		}

		CF_DoublyLinkedNode_WeakRef<ExpansionVehicleCover> cover = ExpansionVehicleCover.s_JM_AllCovers.m_Head;
		while ( cover )
		{
			CF_DoublyLinkedNode_WeakRef<ExpansionVehicleCover> coverNext = cover.m_Next;
			cover.m_Value.Delete();
			cover = coverNext;
		}
		#endif
	}

	void RequestTeleportToVehicle( JMVehicleMetaData meta )
	{
		//! This one moves the ADMIN to the vehicle, so it is the admin's own
		//! position that the undo has to be able to come back to.
		JMTeleportHistory.PushSelf();

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( meta.m_NetworkIDLow );
		rpc.Write( meta.m_NetworkIDHigh );
		rpc.Send( NULL, JMVehiclesModuleRPC.TeleportToVehicle, true );
	}

	private void RPC_TeleportToVehicle( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !GetPermissionsManager().HasPermissionRPC( JMConstants.PERM_VEHICLES_TELEPORT, senderRPC, instance ) )
			return;

		int netLow;
		if ( !ctx.Read( netLow ) )
			return;

		int netHigh;
		if ( !ctx.Read( netHigh ) )
			return;
		
		PlayerBase player;
		if ( !Class.CastTo( player, senderRPC.GetPlayer() ) )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		GetCommunityOnlineToolsBase().Log( senderRPC, "Teleported to vehicle [netId=" + netLow + " " + netHigh + "]" );
		SendWebhookColored( "Teleport", instance, "Teleported to vehicle [netId=" + netLow + " " + netHigh + "]", JMConstants.WEBHOOK_COLOR_INFO );

		Exec_TeleportToVehicle(player, obj);
	}

	void Exec_TeleportToVehicle( PlayerBase player, Object obj )
	{
		vector pos = obj.GetPosition();
		pos[1] = g_Game.SurfaceRoadY3D( pos[0], pos[1], pos[2], RoadSurfaceDetection.UNDER );
		player.SetLastPosition();
		player.SetWorldPosition( pos );
	}


	void RequestTeleportVehicleToMe( JMVehicleMetaData meta )
	{
		//! Recorded from the metadata rather than from the entity: the vehicle
		//! being moved is often nowhere near the admin, so there may be no
		//! client-side object to ask.
		JMTeleportHistory.Push( JMTeleportHistory.ObjectKey( meta.m_NetworkIDLow, meta.m_NetworkIDHigh ), meta.m_Position );

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( meta.m_NetworkIDLow );
		rpc.Write( meta.m_NetworkIDHigh );
		rpc.Send( NULL, JMVehiclesModuleRPC.TeleportVehicleToMe, true );
	}

	private void RPC_TeleportVehicleToMe( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !GetPermissionsManager().HasPermissionRPC( JMConstants.PERM_VEHICLES_TELEPORT, senderRPC, instance ) )
			return;

		int netLow;
		if ( !ctx.Read( netLow ) )
			return;

		int netHigh;
		if ( !ctx.Read( netHigh ) )
			return;

		PlayerBase player;
		if ( !Class.CastTo( player, senderRPC.GetPlayer() ) )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		GetCommunityOnlineToolsBase().Log( senderRPC, "Teleported vehicle [netId=" + netLow + " " + netHigh + "] to self" );
		SendWebhookColored( "Teleport", instance, "Teleported vehicle [netId=" + netLow + " " + netHigh + "] to self", JMConstants.WEBHOOK_COLOR_INFO );

		Exec_TeleportVehicleToMe( player, obj );

		// Position changed - push a fresh metadata upsert to the requester.
		SendVehicleUpsert( netLow, netHigh, senderRPC );
	}

	void Exec_TeleportVehicleToMe( PlayerBase player, Object obj )
	{
		vector pos = player.GetPosition();
		pos[1] = g_Game.SurfaceRoadY3D( pos[0], pos[1], pos[2], RoadSurfaceDetection.UNDER );
		obj.SetPosition( pos );
	}

	//! Move a vehicle to an exact position.
	//!
	//! The undo path. Every other vehicle teleport works out its destination on
	//! the server - the admin's feet, the surface under the vehicle - so this is
	//! the only one that has to carry a position on the wire.
	void RequestTeleportVehicleTo( JMVehicleMetaData meta, vector position )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( meta.m_NetworkIDLow );
		rpc.Write( meta.m_NetworkIDHigh );
		rpc.Write( position );
		rpc.Send( NULL, JMVehiclesModuleRPC.TeleportVehicleTo, true );
	}

	private void RPC_TeleportVehicleTo( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !GetPermissionsManager().HasPermissionRPC( JMConstants.PERM_VEHICLES_TELEPORT, senderRPC, instance ) )
			return;

		int netLow;
		if ( !ctx.Read( netLow ) )
			return;

		int netHigh;
		if ( !ctx.Read( netHigh ) )
			return;

		vector position;
		if ( !ctx.Read( position ) )
			return;

		if ( !CommunityOnlineToolsBase.IsValidWorldPosition( position ) )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		GetCommunityOnlineToolsBase().Log( senderRPC, "Teleported vehicle [netId=" + netLow + " " + netHigh + "] to " + position );
		SendWebhookColored( "Teleport", instance, "Teleported vehicle [netId=" + netLow + " " + netHigh + "] to " + position.ToString(), JMConstants.WEBHOOK_COLOR_INFO );

		Exec_TeleportVehicleTo( obj, position );

		// Position changed - push a fresh metadata upsert to the requester.
		SendVehicleUpsert( netLow, netHigh, senderRPC );
	}

	//! No surface snap: the position is one the vehicle was actually AT, and
	//! re-grounding it would walk it a little further off every undo.
	void Exec_TeleportVehicleTo( Object obj, vector position )
	{
		obj.SetPosition( position );
	}

	// -------------------------------------------------------------------------
	// Repair
	// -------------------------------------------------------------------------

	void RequestRepairVehicle( JMVehicleMetaData meta )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( meta.m_NetworkIDLow );
		rpc.Write( meta.m_NetworkIDHigh );
		rpc.Send( NULL, JMVehiclesModuleRPC.RepairVehicle, true );
	}

	private void RPC_RepairVehicle( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !GetPermissionsManager().HasPermissionRPC( JMConstants.PERM_VEHICLES_REPAIR, senderRPC, instance ) )
			return;

		int netLow;
		if ( !ctx.Read( netLow ) )
			return;

		int netHigh;
		if ( !ctx.Read( netHigh ) )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		GetCommunityOnlineToolsBase().Log( senderRPC, "Repaired vehicle [netId=" + netLow + " " + netHigh + "]" );
		SendWebhookColored( "Repair", instance, "Repaired vehicle [netId=" + netLow + " " + netHigh + "]", JMConstants.WEBHOOK_COLOR_INFO );

		Exec_RepairVehicle( obj );

		SendVehicleUpsert( netLow, netHigh, senderRPC );
	}

	void Exec_RepairVehicle( Object obj )
	{
		EntityAI entity = EntityAI.Cast( obj );
		if ( !entity )
			return;

		// Restore health on vehicle body and all existing attachments
		entity.SetHealth( "", "", entity.GetMaxHealth( "", "" ) );

		int attachCount = entity.GetInventory().AttachmentCount();
		for ( int i = 0; i < attachCount; i++ )
		{
			EntityAI attachment = entity.GetInventory().GetAttachmentFromIndex( i );
			if ( attachment )
				attachment.SetHealth( "", "", attachment.GetMaxHealth( "", "" ) );
		}

		// Spawn any missing compatible attachments (wheels, engine parts, etc.)
		JMObjectSpawnerModule spawnerModule;
		if ( CF_Modules<JMObjectSpawnerModule>.Get( spawnerModule ) )
			spawnerModule.SpawnCompatibleAttachments( entity, NULL, 2 );

		// Refuel
		Exec_RefuelVehicle( obj );
	}

	// -------------------------------------------------------------------------
	// Refuel
	// -------------------------------------------------------------------------

	void RequestRefuelVehicle( JMVehicleMetaData meta )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( meta.m_NetworkIDLow );
		rpc.Write( meta.m_NetworkIDHigh );
		rpc.Send( NULL, JMVehiclesModuleRPC.RefuelVehicle, true );
	}

	private void RPC_RefuelVehicle( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !GetPermissionsManager().HasPermissionRPC( JMConstants.PERM_VEHICLES_REFUEL, senderRPC, instance ) )
			return;

		int netLow;
		if ( !ctx.Read( netLow ) )
			return;

		int netHigh;
		if ( !ctx.Read( netHigh ) )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		GetCommunityOnlineToolsBase().Log( senderRPC, "Refueled vehicle [netId=" + netLow + " " + netHigh + "]" );
		SendWebhookColored( "Refuel", instance, "Refueled vehicle [netId=" + netLow + " " + netHigh + "]", JMConstants.WEBHOOK_COLOR_INFO );

		Exec_RefuelVehicle( obj );

		SendVehicleUpsert( netLow, netHigh, senderRPC );
	}

	void Exec_RefuelVehicle( Object obj )
	{
		CarScript car = CarScript.Cast( obj );
		if ( car )
		{
			car.Fill( CarFluid.FUEL, car.GetFluidCapacity( CarFluid.FUEL ) );
			car.Fill( CarFluid.OIL, car.GetFluidCapacity( CarFluid.OIL ) );
			car.Fill( CarFluid.BRAKE, car.GetFluidCapacity( CarFluid.BRAKE ) );
			car.Fill( CarFluid.COOLANT, car.GetFluidCapacity( CarFluid.COOLANT ) );
			return;
		}

	#ifndef DAYZ_1_29
		MotorbikeScript bike = MotorbikeScript.Cast( obj );
		if ( bike )
		{
			bike.COT_Refuel();
			return;
		}
	#endif

		#ifdef EXPANSIONMODVEHICLE
		ExpansionVehicleBase expVehicle = ExpansionVehicleBase.Cast( obj );
		if ( expVehicle )
		{
			expVehicle.Fill( CarFluid.FUEL, expVehicle.GetFluidCapacity( CarFluid.FUEL ) );
			expVehicle.Fill( CarFluid.OIL, expVehicle.GetFluidCapacity( CarFluid.OIL ) );
			expVehicle.Fill( CarFluid.BRAKE, expVehicle.GetFluidCapacity( CarFluid.BRAKE ) );
			expVehicle.Fill( CarFluid.COOLANT, expVehicle.GetFluidCapacity( CarFluid.COOLANT ) );
		}
		#endif
	}

	// -------------------------------------------------------------------------
	// Unstuck (teleport slightly up so it drops to the ground)
	// -------------------------------------------------------------------------

	void RequestUnstuckVehicle( JMVehicleMetaData meta )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( meta.m_NetworkIDLow );
		rpc.Write( meta.m_NetworkIDHigh );
		rpc.Send( NULL, JMVehiclesModuleRPC.UnstuckVehicle, true );
	}

	private void RPC_UnstuckVehicle( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !GetPermissionsManager().HasPermissionRPC( JMConstants.PERM_VEHICLES_UNSTUCK, senderRPC, instance ) )
			return;

		int netLow;
		if ( !ctx.Read( netLow ) )
			return;

		int netHigh;
		if ( !ctx.Read( netHigh ) )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		GetCommunityOnlineToolsBase().Log( senderRPC, "Unstuck vehicle [netId=" + netLow + " " + netHigh + "]" );
		SendWebhookColored( "Unstuck", instance, "Unstuck vehicle [netId=" + netLow + " " + netHigh + "]", JMConstants.WEBHOOK_COLOR_INFO );

		Exec_UnstuckVehicle( obj );

		SendVehicleUpsert( netLow, netHigh, senderRPC );
	}

	void Exec_UnstuckVehicle( Object obj )
	{
		vector pos = obj.GetPosition();
		pos[1] = pos[1] + 1.5;
		obj.SetPosition( pos );
	}

	// -------------------------------------------------------------------------
	// Cover / Uncover (Expansion only)
	// -------------------------------------------------------------------------

	void RequestCoverVehicle( JMVehicleMetaData meta )
	{
	#ifdef EXPANSIONMODVEHICLE
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( meta.m_NetworkIDLow );
		rpc.Write( meta.m_NetworkIDHigh );
		rpc.Send( NULL, JMVehiclesModuleRPC.CoverVehicle, true );
	#endif
	}

	private void RPC_CoverVehicle( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
	#ifdef EXPANSIONMODVEHICLE
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !GetPermissionsManager().HasPermissionRPC( JMConstants.PERM_VEHICLES_COVER, senderRPC, instance ) )
			return;

		int netLow;
		if ( !ctx.Read( netLow ) )
			return;

		int netHigh;
		if ( !ctx.Read( netHigh ) )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		GetCommunityOnlineToolsBase().Log( senderRPC, "Cover/uncover vehicle [netId=" + netLow + " " + netHigh + "]" );
		SendWebhookColored( "Cover", instance, "Cover/uncover vehicle [netId=" + netLow + " " + netHigh + "]", JMConstants.WEBHOOK_COLOR_INFO );

		JMVehicleMetaData meta = new JMVehicleMetaData();
		meta.AcquireFrom( EntityAI.Cast( obj ) );
		Exec_CoverVehicle( obj, meta );

		// Cover/Uncover swaps the world object for a placeholder or vice versa,
		// so the old netId may be invalid. Easiest correct path is a full
		// refresh here - it's rare and the only action with this semantic.
		RPC_RequestServerVehicles( ctx, senderRPC, target );
	#endif
	}

	void Exec_CoverVehicle( Object obj, JMVehicleMetaData meta )
	{
	#ifdef EXPANSIONMODVEHICLE
		ExpansionEntityStoragePlaceholder placeholder;
		if ( meta.m_IsCover )
		{
			// It is already a cover - uncover it
			placeholder = ExpansionEntityStoragePlaceholder.Cast( obj );
			if ( placeholder && placeholder.Expansion_HasStoredEntity() )
			{
				EntityAI restoredEntity;
				vector position = placeholder.GetPosition();
				vector orientation = placeholder.GetOrientation();

				placeholder.SetPosition( "0 0 0" );

				if ( ExpansionEntityStorageModule.RestoreFromFile( placeholder.Expansion_GetEntityStorageFileName(), restoredEntity, placeholder ) )
				{
					// Calculate proper position offset
					vector placeholderMinMax[2];
					if ( !placeholder.GetCollisionBox( placeholderMinMax ) )
						placeholder.ClippingInfo( placeholderMinMax );

					float placeHolderOffsetY = placeholderMinMax[0][1];
					if ( placeHolderOffsetY > 0 )
						placeHolderOffsetY = 0;

					vector entityMinMax[2];
					if ( !restoredEntity.GetCollisionBox( entityMinMax ) )
						restoredEntity.ClippingInfo( entityMinMax );

					float entityOffsetY = entityMinMax[0][1];
					if ( entityOffsetY > 0 )
						entityOffsetY = 0;

					position[1] = position[1] + placeHolderOffsetY - entityOffsetY;

					// Handle keychain transfer
					int slotId = InventorySlots.GetSlotIdFromString( "KeyChain" );
					auto keychain = ExpansionKeyChainBase.Cast( placeholder.GetInventory().FindAttachment( slotId ) );
					if ( keychain && restoredEntity.GetInventory().HasAttachmentSlot( slotId ) )
					{
						restoredEntity.ServerTakeEntityAsAttachmentEx( keychain, slotId );
					}

					g_Game.ObjectDelete( placeholder );

					restoredEntity.SetPosition( position );
					restoredEntity.SetOrientation( orientation );
				}
				else
				{
					placeholder.SetPosition( position );
				}
			}
		}
		else
		{
			// Cover the vehicle
			ExpansionVehicle vehicle = ExpansionVehicle.Get( obj );
			if ( vehicle && vehicle.CanCover() )
			{
				vehicle.Cover( null, placeholder );
			}
		}
	#endif
	}

	// -------------------------------------------------------------------------
	// Lock / Unlock (Expansion only)
	// -------------------------------------------------------------------------

	void RequestLockVehicle( JMVehicleMetaData meta )
	{
		RequestLockVehicleById( meta.m_NetworkIDLow, meta.m_NetworkIDHigh );
	}

	//! Same request from a caller that only has a network id - the ESP menu
	//! works off JMESPMeta, which never builds a JMVehicleMetaData.
	void RequestLockVehicleById( int netLow, int netHigh )
	{
	#ifdef EXPANSIONMODVEHICLE
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( netLow );
		rpc.Write( netHigh );
		rpc.Send( NULL, JMVehiclesModuleRPC.LockVehicle, true );
	#endif
	}

	private void RPC_LockVehicle( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
	#ifdef EXPANSIONMODVEHICLE
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !GetPermissionsManager().HasPermissionRPC( JMConstants.PERM_VEHICLES_LOCK, senderRPC, instance ) )
			return;

		int netLow;
		if ( !ctx.Read( netLow ) )
			return;

		int netHigh;
		if ( !ctx.Read( netHigh ) )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		GetCommunityOnlineToolsBase().Log( senderRPC, "Lock/unlock vehicle [netId=" + netLow + " " + netHigh + "]" );
		SendWebhookColored( "Lock", instance, "Lock/unlock vehicle [netId=" + netLow + " " + netHigh + "]", JMConstants.WEBHOOK_COLOR_INFO );

		Exec_LockVehicle( obj );

		SendVehicleUpsert( netLow, netHigh, senderRPC );
	#endif
	}

	void Exec_LockVehicle( Object obj )
	{
	#ifdef EXPANSIONMODVEHICLE
		ExpansionVehicle expVehicle;
		if ( ExpansionVehicle.Get( expVehicle, EntityAI.Cast( obj ) ) )
		{
			ExpansionCarAdminKey admincarkey;
			if ( expVehicle.IsLocked() )
				expVehicle.Unlock(admincarkey);
			else
				expVehicle.Lock(admincarkey);
		}
	#endif
	}

	// -------------------------------------------------------------------------
	// UnPair Keys (Expansion only)
	// -------------------------------------------------------------------------

	void RequestUnPairVehicle( JMVehicleMetaData meta )
	{
		RequestUnPairVehicleById( meta.m_NetworkIDLow, meta.m_NetworkIDHigh );
	}

	//! See RequestLockVehicleById.
	void RequestUnPairVehicleById( int netLow, int netHigh )
	{
	#ifdef EXPANSIONMODVEHICLE
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( netLow );
		rpc.Write( netHigh );
		rpc.Send( NULL, JMVehiclesModuleRPC.UnPairVehicle, true );
	#endif
	}

	private void RPC_UnPairVehicle( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
	#ifdef EXPANSIONMODVEHICLE
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !GetPermissionsManager().HasPermissionRPC( JMConstants.PERM_VEHICLES_UNPAIR, senderRPC, instance ) )
			return;

		int netLow;
		if ( !ctx.Read( netLow ) )
			return;

		int netHigh;
		if ( !ctx.Read( netHigh ) )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		GetCommunityOnlineToolsBase().Log( senderRPC, "Unpaired keys for vehicle [netId=" + netLow + " " + netHigh + "]" );
		SendWebhookColored( "Unpair", instance, "Unpaired keys for vehicle [netId=" + netLow + " " + netHigh + "]", JMConstants.WEBHOOK_COLOR_INFO );

		Exec_UnPairVehicle( obj );

		SendVehicleUpsert( netLow, netHigh, senderRPC );
	#endif
	}

	void Exec_UnPairVehicle( Object obj )
	{
	#ifdef EXPANSIONMODVEHICLE
		ExpansionVehicle expVehicle;
		if ( ExpansionVehicle.Get( expVehicle, EntityAI.Cast( obj ) ) )
		{
			expVehicle.ResetKeyPairing();
		}
	#endif
	}

	// -------------------------------------------------------------------------
	// Cargo
	// -------------------------------------------------------------------------

	//! Empty a vehicle's trunk and seats.
	//!
	//! Its own permission rather than Vehicles.Delete: deleting a vehicle and
	//! emptying one are different sizes of mistake, and an admin trusted to
	//! tidy a trunk is not necessarily trusted to remove the car.
	void RequestClearVehicleCargo( JMVehicleMetaData meta )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( meta.m_NetworkIDLow );
		rpc.Write( meta.m_NetworkIDHigh );
		rpc.Send( NULL, JMVehiclesModuleRPC.ClearVehicleCargo, true );
	}

	private void RPC_ClearVehicleCargo( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !GetPermissionsManager().HasPermissionRPC( JMConstants.PERM_VEHICLES_CLEARCARGO, senderRPC, instance ) )
			return;

		int netLow;
		if ( !ctx.Read( netLow ) )
			return;

		int netHigh;
		if ( !ctx.Read( netHigh ) )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		int removed = Exec_ClearVehicleCargo( obj );

		GetCommunityOnlineToolsBase().Log( senderRPC, "Cleared vehicle cargo [netId=" + netLow + " " + netHigh + "] items=" + removed );
		SendWebhookColored( "Vehicle", instance, "Cleared " + removed + " item(s) from vehicle [netId=" + netLow + " " + netHigh + "]", JMConstants.WEBHOOK_COLOR_WARNING );
	}

	//! Answers how many items it removed, which is what the log wants: emptying
	//! a full truck and emptying an empty one are the same action and should
	//! not read the same afterwards.
	//!
	//! Walked BACKWARDS: deleting an item shifts every index above it down, so
	//! a forward loop skips every second one.
	int Exec_ClearVehicleCargo( Object obj )
	{
		EntityAI entity = EntityAI.Cast( obj );

		if ( !entity || !entity.GetInventory() )
			return 0;

		CargoBase cargo = entity.GetInventory().GetCargo();

		if ( !cargo )
			return 0;

		int removed = 0;

		for ( int i = cargo.GetItemCount() - 1; i >= 0; i-- )
		{
			EntityAI item = cargo.GetItem( i );

			if ( !item )
				continue;

			g_Game.ObjectDelete( item );
			removed++;
		}

		return removed;
	}

	// -------------------------------------------------------------------------
	// Keys (Expansion only)
	// -------------------------------------------------------------------------

	//! Spawn a key already paired to this vehicle, into the admin's inventory.
	//!
	//! The counterpart to UnPair: that one orphans a vehicle whose keys are
	//! lost, this one gets back into one whose keys are lost without having to
	//! orphan it first.
	void RequestSpawnVehicleKey( JMVehicleMetaData meta )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( meta.m_NetworkIDLow );
		rpc.Write( meta.m_NetworkIDHigh );
		rpc.Send( NULL, JMVehiclesModuleRPC.SpawnVehicleKey, true );
	}

	private void RPC_SpawnVehicleKey( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !GetPermissionsManager().HasPermissionRPC( JMConstants.PERM_VEHICLES_SPAWNKEY, senderRPC, instance ) )
			return;

		int netLow;
		if ( !ctx.Read( netLow ) )
			return;

		int netHigh;
		if ( !ctx.Read( netHigh ) )
			return;

		PlayerBase player;
		if ( !Class.CastTo( player, senderRPC.GetPlayer() ) )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		if ( !Exec_SpawnVehicleKey( player, obj ) )
			return;

		GetCommunityOnlineToolsBase().Log( senderRPC, "Spawned a paired key for vehicle [netId=" + netLow + " " + netHigh + "]" );
		SendWebhookColored( "Vehicle", instance, "Spawned a paired key for vehicle [netId=" + netLow + " " + netHigh + "]", JMConstants.WEBHOOK_COLOR_INFO );
	}

	//! Expansion owns vehicle keys - vanilla has no such item - so without it
	//! this answers false and the menu row is never offered in the first place.
	bool Exec_SpawnVehicleKey( PlayerBase player, Object obj )
	{
	#ifdef EXPANSIONMODVEHICLE
		ExpansionVehicle expVehicle;

		if ( !ExpansionVehicle.Get( expVehicle, EntityAI.Cast( obj ) ) )
			return false;

		ExpansionCarKey key;

		if ( !Class.CastTo( key, player.GetInventory().CreateInInventory( "ExpansionCarKey" ) ) )
			return false;

		//! Paired from the VEHICLE side: ExpansionCarKey.PairToVehicle is
		//! overloaded four ways and picking between them from a cast is the
		//! sort of thing that resolves differently on the next Expansion build.
		expVehicle.PairKey( key );

		return true;
	#else
		return false;
	#endif
	}

	array< ref JMVehicleMetaData > GetServerVehicles()
	{
		return m_Vehicles;
	}
}
