// =============================================================================
//  UIActionPaginator
//
//  Prev/Next buttons + "page X / N" label.  The caller sets the total page
//  count and responds to CHANGE events to re-render their list's visible slice.
//  GetCurrentPage() is 0-based internally; display is 1-based.
//
//  Usage:
//      m_Pages = UIActionManager.CreatePaginator( parent, this, "OnChange_Page" );
//      m_Pages.SetPageCount( Math.Ceil( items.Count() / ITEMS_PER_PAGE ) );
//      m_Pages.SetPage( 0 );
//
//      void OnChange_Page( UIEvent eid, UIActionBase action )
//      {
//          int first = m_Pages.GetCurrentPage() * ITEMS_PER_PAGE;
//          RebuildList( first, ITEMS_PER_PAGE );
//      }
// =============================================================================
class UIActionPaginator: UIActionBase
{
	protected ButtonWidget m_BtnPrev;
	protected ButtonWidget m_BtnNext;
	protected TextWidget   m_PageLabel;

	protected int m_CurrentPage;
	protected int m_PageCount;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_BtnPrev,   layoutRoot.FindAnyWidget( "action_button_prev" ) );
		Class.CastTo( m_BtnNext,   layoutRoot.FindAnyWidget( "action_button_next" ) );
		Class.CastTo( m_PageLabel, layoutRoot.FindAnyWidget( "action"             ) );

		m_CurrentPage = 0;
		m_PageCount   = 1;
		UpdateLabel();
		UpdateButtons();
	}

	void SetPageCount( int count )
	{
		m_PageCount   = Math.Max( 1, count );
		m_CurrentPage = Math.Clamp( m_CurrentPage, 0, m_PageCount - 1 );
		UpdateLabel();
		UpdateButtons();
	}

	int GetPageCount()
	{
		return m_PageCount;
	}

	//! Set current page (0-based). Does NOT fire the callback.
	void SetPage( int page, bool sendEvent = false )
	{
		int clamped = Math.Clamp( page, 0, m_PageCount - 1 );
		if ( clamped == m_CurrentPage )
			return;

		m_CurrentPage = clamped;
		UpdateLabel();
		UpdateButtons();

		if ( sendEvent )
			CallEvent( UIEvent.CHANGE );
	}

	int GetCurrentPage()
	{
		return m_CurrentPage;
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == m_BtnPrev && m_CurrentPage > 0 )
		{
			m_CurrentPage--;
			UpdateLabel();
			UpdateButtons();
			CallEvent( UIEvent.CHANGE );
			return true;
		}

		if ( w == m_BtnNext && m_CurrentPage < m_PageCount - 1 )
		{
			m_CurrentPage++;
			UpdateLabel();
			UpdateButtons();
			CallEvent( UIEvent.CHANGE );
			return true;
		}

		return false;
	}

	private void UpdateLabel()
	{
		if ( m_PageLabel )
			m_PageLabel.SetText( string.Format( "%1 / %2", m_CurrentPage + 1, m_PageCount ) );
	}

	private void UpdateButtons()
	{
		if ( m_BtnPrev )
		{
			if ( m_CurrentPage > 0 )
				m_BtnPrev.SetAlpha( 1.0 );
			else
				m_BtnPrev.SetAlpha( 0.35 );
		}

		if ( m_BtnNext )
		{
			if ( m_CurrentPage < m_PageCount - 1 )
				m_BtnNext.SetAlpha( 1.0 );
			else
				m_BtnNext.SetAlpha( 0.35 );
		}
	}
}
