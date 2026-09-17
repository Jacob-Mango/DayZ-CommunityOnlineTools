//! "Precipitation" tab of JMWeatherForm - Rain and Snow cards. Back-reference
//! to the owning form, same shape as JMPlayerRowWidget.Menu. See
//! JMWeatherFormTabSky.c's header for why Apply/dirty-tracking/preview
//! mechanics stay on the form.
class JMWeatherFormTabPrecipitation
{
	protected JMWeatherForm m_Form;

	protected UIActionScroller m_ScrollerPrecipitation;

	protected UIActionSlider      m_SliderRainForecast;
	protected UIActionSliderRange m_RangeRainThreshold;
	protected UIActionSlider      m_SliderSnowForecast;
	protected UIActionSliderRange m_RangeSnowThreshold;

	void JMWeatherFormTabPrecipitation( JMWeatherForm form )
	{
		m_Form = form;
	}

	void Build( Widget parent )
	{
		m_ScrollerPrecipitation = UIActionManager.CreateScroller( parent );
		Widget content = m_ScrollerPrecipitation.GetContentWidget();

		UIActionCard rainCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_RAIN" );
		rainCard.AddApplyButton( this, "OnClick_ApplyRain" );
		Widget rainBody = UIActionManager.CreateGridSpacer( rainCard.GetContent(), 2, 1 );

		m_SliderRainForecast = m_Form.CreatePercentSlider( rainBody, "#STR_COT_GENERIC_AMOUNT", "OnChange_Rain", this );

		//! One range control instead of two sliders that must not cross. The
		//! pair is a single "rain is possible between these overcast levels"
		//! statement, and a Min above Max is not a state worth allowing.
		m_RangeRainThreshold = UIActionManager.CreateSliderRange( rainBody, "#STR_COT_WEATHER_THRESHOLDS", 0, 100, this, "OnChange_Rain" );
		m_RangeRainThreshold.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_RangeRainThreshold.SetStep( 1 );
		m_RangeRainThreshold.SetTooltip( "#STR_COT_WEATHER_THRESHOLDS_DESC" );

		UIActionCard snowCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_SNOW" );
		snowCard.AddApplyButton( this, "OnClick_ApplySnow" );
		Widget snowBody = UIActionManager.CreateGridSpacer( snowCard.GetContent(), 2, 1 );

		m_SliderSnowForecast = m_Form.CreatePercentSlider( snowBody, "#STR_COT_GENERIC_AMOUNT", "OnChange_Snow", this );

		m_RangeSnowThreshold = UIActionManager.CreateSliderRange( snowBody, "#STR_COT_WEATHER_THRESHOLDS", 0, 100, this, "OnChange_Snow" );
		m_RangeSnowThreshold.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_RangeSnowThreshold.SetStep( 1 );
		m_RangeSnowThreshold.SetTooltip( "#STR_COT_WEATHER_THRESHOLDS_DESC" );

		m_ScrollerPrecipitation.UpdateScroller();
	}

	void OnResize()
	{
		if ( m_ScrollerPrecipitation )
			m_ScrollerPrecipitation.UpdateScroller();
	}

	void SetRainValues( JMWeatherPreset preset, bool actual )
	{
		m_Form.SetPercentSlider( m_SliderRainForecast, preset.PRain, actual );

		if ( preset.RainThreshold.Time != -1 && m_RangeRainThreshold )
			m_RangeRainThreshold.SetRange( preset.RainThreshold.OvercastMin * 100.0, preset.RainThreshold.OvercastMax * 100.0 );
	}

	void SetSnowValues( JMWeatherPreset preset, bool actual )
	{
		m_Form.SetPercentSlider( m_SliderSnowForecast, preset.PSnow, actual );

		if ( preset.SnowThreshold.Time != -1 && m_RangeSnowThreshold )
			m_RangeSnowThreshold.SetRange( preset.SnowThreshold.OvercastMin * 100.0, preset.SnowThreshold.OvercastMax * 100.0 );
	}

	void ReadInto( JMWeatherPreset preset, float transition, float duration )
	{
		preset.PRain.Forecast    = m_Form.ReadPercent( m_SliderRainForecast );
		preset.PRain.Time        = transition;
		preset.PRain.MinDuration = duration;

		if ( m_RangeRainThreshold )
		{
			preset.RainThreshold.OvercastMin = m_RangeRainThreshold.GetRangeLow() * 0.01;
			preset.RainThreshold.OvercastMax = m_RangeRainThreshold.GetRangeHigh() * 0.01;
		}
		preset.RainThreshold.Time = transition;

		preset.PSnow.Forecast    = m_Form.ReadPercent( m_SliderSnowForecast );
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

		m_Form.m_Module.SetRain( m_Form.ReadPercent( m_SliderRainForecast ), rainTransition, m_Form.GlobalDuration() );

		if ( m_RangeRainThreshold )
			m_Form.m_Module.SetRainThresholds( m_RangeRainThreshold.GetRangeLow() * 0.01, m_RangeRainThreshold.GetRangeHigh() * 0.01, rainTransition );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_RAIN, false );
	}

	void ApplySnow()
	{
		if ( m_Form.PreviewSection( JMWeatherForm.SECTION_SNOW ) )
			return;

		float snowTransition = m_Form.GlobalTransition();

		m_Form.m_Module.SetSnow( m_Form.ReadPercent( m_SliderSnowForecast ), snowTransition, m_Form.GlobalDuration() );

		if ( m_RangeSnowThreshold )
			m_Form.m_Module.SetSnowThresholds( m_RangeSnowThreshold.GetRangeLow() * 0.01, m_RangeSnowThreshold.GetRangeHigh() * 0.01, snowTransition );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_SNOW, false );
	}

	void UpdateStates()
	{
		m_Form.UpdateActionState( m_SliderRainForecast, JMConstants.PERM_WEATHER_RAIN );
		m_Form.UpdateActionState( m_RangeRainThreshold, JMConstants.PERM_WEATHER_RAIN_THRESHOLDS );

		m_Form.UpdateActionState( m_SliderSnowForecast, JMConstants.PERM_WEATHER_SNOW );
		m_Form.UpdateActionState( m_RangeSnowThreshold, JMConstants.PERM_WEATHER_SNOW_THRESHOLDS );
	}
}
