class JMPlayerRowWidget: COT_ScriptedWidgetEventHandler 
{
#ifdef DIAG
	static int s_JMPlayerRowWidgetCount;
#endif

	private Widget layoutRoot;

	private string m_GUID;

	TextWidget Name;
	ButtonWidget Button;
	CheckBoxWidget Checkbox;

	JMPlayerForm Menu;

	void JMPlayerRowWidget()
	{
	#ifdef DIAG
		s_JMPlayerRowWidgetCount++;
	#ifdef COT_DEBUGLOGS
		CF_Log.Info("JMPlayerRowWidget count: " + s_JMPlayerRowWidgetCount);
	#endif
	#endif
	}

	void ~JMPlayerRowWidget()
	{
		if (!GetGame())
			return;

	#ifdef COT_DEBUGLOGS
		auto trace = CF_Trace_0(this);
	#endif

		DestroyWidget(layoutRoot);

	#ifdef DIAG
		s_JMPlayerRowWidgetCount--;
		if (s_JMPlayerRowWidgetCount <= 0)
			CF_Log.Info("JMPlayerRowWidget count: " + s_JMPlayerRowWidgetCount);
	#endif
	}

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
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Show" );
		#endif

		layoutRoot.Show( true );
		Button.Show( true );
		Checkbox.Show( true );
		Name.Show( true );
		OnShow();

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Show" );
		#endif
	}

	void Hide()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Hide" );
		#endif

		OnHide();
		Name.Show( false );
		Button.Show( false );
		Checkbox.Show( false );
		layoutRoot.Show( false );

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Hide" );
		#endif
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

	Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	void SetPlayer( string guid )
	{
		m_GUID = guid;
		
		if ( m_GUID == "" ) 
		{
			Hide();
		} else {
			JMPlayerInstance player = GetPermissionsManager().GetPlayer( m_GUID );
			if ( player )
			{
				Show();

				array< string > roles = player.GetRoles();
				
				if (roles.Count() > 1)
				{
					Name.SetText( "[" + roles[1] + "] " + player.GetName() );
				} else {
					Name.SetText( player.GetName() );
				}

				if ( GetPermissionsManager().GetClientGUID() == m_GUID )
				{
					Name.SetColor( 0xFF2ECC71 );
				} else if ( player.HasPermission( "COT" ) )
				{
					Name.SetColor( 0xFFA85A32 );
				} else  {
					Name.SetColor( 0xFFFFFFFF );
				}

				if ( JM_GetSelected().IsSelected( m_GUID ) )
				{
					Checkbox.SetChecked( true );
				} else {
					Checkbox.SetChecked( false );
				}
			} else {
				Hide();
			}
		}
	}

	string GetGUID()
	{
		return m_GUID;
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
		{
			return false;
		}

		if ( w == Checkbox )
		{
			JMScriptInvokers.MENU_PLAYER_CHECKBOX.Invoke( m_GUID, Checkbox.IsChecked() );

			return true;
		}

		if ( w == Button )
		{
			JMScriptInvokers.MENU_PLAYER_BUTTON.Invoke( m_GUID, !Checkbox.IsChecked() );

			return true;
		}

		return false;
	}
};
