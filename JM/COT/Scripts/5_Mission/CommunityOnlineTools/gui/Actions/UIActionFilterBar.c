// =============================================================================
//  UIActionFilterBar
//
//  A horizontal row of labeled toggle-buttons acting as filters.
//  Two modes:
//    - Exclusive  (default): only one filter active at a time (like tabs).
//    - Multi-select: any combination of filters can be active.
//
//  Fires UIEvent.CHANGE on every toggle.  Use GetActiveFilters() to read
//  which labels are currently active, or GetActiveIndices() for indices.
//
//  Usage:
//      m_Filter = UIActionManager.CreateFilterBar( parent,
//          { "All", "Online", "Offline", "Banned" }, this, "OnChange_Filter" );
//      m_Filter.SetActive( 0 );   // default to "All"
//
//      void OnChange_Filter( UIEvent eid, UIActionBase action )
//      {
//          array<string> active = m_Filter.GetActiveFilters();
//      }
// =============================================================================
class UIActionFilterBar: UIActionBase
{
	protected Widget m_Bar;
	protected Widget m_BarParent;

	protected ref array<string>        m_Labels;
	protected ref array<ButtonWidget>  m_Buttons;
	protected ref array<Widget>        m_Fills;      // the coloured fill image per button
	protected ref array<Widget>        m_Outlines;   // ring, drawn on the active chip only
	protected ref array<bool>          m_Active;

	protected bool m_MultiSelect;
	protected int  m_Hovered;   // -1 = no filter hovered

	//! A chip is the same rounded pill as a button, and an ACTIVE chip is the
	//! same pill as the selected tab - see JMTheme for why the two ladders are
	//! kept apart. The ring marks the active chip, exactly as it does in a tab
	//! bar, so "which filter am I on" is answerable at a glance.
	static const int COLOR_ON            = JMTheme.SELECTED_FILL;
	static const int COLOR_OFF           = JMTheme.BUTTON_FILL;
	static const int COLOR_HOVER         = JMTheme.BUTTON_FILL_HOVER;
	static const int COLOR_OUTLINE_ON    = JMTheme.SELECTED_OUTLINE;
	static const int COLOR_OUTLINE_HOVER = JMTheme.BUTTON_OUTLINE_HOVER;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_BarParent, layoutRoot.FindAnyWidget( "action_wrapper" ) );

		m_Labels      = new array<string>;
		m_Buttons     = new array<ButtonWidget>;
		m_Fills       = new array<Widget>;
		m_Outlines    = new array<Widget>;
		m_Active      = new array<bool>;
		m_MultiSelect = false;
		m_Hovered     = -1;
	}

	//! Build filter buttons from label array.  Clears existing buttons.
	void SetFilters( notnull array<string> labels )
	{
		if ( !m_BarParent )
			return;

		if ( m_Bar )
			m_Bar.Unlink();

		m_Labels.Clear();
		m_Buttons.Clear();
		m_Fills.Clear();
		m_Outlines.Clear();
		m_Active.Clear();

		int count = labels.Count();
		if ( count == 0 )
			return;

		int cols = Math.Clamp( count, 1, 9 );
		string gridLayout = string.Format(
			"JM/COT/GUI/layouts/uiactions/Wrappers/TabBar/TabBar%1.layout", cols );

		m_Bar = g_Game.GetWorkspace().CreateWidgets( gridLayout, m_BarParent );
		if ( !m_Bar )
			return;

		m_Bar.SetHandler( this );

		foreach ( int i, string label : labels )
		{
			m_Labels.Insert( label );
			m_Active.Insert( false );

			Widget btnWidget = g_Game.GetWorkspace().CreateWidgets(
				"JM/COT/GUI/layouts/uiactions/UIActionFilterButton.layout", m_Bar );

			if ( !btnWidget )
			{
				m_Buttons.Insert( null );
				m_Fills.Insert( null );
				m_Outlines.Insert( null );
				continue;
			}

			btnWidget.SetHandler( this );

			ButtonWidget btn;
			Class.CastTo( btn, btnWidget );

			TextWidget txt;
			Class.CastTo( txt, btnWidget.FindAnyWidget( "label" ) );

			if ( txt )
				txt.SetText( Widget.TranslateString( label ) );

			Widget fill = btnWidget.FindAnyWidget( "fill" );
			Widget outline = btnWidget.FindAnyWidget( "outline" );

			m_Buttons.Insert( btn );
			m_Fills.Insert( fill );
			m_Outlines.Insert( outline );
		}

		UpdateColors();
	}

	//! Allow multiple filters active simultaneously (default: false = exclusive).
	void SetMultiSelect( bool multi )
	{
		m_MultiSelect = multi;
	}

	//! Activate filter by index, optionally firing callback.
	void SetActive( int index, bool sendEvent = false )
	{
		if ( index < 0 || index >= m_Labels.Count() )
			return;

		if ( !m_MultiSelect )
		{
			for ( int i = 0; i < m_Active.Count(); i++ )
				m_Active[i] = false;
		}

		m_Active[index] = true;
		UpdateColors();

		if ( sendEvent )
			CallEvent( UIEvent.CHANGE );
	}

	//! Deactivate filter by index.
	void SetInactive( int index, bool sendEvent = false )
	{
		if ( index < 0 || index >= m_Active.Count() )
			return;

		m_Active[index] = false;
		UpdateColors();

		if ( sendEvent )
			CallEvent( UIEvent.CHANGE );
	}

	bool IsActive( int index )
	{
		if ( index < 0 || index >= m_Active.Count() )
			return false;
		return m_Active[index];
	}

	array<string> GetActiveFilters()
	{
		array<string> result = new array<string>;
		for ( int i = 0; i < m_Labels.Count(); i++ )
			if ( m_Active[i] ) result.Insert( m_Labels[i] );
		return result;
	}

	array<int> GetActiveIndices()
	{
		array<int> result = new array<int>;
		for ( int i = 0; i < m_Active.Count(); i++ )
			if ( m_Active[i] ) result.Insert( i );
		return result;
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		super.OnMouseEnter( w, x, y );
		int idx = ButtonIndexOf( w );
		if ( idx >= 0 )
		{
			m_Hovered = idx;
			UpdateColors();
		}
		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		super.OnMouseLeave( w, enterW, x, y );
		int idx = ButtonIndexOf( w );
		if ( idx >= 0 && idx == m_Hovered )
		{
			m_Hovered = -1;
			UpdateColors();
		}
		return false;
	}

	protected int ButtonIndexOf( Widget w )
	{
		foreach ( int i, ButtonWidget btn : m_Buttons )
		{
			if ( btn && w == btn )
				return i;
		}
		return -1;
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		foreach ( int i, ButtonWidget btn : m_Buttons )
		{
			if ( btn && w == btn )
			{
				if ( m_MultiSelect )
				{
					m_Active[i] = !m_Active[i];
				}
				else
				{
					for ( int j = 0; j < m_Active.Count(); j++ )
						m_Active[j] = false;
					m_Active[i] = true;
				}

				UpdateColors();
				CallEvent( UIEvent.CHANGE );
				return true;
			}
		}
		return false;
	}

	private void UpdateColors()
	{
		for ( int i = 0; i < m_Fills.Count(); i++ )
		{
			int color = COLOR_OFF;
			int ring  = COLOR_OUTLINE_HOVER;

			if ( m_Active[i] )
			{
				color = COLOR_ON;
				ring  = COLOR_OUTLINE_ON;
			}
			else if ( i == m_Hovered )
			{
				color = COLOR_HOVER;
			}

			if ( m_Fills[i] )
				m_Fills[i].SetColor( color );

			// The ring is the "on" marker; a hovered chip borrows it briefly so
			// the pointer has something to land on.
			Widget outline = m_Outlines[i];
			if ( outline )
			{
				outline.SetColor( ring );
				outline.Show( m_Active[i] || i == m_Hovered );
			}
		}
	}
}
