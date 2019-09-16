class JMWeatherForm extends JMFormBase
{
	private static const int m_DaysInMonth [ 12 ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	private UIActionSlider m_SliderStormDensity;
	private UIActionSlider m_SliderStormThreshold;
	private UIActionEditableText m_EditTextStormTimeout;

	private UIActionSlider m_SliderFog;
	private UIActionSlider m_SliderRain;
	private UIActionSlider m_SliderRainThresholds;
	private UIActionSlider m_SliderOvercast;
	private UIActionSlider m_SliderWind;
	private UIActionSlider m_SliderWindSpeed;
	private UIActionSlider m_SliderWindFunctionParams;
	private UIActionSlider m_SliderDate;

	void JMWeatherForm()
	{
	}

	void ~JMWeatherForm()
	{
	}

	override void OnInit( bool fromMenu )
	{
		super.OnInit( fromMenu );

		Widget mainWidget = UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "actions_wrapper" ), 3, 1 );

		InitStormWidgets( UIActionManager.CreateGridSpacer( mainWidget, 1, 3 ) );
	}

	private void InitStormWidgets( Widget parent )
	{
		m_SliderStormDensity = UIActionManager.CreateSlider( parent, "Density", 0, 1, this, "OnChange_Storm" );
		m_SliderStormDensity.SetCurrent( 0 );
		m_SliderStormDensity.SetAppend( "" );
		m_SliderStormDensity.SetStepValue( 0.01 );
		//m_SliderStormDensity.SetWidth( 1.25 );
		m_SliderStormDensity.SetWidgetWidth( m_SliderStormDensity.GetLabelWidget(), 0.5 );
		m_SliderStormDensity.SetWidgetWidth( m_SliderStormDensity.GetSliderWidget(), 0.5 );

		m_SliderStormThreshold = UIActionManager.CreateSlider( parent, "Threshold", 0, 1, this, "OnChange_Storm" );
		m_SliderStormThreshold.SetCurrent( 0 );
		m_SliderStormThreshold.SetAppend( "%" );
		m_SliderStormThreshold.SetStepValue( 0.01 );
		//m_SliderStormThreshold.SetWidth( 1.25 );
		m_SliderStormThreshold.SetWidgetWidth( m_SliderStormThreshold.GetLabelWidget(), 0.5 );
		m_SliderStormThreshold.SetWidgetWidth( m_SliderStormThreshold.GetSliderWidget(), 0.5 );

		m_EditTextStormTimeout = UIActionManager.CreateEditableText( parent, "Timeout", this, "OnChange_Storm" );
		m_EditTextStormTimeout.SetOnlyNumbers( true );
		m_EditTextStormTimeout.SetText( "0" );
		//m_EditTextStormTimeout.SetWidth( 1.0 );
		m_EditTextStormTimeout.SetWidgetWidth( m_EditTextStormTimeout.GetLabelWidget(), 0.6 );
		m_EditTextStormTimeout.SetWidgetWidth( m_EditTextStormTimeout.GetEditBoxWidget(), 0.4 );
	}
	
	override string GetTitle()
	{
		return "Weather";
	}
	
	override string GetIconName()
	{
		return "W";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	void OnChange_Storm( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		float density = m_SliderStormDensity.GetCurrent();
		float threshold = m_SliderStormThreshold.GetCurrent();
		float timeout = m_EditTextStormTimeout.GetText().ToFloat();

		JMWeatherModule.Cast( module ).SetStorm( density, threshold, timeout );
	}
}