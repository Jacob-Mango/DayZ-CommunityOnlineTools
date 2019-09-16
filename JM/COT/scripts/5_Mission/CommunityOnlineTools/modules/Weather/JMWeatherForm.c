class JMWeatherForm extends JMFormBase
{
	private static const int m_DaysInMonth [ 12 ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	private UIActionCheckbox m_ButtonMissionWeather;

	private Widget m_WeatherPhenomenonActions;
	private Widget m_MissionWeatherActions;

	private UIActionSlider m_SliderStormDensity;
	private UIActionSlider m_SliderStormThreshold;
	private UIActionEditableText m_EditTextStormTimeout;

	private UIActionSlider m_SliderFogForecast;
	private UIActionEditableText m_EditFogInterpTime;
	private UIActionEditableText m_EditFogMinDuration;

	private UIActionSlider m_SliderRainForecast;
	private UIActionEditableText m_EditRainInterpTime;
	private UIActionEditableText m_EditRainMinDuration;

	private UIActionSlider m_SliderOvercastForecast;
	private UIActionEditableText m_EditOvercastInterpTime;
	private UIActionEditableText m_EditOvercastMinDuration;

	private bool m_MakingPreset;

	void JMWeatherForm()
	{
	}

	void ~JMWeatherForm()
	{
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

	override void OnInit( bool fromMenu )
	{
		super.OnInit( fromMenu );

		Widget actionsWrapper = UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "actions_wrapper" ), 4, 1 );
		m_ButtonMissionWeather = UIActionManager.CreateCheckbox( actionsWrapper, "Mission Weather", this, "OnClick_Mission", GetGame().GetWeather().GetMissionWeather() );

		m_WeatherPhenomenonActions = UIActionManager.CreateGridSpacer( actionsWrapper, 4, 1 );
		InitStormWidgets( m_WeatherPhenomenonActions );
		InitFogWidgets( m_WeatherPhenomenonActions );
		InitRainWidgets( m_WeatherPhenomenonActions );
		InitOvercastWidgets( m_WeatherPhenomenonActions );

		m_MissionWeatherActions = UIActionManager.CreateGridSpacer( actionsWrapper, 1, 1 );

		Widget presetManagementActions = UIActionManager.CreateGridSpacer( actionsWrapper, 1, 1 );
	}

	override void OnShow()
	{
		super.OnShow();

		SetOptionsView( GetGame().GetWeather().GetMissionWeather() );
	}

	void SetOptionsView( bool useMission )
	{
		if ( useMission )
		{
			m_MissionWeatherActions.Show( true );
			m_WeatherPhenomenonActions.Show( false );
		} else
		{
			m_WeatherPhenomenonActions.Show( true );
			m_MissionWeatherActions.Show( false );
		}
	}

	void OnClick_Mission( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		bool useMissionWeather = m_ButtonMissionWeather.IsChecked();
		SetOptionsView( useMissionWeather );

		if ( m_MakingPreset )
			return;

		// JMWeatherModule.Cast( module ).SetMission( useMissionWeather );
	}

	private void InitStormWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 2, 1 );

		UIActionManager.CreateText( parent, "Storm: ", "Modify the weather storm" );
	
		Widget actions = UIActionManager.CreateGridSpacer( parent, 1, 3 );

		m_SliderStormDensity = UIActionManager.CreateSlider( actions, "Density", 0, 1, this, "OnChange_Storm" );
		m_SliderStormDensity.SetCurrent( 0 );
		m_SliderStormDensity.SetAppend( "" );
		m_SliderStormDensity.SetStepValue( 0.01 );
		m_SliderStormDensity.SetWidgetWidth( m_SliderStormDensity.GetLabelWidget(), 0.5 );
		m_SliderStormDensity.SetWidgetWidth( m_SliderStormDensity.GetSliderWidget(), 0.5 );

		m_SliderStormThreshold = UIActionManager.CreateSlider( actions, "Threshold", 0, 1, this, "OnChange_Storm" );
		m_SliderStormThreshold.SetCurrent( 0 );
		m_SliderStormThreshold.SetAppend( "%" );
		m_SliderStormThreshold.SetStepValue( 0.01 );
		m_SliderStormThreshold.SetWidgetWidth( m_SliderStormThreshold.GetLabelWidget(), 0.5 );
		m_SliderStormThreshold.SetWidgetWidth( m_SliderStormThreshold.GetSliderWidget(), 0.5 );

		m_EditTextStormTimeout = UIActionManager.CreateEditableText( actions, "Timeout", this, "OnChange_Storm" );
		m_EditTextStormTimeout.SetOnlyNumbers( true );
		m_EditTextStormTimeout.SetText( "0" );
		m_EditTextStormTimeout.SetWidgetWidth( m_EditTextStormTimeout.GetLabelWidget(), 0.6 );
		m_EditTextStormTimeout.SetWidgetWidth( m_EditTextStormTimeout.GetEditBoxWidget(), 0.4 );
	}

	void OnChange_Storm( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_MakingPreset )
			return;

		float density = m_SliderStormDensity.GetCurrent();
		float threshold = m_SliderStormThreshold.GetCurrent();
		float timeout = m_EditTextStormTimeout.GetText().ToFloat();

		JMWeatherModule.Cast( module ).SetStorm( density, threshold, timeout );
	}

	private void InitFogWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 2, 1 );

		UIActionManager.CreateText( parent, "Fog: ", "Modify the weather fog phenomenon" );
	
		Widget actions = UIActionManager.CreateGridSpacer( parent, 1, 3 );

		m_SliderFogForecast = UIActionManager.CreateSlider( actions, "Forecast", 0, 1, this, "OnChange_Fog" );
		m_SliderFogForecast.SetCurrent( 0 );
		m_SliderFogForecast.SetAppend( "" );
		m_SliderFogForecast.SetStepValue( 0.01 );
		m_SliderFogForecast.SetWidgetWidth( m_SliderFogForecast.GetLabelWidget(), 0.5 );
		m_SliderFogForecast.SetWidgetWidth( m_SliderFogForecast.GetSliderWidget(), 0.5 );

		m_EditFogInterpTime = UIActionManager.CreateEditableText( actions, "Time", this, "OnChange_Fog" );
		m_EditFogInterpTime.SetOnlyNumbers( true );
		m_EditFogInterpTime.SetText( "0" );
		m_EditFogInterpTime.SetWidgetWidth( m_EditFogInterpTime.GetLabelWidget(), 0.6 );
		m_EditFogInterpTime.SetWidgetWidth( m_EditFogInterpTime.GetEditBoxWidget(), 0.4 );

		m_EditFogMinDuration = UIActionManager.CreateEditableText( actions, "Min Duration", this, "OnChange_Fog" );
		m_EditFogMinDuration.SetOnlyNumbers( true );
		m_EditFogMinDuration.SetText( "0" );
		m_EditFogMinDuration.SetWidgetWidth( m_EditFogMinDuration.GetLabelWidget(), 0.6 );
		m_EditFogMinDuration.SetWidgetWidth( m_EditFogMinDuration.GetEditBoxWidget(), 0.4 );
	}

	void OnChange_Fog( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_MakingPreset )
			return;

		float forecast = m_SliderFogForecast.GetCurrent();
		float time = m_EditFogInterpTime.GetCurrent();
		float minDuration = m_EditFogMinDuration.GetText().ToFloat();

		JMWeatherModule.Cast( module ).SetFog( forecast, time, minDuration );
	}

	private void InitRainWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 2, 1 );

		UIActionManager.CreateText( parent, "Rain: ", "Modify the weather rain phenomenon" );
	
		Widget actions = UIActionManager.CreateGridSpacer( parent, 1, 3 );

		m_SliderRainForecast = UIActionManager.CreateSlider( actions, "Forecast", 0, 1, this, "OnChange_Rain" );
		m_SliderRainForecast.SetCurrent( 0 );
		m_SliderRainForecast.SetAppend( "" );
		m_SliderRainForecast.SetStepValue( 0.01 );
		m_SliderRainForecast.SetWidgetWidth( m_SliderRainForecast.GetLabelWidget(), 0.5 );
		m_SliderRainForecast.SetWidgetWidth( m_SliderRainForecast.GetSliderWidget(), 0.5 );

		m_EditRainInterpTime = UIActionManager.CreateEditableText( actions, "Time", this, "OnChange_Rain" );
		m_EditRainInterpTime.SetOnlyNumbers( true );
		m_EditRainInterpTime.SetText( "0" );
		m_EditRainInterpTime.SetWidgetWidth( m_EditRainInterpTime.GetLabelWidget(), 0.6 );
		m_EditRainInterpTime.SetWidgetWidth( m_EditRainInterpTime.GetEditBoxWidget(), 0.4 );

		m_EditRainMinDuration = UIActionManager.CreateEditableText( actions, "Min Duration", this, "OnChange_Rain" );
		m_EditRainMinDuration.SetOnlyNumbers( true );
		m_EditRainMinDuration.SetText( "0" );
		m_EditRainMinDuration.SetWidgetWidth( m_EditRainMinDuration.GetLabelWidget(), 0.6 );
		m_EditRainMinDuration.SetWidgetWidth( m_EditRainMinDuration.GetEditBoxWidget(), 0.4 );
	}

	void OnChange_Rain( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_MakingPreset )
			return;

		float forecast = m_SliderRainForecast.GetCurrent();
		float time = m_EditRainInterpTime.GetCurrent();
		float minDuration = m_EditRainMinDuration.GetText().ToFloat();

		JMWeatherModule.Cast( module ).SetRain( forecast, time, minDuration );
	}

	private void InitOvercastWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 2, 1 );

		UIActionManager.CreateText( parent, "Overcast: ", "Modify the weather overcast phenomenon" );
	
		Widget actions = UIActionManager.CreateGridSpacer( parent, 1, 3 );

		m_SliderOvercastForecast = UIActionManager.CreateSlider( actions, "Forecast", 0, 1, this, "OnChange_Overcast" );
		m_SliderOvercastForecast.SetCurrent( 0 );
		m_SliderOvercastForecast.SetAppend( "" );
		m_SliderOvercastForecast.SetStepValue( 0.01 );
		m_SliderOvercastForecast.SetWidgetWidth( m_SliderOvercastForecast.GetLabelWidget(), 0.5 );
		m_SliderOvercastForecast.SetWidgetWidth( m_SliderOvercastForecast.GetSliderWidget(), 0.5 );

		m_EditOvercastInterpTime = UIActionManager.CreateEditableText( actions, "Time", this, "OnChange_Overcast" );
		m_EditOvercastInterpTime.SetOnlyNumbers( true );
		m_EditOvercastInterpTime.SetText( "0" );
		m_EditOvercastInterpTime.SetWidgetWidth( m_EditOvercastInterpTime.GetLabelWidget(), 0.6 );
		m_EditOvercastInterpTime.SetWidgetWidth( m_EditOvercastInterpTime.GetEditBoxWidget(), 0.4 );

		m_EditOvercastMinDuration = UIActionManager.CreateEditableText( actions, "Min Duration", this, "OnChange_Overcast" );
		m_EditOvercastMinDuration.SetOnlyNumbers( true );
		m_EditOvercastMinDuration.SetText( "0" );
		m_EditOvercastMinDuration.SetWidgetWidth( m_EditOvercastMinDuration.GetLabelWidget(), 0.6 );
		m_EditOvercastMinDuration.SetWidgetWidth( m_EditOvercastMinDuration.GetEditBoxWidget(), 0.4 );
	}

	void OnChange_Overcast( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_MakingPreset )
			return;

		float forecast = m_SliderOvercastForecast.GetCurrent();
		float time = m_EditOvercastInterpTime.GetCurrent();
		float minDuration = m_EditOvercastMinDuration.GetText().ToFloat();

		JMWeatherModule.Cast( module ).SetOvercast( forecast, time, minDuration );
	}
}