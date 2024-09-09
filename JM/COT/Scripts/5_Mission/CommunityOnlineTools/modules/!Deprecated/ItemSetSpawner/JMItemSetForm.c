class JMItemSetForm: JMFormBase
{
	private UIActionScroller m_sclr_MainActions;
	private Widget m_ActionsWrapper;

	private autoptr array< ref UIActionButton > m_ItemSetButtons;

	private JMItemSetSpawnerModule m_Module;

	void JMItemSetForm()
	{
		m_ItemSetButtons = new array< ref UIActionButton >;
	}

	void ~JMItemSetForm()
	{
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_sclr_MainActions = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
		m_ActionsWrapper = m_sclr_MainActions.GetContentWidget();

		m_sclr_MainActions.UpdateScroller();
	}

	override void OnShow()
	{
		if (m_Module.IsLoaded())
			OnSettingsUpdated();
		else
			m_Module.Load();
	}

	override void OnSettingsUpdated()
	{
		if (!m_Module.IsLoaded())
			return;

		array< string > items = new array< string >;
		items.Copy( m_Module.GetItemSets() );
		
		JMStatics.SortStringArray( items );

		for ( int j = 0; j < items.Count(); j++ )
		{
			string name = items[j];
			
			if ( name == "" )
				continue;

			Widget wrapper = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 1, 2 );

			UIActionManager.CreateText( wrapper, name );
			
			Widget buttonswrapper = UIActionManager.CreateGridSpacer( wrapper, 1, 2 );
			
			UIActionButton selButton = UIActionManager.CreateButton( buttonswrapper, "#STR_COT_OBJECT_MODULE_SELECTED_PLAYERS", this, "SpawnOnPlayers" );
			selButton.SetData( new JMItemSpawnerButtonData( name ) );
			m_ItemSetButtons.Insert( selButton );

			UIActionButton curButton = UIActionManager.CreateButton( buttonswrapper, "#STR_COT_OBJECT_MODULE_CURSOR", this, "SpawnOnCursor" );
			curButton.SetData( new JMItemSpawnerButtonData( name ) );
			m_ItemSetButtons.Insert( curButton );
		}

		m_sclr_MainActions.UpdateScroller();
	}

	void SpawnOnPlayers( UIEvent eid, UIActionBase action ) 
	{
		JMItemSpawnerButtonData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_Module.SpawnPlayers( data.ClassName, JM_GetSelected().GetPlayers() );
	}

	void SpawnOnCursor( UIEvent eid, UIActionBase action ) 
	{
		JMItemSpawnerButtonData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_Module.SpawnPosition( data.ClassName, GetCursorPos() );
	}
};
