//! "Sky" tab of JMWeatherForm - Overcast, Fog (+dynamic fog), Storm and
//! Sandstorm cards. Back-reference to the owning form, same shape as
//! JMPlayerRowWidget.Menu.
//!
//! Apply/dirty-tracking/preview mechanics (PreviewSection, SetSectionDirty,
//! GlobalTransition/Duration) stay on the form - every tab's Apply routes
//! through them, and the Presets tab's preview mode hijacks them for every
//! section at once, so they are not this tab's to own.
class JMWeatherFormTabSky
{
	protected JMWeatherForm m_Form;

	protected UIActionScroller m_ScrollerSky;

	protected UIActionSlider m_SliderOvercastForecast;
	protected UIActionSlider m_SliderFogForecast;
	protected UIActionSlider m_SliderDynamicFogDistance;
	protected UIActionSlider m_SliderDynamicFogHeight;
	protected UIActionSlider m_SliderDynamicFogBias;
	protected UIActionSlider m_SliderStormDensity;
	protected UIActionSlider m_SliderStormThreshold;
	protected UIActionSlider m_SliderStormLightning;
	protected UIActionCheckbox m_CheckboxSandstormEnabled;
	protected UIActionSlider   m_SliderSandstormDuration;
	protected UIActionSlider   m_SliderSandstormFadeIn;
	protected UIActionSlider   m_SliderSandstormOvercast;
	protected UIActionSlider   m_SliderSandstormWindMagnitude;

	void JMWeatherFormTabSky( JMWeatherForm form )
	{
		m_Form = form;
	}

	void Build( Widget parent )
	{
		m_ScrollerSky = UIActionManager.CreateScroller( parent );
		Widget content = m_ScrollerSky.GetContentWidget();

		UIActionCard overcastCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_OVERCAST" );
		overcastCard.AddApplyButton( this, "OnClick_ApplyOvercast" );
		m_SliderOvercastForecast = m_Form.CreatePercentSlider( overcastCard.GetContent(), "#STR_COT_GENERIC_AMOUNT", "OnChange_Overcast", this );

		UIActionCard fogCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_FOG" );
		fogCard.AddApplyButton( this, "OnClick_ApplyFog" );
		Widget fogBody = UIActionManager.CreateGridSpacer( fogCard.GetContent(), 4, 1 );

		m_SliderFogForecast = m_Form.CreatePercentSlider( fogBody, "#STR_COT_GENERIC_AMOUNT", "OnChange_Fog", this );

		m_SliderDynamicFogDistance = UIActionManager.CreateSlider( fogBody, "#STR_COT_GENERIC_DISTANCE", 0, 1, this, "OnChange_Fog" );
		m_SliderDynamicFogDistance.SetStepValue( 0.01 );
		m_SliderDynamicFogDistance.SetTooltip( "#STR_COT_WEATHER_DYNFOG_DESC" );

		m_SliderDynamicFogHeight = UIActionManager.CreateSlider( fogBody, "#STR_COT_GENERIC_HEIGHT", 0, 1, this, "OnChange_Fog" );
		m_SliderDynamicFogHeight.SetStepValue( 0.01 );
		m_SliderDynamicFogHeight.SetTooltip( "#STR_COT_WEATHER_DYNFOG_DESC" );

		//! Bias is in metres and signed, unlike every other control on this tab.
		//! It gets the metre format so it cannot be misread as a percentage.
		m_SliderDynamicFogBias = UIActionManager.CreateSlider( fogBody, "#STR_COT_GENERIC_BIAS", -500, 500, this, "OnChange_Fog" );
		m_SliderDynamicFogBias.SetFormat( "#STR_COT_FORMAT_METRE" );
		m_SliderDynamicFogBias.SetStepValue( 1 );
		m_SliderDynamicFogBias.SetTooltip( "#STR_COT_WEATHER_DYNFOG_DESC" );

		UIActionCard stormCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_STORM" );
		stormCard.AddApplyButton( this, "OnClick_ApplyStorm" );
		Widget stormBody = UIActionManager.CreateGridSpacer( stormCard.GetContent(), 3, 1 );

		m_SliderStormDensity = m_Form.CreatePercentSlider( stormBody, "#STR_COT_WEATHER_DENSITY", "OnChange_Storm", this );
		m_SliderStormDensity.SetTooltip( "#STR_COT_WEATHER_DENSITY_DESC" );

		m_SliderStormThreshold = m_Form.CreatePercentSlider( stormBody, "#STR_COT_WEATHER_THRESHOLD", "OnChange_Storm", this );
		m_SliderStormThreshold.SetTooltip( "#STR_COT_WEATHER_STORM_THRESHOLD_DESC" );

		m_SliderStormLightning = UIActionManager.CreateSlider( stormBody, "#STR_COT_WEATHER_LIGHTNING", 0, 120, this, "OnChange_Storm" );
		m_SliderStormLightning.SetFormat( "#STR_COT_FORMAT_SECOND" );
		m_SliderStormLightning.SetStepValue( 1 );
		m_SliderStormLightning.SetCurrent( JMWeatherForm.DEFAULT_LIGHTNING_INTERVAL );
		m_SliderStormLightning.SetTooltip( "#STR_COT_WEATHER_LIGHTNING_DESC" );

	#ifndef DAYZ_1_29
		//! 1.30+
		UIActionCard sandstormCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_SANDSTORM" );
		sandstormCard.AddApplyButton( this, "OnClick_ApplySandstorm" );
		Widget sandstormBody = UIActionManager.CreateGridSpacer( sandstormCard.GetContent(), 2, 1 );

		m_CheckboxSandstormEnabled = UIActionManager.CreateCheckbox( sandstormBody, "#STR_COT_WEATHER_SANDSTORM_ENABLED", this, "OnChange_Sandstorm" );
		m_CheckboxSandstormEnabled.SetTooltip( "#STR_COT_WEATHER_SANDSTORM_ENABLED_DESC" );

		m_SliderSandstormDuration = UIActionManager.CreateSlider( sandstormBody, "#STR_COT_WEATHER_SANDSTORM_DURATION", 0, 120, this, "OnChange_Sandstorm" );
		m_SliderSandstormDuration.SetFormat( "#STR_COT_FORMAT_SECOND" );
		m_SliderSandstormDuration.SetStepValue( 1 );
		m_SliderSandstormDuration.SetCurrent( JMWeatherForm.DEFAULT_SANDSTORM_DURATION );
		m_SliderSandstormDuration.SetTooltip( "#STR_COT_WEATHER_SANDSTORM_DURATION_DESC" );

		m_SliderSandstormFadeIn = UIActionManager.CreateSlider( sandstormBody, "#STR_COT_WEATHER_SANDSTORM_FADEIN", 0, 120, this, "OnChange_Sandstorm" );
		m_SliderSandstormFadeIn.SetFormat( "#STR_COT_FORMAT_SECOND" );
		m_SliderSandstormFadeIn.SetStepValue( 1 );
		m_SliderSandstormFadeIn.SetCurrent( JMWeatherForm.DEFAULT_SANDSTORM_FADEIN );
		m_SliderSandstormFadeIn.SetTooltip( "#STR_COT_WEATHER_SANDSTORM_FADEIN_DESC" );

		m_SliderSandstormOvercast = m_Form.CreatePercentSlider( sandstormBody, "#STR_COT_WEATHER_SANDSTORM_OVERCAST", "OnChange_Sandstorm", this );
		m_SliderSandstormOvercast.SetCurrent( JMWeatherForm.DEFAULT_SANDSTORM_OVERCAST * 100.0 );
		m_SliderSandstormOvercast.SetTooltip( "#STR_COT_WEATHER_SANDSTORM_OVERCAST_DESC" );

		m_SliderSandstormWindMagnitude = UIActionManager.CreateSlider( sandstormBody, "#STR_COT_WEATHER_SANDSTORM_WINDMAGNITUDE", 18, JMWeatherForm.WIND_SCALE_MAX, this, "OnChange_Sandstorm" );
		m_SliderSandstormWindMagnitude.SetFormat( "#STR_COT_FORMAT_MPS" );
		m_SliderSandstormWindMagnitude.SetStepValue( 0.5 );
		m_SliderSandstormWindMagnitude.SetCurrent( JMWeatherForm.DEFAULT_SANDSTORM_WINDMAGNITUDE );
		m_SliderSandstormWindMagnitude.SetTooltip( "#STR_COT_WEATHER_SANDSTORM_WINDMAGNITUDE_DESC" );
	#endif

		m_ScrollerSky.UpdateScroller();
	}

	void OnResize()
	{
		if ( m_ScrollerSky )
			m_ScrollerSky.UpdateScroller();
	}

	// -------------------------------------------------------------------------
	//  Reading the world into the controls - called by the form's
	//  SetUIActionValues() dispatcher.
	// -------------------------------------------------------------------------

	void SetOvercastValues( JMWeatherPreset preset, bool actual )
	{
		m_Form.SetPercentSlider( m_SliderOvercastForecast, preset.POvercast, actual );
	}

	void SetFogValues( JMWeatherPreset preset, bool actual )
	{
		m_Form.SetPercentSlider( m_SliderFogForecast, preset.PFog, actual );

		if ( preset.PDynFog.Distance != -1 )
		{
			if ( m_SliderDynamicFogDistance ) m_SliderDynamicFogDistance.SetCurrent( preset.PDynFog.Distance );
			if ( m_SliderDynamicFogHeight )   m_SliderDynamicFogHeight.SetCurrent( preset.PDynFog.Height );
			if ( m_SliderDynamicFogBias )     m_SliderDynamicFogBias.SetCurrent( preset.PDynFog.Bias );
		}
	}

	//! Storm has no getter in the engine, so SetFromWorld reports -1 and these
	//! keep whatever the admin last set rather than showing a number the server
	//! never had.
	void SetStormValues( JMWeatherPreset preset )
	{
		if ( preset.Storm.Density == -1 )
			return;

		if ( m_SliderStormDensity )   m_SliderStormDensity.SetCurrent( preset.Storm.Density * 100.0 );
		if ( m_SliderStormThreshold ) m_SliderStormThreshold.SetCurrent( preset.Storm.Threshold * 100.0 );
		if ( m_SliderStormLightning ) m_SliderStormLightning.SetCurrent( preset.Storm.MinTimeBetweenLightning );
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

		if ( m_SliderSandstormFadeIn && preset.PSandstorm.FadeInTime != -1 )
			m_SliderSandstormFadeIn.SetCurrent( preset.PSandstorm.FadeInTime );

		if ( m_SliderSandstormOvercast && preset.PSandstorm.OvercastValue != -1 )
			m_SliderSandstormOvercast.SetCurrent( preset.PSandstorm.OvercastValue * 100.0 );

		if ( m_SliderSandstormWindMagnitude && preset.PSandstorm.WindMagnitudeValue != -1 )
			m_SliderSandstormWindMagnitude.SetCurrent( preset.PSandstorm.WindMagnitudeValue );
	}

	// -------------------------------------------------------------------------
	//  Reading the controls into a preset - called by the form's
	//  GetUIActionValues() dispatcher.
	// -------------------------------------------------------------------------

	void ReadInto( JMWeatherPreset preset, float transition, float duration )
	{
		preset.Storm.Density                 = m_Form.ReadPercent( m_SliderStormDensity );
		preset.Storm.Threshold               = m_Form.ReadPercent( m_SliderStormThreshold );
		preset.Storm.MinTimeBetweenLightning = StormLightningInterval();

		if ( m_CheckboxSandstormEnabled )
		{
			if ( m_CheckboxSandstormEnabled.IsChecked() )
				preset.PSandstorm.Enabled = 1;
			else
				preset.PSandstorm.Enabled = 0;
		}
		preset.PSandstorm.Duration           = SandstormDuration();
		preset.PSandstorm.FadeInTime         = SandstormFadeIn();
		preset.PSandstorm.OvercastValue      = SandstormOvercast();
		preset.PSandstorm.WindMagnitudeValue = SandstormWindMagnitude();

		preset.POvercast.Forecast    = m_Form.ReadPercent( m_SliderOvercastForecast );
		preset.POvercast.Time        = transition;
		preset.POvercast.MinDuration = duration;

		preset.PFog.Forecast    = m_Form.ReadPercent( m_SliderFogForecast );
		preset.PFog.Time        = transition;
		preset.PFog.MinDuration = duration;

		if ( m_SliderDynamicFogDistance ) preset.PDynFog.Distance = m_SliderDynamicFogDistance.GetCurrent();
		if ( m_SliderDynamicFogHeight )   preset.PDynFog.Height   = m_SliderDynamicFogHeight.GetCurrent();
		if ( m_SliderDynamicFogBias )     preset.PDynFog.Bias     = m_SliderDynamicFogBias.GetCurrent();
		preset.PDynFog.Time = transition;
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

		m_Form.m_Module.SetOvercast( m_Form.ReadPercent( m_SliderOvercastForecast ), m_Form.GlobalTransition(), m_Form.GlobalDuration() );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_OVERCAST, false );
	}

	void ApplyFog()
	{
		if ( m_Form.PreviewSection( JMWeatherForm.SECTION_FOG ) )
			return;

		float fogTransition = m_Form.GlobalTransition();

		m_Form.m_Module.SetFog( m_Form.ReadPercent( m_SliderFogForecast ), fogTransition, m_Form.GlobalDuration() );

		if ( m_SliderDynamicFogDistance && m_SliderDynamicFogHeight && m_SliderDynamicFogBias )
			m_Form.m_Module.SetDynamicFog( m_SliderDynamicFogDistance.GetCurrent(), m_SliderDynamicFogHeight.GetCurrent(), m_SliderDynamicFogBias.GetCurrent(), fogTransition );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_FOG, false );
	}

	void ApplyStorm()
	{
		if ( m_Form.PreviewSection( JMWeatherForm.SECTION_STORM ) )
			return;

		m_Form.m_Module.SetStorm( m_Form.ReadPercent( m_SliderStormDensity ), m_Form.ReadPercent( m_SliderStormThreshold ), StormLightningInterval() );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_STORM, false );
	}

	void ApplySandstorm()
	{
		if ( m_Form.PreviewSection( JMWeatherForm.SECTION_SANDSTORM ) )
			return;

		bool enabled = false;
		if ( m_CheckboxSandstormEnabled )
			enabled = m_CheckboxSandstormEnabled.IsChecked();

		m_Form.m_Module.SetSandstorm( enabled, SandstormDuration(), SandstormFadeIn(), SandstormOvercast(), SandstormWindMagnitude() );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_SANDSTORM, false );
	}

	//! Read by the Overview tab's quick-action strip too - a quick action
	//! preserves whatever storm density/threshold the admin already dialed in.
	float ReadStormDensity()
	{
		return m_Form.ReadPercent( m_SliderStormDensity );
	}

	float ReadStormThreshold()
	{
		return m_Form.ReadPercent( m_SliderStormThreshold );
	}

	float StormLightningInterval()
	{
		if ( m_SliderStormLightning )
			return m_SliderStormLightning.GetCurrent();

		return JMWeatherForm.DEFAULT_LIGHTNING_INTERVAL;
	}

	float SandstormDuration()
	{
		if ( m_SliderSandstormDuration )
			return m_SliderSandstormDuration.GetCurrent();

		return JMWeatherForm.DEFAULT_SANDSTORM_DURATION;
	}

	float SandstormFadeIn()
	{
		if ( m_SliderSandstormFadeIn )
			return m_SliderSandstormFadeIn.GetCurrent();

		return JMWeatherForm.DEFAULT_SANDSTORM_FADEIN;
	}

	float SandstormOvercast()
	{
		if ( m_SliderSandstormOvercast )
			return m_Form.ReadPercent( m_SliderSandstormOvercast );

		return JMWeatherForm.DEFAULT_SANDSTORM_OVERCAST;
	}

	float SandstormWindMagnitude()
	{
		if ( m_SliderSandstormWindMagnitude )
			return m_SliderSandstormWindMagnitude.GetCurrent();

		return JMWeatherForm.DEFAULT_SANDSTORM_WINDMAGNITUDE;
	}

	// -------------------------------------------------------------------------
	//  Permissions / enable state - called by the form's UpdateStates().
	// -------------------------------------------------------------------------

	void UpdateStates()
	{
		m_Form.UpdateActionState( m_SliderStormDensity, JMConstants.PERM_WEATHER_STORM );
		m_Form.UpdateActionState( m_SliderStormThreshold, JMConstants.PERM_WEATHER_STORM );
		m_Form.UpdateActionState( m_SliderStormLightning, JMConstants.PERM_WEATHER_STORM );

		m_Form.UpdateActionState( m_CheckboxSandstormEnabled, JMConstants.PERM_WEATHER_SANDSTORM );
		m_Form.UpdateActionState( m_SliderSandstormDuration, JMConstants.PERM_WEATHER_SANDSTORM );
		m_Form.UpdateActionState( m_SliderSandstormFadeIn, JMConstants.PERM_WEATHER_SANDSTORM );
		m_Form.UpdateActionState( m_SliderSandstormOvercast, JMConstants.PERM_WEATHER_SANDSTORM );
		m_Form.UpdateActionState( m_SliderSandstormWindMagnitude, JMConstants.PERM_WEATHER_SANDSTORM );

		m_Form.UpdateActionState( m_SliderOvercastForecast, JMConstants.PERM_WEATHER_OVERCAST );

		m_Form.UpdateActionState( m_SliderFogForecast, JMConstants.PERM_WEATHER_FOG );
		m_Form.UpdateActionState( m_SliderDynamicFogDistance, JMConstants.PERM_WEATHER_FOG_DYNAMIC );
		m_Form.UpdateActionState( m_SliderDynamicFogHeight, JMConstants.PERM_WEATHER_FOG_DYNAMIC );
		m_Form.UpdateActionState( m_SliderDynamicFogBias, JMConstants.PERM_WEATHER_FOG_DYNAMIC );
	}
}
