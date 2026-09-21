//! "Info" tab of JMEntityManagerForm - the detail card for whichever entity is
//! currently selected, plus whatever extra rows and widgets the adapter adds.
//! Back-reference to the owning form, same shape as JMPlayerRowWidget.Menu.
class JMEntityManagerFormTabInfo: JMFormTab
{
	protected JMEntityManagerForm m_Form;
	protected UIActionScroller m_InfoScroller;
	protected UIActionText m_InfoName;
	protected UIActionText m_InfoClassName;
	protected UIActionText m_InfoStatus;
	protected UIActionText m_InfoPosition;
	protected ref array<ref UIActionText> m_ExtraRows;
	protected ref array<Widget> m_DynamicWidgets;  // widgets added each ShowEntity call

	void JMEntityManagerFormTabInfo( JMEntityManagerForm form )
	{
		m_Form = form;

		m_ExtraRows      = new array<ref UIActionText>;
		m_DynamicWidgets = new array<Widget>;
	}

	//! True once OnCreate has built the widgets ShowEntity fills.
	bool IsBuilt()
	{
		return m_InfoName && m_InfoScroller;
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		m_InfoScroller = UIActionManager.CreateScroller( panel );
		Widget infoContent = m_InfoScroller.GetContentWidget();

		UIActionCard infoCard = UIActionManager.CreateCard( infoContent, "#STR_COT_VEHICLE_INFORMATION_TITLE" );
		Widget infoGrid = UIActionManager.CreateGridSpacer( infoCard.GetContent(), 4, 1 );
			m_InfoName      = UIActionManager.CreateText( infoGrid, "#STR_COT_ENTITYMANAGER_NAME", "" );
			m_InfoClassName = UIActionManager.CreateText( infoGrid, "#STR_COT_ENTITYMANAGER_CLASSNAME", "" );
			m_InfoStatus    = UIActionManager.CreateText( infoGrid, "#STR_COT_ENTITYMANAGER_STATUS", "" );
			m_InfoPosition  = UIActionManager.CreateText( infoGrid, "#STR_COT_ENTITYMANAGER_POSITION", "" );

		m_InfoScroller.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		UpdateScroller();
	}

	void UpdateScroller()
	{
		if ( m_InfoScroller )
			m_InfoScroller.UpdateScroller();
	}

	//! Repaint the four fixed rows in place - a delta refresh of the entity that
	//! is already on screen.
	void UpdateFields( JMEntityMetaData entity )
	{
		if ( !m_InfoName )
			return;

		m_InfoName.SetText( entity.m_DisplayName );
		m_InfoClassName.SetText( entity.m_ClassName );
		m_InfoStatus.SetText( entity.m_StatusText );
		m_InfoPosition.SetText( entity.m_Position.ToString() );
	}

	//! Fill the tab for a newly picked entity: the fixed rows, then whatever the
	//! adapter contributes.
	void ShowEntity( JMEntityMetaData entity )
	{
		UpdateFields( entity );

		// Tear down any widgets we added on the previous entity click.
		ClearDynamicWidgets();

		Widget infoContent = m_InfoScroller.GetContentWidget();

		JMEntityManagerAdapter adapter = m_Form.GetAdapter();
		array<string> keys = new array<string>;
		map<string, string> vals = new map<string, string>;
		if ( adapter )
			adapter.GetInfoRows( entity, keys, vals );

		if ( keys && keys.Count() > 0 )
		{
			Widget extraGrid = UIActionManager.CreateGridSpacer( infoContent, keys.Count(), 1 );
			m_DynamicWidgets.Insert( extraGrid );
			foreach ( string k: keys )
			{
				UIActionText t = UIActionManager.CreateText( extraGrid, k + ":", vals.Get( k ) );
				m_ExtraRows.Insert( t );
			}
		}

		if ( adapter )
			adapter.OnBuildInfoExtras( infoContent, entity, m_Form );

		m_InfoScroller.UpdateScroller();
	}

	// Adapters can register widgets they create in OnBuildInfoExtras so they
	// get torn down when the user picks a different entity.
	void TrackDynamicWidget( Widget w )
	{
		if ( w )
			m_DynamicWidgets.Insert( w );
	}

	void ClearDynamicWidgets()
	{
		m_ExtraRows.Clear();
		foreach ( Widget w: m_DynamicWidgets )
		{
			if ( w )
				w.Unlink();
		}
		m_DynamicWidgets.Clear();
	}
}
