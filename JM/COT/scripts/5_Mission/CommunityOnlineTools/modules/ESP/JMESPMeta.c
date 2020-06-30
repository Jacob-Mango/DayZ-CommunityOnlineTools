class JMESPMeta : Managed
{
	string name;
	JMESPViewType type;
	int colour;
	JMPlayerInstance player;
	Object target;

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
			widgetRoot.Unlink();

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

		m_Action_Health = UIActionManager.CreateEditableText( parent, "Health: ", this, "Action_SetOrientation", "", "Set" );
		m_Action_Health.SetOnlyNumbers( true );

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );

		m_Action_Delete = UIActionManager.CreateButton( parent, "Delete Object", this, "Action_Delete" );
	}

	void UpdateActions()
	{
		if ( !viewTypeActions || !widgetRoot )
			return;
		
		m_Action_PositionX.SetText( target.GetPosition()[0].ToString() );
		m_Action_PositionY.SetText( target.GetPosition()[1].ToString() );
		m_Action_PositionZ.SetText( target.GetPosition()[2].ToString() );
		
		m_Action_OrientationX.SetText( target.GetOrientation()[0].ToString() );
		m_Action_OrientationY.SetText( target.GetOrientation()[1].ToString() );
		m_Action_OrientationZ.SetText( target.GetOrientation()[2].ToString() );

		m_Action_Health.SetText( "0" );
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

		module.SetHealth( action.GetText().ToFloat(), target );
	}

	void Action_Delete( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		module.DeleteObject(  target );
	}
}

class JMESPMetaBaseBuilding : JMESPMeta
{
	override void CreateActions( Widget parent )
	{
		super.CreateActions( parent );

	}
}