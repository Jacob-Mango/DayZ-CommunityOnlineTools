class JMWebhookForm: JMFormBase
{
	private UIActionButton m_AddSectionButton;

	private UIActionScroller m_sclr_MainActions;
	private Widget m_ActionsWrapper;

	private ref map< string, ref JMWebhookSection > m_Sections;

	private ref array< string > m_Types;

	private string m_RemovingName;

	private string m_CreatingURL;
	private string m_CreatingName;

	private JMWebhookCOTModule m_Module;

	void JMWebhookForm()
	{
		m_Types = new array< string >();
		JMWebhookConstructor.Generate( m_Types );

		m_Sections = new map< string, ref JMWebhookSection >();
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		Widget spacer = UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "header_panel" ), 2, 1 );
		m_AddSectionButton = UIActionManager.CreateButton( spacer, "Add Webhook", this, "Action_AddWebhook" );
		UIActionManager.CreatePanel( spacer, 0xFF000000, 5 );

		m_sclr_MainActions = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
		m_ActionsWrapper = m_sclr_MainActions.GetContentWidget();

		OnSettingsUpdated();

		m_sclr_MainActions.UpdateScroller();
	}

	override void OnSettingsUpdated()
	{
		array< ref JMWebhookConnectionGroup > groups = m_Module.GetConnections();

		array< string > toKeep = new array< string >();
		for ( int i = 0; i < groups.Count(); ++i )
		{
			string name = groups[i].Name;
			JMWebhookSection section = m_Sections.Get( name );
			if ( !section )
			{
				section = new JMWebhookSection( m_ActionsWrapper, this, groups[i], m_Types );
				m_Sections.Insert( name, section );
			}

			toKeep.Insert( name );

			section.UpdateState( groups[i], m_Types );
		}

		for ( i = 0; i < m_Sections.Count(); ++i )
		{
			name = m_Sections.GetKey( i );
			if ( toKeep.Find( name ) != -1 )
				continue;

			m_Sections.Remove( name );
		}

		m_sclr_MainActions.UpdateScroller();
	}

	void Action_AddWebhook( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateConfirmation_Two( JMConfirmationType.EDIT, "Create Webhook", "put name here", "#STR_COT_GENERIC_CANCEL", "Action_AddWebhook_Cancel", "#STR_COT_GENERIC_SELECT_NAME", "Action_AddWebhook_SelectName" );
	}

	void Action_AddWebhook_Cancel( JMConfirmation confirmation )
	{
		m_CreatingURL = "";
		m_CreatingName = "";
	}

	void Action_AddWebhook_SelectName( JMConfirmation confirmation )
	{
		m_CreatingName = confirmation.GetEditBoxValue();

		CreateConfirmation_Two( JMConfirmationType.EDIT, "Create Webhook", "put address here", "#STR_COT_GENERIC_CANCEL", "Action_AddWebhook_Cancel", "#STR_COT_GENERIC_CREATE", "Action_AddWebhook_Create" );
	}

	void Action_AddWebhook_Create( JMConfirmation confirmation )
	{
		m_CreatingURL = confirmation.GetEditBoxValue();

		m_Module.AddConnectionGroup( m_CreatingName, m_CreatingURL );
	}

	void Action_RemoveWebhook( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMWebhookTypeData data;
		Class.CastTo( data, action.GetData() );

		m_RemovingName = data.Name;

		CreateConfirmation_Two( JMConfirmationType.INFO, "Are you sure?", "Are you sure you want to remove the webhook?", "#STR_COT_GENERIC_CANCEL", "Action_RemoveWebhook_Cancel", "#STR_COT_GENERIC_REMOVE", "Action_RemoveWebhook_Remove" );
	}

	void Action_RemoveWebhook_Cancel( JMConfirmation confirmation )
	{
		m_RemovingName = "";
	}

	void Action_RemoveWebhook_Remove( JMConfirmation confirmation )
	{
		m_Module.RemoveConnectionGroup( m_RemovingName );
	}

	void Click_UpdateState( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMWebhookTypeData data;
		Class.CastTo( data, action.GetData() );

		string name = data.Name;
	}
};
