// =============================================================================
//  UIActionSearchBox
//
//  General-purpose search / filter / autocomplete text input.
//    - Optional left-hand label (SetLabel)
//    - Greyed placeholder inside the field when empty (SetPlaceholder)
//    - Autocomplete suggestion via SetTextPreview() - committed on TAB / ENTER
//    - Inline x clear button (visible while the field has text)
//    - Debounced UIEvent.CHANGE (~100ms) on every real text change
//    - UIEvent.CLICK when the x is pressed (text already cleared)
//
//  Exposes the inner widgets (label, edit box, preview box) so callers can
//  still tweak per-widget sizing via UIActionBase.SetWidgetWidth().
// =============================================================================
class UIActionSearchBox: UIActionBase
{
	protected TextWidget     m_Label;
	protected EditBoxWidget  m_EditBox;
	//! Visible field wrapper - see UIActionEditableText.SetEditBoxWidth.
	protected Widget         m_Chrome;
	protected EditBoxWidget  m_PreviewBox;
	protected ButtonWidget   m_ClearBtn;
	protected ImageWidget    m_ClearIcon;
	protected Widget         m_ClearHover;
	protected TextWidget     m_PlaceholderWidget;
	//! Only present in the UIActionDropdownList layout. Shares the right-hand
	//! slot with the clear button, so exactly one of the two is ever up.
	protected ButtonWidget   m_ChevronBtn;
	protected ImageWidget    m_ChevronIcon;
	protected Widget         m_ChevronHover;

	protected string m_PlaceholderText;
	protected string m_PreviewText;
	protected string m_PreviousText;

	// ---- Suggestion list (optional, enabled by InitSuggestionList) ----------
	protected Widget             m_ListAnchor;
	protected Widget             m_ListPanel;
	protected GridSpacerWidget   m_List;
	protected ref array<string>  m_Suggestions;
	protected bool               m_ListOpen;
	protected float              m_OpenDelay;
	protected ref JMAnimFloat    m_AnimT;
	protected float              m_FullHeight;
	protected int                m_MaxVisible;
	protected bool               m_AutoFilter;
	//! Ordinal (position among the rows currently built), -1 when nothing hovered.
	protected int                m_HoveredRow;
	//! Index into m_Suggestions of the committed entry, -1 when none.
	protected int                m_SelectedIndex;

	static const int CHANGE_DEBOUNCE_MS = 100;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Label,              layoutRoot.FindAnyWidget( "action_label"        ) );
		Class.CastTo( m_EditBox,            layoutRoot.FindAnyWidget( "action"              ) );
		m_Chrome = layoutRoot.FindAnyWidget( "action_chrome" );
		Class.CastTo( m_PreviewBox,         layoutRoot.FindAnyWidget( "action_preview"      ) );
		Class.CastTo( m_ClearBtn,           layoutRoot.FindAnyWidget( "action_button_clear" ) );
		Class.CastTo( m_ClearIcon,          layoutRoot.FindAnyWidget( "action_clear_icon"   ) );
		m_ClearHover = layoutRoot.FindAnyWidget( "action_clear_hover" );
		Class.CastTo( m_PlaceholderWidget,  layoutRoot.FindAnyWidget( "action_placeholder"  ) );
		Class.CastTo( m_ChevronBtn,  layoutRoot.FindAnyWidget( "action_button_chevron" ) );
		Class.CastTo( m_ChevronIcon, layoutRoot.FindAnyWidget( "action_chevron"        ) );
		m_ChevronHover = layoutRoot.FindAnyWidget( "action_chevron_hover" );

		if ( m_ClearIcon )
		{
			m_ClearIcon.LoadImageFile( 0, JMConstants.ICON_CLOSE );
			m_ClearIcon.SetImage( 0 );
		}

		m_PreviousText = "";
		m_Suggestions   = new array<string>;
		m_AnimT         = new JMAnimFloat();
		m_AnimT.Set( 0 );
		m_MaxVisible    = 8;
		m_AutoFilter    = true;
		m_HoveredRow    = -1;
		m_SelectedIndex = -1;
		UpdateClearButton();
		UpdatePlaceholder();
	}

	// -------------------------------------------------------------------------
	//  Suggestion list - call InitSuggestionList(anchor) once to enable the
	//  dropdown-style popup.  Provide the full item set with SetSuggestions().
	//  By default the list is filtered live against the field text (prefix
	//  match); disable with SetAutoFilter(false) to show everything.
	// -------------------------------------------------------------------------

	void InitSuggestionList( notnull Widget listAnchor )
	{
		m_ListAnchor = listAnchor;

		m_ListPanel = g_Game.GetWorkspace().CreateWidgets(
			"JM/COT/GUI/layouts/uiactions/UIActionDropdown_List.layout", m_ListAnchor );

		if ( m_ListPanel )
		{
			Class.CastTo( m_List, m_ListPanel.FindAnyWidget( "action_list_grid" ) );
			m_ListPanel.Show( false );
			m_ListPanel.SetHandler( this );
			if ( m_List )
				m_List.SetHandler( this );
		}

		// The chevron is hidden until there is a list for it to open.
		UpdateClearButton();
	}

	void SetSuggestions( notnull array<string> items )
	{
		m_Suggestions.Clear();
		foreach ( string s: items )
			m_Suggestions.Insert( s );
		m_SelectedIndex = -1;
		RebuildList();
	}

	//! Index into the suggestion set of the last committed row, -1 when none.
	override int GetSelection()
	{
		return m_SelectedIndex;
	}

	string GetSelectedText()
	{
		if ( m_SelectedIndex < 0 || m_SelectedIndex >= m_Suggestions.Count() )
			return "";
		return m_Suggestions[m_SelectedIndex];
	}

	void SetAutoFilter( bool enabled )
	{
		m_AutoFilter = enabled;
		RebuildList();
	}

	void SetMaxVisibleSuggestions( int n )
	{
		m_MaxVisible = Math.Max( 1, n );
	}

	// -- Widget getters (parity with UIActionEditableTextPreview) ---------------

	TextWidget      GetLabelWidget()           { return m_Label; }
	EditBoxWidget   GetEditBoxWidget()         { return m_EditBox; }
	EditBoxWidget   GetEditPreviewBoxWidget()  { return m_PreviewBox; }

	// -- Label ------------------------------------------------------------------

	override void SetLabel( string text )
	{
		if ( !m_Label )
			return;
		text = Widget.TranslateString( text );
		m_Label.SetText( text );
		m_Label.Show( text != "" );
	}

	// -- Placeholder (greyed hint inside the field when empty) ------------------

	void SetPlaceholder( string text )
	{
		m_PlaceholderText = Widget.TranslateString( text );
		UpdatePlaceholder();
	}

	// -- Autocomplete preview ---------------------------------------------------

	void SetTextPreview( string text )
	{
		m_PreviewText = Widget.TranslateString( text );
		UpdatePreview();
	}

	string GetTextPreview()
	{
		return m_PreviewText;
	}

	// -- Text -------------------------------------------------------------------

	override void SetText( string text )
	{
		if ( IsFocused() )
			return;

		m_EditBox.SetText( text );
		m_PreviousText = text;
		UpdateClearButton();
		UpdatePlaceholder();
		UpdatePreview();
	}

	override string GetText()
	{
		return m_EditBox.GetText();
	}

	// Resize only the edit box (matches UIActionEditableTextPreview API).
	void SetEditBoxWidth( float width )
	{
		// The edit box, the preview ghost text and the placeholder all fill the
		// field, so the field itself is resized and they follow. The clear
		// button is parented to the chrome in the layout, so it tracks the new
		// right edge on its own.
		if ( m_Chrome )
		{
			SetWidgetWidth( m_Chrome, width );

			if ( m_PreviewBox )
				SetWidgetWidth( m_PreviewBox, width );

			if ( m_PlaceholderWidget )
				SetWidgetWidth( m_PlaceholderWidget, width );

			return;
		}

		if ( m_EditBox )
			SetWidgetWidth( m_EditBox, width );
	}

	void Clear()
	{
		m_EditBox.SetText( "" );
		m_PreviousText = "";
		m_PreviewText = "";
		UpdateClearButton();
		UpdatePlaceholder();
		UpdatePreview();
	}

	override bool IsFocusWidget( Widget widget )
	{
		if ( widget == m_EditBox ) return true;
		if ( widget == m_ClearBtn ) return true;
		if ( widget == m_ChevronBtn ) return true;
		if ( widget == m_ListPanel ) return true;
		return IsSearchOrList( widget );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == m_EditBox )
		{
			SetFocus( m_EditBox );
			if ( m_ListPanel && m_Suggestions.Count() > 0 )
			{
				RebuildList();
				SetListOpen( true );
			}
			return false;
		}

		if ( w == m_ClearBtn )
		{
			Clear();
			CallEvent( UIEvent.CLICK );
			return true;
		}

		if ( w == m_ChevronBtn )
		{
			ToggleList();
			return true;
		}

		// List row click - direct child of m_List carries the index in its name.
		if ( m_List && w.GetParent() == m_List )
		{
			int idx = w.GetName().ToInt();
			CommitSelection( idx );
			return true;
		}
		if ( m_List )
		{
			Widget parent = w.GetParent();
			if ( parent && parent.GetParent() == m_List )
			{
				int rowIdx = parent.GetName().ToInt();
				CommitSelection( rowIdx );
				return true;
			}
		}

		return false;
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( w == m_ClearBtn && m_ClearHover )
			m_ClearHover.Show( true );

		if ( w == m_ChevronBtn && m_ChevronHover )
			m_ChevronHover.Show( true );

		int enterOrdinal = OrdinalOf( w );
		if ( enterOrdinal >= 0 )
			SetHoveredRow( enterOrdinal );

		// The base class paints the hover ring and the tooltip; skipping super
		// here is what left this control with neither.
		return super.OnMouseEnter( w, x, y );
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( w == m_ClearBtn && m_ClearHover )
			m_ClearHover.Show( false );

		if ( w == m_ChevronBtn && m_ChevronHover )
			m_ChevronHover.Show( false );

		int leaveOrdinal = OrdinalOf( w );
		if ( leaveOrdinal >= 0 && leaveOrdinal == m_HoveredRow )
			SetHoveredRow( -1 );

		return super.OnMouseLeave( w, enterW, x, y );
	}

	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		if ( w != m_EditBox )
			return false;

		// Commit autocomplete on ENTER if there's a preview suggestion.
		if ( finished && m_PreviewText != "" )
			m_EditBox.SetText( m_PreviewText );

		string current = m_EditBox.GetText();
		if ( current == m_PreviousText )
			return false;

		m_PreviousText = current;
		UpdateClearButton();
		UpdatePlaceholder();
		UpdatePreview();

		if ( m_ListPanel && m_Suggestions.Count() > 0 )
		{
			RebuildList();
			SetListOpen( true );
		}

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( FireChange );
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( FireChange, CHANGE_DEBOUNCE_MS, false );
		return true;
	}

	override bool OnKeyPress( Widget w, int x, int y, int key )
	{
		// Arrow / ENTER / ESC drive the open suggestion list first; only when no
		// row is highlighted does ENTER fall through to the ghost-text commit.
		if ( m_ListOpen && m_ListPanel )
		{
			int rowCount = CountRows();
			if ( rowCount > 0 )
			{
				if ( key == KeyCode.KC_DOWN )
				{
					int next = m_HoveredRow + 1;
					if ( next >= rowCount )
						next = 0;
					SetHoveredRow( next );
					return true;
				}
				if ( key == KeyCode.KC_UP )
				{
					int prev = m_HoveredRow - 1;
					if ( prev < 0 )
						prev = rowCount - 1;
					SetHoveredRow( prev );
					return true;
				}
				if ( key == KeyCode.KC_RETURN || key == KeyCode.KC_NUMPADENTER || key == KeyCode.KC_TAB )
				{
					if ( m_HoveredRow >= 0 )
					{
						CommitSelection( SourceOfOrdinal( m_HoveredRow ) );
						return true;
					}
				}
			}

			if ( key == KeyCode.KC_ESCAPE )
			{
				SetListOpen( false );
				return true;
			}
		}

		if ( m_PreviewText != "" && w == m_EditBox )
		{
			if ( key == KeyCode.KC_TAB || key == KeyCode.KC_RETURN || key == KeyCode.KC_NUMPADENTER )
			{
				m_EditBox.SetText( m_PreviewText );
				m_PreviousText = m_PreviewText;
				UpdateClearButton();
				UpdatePlaceholder();
				UpdatePreview();
				g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( FireChange );
				g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( FireChange, CHANGE_DEBOUNCE_MS, false );
				return true;
			}
		}
		return super.OnKeyPress( w, x, y, key );
	}

	protected void FireChange()
	{
		CallEvent( UIEvent.CHANGE );
	}

	private void UpdateClearButton()
	{
		bool hasText = m_EditBox.GetText().Length() > 0;

		if ( m_ClearBtn )
			m_ClearBtn.Show( hasText );

		if ( !m_ChevronBtn )
			return;

		// Both sit in the same right-hand slot: the chevron says "there is a
		// list to open", the cross says "there is something to clear". Never
		// both, and no chevron at all on a search box with no list behind it.
		bool hasList = ( m_ListPanel != NULL );
		m_ChevronBtn.Show( hasList && !hasText );

		if ( !m_ChevronBtn.IsVisible() && m_ChevronHover )
			m_ChevronHover.Show( false );
	}

	private void UpdatePlaceholder()
	{
		if ( !m_PlaceholderWidget )
			return;

		bool empty = ( m_EditBox.GetText() == "" );
		if ( empty && m_PlaceholderText != "" )
		{
			m_PlaceholderWidget.SetText( m_PlaceholderText );
			m_PlaceholderWidget.Show( true );
		}
		else
		{
			m_PlaceholderWidget.Show( false );
		}
	}

	private void UpdatePreview()
	{
		if ( !m_PreviewBox )
			return;

		if ( m_PreviewText != "" && m_EditBox.GetText() == "" )
		{
			m_PreviewBox.SetText( m_PreviewText );
			m_PreviewBox.Show( true );
		}
		else
		{
			m_PreviewBox.Show( false );
		}
	}

	// -------------------------------------------------------------------------
	//  Suggestion list internals
	// -------------------------------------------------------------------------

	//! Chevron click: open the list when it is shut, shut it when it is open.
	//! Opening focuses the field so the arrow keys drive the list straight away.
	protected void ToggleList()
	{
		if ( !m_ListPanel )
			return;

		if ( m_ListOpen )
		{
			SetListOpen( false );
			return;
		}

		RebuildList();
		SetListOpen( true );
		SetFocus( m_EditBox );
	}

	protected void SetListOpen( bool open )
	{
		if ( !m_ListPanel )
			return;
		m_ListOpen = open;
		if ( open )
		{
			m_OpenDelay = 0.15;
			m_ListPanel.Show( true );
			// Raise above all siblings (item lists, other panels) so the dropdown
			// is never obscured by widgets drawn later in the same parent.
			m_ListPanel.SetSort( 9999, true );
			// Start the keyboard-nav highlight on the committed row so the first
			// arrow press moves from a sensible place (matches UIActionDropdown).
			m_HoveredRow = OrdinalOfSource( m_SelectedIndex );
		}
		else
		{
			m_HoveredRow = -1;
		}
		RefreshRowColors();
		// Hide is deferred in Update() when the animation finishes.
	}

	protected void RebuildList()
	{
		if ( !m_List )
			return;

		Widget child = m_List.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}

		string needle = m_EditBox.GetText();
		needle.ToLower();

		int shown = 0;
		for ( int i = 0; i < m_Suggestions.Count(); i++ )
		{
			string item = m_Suggestions[i];

			if ( m_AutoFilter && needle != "" )
			{
				string lower = item;
				lower.ToLower();
				if ( lower.IndexOf( needle ) == -1 )
					continue;
			}

			Widget row = g_Game.GetWorkspace().CreateWidgets(
				"JM/COT/GUI/layouts/uiactions/UIActionDropdown_Row.layout", m_List );
			if ( !row )
				continue;
			row.SetHandler( this );

			ImageWidget rowIcon;
			if ( Class.CastTo( rowIcon, row.FindAnyWidget( "row_icon" ) ) )
				rowIcon.Show( false );

			TextWidget rowText;
			if ( Class.CastTo( rowText, row.FindAnyWidget( "row_text" ) ) )
			{
				rowText.SetText( item );
				rowText.SetTextOffset( UIActionDropdown.TEXT_OFFSET_PLAIN, 0 );
			}

			// Encode the source index so OnClick can look up the full value.
			row.SetName( "" + i );

			shown++;
			if ( shown >= m_MaxVisible )
				break;
		}

		// Row set changed, so the old ordinal no longer points at the same entry.
		m_HoveredRow = OrdinalOfSource( m_SelectedIndex );
		RefreshRowColors();

		// Hide panel automatically when there's nothing to show.
		if ( shown == 0 && m_ListOpen )
			SetListOpen( false );
	}

	// -------------------------------------------------------------------------
	//  Row highlighting - same normal/hover/selected palette as UIActionDropdown
	//  so both popups read as one control.
	// -------------------------------------------------------------------------

	protected void RefreshRowColors()
	{
		if ( !m_List )
			return;

		Widget child = m_List.GetChildren();
		int ordinal = 0;
		while ( child )
		{
			int color = UIActionDropdown.COLOR_ROW_NORMAL;
			if ( ordinal == m_HoveredRow )
				color = UIActionDropdown.COLOR_ROW_HOVER;
			else if ( m_SelectedIndex >= 0 && child.GetName().ToInt() == m_SelectedIndex )
				color = UIActionDropdown.COLOR_ROW_SELECT;

			child.SetColor( color );
			child = child.GetSibling();
			ordinal++;
		}
	}

	protected void SetHoveredRow( int ordinal )
	{
		if ( ordinal == m_HoveredRow )
			return;

		m_HoveredRow = ordinal;
		RefreshRowColors();
	}

	//! Ordinal of any widget that is a row, or a child of a row. -1 otherwise.
	protected int OrdinalOf( Widget w )
	{
		if ( !m_List || !w )
			return -1;

		Widget target = w;
		if ( target.GetParent() != m_List )
		{
			Widget parent = target.GetParent();
			if ( !parent || parent.GetParent() != m_List )
				return -1;
			target = parent;
		}

		Widget child = m_List.GetChildren();
		int ordinal = 0;
		while ( child )
		{
			if ( child == target )
				return ordinal;
			child = child.GetSibling();
			ordinal++;
		}
		return -1;
	}

	//! Ordinal of the row whose encoded source index is srcIdx. -1 when that
	//! entry is currently filtered out.
	protected int OrdinalOfSource( int srcIdx )
	{
		if ( srcIdx < 0 || !m_List )
			return -1;

		Widget child = m_List.GetChildren();
		int ordinal = 0;
		while ( child )
		{
			if ( child.GetName().ToInt() == srcIdx )
				return ordinal;
			child = child.GetSibling();
			ordinal++;
		}
		return -1;
	}

	//! Source index behind an ordinal, -1 when out of range.
	protected int SourceOfOrdinal( int ordinal )
	{
		if ( ordinal < 0 || !m_List )
			return -1;

		Widget child = m_List.GetChildren();
		int i = 0;
		while ( child )
		{
			if ( i == ordinal )
				return child.GetName().ToInt();
			child = child.GetSibling();
			i++;
		}
		return -1;
	}

	protected int CountRows()
	{
		if ( !m_List )
			return 0;

		Widget child = m_List.GetChildren();
		int count = 0;
		while ( child )
		{
			count++;
			child = child.GetSibling();
		}
		return count;
	}

	protected void CommitSelection( int idx )
	{
		if ( idx < 0 || idx >= m_Suggestions.Count() )
			return;

		string value = m_Suggestions[idx];
		m_SelectedIndex = idx;
		m_EditBox.SetText( value );
		m_PreviousText = value;
		SetListOpen( false );

		UpdateClearButton();
		UpdatePlaceholder();
		UpdatePreview();

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( FireChange );
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( FireChange, CHANGE_DEBOUNCE_MS, false );
	}

	// -------------------------------------------------------------------------
	//  Focus / input handling for the list
	// -------------------------------------------------------------------------

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		if ( !m_ListPanel || !m_List || !m_ListAnchor || !m_EditBox )
			return;

		float ax, ay, anchorW, anchorH;
		float fx, fy, fw, fh, gw, gh;
		m_ListAnchor.GetScreenPos( ax, ay );
		m_ListAnchor.GetScreenSize( anchorW, anchorH );
		m_EditBox.GetScreenPos( fx, fy );
		m_EditBox.GetScreenSize( fw, fh );
		m_List.GetScreenSize( gw, gh );

		if ( gh >= 1 )
			m_FullHeight = gh;

		float animTarget = 0.0;
		if ( m_ListOpen )
			animTarget = 1.0;
		m_AnimT.SetTarget( animTarget, 12.0 );
		m_AnimT.Step( timeSlice );

		float t = JMUIAnim.EaseOut( m_AnimT.Value );

		// The chevron turns over with the list rather than snapping, so the same
		// eased value that drives the panel height drives the arrow.
		if ( m_ChevronIcon )
			m_ChevronIcon.SetRotation( 0, 0, t * 180.0 );

		if ( !m_ListOpen && !m_AnimT.IsAnimating() )
			m_ListPanel.Show( false );

		if ( m_ListPanel.IsVisible() && m_FullHeight > 0 )
		{
			float animH = m_FullHeight * t;
			m_ListPanel.SetFlags( WidgetFlags.HEXACTSIZE );
			m_ListPanel.SetFlags( WidgetFlags.VEXACTSIZE );
			m_ListPanel.SetSize( fw, animH );
			m_ListPanel.SetAlpha( t );

			float posY;
			if ( fy + fh + m_FullHeight > ay + anchorH )
				posY = fy - ay - animH;
			else
				posY = fy - ay + fh;

			m_ListPanel.SetPos( fx - ax, posY );
		}

		if ( m_ListOpen )
		{
			if ( m_OpenDelay > 0 )
			{
				m_OpenDelay -= timeSlice;
			}
			else if ( ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK ) != 0 )
			{
				Widget under = GetWidgetUnderCursor();
				if ( !IsSearchOrList( under ) )
					SetListOpen( false );
			}
		}
	}

	protected bool IsSearchOrList( Widget w )
	{
		if ( !w )
			return false;
		// The chevron counts as part of the control, otherwise the outside-click
		// check below would close the list on the very press that toggles it and
		// OnClick would immediately reopen it.
		if ( w == m_EditBox || w == m_ClearBtn || w == m_ChevronBtn || w == m_ListPanel )
			return true;
		if ( m_List && m_ListPanel && m_ListPanel.IsVisible() )
		{
			Widget child = m_List.GetChildren();
			while ( child )
			{
				if ( w == child ) return true;
				Widget sub = child.GetChildren();
				while ( sub )
				{
					if ( w == sub ) return true;
					sub = sub.GetSibling();
				}
				child = child.GetSibling();
			}
		}
		return false;
	}
}
