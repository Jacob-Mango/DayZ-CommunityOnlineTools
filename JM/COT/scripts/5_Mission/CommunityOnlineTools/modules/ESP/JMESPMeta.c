class JMESPMeta : Managed
{
	string name;
	JMESPViewType type;
	int colour;
	JMPlayerInstance player;
	Object target;

	JMESPModule module;
	JMESPWidget widget;

	UIActionEditableVector m_Action_Position;
	UIActionEditableVector m_Action_Orientation;
	UIActionEditableText m_Action_Health;

	void Create( JMESPModule mod )
	{
		module = mod;

		Widget w = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/esp_widget.layout", JMStatics.ESP_CONTAINER );

		if ( w == NULL )
			return;

		w.GetScript( widget );

		if ( widget == NULL )
			return;

		Widget viewTypeActions;
		widget.SetInfo( this, viewTypeActions );

		CreateActions( viewTypeActions );

		UpdateActions();
	}

	bool IsValid()
	{
		if ( target == NULL )
			return false;

		return type.IsValid( target, this );
	}

	void Destroy()
	{		
		if ( widget )
			widget.Unlink();
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