class JMRenderableModuleBase extends JMModuleBase
{
	private JMWindowBase m_Window;

	private ButtonWidget m_MenuButton;

	bool InitButton( Widget button_bkg )
	{
		Class.CastTo( m_MenuButton, button_bkg.FindAnyWidget( "btn" ) );

		if ( button_bkg && m_MenuButton )
		{
			TextWidget ttl = TextWidget.Cast( button_bkg.FindAnyWidget( "ttl" ) );
			ttl.SetText( module.GetTitle() );

			ImageWidget btn_img = ImageWidget.Cast( button_bkg.FindAnyWidget( "btn_img" ) );
			TextWidget btn_txt = TextWidget.Cast( button_bkg.FindAnyWidget( "btn_txt" ) );

			if ( module.ImageIsIcon() )
			{
				btn_txt.Show( false );
				btn_img.Show( true );

				btn_img.LoadImageFile( 0, "set:" + module.GetImageSet() + " image:" + module.GetIconName() );
			} else
			{
				btn_txt.Show( true );
				btn_img.Show( false );

				btn_txt.SetText( module.GetIconName() );
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
		return m_Window.GetForm();
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
			if ( !m_Window )
			{
				m_Window = new JMWindowBase;
				m_Window.SetModule( this );
			}

			m_Window.Show();
		}
	}

	void Hide()
	{
		if ( IsVisible() )
		{
			m_Window.Close();
		}
	}

	void ToggleShow()
	{
		if ( IsVisible() ) 
		{
			m_Window.Close();
			return;
		}

		Show();
	}

	override void OnClientPermissionsUpdated()
	{
		if ( IsVisible() && !HasAccess() )
		{
			m_Window.Hide();
		}
	}
}