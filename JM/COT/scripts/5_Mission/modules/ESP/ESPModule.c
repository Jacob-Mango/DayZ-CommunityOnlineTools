class ESPModule: EditorModule
{
	protected ref array< ref ESPInfo > m_ESPObjects;
	protected ref array< ref ESPBox > m_ESPBoxes;

	protected bool m_CanViewPlayers;
	protected bool m_CanViewBaseBuilding;
	protected bool m_CanViewVehicles;
	protected bool m_CanViewItems;

	protected int m_UserID;

	bool ViewPlayers;
	bool ViewBaseBuilding;
	bool ViewVehicles;
	bool ViewItems;

	float ESPRadius;

	void ESPModule()
	{
		m_ESPObjects = new ref array< ref ESPInfo >;
		m_ESPBoxes = new ref array< ref ESPBox >;
		m_UserID = 0;

		ESPRadius = 200;

		GetRPCManager().AddRPC( "COT_ESP", "RequestESPData", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_ESP", "ShowESPData", this, SingeplayerExecutionType.Client );

		GetPermissionsManager().RegisterPermission( "ESP.View.Player" );
		GetPermissionsManager().RegisterPermission( "ESP.View.BaseBuilding" );
		GetPermissionsManager().RegisterPermission( "ESP.View.Vehicles" );
		GetPermissionsManager().RegisterPermission( "ESP.View.Items" );
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
		m_CanViewItems = GetPermissionsManager().HasPermission( "ESP.View.Items" );
	}

	override void OnMissionStart()
	{
		ESPBox.espModule = this;
	}

	void HideESP()
	{
		for (int j = 0; j < m_ESPBoxes.Count(); j++ )
		{
			m_ESPBoxes[j].Unlink();
		}

		m_ESPBoxes.Clear();

		m_ESPObjects.Clear();

		m_UserID = 0;
	}

	void UpdateESP()
	{
		HideESP();

		ref array<Object> objects = new ref array<Object>;
		GetGame().GetObjectsAtPosition3D( GetCurrentPosition(), ESPRadius, objects, NULL );

		Object currentObj;
		PlayerBase player;
		ref ESPInfo espInfo;

		for (int i = 0; i < objects.Count(); ++i)
		{
			currentObj = Object.Cast( objects.Get(i) );

			if ( !currentObj.HasNetworkID() )
			{
				continue;
			}

			player = PlayerBase.Cast( currentObj );

			bool isPlayer = player != NULL;
			
			if ( ViewPlayers && m_CanViewPlayers && isPlayer )
			{
				if ( player )
				{
					GetRPCManager().SendRPC( "COT_ESP", "RequestESPData", new Param1< string >( "Player" ), false, NULL, currentObj );
				}
				continue;
			}
			
			bool isTransport = !isPlayer && GetGame().ObjectIsKindOf( currentObj, "Transport" );
			if ( ViewVehicles && m_CanViewVehicles && isTransport )
			{
				espInfo = new ref ESPInfo;
				espInfo.name = currentObj.GetDisplayName();
				espInfo.target = currentObj;
				espInfo.isPlayer = false;

				CreateESPBox( espInfo );
				continue;
			}

			bool isBaseBuilding = !isTransport && ( GetGame().ObjectIsKindOf( currentObj, "BaseBuildingBase" ) || GetGame().ObjectIsKindOf( currentObj, "Barrel_ColorBase" ) || GetGame().ObjectIsKindOf( currentObj, "Tent_Base" ) );
			if ( ViewBaseBuilding && m_CanViewBaseBuilding && isBaseBuilding )
			{
				espInfo = new ref ESPInfo;
				espInfo.name = currentObj.GetDisplayName();
				espInfo.target = currentObj;
				espInfo.isPlayer = false;

				CreateESPBox( espInfo );
				continue;
			}

			bool isItem = !isBaseBuilding && GetGame().ObjectIsKindOf( currentObj, "Inventory_Base" );
			if ( ViewItems && m_CanViewItems && isItem )
			{
				espInfo = new ref ESPInfo;
				espInfo.name = currentObj.GetDisplayName();
				espInfo.target = currentObj;
				espInfo.isPlayer = false;

				CreateESPBox( espInfo );
				continue;
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
				metadata.name = player.GetIdentity().GetName();
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
			info.isPlayer = false;

			if ( data.param1.isPlayer )
			{
				ref AuthPlayer player = GetPlayerForID( data.param1.steamid );
				if ( player )
				{
					info.isPlayer = true;
					info.player = player;
				}
			}

			CreateESPBox( info );
		}
	}

	void CreateESPBox( ref ESPInfo info )
	{
		ref ESPBox boxScript = NULL;
		ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/gui/layouts/ESP/ESPBox.layout" );

		if ( widget == NULL ) return;

		m_UserID++;

		widget.SetUserID( m_UserID );
		widget.GetScript( boxScript );

		if ( boxScript == NULL ) return;

		boxScript.SetInfo( info );

		m_ESPBoxes.Insert( boxScript );
		m_ESPObjects.Insert( info );
	}
}