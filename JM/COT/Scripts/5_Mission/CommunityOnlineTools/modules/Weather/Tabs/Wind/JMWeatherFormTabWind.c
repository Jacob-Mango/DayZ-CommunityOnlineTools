//! "Wind" tab of JMWeatherForm - magnitude/direction and the wind function
//! curve. Back-reference to the owning form, same shape as
//! JMPlayerRowWidget.Menu. See JMWeatherFormTabSky.c's header for why
//! Apply/dirty-tracking/preview mechanics stay on the form.
class JMWeatherFormTabWind: JMFormTab
{
	protected JMWeatherForm m_Form;
	protected UIActionScroller m_ScrollerWind;
	protected UIActionSlider m_SliderWindMagnitude;
	protected UIActionSlider m_SliderWindDirection;
	protected UIActionSlider m_SliderWindFuncMin;
	protected UIActionSlider m_SliderWindFuncMax;
	protected UIActionSlider m_SliderWindFuncSpeed;
	static autoptr TStringArray CARDINAL_DIRECTIONS = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};

	void JMWeatherFormTabWind( JMWeatherForm form )
	{
		m_Form = form;
	}

	void SetWindFunctionValues( JMWeatherPreset preset )
	{
		if ( preset.WindFunc.Speed == -1 )
			return;

		if ( m_SliderWindFuncMin )   m_SliderWindFuncMin.SetCurrent( preset.WindFunc.Min );
		if ( m_SliderWindFuncMax )   m_SliderWindFuncMax.SetCurrent( preset.WindFunc.Max );
		if ( m_SliderWindFuncSpeed ) m_SliderWindFuncSpeed.SetCurrent( preset.WindFunc.Speed );
	}

	void SetWindValues( JMWeatherPreset preset, bool actual )
	{
		if ( m_SliderWindMagnitude )
		{
			if ( actual )
				m_SliderWindMagnitude.SetCurrent( preset.PWindMagnitude.Actual );
			else if ( preset.PWindMagnitude.Forecast != -1 )
				m_SliderWindMagnitude.SetCurrent( preset.PWindMagnitude.Forecast );
		}

		float directionRad = 0;
		if ( actual )
			directionRad = preset.PWindDirection.Actual;
		else if ( preset.PWindDirection.Forecast != -1 )
			directionRad = preset.PWindDirection.Forecast;

		if ( m_SliderWindDirection )
		{
			m_SliderWindDirection.SetCurrent( PI2DEG( directionRad ) );
			UpdateWindDirectionSlider();
		}
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		m_ScrollerWind = UIActionManager.CreateScroller( panel );
		Widget content = m_ScrollerWind.GetContentWidget();

		UIActionCard windCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_WIND" );
		windCard.AddApplyButton( this, "OnClick_ApplyWind" );
		Widget windBody = UIActionManager.CreateGridSpacer( windCard.GetContent(), 2, 1 );

		//! Wind magnitude is an absolute speed in m/s, NOT a 0-1 phenomenon like
		//! everything else on these tabs. The unit is in the value text because
		//! nothing else on screen would tell you.
		m_SliderWindMagnitude = UIActionManager.CreateSyncedSlider( windBody, "#STR_COT_WEATHER_MAGNITUDE", 0, JMWeatherForm.WIND_SCALE_MAX, this, "OnChange_Wind" );
		m_SliderWindMagnitude.SetFormat( "#STR_COT_FORMAT_MPS" );
		m_SliderWindMagnitude.SetStepValue( 0.1 );
		m_SliderWindMagnitude.SetTooltip( "#STR_COT_WEATHER_MAGNITUDE_DESC" );

		m_SliderWindDirection = UIActionManager.CreateSyncedSlider( windBody, "#STR_COT_GENERIC_DIRECTION", 0, 360, this, "OnChange_WindDirection" );
		m_SliderWindDirection.SetFormat( "" );
		m_SliderWindDirection.SetStepValue( 1 );
		m_SliderWindDirection.SetTooltip( "#STR_COT_WEATHER_DIRECTION_DESC" );

		UIActionCard funcCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_FUNCTION" );
		funcCard.AddApplyButton( this, "OnClick_ApplyWindFunction" );
		Widget funcBody = UIActionManager.CreateGridSpacer( funcCard.GetContent(), 3, 1 );

		m_SliderWindFuncMin = UIActionManager.CreateSyncedSlider( funcBody, "#STR_COT_GENERIC_MIN", 0, 1, this, "OnChange_WindFunction" );
		m_SliderWindFuncMin.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderWindFuncMin.SetStepValue( 0.05 );

		m_SliderWindFuncMax = UIActionManager.CreateSyncedSlider( funcBody, "#STR_COT_GENERIC_MAX", 0, 1, this, "OnChange_WindFunction" );
		m_SliderWindFuncMax.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderWindFuncMax.SetStepValue( 0.05 );

		m_SliderWindFuncSpeed = UIActionManager.CreateSyncedSlider( funcBody, "#STR_COT_GENERIC_FREQUENCY", 0, 60, this, "OnChange_WindFunction" );
		m_SliderWindFuncSpeed.SetFormat( "#STR_COT_FORMAT_SECOND" );
		m_SliderWindFuncSpeed.SetStepValue( 1 );
		m_SliderWindFuncSpeed.SetTooltip( "#STR_COT_WEATHER_FUNCTION_DESC" );

		m_ScrollerWind.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_ScrollerWind )
			m_ScrollerWind.UpdateScroller();
	}

	void ReadInto( JMWeatherPreset preset, float transition, float duration )
	{
		if ( m_SliderWindMagnitude )
			preset.PWindMagnitude.Forecast = m_SliderWindMagnitude.GetCurrent();
		preset.PWindMagnitude.Time        = transition;
		preset.PWindMagnitude.MinDuration = duration;

		if ( m_SliderWindDirection )
			preset.PWindDirection.Forecast = DEG2PI( m_SliderWindDirection.GetCurrent() );
		preset.PWindDirection.Time        = transition;
		preset.PWindDirection.MinDuration = duration;

		if ( m_SliderWindFuncMin )   preset.WindFunc.Min   = m_SliderWindFuncMin.GetCurrent();
		if ( m_SliderWindFuncMax )   preset.WindFunc.Max   = m_SliderWindFuncMax.GetCurrent();
		if ( m_SliderWindFuncSpeed ) preset.WindFunc.Speed = m_SliderWindFuncSpeed.GetCurrent();
	}

	void OnChange_Wind( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_WIND );
	}

	void OnChange_WindFunction( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_WIND_FUNCTION );
	}

	void OnChange_WindDirection( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_WIND );

		UpdateWindDirectionSlider();
	}

	void UpdateWindDirectionSlider()
	{
		if ( !m_SliderWindDirection )
			return;

		float direction = m_SliderWindDirection.GetCurrent();

		//! The degree symbol lives in the stringtable, not here - an Enforce .c
		//! must stay pure ASCII or it poisons the script type-pool.
		string degrees = string.Format( Widget.TranslateString( "#STR_COT_FORMAT_DEGREE" ), Math.Round( direction ) );

		m_SliderWindDirection.SetText( CardinalFor( direction ) + "  " + degrees );
	}

	void OnClick_ApplyWind( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module )
			return;
		if ( !JMPermissions.Has( JMConstants.PERM_WEATHER_WIND ) )
			return;

		action.AnimateFeedback();
		ApplyWind();
	}

	void OnClick_ApplyWindFunction( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module )
			return;
		if ( !JMPermissions.Has( JMConstants.PERM_WEATHER_WIND_FUNCPARAMS ) )
			return;

		action.AnimateFeedback();
		ApplyWindFunction();
	}

	void ApplyWind()
	{
		if ( m_Form.PreviewSection( JMWeatherForm.SECTION_WIND ) )
			return;

		float windTransition = m_Form.GlobalTransition();
		float windDuration   = m_Form.GlobalDuration();

		if ( m_SliderWindMagnitude )
			m_Form.m_Module.SetWindMagnitude( m_SliderWindMagnitude.GetCurrent(), windTransition, windDuration );

		if ( m_SliderWindDirection )
			m_Form.m_Module.SetWindDirection( DEG2PI( m_SliderWindDirection.GetCurrent() ), windTransition, windDuration );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_WIND, false );
	}

	void ApplyWindFunction()
	{
		if ( m_Form.PreviewSection( JMWeatherForm.SECTION_WIND_FUNCTION ) )
			return;

		if ( m_SliderWindFuncMin && m_SliderWindFuncMax && m_SliderWindFuncSpeed )
			m_Form.m_Module.SetWindFunctionParams( m_SliderWindFuncMin.GetCurrent(), m_SliderWindFuncMax.GetCurrent(), m_SliderWindFuncSpeed.GetCurrent() );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_WIND_FUNCTION, false );
	}

	void UpdateStates()
	{
		m_Form.UpdateActionState( m_SliderWindMagnitude, JMConstants.PERM_WEATHER_WIND );
		m_Form.UpdateActionState( m_SliderWindDirection, JMConstants.PERM_WEATHER_WIND );
		m_Form.UpdateActionState( m_SliderWindFuncMin, JMConstants.PERM_WEATHER_WIND_FUNCPARAMS );
		m_Form.UpdateActionState( m_SliderWindFuncMax, JMConstants.PERM_WEATHER_WIND_FUNCPARAMS );
		m_Form.UpdateActionState( m_SliderWindFuncSpeed, JMConstants.PERM_WEATHER_WIND_FUNCPARAMS );
	}

	static float PI2DEG( float value )
	{
		return ( value * Math.RAD2DEG ) + 180;
	}

	static float DEG2PI( float value )
	{
		return ( value * Math.DEG2RAD ) - Math.PI;
	}

	//! Compass point for a heading in degrees, shared with the preset preview
	//! and with JMWeatherFormTabWind.
	static string CardinalFor( float degrees )
	{
		int index = Math.Floor( ( ( degrees + 22.5 ) / 45 ) );

		if ( index > CARDINAL_DIRECTIONS.Count() - 1 )
			index -= CARDINAL_DIRECTIONS.Count();
		else if ( index < 0 )
			index = 0;

		return CARDINAL_DIRECTIONS[index];
	}
}
