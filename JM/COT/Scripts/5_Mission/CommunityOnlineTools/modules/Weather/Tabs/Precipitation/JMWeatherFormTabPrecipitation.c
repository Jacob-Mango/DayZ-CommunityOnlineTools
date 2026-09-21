//! "Precipitation" tab of JMWeatherForm - Rain and Snow cards. Back-reference
//! to the owning form, same shape as JMPlayerRowWidget.Menu. See
//! JMWeatherFormTabSky.c's header for why Apply/dirty-tracking/preview
//! mechanics stay on the form.
//!
//! The amount is a JMWeatherRangeControl (one value, or a span the server rolls
//! between). The thresholds are already a min/max pair of their own - "rain is
//! possible between these overcast levels" - so they stay a plain range.
class JMWeatherFormTabPrecipitation: JMFormTab
{
	protected JMWeatherForm m_Form;
	protected UIActionScroller m_ScrollerPrecipitation;
	protected ref JMWeatherRangeControl m_SliderRainForecast;
	protected UIActionSliderRange       m_RangeRainThreshold;
	protected ref JMWeatherRangeControl m_SliderSnowForecast;
	protected UIActionSliderRange       m_RangeSnowThreshold;

	protected ref JMWeatherRangeToggle m_ToggleRain;
	protected ref JMWeatherRangeToggle m_ToggleSnow;

	void JMWeatherFormTabPrecipitation( JMWeatherForm form )
	{
		m_Form = form;
	}

	//! Every card at once: on while a preset is being edited, off for the live world.
	void SetRangeMode( bool on )
	{
		if ( m_ToggleRain ) m_ToggleRain.SetRanges( on );
		if ( m_ToggleSnow ) m_ToggleSnow.SetRanges( on );
	}

	void SetRainValues( JMWeatherPreset preset, bool actual )
	{
		m_Form.SetPhenomenonRange( m_SliderRainForecast, preset.PRain, actual );

		if ( preset.RainThreshold.Time != -1 && m_RangeRainThreshold )
			m_RangeRainThreshold.SetRange( preset.RainThreshold.OvercastMin * 100.0, preset.RainThreshold.OvercastMax * 100.0 );
	}

	void SetSnowValues( JMWeatherPreset preset, bool actual )
	{
		m_Form.SetPhenomenonRange( m_SliderSnowForecast, preset.PSnow, actual );

		if ( preset.SnowThreshold.Time != -1 && m_RangeSnowThreshold )
			m_RangeSnowThreshold.SetRange( preset.SnowThreshold.OvercastMin * 100.0, preset.SnowThreshold.OvercastMax * 100.0 );
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		m_ScrollerPrecipitation = UIActionManager.CreateScroller( panel );
		Widget content = m_ScrollerPrecipitation.GetContentWidget();

		UIActionCard rainCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_RAIN" );
		rainCard.AddApplyButton( this, "OnClick_ApplyRain" );
		m_ToggleRain = new JMWeatherRangeToggle( m_Form, rainCard, JMWeatherForm.SECTION_RAIN );
		Widget rainBody = UIActionManager.CreateGridSpacer( rainCard.GetContent(), 2, 1 );

		m_SliderRainForecast = m_Form.CreatePercentRange( rainBody, "#STR_COT_GENERIC_AMOUNT", "OnChange_Rain", this );
		m_ToggleRain.Add( m_SliderRainForecast );

		//! One range control instead of two sliders that must not cross. The
		//! pair is a single "rain is possible between these overcast levels"
		//! statement, and a Min above Max is not a state worth allowing.
		m_RangeRainThreshold = UIActionManager.CreateSliderRange( rainBody, "#STR_COT_WEATHER_THRESHOLDS", 0, 100, this, "OnChange_Rain" );
		m_RangeRainThreshold.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_RangeRainThreshold.SetStep( 1 );
		m_RangeRainThreshold.SetTooltip( "#STR_COT_WEATHER_THRESHOLDS_DESC" );

		UIActionCard snowCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_SNOW" );
		snowCard.AddApplyButton( this, "OnClick_ApplySnow" );
		m_ToggleSnow = new JMWeatherRangeToggle( m_Form, snowCard, JMWeatherForm.SECTION_SNOW );
		Widget snowBody = UIActionManager.CreateGridSpacer( snowCard.GetContent(), 2, 1 );

		m_SliderSnowForecast = m_Form.CreatePercentRange( snowBody, "#STR_COT_GENERIC_AMOUNT", "OnChange_Snow", this );
		m_ToggleSnow.Add( m_SliderSnowForecast );

		m_RangeSnowThreshold = UIActionManager.CreateSliderRange( snowBody, "#STR_COT_WEATHER_THRESHOLDS", 0, 100, this, "OnChange_Snow" );
		m_RangeSnowThreshold.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_RangeSnowThreshold.SetStep( 1 );
		m_RangeSnowThreshold.SetTooltip( "#STR_COT_WEATHER_THRESHOLDS_DESC" );

		m_ScrollerPrecipitation.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_ScrollerPrecipitation )
			m_ScrollerPrecipitation.UpdateScroller();
	}

	protected float ReadLow( JMWeatherRangeControl control )
	{
		if ( !control )
			return 0;

		return control.Low();
	}

	protected float ReadHi( JMWeatherRangeControl control )
	{
		if ( !control )
			return 0;

		return control.Hi();
	}

	void ReadInto( JMWeatherPreset preset, float transition, float duration )
	{
		preset.PRain.Forecast    = ReadLow( m_SliderRainForecast );
		preset.PRain.ForecastHi  = ReadHi( m_SliderRainForecast );
		preset.PRain.Time        = transition;
		preset.PRain.MinDuration = duration;

		if ( m_RangeRainThreshold )
		{
			preset.RainThreshold.OvercastMin = m_RangeRainThreshold.GetRangeLow() * 0.01;
			preset.RainThreshold.OvercastMax = m_RangeRainThreshold.GetRangeHigh() * 0.01;
		}
		preset.RainThreshold.Time = transition;

		preset.PSnow.Forecast    = ReadLow( m_SliderSnowForecast );
		preset.PSnow.ForecastHi  = ReadHi( m_SliderSnowForecast );
		preset.PSnow.Time        = transition;
		preset.PSnow.MinDuration = duration;

		if ( m_RangeSnowThreshold )
		{
			preset.SnowThreshold.OvercastMin = m_RangeSnowThreshold.GetRangeLow() * 0.01;
			preset.SnowThreshold.OvercastMax = m_RangeSnowThreshold.GetRangeHigh() * 0.01;
		}
		preset.SnowThreshold.Time = transition;
	}

	void OnChange_Rain( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_RAIN );
	}

	void OnChange_Snow( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_SNOW );
	}

	void OnClick_ApplyRain( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module )
			return;
		if ( !JMPermissions.Has( JMConstants.PERM_WEATHER_RAIN ) )
			return;

		action.AnimateFeedback();
		ApplyRain();
	}

	void OnClick_ApplySnow( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module )
			return;
		if ( !JMPermissions.Has( JMConstants.PERM_WEATHER_SNOW ) )
			return;

		action.AnimateFeedback();
		ApplySnow();
	}

	void ApplyRain()
	{
		if ( m_Form.PreviewSection( JMWeatherForm.SECTION_RAIN ) )
			return;

		float rainTransition = m_Form.GlobalTransition();

		m_Form.m_Module.SetRain( ReadLow( m_SliderRainForecast ), rainTransition, m_Form.GlobalDuration(), ReadHi( m_SliderRainForecast ) );

		if ( m_RangeRainThreshold )
			m_Form.m_Module.SetRainThresholds( m_RangeRainThreshold.GetRangeLow() * 0.01, m_RangeRainThreshold.GetRangeHigh() * 0.01, rainTransition );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_RAIN, false );
	}

	void ApplySnow()
	{
		if ( m_Form.PreviewSection( JMWeatherForm.SECTION_SNOW ) )
			return;

		float snowTransition = m_Form.GlobalTransition();

		m_Form.m_Module.SetSnow( ReadLow( m_SliderSnowForecast ), snowTransition, m_Form.GlobalDuration(), ReadHi( m_SliderSnowForecast ) );

		if ( m_RangeSnowThreshold )
			m_Form.m_Module.SetSnowThresholds( m_RangeSnowThreshold.GetRangeLow() * 0.01, m_RangeSnowThreshold.GetRangeHigh() * 0.01, snowTransition );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_SNOW, false );
	}

	void UpdateStates()
	{
		m_Form.UpdateRangeState( m_SliderRainForecast, JMConstants.PERM_WEATHER_RAIN );
		m_Form.UpdateActionState( m_RangeRainThreshold, JMConstants.PERM_WEATHER_RAIN_THRESHOLDS );
		if ( m_ToggleRain ) m_ToggleRain.UpdateState( JMConstants.PERM_WEATHER_RAIN );

		m_Form.UpdateRangeState( m_SliderSnowForecast, JMConstants.PERM_WEATHER_SNOW );
		m_Form.UpdateActionState( m_RangeSnowThreshold, JMConstants.PERM_WEATHER_SNOW_THRESHOLDS );
		if ( m_ToggleSnow ) m_ToggleSnow.UpdateState( JMConstants.PERM_WEATHER_SNOW );
	}
}
