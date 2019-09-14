class JMPlayerRowWidget extends ScriptedWidgetEventHandler 
{
	private Widget layoutRoot;

	private string m_GUID;

	TextWidget Name;
	ButtonWidget Button;
	CheckBoxWidget Checkbox;

	JMPlayerForm Menu;

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );
		Init();
	}

	void Init() 
	{
		Name = TextWidget.Cast(layoutRoot.FindAnyWidget("text_name"));
		Button = ButtonWidget.Cast(layoutRoot.FindAnyWidget("button"));
		Checkbox = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("checkbox"));
	}

	void Show()
	{
		layoutRoot.Show( true );
		Button.Show( true );
		Checkbox.Show( true );
		Name.Show( true );
		OnShow();
	}

	void Hide()
	{
		OnHide();
		Name.Show( false );
		Button.Show( false );
		Checkbox.Show( false );
		layoutRoot.Show( false );
	}

	void OnShow()
	{
	}

	void OnHide() 
	{
	}

	void Update() 
	{
		
	}

	ref Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	void SetPlayer( string guid )
	{
		m_GUID = guid;
		
		if ( m_GUID == "" ) 
		{
			Hide();
		} else 
		{
			Show();

			JMPlayerInstance player = GetPermissionsManager().GetPlayer( m_GUID );

			if ( GetGame().IsServer() && !GetGame().IsMultiplayer() )
			{
				Name.SetText( "Offline Mode" );
				Name.SetColor( 0xFF2ECC71 );
			} else if ( GetPermissionsManager().GetClientGUID() == m_GUID )
			{
				Name.SetText( player.GetName() );
				Name.SetColor( 0xFF2ECC71 );
			} else 
			{
				Name.SetText( player.GetName() );
				Name.SetColor( 0xFFFFFFFF );
			}
		}
	}

	string GetGUID()
	{
		return m_GUID;
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{		
		if ( w == Checkbox )
		{
			JMScriptInvokers.MENU_PLAYER_CHECKBOX.Invoke( m_GUID, Checkbox.IsChecked() );
		}

		if ( w == Button )
		{
			JMScriptInvokers.MENU_PLAYER_BUTTON.Invoke( m_GUID );
		}

		return true;
	}
}