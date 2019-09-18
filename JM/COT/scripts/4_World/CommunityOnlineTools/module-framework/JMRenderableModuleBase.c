class JMRenderableModuleBase extends JMModuleBase
{
	private ref JMWindowBase m_Window;

	private ButtonWidget m_MenuButton;

	bool InitButton( Widget button_bkg )
	{
		Class.CastTo( m_MenuButton, button_bkg.FindAnyWidget( "btn" ) );

		if ( button_bkg && m_MenuButton )
		{
			TextWidget ttl = TextWidget.Cast( button_bkg.FindAnyWidget( "ttl" ) );
			ttl.SetText( GetTitle() );

			ImageWidget btn_img = ImageWidget.Cast( button_bkg.FindAnyWidget( "btn_img" ) );
			TextWidget btn_txt = TextWidget.Cast( button_bkg.FindAnyWidget( "btn_txt" ) );

			if ( ImageIsIcon() )
			{
				btn_txt.Show( false );
				btn_img.Show( true );

				btn_img.LoadImageFile( 0, "set:" + GetImageSet() + " image:" + GetIconName() );
			} else
			{
				btn_txt.Show( true );
				btn_img.Show( false );

				btn_txt.SetText( GetIconName() );
			}

			return true;
		}

		return false;
	}

	ButtonWidget GetMenuButton()
	{
		return m_MenuButton;
	}

	JMFormBase GetForm()
	{
		if ( !m_Window )
			return NULL;

		return m_Window.GetForm();
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
		if ( !m_Window )
			return false;
		
		return m_Window.IsVisible();
	}

	void Show()
	{
		if ( HasAccess() )
		{
			m_Window = JMWindowBase.Create();
			m_Window.SetModule( this );
		}
	}

	void Hide()
	{
		if ( IsVisible() )
		{
			m_Window.Hide();
			delete m_Window;
		}
	}

	void ToggleShow()
	{
		if ( IsVisible() ) 
		{
			Hide();
		} else
		{
			Show();
		}
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();

		if ( IsVisible() && !HasAccess() )
		{
			Hide();
		}
	}
	
	override void RegisterKeyMouseBindings() 
	{
		super.RegisterKeyMouseBindings();

		if ( GetInputToggle() != "" )
		{
			RegisterBinding( new JMModuleBinding( "Input_ToggleShow", GetInputToggle(), true ) );
		}
	}

	void Input_ToggleShow( UAInput input )
	{
		if ( !input.LocalPress() )
			return;
		
		ToggleShow();
	}
}