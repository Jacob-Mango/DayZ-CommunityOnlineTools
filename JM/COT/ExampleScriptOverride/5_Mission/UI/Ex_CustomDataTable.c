#ifdef JM_CommunityOnlineTools
// Example: creating a custom sortable Data Table panel using UIActionManager.
modded class JMExampleForm
{
	protected ref UIActionDataTable m_ExDataTable;

	override protected void BuildDataTableSection( Widget parent )
	{
		super.BuildDataTableSection( parent );

		if ( !parent )
			return;

		m_ExDataTable = UIActionManager.CreateDataTable( parent, layoutRoot, this, "OnExRowClick" );
		if ( m_ExDataTable )
		{
			m_ExDataTable.AddColumn( "ID", 60 );
			m_ExDataTable.AddColumn( "Name", 200 );
			m_ExDataTable.AddRow( { "1", "Custom Item Alpha" } );
		}
	}

	void OnExRowClick( UIEvent eid, UIActionBase action )
	{
		COTCreateLocalAdminNotification( new StringLocaliser( "Clicked data table row" ), JMConstants.Lucide( "list" ) );
	}
}
#endif
