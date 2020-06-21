class JMESPMeta : Managed
{
	private bool m_IsDestroyed;

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

	void ~JMESPMeta()
	{
		Destroy();
	}

	void Create( JMESPModule mod )
	{
		module = mod;

		widgetRoot = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/esp_widget.layout", JMStatics.ESP_CONTAINER );

		if ( widgetRoot == NULL )
			return;

		widgetRoot.GetScript( widgetHandler );

		if ( widgetHandler == NULL )
			return;

		m_IsDestroyed = false;

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
		Print( "  m_IsDestroyed = " + m_IsDestroyed );
		Print( "  m_Action_Position = " + m_Action_Position );
		Print( "  m_Action_Orientation = " + m_Action_Orientation );
		Print( "  m_Action_Health = " + m_Action_Health );
		Print( "  widgetHandler = " + widgetHandler );
		#endif

		if ( m_IsDestroyed )
		{
			#ifdef JM_COT_ESP_DEBUG
			Print( "-JMESPMeta::Destroy() void;" );
			#endif

			return;
		}

		m_IsDestroyed = true;

		if ( widgetRoot )
			widgetRoot.Unlink();

		m_Action_Position = NULL;
		m_Action_Orientation = NULL;
		m_Action_Health = NULL;

		#ifdef JM_COT_ESP_DEBUG
		Print( "-JMESPMeta::Destroy() void;" );
		#endif
	}

	void CreateActions( Widget parent )
	{
		m_Action_Position = UIActionManager.CreateEditableVector( parent, "Position: ", this, "Action_SetPosition", "Set" );
		m_Action_Orientation = UIActionManager.CreateEditableVector( parent, "Orientation: ", this, "Action_SetOrientation", "Set" );

		m_Action_Health = UIActionManager.CreateEditableText( parent, "Global Health: ", this, "Action_SetOrientation", "Set" );
		m_Action_Health.SetOnlyNumbers( true );
	}

	void UpdateActions()
	{
		if ( !viewTypeActions )
			return;
		
		m_Action_Position.SetValue( target.GetPosition() );
		m_Action_Orientation.SetValue( target.GetOrientation() );
		m_Action_Health.SetText( "0" );
	}

	void Action_SetPosition( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		module.SetPosition( m_Action_Position.GetValue(), target );
	}

	void Action_SetOrientation( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		module.SetOrientation( m_Action_Orientation.GetValue(), target );
	}

	void Action_SetHealth( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		module.SetHealth( m_Action_Health.GetText().ToFloat(), target );
	}
}