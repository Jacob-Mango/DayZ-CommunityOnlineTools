// =============================================================================
//  UIActionKeyValueList
//
//  A compact read-only two-column info panel built from a string map.
//  Rows are created lazily on the first call and updated in-place on
//  subsequent SetValue() calls - no widget churn on data refreshes.
//
//  Usage:
//      m_Info = UIActionManager.CreateKeyValueList( parent );
//      m_Info.SetValue( "Health",  "100%" );
//      m_Info.SetValue( "Status",  "OK"   );
//      m_Info.SetValue( "Ping",    "42ms" );
//
//      // Later, refresh just one value:
//      m_Info.SetValue( "Ping", "88ms" );
//
//      // Remove a row:
//      m_Info.RemoveKey( "Ping" );
//
//      // Clear everything:
//      m_Info.Clear();
// =============================================================================
class UIActionKeyValueList: UIActionBase
{
	protected Widget m_Rows;

	protected ref array<string>            m_Keys;

	//! Height of one row, matching UIActionText.layout's own `size 1 30`.
	static const int ROW_HEIGHT = 30;
	protected ref map<string, TextWidget>  m_ValueWidgets;
	protected ref map<string, string>      m_ValueStrings;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Rows, layoutRoot.FindAnyWidget( "action_rows" ) );

		m_Keys         = new array<string>;
		m_ValueWidgets = new map<string, TextWidget>;
		m_ValueStrings = new map<string, string>;
	}

	//! Set or update a key/value pair.  Creates a new row if key is new.
	//!
	//! `icon` is only read when the row is created - it is part of the row's
	//! identity, not part of its value, and re-setting it on every refresh
	//! would reload the image ten times a second for nothing.
	void SetValue( string key, string value, string icon = "" )
	{
		if ( m_ValueWidgets.Contains( key ) )
		{
			m_ValueStrings.Set( key, value );
			TextWidget tw = m_ValueWidgets.Get( key );
			if ( tw )
				tw.SetText( value );
			return;
		}

		// Create new row via UIActionText layout
		Widget rowWidget = g_Game.GetWorkspace().CreateWidgets(
			"JM/COT/GUI/layouts/uiactions/UIActionText.layout", m_Rows );

		if ( !rowWidget )
			return;

		UIActionText row;
		rowWidget.GetScript( row );

		if ( row )
		{
			row.SetLabel( key );
			row.SetText( value );

			// Right-hand slot: label - value - icon. The left slot sits between
			// the row edge and the label and crowds it on a narrow panel.
			if ( icon != "" )
				row.SetIconRight( icon );

			// Grab the value TextWidget so we can update it cheaply later
			TextWidget valueTw;
			Class.CastTo( valueTw, rowWidget.FindAnyWidget( "action" ) );
			m_ValueWidgets.Insert( key, valueTw );
			m_ValueStrings.Insert( key, value );
			m_Keys.Insert( key );

			UpdateHeight();
		}
	}

	//! Grow the root to fit the rows.
	//!
	//! UIActionKeyValueList.layout declares the root as `size 1 1` with
	//! `vexactsize 1` - one pixel tall, exactly. The inner action_rows grid does
	//! size to its content, but the root never does, so every row was being
	//! drawn inside a 1px box and the whole control rendered as nothing. Nothing
	//! else sizes it, so it has to size itself as rows come and go.
	protected void UpdateHeight()
	{
		if ( !layoutRoot )
			return;

		float w, h;
		layoutRoot.GetSize( w, h );

		// Measure the rows grid rather than multiplying a row count by an
		// assumed row height: the two disagreed, and the difference came off
		// the BOTTOM of the list, so the last row or two of every list were
		// silently clipped. Ask the widget that actually did the layout.
		float rowsW, rowsH;
		float height = m_Keys.Count() * ROW_HEIGHT;

		if ( m_Rows )
		{
			m_Rows.Update();
			m_Rows.GetScreenSize( rowsW, rowsH );

			if ( rowsH > height )
				height = rowsH;
		}

		layoutRoot.SetFlags( WidgetFlags.VEXACTSIZE, true );
		layoutRoot.SetSize( w, height );
		layoutRoot.Update();
	}

	//! Get the current displayed value for a key ("" if not present).
	string GetValue( string key )
	{
		if ( m_ValueStrings.Contains( key ) )
			return m_ValueStrings.Get( key );
		return "";
	}

	//! Remove a single key/value row.
	void RemoveKey( string key )
	{
		if ( !m_ValueWidgets.Contains( key ) )
			return;

		// Walk children to find and unlink the matching row
		Widget child = m_Rows.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();

			TextWidget valueTw;
			Class.CastTo( valueTw, child.FindAnyWidget( "action" ) );

			if ( valueTw && valueTw == m_ValueWidgets.Get( key ) )
			{
				child.Unlink();
				break;
			}

			child = next;
		}

		m_ValueWidgets.Remove( key );
		m_ValueStrings.Remove( key );
		m_Keys.RemoveItem( key );

		UpdateHeight();
	}

	//! Remove all rows.
	void Clear()
	{
		Widget child = m_Rows.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}

		m_ValueWidgets.Clear();
		m_ValueStrings.Clear();
		m_Keys.Clear();

		UpdateHeight();
	}

	//! Returns all registered keys in insertion order.
	array<string> GetKeys()
	{
		return m_Keys;
	}
}
