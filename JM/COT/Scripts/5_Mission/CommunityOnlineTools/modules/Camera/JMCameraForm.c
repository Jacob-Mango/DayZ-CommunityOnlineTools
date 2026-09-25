class JMCameraForm: JMFormBase
{
	protected UIActionScroller m_sclr_MainActions;
	protected UIActionSelectBox m_SelectBox;
	protected ref array< string > m_SelectBoxText =
	{
		"#STR_COT_CAMERA_TAB_EFFECTS",
		"#STR_COT_CAMERA_TAB_TRAVELING",
		"#STR_COT_CAMERA_TAB_BOOKMARKS",
		"Settings"
	};

	// ---- Effects panel ----
	protected GridSpacerWidget m_PanelEffects;
		protected UIActionSlider m_SliderBlurStrength;
		protected UIActionSlider m_SliderFocusDistance;
		protected UIActionSlider m_SliderFocalLength;
		protected UIActionSlider m_SliderFocalNear;
		protected UIActionSlider m_SliderExposure;
		protected UIActionSlider m_SliderVignette;
		protected UIActionSlider m_SliderSpeed;
		protected UIActionSlider m_SliderFOV;
		protected UIActionSlider m_SliderShakeIntensity;
		protected UIActionSlider m_SliderShakeFrequency;

	// ---- Settings panel ----
	protected GridSpacerWidget m_PanelSettings;
		protected UIActionCheckbox m_EnableFullmapCamera;
		protected UIActionCheckbox m_1stPersonADS_HideScope;
		protected UIActionCheckbox m_HideGrass;

	// ---- Bookmarks panel ----
	protected GridSpacerWidget m_PanelBookmarks;
		protected UIActionSelectBox      m_BookmarkSelectBox;
		protected ref TStringArray       m_BookmarkNames;
		protected string                 m_PendingBookmarkName;
		protected UIActionConfirmInline  m_DeleteBookmarkBtn;

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMCameraModule m_Module;

	//! The Traveling panel - waypoints, playback and saved paths.
	protected ref JMCameraTravelPanel m_Travel;

	void JMCameraForm()
	{
		m_Travel = new JMCameraTravelPanel( this );

		m_BookmarkNames = new TStringArray;
	}

	bool GetCurrentCamera1stPersonADSHideScope()
	{
		if (CurrentActiveCamera)
			return CurrentActiveCamera.m_JM_1stPersonADS_HideScope;
		return false;
	}

	// ----------------------------------------------------------------
	//  Shared helpers
	// ----------------------------------------------------------------

	//! The module this form drives. Public for the panels split out of this form.
	JMCameraModule GetModule()
	{
		return m_Module;
	}

	string GetSelectedBookmarkName()
	{
		int idx = m_BookmarkSelectBox.GetSelection();
		if ( idx >= 0 && m_BookmarkNames && m_BookmarkNames.Count() > idx )
			return m_BookmarkNames[idx];
		return "";
	}

	void SetEnableFullmapCamera(bool enable)
	{
		m_EnableFullmapCamera.SetChecked(enable);
	}

	override void OnHide()
	{
		m_Travel.SaveToModule();

		if (m_EnableFullmapCamera)
			m_Module.m_EnableFullmapCamera = m_EnableFullmapCamera.IsChecked();
		if (m_HideGrass)
			m_Module.m_HideGrass = m_HideGrass.IsChecked();
		if (m_SliderBlurStrength)
			m_Module.m_BlurStrength = m_SliderBlurStrength.GetCurrent();
		if (m_SliderFocusDistance)
			m_Module.m_FocusDistance = m_SliderFocusDistance.GetCurrent();
		if (m_SliderFocalLength)
			m_Module.m_FocalLength = m_SliderFocalLength.GetCurrent();
		if (m_SliderFocalNear)
			m_Module.m_FocalNear = m_SliderFocalNear.GetCurrent();
		if (m_SliderExposure)
			m_Module.m_Exposure = m_SliderExposure.GetCurrent();
		if (m_SliderVignette)
			m_Module.m_Vignette = m_SliderVignette.GetCurrent();

		super.OnHide();
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnCreate()
	{
		m_sclr_MainActions = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
		Widget actions = m_sclr_MainActions.GetContentWidget();

		m_SelectBox = UIActionManager.CreateSelectionBox( actions, "", m_SelectBoxText, this, "OnClick_SelectBox" );
		m_SelectBox.SetSelectorWidth(1.0);
		m_SelectBox.SetSelection(0, false);

		m_PanelEffects = UIActionManager.CreateGridSpacer( actions, 1, 1 );
			InitCameraEffects();

		m_Travel.Build( actions );

		m_PanelBookmarks = UIActionManager.CreateGridSpacer( actions, 1, 1 );
			InitCameraBookmarks();

		m_PanelSettings = UIActionManager.CreateGridSpacer( actions, 1, 1 );
			InitCameraSettings();

		if ( m_Module )
		{
			m_SliderBlurStrength.SetCurrent(m_Module.m_BlurStrength);
			m_SliderFocusDistance.SetCurrent(m_Module.m_FocusDistance);
			m_SliderFocalLength.SetCurrent(m_Module.m_FocalLength);
			m_SliderFocalNear.SetCurrent(m_Module.m_FocalNear);
			m_SliderExposure.SetCurrent(m_Module.m_Exposure);
			m_SliderVignette.SetCurrent(m_Module.m_Vignette);
			m_SliderShakeIntensity.SetCurrent(m_Module.m_ShakeIntensity);
			m_SliderShakeFrequency.SetCurrent(m_Module.m_ShakeFrequency);
			m_EnableFullmapCamera.SetChecked(m_Module.m_EnableFullmapCamera);
			m_HideGrass.SetChecked(m_Module.m_HideGrass);

			m_Travel.LoadFromModule();
			RefreshBookmarkSelectBox();
		}

		Exec_SelectBox();
		m_Travel.UpdateUIWaypoint();
		m_Travel.UpdateDurationLabel();
		m_sclr_MainActions.UpdateScroller();
	}

	// ----------------------------------------------------------------
	//  Panel init helpers
	// ----------------------------------------------------------------

	void InitCameraEffects()
	{
		Widget col = UIActionManager.CreateGridSpacer( m_PanelEffects, 1, 1 );

		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_DOF", "" );
		UIActionManager.CreateDivider( col, JMTheme.DIVIDER_DARK, 2 );

		m_SliderBlurStrength = UIActionManager.CreateSyncedSlider( col, "#STR_COT_CAMERA_MODULE_BLUR", 0, 100, this, "OnChange_Blur" );
		m_SliderBlurStrength.SetCurrent( 0 );
		m_SliderBlurStrength.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderBlurStrength.SetStepValue( 0.1 );
		m_SliderBlurStrength.SetWidth( 1.0 );
		m_SliderBlurStrength.SetWidgetWidth( m_SliderBlurStrength.GetLabelWidget(), 0.4 );
		m_SliderBlurStrength.SetWidgetWidth( m_SliderBlurStrength.GetSliderWidget(), 0.6 );

		m_SliderFocusDistance = UIActionManager.CreateSyncedSlider( col, "#STR_COT_CAMERA_MODULE_FOCUS", 0, 1000, this, "OnChange_Focus" );
		m_SliderFocusDistance.SetCurrent( 0 );
		m_SliderFocusDistance.SetFormat( "#STR_COT_FORMAT_METRE" );
		m_SliderFocusDistance.SetStepValue( 0.1 );
		m_SliderFocusDistance.SetWidth( 1.0 );
		m_SliderFocusDistance.SetWidgetWidth( m_SliderFocusDistance.GetLabelWidget(), 0.4 );
		m_SliderFocusDistance.SetWidgetWidth( m_SliderFocusDistance.GetSliderWidget(), 0.6 );

		m_SliderFocalLength = UIActionManager.CreateSyncedSlider( col, "#STR_COT_CAMERA_MODULE_FOCAL_LENGTH", 0, 1000, this, "OnChange_FocalLength" );
		m_SliderFocalLength.SetCurrent( 0 );
		m_SliderFocalLength.SetFormat( "#STR_COT_FORMAT_METRE" );
		m_SliderFocalLength.SetStepValue( 0.1 );
		m_SliderFocalLength.SetWidth( 1.0 );
		m_SliderFocalLength.SetWidgetWidth( m_SliderFocalLength.GetLabelWidget(), 0.4 );
		m_SliderFocalLength.SetWidgetWidth( m_SliderFocalLength.GetSliderWidget(), 0.6 );

		m_SliderFocalNear = UIActionManager.CreateSyncedSlider( col, "#STR_COT_CAMERA_MODULE_FOCAL_NEAR", 0, 1000, this, "OnChange_FocalNear" );
		m_SliderFocalNear.SetCurrent( 0 );
		m_SliderFocalNear.SetFormat( "#STR_COT_FORMAT_METRE" );
		m_SliderFocalNear.SetStepValue( 0.1 );
		m_SliderFocalNear.SetWidth( 1.0 );
		m_SliderFocalNear.SetWidgetWidth( m_SliderFocalNear.GetLabelWidget(), 0.4 );
		m_SliderFocalNear.SetWidgetWidth( m_SliderFocalNear.GetSliderWidget(), 0.6 );

		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_POSTPROCESS", "" );
		UIActionManager.CreateDivider( col, JMTheme.DIVIDER_DARK, 2 );

		m_SliderExposure = UIActionManager.CreateSyncedSlider( col, "#STR_COT_CAMERA_MODULE_EXPOSURE", -5, 5, this, "OnChange_Exposure" );
		m_SliderExposure.SetCurrent( 0 );
		m_SliderExposure.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_SliderExposure.SetStepValue( 0.05 );
		m_SliderExposure.SetWidth( 1.0 );
		m_SliderExposure.SetWidgetWidth( m_SliderExposure.GetLabelWidget(), 0.4 );
		m_SliderExposure.SetWidgetWidth( m_SliderExposure.GetSliderWidget(), 0.6 );

		m_SliderVignette = UIActionManager.CreateSyncedSlider( col, "#STR_COT_CAMERA_MODULE_VIGNETTE", 0, 1, this, "OnChange_Vignette" );
		m_SliderVignette.SetCurrent( 0 );
		m_SliderVignette.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderVignette.SetStepValue( 0.01 );
		m_SliderVignette.SetWidth( 1.0 );
		m_SliderVignette.SetWidgetWidth( m_SliderVignette.GetLabelWidget(), 0.4 );
		m_SliderVignette.SetWidgetWidth( m_SliderVignette.GetSliderWidget(), 0.6 );

		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_CONTROLS", "" );
		UIActionManager.CreateDivider( col, JMTheme.DIVIDER_DARK, 2 );

		m_SliderSpeed = UIActionManager.CreateSyncedSlider( col, "#STR_COT_CAMERA_MODULE_SPEED", 0.001, 10, this, "OnChange_Speed" );
		m_SliderSpeed.SetCurrent( JMCameraBase.s_CurrentSpeed );
		m_SliderSpeed.SetStepValue( 0.001 );
		m_SliderSpeed.SetWidth( 1.0 );
		m_SliderSpeed.SetWidgetWidth( m_SliderSpeed.GetLabelWidget(), 0.4 );
		m_SliderSpeed.SetWidgetWidth( m_SliderSpeed.GetSliderWidget(), 0.6 );

		m_SliderFOV = UIActionManager.CreateSyncedSlider( col, "#STR_COT_CAMERA_MODULE_FOV", 0.001, 4, this, "OnChange_FOV" );
		m_SliderFOV.SetCurrent( m_Module.m_CurrentFOV );
		m_SliderFOV.SetStepValue( 0.001 );
		m_SliderFOV.SetWidth( 1.0 );
		m_SliderFOV.SetWidgetWidth( m_SliderFOV.GetLabelWidget(), 0.4 );
		m_SliderFOV.SetWidgetWidth( m_SliderFOV.GetSliderWidget(), 0.6 );

		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_SHAKE", "" );
		UIActionManager.CreateDivider( col, JMTheme.DIVIDER_DARK, 2 );

		m_SliderShakeIntensity = UIActionManager.CreateSyncedSlider( col, "#STR_COT_CAMERA_MODULE_SHAKE_INTENSITY", 0, 0.5, this, "OnChange_ShakeIntensity" );
		m_SliderShakeIntensity.SetCurrent( 0 );
		m_SliderShakeIntensity.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_SliderShakeIntensity.SetStepValue( 0.005 );
		m_SliderShakeIntensity.SetWidth( 1.0 );
		m_SliderShakeIntensity.SetWidgetWidth( m_SliderShakeIntensity.GetLabelWidget(), 0.4 );
		m_SliderShakeIntensity.SetWidgetWidth( m_SliderShakeIntensity.GetSliderWidget(), 0.6 );

		m_SliderShakeFrequency = UIActionManager.CreateSyncedSlider( col, "#STR_COT_CAMERA_MODULE_SHAKE_FREQUENCY", 0.1, 10, this, "OnChange_ShakeFrequency" );
		m_SliderShakeFrequency.SetCurrent( 1.0 );
		m_SliderShakeFrequency.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_SliderShakeFrequency.SetStepValue( 0.1 );
		m_SliderShakeFrequency.SetWidth( 1.0 );
		m_SliderShakeFrequency.SetWidgetWidth( m_SliderShakeFrequency.GetLabelWidget(), 0.4 );
		m_SliderShakeFrequency.SetWidgetWidth( m_SliderShakeFrequency.GetSliderWidget(), 0.6 );

		UIActionButton btnResetFx = UIActionManager.CreateButton( col, "#STR_COT_CAMERA_RESET", this, "" );
		if ( btnResetFx ) btnResetFx.SetOnClick( this, "OnClick_ResetEffects" );
		btnResetFx.SetTooltip( "#STR_COT_CAMERA_RESET_EVERY_SCREEN_EFFECT_BACK_TO" );
	}

	void InitCameraSettings()
	{
		Widget col = UIActionManager.CreateGridSpacer( m_PanelSettings, 1, 1 );

		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_OPTIONS", "" );
		UIActionManager.CreateDivider( col, JMTheme.DIVIDER_DARK, 2 );

		m_EnableFullmapCamera    = UIActionManager.CreateCheckbox( col, "#STR_COT_CAMERA_MODULE_FULLMAP_UPDATE",  this, "OnClick_EnableFullmap",         m_Module.m_EnableFullmapCamera );
		m_EnableFullmapCamera.SetTooltip( "#STR_COT_CAMERA_UPDATE_THE_FULLSCREEN_MAP_TO_FOLLOW" );
		m_1stPersonADS_HideScope = UIActionManager.CreateCheckbox( col, "#STR_COT_CAMERA_MODULE_HIDE_SCOPE",     this, "OnClick_1stPersonADS_HideScope", GetCurrentCamera1stPersonADSHideScope() );
		m_1stPersonADS_HideScope.SetTooltip( "#STR_COT_CAMERA_HIDE_THE_SCOPE_RETICLE_OVERLAY_WHILE" );
		m_HideGrass              = UIActionManager.CreateCheckbox( col, "#STR_COT_CAMERA_MODULE_HIDE_GRASS",     this, "OnClick_HideGrass",              m_Module.m_HideGrass );
		m_HideGrass.SetTooltip( "#STR_COT_CAMERA_DISABLE_GRASS_RENDERING_SO_DISTANT_OBJEC" );
	}

	void InitCameraBookmarks()
	{
		Widget col = UIActionManager.CreateGridSpacer( m_PanelBookmarks, 1, 1 );

		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_BOOKMARKS", "" );
		UIActionManager.CreateDivider( col, JMTheme.DIVIDER_DARK, 2 );

		m_BookmarkNames     = m_Module.GetBookmarkNames();
		m_BookmarkSelectBox = UIActionManager.CreateSelectionBox( col, "#STR_COT_CAMERA_MODULE_BOOKMARKS", m_BookmarkNames, this, "OnClick_BookmarkSelectBox" );
		m_BookmarkSelectBox.SetSelectorWidth(1.0);

		Widget gridBookmarkActions = UIActionManager.CreateGridSpacer( col, 1, 3 );
		UIActionButton btnSaveBm = UIActionManager.CreateButton( gridBookmarkActions, "#STR_COT_CAMERA_SAVE",   this, ""    );
		if ( btnSaveBm ) btnSaveBm.SetOnClick( this, "OnClick_SaveBookmark" );
		btnSaveBm.SetTooltip( "#STR_COT_CAMERA_SAVE_THE_CURRENT_CAMERA_POSITION_AS" );
		UIActionButton btnGoToBm = UIActionManager.CreateButton( gridBookmarkActions, "#STR_COT_CAMERA_GO_TO",  this, "" );
		if ( btnGoToBm ) btnGoToBm.SetOnClick( this, "OnClick_TeleportBookmark" );
		btnGoToBm.SetTooltip( "#STR_COT_CAMERA_TELEPORT_THE_CAMERA_TO_THE_SELECTED" );
		m_DeleteBookmarkBtn = UIActionManager.CreateConfirmInline( gridBookmarkActions, "#STR_COT_GENERIC_DELETE", this, "OnClick_DeleteBookmark" );
		UIActionIconGrid.ApplyDeletePreset( m_DeleteBookmarkBtn );
		m_DeleteBookmarkBtn.SetTooltip( "#STR_COT_CAMERA_REMOVE_THE_SELECTED_BOOKMARK" );
	}

	// ----------------------------------------------------------------
	//  Update tick
	// ----------------------------------------------------------------

	override void OnResize( float w, float h )
	{
		if ( m_sclr_MainActions )
			m_sclr_MainActions.UpdateScroller();
	}

	override void Update()
	{
		OnSliderUpdate();
	}

	void OnSliderUpdate()
	{
		if ( m_Module.m_Blur == 0 )
		{
			m_Module.m_DOF = false;
			PPEffects.ResetDOFOverride();
		}
		else
			m_Module.m_DOF = true;

		if ( m_Module.m_FDist == 0 )
			m_Module.m_AutoFocus = true;
		else
			m_Module.m_AutoFocus = false;

		m_SliderSpeed.SetCurrent(JMCameraBase.s_CurrentSpeed);
		m_SliderFOV.SetCurrent(m_Module.m_CurrentFOV);
	}

	// ----------------------------------------------------------------
	//  Tab selection
	// ----------------------------------------------------------------

	void OnClick_SelectBox( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		Exec_SelectBox();
	}

	void Exec_SelectBox()
	{
		int id = m_SelectBox.GetSelection();
		m_PanelEffects.Show(   id == 0 );
		m_Travel.Show( id == 1 );
		m_PanelBookmarks.Show( id == 2 );
		m_PanelSettings.Show(  id == 3 );
	}

	// ----------------------------------------------------------------
	//  Effects panel handlers
	// ----------------------------------------------------------------

	void OnChange_Blur( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		m_Module.m_Blur = action.GetCurrent();
		OnSliderUpdate();
	}

	void OnChange_Focus( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		m_Module.m_FDist = action.GetCurrent();
		OnSliderUpdate();
	}

	void OnChange_FocalLength( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		m_Module.m_Flength = action.GetCurrent();
		OnSliderUpdate();
	}

	void OnChange_FocalNear( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		m_Module.m_FNear = action.GetCurrent();
		OnSliderUpdate();
	}

	void OnChange_Exposure( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		m_Module.m_Exposure = action.GetCurrent();
		OnSliderUpdate();
	}

	void OnChange_Vignette( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		m_Module.m_Vignette = action.GetCurrent();
		OnSliderUpdate();
	}

	void OnChange_Speed( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		JMCameraBase.s_CurrentSpeed = action.GetCurrent();
		OnSliderUpdate();
	}

	void OnChange_FOV( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		m_Module.m_TargetFOV = action.GetCurrent();
		OnSliderUpdate();
	}

	void OnChange_ShakeIntensity( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		m_Module.m_ShakeIntensity = action.GetCurrent();
	}

	void OnChange_ShakeFrequency( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		m_Module.m_ShakeFrequency = action.GetCurrent();
	}

	void OnClick_EnableFullmap( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		m_Module.m_EnableFullmapCamera = action.IsChecked();
	}

	void OnClick_1stPersonADS_HideScope( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		if (CurrentActiveCamera)
			CurrentActiveCamera.m_JM_1stPersonADS_HideScope = action.IsChecked();
	}

	void OnClick_HideGrass( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		m_Module.m_HideGrass = action.IsChecked();
	}

	void OnClick_ResetEffects( UIActionBase action )
	{
		m_SliderBlurStrength.SetCurrent( 1 );
		m_SliderFocusDistance.SetCurrent( 0 );
		m_SliderFocalLength.SetCurrent( 0 );
		m_SliderFocalNear.SetCurrent( 0 );
		m_SliderExposure.SetCurrent( 0 );
		m_SliderVignette.SetCurrent( 0 );
		m_SliderShakeIntensity.SetCurrent( 0 );
		m_SliderShakeFrequency.SetCurrent( 1.0 );

		m_Module.m_Blur           = 1;
		m_Module.m_FDist          = 0;
		m_Module.m_Flength        = 0;
		m_Module.m_FNear          = 0;
		m_Module.m_Exposure       = 0;
		m_Module.m_Vignette       = 0;
		m_Module.m_BlurStrength   = 1;
		m_Module.m_FocusDistance  = 0;
		m_Module.m_FocalLength    = 0;
		m_Module.m_FocalNear      = 0;
		m_Module.m_ShakeIntensity = 0;
		m_Module.m_ShakeFrequency = 1.0;

		g_Game.SetEVValue( 0 );
		PPEffects.SetVignette( 0, 0, 0, 0, 0 );
		PPEffects.ResetDOFOverride();
		OnSliderUpdate();
	}

	// ----------------------------------------------------------------
	//  Bookmarks panel handlers
	// ----------------------------------------------------------------

	void RefreshBookmarkSelectBox()
	{
		if ( !m_BookmarkSelectBox )
		{
			Error("[JMCameraForm] RefreshBookmarkSelectBox failed: m_BookmarkSelectBox is null!");
			return;
		}
		m_BookmarkNames = m_Module.GetBookmarkNames();
		// Belt + suspenders: even though UIActionSelectBox.SetSelections seeds
		// an empty array with "(empty)", guard here too so the call is
		// self-documenting and survives if SetSelections' guard changes.
		if ( m_BookmarkNames.Count() == 0 )
			m_BookmarkNames.Insert( "(no bookmarks)" );
		m_BookmarkSelectBox.SetSelections( m_BookmarkNames );
	}

	void OnClick_BookmarkSelectBox( UIEvent eid, UIActionBase action )
	{
		// selection drives teleport/delete - no name field to populate
	}

	void OnClick_SaveBookmark( UIActionBase action )
	{
		PromptInput( "#STR_COT_CAMERA_MODULE_BOOKMARK_SAVE", "#STR_COT_CAMERA_MODULE_BOOKMARK_NAME", "OnClick_SaveBookmark_Confirm" );
	}

	void OnClick_SaveBookmark_Confirm( JMConfirmation confirmation )
	{
		string name = confirmation.GetEditBoxValue();
		if ( name == "" ) return;
		vector pos;
		if ( CurrentActiveCamera )
			pos = CurrentActiveCamera.GetPosition();
		else
			pos = g_Game.GetCurrentCameraPosition();
		m_Module.SaveBookmark( name, pos );
		RefreshBookmarkSelectBox();
	}

	void OnClick_TeleportBookmark( UIActionBase action )
	{
		string name = GetSelectedBookmarkName();
		if ( name == "" ) return;
		m_Module.TeleportToBookmark( name );
	}

	void OnClick_DeleteBookmark( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		string name = GetSelectedBookmarkName();
		if ( name == "" ) return;
		m_Module.DeleteBookmark( name );
		RefreshBookmarkSelectBox();
	}
}
