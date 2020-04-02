class JMESPViewTypeWidget extends ScriptedWidgetEventHandler 
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
		m_LayoutRoot.Show( true );
		OnShow();
	}

	void Hide()
	{
		OnHide();
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
		return m_LayoutRoot;
	}

	void Set( JMESPViewType type )
	{
		m_Type = type;
		
		if ( m_Type ) 
		{
			m_Name.SetText( type.Localisation );
			m_Checkbox.SetChecked( type.View );

			Show();
		} else 
		{
			Hide();
		}
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

			return true;
		}

		return false;
	}
}