class ESPModule: EditorModule
{
	protected ref array< ref ESPInfo > m_ESPObjects;
	protected ref array< ref ESPBox > m_ESPBoxes;
	protected ref array< ref ESPBox > m_SelectedBoxes;

	protected bool m_CanViewPlayers;
	protected bool m_CanViewBaseBuilding;
	protected bool m_CanViewVehicles;
	protected bool m_CanViewItems;
	protected bool m_CanViewInfected;
	protected bool m_CanViewCreature;
	protected bool m_CanViewEverything;

	protected int m_UserID;

	bool ViewPlayers;
	bool ViewBaseBuilding;
	bool ViewVehicles;
	bool ViewItems;
	bool ViewInfected;
	bool ViewCreature;
	bool ViewEverything;

	float ESPRadius;

	vector Position;
	vector Rotation;

	float MaxHealth;
	float Health;

	float ESPUpdateTime;

	bool IsShowing;

	void ESPModule()
	{
		m_ESPObjects = new ref array< ref ESPInfo >;
		m_ESPBoxes = new ref array< ref ESPBox >;
		m_SelectedBoxes = new ref array< ref ESPBox >;
		m_UserID = 0;

		ESPRadius = 200;

		Position = "0 0 0";
		Rotation = "0 0 0";

		ESPUpdateTime = 0;
		IsShowing = false;

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
		m_CanViewInfected = GetPermissionsManager().HasPermission( "ESP.View.Infected" );
		m_CanViewCreature = GetPermissionsManager().HasPermission( "ESP.View.Creature" );
		m_CanViewEverything = GetPermissionsManager().HasPermission( "ESP.View" );
	}

	override void OnMissionStart()
	{
		ESPBox.espModule = this;
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
		
		if( type == CallType.Server )
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
		
		if( type == CallType.Server )
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

	void SetSelected()
	{
		if ( m_SelectedBoxes.Count() > 0 ) 
		{
			GetRPCManager().SendRPC( "COT_ESP", "ServerSetSelected", new Param3< vector, vector, float >( Position, Rotation, Health ), true, NULL, m_SelectedBoxes[0].Info.target );
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

		GetRPCManager().SendRPC( "COT_ESP", "ServerDeleteSelected", new Param1< ref array< Object > >( objects ), true );

		m_SelectedBoxes.Clear();
		delete objects;
	}

	void SelectBox( ref ESPBox box, bool checked, bool button )
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
			Position = m_SelectedBoxes[0].Info.target.GetPosition();
			Rotation = m_SelectedBoxes[0].Info.target.GetYawPitchRoll();
			MaxHealth = m_SelectedBoxes[0].Info.target.GetMaxHealth( "", "" );
			Health = m_SelectedBoxes[0].Info.target.GetHealth( "", "" );
		}
	}

	void HideESP( bool fromUpdate = false )
	{
		if ( !fromUpdate )
		{
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( this.UpdateESP );

			IsShowing = false;
		}

		for (int j = 0; j < m_ESPBoxes.Count(); j++ )
		{
			m_ESPBoxes[j].Unlink();
		}

		m_ESPBoxes.Clear();

		m_SelectedBoxes.Clear();

		m_ESPObjects.Clear();

		m_UserID = 0;
	}

	void UpdateESP()
	{
		HideESP( true );

		IsShowing = true;

		ref array<Object> objects = new ref array<Object>;
		GetGame().GetObjectsAtPosition( GetCurrentPosition(), ESPRadius, objects, NULL );

		Object currentObj;
		EntityAI entity;
		ref ESPInfo espInfo;

		for (int i = 0; i < objects.Count(); ++i)
		{
			currentObj = Object.Cast( objects.Get(i) );

			if ( currentObj == NULL )
				continue;

			if ( !currentObj.HasNetworkID() )
				continue;

			if ( currentObj.IsRock() )
				continue;
			if ( currentObj.IsWoodBase() )
				continue;
			if ( currentObj.IsBush() )
				continue;
			if ( currentObj.IsTree() )
				continue;
			if ( currentObj.IsBuilding() )
				continue;

			entity = EntityAI.Cast( currentObj );

			bool isPlayer = false;
			
			if ( entity != NULL )
			{
				isPlayer = entity.IsPlayer();
			
				if ( (ViewPlayers || ViewEverything) && (m_CanViewPlayers || m_CanViewEverything) && isPlayer )
				{
					GetRPCManager().SendRPC( "COT_ESP", "RequestPlayerESPData", new Param, false, NULL, currentObj );
					continue;
				}

				bool isInfected = !isPlayer && ( entity.IsZombie() || entity.IsZombieMilitary() );
				if ( (ViewInfected || ViewEverything) && (m_CanViewInfected || m_CanViewEverything) && isInfected )
				{
					espInfo = new ref ESPInfo;

					espInfo.name = currentObj.GetDisplayName();
					if ( espInfo.name == "" )
						espInfo.name = currentObj.GetType();

					espInfo.target = currentObj;
					espInfo.type = ESPType.INFECTED;

					CreateESPBox( espInfo );
					continue;
				} 

				bool isCreature = !isInfected && entity.IsAnimal();
				if ( (ViewCreature || ViewEverything) && (m_CanViewCreature || m_CanViewEverything) && isCreature )
				{
					espInfo = new ref ESPInfo;

					espInfo.name = currentObj.GetDisplayName();
					if ( espInfo.name == "" )
						espInfo.name = currentObj.GetType();

					espInfo.target = currentObj;
					espInfo.type = ESPType.CREATURE;

					CreateESPBox( espInfo );
					continue;
				}
			}

			bool isTransport = !isPlayer && currentObj.IsTransport();
			if ( (ViewVehicles || ViewEverything) && (m_CanViewVehicles || m_CanViewEverything) && isTransport )
			{
				espInfo = new ref ESPInfo;
				
				espInfo.name = currentObj.GetDisplayName();
				if ( espInfo.name == "" )
					espInfo.name = currentObj.GetType();

				espInfo.target = currentObj;
				espInfo.type = ESPType.VEHICLE;

				CreateESPBox( espInfo );
				continue;
			}

			bool isBaseBuilding = !isTransport && ( currentObj.IsContainer() || currentObj.CanUseConstruction() || currentObj.IsFireplace() );
			if ( (ViewBaseBuilding || ViewEverything) && (m_CanViewBaseBuilding || m_CanViewEverything) && isBaseBuilding )
			{
				espInfo = new ref ESPInfo;
				
				espInfo.name = currentObj.GetDisplayName();
				if ( espInfo.name == "" )
					espInfo.name = currentObj.GetType();

				espInfo.target = currentObj;
				espInfo.type = ESPType.BASEBUILDING;

				CreateESPBox( espInfo );
				continue;
			}

			bool isItem = !isBaseBuilding && ( currentObj.IsItemBase() || currentObj.IsInventoryItem() );
			if ( (ViewItems || ViewEverything) && (m_CanViewItems || m_CanViewEverything) && isItem )
			{
				espInfo = new ref ESPInfo;
				
				espInfo.name = currentObj.GetDisplayName();
				if ( espInfo.name == "" )
					espInfo.name = currentObj.GetType();

				espInfo.target = currentObj;
				espInfo.type = ESPType.ITEM;

				CreateESPBox( espInfo );
				continue;
			} 

			if ( ViewEverything && m_CanViewEverything )
			{
				espInfo = new ref ESPInfo;
				
				espInfo.name = currentObj.GetDisplayName();
				if ( espInfo.name == "" )
					espInfo.name = currentObj.GetType();

				espInfo.target = currentObj;
				espInfo.type = ESPType.ALL;

				CreateESPBox( espInfo );
				continue;
			} 
		}

		objects.Clear();
		delete objects;

		GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( this.UpdateESP );

		if ( ESPUpdateTime > 0 )
		{
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( this.UpdateESP, ESPUpdateTime * 1000.0 );
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
			
			ref AuthPlayer player = GetPlayerForID( data.param1.steamid );
			if ( player )
			{
				info.type = ESPType.PLAYER;
				info.player = player;
				info.target = target;

				CreateESPBox( info );
			}
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