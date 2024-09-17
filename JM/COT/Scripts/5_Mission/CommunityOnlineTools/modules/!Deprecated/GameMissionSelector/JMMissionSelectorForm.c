class JMMissionSelectorForm: JMFormBase
{
	private Widget m_ActionsWrapper;
	private Widget m_ActionsFilterWrapper;

	private UIActionEditableText m_Filter;
	private UIActionEditableText m_SelectedMission;

	private TextListboxWidget m_MissionsList;

	private string m_PathToMissions;
	
	private JMMissionSelectorModule m_Module;

	protected override bool SetModule(  JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}
	
	override void OnInit()
	{
		super.OnInit();
		
		m_ActionsFilterWrapper = layoutRoot.FindAnyWidget( "actions_filter_wrapper" );

		m_Filter = UIActionManager.CreateEditableText( m_ActionsFilterWrapper, "Filter: ", this, "Type_UpdateList" );

		m_MissionsList = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("game_missions_list") );

		m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );

		m_SelectedMission = UIActionManager.CreateEditableText( m_ActionsWrapper, "Mission: ", this, "Type_SelectedMission" );

		if ( IsMissionOffline() )
			UIActionManager.CreateButton( m_ActionsWrapper, "Load Mission", this, "LoadMission" );

		UpdateList();
	}

	void Type_UpdateList( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateList();
	}

	void Type_SelectedMission( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
	}

	void UpdateList()
	{
		m_MissionsList.ClearItems();

		string filter = "" + m_Filter.GetText();
		filter.ToLower();

		TStringArray missions = GetMissionList();
		if ( !missions || missions.Count() == 0 )
			return;

		for ( int i = 0; i < missions.Count(); i++ )
		{
			string name = "" + missions[i];
			name.ToLower();

			if ( (filter != "" && (!name.Contains( filter ))) ) 
			{
				continue;
			}

			m_MissionsList.AddItem( missions[i], NULL, 0 );
		}
	}

	private TStringArray GetMissionList()
	{
		string file_name;
		int file_attr;
		int flags;
		TStringArray list = new TStringArray;
		
		m_PathToMissions = "missions";
		
		string path_find_pattern = m_PathToMissions + "\\*";
		FindFileHandle file_handler = FindFile( path_find_pattern, file_name, file_attr, flags );
		
		bool found = true;
		while ( found )
		{				
			if ( file_attr & FileAttr.DIRECTORY )
			{
				list.Insert( file_name );
			}
			
			found = FindNextFile( file_handler, file_name, file_attr );
		}
		
		return list;
	}

	string GetCurrentSelectedMissionName()
	{
		if ( m_MissionsList.GetSelectedRow() != -1 )
		{
			string mission_name;
			m_MissionsList.GetItemText( m_MissionsList.GetSelectedRow(), 0, mission_name );
			return mission_name;
		}

		return "";
	}

	override bool OnItemSelected( Widget w, int x, int y, int row, int column, int oldRow, int oldColumn )
	{
		m_SelectedMission.SetText( GetCurrentSelectedMissionName() );

		return true;
	}
	
	void LoadMission( UIEvent eid, UIActionBase action ) 
	{
		string mission_name = m_SelectedMission.GetText();
		if ( mission_name != "" )
		{
			string mission_path = m_PathToMissions + "\\" + mission_name;
					
			GetGame().PlayMission( mission_path );
		}
	}
};