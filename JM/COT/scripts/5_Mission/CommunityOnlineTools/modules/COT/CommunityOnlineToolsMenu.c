class CommunityOnlineToolsForm extends JMFormBase
{
	protected Widget m_ActionsWrapper;

	protected ref UIActionCheckbox m_UITestSort;

	void CommunityOnlineToolsForm()
	{
		CommunityOnlineToolsModule.DATA_UPDATED.Insert( UpdateData );
	}

	void ~CommunityOnlineToolsForm()
	{
		CommunityOnlineToolsModule.DATA_UPDATED.Remove( UpdateData );
	}

	override string GetTitle()
	{
		return "Community Online Tools";
	}

	override string GetImageSet()
	{
		return "ccgui_enforce";
	}
	
	override string GetIconName()
	{
		return "HudBuild";
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override void OnInit( bool fromMenu )
	{
		m_ActionsWrapper	= UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "actions_wrapper" ), 4, 1 );

		m_UITestSort		= UIActionManager.CreateCheckbox( m_ActionsWrapper, "Player List Sort: " );
		
		UIActionManager.CreateButton( m_ActionsWrapper, "Update Settings", this, "LoadData" );
	}

	void UpdateData( ref CommunityOnlineToolsData data )
	{
		m_UITestSort.SetChecked( data.TestSort );
	}

	void LoadData( UIEvent eid, ref UIActionButton action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CommunityOnlineToolsModule cotModule = CommunityOnlineToolsModule.Cast( module );
		
		if ( cotModule )
		{
			cotModule.GetData().TestSort = m_UITestSort.IsChecked();
		
			GetRPCManager().SendRPC( "COT_Module", "LoadData", new Param1< ref CommunityOnlineToolsData >( cotModule.GetData() ), false, NULL );
		}
	}
}