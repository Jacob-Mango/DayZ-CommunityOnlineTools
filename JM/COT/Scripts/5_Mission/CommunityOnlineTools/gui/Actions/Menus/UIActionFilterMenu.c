//  UIActionFilterMenu: one UIActionContextMenu driven as a stack of named PAGES (filter button
//  next to a search box); a drill-down is a page, never a second overlay menu (that went dead to clicks).
//  AddItem()/ClearItems() recreate row widgets; doing that inside the row's own click corrupts hit-testing,
//  so Refresh()/GoToPage() defer via CALL_CATEGORY_GUI.CallLater. Rows from JMFilterRegistry show on the root page (SetRegistryScope).

class JMFilterMenuRow
{
	string Id;
	string GotoPage;

	//! Stay open after this row is picked even though its page closes on pick - for a checkbox
	//! that sits among commands.
	bool KeepOpen;
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

		//! A picked row does not close the menu by default (rebuilds in place, as toggle rows want).
		//! SetPageCloseOnPick() opts a page (the spawner's radio pages) out.
		m_Menu.SetCloseOnClick( false );
	}

	//! `BuildFn( UIActionFilterMenu menu )` runs on every redraw and must call menu.AddRow() per row
	//! (Back/Toggle-All are automatic). `ChangeFn( string id )` gets the picked row id or ID_TOGGLE_ALL
	//! and owns mutating state. The first page added becomes root unless SetRootPage() says otherwise.
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

	//! Opens flush under `button`, always on the root page; closes if already open (toggle).
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

	//! Mark a row just added on a close-on-pick page as one that leaves the menu open when picked.
	void KeepRowOpen( string id )
	{
		JMFilterMenuRow row = FindRow( id );

		if ( row )
			row.KeepOpen = true;
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

		JMFilterMenuRow picked = FindRow( id );
		bool keepOpen = picked && picked.KeepOpen;

		if ( page.CloseOnPick && !keepOpen )
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
