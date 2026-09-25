// =============================================================================
//  JMPagedActionMenu
//
//  The paging mechanics behind JMESPActionMenu - the world right-click menu
//  ESP tags and the tracked list raise over whatever the cursor is pointing
//  at - lifted out so a 2D row menu (the object spawner's class list, ...)
//  can be built on the exact same tech instead of re-inventing paging with
//  its own ad hoc page/back state. One UIActionContextMenu, one current page
//  id, rebuilt and reopened on every navigation.
//
//  A subclass:
//    - creates m_Menu (UIActionContextMenu.CreateContextMenu/CreateOverlayMenu)
//    - overrides BuildPage(page) to Add/AddPage/AddBack for that page
//    - calls OpenPage(PAGE_MAIN) to open, and OpenPageAtMouse(PAGE_MAIN) for
//      a right-click menu that follows the cursor
//    - routes its own OnClick_* callback's id through HandlePageClick(id)
//      first; a false return means the id was a command, not a page.
//
//  Usage:
//      class MyRowMenu: JMPagedActionMenu
//      {
//          static const string PAGE_COPY = "copy";
//
//          protected override void BuildPage( string page )
//          {
//              if ( page == PAGE_COPY )
//              {
//                  AddBack();
//                  Add( "copy_raw", "Copy Raw", JMConstants.Lucide( "copy" ) );
//                  return;
//              }
//
//              AddPage( PAGE_COPY, "Copy", JMConstants.Lucide( "copy" ) );
//          }
//
//          void OnClick_MyRowMenu( UIEvent eid, UIActionBase action )
//          {
//              if ( eid != UIEvent.CLICK || !m_Menu )
//                  return;
//
//              string id = m_Menu.GetLastClickedId();
//              if ( HandlePageClick( id ) )
//                  return;
//
//              if ( id == "copy_raw" )
//                  COTFeedback.Copy( ... );
//          }
//      }
// =============================================================================
class JMPagedActionMenu
{
	protected UIActionContextMenu m_Menu;
	protected string m_Page;
	protected float  m_OpenX;
	protected float  m_OpenY;

	//! Page ids route through this prefix, same as JMESPActionMenu's own -
	//! "p:copy" navigates, anything else is a command for the subclass.
	static const string PREFIX_PAGE = "p:";
	static const string PAGE_MAIN   = "main";

	string GetPage()
	{
		return m_Page;
	}

	//! Open (or reopen) at a fixed screen position, starting from `page`.
	void OpenPage( string page, float x, float y )
	{
		m_Page  = page;
		m_OpenX = x;
		m_OpenY = y;

		Show();
	}

	//! Open at the current cursor - what a right-click menu wants. The cursor
	//! is sampled ONCE, here, not on every reopen: a page-navigation click
	//! lands on whatever row the submenu happened to draw at, and re-sampling
	//! the mouse on each Show() moved the menu to chase that row instead of
	//! staying put - the exact bug this fixes. Same fixed-position behaviour
	//! as JMESPActionMenu, whose m_X/m_Y are likewise set once by Open() and
	//! reused by every later GoToPage().
	void OpenPageAtMouse( string page )
	{
		int mx, my;
		GetMousePos( mx, my );

		OpenPage( page, mx, my );
	}

	void Close()
	{
		if ( m_Menu )
			m_Menu.Close();
	}

	//! Rebuild the menu for the current page and reopen it.
	//!
	//! The reopen is not redundant on a page-navigation click:
	//! UIActionContextMenu.OnClick already closes the menu (m_CloseOnClick
	//! defaults true) before the subclass's own click callback ever runs, so
	//! a page row that only rebuilt without this would close the menu and
	//! never show what it navigated to - the exact shape JMESPActionMenu.Show()
	//! already handles by calling OpenAt() on every Show(), page nav included.
	protected void Show()
	{
		if ( !m_Menu )
		{
			Error("[JMPagedActionMenu] Show failed: m_Menu is null!");
			return;
		}

		m_Menu.ClearItems();
		BuildPage( m_Page );

		m_Menu.OpenAt( m_OpenX, m_OpenY );
	}

	//! Override: add this page's rows via Add / AddPage / AddBack below.
	protected void BuildPage( string page )
	{
	}

	//! First thing a subclass's click handler calls. True (and already
	//! navigated) when `id` was a page row; false means the subclass should
	//! treat it as one of its own command ids.
	protected bool HandlePageClick( string id )
	{
		if ( id.IndexOf( PREFIX_PAGE ) != 0 )
			return false;

		m_Page = id.Substring( PREFIX_PAGE.Length(), id.Length() - PREFIX_PAGE.Length() );
		Show();

		return true;
	}

	//! A plain command row.
	protected void Add( string id, string label, string icon, bool enabled = true, int colour = 0 )
	{
		if ( !m_Menu )
		{
			Error("[JMPagedActionMenu] Add failed: m_Menu is null!");
			return;
		}

		if ( !enabled )
			return;

		m_Menu.AddItem( id, label, icon, colour );
	}

	//! A row that opens another page. The trailing chevron is the whole
	//! point - a page and a command look identical without it.
	protected void AddPage( string page, string label, string icon, bool enabled = true )
	{
		if ( !m_Menu )
		{
			Error("[JMPagedActionMenu] AddPage failed: m_Menu is null!");
			return;
		}

		if ( !enabled )
			return;

		m_Menu.AddItem( PREFIX_PAGE + page, label, icon, 0, true );
	}

	//! `page` lets a sub-page of a sub-page go back one level instead of all
	//! the way to Main - see JMESPActionMenu's own AddBack for why.
	protected void AddBack( string page = PAGE_MAIN )
	{
		Add( PREFIX_PAGE + page, "#STR_COT_ESP_MODULE_MENU_BACK", JMConstants.Lucide( "chevron-left" ) );
	}
}
