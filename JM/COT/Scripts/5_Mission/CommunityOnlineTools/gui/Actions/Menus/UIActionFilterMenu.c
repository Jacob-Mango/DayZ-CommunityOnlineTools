// =============================================================================
//  UIActionFilterMenu
//
//  A single UIActionContextMenu driven as a small stack of named PAGES, for
//  the "filter button next to a search box, opens Categories/toggles, one of
//  which drills into a list of checkboxes" shape used by the ESP tracked-
//  object list, the teleport list and the object spawner. All three used to
//  hand-roll this with a SECOND UIActionContextMenu for the drill-down
//  (locations/vehicles/players, or a category group) - two live overlay
//  menus sharing one anchor, which is what left the drill-down menu dead to
//  clicks and hover until the whole COT window was closed and reopened. This
//  class never opens a second menu; a drill-down is just another page of the
//  same one.
//
//  It also owns the other half of that same bug class: UIActionContextMenu.
//  AddItem()/ClearItems() destroy and recreate every row widget, and doing
//  that from inside the row's OWN click handler - which every one of the
//  three callers used to do, rebuilding the very menu whose click was still
//  on the call stack - corrupts the engine's hit-test state the same way.
//  Every page switch and toggle here goes through Refresh()/GoToPage(),
//  which defer the rebuild with `CALL_CATEGORY_GUI.CallLater` so it runs
//  after the click has finished unwinding rather than inside it.
//
//  A page is built by a caller-supplied callback that calls AddRow() back
//  into this menu - the same Target/Callback shape UIActionContextMenu's own
//  JMContextMenuEntry already carries (see its Target/Callback fields) - so
//  the row's icon/colour/enabled state stays 100% caller-decided (a
//  checkbox for teleport, the category's own glyph for ESP, an accent tint
//  for the spawner's radio selection) and this class never has to know
//  which presentation convention a given page is using.
//
//  Rows a mod registers with JMFilterRegistry appear on the ROOT page of any menu that names a
//  scope (SetRegistryScope), and their callbacks run when clicked - so every filter menu in COT
//  is extensible the same way, without the mod overriding the form's page builder.
//
//  Usage (from a form; the helper also registers the menu as one of the form's overlays):
//      m_FilterMenu = UIActionManager.CreateOverlayFilterMenu( this, m_FilterButton.GetLayoutRoot(), JMFilterRegistry.OBJECTS );
//      m_FilterMenu.AddPage( "root", this, "BuildRootPage", "", false, "", this, "OnRootPageChange" );
//      ...
//      m_FilterMenu.ToggleAt( m_FilterButton.GetLayoutRoot() );
//
//  or, when you manage parent and anchor yourself:
//      m_FilterMenu = UIActionManager.CreateFilterMenu( parent, anchor );
//      m_FilterMenu.AddPage( "root", this, "BuildRootPage", "", false, "", this, "OnRootPageChange" );
//      m_FilterMenu.AddPage( "categories", this, "BuildCategoriesPage", "root", true, "#STR_...", this, "OnCategoriesChange" );
//      m_FilterMenu.SetRootPage( "root" );
//
//      void BuildRootPage( UIActionFilterMenu menu )
//      {
//          menu.AddRow( "categories", "Categories", icon, color, true, "categories" ); // drill-down
//          menu.AddRow( "selected", "Selected only", UIActionFilterMenu.CheckIcon( m_SelectedOnly ), UIActionFilterMenu.ToggleTextColor( m_SelectedOnly ) );
//      }
//
//      void OnRootPageChange( string id )
//      {
//          if ( id == "selected" )
//              m_SelectedOnly = !m_SelectedOnly;
//      }
// =============================================================================

class JMFilterMenuRow
{
	string Id;
	string GotoPage;
}

class JMFilterMenuPage
{
	string Key;
	string BackToPage;
	bool   ShowToggleAll;
	string ToggleAllLabel;
	bool   CloseOnPick;
	Class  BuildTarget;
	string BuildFn;
	Class  ChangeTarget;
	string ChangeFn;
}

class UIActionFilterMenu
{
	//! Reserved row ids - a caller's own row id must never collide with
	//! these. Both are short enough, and prefixed, that a category/role/type
	//! id from any of the three current callers cannot reach them.
	static const string ID_BACK       = "__filtermenu_back";
	static const string ID_TOGGLE_ALL = "__filtermenu_toggleall";
	protected ref UIActionContextMenu m_Menu;
	protected ref map<string, ref JMFilterMenuPage> m_Pages;
	protected ref array<ref JMFilterMenuRow> m_CurrentRows;
	protected string m_RootPage;
	protected string m_CurrentPage;

	//! JMFilterRegistry scope whose entries are appended to the root page ("" = none).
	protected string m_RegistryScope;

	//! UIActionFilterMenu is a plain class, not itself a
	//! COT_ScriptedWidgetEventHandler, so it cannot be handed to
	//! JMFormBase.AddOverlay directly - the inner menu it wraps can be:
	//! `m_Form.AddOverlay( m_FilterMenu.GetInnerMenu() )`.
	UIActionContextMenu GetInnerMenu()
	{
		return m_Menu;
	}

	bool IsOpen()
	{
		return m_Menu && m_Menu.IsOpen();
	}

	protected bool IsRegistryPage( JMFilterMenuPage page )
	{
		return m_RegistryScope != "" && page.Key == m_RootPage;
	}

	void SetMenuWidth( float pixels )
	{
		if ( m_Menu )
			m_Menu.SetMenuWidth( pixels );
	}

	void SetOwnerWidget( Widget owner )
	{
		if ( m_Menu )
			m_Menu.SetOwnerWidget( owner );
	}

	//! Append the rows registered under `scope` in JMFilterRegistry to the root page and
	//! dispatch their clicks to the registered callbacks.
	void SetRegistryScope( string scope )
	{
		m_RegistryScope = scope;
	}

	void SetRootPage( string key )
	{
		m_RootPage = key;
	}

	void InitFilterMenu( notnull Widget parent, notnull Widget anchor )
	{
		m_Pages = new map<string, ref JMFilterMenuPage>;

		m_Menu = UIActionManager.CreateContextMenu( parent, anchor, this, "OnClick_InnerMenu" );

		if ( !m_Menu )
		{
			Error("[UIActionFilterMenu] InitFilterMenu failed: Could not create inner UIActionContextMenu (m_Menu is null)!");
			return;
		}

		//! A row picking a value does not close the menu here by default -
		//! ToggleAtButton/GoToPage/Refresh all rebuild in place instead, the
		//! same "stays open, repaints" behaviour every existing caller wants
		//! for its toggle rows. SetPageCloseOnPick() opts a page (the
		//! spawner's radio pages) out of that.
		m_Menu.SetCloseOnClick( false );
	}

	//! `buildTarget`/`buildFn` is called as `void BuildFn( UIActionFilterMenu menu )`
	//! every time this page needs to redraw; it must call menu.AddRow() for
	//! each row (Back/Toggle-All rows are added automatically, not by the
	//! callback). `changeTarget`/`changeFn` is called as `void ChangeFn( string id )`
	//! - `id` is the clicked row's id, or ID_TOGGLE_ALL - whenever a leaf row
	//! on this page is picked; the callback owns mutating whatever state
	//! that row represents. The first page added becomes the root page
	//! unless SetRootPage() says otherwise.
	void AddPage( string key, Class buildTarget, string buildFn, string backToPage = "", bool showToggleAll = false, string toggleAllLabel = "#STR_COT_TELEPORT_MODULE_TOGGLE_ALL", Class changeTarget = null, string changeFn = "", bool closeOnPick = false )
	{
		JMFilterMenuPage page = new JMFilterMenuPage();
		page.Key            = key;
		page.BuildTarget     = buildTarget;
		page.BuildFn         = buildFn;
		page.BackToPage      = backToPage;
		page.ShowToggleAll   = showToggleAll;
		page.ToggleAllLabel  = toggleAllLabel;
		page.ChangeTarget    = changeTarget;
		page.ChangeFn        = changeFn;
		page.CloseOnPick     = closeOnPick;

		m_Pages.Insert( key, page );

		if ( m_RootPage == "" )
			m_RootPage = key;
	}

	//! Remove a page. If it was the root page the first remaining page becomes the root.
	void RemovePageById( string key )
	{
		if ( !m_Pages.Contains( key ) )
			return;

		m_Pages.Remove( key );

		if ( m_RootPage == key )
		{
			m_RootPage = "";

			if ( m_Pages.Count() > 0 )
				m_RootPage = m_Pages.GetKey( 0 );
		}

		if ( m_CurrentPage == key )
			m_CurrentPage = m_RootPage;
	}

	string CurrentPage()
	{
		return m_CurrentPage;
	}

	void Close()
	{
		if ( m_Menu )
			m_Menu.Close();
	}

	//! Opens flush under `button`'s own rect, always back on the root page -
	//! a menu that reopens mid-drill-down from last time is surprising, not
	//! useful. Closes instead if already open, so the button doubles as its
	//! own toggle.
	//! DEPRECATED - use ToggleAt
	void ToggleAtButton( notnull Widget button )
	{
		JMDeprecated.WarnOnce( this, "UIActionFilterMenu.ToggleAtButton() is deprecated. Please use ToggleAt()." );

		ToggleAt( button );
	}

	void ToggleAt( notnull Widget button )
	{
		if ( !m_Menu )
		{
			Error("[UIActionFilterMenu] ToggleAt failed: m_Menu is null!");
			return;
		}

		if ( m_Menu.IsOpen() )
		{
			Close();
			return;
		}

		m_CurrentPage = m_RootPage;
		RebuildCurrentPage();

		float bx, by, bw, bh;
		button.GetScreenPos( bx, by );
		button.GetScreenSize( bw, bh );

		m_Menu.OpenAt( bx, by + bh );
	}

	//! Switches page and redraws. Deferred like Refresh() - see the class
	//! note on why a page switch can never rebuild synchronously from
	//! inside this same menu's own click handler.
	void GoToPage( string key )
	{
		m_CurrentPage = key;
		DeferRebuild();
	}

	//! Redraws the current page - call after a caller's ChangeFn mutates
	//! whatever state a row's icon/colour depends on, unless that page
	//! closes on pick (then there is nothing left to redraw).
	void Refresh()
	{
		DeferRebuild();
	}

	//! In-place repaint of one row, no rebuild - the cheap path for a
	//! caller that already knows exactly which row changed and would
	//! rather not tear down and recreate every row on the page for it.
	void MarkRow( string id, string icon, int textColor )
	{
		if ( !m_Menu )
		{
			Error("[UIActionFilterMenu] MarkRow failed: m_Menu is null!");
			return;
		}

		m_Menu.SetItemIcon( id, icon );
		m_Menu.SetItemTextColor( id, textColor );
	}

	//! Called by a page's BuildFn once per row. `gotoPage` non-empty makes
	//! this a drill-down row (draws the chevron, never reaches ChangeFn -
	//! it switches page instead).
	void AddRow( string id, string label, string icon = "", int textColor = 0, bool enabled = true, string gotoPage = "" )
	{
		if ( !m_Menu )
		{
			Error("[UIActionFilterMenu] AddRow failed: m_Menu is null!");
			return;
		}

		JMFilterMenuRow row = new JMFilterMenuRow();
		row.Id       = id;
		row.GotoPage = gotoPage;

		m_CurrentRows.Insert( row );

		m_Menu.AddItem( id, label, icon, textColor, gotoPage != "" );

		if ( !enabled )
			m_Menu.SetItemEnabled( id, false );
	}

	//! A checkbox row: `on` picks the tick and the text colour. With an `icon` the row keeps its
	//! own glyph and only dims when off - "the row IS the checkbox".
	void AddToggleRow( string id, string label, bool on, string icon = "", bool enabled = true )
	{
		string rowIcon = icon;
		if ( rowIcon == "" )
			rowIcon = CheckIcon( on );

		AddRow( id, label, rowIcon, ToggleTextColor( on ), enabled );
	}

	//! Rows mods registered for this menu's scope - added after the page's own rows.
	protected void AddRegisteredRows()
	{
		array< ref JMFilterEntry > entries = JMFilterRegistry.Get( m_RegistryScope );

		foreach ( JMFilterEntry entry : entries )
		{
			if ( entry.m_StateCallback != "" )
			{
				AddToggleRow( entry.m_Id, entry.m_Label, entry.IsOn() );
				continue;
			}

			string icon = "";
			if ( entry.m_Icon != "" )
				icon = JMConstants.Lucide( entry.m_Icon );

			AddRow( entry.m_Id, entry.m_Label, icon, entry.m_Color );
		}
	}

	protected void DeferRebuild()
	{
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( RebuildCurrentPage );
	}

	protected void RebuildCurrentPage()
	{
		if ( !m_Menu )
		{
			Error("[UIActionFilterMenu] RebuildCurrentPage failed: m_Menu is null!");
			return;
		}

		JMFilterMenuPage page = m_Pages.Get( m_CurrentPage );
		if ( !page )
		{
			Error("[UIActionFilterMenu] RebuildCurrentPage failed: page '" + m_CurrentPage + "' not found!");
			return;
		}

		m_Menu.ClearItems();
		m_CurrentRows = new array<ref JMFilterMenuRow>;

		if ( page.BackToPage != "" )
			m_Menu.AddItem( ID_BACK, "#STR_COT_GENERIC_BACK", JMConstants.Lucide( "arrow-left" ) );

		if ( page.ShowToggleAll )
			m_Menu.AddItem( ID_TOGGLE_ALL, page.ToggleAllLabel, JMConstants.ICON_SELECT_ALL );

		if ( page.BuildTarget && page.BuildFn != "" )
			GetGame().GameScript.CallFunctionParams( page.BuildTarget, page.BuildFn, null, new Param1<UIActionFilterMenu>( this ) );

		if ( IsRegistryPage( page ) )
			AddRegisteredRows();
	}

	protected JMFilterMenuRow FindRow( string id )
	{
		if ( !m_CurrentRows )
			return null;

		foreach ( JMFilterMenuRow row : m_CurrentRows )
		{
			if ( row.Id == id )
				return row;
		}

		return null;
	}

	void OnClick_InnerMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_Menu )
		{
			Error("[UIActionFilterMenu] OnClick_InnerMenu failed: m_Menu is null!");
			return;
		}

		string id = m_Menu.GetLastClickedId();

		JMFilterMenuPage page = m_Pages.Get( m_CurrentPage );
		if ( !page )
			return;

		if ( id == ID_BACK )
		{
			GoToPage( page.BackToPage );
			return;
		}

		if ( id != ID_TOGGLE_ALL )
		{
			JMFilterMenuRow row = FindRow( id );

			if ( row && row.GotoPage != "" )
			{
				GoToPage( row.GotoPage );
				return;
			}
		}

		JMFilterEntry registered;
		if ( IsRegistryPage( page ) )
			registered = JMFilterRegistry.Find( m_RegistryScope, id );

		if ( registered && registered.m_Target && registered.m_Callback != "" )
			GetGame().GameScript.CallFunctionParams( registered.m_Target, registered.m_Callback, null, new Param1<string>( id ) );
		else if ( page.ChangeTarget && page.ChangeFn != "" )
			GetGame().GameScript.CallFunctionParams( page.ChangeTarget, page.ChangeFn, null, new Param1<string>( id ) );

		if ( page.CloseOnPick )
			Close();
		else
			Refresh();
	}

	static string CheckIcon( bool on )
	{
		if ( on )
			return JMConstants.Lucide( "square-check" );

		return JMConstants.Lucide( "square" );
	}

	static int ToggleTextColor( bool on )
	{
		if ( on )
			return JMTheme.TEXT_PRIMARY;

		return JMTheme.TEXT_DISABLED;
	}
}
