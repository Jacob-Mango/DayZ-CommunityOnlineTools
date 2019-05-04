class COTManagerMenu extends Form
{
	protected Widget m_ActionsWrapper;

	protected UIActionCheckbox m_DebugMode;

	void COTManagerMenu()
	{
	}

	void ~COTManagerMenu()
	{
	}

	override string GetTitle()
	{
		return "COT Manager";
	}
	
	override string GetIconName()
	{
		return "CM";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override void OnInit( bool fromMenu )
	{
		m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );

		ref Widget settings = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 2, 1 );

		m_DebugMode = UIActionManager.CreateCheckbox( settings, "Debug Mode" );

		UIActionManager.CreateButton( settings, "Apply Settings", this, "Click_Set" );
	}

	override void OnShow()
	{
		super.OnShow();

		COTManagerModule cm = COTManagerModule.Cast( module );

		if ( !cm ) return;

		m_DebugMode.SetChecked( cm.GetSettings().DebugMode );
	}

	void Click_Set( UIEvent eid, ref UIActionButton action )
	{
		COTManagerModule cm = COTManagerModule.Cast( module );

		if ( !cm ) return;

        COTManagerSettings settings = cm.GetSettings();

		if ( !settings ) return;

		settings.DebugMode = m_DebugMode.IsChecked();

		GetRPCManager().SendRPC( "COT_Manager", "LoadData", new Param1< string >( JsonFileLoader< COTManagerSettings >.JsonMakeData( settings ) ), false );
	}

	override void OnHide()
	{
		super.OnHide();
	}
}