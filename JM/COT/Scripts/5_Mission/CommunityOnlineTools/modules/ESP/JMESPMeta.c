class JMESPMeta : Managed
{
	string name;
	JMESPViewType type;
	int colour;
	JMPlayerInstance player;
	Object target;

	int networkLow;
	int networkHigh;

	JMESPModule module;
	JMESPWidgetHandler widgetHandler;
	Widget widgetRoot;
	Widget viewTypeActions;

	UIActionEditableText m_Action_PositionX;
	UIActionEditableText m_Action_PositionY;
	UIActionEditableText m_Action_PositionZ;
	UIActionButton m_Action_Position;
	
	UIActionEditableText m_Action_OrientationX;
	UIActionEditableText m_Action_OrientationY;
	UIActionEditableText m_Action_OrientationZ;
	UIActionButton m_Action_Orientation;

	UIActionEditableText m_Action_Health;

	UIActionButton m_Action_Delete;

	void ~JMESPMeta()
	{
		Destroy();
	}

	void Create( JMESPModule mod )
	{
		module = mod;

		if ( widgetRoot )
			return;

		if ( !Class.CastTo( widgetRoot, GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/esp_widget.layout", JMStatics.ESP_CONTAINER ) ) )
			return;

		widgetRoot.GetScript( widgetHandler );
		if ( !widgetHandler )
			return;

		widgetHandler.SetInfo( this, viewTypeActions );

		CreateActions( viewTypeActions );

		UpdateActions();
	}

	bool IsValid()
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPMeta::IsValid() bool;" );
		Print( "  target = " + Object.GetDebugName( target ) );
		#endif

		if ( target == NULL )
			return false;

		if ( !(type.HasPermission && type.View) )
			return false;

		return type.IsValid( target, this );
	}

	void Destroy()
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPMeta::Destroy() void;" );
		Print( "  m_Action_Position = " + m_Action_Position );
		Print( "  m_Action_Orientation = " + m_Action_Orientation );
		Print( "  m_Action_Health = " + m_Action_Health );
		Print( "  widgetHandler = " + widgetHandler );
		#endif

		if ( widgetRoot )
			widgetRoot.Show( false );

		#ifdef JM_COT_ESP_DEBUG
		Print( "-JMESPMeta::Destroy() void;" );
		#endif
	}

	void CreateActions( Widget parent )
	{
		Widget positionActions = UIActionManager.CreateGridSpacer( parent, 2, 1 );
		Widget positionActionsVec = UIActionManager.CreateGridSpacer( positionActions, 1, 3 );

		m_Action_PositionX = UIActionManager.CreateEditableText( positionActionsVec, "X:" );
		m_Action_PositionY = UIActionManager.CreateEditableText( positionActionsVec, "Y:" );
		m_Action_PositionZ = UIActionManager.CreateEditableText( positionActionsVec, "Z:" );

		m_Action_PositionX.SetOnlyNumbers( true );
		m_Action_PositionY.SetOnlyNumbers( true );
		m_Action_PositionZ.SetOnlyNumbers( true );
		
		m_Action_Position = UIActionManager.CreateButton( positionActions, "Set Position", this, "Action_SetPosition" );

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );

		Widget orientationActions = UIActionManager.CreateGridSpacer( parent, 2, 1 );
		Widget orientationActionsVec = UIActionManager.CreateGridSpacer( orientationActions, 1, 3 );

		m_Action_OrientationX = UIActionManager.CreateEditableText( orientationActionsVec, "X:" );
		m_Action_OrientationY = UIActionManager.CreateEditableText( orientationActionsVec, "Y:" );
		m_Action_OrientationZ = UIActionManager.CreateEditableText( orientationActionsVec, "Z:" );

		m_Action_OrientationX.SetOnlyNumbers( true );
		m_Action_OrientationY.SetOnlyNumbers( true );
		m_Action_OrientationZ.SetOnlyNumbers( true );
		
		m_Action_Orientation = UIActionManager.CreateButton( orientationActions, "Set Orientation", this, "Action_SetOrientation" );

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );

		m_Action_Health = UIActionManager.CreateEditableText( parent, "Health: ", this, "Action_SetHealth", "", "Set" );
		m_Action_Health.SetOnlyNumbers( true );

		if ( CanDelete() )
		{
			UIActionManager.CreatePanel( parent, 0xFF000000, 1 );

			m_Action_Delete = UIActionManager.CreateButton( parent, "Delete Object", this, "Action_Delete" );
		}
	}

	bool CanDelete()
	{
		return true;
	}

	private string FloatToString( float num )
	{
		string ret = num.ToString();
		if ( ret == "" )
			ret = "0";

		return ret;
	}

	void UpdateActions()
	{
		if ( !viewTypeActions || !widgetRoot )
			return;
		
		if ( !m_Action_PositionX.IsFocused() ) m_Action_PositionX.SetText( FloatToString( target.GetPosition()[0] ) );
		if ( !m_Action_PositionY.IsFocused() ) m_Action_PositionY.SetText( FloatToString( target.GetPosition()[1] ) );
		if ( !m_Action_PositionZ.IsFocused() ) m_Action_PositionZ.SetText( FloatToString( target.GetPosition()[2] ) );
		
		if ( !m_Action_OrientationX.IsFocused() ) m_Action_OrientationX.SetText( FloatToString( target.GetOrientation()[0] ) );
		if ( !m_Action_OrientationY.IsFocused() ) m_Action_OrientationY.SetText( FloatToString( target.GetOrientation()[1] ) );
		if ( !m_Action_OrientationZ.IsFocused() ) m_Action_OrientationZ.SetText( FloatToString( target.GetOrientation()[2] ) );

		if ( !m_Action_Health.IsFocused() ) m_Action_Health.SetText( "-1" );
	}

	void Action_SetPosition( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		vector pos = vector.Zero;
		pos[0] = m_Action_PositionX.GetText().ToFloat();
		pos[1] = m_Action_PositionY.GetText().ToFloat();
		pos[2] = m_Action_PositionZ.GetText().ToFloat();

		module.SetPosition( pos, target );
	}

	void Action_SetOrientation( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		vector pos = vector.Zero;
		pos[0] = m_Action_OrientationX.GetText().ToFloat();
		pos[1] = m_Action_OrientationY.GetText().ToFloat();
		pos[2] = m_Action_OrientationZ.GetText().ToFloat();

		module.SetOrientation( pos, target );
	}

	void Action_SetHealth( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		float health = action.GetText().ToFloat();
		if ( health == -1 )
			return;

		module.SetHealth( health, "", target );
	}

	void Action_Delete( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( IsMissionOffline() )
			module.DeleteObject( target );
		else
			module.DeleteObject( networkLow, networkHigh );
	}
}

class JMESPMetaPlayer : JMESPMeta
{
	UIActionText m_Player_Name;
	UIActionText m_Player_GUID;
	UIActionText m_Player_Steam;
	//UIActionText m_Player_Speaking;
	
	override void CreateActions( Widget parent )
	{
		m_Player_Name = UIActionManager.CreateText( parent, "Name: ", "" );
		m_Player_GUID = UIActionManager.CreateText( parent, "GUID: ", "" );
		m_Player_Steam = UIActionManager.CreateText( parent, "Steam: ", "" );
		//m_Player_Speaking = UIActionManager.CreateText( parent, "Talking: ", "" );

		super.CreateActions( parent );
	}

	override void UpdateActions()
	{
		super.UpdateActions();
		
		m_Player_Name.SetText( player.GetName() );
		m_Player_GUID.SetText( player.GetGUID() );
		m_Player_Steam.SetText( player.GetSteam64ID() );

		PlayerBase player = PlayerBase.Cast(target);
		if (player)
		{
			//if (player.IsPlayerSpeaking() > 0.1) m_Player_Speaking.SetText("Yes");
			//else m_Player_Speaking.SetText("No");
		}
	}
	
	override bool CanDelete()
	{
		return false;
	}
}

class JMESPMetaBaseBuilding : JMESPMeta
{
	ref map< string, ref JMConstructionPartData > m_Parts;

	ref array< UIActionText > m_StateHeaders;
	ref array< UIActionButton > m_BuildButtons;
	ref array< UIActionButton > m_DismantleButtons;
	ref array< UIActionButton > m_RepairButtons;

	BaseBuildingBase m_BaseBuilding;

	void JMESPMetaBaseBuilding()
	{
		m_Parts = new map< string, ref JMConstructionPartData >;

		m_StateHeaders = new array< UIActionText >;
		m_BuildButtons = new array< UIActionButton >;
		m_DismantleButtons = new array< UIActionButton >;
		m_RepairButtons = new array< UIActionButton >;
	}

	void ~JMESPMetaBaseBuilding()
	{
		if ( m_BaseBuilding )
		{
			m_BaseBuilding.m_COT_ConstructionUpdate.Remove( OnPartsUpdate );
		}

		delete m_Parts;

		delete m_StateHeaders;
		delete m_BuildButtons;
		delete m_DismantleButtons;
		delete m_RepairButtons;
	}

	override void CreateActions( Widget parent )
	{
		super.CreateActions( parent );

		m_BaseBuilding = BaseBuildingBase.Cast( target );

		m_BaseBuilding.m_COT_ConstructionUpdate.Insert( OnPartsUpdate );

		m_BaseBuilding.GetConstruction().COT_GetParts( m_Parts, false );

		for ( int i = 0; i < m_Parts.Count(); ++i )
		{
			string partName = m_Parts.GetKey( i );
			JMConstructionPartData part = m_Parts.Get( partName );

			Widget partActions = UIActionManager.CreateGridSpacer( parent, 2, 1 );

			UIActionText txt_H = UIActionManager.CreateText( partActions, part.m_DisplayName, "INIT" );

			Widget buttonActions = UIActionManager.CreateGridSpacer( partActions, 1, 3 );
			UIActionButton btn_B = UIActionManager.CreateButton( buttonActions, "Build", this, "Action_Build" );
			UIActionButton btn_D = UIActionManager.CreateButton( buttonActions, "Dismantle", this, "Action_Dismantle" );
			UIActionButton btn_R = UIActionManager.CreateButton( buttonActions, "Repair", this, "Action_Repair" );

			txt_H.SetData( part );
			btn_B.SetData( part );
			btn_D.SetData( part );
			btn_R.SetData( part );

			m_StateHeaders.Insert( txt_H );
			m_BuildButtons.Insert( btn_B );
			m_DismantleButtons.Insert( btn_D );
			m_RepairButtons.Insert( btn_R );

			//UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
		}

		UpdateButtonStates();
	}

	void OnPartsUpdate()
	{
		m_BaseBuilding.GetConstruction().COT_GetParts( m_Parts, false );

		UpdateButtonStates();
	}

	private string StateToString( JMConstructionPartState state )
	{
		switch ( state )
		{
			case JMConstructionPartState.BUILT:
				return "Built";
			case JMConstructionPartState.CAN_BUILD:
				return "Can Build";
			case JMConstructionPartState.REQUIRED_PART_NOT_BUILT:
				return "Requires Part";
			case JMConstructionPartState.CONFLICTING_PART:
				return "Conflicting Part";
			case JMConstructionPartState.NOT_ENOUGH_MATERIALS:
				return "Not Enough Materials";
		}

		return "UNKNOWN";
	}

	private void UpdateButtonStates()
	{
		for ( int i = 0; i < m_StateHeaders.Count(); ++i )
		{
			JMConstructionPartData data;
			Class.CastTo( data, m_StateHeaders[i].GetData() );

			m_StateHeaders[i].SetText( StateToString( data.m_State ) );

			if ( data.m_State == JMConstructionPartState.BUILT )
			{
				m_BuildButtons[i].Disable();
				m_DismantleButtons[i].Enable();
				m_RepairButtons[i].Enable();
			} else if ( data.m_State == JMConstructionPartState.CAN_BUILD )
			{
				m_BuildButtons[i].Enable();
				m_DismantleButtons[i].Disable();
				m_RepairButtons[i].Disable();
			} else 
			{
				m_StateHeaders[i].Hide();
				m_BuildButtons[i].Hide();
				m_DismantleButtons[i].Hide();
				m_RepairButtons[i].Hide();

				continue;
			}

			m_BuildButtons[i].Show();
			m_DismantleButtons[i].Show();
			m_RepairButtons[i].Show();
		}
	}

	void Action_Build( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMConstructionPartData data;
		Class.CastTo( data, action.GetData() );

		module.BaseBuilding_Build( m_BaseBuilding, data.m_Name );
	}

	void Action_Dismantle( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMConstructionPartData data;
		Class.CastTo( data, action.GetData() );

		module.BaseBuilding_Dismantle( m_BaseBuilding, data.m_Name );
	}

	void Action_Repair( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMConstructionPartData data;
		Class.CastTo( data, action.GetData() );

		module.BaseBuilding_Repair( m_BaseBuilding, data.m_Name );
	}
}