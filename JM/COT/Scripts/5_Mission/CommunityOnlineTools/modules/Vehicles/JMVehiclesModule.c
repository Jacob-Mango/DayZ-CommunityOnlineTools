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

	string m_LastDriverUID;
	
	static JMVehicleMetaData CreateCarScript( CarScript car )
	{
		JMVehicleMetaData meta = new JMVehicleMetaData();

		car.GetNetworkID( meta.m_NetworkIDLow, meta.m_NetworkIDHigh );
		
		#ifdef EXPANSIONMODVEHICLE
		meta.m_PersistentIDA = car.GetPersistentIDA();
		meta.m_PersistentIDB = car.GetPersistentIDB();
		meta.m_PersistentIDC = car.GetPersistentIDC();
		meta.m_PersistentIDD = car.GetPersistentIDD();
		#else
		car.GetPersistentID(meta.m_PersistentIDA, meta.m_PersistentIDB, meta.m_PersistentIDC, meta.m_PersistentIDD);
		#endif

		meta.m_ClassName = car.ClassName();
		meta.m_Position = car.GetPosition();
		meta.m_Orientation = car.GetOrientation();

		meta.m_VehicleType = JMVT_NONE;
		#ifdef EXPANSIONMODVEHICLE
		if ( car.IsCar() )
		#endif
			meta.m_VehicleType |= JMVT_CAR;
		#ifdef EXPANSIONMODVEHICLE
		if ( car.IsBoat() )
			meta.m_VehicleType |= JMVT_BOAT;
		if ( car.IsHelicopter() )
			meta.m_VehicleType |= JMVT_HELICOPTER;
		if ( car.IsPlane() )
			meta.m_VehicleType |= JMVT_PLANE;
		#endif
		
		meta.m_DestructionType = JMDT_NONE;
		#ifdef EXPANSIONMODVEHICLE
		if ( car.IsExploded() )
			meta.m_DestructionType |= JMDT_EXPLODED;
		#endif
		if ( car.IsDamageDestroyed() )
			meta.m_DestructionType |= JMDT_DESTROYED;

		#ifdef EXPANSIONMODVEHICLE
		meta.m_HasKeys = car.HasKey();
		#endif

		#ifdef EXPANSIONMODCORE
		meta.m_LastDriverUID = car.ExpansionGetLastDriverUID();
		#endif

		return meta;
	}
	
	#ifdef EXPANSIONMODVEHICLE
	static JMVehicleMetaData CreateVehicle( ExpansionVehicleBase vehicle )
	{
		JMVehicleMetaData meta = new JMVehicleMetaData();

		vehicle.GetNetworkID( meta.m_NetworkIDLow, meta.m_NetworkIDHigh );
		
		meta.m_PersistentIDA = vehicle.GetPersistentIDA();
		meta.m_PersistentIDB = vehicle.GetPersistentIDB();
		meta.m_PersistentIDC = vehicle.GetPersistentIDC();
		meta.m_PersistentIDD = vehicle.GetPersistentIDD();

		meta.m_ClassName = vehicle.ClassName();
		meta.m_Position = vehicle.GetPosition();
		meta.m_Orientation = vehicle.GetOrientation();

		meta.m_VehicleType = JMVT_NONE;
		if ( vehicle.IsCar() )
			meta.m_VehicleType |= JMVT_CAR;
		if ( vehicle.IsBoat() )
			meta.m_VehicleType |= JMVT_BOAT;
		if ( vehicle.IsHelicopter() )
			meta.m_VehicleType |= JMVT_HELICOPTER;
		if ( vehicle.IsPlane() )
			meta.m_VehicleType |= JMVT_PLANE;
		
		meta.m_DestructionType = JMDT_NONE;
		if ( vehicle.IsExploded() )
			meta.m_DestructionType |= JMDT_EXPLODED;
		if ( vehicle.IsDamageDestroyed() )
			meta.m_DestructionType |= JMDT_DESTROYED;

		meta.m_HasKeys = vehicle.HasKey();
		meta.m_LastDriverUID = vehicle.ExpansionGetLastDriverUID();
		

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
};

class JMVehiclesModule: JMRenderableModuleBase
{
	private ref array<ref JMVehicleMetaData> m_Vehicles;
	private int m_TimeSinceLastChecked;

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
		return "V";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();
	}

	private void UpdateVehiclesMetaData()
	{
		if ( m_TimeSinceLastChecked + 500 > GetGame().GetTime() )
			return;

		m_TimeSinceLastChecked = GetGame().GetTime();

		if ( m_Vehicles.Count() >= 0 )
			m_Vehicles.Clear();

		int i;
		
		auto node = CarScript.s_JM_AllCars.m_Head;
		while ( node )
		{
			m_Vehicles.Insert( JMVehicleMetaData.CreateCarScript( node.m_Value ) );
			node = node.m_Next;
		}
		
		#ifdef EXPANSIONMODVEHICLE
		for ( i = 0; i < ExpansionVehicleBase.GetAll().Count(); i++ )
		{
			ExpansionVehicleBase vehicle = ExpansionVehicleBase.GetAll()[i];
			if ( !vehicle ) // should not be possible
				continue;

			m_Vehicles.Insert( JMVehicleMetaData.CreateVehicle( vehicle ) );
		}
		#endif
	}

	override int GetRPCMin()
	{
		return JMVehiclesModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMVehiclesModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ref ParamsReadContext ctx )
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
		if ( IsMissionClient() )
		{
			auto rpc = new ScriptRPC();
 			rpc.Send( NULL, JMVehiclesModuleRPC.RequestServerVehicles, true );
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
		rpc.Write( m_Vehicles );
		rpc.Send( NULL, JMVehiclesModuleRPC.SendServerVehicles, true, senderRPC );
	}

	private void RPC_SendServerVehicles( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionClient() )
			return;
		
		ctx.Read( m_Vehicles );
	}

	void DeleteVehicleUnclaimed( )
	{
		if ( IsMissionClient() )
		{
			auto rpc = new ScriptRPC();
			
			rpc.Send( NULL, JMVehiclesModuleRPC.DeleteVehicleUnclaimed, true );
		}
	}

	void DeleteVehicleDestroyed( )
	{
		if ( IsMissionClient() )
		{
			auto rpc = new ScriptRPC();
			
			rpc.Send( NULL, JMVehiclesModuleRPC.DeleteVehicleDestroyed, true );
		}
	}

	void DeleteVehicleAll( )
	{
		if ( IsMissionClient() )
		{
			auto rpc = new ScriptRPC();
			
			rpc.Send( NULL, JMVehiclesModuleRPC.DeleteVehicleAll, true );
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
	}

	#ifdef EXPANSIONMODVEHICLE
	private void RPC_DeleteVehicleUnclaimed( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Vehicles.Delete.Unclaimed", senderRPC ) )
			return;

		int i;
		
		auto node = CarScript.s_JM_AllCars.m_Head;
		while ( node )
		{
			if ( !node.m_Value.HasKey() )
				node.m_Value.Delete();

			node = node.m_Next;
		}

		for ( i = 0; i < ExpansionVehicleBase.GetAll().Count(); i++ )
		{
			ExpansionVehicleBase vehicle = ExpansionVehicleBase.GetAll()[i];
			if ( !vehicle )
				continue;

			if ( vehicle.HasKey() )
				continue;
			
			GetGame().ObjectDelete( vehicle );
		}
	}
	#endif

	private void RPC_DeleteVehicleDestroyed( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Vehicles.Delete.Destroyed", senderRPC ) )
			return;

		int i;
		
		auto node = CarScript.s_JM_AllCars.m_Head;
		while ( node )
		{
			if ( node.m_Value.IsDamageDestroyed() )
				node.m_Value.Delete();

			node = node.m_Next;
		}
		
		#ifdef EXPANSIONMODVEHICLE
		for ( i = 0; i < ExpansionVehicleBase.GetAll().Count(); i++ )
		{
			ExpansionVehicleBase vehicle = ExpansionVehicleBase.GetAll()[i];
			if ( !vehicle )
				continue;

			if ( !vehicle.IsDamageDestroyed() )
				continue;

			GetGame().ObjectDelete( vehicle );
		}
		#endif
	}

	private void RPC_DeleteVehicleAll( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Vehicles.Delete.All", senderRPC ) )
			return;

		int i;
		
		auto node = CarScript.s_JM_AllCars.m_Head;
		while ( node )
		{
			node.m_Value.Delete();
			node = node.m_Next;
		}
		
		#ifdef EXPANSIONMODVEHICLE
		for ( i = 0; i < ExpansionVehicleBase.GetAll().Count(); i++ )
		{
			ExpansionVehicleBase vehicle = ExpansionVehicleBase.GetAll()[i];
			if ( !vehicle )
				return;

			GetGame().ObjectDelete( vehicle );
		}
		#endif
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

		vector pos = obj.GetPosition();
		vector minMax[2];
		obj.ClippingInfo( minMax );

		player.SetLastPosition();
		player.SetWorldPosition( pos + minMax[1] );
	}

	array< ref JMVehicleMetaData > GetServerVehicles()
	{
		return m_Vehicles;
	}
};
