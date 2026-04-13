class JMCOTSideBarCategory: COT_ScriptedWidgetEventHandler
{
	private string m_CategoryName;
	private ref array< JMRenderableModuleBase > m_Modules;

	private Widget m_Root;
	private ButtonWidget m_CatBtn;

	private Widget m_FlyoutRoot;
	private Widget m_FlyoutButtons;

	private bool m_FlyoutVisible;
	private bool m_MouseOverCategory;
	private bool m_MouseOverFlyout;
	private float m_HideTimer;
	static const float HIDE_DELAY = 0.15;

	// Reference back to the sidebar so ShowFlyout can close sibling flyouts
	private JMCOTSideBar m_SideBar;

	void JMCOTSideBarCategory()
	{
		m_Modules = new array< JMRenderableModuleBase >;
	}

	void ~JMCOTSideBarCategory()
	{
		HideFlyout();
		if ( m_FlyoutRoot )
		{
			m_FlyoutRoot.Unlink();
		}
	}

	void SetSideBar( JMCOTSideBar sideBar )
	{
		m_SideBar = sideBar;
	}

	void Init( string categoryName, Widget categoryWidget )
	{
		m_CategoryName = categoryName;
		m_Root = categoryWidget;
		m_Root.SetHandler( this );

		Class.CastTo( m_CatBtn, m_Root.FindAnyWidget( "cat_btn" ) );
		if ( m_CatBtn )
			m_CatBtn.SetHandler( this );

		// Create flyout as a top-level workspace widget so it can appear outside the sidebar
		m_FlyoutRoot = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/sidebar_flyout.layout", NULL );
		if ( m_FlyoutRoot )
		{
			m_FlyoutRoot.SetHandler( this );
			m_FlyoutButtons = m_FlyoutRoot.FindAnyWidget( "FlyoutButtons" );
			if ( m_FlyoutButtons )
				m_FlyoutButtons.SetHandler( this );
			m_FlyoutRoot.Show( false );
		}

		m_FlyoutVisible = false;
		m_MouseOverCategory = false;
		m_MouseOverFlyout = false;
	}

	void AddModule( JMRenderableModuleBase module )
	{
		m_Modules.Insert( module );

		if ( !m_FlyoutButtons )
			return;

		Widget btnWidget = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/sidebar_button.layout", m_FlyoutButtons );
		module.InitButton( btnWidget );

		// Set our handler on the root button widget and its ButtonWidget child
		// so both OnMouseButtonDown (Panel) and OnClick (Button) reach us
		if ( btnWidget )
		{
			btnWidget.SetHandler( this );
			// The inner ButtonWidget "btn" fires OnClick — set handler on it too
			Widget innerBtn = btnWidget.FindAnyWidget( "btn" );
			if ( innerBtn )
				innerBtn.SetHandler( this );
		}
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

		int scrW, scrH;
		g_Game.GetScreenSize( scrW, scrH );
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
		int scrW, scrH;
		g_Game.GetScreenSize( scrW, scrH );
		if ( posY + flyH > scrH )
			posY = scrH - flyH;
		if ( posY < 0 )
			posY = 0;

		m_FlyoutRoot.SetPos( posX, posY );
		m_FlyoutRoot.Show( true );
		m_FlyoutVisible = true;
		m_HideTimer = 0.0;
	}

	private void HideFlyout()
	{
		if ( m_FlyoutRoot )
			m_FlyoutRoot.Show( false );

		m_FlyoutVisible = false;
		m_HideTimer = 0.0;
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( w == m_Root || w == m_CatBtn )
		{
			m_MouseOverCategory = true;
			if ( !m_FlyoutVisible )
				ShowFlyout();
		}
		else if ( w == m_FlyoutRoot || IsChildOf( w, m_FlyoutRoot ) )
		{
			m_MouseOverFlyout = true;
		}

		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( w == m_Root || w == m_CatBtn )
		{
			m_MouseOverCategory = false;
		}
		else if ( w == m_FlyoutRoot || IsChildOf( w, m_FlyoutRoot ) )
		{
			m_MouseOverFlyout = false;
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
