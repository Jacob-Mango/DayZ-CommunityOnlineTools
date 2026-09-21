//! "Wind" tab of JMWeatherForm - magnitude/direction and the wind function
//! curve. Back-reference to the owning form, same shape as
//! JMPlayerRowWidget.Menu. See JMWeatherFormTabSky.c's header for why
//! Apply/dirty-tracking/preview mechanics stay on the form.
//!
//! Every value is a JMWeatherRangeControl: one value, or a span the server rolls
//! between on each apply. Direction is stored as radians in <-PI,+PI> and shown as
//! a compass heading in degrees, which is just a scale and an offset.
class JMWeatherFormTabWind: JMFormTab
{
	protected JMWeatherForm m_Form;
	protected UIActionScroller m_ScrollerWind;
	protected ref JMWeatherRangeControl m_SliderWindMagnitude;
	protected ref JMWeatherRangeControl m_SliderWindDirection;
	protected ref JMWeatherRangeControl m_SliderWindFuncMin;
	protected ref JMWeatherRangeControl m_SliderWindFuncMax;
	protected ref JMWeatherRangeControl m_SliderWindFuncSpeed;

	protected ref JMWeatherRangeToggle m_ToggleWind;
	protected ref JMWeatherRangeToggle m_ToggleWindFunction;

	static autoptr TStringArray CARDINAL_DIRECTIONS = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};

	void JMWeatherFormTabWind( JMWeatherForm form )
	{
		m_Form = form;
	}

	//! Every card at once: on while a preset is being edited, off for the live world.
	void SetRangeMode( bool on )
	{
		if ( m_ToggleWind )         m_ToggleWind.SetRanges( on );
		if ( m_ToggleWindFunction ) m_ToggleWindFunction.SetRanges( on );
	}

	void SetWindFunctionValues( JMWeatherPreset preset )
	{
		if ( preset.WindFunc.Speed == -1 )
			return;

		if ( m_SliderWindFuncMin )   m_SliderWindFuncMin.Set( preset.WindFunc.Min, Math.Max( preset.WindFunc.MinHi, preset.WindFunc.Min ) );
		if ( m_SliderWindFuncMax )   m_SliderWindFuncMax.Set( preset.WindFunc.Max, Math.Max( preset.WindFunc.MaxHi, preset.WindFunc.Max ) );
		if ( m_SliderWindFuncSpeed ) m_SliderWindFuncSpeed.Set( preset.WindFunc.Speed, Math.Max( preset.WindFunc.SpeedHi, preset.WindFunc.Speed ) );
	}

	void SetWindValues( JMWeatherPreset preset, bool actual )
	{
		m_Form.SetPhenomenonRange( m_SliderWindMagnitude, preset.PWindMagnitude, actual );

		//! With nothing stored the heading falls back to 0 radians - due south on
		//! the scale below - rather than keeping whatever was last on screen.
		if ( !m_SliderWindDirection )
			return;

		if ( actual || preset.PWindDirection.Forecast != -1 )
			m_Form.SetPhenomenonRange( m_SliderWindDirection, preset.PWindDirection, actual );
		else
			m_SliderWindDirection.SetValue( 0 );
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		m_ScrollerWind = UIActionManager.CreateScroller( panel );
		Widget content = m_ScrollerWind.GetContentWidget();

		UIActionCard windCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_WIND" );
		windCard.AddApplyButton( this, "OnClick_ApplyWind" );
		m_ToggleWind = new JMWeatherRangeToggle( m_Form, windCard, JMWeatherForm.SECTION_WIND );
		Widget windBody = UIActionManager.CreateGridSpacer( windCard.GetContent(), 2, 1 );

		//! Wind magnitude is an absolute speed in m/s, NOT a 0-1 phenomenon like
		//! everything else on these tabs. The unit is in the value text because
		//! nothing else on screen would tell you.
		m_SliderWindMagnitude = JMWeatherRangeControl.Create( windBody, "#STR_COT_WEATHER_MAGNITUDE", 0, JMWeatherForm.WIND_SCALE_MAX, 0.1, "#STR_COT_FORMAT_MPS", 1.0, 0.0, this, "OnChange_Wind" );
		m_SliderWindMagnitude.SetTooltip( "#STR_COT_WEATHER_MAGNITUDE_DESC" );
		m_ToggleWind.Add( m_SliderWindMagnitude );

		//! Radians in <-PI,+PI> stored, degrees 0-360 shown: the engine's 0 is 180 on
		//! the compass. The degree symbol lives in the stringtable, not here - an
		//! Enforce .c must stay pure ASCII or it poisons the script type-pool.
		m_SliderWindDirection = JMWeatherRangeControl.Create( windBody, "#STR_COT_GENERIC_DIRECTION", 0, 360, 1, "#STR_COT_FORMAT_DEGREE", Math.RAD2DEG, 180.0, this, "OnChange_Wind" );
		m_SliderWindDirection.SetTooltip( "#STR_COT_WEATHER_DIRECTION_DESC" );
		m_ToggleWind.Add( m_SliderWindDirection );

		UIActionCard funcCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_FUNCTION" );
		funcCard.AddApplyButton( this, "OnClick_ApplyWindFunction" );
		m_ToggleWindFunction = new JMWeatherRangeToggle( m_Form, funcCard, JMWeatherForm.SECTION_WIND_FUNCTION );
		Widget funcBody = UIActionManager.CreateGridSpacer( funcCard.GetContent(), 3, 1 );

		m_SliderWindFuncMin = JMWeatherRangeControl.Create( funcBody, "#STR_COT_GENERIC_MIN", 0, 100, 1, "#STR_COT_FORMAT_PERCENTAGE", 100.0, 0.0, this, "OnChange_WindFunction" );
		m_ToggleWindFunction.Add( m_SliderWindFuncMin );

		m_SliderWindFuncMax = JMWeatherRangeControl.Create( funcBody, "#STR_COT_GENERIC_MAX", 0, 100, 1, "#STR_COT_FORMAT_PERCENTAGE", 100.0, 0.0, this, "OnChange_WindFunction" );
		m_ToggleWindFunction.Add( m_SliderWindFuncMax );

		m_SliderWindFuncSpeed = JMWeatherRangeControl.Create( funcBody, "#STR_COT_GENERIC_FREQUENCY", 0, 60, 1, "#STR_COT_FORMAT_SECOND", 1.0, 0.0, this, "OnChange_WindFunction" );
		m_SliderWindFuncSpeed.SetTooltip( "#STR_COT_WEATHER_FUNCTION_DESC" );
		m_ToggleWindFunction.Add( m_SliderWindFuncSpeed );

		m_ScrollerWind.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_ScrollerWind )
			m_ScrollerWind.UpdateScroller();
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
		if ( m_SliderWindMagnitude )
		{
			preset.PWindMagnitude.Forecast   = m_SliderWindMagnitude.Low();
			preset.PWindMagnitude.ForecastHi = m_SliderWindMagnitude.Hi();
		}
		preset.PWindMagnitude.Time        = transition;
		preset.PWindMagnitude.MinDuration = duration;

		if ( m_SliderWindDirection )
		{
			preset.PWindDirection.Forecast   = m_SliderWindDirection.Low();
			preset.PWindDirection.ForecastHi = m_SliderWindDirection.Hi();
		}
		preset.PWindDirection.Time        = transition;
		preset.PWindDirection.MinDuration = duration;

		if ( m_SliderWindFuncMin )
		{
			preset.WindFunc.Min   = m_SliderWindFuncMin.Low();
			preset.WindFunc.MinHi = m_SliderWindFuncMin.Hi();
		}

		if ( m_SliderWindFuncMax )
		{
			preset.WindFunc.Max   = m_SliderWindFuncMax.Low();
			preset.WindFunc.MaxHi = m_SliderWindFuncMax.Hi();
		}

		if ( m_SliderWindFuncSpeed )
		{
			preset.WindFunc.Speed   = m_SliderWindFuncSpeed.Low();
			preset.WindFunc.SpeedHi = m_SliderWindFuncSpeed.Hi();
		}
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
			m_Form.m_Module.SetWindMagnitude( m_SliderWindMagnitude.Low(), windTransition, windDuration, m_SliderWindMagnitude.Hi() );

		if ( m_SliderWindDirection )
			m_Form.m_Module.SetWindDirection( m_SliderWindDirection.Low(), windTransition, windDuration, m_SliderWindDirection.Hi() );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_WIND, false );
	}

	void ApplyWindFunction()
	{
		if ( m_Form.PreviewSection( JMWeatherForm.SECTION_WIND_FUNCTION ) )
			return;

		if ( m_SliderWindFuncMin && m_SliderWindFuncMax && m_SliderWindFuncSpeed )
			m_Form.m_Module.SetWindFunctionParams( m_SliderWindFuncMin.Low(), m_SliderWindFuncMax.Low(), m_SliderWindFuncSpeed.Low(), m_SliderWindFuncMin.Hi(), m_SliderWindFuncMax.Hi(), m_SliderWindFuncSpeed.Hi() );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_WIND_FUNCTION, false );
	}

	void UpdateStates()
	{
		m_Form.UpdateRangeState( m_SliderWindMagnitude, JMConstants.PERM_WEATHER_WIND );
		m_Form.UpdateRangeState( m_SliderWindDirection, JMConstants.PERM_WEATHER_WIND );
		if ( m_ToggleWind ) m_ToggleWind.UpdateState( JMConstants.PERM_WEATHER_WIND );

		m_Form.UpdateRangeState( m_SliderWindFuncMin, JMConstants.PERM_WEATHER_WIND_FUNCPARAMS );
		m_Form.UpdateRangeState( m_SliderWindFuncMax, JMConstants.PERM_WEATHER_WIND_FUNCPARAMS );
		m_Form.UpdateRangeState( m_SliderWindFuncSpeed, JMConstants.PERM_WEATHER_WIND_FUNCPARAMS );
		if ( m_ToggleWindFunction ) m_ToggleWindFunction.UpdateState( JMConstants.PERM_WEATHER_WIND_FUNCPARAMS );
	}

	static float PI2DEG( float value )
	{
		return ( value * Math.RAD2DEG ) + 180;
	}

	static float DEG2PI( float value )
	{
		return ( value * Math.DEG2RAD ) - Math.PI;
	}

	//! Compass point for a heading in degrees.
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
