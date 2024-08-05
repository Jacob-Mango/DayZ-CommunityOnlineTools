class JMRenderableModuleBase: JMModuleBase
{
	private ref CF_Window m_Window;
	private JMFormBase m_Form;

	private ButtonWidget m_MenuButton;

	private int m_MenuButtonColour = 0;

	void JMRenderableModuleBase()
	{
		SetMenuButtonColour( 1, 1, 0, 0 );
	}

	bool InitButton( Widget button_bkg )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::InitButton" );
		#endif

		Class.CastTo( m_MenuButton, button_bkg.FindAnyWidget( "btn" ) );

		if ( button_bkg && m_MenuButton )
		{
			m_MenuButton.SetColor( m_MenuButtonColour );

			TextWidget ttl = TextWidget.Cast( button_bkg.FindAnyWidget( "ttl" ) );
			ttl.SetText( GetLocalisedTitle() );

			ImageWidget btn_img = ImageWidget.Cast( button_bkg.FindAnyWidget( "btn_img" ) );
			TextWidget btn_txt = TextWidget.Cast( button_bkg.FindAnyWidget( "btn_txt" ) );

			if ( ImageIsIcon() )
			{
				btn_txt.Show( false );
				btn_img.Show( true );

				if (ImageHasPath())
					btn_img.LoadImageFile( 0, GetIconName());
				else
					btn_img.LoadImageFile( 0, "set:" + GetImageSet() + " image:" + GetIconName() );
			} else
			{
				btn_txt.Show( true );
				btn_img.Show( false );

				btn_txt.SetText( GetIconName() );
			}

			#ifdef COT_DEBUGLOGS
			Print( "-" + this + "::InitButton true" );
			#endif
			return true;
		}

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::InitButton" );
		#endif
		return false;
	}

	void SetMenuButtonColour( float r, float g, float b, float alpha )
	{
		m_MenuButtonColour = ARGB( r * 255, g * 255, b * 255, alpha * 255 );
		if ( m_MenuButton )
			m_MenuButton.SetColor( m_MenuButtonColour );
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

	void ToggleShow()
	{
		if ( IsVisible() ) 
		{
			Close();
		} else
		{
			if ( GetGame().GetUIManager().GetMenu() )
			{
				return;
			}

			if ( !GetCommunityOnlineToolsBase().IsActive() )
			{
				COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF" ) );
				return;
			}
			
			Show();
		}
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
};
