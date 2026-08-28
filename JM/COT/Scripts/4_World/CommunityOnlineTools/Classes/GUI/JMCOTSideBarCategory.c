// Tracks scroll state for one sidebar button whose title is wider than its clip area.
class JMSidebarScrollEntry
{
	TextWidget  tw;
	float       offset;
	float       maxOffset;
	float       pause;
	bool        scrolling;

	static const float SCROLL_SPEED  = 40.0;
	static const float PAUSE_SECONDS = 1.2;

	void JMSidebarScrollEntry( TextWidget textWidget )
	{
		tw        = textWidget;
		offset    = 0;
		maxOffset = 0;
		pause     = PAUSE_SECONDS;
		scrolling = false;
	}

	// Call once after layout is resolved to measure actual overflow.
	// Returns false if there is no overflow and the entry should be discarded.
	bool MeasureOverflow()
	{
		if ( !tw )
			return false;

		int textW, textH;
		tw.GetTextSize( textW, textH );

		float widgetW, widgetH;
		tw.GetScreenSize( widgetW, widgetH );

		float overflow = textW - widgetW;
		if ( overflow <= 2 )
			return false;

		maxOffset = overflow;
		return true;
	}

	void Update( float timeslice )
	{
		if ( maxOffset <= 0 )
			return;

		if ( pause > 0 )
		{
			pause -= timeslice;
			return;
		}

		if ( !scrolling )
		{
			offset += SCROLL_SPEED * timeslice;
			if ( offset >= maxOffset )
			{
				offset    = maxOffset;
				pause     = PAUSE_SECONDS;
				scrolling = true;
			}
		}
		else
		{
			offset -= SCROLL_SPEED * timeslice;
			if ( offset <= 0 )
			{
				offset    = 0;
				pause     = PAUSE_SECONDS;
				scrolling = false;
			}
		}

		tw.SetPos( -(int)offset, 0 );
	}
}

class JMCOTSideBarCategory: COT_ScriptedWidgetEventHandler
{
	private string m_CategoryName;
	private ref array< JMRenderableModuleBase > m_Modules;

	private Widget m_Root;
	private ButtonWidget m_CatBtn;

	private Widget m_FlyoutRoot;
	private Widget m_FlyoutButtons;

	//! Red hover skin for the category tile itself.
	private ref UIActionSurfaceSkin m_CatSkin;
	//! Chevron shown only when this category actually has something to expand.
	private ImageWidget m_CatArrow;
	//! Current and target chevron angle in degrees, eased in OnUpdate.
	//! Collapsed points down (0); focused turns it left (-90) to point at the
	//! flyout, which opens on the left-hand side of the sidebar.
	private float m_ArrowAngle;
	private float m_ArrowTarget;

	static const float ARROW_COLLAPSED_ANGLE = 0.0;
	static const float ARROW_FOCUSED_ANGLE   = -90.0;
	//! Degrees per second. 90 degrees in ~0.18s reads as snappy, not floaty.
	static const float ARROW_TURN_SPEED      = 500.0;

	//! One red hover skin per flyout entry, parallel to m_ButtonRoots.
	private ref array< ref UIActionSurfaceSkin > m_ButtonSkins;
	private ref array< Widget >                  m_ButtonRoots;

	private ref array< ref JMSidebarScrollEntry > m_ScrollEntries;
	// Candidates waiting for deferred overflow measurement (filled during AddModule).
	private ref array< TextWidget > m_ScrollCandidates;
	private bool m_ScrollMeasured;

	private bool m_FlyoutVisible;
	private bool m_MouseOverCategory;
	private bool m_MouseOverFlyout;
	private float m_HideTimer;
	static const float HIDE_DELAY = 0.15;

	// Reference back to the sidebar so ShowFlyout can close sibling flyouts
	private JMCOTSideBar m_SideBar;

	void JMCOTSideBarCategory()
	{
		m_Modules          = new array< JMRenderableModuleBase >;
		m_ButtonSkins      = new array< ref UIActionSurfaceSkin >;
		m_ButtonRoots      = new array< Widget >;
		m_ScrollEntries    = new array< ref JMSidebarScrollEntry >;
		m_ScrollCandidates = new array< TextWidget >;
		m_ScrollMeasured   = false;
	}

	void ~JMCOTSideBarCategory()
	{
		HideFlyout();
		if ( m_FlyoutRoot )
		{
			m_FlyoutRoot.Unlink();
		}
	}

	//! Build the shared red sidebar hover treatment for one row: soft red wash
	//! behind it, hard red bar down the left edge, red label + icon.
	private UIActionSurfaceSkin MakeSidebarSkin( Widget frame, TextWidget label, ImageWidget icon )
	{
		UIActionSurfaceSkin skin = new UIActionSurfaceSkin();

		skin.SetRole( JMUISurfaceStyle.ROLE_ROW );
		skin.SetLabelWidget( label );
		skin.SetIconWidget( icon );
		skin.Attach( frame );

		skin.SetHoverFill( JMUISurfaceStyle.SIDEBAR_HOVER_FILL );
		skin.SetPressFill( JMUISurfaceStyle.SIDEBAR_PRESS_FILL );
		skin.SetAccentColor( JMUISurfaceStyle.SIDEBAR_ACCENT );
		skin.SetTextColors( JMUISurfaceStyle.SIDEBAR_TEXT_IDLE, JMUISurfaceStyle.SIDEBAR_TEXT_HOVER );

		return skin;
	}

	void SetSideBar( JMCOTSideBar sideBar )
	{
		m_SideBar = sideBar;
	}

	void Init( string categoryName, Widget categoryWidget )
	{
		Print("[COT-TRACE] Category.Init begin: " + categoryName);
		m_CategoryName = categoryName;
		m_Root = categoryWidget;
		m_Root.SetHandler( this );

		Print("[COT-TRACE] Category.Init: find cat_btn");
		Class.CastTo( m_CatBtn, m_Root.FindAnyWidget( "cat_btn" ) );
		if ( m_CatBtn )
			m_CatBtn.SetHandler( this );

		TextWidget  catLabel;
		ImageWidget catIcon;
		Class.CastTo( catLabel, m_Root.FindAnyWidget( "cat_ttl"  ) );
		Class.CastTo( catIcon,  m_Root.FindAnyWidget( "cat_icon" ) );
		Class.CastTo( m_CatArrow, m_Root.FindAnyWidget( "cat_arrow" ) );

		m_CatSkin = MakeSidebarSkin( m_Root, catLabel, catIcon );

		// Create flyout as a top-level workspace widget so it can appear outside the sidebar
		Print("[COT-TRACE] Category.Init: create sidebar_flyout.layout");
		m_FlyoutRoot = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/sidebar_flyout.layout", NULL );
		Print("[COT-TRACE] Category.Init: flyout created=" + (m_FlyoutRoot != null).ToString());
		if ( m_FlyoutRoot )
		{
			m_FlyoutRoot.SetHandler( this );
			m_FlyoutRoot.SetSort( JMUILayout.SORT_FLYOUT );
			m_FlyoutButtons = m_FlyoutRoot.FindAnyWidget( "FlyoutButtons" );
			Print("[COT-TRACE] Category.Init: FlyoutButtons found=" + (m_FlyoutButtons != null).ToString());
			if ( m_FlyoutButtons )
				m_FlyoutButtons.SetHandler( this );
			m_FlyoutRoot.Show( false );
		}

		m_FlyoutVisible = false;
		m_MouseOverCategory = false;
		m_MouseOverFlyout = false;
		Print("[COT-TRACE] Category.Init end: " + categoryName);
	}

	void AddModule( JMRenderableModuleBase module )
	{
		Print("[COT-TRACE] Category.AddModule begin: '" + module.GetTitle() + "' (cat=" + m_CategoryName + ")");
		m_Modules.Insert( module );

		if ( !m_FlyoutButtons )
		{
			Print("[COT-TRACE] Category.AddModule: no m_FlyoutButtons, abort");
			return;
		}

		Print("[COT-TRACE] Category.AddModule: create sidebar_button.layout");
		Widget btnWidget = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/sidebar_button.layout", m_FlyoutButtons );
		Print("[COT-TRACE] Category.AddModule: btnWidget=" + (btnWidget != null).ToString());

		Print("[COT-TRACE] Category.AddModule: module.InitButton");
		module.InitButton( btnWidget );
		Print("[COT-TRACE] Category.AddModule: module.InitButton returned");

		// Set our handler on the root button widget and its ButtonWidget child
		// so both OnMouseButtonDown (Panel) and OnClick (Button) reach us
		if ( btnWidget )
		{
			btnWidget.SetHandler( this );
			// The inner ButtonWidget "btn" fires OnClick - set handler on it too
			Widget innerBtn = btnWidget.FindAnyWidget( "btn" );
			if ( innerBtn )
				innerBtn.SetHandler( this );

			// Collect all title widgets; overflow is measured after layout resolves.
			TextWidget ttl = TextWidget.Cast( btnWidget.FindAnyWidget( "ttl" ) );
			if ( ttl )
				m_ScrollCandidates.Insert( ttl );

			ImageWidget btnImg = ImageWidget.Cast( btnWidget.FindAnyWidget( "btn_img" ) );

			m_ButtonSkins.Insert( MakeSidebarSkin( btnWidget, ttl, btnImg ) );
			m_ButtonRoots.Insert( btnWidget );
		}

		// The chevron only means something once there is a submenu to open.
		if ( m_CatArrow )
		{
			m_CatArrow.Show( m_Modules.Count() > 0 );
		}
		Print("[COT-TRACE] Category.AddModule end: '" + module.GetTitle() + "'");
	}

	string GetCategoryName()
	{
		return m_CategoryName;
	}

	Widget GetRoot()
	{
		return m_Root;
	}

	// Returns true if w is the flyout root or any descendant of it
	bool ContainsWidget( Widget w )
	{
		if ( !m_FlyoutRoot || !w )
			return false;

		if ( w == m_FlyoutRoot )
			return true;

		return IsChildOf( w, m_FlyoutRoot );
	}

	bool HasModules()
	{
		return m_Modules.Count() > 0;
	}

	void OnUpdate( float timeslice )
	{
		AnimateArrow( timeslice );

		if ( m_FlyoutVisible && !m_MouseOverCategory && !m_MouseOverFlyout )
		{
			m_HideTimer += timeslice;
			if ( m_HideTimer >= HIDE_DELAY )
			{
				HideFlyout();
			}
		}
		else
		{
			m_HideTimer = 0.0;
		}

		if ( m_FlyoutVisible )
		{
			foreach ( JMSidebarScrollEntry entry: m_ScrollEntries )
				entry.Update( timeslice );
		}
	}

	void RepositionFlyout()
	{
		if ( !m_FlyoutVisible || !m_FlyoutRoot || !m_Root )
			return;

		float flyW, flyH;
		m_FlyoutRoot.GetSize( flyW, flyH );

		float tileX, tileY;
		m_Root.GetScreenPos( tileX, tileY );

		float posX = tileX - flyW;
		float posY = tileY;

		if ( posX < 0 )
			posX = 0;

		float scrW, scrH;
		g_Game.GetWorkspace().GetScreenSize( scrW, scrH );
		if ( posY + flyH > scrH )
			posY = scrH - flyH;
		if ( posY < 0 )
			posY = 0;

		m_FlyoutRoot.SetPos( posX, posY );
	}

	void ForceHideFlyout()
	{
		HideFlyout();
	}

	// Called once after the flyout has been shown and layout has resolved.
	private void MeasureScrollEntries()
	{
		m_ScrollMeasured = true;
		m_ScrollEntries.Clear();

		foreach ( TextWidget tw : m_ScrollCandidates )
		{
			ref JMSidebarScrollEntry entry = new JMSidebarScrollEntry( tw );
			if ( entry.MeasureOverflow() )
				m_ScrollEntries.Insert( entry );
		}
	}

	private void ShowFlyout()
	{
		if ( !m_FlyoutRoot || !m_Root )
			return;

		// Instantly close any other open flyout before showing this one
		if ( m_SideBar )
			m_SideBar.CloseOtherFlyouts( this );

		// Each sidebar_button entry: btn_bkg is size 290x50, position 5 5 (pixel exact)
		// WrapSpacer slot = 50px (child declared size); child visual bottom = slot_top + 5 + 50 = slot_top + 55
		// Last entry overflows by 5px beyond WrapSpacer content height, plus 15px panel border padding
		float flyW = 300;
		float flyH = m_Modules.Count() * 70;

		// Resize the panel and spacer to the exact content height
		m_FlyoutRoot.SetSize( flyW, flyH );
		if ( m_FlyoutButtons )
			m_FlyoutButtons.SetSize( flyW, flyH );

		// Get the screen pixel position of the category tile
		float tileX, tileY;
		m_Root.GetScreenPos( tileX, tileY );

		float posX = tileX - flyW;
		float posY = tileY;

		if ( posX < 0 )
			posX = 0;

		// Clamp: don't overflow the bottom of the screen
		float scrW, scrH;
		g_Game.GetWorkspace().GetScreenSize( scrW, scrH );
		if ( posY + flyH > scrH )
			posY = scrH - flyH;
		if ( posY < 0 )
			posY = 0;

		m_FlyoutRoot.SetPos( posX, posY );
		m_FlyoutRoot.Show( true );
		m_FlyoutVisible = true;
		m_HideTimer = 0.0;

		UpdateArrow();

		if ( !m_ScrollMeasured )
			g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( MeasureScrollEntries, 100, false );
	}

	private void HideFlyout()
	{
		if ( m_FlyoutRoot )
			m_FlyoutRoot.Show( false );

		m_FlyoutVisible = false;
		m_HideTimer = 0.0;

		ClearEntryHovers();
		UpdateArrow();

		// The tile only held its red state to mark the open submenu.
		if ( m_CatSkin && !m_MouseOverCategory )
			m_CatSkin.SetHovered( false );
	}

	//! Which flyout entry's skin owns this event widget. Events arrive on the
	//! inner ButtonWidget "btn", so match its parent row as well as the row
	//! itself.
	private UIActionSurfaceSkin SkinForEntry( Widget w )
	{
		if ( !w )
			return NULL;

		Widget parent = w.GetParent();

		for ( int i = 0; i < m_ButtonRoots.Count(); i++ )
		{
			if ( m_ButtonRoots[i] == w || m_ButtonRoots[i] == parent )
				return m_ButtonSkins[i];
		}

		return NULL;
	}

	//! Clear every entry's hover, used when the whole flyout goes away.
	private void ClearEntryHovers()
	{
		foreach ( UIActionSurfaceSkin entrySkin : m_ButtonSkins )
		{
			entrySkin.SetHovered( false );
			entrySkin.SetPressed( false );
		}
	}

	//! Aim the chevron. The actual turn is eased in OnUpdate so focusing a
	//! category visibly rotates the arrow rather than snapping it.
	private void UpdateArrow()
	{
		if ( m_FlyoutVisible )
			m_ArrowTarget = ARROW_FOCUSED_ANGLE;
		else
			m_ArrowTarget = ARROW_COLLAPSED_ANGLE;
	}

	//! Step the chevron toward its target angle.
	private void AnimateArrow( float timeslice )
	{
		if ( !m_CatArrow )
			return;

		if ( Math.AbsFloat( m_ArrowTarget - m_ArrowAngle ) < 0.5 )
		{
			if ( m_ArrowAngle != m_ArrowTarget )
			{
				m_ArrowAngle = m_ArrowTarget;
				m_CatArrow.SetRotation( 0, 0, m_ArrowAngle );
			}
			return;
		}

		float step = ARROW_TURN_SPEED * timeslice;

		if ( m_ArrowTarget > m_ArrowAngle )
			m_ArrowAngle = Math.Min( m_ArrowAngle + step, m_ArrowTarget );
		else
			m_ArrowAngle = Math.Max( m_ArrowAngle - step, m_ArrowTarget );

		m_CatArrow.SetRotation( 0, 0, m_ArrowAngle );
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( w == m_Root || w == m_CatBtn )
		{
			m_MouseOverCategory = true;

			if ( m_CatSkin )
				m_CatSkin.SetHovered( true );

			if ( !m_FlyoutVisible )
				ShowFlyout();
		}
		else if ( w == m_FlyoutRoot || IsChildOf( w, m_FlyoutRoot ) )
		{
			m_MouseOverFlyout = true;

			UIActionSurfaceSkin entrySkin = SkinForEntry( w );
			if ( entrySkin )
				entrySkin.SetHovered( true );
		}

		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( w == m_Root || w == m_CatBtn )
		{
			m_MouseOverCategory = false;

			// The tile keeps its red state while its own flyout is open, so the
			// user can see which category the submenu belongs to.
			if ( m_CatSkin && !m_FlyoutVisible )
				m_CatSkin.SetHovered( false );
		}
		else if ( w == m_FlyoutRoot || IsChildOf( w, m_FlyoutRoot ) )
		{
			m_MouseOverFlyout = false;

			UIActionSurfaceSkin leftSkin = SkinForEntry( w );
			if ( leftSkin )
			{
				leftSkin.SetHovered( false );
				leftSkin.SetPressed( false );
			}
		}

		return false;
	}

	// PanelWidget (btn_bkg) fires this
	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( !IsMissionClient() )
			return false;

		if ( button != 0 )
			return false;

		JMRenderableModuleBase module = FindModuleForWidget( w );
		if ( module )
		{
			module.ToggleShow();
			HideFlyout();
			return true;
		}

		return false;
	}

	// ButtonWidget (btn) fires this
	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( !IsMissionClient() )
			return false;

		if ( button != 0 )
			return false;

		JMRenderableModuleBase module = FindModuleForWidget( w );
		if ( module )
		{
			module.ToggleShow();
			HideFlyout();
			return true;
		}

		return false;
	}

	// Find which module owns the clicked widget by checking GetMenuButton()
	// and the root btn_bkg widget (stored as the button widget passed to InitButton)
	private JMRenderableModuleBase FindModuleForWidget( Widget w )
	{
		foreach ( JMRenderableModuleBase module: m_Modules )
		{
			// w is the ButtonWidget "btn" (from OnClick)
			if ( w == module.GetMenuButton() )
				return module;

			// w is the PanelWidget "btn_bkg" (from OnMouseButtonDown)
			// btn_bkg is the parent of btn
			ButtonWidget btn = module.GetMenuButton();
			if ( btn && btn.GetParent() == w )
				return module;
		}

		return NULL;
	}

	private bool IsChildOf( Widget child, Widget parent )
	{
		if ( !child || !parent )
			return false;

		Widget cur = child.GetParent();
		while ( cur )
		{
			if ( cur == parent )
				return true;
			cur = cur.GetParent();
		}

		return false;
	}
}
