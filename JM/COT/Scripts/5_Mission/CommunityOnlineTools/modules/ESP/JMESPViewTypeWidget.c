class JMESPViewTypeWidget: ScriptedWidgetEventHandler 
{
	private Widget m_LayoutRoot;
	private CheckBoxWidget m_Checkbox;
	private TextWidget m_Name;

	private JMESPViewType m_Type;

	void OnWidgetScriptInit( Widget w )
	{
		m_LayoutRoot = w;
		m_LayoutRoot.SetHandler( this );
		Init();
	}

	void Init() 
	{
		Class.CastTo( m_Checkbox, m_LayoutRoot.FindAnyWidget( "checkbox" ) );
		Class.CastTo( m_Name, m_LayoutRoot.FindAnyWidget( "text_name" ) );
	}

	void Show()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Show" );
		#endif

		m_LayoutRoot.Show( true );
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

		m_LayoutRoot.Show( false );
		OnHide();

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
		return m_LayoutRoot;
	}

	void Set( JMESPViewType type )
	{
		m_Type = type;
		
		if ( m_Type ) 
		{
			m_Name.SetText( Widget.TranslateString( type.Localisation ) );
			m_Checkbox.SetChecked( type.View );

			Show();
		} else 
		{
			Hide();
		}
	}

	void SetChecked( bool state )
	{
		m_Checkbox.SetChecked(state);
		m_Type.View = state;

		JMScriptInvokers.ESP_VIEWTYPE_CHANGED.Invoke(m_Type);
	}

	bool IsChecked()
	{
		return m_Checkbox.IsChecked();
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
		{
			return false;
		}

		if ( w == m_Checkbox )
		{
			m_Type.View = m_Checkbox.IsChecked();

			JMScriptInvokers.ESP_VIEWTYPE_CHANGED.Invoke(m_Type);

			return true;
		}

		return false;
	}
};
