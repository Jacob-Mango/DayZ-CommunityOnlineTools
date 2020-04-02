class JMWeatherPresetWidget extends ScriptedWidgetEventHandler 
{
	private Widget layoutRoot;

	private TextWidget m_WName;
    private string m_Name;

	private ButtonWidget m_WRemoveButton;
	private ButtonWidget m_WSelectButton;

    private string m_Permission;

	private JMWeatherForm m_Form;

	private bool m_IsCreatingNew;

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );
	}

	void Init( notnull JMWeatherForm form ) 
	{
		m_WName = TextWidget.Cast( layoutRoot.FindAnyWidget( "preset_name_text" ) );

		m_WRemoveButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "remove_button" ) );
		m_WSelectButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "select_button" ) );		

		m_Form = form;
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
		Show();
		m_WName.SetText( "Add New" );
		m_WName.SetColor( 0xFFFFFFFF );
		m_WRemoveButton.Show( false );

		m_IsCreatingNew = true;
    }

	void SetPreset( string name, string permission )
	{
		m_Name = name;
        m_Permission = permission;

		m_IsCreatingNew = false;
		
		if ( m_Permission == "" ) 
		{
			Hide();
		} else 
		{
			Show();

			m_WName.SetText( m_Name );
			m_WName.SetColor( 0xFFFFFFFF );

			if ( GetPermissionsManager().HasPermission( "Weather.Preset.Remove", NULL ) )
			{
				m_WRemoveButton.Show( true );
			} else
			{
				m_WRemoveButton.Show( false );
			}
		}
	}

	string GetName()
	{
		return m_Name;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if ( w == NULL )
			return false;

		if ( w == m_WSelectButton )
		{
			if ( m_IsCreatingNew )
			{
				m_Form.CreateNew();
			} else 
			{
				m_Form.SetSelectedPreset( m_Permission );
			}
			return true;
		}

		if ( w == m_WRemoveButton )
		{
			m_Form.RemovePreset( m_Permission );
			return true;
		}
		
		return false;
	}
}