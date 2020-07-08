class JMWebhookForm extends JMFormBase
{
	private UIActionScroller m_sclr_MainActions;
	private Widget m_ActionsWrapper;

	private ref array< ref JMWebhookSection > m_Sections;

	private JMWebhookCOTModule m_Module;

	void JMWebhookForm()
	{
		m_Sections = new array< ref JMWebhookSection >;
	}

	void ~JMWebhookForm()
	{
		delete m_Sections;
	}

	protected override bool SetModule( ref JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_sclr_MainActions = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
		m_ActionsWrapper = m_sclr_MainActions.GetContentWidget();

        
        array< string > test = new array< string >;
        test.Insert( "test" );
        test.Insert( "a" );
        test.Insert( "b" );
        test.Insert( "abc" );
        test.Insert( "jacob" );
        test.Insert( "ficarra" );
        UIActionManager.CreateDropdownBox( m_ActionsWrapper, GetLayoutRoot(), "Dropdown", test );

		m_sclr_MainActions.UpdateScroller();
	}
}