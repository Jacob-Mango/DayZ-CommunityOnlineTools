class ESPModule: EditorModule
{
	protected ref array< ref ESPInfo > m_ESPObjects;

	protected float m_ESPRadius;

	protected bool m_CanViewPlayers;
	protected bool m_CanViewBaseBuilding;
	protected bool m_CanViewVehicles;

	bool ViewPlayers;
	bool ViewBaseBuilding;
	bool ViewVehicles;

	void ESPModule()
	{
		m_ESPObjects = new ref array< ref ESPInfo >;
		m_ESPRadius = 200;

		GetRPCManager().AddRPC( "COT_ESP", "RequestESPData", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_ESP", "ShowESPData", this, SingeplayerExecutionType.Client );

		GetPermissionsManager().RegisterPermission( "ESP.View.Player" );
		GetPermissionsManager().RegisterPermission( "ESP.View.BaseBuilding" );
		GetPermissionsManager().RegisterPermission( "ESP.View.Vehicles" );
		GetPermissionsManager().RegisterPermission( "ESP.View" );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "ESP.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/gui/layouts/ESP/ESPMenu.layout";
	}

	override void OnClientPermissionsUpdated()
	{
		m_CanViewPlayers = GetPermissionsManager().HasPermission( "ESP.View.Player" );
		m_CanViewBaseBuilding = GetPermissionsManager().HasPermission( "ESP.View.BaseBuilding" );
		m_CanViewVehicles = GetPermissionsManager().HasPermission( "ESP.View.Vehicles" );
	}

	void UpdateESP()
	{
		m_ESPObjects.Clear();

		ref array<Object> objects = new ref array<Object>;
		GetGame().GetObjectsAtPosition3D( GetCurrentPosition(), m_ESPRadius, objects, NULL );

		Object currentObj;

		for (int i = 0; i < objects.Count(); ++i)
		{
			currentObj = Object.Cast( objects.Get(i) );

			if ( ViewPlayers && m_CanViewPlayers && GetGame().ObjectIsKindOf( currentObj, "SurvivorBase" ) )
			{
				PlayerBase player = PlayerBase.Cast( currentObj );

				if ( player )
				{
					GetRPCManager().SendRPC( "COT_ESP", "RequestESPData", new Param1< string >( "Player" ), false, NULL, currentObj );
				}
			} else if ( ViewVehicles && m_CanViewVehicles && GetGame().ObjectIsKindOf( currentObj, "Transport" ) )
			{
				GetRPCManager().SendRPC( "COT_ESP", "RequestESPData", new Param1< string >( "Vehicles" ), false, NULL, currentObj );
			} else if ( ViewBaseBuilding && m_CanViewBaseBuilding && ( GetGame().ObjectIsKindOf( currentObj, "BaseBuildingBase" ) || GetGame().ObjectIsKindOf( currentObj, "Barrel_ColorBase" ) || GetGame().ObjectIsKindOf( currentObj, "Tent_Base" ) ) )
			{
				GetRPCManager().SendRPC( "COT_ESP", "RequestESPData", new Param1< string >( "BaseBuilding" ), false, NULL, currentObj );
			} 
		}

		objects.Clear();
		delete objects;
	}

	void RequestESPData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		ref Param1< string > data;
		if ( !ctx.Read( data ) ) return;

		if ( data.param1 == "" || !GetPermissionsManager().HasPermission( "ESP.View." + data.param1, sender ) )
			return;
		
		if( type == CallType.Server )
		{
			ESPInfoMetaData metadata = new ESPInfoMetaData;

			if ( data.param1 == "Player" )
			{
				PlayerBase player = PlayerBase.Cast( target );
				if ( !player ) return;

				metadata.isPlayer = true;
				metadata.steamid = player.GetIdentity().GetPlainId();
			} else if ( target != NULL )
			{
				metadata.name = target.GetDisplayName();
				metadata.isPlayer = false;
			} else {
				return;
			}

			GetRPCManager().SendRPC( "COT_ESP", "RequestESPData", new Param1< ESPInfoMetaData >( metadata ), false, sender, target );
		}
	}

	void ShowESPData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		ref Param1< ESPInfoMetaData > data;
		if ( !ctx.Read( data ) ) return;
		
		if( type == CallType.Client )
		{
			ref ESPInfo info = new ref ESPInfo;
			info.name = data.param1.name;
			info.target = target;

			if ( data.param1.isPlayer )
			{
				ref AuthPlayer player = GetPlayerForID( data.param1.steamid );
				if ( player )
				{
					info.isPlayer = true;
					info.player = player;
				}
			}

			m_ESPObjects.Insert( info );
		}
	}
}