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

	UIActionEditableVector m_Action_Position;
	UIActionEditableVector m_Action_Orientation;
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
		m_Action_Position = UIActionManager.CreateEditableVector( parent, "Position: ", this, "Action_SetPosition", "Set" );
		m_Action_Orientation = UIActionManager.CreateEditableVector( parent, "Orientation: ", this, "Action_SetOrientation", "Set" );

		m_Action_Health = UIActionManager.CreateEditableText( parent, "Global Health: ", this, "Action_SetOrientation", "", "Set" );
		m_Action_Health.SetOnlyNumbers( true );

		m_Action_Delete = UIActionManager.CreateButton( parent, "Delete", this, "Action_Delete" );
	}

	void UpdateActions()
	{
		if ( !viewTypeActions || !widgetRoot )
			return;
		
		m_Action_Position.SetValue( target.GetPosition() );
		m_Action_Orientation.SetValue( target.GetOrientation() );
		m_Action_Health.SetText( "0" );
	}

	void Action_SetPosition( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		module.SetPosition( action.GetValue(), target );
	}

	void Action_SetOrientation( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		module.SetOrientation( action.GetValue(), target );
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