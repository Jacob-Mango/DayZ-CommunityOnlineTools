// =============================================================================
//  JMWeatherForm
//
//  Six tabs over one weather controller: Overview, Sky, Precipitation, Wind,
//  Time, Presets.
//
//  ---------------------------------------------------------------------------
//  WHAT REPLACED WHAT
//
//  The old form was a single scrolling page carrying every control at once,
//  plus eight [x] show/hide buttons and an "Easy Mode" checkbox that existed
//  only to make that page manageable. Both are gone: tabs do the job the
//  toggles were doing, and Easy Mode's other half - forcing sane transition and
//  duration numbers - is now simply the default.
//
//  That removal changes what Apply means. The old Apply read panel VISIBILITY
//  as intent ("if ( m_PanelFog.IsVisible() ) SetFog(...)"), which only worked
//  because the toggles could hide a section. With tabs, one tab is visible at a
//  time and that rule would silently shrink Apply to the current tab. So intent
//  is now tracked explicitly: every control marks its section dirty, each
//  section card carries its own Apply, and the Overview's Apply All writes the
//  sections that were actually touched. Nothing is written that the admin did
//  not move.
//  ---------------------------------------------------------------------------
//
//  The Overview's live readout is not new logic - JMWorldConditions already
//  classifies current and forecast weather for the sidebar, and this consumes
//  the same API so the two can never disagree about what "rain" means.
// =============================================================================

class JMWeatherForm: JMFormBase
{
	//! Tab indices - what the strip's AddTab() returned for each tab, never written as numbers.
	protected int m_TabIdOverview;
	protected int m_TabIdSky;
	protected int m_TabIdPrecipitation;
	protected int m_TabIdWind;
	protected int m_TabIdTime;
	protected int m_TabIdPresets;

	//! UNUSED - kept only so an external reference does not break. The Presets
	//! tab no longer pre-builds rows; see JMWeatherFormTabPresets.
	//!
	//! Rows of preset widgets built into the Presets tab. 100 was the old
	//! ceiling and no server has come close to it.
	static const int PRESET_ROWS = 100;

	//! Defaults for the two global timing fields. These were what Easy Mode
	//! forced; with Easy Mode gone they are simply what the form starts at.
	static const float DEFAULT_TRANSITION = 0;
	static const float DEFAULT_DURATION   = 240;

	//! Top of the wind scale, shared by the Overview readout and the Wind tab.
	static const float WIND_SCALE_MAX = 20;

	//! Height of the mode banner, in the same layout units as the base class's
	//! TAB_STRIP_HEIGHT - the two are pinned together, see OnResize.
	static const int BANNER_HEIGHT = 26;

	//! Which block of controls an Apply covers. One per CARD, not per tab: a
	//! card's Apply button writes that card and nothing else, so pressing Apply
	//! on Fog cannot also push the overcast and storm sliders sitting on the
	//! same tab.
	static const int SECTION_OVERCAST      = 0;
	static const int SECTION_FOG           = 1;
	static const int SECTION_STORM         = 2;
	static const int SECTION_RAIN          = 3;
	static const int SECTION_SNOW          = 4;
	static const int SECTION_WIND          = 5;
	static const int SECTION_WIND_FUNCTION = 6;
	static const int SECTION_TIME          = 7;
	static const int SECTION_SANDSTORM     = 8;

	//! Public enough (no modifier), not protected: JMWeatherFormTabSky/
	//! Precipitation/Wind/Time reach this through their back-reference.
	JMWeatherModule m_Module;
	protected UIActionTabs m_Tabs;

	//! One class per tab, in its own file - only this form constructs/
	//! dispatches to them. What every tab needs from the others stays here as
	//! a thin forward: GlobalTransition/GlobalDuration (Overview's pickers),
	//! PreviewSection and the dirty mask (the Presets selection decides whether an
	//! Apply is a preview).
	ref JMWeatherFormTabOverview      m_TabOverviewCtrl;
	ref JMWeatherFormTabSky           m_TabSkyCtrl;
	ref JMWeatherFormTabPrecipitation m_TabPrecipitationCtrl;
	ref JMWeatherFormTabWind          m_TabWindCtrl;
	ref JMWeatherFormTabTime          m_TabTimeCtrl;
	ref JMWeatherFormTabPresets       m_TabPresetsCtrl;

	//! How often the dynamic weather machine's position is asked for while the
	//! form is open. The countdown runs locally in between - see
	//! JMWeatherDynamicStatus.Remaining - so this only has to be often enough to
	//! catch a phase change and a configuration revision.
	static const float DYNAMIC_POLL_INTERVAL = 2.0;
	protected float m_LastDynamicPoll;

	//! Bitmask tracking which sections the admin has touched since last refresh.
	protected int m_DirtySectionsMask;

	//! The mode banner across the top: which mode the tabs are in, plus Save / Close.
	protected ref JMWeatherModeBanner m_Banner;

	// -------------------------------------------------------------------------
	//  Reading the controls into a preset
	// -------------------------------------------------------------------------

	void GetUIActionValues( out JMWeatherPreset preset )
	{
		float transition = GlobalTransition();
		float duration   = GlobalDuration();

		if ( m_TabPresetsCtrl )
			preset.Name = m_TabPresetsCtrl.GetEditedName();

		//! A preset carries no date - the Time tab is disabled while one is edited, and
		//! -1 is what makes JMWeatherDate.Apply leave the clock alone.
		if ( IsPresetMode() )
			preset.ClearDate();
		else if ( m_TabTimeCtrl )
			m_TabTimeCtrl.ReadInto( preset );

		if ( m_TabSkyCtrl )
			m_TabSkyCtrl.ReadInto( preset, transition, duration );

		if ( m_TabPrecipitationCtrl )
			m_TabPrecipitationCtrl.ReadInto( preset, transition, duration );

		if ( m_TabWindCtrl )
			m_TabWindCtrl.ReadInto( preset, transition, duration );
	}

	// -------------------------------------------------------------------------
	//  Preset edit mode
	//
	//  With a preset picked on the Presets tab, every other tab edits THAT
	//  preset instead of the live world: the controls are loaded from it, the
	//  live poll stops writing over them, and Apply becomes a local preview.
	//  The selection itself belongs to JMWeatherFormTabPresets; until that tab
	//  has been opened nothing is selected and the answer is "live world".
	// -------------------------------------------------------------------------

	bool IsPresetMode()
	{
		return m_TabPresetsCtrl && m_TabPresetsCtrl.IsPresetMode();
	}

	bool IsSectionDirty( int sectionId )
	{
		return ( m_DirtySectionsMask & ( 1 << sectionId ) ) != 0;
	}

	//! Any card has been touched since the last load - what the Presets tab's Save
	//! button glows for while a preset is being edited.
	bool HasDirtySections()
	{
		return m_DirtySectionsMask != 0;
	}

	//! Public: every per-tab Set*Values() calls this to paint a control from a
	//! phenomenon's actual or stored value. A stored value that is a RANGE (its Hi
	//! twin above it) paints as a span; anything else as one value. The control
	//! itself converts to the units it shows.
	void SetPhenomenonRange( JMWeatherRangeControl control, JMWeatherPhenomenon phenomenon, bool actual )
	{
		if ( !control )
			return;

		if ( actual )
			control.SetValue( phenomenon.Actual );
		else if ( phenomenon.Forecast != -1 )
			control.Set( phenomenon.Forecast, Math.Max( phenomenon.ForecastHi, phenomenon.Forecast ) );
	}

	void SetSectionDirty( int sectionId, bool dirty = true )
	{
		if ( dirty )
			m_DirtySectionsMask = m_DirtySectionsMask | ( 1 << sectionId );
		else
			m_DirtySectionsMask = m_DirtySectionsMask & ~( 1 << sectionId );

		UpdateApplyButtonStates();
	}

	void SetSelectedPreset( string preset )
	{
		if ( m_TabPresetsCtrl )
			m_TabPresetsCtrl.SetSelectedPreset( preset );
	}

	//! A section the admin has edited is left alone. Otherwise auto-refresh
	//! would overwrite a slider between the moment it is dragged and the moment
	//! Apply is pressed. Dispatches into whichever tab owns each section - see
	//! JMWeatherFormTabSky.c's header for why the dispatch itself stays here.
	void SetUIActionValues( JMWeatherPreset preset, bool actual = false )
	{
		if ( !IsSectionDirty( SECTION_TIME ) && m_TabTimeCtrl )
			m_TabTimeCtrl.SetTimeValues( preset );

		if ( !IsSectionDirty( SECTION_OVERCAST ) && m_TabSkyCtrl )
			m_TabSkyCtrl.SetOvercastValues( preset, actual );

		if ( !IsSectionDirty( SECTION_FOG ) && m_TabSkyCtrl )
			m_TabSkyCtrl.SetFogValues( preset, actual );

		if ( !IsSectionDirty( SECTION_STORM ) && m_TabSkyCtrl )
			m_TabSkyCtrl.SetStormValues( preset );

		if ( !IsSectionDirty( SECTION_SANDSTORM ) && m_TabSkyCtrl )
			m_TabSkyCtrl.SetSandstormValues( preset );

		if ( !IsSectionDirty( SECTION_RAIN ) && m_TabPrecipitationCtrl )
			m_TabPrecipitationCtrl.SetRainValues( preset, actual );

		if ( !IsSectionDirty( SECTION_SNOW ) && m_TabPrecipitationCtrl )
			m_TabPrecipitationCtrl.SetSnowValues( preset, actual );

		if ( !IsSectionDirty( SECTION_WIND ) && m_TabWindCtrl )
			m_TabWindCtrl.SetWindValues( preset, actual );

		if ( !IsSectionDirty( SECTION_WIND_FUNCTION ) && m_TabWindCtrl )
			m_TabWindCtrl.SetWindFunctionValues( preset );
	}

	protected void ClearDirtySections()
	{
		m_DirtySectionsMask = 0;
		UpdateApplyButtonStates();
	}

	//! Public: JMWeatherFormTabOverview's Refresh button discards local edits.
	void ClearDirty()
	{
		ClearDirtySections();
	}

	//! Was called from SetSectionDirty()/ClearDirtySections() with no definition
	//! anywhere in the codebase - a pre-existing dangling reference found while
	//! splitting this form's tabs into their own classes, unrelated to that
	//! split. Left as a no-op placeholder rather than guessed at, since there is
	//! no existing per-section Apply-button visual state to restore.
	protected void UpdateApplyButtonStates()
	{
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	// -------------------------------------------------------------------------
	//  Construction
	// -------------------------------------------------------------------------

	override void OnCreate()
	{
		m_RightPanel        = layoutRoot.FindAnyWidget( "panel_right" );
		m_RightPanelDisable = layoutRoot.FindAnyWidget( "panel_right_disable" );
		m_RightTabStrip     = layoutRoot.FindAnyWidget( "panel_right_tabs" );
		m_RightContent      = layoutRoot.FindAnyWidget( "panel_right_content" );

		Widget tabOverview      = layoutRoot.FindAnyWidget( "tab_overview" );
		Widget tabSky           = layoutRoot.FindAnyWidget( "tab_sky" );
		Widget tabPrecipitation = layoutRoot.FindAnyWidget( "tab_precipitation" );
		Widget tabWind          = layoutRoot.FindAnyWidget( "tab_wind" );
		Widget tabTime          = layoutRoot.FindAnyWidget( "tab_time" );
		Widget tabPresets       = layoutRoot.FindAnyWidget( "tab_presets" );

		m_Banner = new JMWeatherModeBanner( this );
		m_Banner.Build();


		m_Tabs = UIActionManager.CreateTabStrip( m_RightTabStrip, this, "OnChange_Tab" );

		m_TabIdOverview = m_Tabs.AddTab( "#STR_COT_WEATHER_TAB_OVERVIEW", JMConstants.Lucide( "gauge" ), tabOverview );
		m_TabIdSky = m_Tabs.AddTab( "#STR_COT_WEATHER_TAB_SKY", JMConstants.Lucide( "cloud-sun" ), tabSky );
		m_TabIdPrecipitation = m_Tabs.AddTab( "#STR_COT_WEATHER_TAB_PRECIPITATION", JMConstants.Lucide( "cloud-rain" ), tabPrecipitation );
		m_TabIdWind = m_Tabs.AddTab( "#STR_COT_WEATHER_MODULE_WIND", JMConstants.Lucide( "wind" ), tabWind );
		m_TabIdTime = m_Tabs.AddTab( "#STR_COT_WEATHER_TAB_TIME", JMConstants.Lucide( "calendar-clock" ), tabTime );
		m_TabIdPresets = m_Tabs.AddTab( "#STR_COT_WEATHER_TAB_DYNAMIC", JMConstants.Lucide( "workflow" ), tabPresets );

		DeclareTabs( m_TabIdPresets + 1 );

		m_Tabs.SetSelection( m_TabIdOverview, false );

		InitTabFocus( m_TabIdOverview );
	}

	//! Every tab is a JMFormTab class, registered against the index reserved for it.
	protected void OnTabCreateController( int tab, Widget panel )
	{
		if ( tab == m_TabIdOverview )
		{
			m_TabOverviewCtrl = new JMWeatherFormTabOverview( this );
			RegisterTab( m_TabIdOverview, m_TabOverviewCtrl );
			m_TabOverviewCtrl.OnCreate( panel );
		}
		else if ( tab == m_TabIdSky )
		{
			m_TabSkyCtrl = new JMWeatherFormTabSky( this );
			RegisterTab( m_TabIdSky, m_TabSkyCtrl );
			m_TabSkyCtrl.OnCreate( panel );
		}
		else if ( tab == m_TabIdPrecipitation )
		{
			m_TabPrecipitationCtrl = new JMWeatherFormTabPrecipitation( this );
			RegisterTab( m_TabIdPrecipitation, m_TabPrecipitationCtrl );
			m_TabPrecipitationCtrl.OnCreate( panel );
		}
		else if ( tab == m_TabIdWind )
		{
			m_TabWindCtrl = new JMWeatherFormTabWind( this );
			RegisterTab( m_TabIdWind, m_TabWindCtrl );
			m_TabWindCtrl.OnCreate( panel );
		}
		else if ( tab == m_TabIdTime )
		{
			m_TabTimeCtrl = new JMWeatherFormTabTime( this );
			RegisterTab( m_TabIdTime, m_TabTimeCtrl );
			m_TabTimeCtrl.OnCreate( panel );
		}
		else if ( tab == m_TabIdPresets )
		{
			m_TabPresetsCtrl = new JMWeatherFormTabPresets( this );
			RegisterTab( m_TabIdPresets, m_TabPresetsCtrl );
			m_TabPresetsCtrl.OnCreate( panel );
		}
	}

	override protected void OnTabCreate( int tab, Widget panel )
	{
		OnTabCreateController( tab, panel );

		//! A tab built after the form was already open has missed every refresh
		//! that ran before it existed - and in preset mode it has to come up
		//! showing the preset, not the world.
		LoadEditorValues();
		UpdateModeBanner();
		UpdateStates();
	}

	protected override COT_ScriptedWidgetEventHandler GetTabStrip()
	{
		return m_Tabs;
	}

	void OnChange_Tab( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CHANGE )
			HandleTabChange();
	}

	override protected void OnTabUpdate( int tab )
	{
		super.OnTabUpdate( tab );

		UpdateStates();
	}

	// -------------------------------------------------------------------------
	//  Shared builders
	// -------------------------------------------------------------------------

	//! Every 0-1 phenomenon is edited as a whole percentage. The engine wants the
	//! fraction, so the control converts (scale 100). `instance` is the tab class
	//! that owns the CHANGE callback.
	JMWeatherRangeControl CreatePercentRange( Widget parent, string label, string callback, Class instance )
	{
		return JMWeatherRangeControl.Create( parent, label, 0, 100, 1, "#STR_COT_FORMAT_PERCENTAGE", 100.0, 0.0, instance, callback );
	}

	// -------------------------------------------------------------------------
	//  Lifecycle
	// -------------------------------------------------------------------------

	override void OnShow()
	{
		super.OnShow();

		if ( m_Module )
		{
			if ( m_Module.IsLoaded() )
				OnSettingsUpdated();
			else
				m_Module.Load();
		}

		LoadEditorValues();
		UpdateModeBanner();
		UpdateStates();
	}

	override void OnResize( float w, float h )
	{
		super.OnResize( w, h );

		//! The mode banner sits between the tab strip and the content; the base pins the three together.
		Widget bannerWidget;
		if ( m_Banner )
			bannerWidget = m_Banner.GetWidget();

		PinRightPanelGeometry( h, bannerWidget, BANNER_HEIGHT );

		ResizeTabs( w, h );
	}

	override void OnSettingsUpdated()
	{
		if ( !m_Module || !m_Module.IsLoaded() )
			return;

		//! Rebuilds the dynamic controls as well. Before the Presets tab has been
		//! opened there is nothing to refresh - it fills itself when it is built.
		if ( m_TabPresetsCtrl )
			m_TabPresetsCtrl.UpdatePresetList();
	}

	//! Ask the server where the dynamic weather machine is, and repaint the
	//! status card. Runs whatever tab is showing: the Overview and the Presets
	//! tab both read the answer.
	protected void PollDynamicStatus()
	{
		if ( !m_Module || !m_Module.IsLoaded() )
			return;

		float now = g_Game.GetTickTime();

		if ( now - m_LastDynamicPoll >= DYNAMIC_POLL_INTERVAL )
		{
			m_LastDynamicPoll = now;
			m_Module.RequestDynamicStatus();
		}

		if ( m_TabPresetsCtrl )
			m_TabPresetsCtrl.Poll();
	}

	override void Update()
	{
		if ( m_TabOverviewCtrl )
			m_TabOverviewCtrl.Poll();

		PollDynamicStatus();

		//! Auto-refresh pulls the LIVE WORLD into the editing controls several
		//! times a second. In preset mode those controls hold the preset, so
		//! the poll would wipe out the edit as fast as it was made.
		if ( IsPresetMode() )
			return;

		if ( m_TabOverviewCtrl && m_TabOverviewCtrl.IsAutoRefreshEnabled() )
			RefreshValues( true );
	}

	// -------------------------------------------------------------------------
	//  Reading the world into the controls
	// -------------------------------------------------------------------------

	void RefreshValues( bool actual = false )
	{
		JMWeatherPreset preset = new JMWeatherPreset;
		preset.SetFromWorld();

		SetUIActionValues( preset, actual );
	}

	//! Public: read by every per-tab Apply*()/ReadInto(), which is why these
	//! stay on the form as forwards even though the pickers themselves are
	//! Overview-owned.
	float GlobalTransition()
	{
		if ( m_TabOverviewCtrl )
			return m_TabOverviewCtrl.GetTransition();

		return DEFAULT_TRANSITION;
	}

	float GlobalDuration()
	{
		if ( m_TabOverviewCtrl )
			return m_TabOverviewCtrl.GetDuration();

		return DEFAULT_DURATION;
	}

	// -------------------------------------------------------------------------
	//  Apply - per-section Apply buttons/logic live in their owning tab class
	//  (JMWeatherFormTabSky/Precipitation/Wind/Time.c) and Apply All in
	//  JMWeatherFormTabOverview. What they share - the preview switch below -
	//  stays here.
	// -------------------------------------------------------------------------

	//! In preset mode Apply is a PREVIEW, and a local one.
	//!
	//! The values go straight into THIS CLIENT's weather instead of being sent
	//! to the server, so an admin can look at the preset they are authoring
	//! without changing the sky for everyone playing. Nothing is stored either
	//! way - Save Preset on the Presets tab is what commits it.
	//!
	//! What makes it local is simply not going through the module: every
	//! JMWeatherBase.Apply talks to g_Game.GetWeather() directly, while
	//! m_Module.SetX RPCs to the server when called from a client.
	//!
	//! The server owns the weather and syncs it, so a preview lasts until the
	//! next sync rather than forever. That is the right lifetime for a preview,
	//! but it does mean a long fade may be overwritten part way through.
	bool PreviewSection( int section )
	{
		if ( !IsPresetMode() )
			return false;

		JMWeatherPreset preset = new JMWeatherPreset;
		GetUIActionValues( preset );

		//! A preview answers "what does this look like" NOW, so it lands
		//! instantly instead of fading in over the preset's transition. The
		//! stored transition is untouched - this is a copy built from the
		//! controls, and it is what the preset applies with for real.
		StripPreviewFade( preset );

		if ( section == SECTION_OVERCAST )
		{
			preset.POvercast.Apply();
		}
		else if ( section == SECTION_FOG )
		{
			preset.PFog.Apply();
			preset.PDynFog.Apply();
		}
		else if ( section == SECTION_STORM )
		{
			preset.Storm.Apply();
		}
		else if ( section == SECTION_SANDSTORM )
		{
			preset.PSandstorm.Apply();
		}
		else if ( section == SECTION_RAIN )
		{
			preset.PRain.Apply();
			preset.RainThreshold.Apply();
		}
		else if ( section == SECTION_SNOW )
		{
			preset.PSnow.Apply();
			preset.SnowThreshold.Apply();
		}
		else if ( section == SECTION_WIND )
		{
			preset.PWindMagnitude.Apply();
			preset.PWindDirection.Apply();
		}
		else if ( section == SECTION_WIND_FUNCTION )
		{
			preset.WindFunc.Apply();
		}
		else if ( section == SECTION_TIME )
		{
			preset.PDate.Apply();
		}

		return true;
	}

	//! Zero every fade so a preview is immediate.
	protected void StripPreviewFade( JMWeatherPreset preset )
	{
		preset.POvercast.Time      = 0;
		preset.PFog.Time           = 0;
		preset.PDynFog.Time        = 0;
		preset.PRain.Time          = 0;
		preset.RainThreshold.Time  = 0;
		preset.PSnow.Time          = 0;
		preset.SnowThreshold.Time  = 0;
		preset.PWindMagnitude.Time = 0;
		preset.PWindDirection.Time = 0;
	}

	//! The stored preset the tabs are editing. NULL while authoring a new one -
	//! there is nothing saved to load, so the world stays as the starting
	//! point - and NULL when not in preset mode at all.
	protected JMWeatherPreset EditingPreset()
	{
		if ( !m_TabPresetsCtrl )
			return NULL;

		return m_TabPresetsCtrl.EditingPreset();
	}

	//! Put the right values in front of the admin: the selected preset's, or
	//! the live world's when no preset is selected.
	void LoadEditorValues()
	{
		JMWeatherPreset preset = EditingPreset();

		if ( !preset )
		{
			RefreshValues( true );
			return;
		}

		//! A stored value can be a span, and a control in single mode would collapse it
		//! - so every card is a min/max card before anything is loaded into it.
		SetRangeModeAll( true );

		//! Cleared first because the dirty flags mean "do not overwrite this
		//! from the world" - loading a preset IS the overwrite being asked for,
		//! and the controls match what is stored the moment it lands.
		ClearDirty();

		//! The world goes in FIRST, then the preset over the top of it.
		//!
		//! A preset only stores the fields it actually drives; everything else
		//! is -1, "leave this alone", and the setters skip those. Loading the
		//! preset on its own would therefore leave every field it does not
		//! drive showing whatever happened to be in the control - values from
		//! the last preset looked at, or from a world poll minutes ago. The
		//! world underneath gives those a real, current number, so every
		//! control shows what this preset would actually produce.
		RefreshValues( true );

		SetUIActionValues( preset );

		//! The stored fade and hold live on the phenomena rather than in a
		//! field of their own, so they are read back off one of them.
		if ( m_TabOverviewCtrl && preset.POvercast.Time != -1 )
			m_TabOverviewCtrl.SetTransition( preset.POvercast.Time );

		if ( m_TabOverviewCtrl && preset.POvercast.MinDuration != -1 )
			m_TabOverviewCtrl.SetDuration( preset.POvercast.MinDuration );
	}

	//! The banner's Save / Close are raw widgets, so their mouse events arrive here.
	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( m_Banner && m_Banner.OnClick( w, button ) )
			return true;

		return super.OnClick( w, x, y, button );
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( m_Banner )
			m_Banner.OnMouseEnter( w );

		return super.OnMouseEnter( w, x, y );
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( m_Banner )
			m_Banner.OnMouseLeave( w );

		return super.OnMouseLeave( w, enterW, x, y );
	}

	// -------------------------------------------------------------------------
	//  Presets - owned by JMWeatherFormTabPresets. What outside callers reach
	//  (JMWeatherPresetWidget, JMConfirmation) stays here as a thin forward.
	// -------------------------------------------------------------------------

	void ExitPresetMode()
	{
		if ( m_TabPresetsCtrl )
			m_TabPresetsCtrl.ExitPresetMode();
	}

	void SavePreset()
	{
		if ( m_TabPresetsCtrl )
			m_TabPresetsCtrl.SavePreset();
	}

	void CreateNew()
	{
		if ( m_TabPresetsCtrl )
			m_TabPresetsCtrl.CreateNew();
	}

	void RemovePreset( string preset )
	{
		if ( m_TabPresetsCtrl )
			m_TabPresetsCtrl.RemovePreset( preset );
	}

	//! JMConfirmation looks its callback up on the form, not on the tab that asked.
	void RemovePreset_Yes( JMConfirmation confirmation )
	{
		if ( m_TabPresetsCtrl )
			m_TabPresetsCtrl.RemovePresetConfirmed();
	}

	//! The rename popups' answers - same lookup rule as above.
	void RenamePreset_Confirm( JMConfirmation confirmation = NULL )
	{
		if ( confirmation && m_TabPresetsCtrl )
			m_TabPresetsCtrl.RenamePresetConfirmed( confirmation.GetEditBoxValue() );
	}

	void RenamePhase_Confirm( JMConfirmation confirmation = NULL )
	{
		if ( confirmation && m_TabPresetsCtrl )
			m_TabPresetsCtrl.RenamePhaseConfirmed( confirmation.GetEditBoxValue() );
	}

	// -------------------------------------------------------------------------
	//  Permissions / enable state
	// -------------------------------------------------------------------------

	//! Every control's enabled state is a function of permission, so a
	//! permission change has to re-run the same pass the value updates do -
	//! without this the tabs kept showing the old rights until something else
	//! happened to call UpdateStates().
	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();

		UpdateStates();
	}

	//! Public: every per-tab UpdateStates() calls this through the back-reference.
	void UpdateActionState( UIActionBase action, string permission, bool shouldDisable = false )
	{
		if ( !action )
			return;

		bool disable;
		if ( permission != "" )
			disable = !JMPermissions.Has( permission );

		if ( disable || shouldDisable )
			action.Disable();
		else
			action.Enable();
	}

	void UpdateStates()
	{
		if ( m_TabOverviewCtrl )      m_TabOverviewCtrl.UpdateStates();
		if ( m_TabTimeCtrl )          m_TabTimeCtrl.UpdateStates();
		if ( m_TabSkyCtrl )           m_TabSkyCtrl.UpdateStates();
		if ( m_TabPrecipitationCtrl ) m_TabPrecipitationCtrl.UpdateStates();
		if ( m_TabWindCtrl )          m_TabWindCtrl.UpdateStates();
		if ( m_TabPresetsCtrl )       m_TabPresetsCtrl.UpdateStates();

		if ( m_RightPanelDisable )
			m_RightPanelDisable.Show( false );
	}

	//! Public: the Presets tab repaints the banner whenever its selection changes.
	void UpdateModeBanner()
	{
		if ( m_Banner )
			m_Banner.Update();

		UpdateTimeTabState();
	}

	//! A preset holds weather, not a date, so the Time tab is switched off while one
	//! is being edited - and an admin sitting on it is moved to the Overview rather
	//! than left looking at controls that no longer do anything.
	protected void UpdateTimeTabState()
	{
		if ( !m_Tabs )
			return;

		bool presetMode = IsPresetMode();

		m_Tabs.SetTabEnabled( m_TabIdTime, !presetMode );

		if ( presetMode && m_Tabs.GetSelection() == m_TabIdTime )
			m_Tabs.SetSelection( m_TabIdOverview );
	}

	//! Every weather card between one value and a min/max span at once. On while a
	//! preset is edited, where a span is the normal thing to want; off for the live
	//! world. Only the tabs that have been opened exist yet - a tab built later
	//! catches up in LoadEditorValues.
	void SetRangeModeAll( bool on )
	{
		if ( m_TabSkyCtrl )           m_TabSkyCtrl.SetRangeMode( on );
		if ( m_TabPrecipitationCtrl ) m_TabPrecipitationCtrl.SetRangeMode( on );
		if ( m_TabWindCtrl )          m_TabWindCtrl.SetRangeMode( on );
	}

	//! UpdateActionState for a JMWeatherRangeControl, which wraps the action.
	void UpdateRangeState( JMWeatherRangeControl control, string permission, bool shouldDisable = false )
	{
		if ( control )
			UpdateActionState( control.GetAction(), permission, shouldDisable );
	}
}
