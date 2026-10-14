//  JMPagedActionMenu - paging mechanics lifted out of JMESPActionMenu: one
//  UIActionContextMenu, one page id, rebuilt and reopened per navigation. A
//  subclass creates m_Menu, overrides BuildPage(page), opens via OpenPage[AtMouse]
//  and routes click ids through HandlePageClick(id) (false = a command).
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

	//! Open at the current cursor (right-click menu). The cursor is sampled ONCE,
	//! not per reopen: a page-navigation click lands on whichever row the submenu
	//! drew, and re-sampling made the menu chase it. Same as JMESPActionMenu.
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

	//! Rebuild the menu for the current page and reopen it. The reopen is needed:
	//! UIActionContextMenu.OnClick closes the menu (m_CloseOnClick defaults true)
	//! before the subclass callback runs, so a rebuild alone would show nothing
	//! (same as JMESPActionMenu.Show()).
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
