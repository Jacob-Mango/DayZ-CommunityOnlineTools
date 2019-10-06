enum JMESPModuleRPC
{
    INVALID = 10300,
	Log,
    FullMap,
    COUNT
};

class JMESPModule: JMRenderableModuleBase
{
	protected const int m_UserIDStart = 10000;

	protected autoptr array< ref JMESPMeta > m_ESPObjects;
	protected autoptr array< ref JMESPWidget > m_ESPBoxes;

	protected autoptr array< ref JMESPViewType > m_ViewTypes;

	protected int m_UserID;

	string Filter;

	float ESPRadius;
	float ESPUpdateTime;
	bool ESPIsUpdating;

	bool IsShowing;

	void JMESPModule()
	{
		m_ESPObjects = new array< ref JMESPMeta >;
		m_ESPBoxes = new array< ref JMESPWidget >;
		m_UserID = m_UserIDStart;

		m_ViewTypes = new array< ref JMESPViewType >;

		ESPRadius = 200;

		ESPUpdateTime = 0.5;
		IsShowing = false;

		GetRPCManager().AddRPC( "COT_ESP", "ESPLog", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_ESP", "RequestFullMapESP", this, SingeplayerExecutionType.Both );

		GetPermissionsManager().RegisterPermission( "ESP.View" );
	}

	void ~JMESPModule()
	{
		HideESP();

		Hide();
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "ESP.View" );
	}

	override string GetInputToggle()
	{
		return "UACOTToggleESP";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/esp_form.layout";
	}

	override string GetTitle()
	{
		return "ESP Tools";
	}
	
	override string GetIconName()
	{
		return "X";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();

		for ( int i = 0; i < m_ViewTypes.Count(); i++ )
		{
			m_ViewTypes[i].HasPermission = GetPermissionsManager().HasPermission( "ESP.View." + m_ViewTypes[i].Permission );
		}

		JMESPForm form;
		if ( Class.CastTo( form, GetForm() ) )
		{
			form.DisableToggleableOptions();
		}
	}

	override void OnInit()
	{
		RegisterTypes();

		for ( int i = 0; i < m_ViewTypes.Count(); i++ )
		{
			GetPermissionsManager().RegisterPermission( "ESP.View." + m_ViewTypes[i].Permission );
		}
	}

	void RegisterTypes()
	{
		m_ViewTypes.Insert( new JMESPViewTypePlayer );
		m_ViewTypes.Insert( new JMESPViewTypeInfected );
		m_ViewTypes.Insert( new JMESPViewTypeAnimal );
		m_ViewTypes.Insert( new JMESPViewTypeCar );
		m_ViewTypes.Insert( new JMESPViewTypeBaseBuilding );
		m_ViewTypes.Insert( new JMESPViewTypeBoltActionRifle );
		m_ViewTypes.Insert( new JMESPViewTypeRifle );
		m_ViewTypes.Insert( new JMESPViewTypePistol );
		m_ViewTypes.Insert( new JMESPViewTypeWeapon );
		m_ViewTypes.Insert( new JMESPViewTypeItemTool );
		m_ViewTypes.Insert( new JMESPViewTypeItemCrafted );
		m_ViewTypes.Insert( new JMESPViewTypeItemTent );
		m_ViewTypes.Insert( new JMESPViewTypeItemMaterial );
		m_ViewTypes.Insert( new JMESPViewTypeItemAttachment );
		m_ViewTypes.Insert( new JMESPViewTypeItemFood );
		m_ViewTypes.Insert( new JMESPViewTypeItemExplosive );
		m_ViewTypes.Insert( new JMESPViewTypeItemBook );
		m_ViewTypes.Insert( new JMESPViewTypeItemContainer );
		m_ViewTypes.Insert( new JMESPViewTypeItemEyewear );
		m_ViewTypes.Insert( new JMESPViewTypeItemAmmo );
		m_ViewTypes.Insert( new JMESPViewTypeItem );
	}

	array< ref JMESPViewType > GetViewTypes()
	{
		return m_ViewTypes;
	}

	override void OnMissionStart()
	{
		if ( IsMissionClient() )
		{
			JMESPWidget.espModule = this;
		}
	}

	void RequestFullMapESP( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "ESP.Manipulation.Delete", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			PlayerBase player = GetPlayerObjectByIdentity( senderRPC );
			if ( player )
			{
				GetCommunityOnlineToolsBase().Log( senderRPC, "Entering Full Map ESP" );
				GetGame().ObjectDelete( player );
			}
			
			GetRPCManager().SendRPC( "COT_ESP", "RequestFullMapESP", new Param );
		}
		
		if ( type == CallType.Client )
		{
			COTPlayerIsRemoved = true;
		}
	}

	void EnableFullMap()
	{
		if ( CurrentActiveCamera == NULL )
			return;

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

		bool isUsingFilter = Filter.Length() > 0;

		string filter = Filter + "";
		filter.ToLower();

		for (int i = 0; i < objects.Count(); ++i)
		{
			Object obj = objects[i];

			if ( obj == NULL )
				continue;

			string type = obj.GetType();
			type.ToLower();

			if ( !IsMissionOffline() && !obj.HasNetworkID() )
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

			if ( isUsingFilter && !type.Contains( filter ) )
				continue;

			for ( int j = 0; j < m_ViewTypes.Count(); j++ )
			{
				JMESPMeta espInfo;
				if ( m_ViewTypes[j].IsValid( obj, espInfo ) )
				{
					CreateESPBox( espInfo );
				}
			}
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

	void ESPLog( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		ref Param1< string > data;
		if ( !ctx.Read( data ) ) return;
		
		if ( type == CallType.Server )
		{
			GetCommunityOnlineToolsBase().Log( senderRPC, "ESP " + data.param1 );
		}
	}

	void CreateESPBox( ref JMESPMeta info )
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