class JMWeatherPresetWidget extends ScriptedWidgetEventHandler 
{
	private Widget layoutRoot;

	private TextWidget m_WName;
    private string m_Name;

    private string m_Permission;

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );
	}

	void Init() 
	{
		m_WName = TextWidget.Cast( layoutRoot.FindAnyWidget( "name" ) );
	}

	void Show()
	{
		layoutRoot.Show( true );
		m_WName.Show( true );

		OnShow();
	}

	void Hide()
	{
		OnHide();

		m_WName.Show( false );
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

    void SetCreateNew()
    {

    }

	void SetPreset( string name, string permission )
	{
		m_Name = name;
        m_Permission = permission;
		
		if ( m_Permission == "" ) 
		{
			Hide();
		} else 
		{
			Show();

			m_WName.SetText( m_Name );
			m_WName.SetColor( 0xFFFFFFFF );
		}
	}

	string GetName()
	{
		return m_Name;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		return false;
	}
}