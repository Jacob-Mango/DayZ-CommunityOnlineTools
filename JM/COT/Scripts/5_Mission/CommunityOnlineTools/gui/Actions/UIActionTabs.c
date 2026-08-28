// =============================================================================
//  UIActionTabs
//
//  A row of tab buttons. Content panels are managed externally - the caller
//  provides one Widget per tab and UIActionTabs handles show/hide.
//  Fires UIEvent.CHANGE when the active tab changes; GetSelection() returns
//  the new index.
//
//  Usage (in OnInit):
//      m_Tabs = UIActionManager.CreateTabs( parent, { "General", "Stats", "Quick" }, this, "OnChange_Tab" );
//      m_Tabs.AddContent( m_PanelGeneral );
//      m_Tabs.AddContent( m_PanelStats   );
//      m_Tabs.AddContent( m_PanelQuick   );
//      m_Tabs.SetSelection( 0 );
//
//  Optional per-tab icons - pass one path per label, "" to leave a tab iconless:
//      m_Tabs.SetTabIcons( { JMConstants.ICON_SETTINGS_KNOBS, JMConstants.ICON_STACK, "" } );
// =============================================================================
class UIActionTabs: UIActionBase
{
	protected Widget                   m_TabBar;
	protected Widget                   m_TabBarParent;
	protected ref array<ButtonWidget>  m_TabButtons;
	protected ref array<Widget>        m_TabFills;   // the fill image that actually paints
	protected ref array<Widget>        m_TabOutlines;
	protected ref array<ImageWidget>   m_TabIcons;
	protected ref array<TextWidget>    m_TabLabels;  // glyphs get offset when a tab has an icon
	protected ref array<Widget>        m_ContentPanels;
	protected ref array<string>        m_Labels;
	protected int                      m_Selected;
	protected int                      m_Hovered;    // -1 when no tab hovered

	//! The selected tab is the mod's canonical "this one is ON" pill; every
	//! other on-state in the UI (filter chip, icon cell) pulls the same tokens.
	static const int COLOR_ACTIVE   = JMTheme.SELECTED_FILL;
	static const int COLOR_HOVER    = JMTheme.ACCENT_WASH;
	//! Inactive tabs carry no pill at all - only the icon and label show.
	static const int COLOR_INACTIVE = JMTheme.SELECTED_FILL_OFF;

	//! Only the selected tab draws its outline.
	static const int COLOR_OUTLINE  = JMTheme.SELECTED_OUTLINE;

	static const int ICON_ACTIVE    = JMTheme.BUTTON_TEXT;
	static const int ICON_INACTIVE  = JMTheme.BUTTON_TEXT_INACTIVE;

	// The tab label is centred across the WHOLE button, while the icon sits at a
	// fixed x=10 on the left (see UIActionTabButton.layout). A centred label
	// therefore grows leftwards straight into the icon as soon as the text is
	// wide enough - which is what made the tab bar's icons and text collide.
	//
	// Shifting the glyphs right by half the reserved strip re-centres the label
	// in the space LEFT OF nothing / RIGHT OF the icon instead:
	//   glyph centre moves from  W/2  to  W/2 + LABEL_OFFSET_ICON
	//   which is the centre of [ICON_RIGHT_EDGE + ICON_TEXT_GAP, W]
	// Tabs are equal-width grid columns, so a label that is wider than
	// W - (ICON_RIGHT_EDGE + ICON_TEXT_GAP) can still reach the icon; keep tab
	// captions short.
	static const int ICON_RIGHT_EDGE    = 26;   // 10 (position) + 16 (size)
	static const int ICON_TEXT_GAP      = 8;
	static const int LABEL_OFFSET_ICON  = ( ICON_RIGHT_EDGE + ICON_TEXT_GAP ) / 2;   // 17

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_TabBarParent, layoutRoot.FindAnyWidget( "action_wrapper" ) );

		m_TabButtons    = new array<ButtonWidget>;
		m_TabFills      = new array<Widget>;
		m_TabOutlines   = new array<Widget>;
		m_TabIcons      = new array<ImageWidget>;
		m_TabLabels     = new array<TextWidget>;
		m_ContentPanels = new array<Widget>;
		m_Labels        = new array<string>;
		m_Selected      = -1;
		m_Hovered       = -1;
	}

	//! Build the tab buttons from an array of labels.
	void SetTabs( notnull array<string> labels )
	{
		if ( !m_TabBarParent )
			return;

		if ( m_TabBar )
			m_TabBar.Unlink();

		int count = labels.Count();
		if ( count == 0 )
			return;

		int cols = Math.Clamp( count, 1, 9 );
		string gridLayout = string.Format(
			"JM/COT/GUI/layouts/uiactions/Wrappers/TabBar/TabBar%1.layout", cols );

		m_TabBar = g_Game.GetWorkspace().CreateWidgets( gridLayout, m_TabBarParent );
		if ( !m_TabBar )
			return;

		m_TabBar.SetHandler( this );
		m_TabButtons.Clear();
		m_TabFills.Clear();
		m_TabOutlines.Clear();
		m_TabIcons.Clear();
		m_TabLabels.Clear();
		m_Labels.Clear();

		foreach ( int i, string label : labels )
		{
			Widget tabWidget = g_Game.GetWorkspace().CreateWidgets(
				"JM/COT/GUI/layouts/uiactions/UIActionTabButton.layout", m_TabBar );

			if ( !tabWidget )
				continue;

			ButtonWidget btn;
			Class.CastTo( btn, tabWidget );

			TextWidget txt;
			Class.CastTo( txt, tabWidget.FindAnyWidget( "label" ) );
			if ( txt )
			{
				txt.SetText( Widget.TranslateString( label ) );
				txt.SetTextOffset( 0, 0 );
			}

			tabWidget.SetHandler( this );

			Widget fill = tabWidget.FindAnyWidget( "fill" );
			Widget outline = tabWidget.FindAnyWidget( "outline" );

			ImageWidget icon;
			Class.CastTo( icon, tabWidget.FindAnyWidget( "icon" ) );

			m_TabButtons.Insert( btn );
			m_TabFills.Insert( fill );
			m_TabOutlines.Insert( outline );
			m_TabIcons.Insert( icon );
			m_TabLabels.Insert( txt );
			m_Labels.Insert( label );
		}

		UpdateTabColors();
	}

	//! One icon path per tab, in the same order as the labels passed to SetTabs.
	//! Pass "" for a tab that should stay text-only. Extra entries are ignored,
	//! missing ones leave that tab iconless.
	void SetTabIcons( notnull array<string> icons )
	{
		for ( int i = 0; i < m_TabIcons.Count(); i++ )
		{
			ImageWidget icon = m_TabIcons[i];
			if ( !icon )
				continue;

			TextWidget label;
			if ( i < m_TabLabels.Count() )
				label = m_TabLabels[i];

			if ( i >= icons.Count() || icons[i] == "" )
			{
				icon.Show( false );
				if ( label )
					label.SetTextOffset( 0, 0 );
				continue;
			}

			icon.LoadImageFile( 0, icons[i] );
			icon.Show( true );
			if ( label )
				label.SetTextOffset( LABEL_OFFSET_ICON, 0 );
		}

		UpdateTabColors();
	}

	void AddContent( Widget panel )
	{
		m_ContentPanels.Insert( panel );
	}

	override int GetSelection()
	{
		return m_Selected;
	}

	override void SetSelection( int i, bool sendEvent = true )
	{
		if ( i < 0 || i >= m_TabButtons.Count() )
			return;

		m_Selected = i;
		UpdateTabColors();
		UpdateContentVisibility();

		if ( sendEvent )
			CallEvent( UIEvent.CHANGE );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		int idx = TabIndexOf( w );
		if ( idx >= 0 )
		{
			SetSelection( idx );
			return true;
		}
		return false;
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		super.OnMouseEnter( w, x, y );
		int idx = TabIndexOf( w );
		if ( idx >= 0 )
		{
			m_Hovered = idx;
			UpdateTabColors();
		}
		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		super.OnMouseLeave( w, enterW, x, y );
		int idx = TabIndexOf( w );
		if ( idx >= 0 && idx == m_Hovered )
		{
			m_Hovered = -1;
			UpdateTabColors();
		}
		return false;
	}

	protected int TabIndexOf( Widget w )
	{
		foreach ( int i, ButtonWidget btn : m_TabButtons )
		{
			if ( btn && w == btn )
				return i;
		}
		return -1;
	}

	private void UpdateTabColors()
	{
		for ( int i = 0; i < m_TabFills.Count(); i++ )
		{
			int color = COLOR_INACTIVE;
			if ( i == m_Selected )
				color = COLOR_ACTIVE;
			else if ( i == m_Hovered )
				color = COLOR_HOVER;

			Widget fill = m_TabFills[i];
			if ( fill )
				fill.SetColor( color );

			// The cyan ring marks the active tab only - a hovered tab just warms
			// its fill, so the ring stays a reliable "you are here".
			Widget outline = m_TabOutlines[i];
			if ( outline )
				outline.Show( i == m_Selected );

			ImageWidget icon = m_TabIcons[i];
			if ( icon )
			{
				if ( i == m_Selected )
					icon.SetColor( ICON_ACTIVE );
				else
					icon.SetColor( ICON_INACTIVE );
			}
		}
	}

	private void UpdateContentVisibility()
	{
		foreach ( int i, Widget panel : m_ContentPanels )
		{
			if ( panel )
				panel.Show( i == m_Selected );
		}
	}
}
