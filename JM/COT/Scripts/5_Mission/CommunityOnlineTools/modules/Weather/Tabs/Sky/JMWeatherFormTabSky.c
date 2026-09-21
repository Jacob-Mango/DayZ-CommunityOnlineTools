//! "Sky" tab of JMWeatherForm - Overcast, Fog (+dynamic fog), Storm and
//! Sandstorm cards. Back-reference to the owning form, same shape as
//! JMPlayerRowWidget.Menu.
//!
//! Apply/dirty-tracking/preview mechanics (PreviewSection, SetSectionDirty,
//! GlobalTransition/Duration) stay on the form - every tab's Apply routes
//! through them, and preset edit mode hijacks them for every section at once,
//! so they are not this tab's to own.
//!
//! Every numeric value here is a JMWeatherRangeControl: one value, or - with the
//! card's title-bar switch on - a span the server rolls between on each apply.
class JMWeatherFormTabSky: JMFormTab
{
	protected JMWeatherForm m_Form;
	protected UIActionScroller m_ScrollerSky;
	protected ref JMWeatherRangeControl m_SliderOvercastForecast;
	protected ref JMWeatherRangeControl m_SliderFogForecast;
	protected ref JMWeatherRangeControl m_SliderDynamicFogDistance;
	protected ref JMWeatherRangeControl m_SliderDynamicFogHeight;
	protected ref JMWeatherRangeControl m_SliderDynamicFogBias;
	protected ref JMWeatherRangeControl m_SliderStormDensity;
	protected ref JMWeatherRangeControl m_SliderStormThreshold;
	protected ref JMWeatherRangeControl m_SliderStormLightning;
	protected UIActionCheckbox m_CheckboxSandstormEnabled;
	protected ref JMWeatherRangeControl m_SliderSandstormDuration;
	protected ref JMWeatherRangeControl m_SliderSandstormFadeIn;
	protected ref JMWeatherRangeControl m_SliderSandstormOvercast;
	protected ref JMWeatherRangeControl m_SliderSandstormWindMagnitude;

	//! One switch per card - see JMWeatherRangeToggle.
	protected ref JMWeatherRangeToggle m_ToggleOvercast;
	protected ref JMWeatherRangeToggle m_ToggleFog;
	protected ref JMWeatherRangeToggle m_ToggleStorm;
	protected ref JMWeatherRangeToggle m_ToggleSandstorm;

	//! Seconds between lightning strikes when nothing else has been chosen.
	static const float DEFAULT_LIGHTNING_INTERVAL = 25;
	static const float DEFAULT_SANDSTORM_DURATION = 30;
	static const float DEFAULT_SANDSTORM_FADEIN = 10;
	static const float DEFAULT_SANDSTORM_OVERCAST = 0.8;
	static const float DEFAULT_SANDSTORM_WINDMAGNITUDE = 18.0;

	void JMWeatherFormTabSky( JMWeatherForm form )
	{
		m_Form = form;
	}

	//! Every card at once: on while a preset is being edited, off for the live world.
	void SetRangeMode( bool on )
	{
		if ( m_ToggleOvercast )  m_ToggleOvercast.SetRanges( on );
		if ( m_ToggleFog )       m_ToggleFog.SetRanges( on );
		if ( m_ToggleStorm )     m_ToggleStorm.SetRanges( on );
		if ( m_ToggleSandstorm ) m_ToggleSandstorm.SetRanges( on );
	}

	// -------------------------------------------------------------------------
	//  Reading the world (or a stored preset) into the controls - called by the
	//  form's SetUIActionValues() dispatcher.
	// -------------------------------------------------------------------------

	void SetFogValues( JMWeatherPreset preset, bool actual )
	{
		m_Form.SetPhenomenonRange( m_SliderFogForecast, preset.PFog, actual );

		if ( preset.PDynFog.Distance != -1 )
		{
			if ( m_SliderDynamicFogDistance ) m_SliderDynamicFogDistance.Set( preset.PDynFog.Distance, Math.Max( preset.PDynFog.DistanceHi, preset.PDynFog.Distance ) );
			if ( m_SliderDynamicFogHeight )   m_SliderDynamicFogHeight.Set( preset.PDynFog.Height, Math.Max( preset.PDynFog.HeightHi, preset.PDynFog.Height ) );
			if ( m_SliderDynamicFogBias )     m_SliderDynamicFogBias.Set( preset.PDynFog.Bias, Math.Max( preset.PDynFog.BiasHi, preset.PDynFog.Bias ) );
		}
	}

	void SetOvercastValues( JMWeatherPreset preset, bool actual )
	{
		m_Form.SetPhenomenonRange( m_SliderOvercastForecast, preset.POvercast, actual );
	}

	//! Start/Stop have no forecast either - IsActive is the only thing read
	//! back, and only into the checkbox. Duration is what the NEXT Apply will
	//! fade with, not anything the world remembers, so it is left alone here.
	void SetSandstormValues( JMWeatherPreset preset )
	{
		if ( preset.PSandstorm.Enabled == -1 )
			return;

		if ( m_CheckboxSandstormEnabled )
			m_CheckboxSandstormEnabled.SetChecked( preset.PSandstorm.Enabled == 1 );

		if ( m_SliderSandstormDuration && preset.PSandstorm.Duration != -1 )
			m_SliderSandstormDuration.Set( preset.PSandstorm.Duration, Math.Max( preset.PSandstorm.DurationHi, preset.PSandstorm.Duration ) );

		if ( m_SliderSandstormFadeIn && preset.PSandstorm.FadeInTime != -1 )
			m_SliderSandstormFadeIn.Set( preset.PSandstorm.FadeInTime, Math.Max( preset.PSandstorm.FadeInTimeHi, preset.PSandstorm.FadeInTime ) );

		if ( m_SliderSandstormOvercast && preset.PSandstorm.OvercastValue != -1 )
			m_SliderSandstormOvercast.Set( preset.PSandstorm.OvercastValue, Math.Max( preset.PSandstorm.OvercastValueHi, preset.PSandstorm.OvercastValue ) );

		if ( m_SliderSandstormWindMagnitude && preset.PSandstorm.WindMagnitudeValue != -1 )
			m_SliderSandstormWindMagnitude.Set( preset.PSandstorm.WindMagnitudeValue, Math.Max( preset.PSandstorm.WindMagnitudeValueHi, preset.PSandstorm.WindMagnitudeValue ) );
	}

	//! Storm has no getter in the engine, so SetFromWorld reports -1 and these
	//! keep whatever the admin last set rather than showing a number the server
	//! never had.
	void SetStormValues( JMWeatherPreset preset )
	{
		if ( preset.Storm.Density == -1 )
			return;

		if ( m_SliderStormDensity )   m_SliderStormDensity.Set( preset.Storm.Density, Math.Max( preset.Storm.DensityHi, preset.Storm.Density ) );
		if ( m_SliderStormThreshold ) m_SliderStormThreshold.Set( preset.Storm.Threshold, Math.Max( preset.Storm.ThresholdHi, preset.Storm.Threshold ) );
		if ( m_SliderStormLightning ) m_SliderStormLightning.Set( preset.Storm.MinTimeBetweenLightning, Math.Max( preset.Storm.MinTimeBetweenLightningHi, preset.Storm.MinTimeBetweenLightning ) );
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		m_ScrollerSky = UIActionManager.CreateScroller( panel );
		Widget content = m_ScrollerSky.GetContentWidget();

		UIActionCard overcastCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_OVERCAST" );
		overcastCard.AddApplyButton( this, "OnClick_ApplyOvercast" );
		m_ToggleOvercast = new JMWeatherRangeToggle( m_Form, overcastCard, JMWeatherForm.SECTION_OVERCAST );
		m_SliderOvercastForecast = m_Form.CreatePercentRange( overcastCard.GetContent(), "#STR_COT_GENERIC_AMOUNT", "OnChange_Overcast", this );
		m_ToggleOvercast.Add( m_SliderOvercastForecast );

		UIActionCard fogCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_FOG" );
		fogCard.AddApplyButton( this, "OnClick_ApplyFog" );
		m_ToggleFog = new JMWeatherRangeToggle( m_Form, fogCard, JMWeatherForm.SECTION_FOG );
		Widget fogBody = UIActionManager.CreateGridSpacer( fogCard.GetContent(), 4, 1 );

		m_SliderFogForecast = m_Form.CreatePercentRange( fogBody, "#STR_COT_GENERIC_AMOUNT", "OnChange_Fog", this );
		m_ToggleFog.Add( m_SliderFogForecast );

		m_SliderDynamicFogDistance = m_Form.CreatePercentRange( fogBody, "#STR_COT_GENERIC_DISTANCE", "OnChange_Fog", this );
		m_SliderDynamicFogDistance.SetTooltip( "#STR_COT_WEATHER_DYNFOG_DESC" );
		m_ToggleFog.Add( m_SliderDynamicFogDistance );

		m_SliderDynamicFogHeight = m_Form.CreatePercentRange( fogBody, "#STR_COT_GENERIC_HEIGHT", "OnChange_Fog", this );
		m_SliderDynamicFogHeight.SetTooltip( "#STR_COT_WEATHER_DYNFOG_DESC" );
		m_ToggleFog.Add( m_SliderDynamicFogHeight );

		//! Bias is in metres and signed, unlike every other control on this tab.
		//! It gets the metre format so it cannot be misread as a percentage.
		m_SliderDynamicFogBias = JMWeatherRangeControl.Create( fogBody, "#STR_COT_GENERIC_BIAS", -500, 500, 1, "#STR_COT_FORMAT_METRE", 1.0, 0.0, this, "OnChange_Fog" );
		m_SliderDynamicFogBias.SetTooltip( "#STR_COT_WEATHER_DYNFOG_DESC" );
		m_ToggleFog.Add( m_SliderDynamicFogBias );

		UIActionCard stormCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_STORM" );
		stormCard.AddApplyButton( this, "OnClick_ApplyStorm" );
		m_ToggleStorm = new JMWeatherRangeToggle( m_Form, stormCard, JMWeatherForm.SECTION_STORM );
		Widget stormBody = UIActionManager.CreateGridSpacer( stormCard.GetContent(), 3, 1 );

		m_SliderStormDensity = m_Form.CreatePercentRange( stormBody, "#STR_COT_WEATHER_DENSITY", "OnChange_Storm", this );
		m_SliderStormDensity.SetTooltip( "#STR_COT_WEATHER_DENSITY_DESC" );
		m_ToggleStorm.Add( m_SliderStormDensity );

		m_SliderStormThreshold = m_Form.CreatePercentRange( stormBody, "#STR_COT_WEATHER_THRESHOLD", "OnChange_Storm", this );
		m_SliderStormThreshold.SetTooltip( "#STR_COT_WEATHER_STORM_THRESHOLD_DESC" );
		m_ToggleStorm.Add( m_SliderStormThreshold );

		m_SliderStormLightning = JMWeatherRangeControl.Create( stormBody, "#STR_COT_WEATHER_LIGHTNING", 0, 120, 1, "#STR_COT_FORMAT_SECOND", 1.0, 0.0, this, "OnChange_Storm" );
		m_SliderStormLightning.SetValue( DEFAULT_LIGHTNING_INTERVAL );
		m_SliderStormLightning.SetTooltip( "#STR_COT_WEATHER_LIGHTNING_DESC" );
		m_ToggleStorm.Add( m_SliderStormLightning );

	#ifndef DAYZ_1_29
		//! 1.30+
		UIActionCard sandstormCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_SANDSTORM" );
		sandstormCard.AddApplyButton( this, "OnClick_ApplySandstorm" );
		m_ToggleSandstorm = new JMWeatherRangeToggle( m_Form, sandstormCard, JMWeatherForm.SECTION_SANDSTORM );
		Widget sandstormBody = UIActionManager.CreateGridSpacer( sandstormCard.GetContent(), 5, 1 );

		m_CheckboxSandstormEnabled = UIActionManager.CreateCheckbox( sandstormBody, "#STR_COT_WEATHER_SANDSTORM_ENABLED", this, "OnChange_Sandstorm" );
		m_CheckboxSandstormEnabled.SetTooltip( "#STR_COT_WEATHER_SANDSTORM_ENABLED_DESC" );

		m_SliderSandstormDuration = JMWeatherRangeControl.Create( sandstormBody, "#STR_COT_WEATHER_SANDSTORM_DURATION", 0, 120, 1, "#STR_COT_FORMAT_SECOND", 1.0, 0.0, this, "OnChange_Sandstorm" );
		m_SliderSandstormDuration.SetValue( DEFAULT_SANDSTORM_DURATION );
		m_SliderSandstormDuration.SetTooltip( "#STR_COT_WEATHER_SANDSTORM_DURATION_DESC" );
		m_ToggleSandstorm.Add( m_SliderSandstormDuration );

		m_SliderSandstormFadeIn = JMWeatherRangeControl.Create( sandstormBody, "#STR_COT_WEATHER_SANDSTORM_FADEIN", 0, 120, 1, "#STR_COT_FORMAT_SECOND", 1.0, 0.0, this, "OnChange_Sandstorm" );
		m_SliderSandstormFadeIn.SetValue( DEFAULT_SANDSTORM_FADEIN );
		m_SliderSandstormFadeIn.SetTooltip( "#STR_COT_WEATHER_SANDSTORM_FADEIN_DESC" );
		m_ToggleSandstorm.Add( m_SliderSandstormFadeIn );

		m_SliderSandstormOvercast = m_Form.CreatePercentRange( sandstormBody, "#STR_COT_WEATHER_SANDSTORM_OVERCAST", "OnChange_Sandstorm", this );
		m_SliderSandstormOvercast.SetValue( DEFAULT_SANDSTORM_OVERCAST );
		m_SliderSandstormOvercast.SetTooltip( "#STR_COT_WEATHER_SANDSTORM_OVERCAST_DESC" );
		m_ToggleSandstorm.Add( m_SliderSandstormOvercast );

		m_SliderSandstormWindMagnitude = JMWeatherRangeControl.Create( sandstormBody, "#STR_COT_WEATHER_SANDSTORM_WINDMAGNITUDE", 18, JMWeatherForm.WIND_SCALE_MAX, 0.5, "#STR_COT_FORMAT_MPS", 1.0, 0.0, this, "OnChange_Sandstorm" );
		m_SliderSandstormWindMagnitude.SetValue( DEFAULT_SANDSTORM_WINDMAGNITUDE );
		m_SliderSandstormWindMagnitude.SetTooltip( "#STR_COT_WEATHER_SANDSTORM_WINDMAGNITUDE_DESC" );
		m_ToggleSandstorm.Add( m_SliderSandstormWindMagnitude );
	#endif

		m_ScrollerSky.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_ScrollerSky )
			m_ScrollerSky.UpdateScroller();
	}

	// -------------------------------------------------------------------------
	//  Reading the controls into a preset - called by the form's
	//  GetUIActionValues() dispatcher.
	// -------------------------------------------------------------------------

	void ReadInto( JMWeatherPreset preset, float transition, float duration )
	{
		preset.Storm.Density                   = ReadLow( m_SliderStormDensity, 0 );
		preset.Storm.DensityHi                 = ReadHi( m_SliderStormDensity );
		preset.Storm.Threshold                 = ReadLow( m_SliderStormThreshold, 0 );
		preset.Storm.ThresholdHi               = ReadHi( m_SliderStormThreshold );
		preset.Storm.MinTimeBetweenLightning   = ReadLow( m_SliderStormLightning, DEFAULT_LIGHTNING_INTERVAL );
		preset.Storm.MinTimeBetweenLightningHi = ReadHi( m_SliderStormLightning );

		if ( m_CheckboxSandstormEnabled )
		{
			if ( m_CheckboxSandstormEnabled.IsChecked() )
				preset.PSandstorm.Enabled = 1;
			else
				preset.PSandstorm.Enabled = 0;
		}
		preset.PSandstorm.Duration             = ReadLow( m_SliderSandstormDuration, DEFAULT_SANDSTORM_DURATION );
		preset.PSandstorm.DurationHi           = ReadHi( m_SliderSandstormDuration );
		preset.PSandstorm.FadeInTime           = ReadLow( m_SliderSandstormFadeIn, DEFAULT_SANDSTORM_FADEIN );
		preset.PSandstorm.FadeInTimeHi         = ReadHi( m_SliderSandstormFadeIn );
		preset.PSandstorm.OvercastValue        = ReadLow( m_SliderSandstormOvercast, DEFAULT_SANDSTORM_OVERCAST );
		preset.PSandstorm.OvercastValueHi      = ReadHi( m_SliderSandstormOvercast );
		preset.PSandstorm.WindMagnitudeValue   = ReadLow( m_SliderSandstormWindMagnitude, DEFAULT_SANDSTORM_WINDMAGNITUDE );
		preset.PSandstorm.WindMagnitudeValueHi = ReadHi( m_SliderSandstormWindMagnitude );

		preset.POvercast.Forecast    = ReadLow( m_SliderOvercastForecast, 0 );
		preset.POvercast.ForecastHi  = ReadHi( m_SliderOvercastForecast );
		preset.POvercast.Time        = transition;
		preset.POvercast.MinDuration = duration;

		preset.PFog.Forecast    = ReadLow( m_SliderFogForecast, 0 );
		preset.PFog.ForecastHi  = ReadHi( m_SliderFogForecast );
		preset.PFog.Time        = transition;
		preset.PFog.MinDuration = duration;

		if ( m_SliderDynamicFogDistance )
		{
			preset.PDynFog.Distance   = m_SliderDynamicFogDistance.Low();
			preset.PDynFog.DistanceHi = m_SliderDynamicFogDistance.Hi();
		}

		if ( m_SliderDynamicFogHeight )
		{
			preset.PDynFog.Height   = m_SliderDynamicFogHeight.Low();
			preset.PDynFog.HeightHi = m_SliderDynamicFogHeight.Hi();
		}

		if ( m_SliderDynamicFogBias )
		{
			preset.PDynFog.Bias   = m_SliderDynamicFogBias.Low();
			preset.PDynFog.BiasHi = m_SliderDynamicFogBias.Hi();
		}

		preset.PDynFog.Time = transition;
	}

	//! A control's value, or `fallback` when the control was never built.
	protected float ReadLow( JMWeatherRangeControl control, float fallback )
	{
		if ( !control )
			return fallback;

		return control.Low();
	}

	protected float ReadHi( JMWeatherRangeControl control )
	{
		if ( !control )
			return 0;

		return control.Hi();
	}

	// -------------------------------------------------------------------------
	//  Dirty tracking
	// -------------------------------------------------------------------------

	void OnChange_Overcast( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_OVERCAST );
	}

	void OnChange_Fog( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_FOG );
	}

	void OnChange_Storm( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_STORM );
	}

	void OnChange_Sandstorm( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_SANDSTORM );
	}

	// -------------------------------------------------------------------------
	//  Apply
	// -------------------------------------------------------------------------

	void OnClick_ApplyOvercast( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module )
			return;
		if ( !JMPermissions.Has( JMConstants.PERM_WEATHER_OVERCAST ) )
			return;

		action.AnimateFeedback();
		ApplyOvercast();
	}

	void OnClick_ApplyFog( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module )
			return;
		if ( !JMPermissions.Has( JMConstants.PERM_WEATHER_FOG ) )
			return;

		action.AnimateFeedback();
		ApplyFog();
	}

	void OnClick_ApplyStorm( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module )
			return;
		if ( !JMPermissions.Has( JMConstants.PERM_WEATHER_STORM ) )
			return;

		action.AnimateFeedback();
		ApplyStorm();
	}

	void OnClick_ApplySandstorm( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module )
			return;
		if ( !JMPermissions.Has( JMConstants.PERM_WEATHER_SANDSTORM ) )
			return;

		action.AnimateFeedback();
		ApplySandstorm();
	}

	void ApplyOvercast()
	{
		if ( m_Form.PreviewSection( JMWeatherForm.SECTION_OVERCAST ) )
			return;

		m_Form.m_Module.SetOvercast( ReadLow( m_SliderOvercastForecast, 0 ), m_Form.GlobalTransition(), m_Form.GlobalDuration(), ReadHi( m_SliderOvercastForecast ) );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_OVERCAST, false );
	}

	void ApplyFog()
	{
		if ( m_Form.PreviewSection( JMWeatherForm.SECTION_FOG ) )
			return;

		float fogTransition = m_Form.GlobalTransition();

		m_Form.m_Module.SetFog( ReadLow( m_SliderFogForecast, 0 ), fogTransition, m_Form.GlobalDuration(), ReadHi( m_SliderFogForecast ) );

		if ( m_SliderDynamicFogDistance && m_SliderDynamicFogHeight && m_SliderDynamicFogBias )
			m_Form.m_Module.SetDynamicFog( m_SliderDynamicFogDistance.Low(), m_SliderDynamicFogHeight.Low(), m_SliderDynamicFogBias.Low(), fogTransition, m_SliderDynamicFogDistance.Hi(), m_SliderDynamicFogHeight.Hi(), m_SliderDynamicFogBias.Hi() );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_FOG, false );
	}

	void ApplyStorm()
	{
		if ( m_Form.PreviewSection( JMWeatherForm.SECTION_STORM ) )
			return;

		m_Form.m_Module.SetStorm( ReadStormDensity(), ReadStormThreshold(), StormLightningInterval(), ReadHi( m_SliderStormDensity ), ReadHi( m_SliderStormThreshold ), ReadHi( m_SliderStormLightning ) );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_STORM, false );
	}

	void ApplySandstorm()
	{
		if ( m_Form.PreviewSection( JMWeatherForm.SECTION_SANDSTORM ) )
			return;

		bool enabled = false;
		if ( m_CheckboxSandstormEnabled )
			enabled = m_CheckboxSandstormEnabled.IsChecked();

		m_Form.m_Module.SetSandstorm( enabled, SandstormDuration(), SandstormFadeIn(), SandstormOvercast(), SandstormWindMagnitude(), ReadHi( m_SliderSandstormDuration ), ReadHi( m_SliderSandstormFadeIn ), ReadHi( m_SliderSandstormOvercast ), ReadHi( m_SliderSandstormWindMagnitude ) );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_SANDSTORM, false );
	}

	//! Read by the Overview tab's quick-action strip too - a quick action
	//! preserves whatever storm density/threshold the admin already dialed in.
	float ReadStormDensity()
	{
		return ReadLow( m_SliderStormDensity, 0 );
	}

	float ReadStormThreshold()
	{
		return ReadLow( m_SliderStormThreshold, 0 );
	}

	float StormLightningInterval()
	{
		return ReadLow( m_SliderStormLightning, DEFAULT_LIGHTNING_INTERVAL );
	}

	float SandstormDuration()
	{
		return ReadLow( m_SliderSandstormDuration, DEFAULT_SANDSTORM_DURATION );
	}

	float SandstormFadeIn()
	{
		return ReadLow( m_SliderSandstormFadeIn, DEFAULT_SANDSTORM_FADEIN );
	}

	float SandstormOvercast()
	{
		return ReadLow( m_SliderSandstormOvercast, DEFAULT_SANDSTORM_OVERCAST );
	}

	float SandstormWindMagnitude()
	{
		return ReadLow( m_SliderSandstormWindMagnitude, DEFAULT_SANDSTORM_WINDMAGNITUDE );
	}

	// -------------------------------------------------------------------------
	//  Permissions / enable state - called by the form's UpdateStates().
	// -------------------------------------------------------------------------

	void UpdateStates()
	{
		m_Form.UpdateRangeState( m_SliderStormDensity, JMConstants.PERM_WEATHER_STORM );
		m_Form.UpdateRangeState( m_SliderStormThreshold, JMConstants.PERM_WEATHER_STORM );
		m_Form.UpdateRangeState( m_SliderStormLightning, JMConstants.PERM_WEATHER_STORM );
		if ( m_ToggleStorm ) m_ToggleStorm.UpdateState( JMConstants.PERM_WEATHER_STORM );

		m_Form.UpdateActionState( m_CheckboxSandstormEnabled, JMConstants.PERM_WEATHER_SANDSTORM );
		m_Form.UpdateRangeState( m_SliderSandstormDuration, JMConstants.PERM_WEATHER_SANDSTORM );
		m_Form.UpdateRangeState( m_SliderSandstormFadeIn, JMConstants.PERM_WEATHER_SANDSTORM );
		m_Form.UpdateRangeState( m_SliderSandstormOvercast, JMConstants.PERM_WEATHER_SANDSTORM );
		m_Form.UpdateRangeState( m_SliderSandstormWindMagnitude, JMConstants.PERM_WEATHER_SANDSTORM );
		if ( m_ToggleSandstorm ) m_ToggleSandstorm.UpdateState( JMConstants.PERM_WEATHER_SANDSTORM );

		m_Form.UpdateRangeState( m_SliderOvercastForecast, JMConstants.PERM_WEATHER_OVERCAST );
		if ( m_ToggleOvercast ) m_ToggleOvercast.UpdateState( JMConstants.PERM_WEATHER_OVERCAST );

		m_Form.UpdateRangeState( m_SliderFogForecast, JMConstants.PERM_WEATHER_FOG );
		m_Form.UpdateRangeState( m_SliderDynamicFogDistance, JMConstants.PERM_WEATHER_FOG_DYNAMIC );
		m_Form.UpdateRangeState( m_SliderDynamicFogHeight, JMConstants.PERM_WEATHER_FOG_DYNAMIC );
		m_Form.UpdateRangeState( m_SliderDynamicFogBias, JMConstants.PERM_WEATHER_FOG_DYNAMIC );
		if ( m_ToggleFog ) m_ToggleFog.UpdateState( JMConstants.PERM_WEATHER_FOG );
	}
}
