// =============================================================================
//  JMObjectSpawnerRowMenu.c
//
//  Right-click menu for a row in the object spawner's class list. Spawn at
//  cursor / spawn at self are quick actions on the class the right-click
//  landed on. Copy is a page of the same export formats ExportList already
//  produces for the whole list, scoped to just this one classname.
//
//  Built on JMPagedActionMenu - the same paging tech the 3D world context
//  menu (JMESPActionMenu, raised over ESP tags and the tracked list) already
//  uses, rather than a second, separately-maintained page/back implementation
//  for this 2D one.
//
//  The row is already selected by the time this opens (UIActionItemList
//  selects on the press, left or right), so the spawn actions just call the
//  form's existing SpawnObject() rather than re-deriving the classname.
// =============================================================================
class JMObjectSpawnerRowMenu: JMPagedActionMenu
{
	protected JMObjectSpawnerForm m_Form;
	protected string              m_ClassName;

	static const string PAGE_COPY = "copy";

	static const string ACTION_SPAWN_CURSOR = "spawn_cursor";
	static const string ACTION_SPAWN_SELF   = "spawn_self";
	static const string ACTION_COPY_RAW     = "copy_raw";
	static const string ACTION_COPY_TYPES   = "copy_types";
	static const string ACTION_COPY_MARKET  = "copy_market";

	void JMObjectSpawnerRowMenu( JMObjectSpawnerForm form )
	{
		m_Form = form;
	}

	//! Right-click on a class-list row. Opens fresh every time on the main
	//! page - the menu never stays on Copy from whatever the previous row
	//! left it on.
	void Open( string className )
	{
		if ( className == "" )
			return;

		if ( !m_Form )
		{
			Error("[JMObjectSpawnerRowMenu] Open failed: m_Form is null!");
			return;
		}

		if ( !m_Menu )
		{
			m_Menu = UIActionManager.CreateOverlayMenu( m_Form, this, "OnClick_SpawnerRowMenu" );

			if ( !m_Menu )
			{
				Error("[JMObjectSpawnerRowMenu] Open failed: Could not create UIActionContextMenu (m_Menu is null)!");
				return;
			}
		}

		m_ClassName = className;

		OpenPageAtMouse( PAGE_MAIN );
	}

	protected override void BuildPage( string page )
	{
		if ( page == PAGE_COPY )
		{
			BuildCopyPage();
			return;
		}

		BuildMainPage();
	}

	protected void BuildMainPage()
	{
		Add( ACTION_SPAWN_CURSOR, "#STR_COT_OBJECT_MODULE_SPAWN_AT_CROSSHAIR", JMConstants.Lucide( "crosshair" ) );
		Add( ACTION_SPAWN_SELF,   "#STR_COT_OBJECT_MODULE_SPAWN_AT_PLAYER",    JMConstants.Lucide( "user" ) );

		AddPage( PAGE_COPY, "#STR_COT_OBJECT_MODULE_ROWMENU_COPY", JMConstants.Lucide( "copy" ) );
	}

	protected void BuildCopyPage()
	{
		AddBack();

		Add( ACTION_COPY_RAW,   "#STR_COT_OBJECT_MODULE_ROWMENU_COPY_CLASSNAME", JMConstants.Lucide( "copy" ) );
		Add( ACTION_COPY_TYPES, "#STR_COT_OBJECT_MODULE_ROWMENU_COPY_TYPES",     JMConstants.Lucide( "file-code" ) );

#ifdef DZ_Expansion_Market
		Add( ACTION_COPY_MARKET, "#STR_COT_OBJECT_MODULE_ROWMENU_COPY_MARKET", JMConstants.Lucide( "shopping-cart" ) );
#endif
	}

	void OnClick_SpawnerRowMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_Menu )
		{
			Error("[JMObjectSpawnerRowMenu] OnClick_SpawnerRowMenu failed: m_Menu is null!");
			return;
		}

		string id = m_Menu.GetLastClickedId();

		if ( HandlePageClick( id ) )
			return;

		if ( m_ClassName == "" || !m_Form )
			return;

		if ( id == ACTION_SPAWN_CURSOR )
		{
			m_Form.SpawnObject( COT_ObjectSpawnerMode.CURSOR );
			return;
		}

		if ( id == ACTION_SPAWN_SELF )
		{
			m_Form.SpawnObject( COT_ObjectSpawnerMode.PLAYER_POSITION );
			return;
		}

		if ( id == ACTION_COPY_RAW )
		{
			COTFeedback.Copy( m_ClassName );
			return;
		}

		if ( id == ACTION_COPY_TYPES )
		{
			COTFeedback.Copy( m_Form.BuildTypesXmlEntry( m_ClassName ) );
			return;
		}

#ifdef DZ_Expansion_Market
		if ( id == ACTION_COPY_MARKET )
		{
			m_Form.CopyMarketEntry( m_ClassName );
			return;
		}
#endif
	}
}
