class JMRenderableModuleBase: JMModuleBase
{
	private ref CF_Window m_Window;
	private JMFormBase m_Form;

	private ButtonWidget m_MenuButton;

	//! Tint over this module's sidebar entry, marking whether its window is open.
	private int m_MenuButtonColour = JMTheme.TRANSPARENT;

	void JMRenderableModuleBase()
	{
		SetMenuButtonColor( JMTheme.TRANSPARENT );
	}

#ifndef DAYZ_1_26
	//! 1.27+
	override void UnloadModule()
	{
		super.UnloadModule();

		if (g_JMModuleManager)
			g_JMModuleManager.RemoveCOTModule(this);
	}
#endif

	bool InitButton( Widget button_bkg )
	{

		if ( !button_bkg )
		{
			return false;
		}

		Class.CastTo( m_MenuButton, button_bkg.FindAnyWidget( "btn" ) );

		if ( !m_MenuButton )
		{
			return false;
		}

		m_MenuButton.SetColor( m_MenuButtonColour );

		TextWidget ttl = TextWidget.Cast( button_bkg.FindAnyWidget( "ttl" ) );
		if ( ttl )
			ttl.SetText( GetLocalisedTitle() );

		ImageWidget btn_img = ImageWidget.Cast( button_bkg.FindAnyWidget( "btn_img" ) );
		TextWidget btn_txt = TextWidget.Cast( button_bkg.FindAnyWidget( "btn_txt" ) );

		string iconName = GetIconName();

		if ( ImageIsIcon() )
		{
			if ( btn_txt ) btn_txt.Show( false );
			if ( btn_img )
			{
				btn_img.Show( true );

				if (ImageHasPath())
				{
					btn_img.LoadImageFile( 0, iconName );
				}
				else
				{
					string setRef = "set:" + GetImageSet() + " image:" + iconName;
					btn_img.LoadImageFile( 0, setRef );
				}
			}
		}
		else
		{
			if ( btn_txt )
			{
				btn_txt.Show( true );
				btn_txt.SetText( iconName );
			}
			if ( btn_img ) btn_img.Show( false );
		}

		return true;
	}

	//! Set the sidebar entry tint directly. Prefer a JMTheme token.
	void SetMenuButtonColor( int color )
	{
		m_MenuButtonColour = color;

		if ( m_MenuButton )
			m_MenuButton.SetColor( m_MenuButtonColour );
	}

	//! Legacy float overload, kept so external callers keep compiling.
	//!
	//! @note this used to build its value as ARGB( r, g, b, alpha ) - but ARGB
	//! takes ( a, r, g, b ). Every call therefore packed the red channel from
	//! `r`, the green from `g`... and the ALPHA from `r` too, so the intended
	//! fully transparent SetMenuButtonColour( 1, 1, 0, 0 ) came out as opaque
	//! RED over every sidebar entry. The channels are ordered correctly now.
	void SetMenuButtonColour( float r, float g, float b, float alpha )
	{
		SetMenuButtonColor( ARGB( alpha * 255, r * 255, g * 255, b * 255 ) );
	}

	ButtonWidget GetMenuButton()
	{
		return m_MenuButton;
	}

	void SetForm(JMFormBase form)
	{
		m_Form = form;
	}

	JMFormBase GetForm()
	{
		return m_Form;
	}

	JMFormBase InitForm( Widget root )
	{
		Error( "Not implemented." );
		return NULL;
	}

	string GetInputToggle()
	{
		return "";
	}

	string GetLayoutRoot()
	{
		return "";
	}

	string GetTitle()
	{
		return "";
	}

	string GetLocalisedTitle()
	{
		string text = Widget.TranslateString( "#" + GetTitle() );

		if ( text == "" || text.Get( 0 ) == " " )
		{
			return GetTitle();
		}
		
		return text;
	}

	string GetCategory()
	{
		return "Other";
	}

	string GetImageSet()
	{
		return "";
	}

	string GetIconName()
	{
		return "";
	}

	bool ImageIsIcon()
	{
		return false;
	}

	bool ImageHasPath()
	{
		return false;
	}

	bool HasAccess()
	{
		return true;
	}

	bool HasButton()
	{
		return true;
	}

	bool IsVisible()
	{
		#ifdef CF_WINDOWS
		return m_Window != null;
		#else
		if ( !m_Window )
			return false;
		
		return m_Window.IsVisible();
		#endif
	}

	void Show()
	{

		if ( HasAccess() )
		{
			#ifdef CF_WINDOWS
			m_Window = new CF_Window();

			Widget widgets = m_Window.CreateWidgets(GetLayoutRoot());

			widgets.GetScript(m_Form);

			m_Form.Init(m_Window, this);
			#else
			if ( !m_Window )
			{
				m_Window = GetCOTWindowManager().Create();
				m_Window.SetModule( this );
			}

			m_Window.Show();
			#endif
		}
		else
		{
		}
	}

	void Hide()
	{
		m_Window.Hide();
	}

	void Close()
	{
		if (m_Window)
		{
			m_Window.DestroyLater();
		}
	}

	//! Called by JMWindowManager.RemoveWindow() once m_Window is actually gone
	//! - both Close()'s own DestroyLater() and JMWindowManager.DestroyAllWindows()
	//! (mission end / respawn, see MissionGameplay.OnMissionFinish) end up
	//! deleting the window object without ever routing back through Close(),
	//! and neither path used to clear this reference. A module left open
	//! across a respawn was then a dangling m_Window: IsVisible() kept
	//! dereferencing the freed CF_Window every frame via JMCOTSideBar's
	//! CheckForVisibleModules(), reading unstable garbage that flapped
	//! m_IsTargetCompact and re-triggered the sidebar's open/close animation
	//! every frame it disagreed with itself - the "open close open close"
	//! flicker on the next COT open after that module's window died this way.
	void OnWindowDestroyed()
	{
		m_Window = NULL;
	}

	void ToggleShow()
	{

		if ( IsVisible() )
		{
			Close();
		} else
		{
			if ( g_Game.GetUIManager().GetMenu() )
			{
				return;
			}

			if ( !HasAccess() )
				return;

			if ( !GetCommunityOnlineToolsBase().IsActive() )
			{
				ShowInactiveNotification(GetTitle());
				return;
			}

			Show();
		}
	}

	void ShowInactiveNotification(string inputLoc)
	{
		GetCommunityOnlineToolsBase().ShowInactiveNotification(inputLoc);
	}

	override void OnSettingsUpdated()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnSettingsUpdated");
		#endif

		super.OnSettingsUpdated();

		if ( GetForm() )
		{
			GetForm().OnSettingsUpdated();
		}
	}

	override void OnClientPermissionsUpdated()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnClientPermissionsUpdated");
		#endif

		super.OnClientPermissionsUpdated();

		bool hasAccess = HasAccess();

		if ( GetForm() && hasAccess )
		{
			GetForm().OnClientPermissionsUpdated();
		}

		if ( IsVisible() && !hasAccess )
		{
			Hide();
		}
	}
	
	override void RegisterKeyMouseBindings() 
	{
		super.RegisterKeyMouseBindings();

		if ( GetInputToggle() != "" )
		{
			Bind( new JMModuleBinding( "Input_ToggleShow", GetInputToggle(), true ) );
		}
	}

	void Input_ToggleShow( UAInput input )
	{
		if ( !input.LocalClick() )
			return;
		
		ToggleShow();
	}
}
