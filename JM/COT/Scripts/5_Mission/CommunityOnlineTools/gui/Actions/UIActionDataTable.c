// =============================================================================
//  UIActionDataTable
//
//  A multi-column read-only table with a header row and scrollable data rows.
//  Columns are defined by calling SetColumns(); rows are added with AddRow().
//  Clicking a row fires UIEvent.CLICK and GetSelectedRow() returns its index.
//  Call ClearRows() + AddRow() to rebuild the table on data changes.
//
//  Usage:
//      m_Table = UIActionManager.CreateDataTable( parent, this, "OnClick_Row" );
//      m_Table.SetColumns( { "Name", "Status", "Ping" }, { 0.5, 0.3, 0.2 } );
//      foreach ( PlayerInfo p : players )
//          m_Table.AddRow( { p.Name, p.Status, "" + p.Ping } );
//
//      void OnClick_Row( UIEvent eid, UIActionBase action )
//      {
//          int row = m_Table.GetSelectedRow();
//      }
//
//  Right-clicking a row selects it and fires UIEvent.CLICK_RIGHT instead, with
//  the cursor position available from GetLastRightClickX/Y() - enough to open a
//  context menu over the row that was hit:
//
//      void OnClick_Row( UIEvent eid, UIActionBase action )
//      {
//          if ( eid != UIEvent.CLICK_RIGHT )
//              return;
//          m_Menu.ShowAt( m_Table.GetLastRightClickX(), m_Table.GetLastRightClickY() );
//      }
// =============================================================================
class UIActionDataTable: UIActionBase
{
	protected Widget  m_HeaderContainer;
	protected Widget  m_BodyContainer;
	protected Widget  m_HeaderRow;
	protected Widget  m_Body;

	protected ref array<string>  m_ColumnLabels;
	protected ref array<float>   m_ColumnWidths;
	protected ref array<Widget>  m_RowWidgets;

	protected int m_SelectedRow;

	//! Cursor position of the last right-click, in screen pixels. Captured
	//! because the handler that opens a context menu runs after the event has
	//! been dispatched and has no other way back to where the click landed.
	protected int m_LastRightClickX;
	protected int m_LastRightClickY;

	//! Row the pointer is currently over, and where it entered. -1 when the
	//! pointer is not over any row.
	protected int m_HoveredRow;
	protected int m_LastHoverX;
	protected int m_LastHoverY;

	static const int COLOR_ROW_EVEN     = JMTheme.INK_800;
	static const int COLOR_ROW_ODD      = JMTheme.INK_700;
	static const int COLOR_ROW_SELECTED = JMTheme.ACCENT_WASH_STRONG;

	static const int ROW_HEIGHT = 24;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_HeaderContainer, layoutRoot.FindAnyWidget( "action_header_container" ) );
		Class.CastTo( m_BodyContainer,   layoutRoot.FindAnyWidget( "action_body_container"   ) );

		m_ColumnLabels = new array<string>;
		m_ColumnWidths = new array<float>;
		m_RowWidgets   = new array<Widget>;
		m_SelectedRow  = -1;

		m_LastRightClickX = 0;
		m_LastRightClickY = 0;

		m_HoveredRow = -1;
		m_LastHoverX = 0;
		m_LastHoverY = 0;
	}

	//! Define column headers and their fractional widths (should sum to ~1.0).
	void SetColumns( notnull array<string> labels, notnull array<float> widths )
	{
		m_ColumnLabels.Copy( labels );
		m_ColumnWidths.Copy( widths );
		RebuildHeader();
	}

	//! Add a data row. Values map to columns in order.
	void AddRow( notnull array<string> values )
	{
		if ( !m_Body )
			return;

		int idx   = m_RowWidgets.Count();
		int cols  = m_ColumnLabels.Count();
		if ( cols == 0 )
			cols = values.Count();
		int count = values.Count();

		string rowLayout = string.Format(
			"JM/COT/GUI/layouts/uiactions/Wrappers/1/GridSpacer%1.layout",
			Math.Clamp( cols, 1, 9 ) );

		Widget rowWidget = g_Game.GetWorkspace().CreateWidgets( rowLayout, m_Body );
		if ( !rowWidget )
			return;

		int rowColor;
		if ( idx - ( idx / 2 ) * 2 == 0 )
			rowColor = COLOR_ROW_EVEN;
		else
			rowColor = COLOR_ROW_ODD;

		rowWidget.SetColor( rowColor );
		rowWidget.SetHandler( this );

		for ( int c = 0; c < cols; c++ )
		{
			string val = "";
			if ( c < count )
				val = values[c];

			Widget cellWidget = g_Game.GetWorkspace().CreateWidgets(
				"JM/COT/GUI/layouts/uiactions/UIActionText.layout", rowWidget );

			if ( !cellWidget )
				continue;

			UIActionText cell;
			cellWidget.GetScript( cell );
			if ( cell )
			{
				cell.SetLabel( "" );
				cell.SetText( val );
				float w;
				if ( m_ColumnWidths.Count() > c )
					w = m_ColumnWidths[c];
				else
					w = 1.0 / cols;
				SetWidgetWidth( cellWidget, w );
			}
		}

		m_RowWidgets.Insert( rowWidget );
	}

	//! Remove all data rows (keeps header).
	void ClearRows()
	{
		if ( !m_Body )
			return;

		Widget child = m_Body.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}
		m_RowWidgets.Clear();
		m_SelectedRow = -1;
		m_HoveredRow  = -1;
	}

	int GetSelectedRow()
	{
		return m_SelectedRow;
	}

	int GetLastRightClickX()
	{
		return m_LastRightClickX;
	}

	int GetLastRightClickY()
	{
		return m_LastRightClickY;
	}

	int GetHoveredRow()
	{
		return m_HoveredRow;
	}

	int GetLastHoverX()
	{
		return m_LastHoverX;
	}

	int GetLastHoverY()
	{
		return m_LastHoverY;
	}

	//! Rows are built from several widgets, so the pointer crosses a boundary
	//! every time it moves between two cells of the SAME row. Only report a
	//! change when the row index actually changes, or a hover preview would be
	//! torn down and rebuilt mid-row.
	override bool OnMouseEnter( Widget w, int x, int y )
	{
		int idx = RowIndexOf( w );
		if ( idx < 0 )
			return false;

		m_LastHoverX = x;
		m_LastHoverY = y;

		if ( idx == m_HoveredRow )
			return false;

		m_HoveredRow = idx;
		CallEvent( UIEvent.MOUSE_ENTER );
		return true;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( m_HoveredRow < 0 )
			return false;

		// Still inside the same row, just over a different cell of it.
		if ( RowIndexOf( enterW ) == m_HoveredRow )
			return false;

		m_HoveredRow = -1;
		CallEvent( UIEvent.MOUSE_LEAVE );
		return true;
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		int idx = RowIndexOf( w );
		if ( idx < 0 )
			return false;

		SelectRow( idx );
		CallEvent( UIEvent.CLICK );
		return true;
	}

	//! Right-click selects the row under the cursor and reports it separately
	//! from a left click. OnClick never fires for the right button, so the menu
	//! path has to hang off the button-down event.
	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( button != MouseState.RIGHT )
			return super.OnMouseButtonDown( w, x, y, button );

		int idx = RowIndexOf( w );
		if ( idx < 0 )
			return false;

		m_LastRightClickX = x;
		m_LastRightClickY = y;

		SelectRow( idx );
		CallEvent( UIEvent.CLICK_RIGHT );
		return true;
	}

	//! Paint the given row as selected and restore the one that was.
	protected void SelectRow( int idx )
	{
		if ( m_SelectedRow >= 0 && m_SelectedRow < m_RowWidgets.Count() )
		{
			int prev = m_SelectedRow;
			if ( prev - ( prev / 2 ) * 2 == 0 )
				m_RowWidgets[prev].SetColor( COLOR_ROW_EVEN );
			else
				m_RowWidgets[prev].SetColor( COLOR_ROW_ODD );
		}

		m_SelectedRow = idx;

		if ( idx >= 0 && idx < m_RowWidgets.Count() )
			m_RowWidgets[idx].SetColor( COLOR_ROW_SELECTED );
	}

	//! Row index for a row widget or any descendant of one; -1 when the widget
	//! belongs to no row (the header, the body container, the table root).
	protected int RowIndexOf( Widget w )
	{
		foreach ( int i, Widget row : m_RowWidgets )
		{
			if ( w == row || IsChildOf( w, row ) )
				return i;
		}
		return -1;
	}

	private void RebuildHeader()
	{
		if ( !m_HeaderContainer )
			return;

		if ( m_HeaderRow )
			m_HeaderRow.Unlink();

		int cols = m_ColumnLabels.Count();
		if ( cols == 0 )
			return;

		string headerLayout = string.Format(
			"JM/COT/GUI/layouts/uiactions/Wrappers/1/GridSpacer%1.layout",
			Math.Clamp( cols, 1, 9 ) );

		m_HeaderRow = g_Game.GetWorkspace().CreateWidgets( headerLayout, m_HeaderContainer );
		if ( !m_HeaderRow )
			return;

		m_HeaderRow.SetHandler( this );

		// Also create the body GridSpacer now that we know column count
		if ( m_BodyContainer )
		{
			if ( m_Body )
				m_Body.Unlink();

			string bodyLayout = "JM/COT/GUI/layouts/uiactions/Wrappers/1/GridSpacer1.layout";
			m_Body = g_Game.GetWorkspace().CreateWidgets( bodyLayout, m_BodyContainer );
			if ( m_Body )
				m_Body.SetHandler( this );
		}

		for ( int c = 0; c < cols; c++ )
		{
			Widget cellWidget = g_Game.GetWorkspace().CreateWidgets(
				"JM/COT/GUI/layouts/uiactions/UIActionText.layout", m_HeaderRow );

			if ( !cellWidget )
				continue;

			UIActionText cell;
			cellWidget.GetScript( cell );
			if ( cell )
			{
				cell.SetLabel( m_ColumnLabels[c] );
				cell.SetText( "" );
				float w;
				if ( m_ColumnWidths.Count() > c )
					w = m_ColumnWidths[c];
				else
					w = 1.0 / cols;
				SetWidgetWidth( cellWidget, w );
			}
		}
	}

	private bool IsChildOf( Widget w, Widget parent )
	{
		Widget cur = w;
		while ( cur )
		{
			if ( cur == parent )
				return true;
			cur = cur.GetParent();
		}
		return false;
	}
}
