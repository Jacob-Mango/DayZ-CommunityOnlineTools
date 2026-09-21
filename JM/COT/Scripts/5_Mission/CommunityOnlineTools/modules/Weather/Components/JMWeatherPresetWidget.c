//! #define scope in Enforce is per-file, NOT per compiled module - see COTModule.c.
#define COT_DEBUGLOGS

class JMWeatherPresetWidget: COT_ScriptedWidgetEventHandler 
{
#ifdef DIAG
	static int s_JMWeatherPresetWidgetCount;
#endif

	protected Widget layoutRoot;
	protected TextWidget m_WName;
	protected string m_Name;
	protected ButtonWidget m_WRemoveButton;
	protected ButtonWidget m_WSelectButton;
	protected JMWeatherForm m_Form;
	protected bool m_IsCreatingNew;

	void JMWeatherPresetWidget()
	{
	#ifdef DIAG
		s_JMWeatherPresetWidgetCount++;
	#ifdef COT_DEBUGLOGS
		CF_Log.Info("JMWeatherPresetWidget count: " + s_JMWeatherPresetWidgetCount);
	#endif
	#endif
	}

	void ~JMWeatherPresetWidget()
	{
		if (!g_Game)
			return;

	#ifdef COT_DEBUGLOGS
		auto trace = CF_Trace_0(this);
	#endif

	#ifdef DAYZ_1_28
		DestroyWidget(layoutRoot);
	#endif

	#ifdef DIAG
		s_JMWeatherPresetWidgetCount--;
		if (s_JMWeatherPresetWidgetCount <= 0)
			CF_Log.Info("JMWeatherPresetWidget count: " + s_JMWeatherPresetWidgetCount);
	#endif
	}

	Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	string GetName()
	{
		return m_Name;
	}

	void SetCreateNew()
	{
		Show();
		m_WName.SetText( Widget.TranslateString( "#STR_COT_WEATHER_DYN_ADD_NEW" ) );
		m_WName.SetColor( 0xFFFFFFFF );
		m_WRemoveButton.Show( false );

		m_IsCreatingNew = true;
	}

	void SetPreset( string name )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+JMWeatherPresetWidget::SetPreset" );
		#endif

		m_Name = name;

		m_IsCreatingNew = false;
		
		if (name)
			Show();
		else
			Hide();

		m_WName.SetText( m_Name );
		m_WName.SetColor( 0xFFFFFFFF );

		m_WRemoveButton.Show( JMPermissions.Has( JMConstants.PERM_WEATHER_PRESET_REMOVE ) );

		#ifdef COT_DEBUGLOGS
		Print( "-JMWeatherPresetWidget::SetPreset" );
		#endif
	}

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
				m_Form.SetSelectedPreset( m_Name );
			}
			return true;
		}

		if ( w == m_WRemoveButton )
		{
			m_Form.RemovePreset( m_Name );
			return true;
		}
		
		return false;
	}
}
