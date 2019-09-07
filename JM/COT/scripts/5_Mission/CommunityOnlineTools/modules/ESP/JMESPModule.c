class JMESPModule: JMRenderableModuleBase
{
	protected const int m_UserIDStart = 10000;

	protected ref array< ref JMObjectMeta > m_ESPObjects;
	protected ref array< ref JMESPWidget > m_ESPBoxes;

	bool CanViewPlayers;
	bool CanViewBaseBuilding;
	bool CanViewVehicles;
	bool CanViewItems;
	bool CanViewInfected;
	bool CanViewCreature;
	bool CanViewEverything;

	protected int m_UserID;

	string Filter;

	bool ViewPlayers;
	bool ViewBaseBuilding;
	bool ViewVehicles;
	bool ViewItems;
	bool ViewInfected;
	bool ViewCreature;
	bool ViewEverything;

	float ESPRadius;

	string Name;

	vector Position;
	vector Rotation;

	float MaxHealth;
	float Health;

	float ESPUpdateTime;
	bool ESPIsUpdating;

	bool IsShowing;

	void JMESPModule()
	{
		m_ESPObjects = new array< ref JMObjectMeta >;
		m_ESPBoxes = new array< ref JMESPWidget >;
		m_UserID = m_UserIDStart;

		ESPRadius = 200;

		Position = "0 0 0";
		Rotation = "0 0 0";

		ESPUpdateTime = 0.5;
		IsShowing = false;

		GetRPCManager().AddRPC( "COT_ESP", "ESPLog", this, SingeplayerExecutionType.Server );

		GetRPCManager().AddRPC( "COT_ESP", "RequestFullMapESP", this, SingeplayerExecutionType.Both );

		GetRPCManager().AddRPC( "COT_ESP", "RequestPlayerESPData", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_ESP", "ShowPlayerESPData", this, SingeplayerExecutionType.Client );

		GetRPCManager().AddRPC( "COT_ESP", "ServerDeleteSelected", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_ESP", "ServerSetSelected", this, SingeplayerExecutionType.Server );

		GetPermissionsManager().RegisterPermission( "ESP.Manipulation.Set" );
		GetPermissionsManager().RegisterPermission( "ESP.Manipulation.Delete" );

		GetPermissionsManager().RegisterPermission( "ESP.View.Player" );
		GetPermissionsManager().RegisterPermission( "ESP.View.BaseBuilding" );
		GetPermissionsManager().RegisterPermission( "ESP.View.Vehicles" );
		GetPermissionsManager().RegisterPermission( "ESP.View.Items" );
		GetPermissionsManager().RegisterPermission( "ESP.View.Infected" );
		GetPermissionsManager().RegisterPermission( "ESP.View.Creature" );
		GetPermissionsManager().RegisterPermission( "ESP.View" );
	}

	void ~JMESPModule()
	{
		Hide();
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "ESP.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/esp_form.layout";
	}

	override void OnClientPermissionsUpdated()
	{
		CanViewPlayers = GetPermissionsManager().HasPermission( "ESP.View.Player" );
		CanViewBaseBuilding = GetPermissionsManager().HasPermission( "ESP.View.BaseBuilding" );
		CanViewVehicles = GetPermissionsManager().HasPermission( "ESP.View.Vehicles" );
		CanViewItems = GetPermissionsManager().HasPermission( "ESP.View.Items" );
		CanViewInfected = GetPermissionsManager().HasPermission( "ESP.View.Infected" );
		CanViewCreature = GetPermissionsManager().HasPermission( "ESP.View.Creature" );
		CanViewEverything = GetPermissionsManager().HasPermission( "ESP.View" );

		if ( JMESPForm.Cast( form ) )
		{
			JMESPForm.Cast( form ).DisableToggleableOptions();
		}
	}

	override void OnMissionStart()
	{
		JMESPWidget.espModule = this;
	}

	int GetSelectedCount()
	{
		return 0;
	}

	void ServerDeleteSelected( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "ESP.Manipulation.Delete", sender ) )
			return;

		ref Param1< ref array< Object > > data;
		if ( !ctx.Read( data ) ) return;

		ref array< Object > copy = new array< Object >;
		copy.Copy( data.param1 );
		
		if ( type == CallType.Server )
		{
			for ( int i = 0; i < copy.Count(); i++ )
			{
				if ( copy[i] == NULL ) continue;

				string obtype;
				GetGame().ObjectGetType( copy[i], obtype );

				GetCommunityOnlineToolsBase().Log( sender, "Deleted object " + copy[i].GetDisplayName() + " (" + obtype + ") at " + copy[i].GetPosition() );
				GetGame().ObjectDelete( copy[i] );
			}
		}

		delete copy;
	}

	void ServerSetSelected( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "ESP.Manipulation.Set", sender ) )
			return;

		ref Param3< vector, vector, float > data;
		if ( !ctx.Read( data ) ) return;
		
		if ( type == CallType.Server )
		{
			if ( target == NULL ) return;

			string obtype;
			GetGame().ObjectGetType( target, obtype );

			GetCommunityOnlineToolsBase().Log( sender, "Set object " + target.GetDisplayName() + " (" + obtype + ") Position [" + target.GetPosition() + "] -> [" + data.param1 + "] Rotation [" + target.GetYawPitchRoll() + "] -> [" + data.param2 +  "] Health [" + target.GetHealth( "", "" ) + "] -> [" + data.param3 +  "]" );

			target.SetPosition( data.param1 );
			target.SetYawPitchRoll( data.param2 );
			target.SetHealth( "", "", data.param3 );
		}
	}

	void RequestFullMapESP( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "ESP.Manipulation.Delete", sender ) )
			return;

		if ( type == CallType.Server )
		{
			PlayerBase player = GetPlayerObjectByIdentity( sender );
			if ( !player ) return;

			GetCommunityOnlineToolsBase().Log( sender, "Entering Full Map ESP" );
			GetGame().ObjectDelete( player );
			
			GetRPCManager().SendRPC( "COT_ESP", "RequestFullMapESP", new Param );
		}
		
		if ( type == CallType.Client )
		{
			COTPlayerIsRemoved = true;
		}
	}

	void EnableFullMap()
	{
		if ( CurrentActiveCamera == NULL ) return;

		GetRPCManager().SendRPC( "COT_ESP", "RequestFullMapESP", new Param );
	}

	override void OnMissionFinish()
	{
		HideESP();
	}

	void HideESP()
	{
		IsShowing = false;

		for (int j = 0; j < m_ESPBoxes.Count(); j++ )
		{
			m_ESPBoxes[j].Unlink();
		}

		m_ESPBoxes.Clear();

		m_ESPObjects.Clear();

		m_UserID = m_UserIDStart;
	}

	void UpdateESP()
	{
		HideESP();

		IsShowing = true;

		array<Object> objects = new array<Object>;
		GetGame().GetObjectsAtPosition( GetCurrentPosition(), ESPRadius, objects, NULL );

		Object obj;
		EntityAI entity;
		JMObjectMeta espInfo;

		bool isUsingFilter = false;

		if ( Filter != "" ) 
		{
			isUsingFilter = true;
		}

		string filter = Filter + "";
		filter.ToLower();

		for (int i = 0; i < objects.Count(); ++i)
		{
			obj = Object.Cast( objects.Get(i) );
			entity = EntityAI.Cast( obj );
			
			if ( obj == NULL )
				continue;

			string type = obj.GetType();
			type.ToLower();

			if ( isUsingFilter && !type.Contains( filter ) )
				continue;
				
			if ( !obj.HasNetworkID() )
				continue;
			if ( obj.IsRock() )
				continue;
			if ( obj.IsWoodBase() )
				continue;
			if ( obj.IsBush() )
				continue;
			if ( obj.IsTree() )
				continue;
			if ( obj.IsBuilding() && !obj.IsInherited( GardenBase ) )
				continue;

			entity = EntityAI.Cast( obj );

			bool isPlayer = false;
			
			if ( entity != NULL )
			{
				isPlayer = entity.IsPlayer();

				Man man = Man.Cast( entity );
			
				if ( (ViewPlayers || ViewEverything) && (CanViewPlayers || CanViewEverything) && isPlayer )
				{
					espInfo = new JMObjectMeta;

					espInfo.name = obj.GetDisplayName();
					if ( espInfo.name == "" )
						espInfo.name = obj.GetType();

					espInfo.type = JMESPType.PLAYER;
					
					if ( man && man.GetIdentity() )
					{
						PlayerIdentity identity = man.GetIdentity();
						espInfo.player = GetPermissionsManager().GetPlayerByIdentity( identity );
						espInfo.name = identity.GetName();
					}

					espInfo.target = obj;

					CreateESPBox( espInfo );
					continue;
				}

				bool isInfected = !isPlayer && ( entity.IsZombie() || entity.IsZombieMilitary() );
				if ( (ViewInfected || ViewEverything) && (CanViewInfected || CanViewEverything) && isInfected )
				{
					espInfo = new JMObjectMeta;

					espInfo.name = obj.GetDisplayName();
					if ( espInfo.name == "" )
						espInfo.name = obj.GetType();

					espInfo.target = obj;
					espInfo.type = JMESPType.INFECTED;

					CreateESPBox( espInfo );
					continue;
				} 

				bool isCreature = !isInfected && entity.IsAnimal();
				if ( (ViewCreature || ViewEverything) && (CanViewCreature || CanViewEverything) && isCreature )
				{
					espInfo = new JMObjectMeta;

					espInfo.name = obj.GetDisplayName();
					if ( espInfo.name == "" )
						espInfo.name = obj.GetType();

					espInfo.target = obj;
					espInfo.type = JMESPType.CREATURE;

					CreateESPBox( espInfo );
					continue;
				}
			}

			bool isTransport = !isPlayer && obj.IsTransport();
			if ( (ViewVehicles || ViewEverything) && (CanViewVehicles || CanViewEverything) && isTransport )
			{
				espInfo = new JMObjectMeta;
				
				espInfo.name = obj.GetDisplayName();
				if ( espInfo.name == "" )
					espInfo.name = obj.GetType();

				espInfo.target = obj;
				espInfo.type = JMESPType.VEHICLE;

				CreateESPBox( espInfo );
				continue;
			}

			bool isBaseBuilding = !isTransport && ( obj.IsContainer() || obj.CanUseConstruction() || obj.IsFireplace() || obj.IsInherited( GardenBase ) );
			if ( (ViewBaseBuilding || ViewEverything) && (CanViewBaseBuilding || CanViewEverything) && isBaseBuilding )
			{
				espInfo = new JMObjectMeta;
				
				espInfo.name = obj.GetDisplayName();
				if ( espInfo.name == "" )
					espInfo.name = obj.GetType();

				espInfo.target = obj;
				espInfo.type = JMESPType.BASEBUILDING;

				CreateESPBox( espInfo );
				continue;
			}

			bool isItem = !isBaseBuilding && ( obj.IsItemBase() || obj.IsInventoryItem() );
			if ( (ViewItems || ViewEverything) && (CanViewItems || CanViewEverything) && isItem )
			{
				espInfo = new JMObjectMeta;
				
				espInfo.name = obj.GetDisplayName();
				if ( espInfo.name == "" )
					espInfo.name = obj.GetType();

				espInfo.target = obj;
				espInfo.type = JMESPType.ITEM;

				CreateESPBox( espInfo );
				continue;
			}

			/*
			if ( ViewEverything && CanViewEverything )
			{
				espInfo = new JMObjectMeta;
				
				espInfo.name = obj.GetDisplayName();
				if ( espInfo.name == "" )
					espInfo.name = obj.GetType();

				espInfo.target = obj;
				espInfo.type = JMESPType.ALL;

				CreateESPBox( espInfo );
				continue;
			} 
			*/
		}

		objects.Clear();
		delete objects;

		if ( ESPIsUpdating )
		{
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( this.UpdateESP, ESPUpdateTime * 1000.0 );
		} else
		{
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( this.UpdateESP );
		}
	}

	void ESPUpdateLoop( bool isDoing )
	{
		ESPIsUpdating = isDoing;

		if ( ESPIsUpdating )
		{
			if ( IsShowing )
			{
				GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( this.UpdateESP, ESPUpdateTime * 1000.0 );
			} else
			{
				GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( this.UpdateESP );
			}
		} else 
		{
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( this.UpdateESP );
		}
	}

	void ESPLog( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		ref Param1< string > data;
		if ( !ctx.Read( data ) ) return;
		
		if ( type == CallType.Server )
		{
			GetCommunityOnlineToolsBase().Log( sender, "ESP " + data.param1 );
		}
	}

	void CreateESPBox( ref JMObjectMeta info )
	{
		ref JMESPWidget boxScript = NULL;
		ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/esp_widget.layout" );

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