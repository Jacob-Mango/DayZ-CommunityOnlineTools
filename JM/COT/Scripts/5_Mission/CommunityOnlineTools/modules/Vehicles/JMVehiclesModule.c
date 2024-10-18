const int JMVT_NONE = 0;
const int JMVT_CAR = 2;
const int JMVT_BOAT = 4;
const int JMVT_HELICOPTER = 8;
const int JMVT_PLANE = 32;
const int JMVT_BIKE = 512;

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

	string m_OwnerName;
	string m_OwnerUID;
	string m_LastDriverUID;

	[NonSerialized()]
	string m_DisplayName;

	[NonSerialized()]
	EntityAI m_Entity;
	
	static JMVehicleMetaData Create(EntityAI entity, string type = string.Empty)
	{
		JMVehicleMetaData meta = new JMVehicleMetaData();

		meta.AcquireFrom(entity, type);

		return meta;
	}

	void AcquireFrom(EntityAI entity, string type = string.Empty)
	{
		m_Entity = entity;

		entity.GetNetworkID(m_NetworkIDLow, m_NetworkIDHigh);
		entity.GetPersistentID(m_PersistentIDA, m_PersistentIDB, m_PersistentIDC, m_PersistentIDD);

		if (type == string.Empty)
			type = entity.GetType();

		m_ClassName = type;
		m_Position = entity.GetPosition();
		m_Orientation = entity.GetOrientation();

		if (entity.IsDamageDestroyed())
			m_DestructionType |= JMDT_DESTROYED;

	#ifdef EXPANSIONMODVEHICLE
		ExpansionVehicle vehicle;
		if (ExpansionVehicle.Get(vehicle, entity))
		{
			if (vehicle.IsCar())
				m_VehicleType |= JMVT_CAR;

			if (vehicle.IsBoat())
				m_VehicleType |= JMVT_BOAT;

			if (vehicle.IsHelicopter())
				m_VehicleType |= JMVT_HELICOPTER;

			if (vehicle.IsPlane())
				m_VehicleType |= JMVT_PLANE;

			if (vehicle.IsExploded())
				m_DestructionType |= JMDT_EXPLODED;

			m_HasKeys = vehicle.HasKey();

			m_OwnerName = vehicle.GetOwnerName();
			m_OwnerUID = vehicle.GetOwnerUID();
			m_LastDriverUID = vehicle.GetLastDriverUID();
		}
	#else
		if (entity.IsInherited(CarScript))
			m_VehicleType = JMVT_CAR;
		else if (entity.IsInherited(BoatScript))
			m_VehicleType = JMVT_BOAT;
	#endif
	}

	static JMVehicleMetaData CreateCarScript( CarScript car )
	{
		Error("DEPRECATED, use Create");
		return Create(car);
	}
	
	#ifdef EXPANSIONMODVEHICLE
	static JMVehicleMetaData CreateVehicle( ExpansionVehicleBase vehicle )
	{
		Error("DEPRECATED, use Create");
		return Create(vehicle);
	}
	
	static JMVehicleMetaData CreateCover( ExpansionVehicleCover cover )
	{
		string type = cover.Expansion_GetStoredEntityType();

		JMVehicleMetaData meta = Create(cover, type);

		if ( GetGame().IsKindOf(type, "ExpansionHelicopterScript") )
			meta.m_VehicleType |= JMVT_HELICOPTER;
		else if ( GetGame().IsKindOf(type, "ExpansionBoatScript") || GetGame().IsKindOf(type, "BoatScript") )
			meta.m_VehicleType |= JMVT_BOAT;
		else
			meta.m_VehicleType |= JMVT_CAR;

		auto keychain = ExpansionKeyChainBase.Cast(cover.GetAttachmentByType(ExpansionKeyChainBase));
		if (keychain && keychain.Expansion_HasOwner())
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
			type += "Bike ";

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
		if (GetGame().ConfigIsExisting("cfgVehicles " + m_ClassName + " displayName"))
			GetGame().ConfigGetText( "cfgVehicles " + m_ClassName + " displayName", m_DisplayName );
		else
			m_DisplayName = m_ClassName;
	}

	void Write( ParamsWriteContext ctx )
	{
		ctx.Write(m_NetworkIDLow);
		ctx.Write(m_NetworkIDHigh);

		ctx.Write(m_PersistentIDA);
		ctx.Write(m_PersistentIDB);
		ctx.Write(m_PersistentIDC);
		ctx.Write(m_PersistentIDD);

		ctx.Write(m_ClassName);
		ctx.Write(m_Position);
		ctx.Write(m_Orientation);

		ctx.Write(m_VehicleType);
		ctx.Write(m_DestructionType);

	#ifdef EXPANSIONMODVEHICLE
		ctx.Write(m_HasKeys);
		ctx.Write(m_IsCover);

		ctx.Write(m_OwnerName);
		ctx.Write(m_OwnerUID);
	#endif

	#ifdef EXPANSIONMODCORE
		ctx.Write(m_LastDriverUID);
	#endif
	}

	bool Read( ParamsReadContext ctx )
	{
		if (!ctx.Read(m_NetworkIDLow)) return false;
		if (!ctx.Read(m_NetworkIDHigh)) return false;

		if (!ctx.Read(m_PersistentIDA)) return false;
		if (!ctx.Read(m_PersistentIDB)) return false;
		if (!ctx.Read(m_PersistentIDC)) return false;
		if (!ctx.Read(m_PersistentIDD)) return false;

		if (!ctx.Read(m_ClassName)) return false;
		if (!ctx.Read(m_Position)) return false;
		if (!ctx.Read(m_Orientation)) return false;

		if (!ctx.Read(m_VehicleType)) return false;
		if (!ctx.Read(m_DestructionType)) return false;

	#ifdef EXPANSIONMODVEHICLE
		if (!ctx.Read(m_HasKeys)) return false;
		if (!ctx.Read(m_IsCover)) return false;

		if (!ctx.Read(m_OwnerName)) return false;
		if (!ctx.Read(m_OwnerUID)) return false;
	#endif

	#ifdef EXPANSIONMODCORE
		if (!ctx.Read(m_LastDriverUID)) return false;
	#endif

		return true;
	}
};

class JMVehiclesModule: JMRenderableModuleBase
{
	private ref array<ref JMVehicleMetaData> m_Vehicles;

	void JMVehiclesModule()
	{
		GetPermissionsManager().RegisterPermission( "Vehicles.View" );
		
		GetPermissionsManager().RegisterPermission( "Vehicles.Delete" );
		GetPermissionsManager().RegisterPermission( "Vehicles.Delete.All" );
		GetPermissionsManager().RegisterPermission( "Vehicles.Delete.Destroyed" );
		#ifdef EXPANSIONMODVEHICLE
		GetPermissionsManager().RegisterPermission( "Vehicles.Delete.Unclaimed" );
		#endif
		GetPermissionsManager().RegisterPermission( "Vehicles.Teleport" );
		
		m_Vehicles = new array<ref JMVehicleMetaData>;
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Vehicles.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/vehicles/Vehicles_Menu.layout";
	}

	override string GetTitle()
	{
		return "#STR_COT_VEHICLE_MANAGEMENT_MODULE_NAME";
	}

	override string GetIconName()
	{
		return "JM\\COT\\GUI\\textures\\modules\\Vehicles.paa";
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
	}

	private void UpdateVehiclesMetaData()
	{
		if ( m_Vehicles.Count() > 0 )
			m_Vehicles.Clear();
		
		auto node = CarScript.s_JM_AllCars.m_Head;
		while ( node )
		{
			if ( !node.m_Value.IsSetForDeletion() )
				m_Vehicles.Insert( JMVehicleMetaData.Create( node.m_Value ) );
			node = node.m_Next;
		}
		
		auto boat = BoatScript.s_JM_AllBoats.m_Head;
		while ( boat )
		{
			if ( !boat.m_Value.IsSetForDeletion() )
				m_Vehicles.Insert( JMVehicleMetaData.Create( boat.m_Value ) );
			boat = boat.m_Next;
		}
		
		#ifdef EXPANSIONMODVEHICLE
		auto vehicles = ExpansionVehicleBase.GetAll();
		foreach ( ExpansionVehicleBase vehicle: vehicles )
		{
			if ( !vehicle ) // should not be possible
				continue;

			if ( !vehicle.IsSetForDeletion() )
				m_Vehicles.Insert( JMVehicleMetaData.Create( vehicle ) );
		}

		auto cover = ExpansionVehicleCover.s_JM_AllCovers.m_Head;
		while ( cover )
		{
			if ( !cover.m_Value.IsSetForDeletion() )
				m_Vehicles.Insert( JMVehicleMetaData.CreateCover( cover.m_Value ) );
			cover = cover.m_Next;
		}
		#endif
	}

	void UpdateVehiclesMetaData_SP()
	{
		UpdateVehiclesMetaData();

		foreach (auto meta: m_Vehicles)
		{
			meta.SetDisplayName();
		}

		JMVehiclesMenu form;
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
		}
	}

	void RequestServerVehicles()
	{
		if (GetGame().IsClient())
		{
			auto rpc = new ScriptRPC();
 			rpc.Send( NULL, JMVehiclesModuleRPC.RequestServerVehicles, true );
		}
		else
		{
			UpdateVehiclesMetaData_SP();
		}
	}

	void RPC_RequestServerVehicles( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Vehicles.View", senderRPC ) )
			return;
		
		UpdateVehiclesMetaData();

		auto rpc = new ScriptRPC();
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
			auto vehicle = new JMVehicleMetaData();

			if ( !vehicle.Read( ctx ) )
			{
				Error("Couldn't read vehicle");
				return;
			}

			vehicle.SetDisplayName();

			m_Vehicles.Insert( vehicle );

			count--;
		}

		JMVehiclesMenu form;
		if ( Class.CastTo( form, GetForm() ) )
			form.LoadVehicles();
	}

	void DeleteVehicleUnclaimed( )
	{
	#ifdef EXPANSIONMODVEHICLE
		if (GetGame().IsClient())
		{
			auto rpc = new ScriptRPC();
			
			rpc.Send( NULL, JMVehiclesModuleRPC.DeleteVehicleUnclaimed, true );
		}
		else
		{
			Exec_DeleteVehicleUnclaimed();
			UpdateVehiclesMetaData_SP();
		}
	#endif
	}

	void DeleteVehicleDestroyed( )
	{
		if (GetGame().IsClient())
		{
			auto rpc = new ScriptRPC();
			
			rpc.Send( NULL, JMVehiclesModuleRPC.DeleteVehicleDestroyed, true );
		}
		else
		{
			Exec_DeleteVehicleDestroyed();
			UpdateVehiclesMetaData_SP();
		}
	}

	void DeleteVehicleAll( )
	{
		if (GetGame().IsClient())
		{
			auto rpc = new ScriptRPC();
			
			rpc.Send( NULL, JMVehiclesModuleRPC.DeleteVehicleAll, true );
		}
		else
		{
			Exec_DeleteVehicleAll();
			UpdateVehiclesMetaData_SP();
		}
	}

	void DeleteVehicle(JMVehicleMetaData meta)
	{
		if (GetGame().IsClient())
		{
			DeleteVehicle(meta.m_NetworkIDLow, meta.m_NetworkIDHigh);
		}
		else
		{
			GetGame().ObjectDelete(meta.m_Entity);
			UpdateVehiclesMetaData_SP();
		}
	}

	void DeleteVehicle( int netLow, int netHigh )
	{
		if ( IsMissionClient() )
		{
			auto rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
			rpc.Send( NULL, JMVehiclesModuleRPC.DeleteVehicle, true );
		}
	}

	private void RPC_DeleteVehicle( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Vehicles.Delete", senderRPC ) )
			return;
		
		int netLow;
		if ( !ctx.Read( netLow ) )
			return;

		int netHigh;
		if ( !ctx.Read( netHigh ) )
			return;
		
		Object obj = GetGame().GetObjectByNetworkId( netLow, netHigh );

		if ( !obj )
			return;

		GetGame().ObjectDelete( obj );

		RPC_RequestServerVehicles( ctx, senderRPC, target );
	}

	#ifdef EXPANSIONMODVEHICLE
	private void RPC_DeleteVehicleUnclaimed( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Vehicles.Delete.Unclaimed", senderRPC ) )
			return;
		
		Exec_DeleteVehicleUnclaimed();

		RPC_RequestServerVehicles( ctx, senderRPC, target );
	}

	void Exec_DeleteVehicleUnclaimed()
	{
		auto node = CarScript.s_JM_AllCars.m_Head;
		while ( node )
		{
			if ( !node.m_Value.GetExpansionVehicle().HasKey() )
				node.m_Value.Delete();

			node = node.m_Next;
		}

		auto boats = BoatScript.s_JM_AllBoats.m_Head;
		while ( boats )
		{
			if ( !boats.m_Value.GetExpansionVehicle().HasKey() )
				boats.m_Value.Delete();

			boats = boats.m_Next;
		}

		auto vehicles = ExpansionVehicleBase.GetAll();
		foreach ( ExpansionVehicleBase vehicle: vehicles )
		{
			if ( !vehicle )
				continue;

			if ( vehicle.GetExpansionVehicle().HasKey() )
				continue;
			
			GetGame().ObjectDelete( vehicle );
		}

		auto cover = ExpansionVehicleCover.s_JM_AllCovers.m_Head;
		while ( cover )
		{
			auto keychain = ExpansionKeyChainBase.Cast(cover.m_Value.GetAttachmentByType(ExpansionKeyChainBase));
			if (!keychain || !keychain.Expansion_HasOwner())
				cover.m_Value.Delete();

			cover = cover.m_Next;
		}
	}
	#endif

	private void RPC_DeleteVehicleDestroyed( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Vehicles.Delete.Destroyed", senderRPC ) )
			return;

		Exec_DeleteVehicleDestroyed();

		RPC_RequestServerVehicles( ctx, senderRPC, target );
	}

	void Exec_DeleteVehicleDestroyed()
	{
		auto node = CarScript.s_JM_AllCars.m_Head;
		while ( node )
		{
			if ( node.m_Value.IsDamageDestroyed() )
				node.m_Value.Delete();

			node = node.m_Next;
		}

		auto boats = BoatScript.s_JM_AllBoats.m_Head;
		while ( boats )
		{
			if ( boats.m_Value.IsDamageDestroyed() )
				boats.m_Value.Delete();

			boats = boats.m_Next;
		}
		
		#ifdef EXPANSIONMODVEHICLE
		auto vehicles = ExpansionVehicleBase.GetAll();
		foreach ( ExpansionVehicleBase vehicle: vehicles )
		{
			if ( !vehicle )
				continue;

			if ( !vehicle.IsDamageDestroyed() )
				continue;

			GetGame().ObjectDelete( vehicle );
		}

		auto cover = ExpansionVehicleCover.s_JM_AllCovers.m_Head;
		while ( cover )
		{
			if ( cover.m_Value.IsDamageDestroyed() )
				cover.m_Value.Delete();

			cover = cover.m_Next;
		}
		#endif
	}

	private void RPC_DeleteVehicleAll( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Vehicles.Delete.All", senderRPC ) )
			return;
		
		Exec_DeleteVehicleAll();

		RPC_RequestServerVehicles( ctx, senderRPC, target );
	}

	void Exec_DeleteVehicleAll()
	{
		auto node = CarScript.s_JM_AllCars.m_Head;
		while ( node )
		{
			node.m_Value.Delete();
			node = node.m_Next;
		}

		auto boats = BoatScript.s_JM_AllBoats.m_Head;
		while ( boats )
		{
			boats.m_Value.Delete();
			boats = boats.m_Next;
		}
		
		#ifdef EXPANSIONMODVEHICLE
		auto vehicles = ExpansionVehicleBase.GetAll();
		foreach ( ExpansionVehicleBase vehicle: vehicles )
		{
			if ( !vehicle )
				return;

			GetGame().ObjectDelete( vehicle );
		}

		auto cover = ExpansionVehicleCover.s_JM_AllCovers.m_Head;
		while ( cover )
		{
			cover.m_Value.Delete();
			cover = cover.m_Next;
		}
		#endif
	}

	void RequestTeleportToVehicle(JMVehicleMetaData meta)
	{
		if (GetGame().IsClient())
		{
			RequestTeleportToVehicle(meta.m_NetworkIDLow, meta.m_NetworkIDHigh);
		}
		else
		{
			Exec_TeleportToVehicle(PlayerBase.Cast(GetGame().GetPlayer()), meta.m_Entity);
		}
	}

	void RequestTeleportToVehicle( int netLow, int netHigh )
	{
		if ( IsMissionClient() )
		{
			auto rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
 			rpc.Send( NULL, JMVehiclesModuleRPC.TeleportToVehicle, true );
		}
	}

	private void RPC_TeleportToVehicle( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Vehicles.Teleport", senderRPC, instance ) )
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

		Object obj = GetGame().GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		Exec_TeleportToVehicle(player, obj);
	}

	void Exec_TeleportToVehicle(PlayerBase player, Object obj)
	{
		vector pos = obj.GetPosition();
		vector minMax[2];
		obj.ClippingInfo( minMax );

		player.SetLastPosition();
		pos = pos + minMax[1];
		pos[1] = GetGame().SurfaceRoadY3D(pos[0], pos[1], pos[2], RoadSurfaceDetection.UNDER);
		player.SetWorldPosition( pos );
	}

	array< ref JMVehicleMetaData > GetServerVehicles()
	{
		return m_Vehicles;
	}
};
