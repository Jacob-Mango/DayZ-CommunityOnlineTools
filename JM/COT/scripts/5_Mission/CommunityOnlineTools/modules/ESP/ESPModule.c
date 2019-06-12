class ESPModule: EditorModule
{
	protected const int m_UserIDStart = 10000;

	protected ref array< ref ESPInfo > m_ESPObjects;
	protected ref array< ref ESPRenderBox > m_ESPBoxes;
	protected ref array< ref ESPRenderBox > m_SelectedBoxes;

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

	void ESPModule()
	{
		m_ESPObjects = new ref array< ref ESPInfo >;
		m_ESPBoxes = new ref array< ref ESPRenderBox >;
		m_SelectedBoxes = new ref array< ref ESPRenderBox >;
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

	void ~ESPModule()
	{
		Hide();
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "ESP.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/ESP/ESPMenu.layout";
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

		if ( ESPMenu.Cast( form ) )
		{
			ESPMenu.Cast( form ).DisableToggleableOptions();
		}
	}

	override void OnMissionStart()
	{
		ESPRenderBox.espModule = this;
	}

	int GetSelectedCount()
	{
		return m_SelectedBoxes.Count();
	}

	void ServerDeleteSelected( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "ESP.Manipulation.Delete", sender ) )
			return;

		ref Param1< ref array< Object > > data;
		if ( !ctx.Read( data ) ) return;

		ref array< Object > copy = new ref array< Object >;
		copy.Copy( data.param1 );
		
		if ( type == CallType.Server )
		{
			for ( int i = 0; i < copy.Count(); i++ )
			{
				if ( copy[i] == NULL ) continue;

				string obtype;
				GetGame().ObjectGetType( copy[i], obtype );

				COTLog( sender, "Deleted object " + copy[i].GetDisplayName() + " (" + obtype + ") at " + copy[i].GetPosition() );
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

			COTLog( sender, "Set object " + target.GetDisplayName() + " (" + obtype + ") Position [" + target.GetPosition() + "] -> [" + data.param1 + "] Rotation [" + target.GetYawPitchRoll() + "] -> [" + data.param2 +  "] Health [" + target.GetHealth( "", "" ) + "] -> [" + data.param3 +  "]" );

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

			COTLog( sender, "Entering Full Map ESP" );
			GetGame().ObjectDelete( player );
			
			GetRPCManager().SendRPC( "COT_ESP", "RequestFullMapESP", new Param );
		}
		
		if ( type == CallType.Client )
		{
			COTPlayerIsRemoved = true;
		}
	}

	void SetSelected()
	{
		if ( m_SelectedBoxes.Count() > 0 ) 
		{
			GetRPCManager().SendRPC( "COT_ESP", "ServerSetSelected", new Param3< vector, vector, float >( Position, Rotation, Health ), false, NULL, m_SelectedBoxes[0].Info.target );
		}
	}

	void DeleteSelected()
	{
		array< Object > objects = new array< Object >;

		for (int j = 0; j < m_SelectedBoxes.Count(); j++ )
		{
			objects.Insert( m_SelectedBoxes[j].Info.target );

			m_ESPBoxes.RemoveItem( m_SelectedBoxes[j] );
			m_ESPObjects.RemoveItem( m_SelectedBoxes[j].Info );

			m_SelectedBoxes[j].Unlink();
		}

		GetRPCManager().SendRPC( "COT_ESP", "ServerDeleteSelected", new Param1< ref array< Object > >( objects ) );

		m_SelectedBoxes.Clear();
		delete objects;
	}

	void EnableFullMap()
	{
		if ( CurrentActiveCamera == NULL ) return;

		GetRPCManager().SendRPC( "COT_ESP", "RequestFullMapESP", new Param );
	}

	void SelectBox( ref ESPRenderBox box, bool checked, bool button )
	{
		int existsAt = m_SelectedBoxes.Find( box );

		if ( button )
		{
			for (int j = 0; j < m_SelectedBoxes.Count(); j++ )
			{
				m_SelectedBoxes[j].Checkbox.SetChecked( false );
			}

			m_SelectedBoxes.Clear();

			box.Checkbox.SetChecked( true );
			m_SelectedBoxes.Insert( box );
		} else
		{
			box.Checkbox.SetChecked( checked );
			if ( checked )
			{
				m_SelectedBoxes.Insert( box );
			} else if ( existsAt >= 0 )
			{
				m_SelectedBoxes.Remove( existsAt );
			}
		}

		if ( m_SelectedBoxes.Count() > 0 && m_SelectedBoxes[0].Info.target != NULL )
		{
			Name = m_SelectedBoxes[0].Info.target.GetType();
			Position = m_SelectedBoxes[0].Info.target.GetPosition();
			Rotation = m_SelectedBoxes[0].Info.target.GetYawPitchRoll();
			MaxHealth = m_SelectedBoxes[0].Info.target.GetMaxHealth( "", "" );
			Health = m_SelectedBoxes[0].Info.target.GetHealth( "", "" );
		}
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

		m_SelectedBoxes.Clear();

		m_ESPObjects.Clear();

		m_UserID = m_UserIDStart;
	}

	void UpdateESP()
	{
		HideESP();

		IsShowing = true;

		ref array<Object> objects = new ref array<Object>;
		GetGame().GetObjectsAtPosition( GetCurrentPosition(), ESPRadius, objects, NULL );

		Object obj;
		EntityAI entity;
		ref ESPInfo espInfo;

		bool isUsingFilter = false;

		if ( Filter != "" ) 
			isUsingFilter = true;

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
			
				if ( (ViewPlayers || ViewEverything) && (CanViewPlayers || CanViewEverything) && isPlayer )
				{
					GetRPCManager().SendRPC( "COT_ESP", "RequestPlayerESPData", new Param, false, NULL, obj );
					continue;
				}

				bool isInfected = !isPlayer && ( entity.IsZombie() || entity.IsZombieMilitary() );
				if ( (ViewInfected || ViewEverything) && (CanViewInfected || CanViewEverything) && isInfected )
				{
					espInfo = new ref ESPInfo;

					espInfo.name = obj.GetDisplayName();
					if ( espInfo.name == "" )
						espInfo.name = obj.GetType();

					espInfo.target = obj;
					espInfo.type = ESPType.INFECTED;

					CreateESPBox( espInfo );
					continue;
				} 

				bool isCreature = !isInfected && entity.IsAnimal();
				if ( (ViewCreature || ViewEverything) && (CanViewCreature || CanViewEverything) && isCreature )
				{
					espInfo = new ref ESPInfo;

					espInfo.name = obj.GetDisplayName();
					if ( espInfo.name == "" )
						espInfo.name = obj.GetType();

					espInfo.target = obj;
					espInfo.type = ESPType.CREATURE;

					CreateESPBox( espInfo );
					continue;
				}
			}

			bool isTransport = !isPlayer && obj.IsTransport();
			if ( (ViewVehicles || ViewEverything) && (CanViewVehicles || CanViewEverything) && isTransport )
			{
				espInfo = new ref ESPInfo;
				
				espInfo.name = obj.GetDisplayName();
				if ( espInfo.name == "" )
					espInfo.name = obj.GetType();

				espInfo.target = obj;
				espInfo.type = ESPType.VEHICLE;

				CreateESPBox( espInfo );
				continue;
			}

			bool isBaseBuilding = !isTransport && ( obj.IsContainer() || obj.CanUseConstruction() || obj.IsFireplace() || obj.IsInherited( GardenBase ) );
			if ( (ViewBaseBuilding || ViewEverything) && (CanViewBaseBuilding || CanViewEverything) && isBaseBuilding )
			{
				espInfo = new ref ESPInfo;
				
				espInfo.name = obj.GetDisplayName();
				if ( espInfo.name == "" )
					espInfo.name = obj.GetType();

				espInfo.target = obj;
				espInfo.type = ESPType.BASEBUILDING;

				CreateESPBox( espInfo );
				continue;
			}

			bool isItem = !isBaseBuilding && ( obj.IsItemBase() || obj.IsInventoryItem() );
			if ( (ViewItems || ViewEverything) && (CanViewItems || CanViewEverything) && isItem )
			{
				espInfo = new ref ESPInfo;
				
				espInfo.name = obj.GetDisplayName();
				if ( espInfo.name == "" )
					espInfo.name = obj.GetType();

				espInfo.target = obj;
				espInfo.type = ESPType.ITEM;

				CreateESPBox( espInfo );
				continue;
			}

			/*
			if ( ViewEverything && CanViewEverything )
			{
				espInfo = new ref ESPInfo;
				
				espInfo.name = obj.GetDisplayName();
				if ( espInfo.name == "" )
					espInfo.name = obj.GetType();

				espInfo.target = obj;
				espInfo.type = ESPType.ALL;

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

	void RequestPlayerESPData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "ESP.View.Player", sender ) )
			return;
		
		if ( type == CallType.Server )
		{
			ESPInfoMetaData metadata = new ESPInfoMetaData;

			PlayerBase player = PlayerBase.Cast( target );
			if ( !player ) return;
			if ( !player.GetIdentity() ) return;

			metadata.isPlayer = true;
			metadata.name = player.GetIdentity().GetName();
			metadata.steamid = player.GetIdentity().GetPlainId();

			GetRPCManager().SendRPC( "COT_ESP", "ShowPlayerESPData", new Param1< ref ESPInfoMetaData >( metadata ), false, sender, target );
		}
	}

	void ShowPlayerESPData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		ref Param1< ref ESPInfoMetaData > data;
		if ( !ctx.Read( data ) ) return;
		
		if ( type == CallType.Client )
		{
			ref ESPInfo info = new ref ESPInfo;
			info.name = data.param1.name;
			
			ref AuthPlayer player = GetPermissionsManager().GetPlayerBySteam64ID( data.param1.steamid );
			if ( player )
			{
				info.type = ESPType.PLAYER;
				info.player = player;
				info.target = target;

				CreateESPBox( info );
			}
		}
	}

	void ESPLog( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		ref Param1< string > data;
		if ( !ctx.Read( data ) ) return;
		
		if ( type == CallType.Server )
		{
			COTLog( sender, "ESP " + data.param1 );
		}
	}

	void CreateESPBox( ref ESPInfo info )
	{
		ref ESPRenderBox boxScript = NULL;
		ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/ESP/ESPRenderBox.layout" );

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