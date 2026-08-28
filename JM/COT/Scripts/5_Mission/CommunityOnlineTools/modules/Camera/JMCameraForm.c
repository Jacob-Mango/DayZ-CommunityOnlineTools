class JMCameraForm: JMFormBase
{
	private UIActionScroller m_sclr_MainActions;

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
		private UIActionSlider m_SliderBlurStrength;
		private UIActionSlider m_SliderFocusDistance;
		private UIActionSlider m_SliderFocalLength;
		private UIActionSlider m_SliderFocalNear;
		private UIActionSlider m_SliderExposure;
		private UIActionSlider m_SliderVignette;
		private UIActionSlider m_SliderSpeed;
		private UIActionSlider m_SliderFOV;
		private UIActionSlider m_SliderShakeIntensity;
		private UIActionSlider m_SliderShakeFrequency;

	// ---- Settings panel ----
	protected GridSpacerWidget m_PanelSettings;
		private UIActionCheckbox m_EnableFullmapCamera;
		private UIActionCheckbox m_1stPersonADS_HideScope;
		private UIActionCheckbox m_HideGrass;

	// ---- Traveling panel ----
	protected GridSpacerWidget m_PanelTraveling;
		// Waypoint list
		private UIActionSelectBox      m_WaypointSelectBox;
		private ref TStringArray       m_WaypointNames;

		// Edit-selected-waypoint controls
		private UIActionSlider         m_SliderWaypointSpeed;
		private UIActionSlider         m_SliderHoldTime;
		private UIActionCheckbox       m_ToggleCatmull;
		private UIActionCheckbox       m_ToggleTrackTarget;
		private UIActionCheckbox       m_CaptureOrientation;
		private UIActionSelectBox      m_EasingSelectBox;
		private ref TStringArray       m_EasingNames;
		// Per-waypoint screen effects + shake
		private UIActionSlider         m_WP_SliderExposure;
		private UIActionSlider         m_WP_SliderVignette;
		private UIActionSlider         m_WP_SliderBlur;
		private UIActionSlider         m_WP_SliderFOV;
		private UIActionSlider         m_WP_SliderShakeIntensity;
		private UIActionSlider         m_WP_SliderShakeFrequency;

		// Playback buttons
		private UIActionButton         m_BtnTravel;
		private UIActionButton         m_BtnPauseResume;

		// Path controls
		private UIActionSelectBox      m_TravelModeSelectBox;
		private ref TStringArray       m_TravelModeNames;
		private UIActionSlider         m_SliderSpeedMult;
		private UIActionText           m_LabelDuration;

		// Path save/load
		private UIActionEditableText   m_PathName;
		private UIActionSelectBox      m_PathSelectBox;
		private ref TStringArray       m_PathNames;

		// Working waypoint list (shared with module)
		private int                              m_WaypointID;
		private ref array< ref JMCameraWaypoint > m_Waypoints;

	// ---- Bookmarks panel ----
	protected GridSpacerWidget m_PanelBookmarks;
		private UIActionSelectBox      m_BookmarkSelectBox;
		private ref TStringArray       m_BookmarkNames;
		private string                 m_PendingBookmarkName;
		private UIActionConfirmInline  m_DeleteBookmarkBtn;

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMCameraModule m_Module;

	void JMCameraForm()
	{
		m_Waypoints     = new array< ref JMCameraWaypoint >;
		m_PathNames     = new TStringArray;
		m_BookmarkNames = new TStringArray;
		m_WaypointNames = new TStringArray;
		m_EasingNames   = new TStringArray;
		m_TravelModeNames = new TStringArray;

		// Waypoint SelectBox is created during OnInit, before any waypoints
		// exist. OptionSelectorMultistate VME's on an empty options array,
		// so seed a placeholder and let UpdateWaypoints replace it once
		// real waypoints are loaded. (See also UIActionSelectBox.SetSelections
		// guard against empty arrays.)
		if ( m_WaypointNames.Count() == 0 )
			m_WaypointNames.Insert( "(none)" );

		m_EasingNames.Insert("Linear");
		m_EasingNames.Insert("Ease In");
		m_EasingNames.Insert("Ease Out");
		m_EasingNames.Insert("Ease In/Out");
		m_EasingNames.Insert("Smoother Step");

		m_TravelModeNames.Insert("Once");
		m_TravelModeNames.Insert("Loop");
		m_TravelModeNames.Insert("Ping-Pong");
	}

	override void OnHide()
	{
		if ( m_Waypoints.Count() > 0 )
			FlushWaypointEdits();

		m_Module.m_Waypoints = m_Waypoints;

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

	override void OnInit()
	{
		m_sclr_MainActions = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
		Widget actions = m_sclr_MainActions.GetContentWidget();

		m_SelectBox = UIActionManager.CreateSelectionBox( actions, "", m_SelectBoxText, this, "OnClick_SelectBox" );
		m_SelectBox.SetSelectorWidth(1.0);
		m_SelectBox.SetSelection(0, false);

		m_PanelEffects = UIActionManager.CreateGridSpacer( actions, 1, 1 );
			InitCameraEffects();

		m_PanelTraveling = UIActionManager.CreateGridSpacer( actions, 1, 1 );
			InitCameraTraveling();

		m_PanelBookmarks = UIActionManager.CreateGridSpacer( actions, 1, 1 );
			InitCameraBookmarks();

		m_PanelSettings = UIActionManager.CreateGridSpacer( actions, 1, 1 );
			InitCameraSettings();

		if ( m_Module )
		{
			m_Waypoints = m_Module.m_Waypoints;

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

			m_TravelModeSelectBox.SetSelection((int)m_Module.m_TravelMode, false);
			m_SliderSpeedMult.SetCurrent(m_Module.m_TravelSpeedMult);

			RefreshPathSelectBox();
			RefreshBookmarkSelectBox();
		}

		Exec_SelectBox();
		UpdateUIWaypoint();
		UpdateDurationLabel();
		m_sclr_MainActions.UpdateScroller();
	}

	// ----------------------------------------------------------------
	//  Panel init helpers
	// ----------------------------------------------------------------

	void InitCameraEffects()
	{
		Widget col = UIActionManager.CreateGridSpacer( m_PanelEffects, 1, 1 );

		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_DOF", "" );
		UIActionManager.CreatePanel( col, JMTheme.DIVIDER_DARK, 2 );

		m_SliderBlurStrength = UIActionManager.CreateSlider( col, "#STR_COT_CAMERA_MODULE_BLUR", 0, 100, this, "OnChange_Blur" );
		m_SliderBlurStrength.SetCurrent( 0 );
		m_SliderBlurStrength.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderBlurStrength.SetStepValue( 0.1 );
		m_SliderBlurStrength.SetWidth( 1.0 );
		m_SliderBlurStrength.SetWidgetWidth( m_SliderBlurStrength.GetLabelWidget(), 0.4 );
		m_SliderBlurStrength.SetWidgetWidth( m_SliderBlurStrength.GetSliderWidget(), 0.6 );

		m_SliderFocusDistance = UIActionManager.CreateSlider( col, "#STR_COT_CAMERA_MODULE_FOCUS", 0, 1000, this, "OnChange_Focus" );
		m_SliderFocusDistance.SetCurrent( 0 );
		m_SliderFocusDistance.SetFormat( "#STR_COT_FORMAT_METRE" );
		m_SliderFocusDistance.SetStepValue( 0.1 );
		m_SliderFocusDistance.SetWidth( 1.0 );
		m_SliderFocusDistance.SetWidgetWidth( m_SliderFocusDistance.GetLabelWidget(), 0.4 );
		m_SliderFocusDistance.SetWidgetWidth( m_SliderFocusDistance.GetSliderWidget(), 0.6 );

		m_SliderFocalLength = UIActionManager.CreateSlider( col, "#STR_COT_CAMERA_MODULE_FOCAL_LENGTH", 0, 1000, this, "OnChange_FocalLength" );
		m_SliderFocalLength.SetCurrent( 0 );
		m_SliderFocalLength.SetFormat( "#STR_COT_FORMAT_METRE" );
		m_SliderFocalLength.SetStepValue( 0.1 );
		m_SliderFocalLength.SetWidth( 1.0 );
		m_SliderFocalLength.SetWidgetWidth( m_SliderFocalLength.GetLabelWidget(), 0.4 );
		m_SliderFocalLength.SetWidgetWidth( m_SliderFocalLength.GetSliderWidget(), 0.6 );

		m_SliderFocalNear = UIActionManager.CreateSlider( col, "#STR_COT_CAMERA_MODULE_FOCAL_NEAR", 0, 1000, this, "OnChange_FocalNear" );
		m_SliderFocalNear.SetCurrent( 0 );
		m_SliderFocalNear.SetFormat( "#STR_COT_FORMAT_METRE" );
		m_SliderFocalNear.SetStepValue( 0.1 );
		m_SliderFocalNear.SetWidth( 1.0 );
		m_SliderFocalNear.SetWidgetWidth( m_SliderFocalNear.GetLabelWidget(), 0.4 );
		m_SliderFocalNear.SetWidgetWidth( m_SliderFocalNear.GetSliderWidget(), 0.6 );

		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_POSTPROCESS", "" );
		UIActionManager.CreatePanel( col, JMTheme.DIVIDER_DARK, 2 );

		m_SliderExposure = UIActionManager.CreateSlider( col, "#STR_COT_CAMERA_MODULE_EXPOSURE", -5, 5, this, "OnChange_Exposure" );
		m_SliderExposure.SetCurrent( 0 );
		m_SliderExposure.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_SliderExposure.SetStepValue( 0.05 );
		m_SliderExposure.SetWidth( 1.0 );
		m_SliderExposure.SetWidgetWidth( m_SliderExposure.GetLabelWidget(), 0.4 );
		m_SliderExposure.SetWidgetWidth( m_SliderExposure.GetSliderWidget(), 0.6 );

		m_SliderVignette = UIActionManager.CreateSlider( col, "#STR_COT_CAMERA_MODULE_VIGNETTE", 0, 1, this, "OnChange_Vignette" );
		m_SliderVignette.SetCurrent( 0 );
		m_SliderVignette.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderVignette.SetStepValue( 0.01 );
		m_SliderVignette.SetWidth( 1.0 );
		m_SliderVignette.SetWidgetWidth( m_SliderVignette.GetLabelWidget(), 0.4 );
		m_SliderVignette.SetWidgetWidth( m_SliderVignette.GetSliderWidget(), 0.6 );

		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_CONTROLS", "" );
		UIActionManager.CreatePanel( col, JMTheme.DIVIDER_DARK, 2 );

		m_SliderSpeed = UIActionManager.CreateSlider( col, "#STR_COT_CAMERA_MODULE_SPEED", 0.001, 10, this, "OnChange_Speed" );
		m_SliderSpeed.SetCurrent( JMCameraBase.s_CurrentSpeed );
		m_SliderSpeed.SetStepValue( 0.001 );
		m_SliderSpeed.SetWidth( 1.0 );
		m_SliderSpeed.SetWidgetWidth( m_SliderSpeed.GetLabelWidget(), 0.4 );
		m_SliderSpeed.SetWidgetWidth( m_SliderSpeed.GetSliderWidget(), 0.6 );

		m_SliderFOV = UIActionManager.CreateSlider( col, "#STR_COT_CAMERA_MODULE_FOV", 0.001, 4, this, "OnChange_FOV" );
		m_SliderFOV.SetCurrent( m_Module.m_CurrentFOV );
		m_SliderFOV.SetStepValue( 0.001 );
		m_SliderFOV.SetWidth( 1.0 );
		m_SliderFOV.SetWidgetWidth( m_SliderFOV.GetLabelWidget(), 0.4 );
		m_SliderFOV.SetWidgetWidth( m_SliderFOV.GetSliderWidget(), 0.6 );

		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_SHAKE", "" );
		UIActionManager.CreatePanel( col, JMTheme.DIVIDER_DARK, 2 );

		m_SliderShakeIntensity = UIActionManager.CreateSlider( col, "#STR_COT_CAMERA_MODULE_SHAKE_INTENSITY", 0, 0.5, this, "OnChange_ShakeIntensity" );
		m_SliderShakeIntensity.SetCurrent( 0 );
		m_SliderShakeIntensity.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_SliderShakeIntensity.SetStepValue( 0.005 );
		m_SliderShakeIntensity.SetWidth( 1.0 );
		m_SliderShakeIntensity.SetWidgetWidth( m_SliderShakeIntensity.GetLabelWidget(), 0.4 );
		m_SliderShakeIntensity.SetWidgetWidth( m_SliderShakeIntensity.GetSliderWidget(), 0.6 );

		m_SliderShakeFrequency = UIActionManager.CreateSlider( col, "#STR_COT_CAMERA_MODULE_SHAKE_FREQUENCY", 0.1, 10, this, "OnChange_ShakeFrequency" );
		m_SliderShakeFrequency.SetCurrent( 1.0 );
		m_SliderShakeFrequency.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_SliderShakeFrequency.SetStepValue( 0.1 );
		m_SliderShakeFrequency.SetWidth( 1.0 );
		m_SliderShakeFrequency.SetWidgetWidth( m_SliderShakeFrequency.GetLabelWidget(), 0.4 );
		m_SliderShakeFrequency.SetWidgetWidth( m_SliderShakeFrequency.GetSliderWidget(), 0.6 );

		UIActionButton btnResetFx = UIActionManager.CreateButton( col, "Reset", this, "OnClick_ResetEffects" );
		btnResetFx.SetTooltip( "Reset every screen effect back to its default" );
	}

	void InitCameraSettings()
	{
		Widget col = UIActionManager.CreateGridSpacer( m_PanelSettings, 1, 1 );

		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_OPTIONS", "" );
		UIActionManager.CreatePanel( col, JMTheme.DIVIDER_DARK, 2 );

		m_EnableFullmapCamera    = UIActionManager.CreateCheckbox( col, "#STR_COT_CAMERA_MODULE_FULLMAP_UPDATE",  this, "OnClick_EnableFullmap",         m_Module.m_EnableFullmapCamera );
		m_EnableFullmapCamera.SetTooltip( "Update the fullscreen map to follow the free camera position" );
		m_1stPersonADS_HideScope = UIActionManager.CreateCheckbox( col, "#STR_COT_CAMERA_MODULE_HIDE_SCOPE",     this, "OnClick_1stPersonADS_HideScope", GetCurrentCamera1stPersonADSHideScope() );
		m_1stPersonADS_HideScope.SetTooltip( "Hide the scope reticle overlay while aiming in first-person" );
		m_HideGrass              = UIActionManager.CreateCheckbox( col, "#STR_COT_CAMERA_MODULE_HIDE_GRASS",     this, "OnClick_HideGrass",              m_Module.m_HideGrass );
		m_HideGrass.SetTooltip( "Disable grass rendering so distant objects are visible" );
	}

	void InitCameraTraveling()
	{
		Widget col = UIActionManager.CreateGridSpacer( m_PanelTraveling, 1, 1 );

		// ---- Waypoint List ----
		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_WAYPOINT_EDITOR", "" );
		UIActionManager.CreatePanel( col, JMTheme.DIVIDER_DARK, 2 );

		m_WaypointSelectBox = UIActionManager.CreateSelectionBox( col, "Select", m_WaypointNames, this, "OnClick_WaypointSelectBox" );
		m_WaypointSelectBox.SetSelectorWidth(1.0);

		// Add / Remove / Move Up / Move Down in a 2x2 grid
		Widget gridListButtons = UIActionManager.CreateGridSpacer( col, 2, 2 );
		UIActionButton btnAddWp = UIActionManager.CreateButton( gridListButtons, "Add",    this, "OnClick_AddWaypoint"     );
		btnAddWp.SetTooltip( "Add a new waypoint at the current camera position" );
		UIActionConfirmInline delWaypointBtn = UIActionManager.CreateConfirmInline( gridListButtons, "Delete", this, "OnClick_DeleteWaypoint" );
		UIActionIconGrid.ApplyDeletePreset( delWaypointBtn );
		delWaypointBtn.SetTooltip( "Remove the selected waypoint" );
		UIActionButton btnUp   = UIActionManager.CreateButton( gridListButtons, "Up",             this, "OnClick_MoveWaypointUp"   );
		btnUp.SetTooltip( "Move the selected waypoint earlier in the path" );
		UIActionButton btnDown = UIActionManager.CreateButton( gridListButtons, "Down",           this, "OnClick_MoveWaypointDown" );
		btnDown.SetTooltip( "Move the selected waypoint later in the path" );

		UIActionButton btnClear = UIActionManager.CreateButton( col, "Clear All", this, "OnClick_ClearWaypoints" );
		btnClear.SetTooltip( "Remove every waypoint from the current path" );

		// ---- Edit Selected Waypoint ----
		UIActionManager.CreateText( col, "Edit Selected Waypoint", "" );
		UIActionManager.CreatePanel( col, JMTheme.DIVIDER_DARK, 2 );

		UIActionButton btnCapPos = UIActionManager.CreateButton( col, "Capture Position", this, "OnClick_CapturePosition" );
		btnCapPos.SetTooltip( "Update the selected waypoint with the current camera position" );

		m_SliderWaypointSpeed = UIActionManager.CreateSlider( col, "Speed (m/s)", 0.1, 50, this, "OnChange_WaypointSpeed" );
		m_SliderWaypointSpeed.SetCurrent( 5 );
		m_SliderWaypointSpeed.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_SliderWaypointSpeed.SetStepValue( 0.1 );
		m_SliderWaypointSpeed.SetWidth( 1.0 );
		m_SliderWaypointSpeed.SetWidgetWidth( m_SliderWaypointSpeed.GetLabelWidget(), 0.5 );
		m_SliderWaypointSpeed.SetWidgetWidth( m_SliderWaypointSpeed.GetSliderWidget(), 0.5 );

		m_SliderHoldTime = UIActionManager.CreateSlider( col, "Hold Time", 0, 10, this, "OnChange_HoldTime" );
		m_SliderHoldTime.SetCurrent( 0 );
		m_SliderHoldTime.SetFormat( "#STR_COT_FORMAT_SECOND" );
		m_SliderHoldTime.SetStepValue( 0.5 );
		m_SliderHoldTime.SetWidth( 1.0 );
		m_SliderHoldTime.SetWidgetWidth( m_SliderHoldTime.GetLabelWidget(), 0.5 );
		m_SliderHoldTime.SetWidgetWidth( m_SliderHoldTime.GetSliderWidget(), 0.5 );

		// Catmull + Track Target on the same row
		Widget gridCatmullTrack = UIActionManager.CreateGridSpacer( col, 1, 2 );
		m_ToggleCatmull     = UIActionManager.CreateCheckbox( gridCatmullTrack, "Catmull-Rom",  this );
		m_ToggleCatmull.SetTooltip( "Smooth the travel path with Catmull-Rom spline interpolation" );
		m_ToggleTrackTarget = UIActionManager.CreateCheckbox( gridCatmullTrack, "Track Target", this, "OnClick_TrackTarget" );
		m_ToggleTrackTarget.SetTooltip( "Keep the camera pointed at the selected target during travel" );

		m_CaptureOrientation = UIActionManager.CreateCheckbox( col, "#STR_COT_CAMERA_MODULE_CAPTURE_ORIENT", this, "OnClick_CaptureOrientation" );
		m_CaptureOrientation.SetTooltip( "Store the current camera rotation when Capture Position is pressed" );

		m_EasingSelectBox = UIActionManager.CreateSelectionBox( col, "Easing", m_EasingNames, this, "OnClick_EasingSelectBox" );
		m_EasingSelectBox.SetSelectorWidth(1.0);
		m_EasingSelectBox.SetSelection((int)JMCameraEasing.EASE_IN_OUT, false);

		// ---- Per-Waypoint Screen Effects ----
		UIActionManager.CreateText( col, "Waypoint Effects", "" );
		UIActionManager.CreatePanel( col, JMTheme.DIVIDER_DARK, 2 );

		m_WP_SliderExposure = UIActionManager.CreateSlider( col, "Exposure (EV)", -5, 5, this, "OnChange_WP_Exposure" );
		m_WP_SliderExposure.SetCurrent( 0 );
		m_WP_SliderExposure.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_WP_SliderExposure.SetStepValue( 0.05 );
		m_WP_SliderExposure.SetWidth( 1.0 );
		m_WP_SliderExposure.SetWidgetWidth( m_WP_SliderExposure.GetLabelWidget(), 0.4 );
		m_WP_SliderExposure.SetWidgetWidth( m_WP_SliderExposure.GetSliderWidget(), 0.6 );

		m_WP_SliderVignette = UIActionManager.CreateSlider( col, "Vignette", 0, 1, this, "OnChange_WP_Vignette" );
		m_WP_SliderVignette.SetCurrent( 0 );
		m_WP_SliderVignette.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_WP_SliderVignette.SetStepValue( 0.01 );
		m_WP_SliderVignette.SetWidth( 1.0 );
		m_WP_SliderVignette.SetWidgetWidth( m_WP_SliderVignette.GetLabelWidget(), 0.4 );
		m_WP_SliderVignette.SetWidgetWidth( m_WP_SliderVignette.GetSliderWidget(), 0.6 );

		m_WP_SliderBlur = UIActionManager.CreateSlider( col, "Blur", 0, 100, this, "OnChange_WP_Blur" );
		m_WP_SliderBlur.SetCurrent( 1 );
		m_WP_SliderBlur.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_WP_SliderBlur.SetStepValue( 0.1 );
		m_WP_SliderBlur.SetWidth( 1.0 );
		m_WP_SliderBlur.SetWidgetWidth( m_WP_SliderBlur.GetLabelWidget(), 0.4 );
		m_WP_SliderBlur.SetWidgetWidth( m_WP_SliderBlur.GetSliderWidget(), 0.6 );

		m_WP_SliderFOV = UIActionManager.CreateSlider( col, "FOV", 0.001, 4, this, "OnChange_WP_FOV" );
		m_WP_SliderFOV.SetCurrent( 1 );
		m_WP_SliderFOV.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_WP_SliderFOV.SetStepValue( 0.001 );
		m_WP_SliderFOV.SetWidth( 1.0 );
		m_WP_SliderFOV.SetWidgetWidth( m_WP_SliderFOV.GetLabelWidget(), 0.4 );
		m_WP_SliderFOV.SetWidgetWidth( m_WP_SliderFOV.GetSliderWidget(), 0.6 );

		m_WP_SliderShakeIntensity = UIActionManager.CreateSlider( col, "Shake Intensity", 0, 0.5, this, "OnChange_WP_ShakeIntensity" );
		m_WP_SliderShakeIntensity.SetCurrent( 0 );
		m_WP_SliderShakeIntensity.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_WP_SliderShakeIntensity.SetStepValue( 0.005 );
		m_WP_SliderShakeIntensity.SetWidth( 1.0 );
		m_WP_SliderShakeIntensity.SetWidgetWidth( m_WP_SliderShakeIntensity.GetLabelWidget(), 0.4 );
		m_WP_SliderShakeIntensity.SetWidgetWidth( m_WP_SliderShakeIntensity.GetSliderWidget(), 0.6 );

		m_WP_SliderShakeFrequency = UIActionManager.CreateSlider( col, "Shake Frequency", 0.1, 10, this, "OnChange_WP_ShakeFrequency" );
		m_WP_SliderShakeFrequency.SetCurrent( 1.0 );
		m_WP_SliderShakeFrequency.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_WP_SliderShakeFrequency.SetStepValue( 0.1 );
		m_WP_SliderShakeFrequency.SetWidth( 1.0 );
		m_WP_SliderShakeFrequency.SetWidgetWidth( m_WP_SliderShakeFrequency.GetLabelWidget(), 0.4 );
		m_WP_SliderShakeFrequency.SetWidgetWidth( m_WP_SliderShakeFrequency.GetSliderWidget(), 0.6 );

		// Clipboard shortcuts
		Widget gridPosActions = UIActionManager.CreateGridSpacer( col, 1, 2 );
		UIActionButton btnCopyPos  = UIActionManager.CreateButton( gridPosActions, "Copy Pos",  this, "OnClick_CopyPos"  );
		btnCopyPos.SetIcon( JMConstants.ICON_STACK );
		btnCopyPos.SetTooltip( "Copy this waypoint's world position to the clipboard" );
		UIActionButton btnPastePos = UIActionManager.CreateButton( gridPosActions, "Paste Pos", this, "OnClick_PastePos" );
		btnPastePos.SetTooltip( "Paste a world position from the clipboard into this waypoint" );

		UIActionButton btnLookAt = UIActionManager.CreateButton( col, "Look At", this, "OnClick_LookAtSelection" );
		btnLookAt.SetTooltip( "Rotate this waypoint so the camera points at the current target" );

		// ---- Path Controls ----
		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_PATH_CONTROLS", "" );
		UIActionManager.CreatePanel( col, JMTheme.DIVIDER_DARK, 2 );

		m_TravelModeSelectBox = UIActionManager.CreateSelectionBox( col, "Mode", m_TravelModeNames, this, "OnClick_TravelModeSelectBox" );
		m_TravelModeSelectBox.SetSelectorWidth(1.0);
		m_TravelModeSelectBox.SetSelection(0, false);

		m_SliderSpeedMult = UIActionManager.CreateSlider( col, "Speed Mult.", 0.1, 4, this, "OnChange_SpeedMult" );
		m_SliderSpeedMult.SetCurrent( 1.0 );
		m_SliderSpeedMult.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_SliderSpeedMult.SetStepValue( 0.1 );
		m_SliderSpeedMult.SetWidth( 1.0 );
		m_SliderSpeedMult.SetWidgetWidth( m_SliderSpeedMult.GetLabelWidget(), 0.4 );
		m_SliderSpeedMult.SetWidgetWidth( m_SliderSpeedMult.GetSliderWidget(), 0.6 );

		m_LabelDuration = UIActionManager.CreateText( col, "Est. Duration: --", "" );

		Widget gridPlayback = UIActionManager.CreateGridSpacer( col, 1, 2 );
		m_BtnTravel = UIActionManager.CreateButton( gridPlayback, "Travel", this, "OnClick_GoToPositions" );
		m_BtnTravel.SetIcon( JMConstants.ICON_PLAY );
		m_BtnTravel.SetTooltip( "Start travelling through the waypoints" );
		m_BtnPauseResume = UIActionManager.CreateButton( gridPlayback, "Pause", this, "OnClick_PauseResume" );
		m_BtnPauseResume.SetIcon( JMConstants.ICON_PAUSE );
		m_BtnPauseResume.SetTooltip( "Pause or resume the current travel" );

		// ---- Saved Paths ----
		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_SAVED_PATHS", "" );
		UIActionManager.CreatePanel( col, JMTheme.DIVIDER_DARK, 2 );

		m_PathNames     = m_Module.GetPathNames();
		m_PathSelectBox = UIActionManager.CreateSelectionBox( col, "#STR_COT_CAMERA_MODULE_SAVED_PATHS", m_PathNames, this, "OnClick_PathSelectBox" );
		m_PathSelectBox.SetSelectorWidth(1.0);

		m_PathName = UIActionManager.CreateEditableText( col, "#STR_COT_CAMERA_MODULE_PATH_NAME", this );

		Widget gridPathActions = UIActionManager.CreateGridSpacer( col, 1, 3 );
		UIActionButton btnSavePath = UIActionManager.CreateButton( gridPathActions, "Save",           this, "OnClick_SavePath"   );
		btnSavePath.SetTooltip( "Save the current waypoint path under the name above" );
		UIActionButton btnLoadPath = UIActionManager.CreateButton( gridPathActions, "Load",          this, "OnClick_LoadPath"   );
		btnLoadPath.SetTooltip( "Load the selected saved path into the editor" );
		UIActionConfirmInline delPathBtn = UIActionManager.CreateConfirmInline( gridPathActions, "Delete", this, "OnClick_DeletePath" );
		UIActionIconGrid.ApplyDeletePreset( delPathBtn );
		delPathBtn.SetTooltip( "Delete the selected saved path" );
	}

	void InitCameraBookmarks()
	{
		Widget col = UIActionManager.CreateGridSpacer( m_PanelBookmarks, 1, 1 );

		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_BOOKMARKS", "" );
		UIActionManager.CreatePanel( col, JMTheme.DIVIDER_DARK, 2 );

		m_BookmarkNames     = m_Module.GetBookmarkNames();
		m_BookmarkSelectBox = UIActionManager.CreateSelectionBox( col, "#STR_COT_CAMERA_MODULE_BOOKMARKS", m_BookmarkNames, this, "OnClick_BookmarkSelectBox" );
		m_BookmarkSelectBox.SetSelectorWidth(1.0);

		Widget gridBookmarkActions = UIActionManager.CreateGridSpacer( col, 1, 3 );
		UIActionButton btnSaveBm = UIActionManager.CreateButton( gridBookmarkActions, "Save",   this, "OnClick_SaveBookmark"    );
		btnSaveBm.SetTooltip( "Save the current camera position as a bookmark" );
		UIActionButton btnGoToBm = UIActionManager.CreateButton( gridBookmarkActions, "Go To",  this, "OnClick_TeleportBookmark" );
		btnGoToBm.SetTooltip( "Teleport the camera to the selected bookmark" );
		m_DeleteBookmarkBtn = UIActionManager.CreateConfirmInline( gridBookmarkActions, "Delete", this, "OnClick_DeleteBookmark" );
		UIActionIconGrid.ApplyDeletePreset( m_DeleteBookmarkBtn );
		m_DeleteBookmarkBtn.SetTooltip( "Remove the selected bookmark" );
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
		m_PanelTraveling.Show( id == 1 );
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

	void OnClick_ResetEffects( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;

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
	//  Traveling panel handlers
	// ----------------------------------------------------------------

	void OnClick_LookAtSelection( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		m_Module.LookAtSelection();
	}

	void OnClick_CopyPos( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		if ( m_Waypoints.Count() > 0 )
			g_Game.CopyToClipboard(m_Waypoints[m_WaypointID].Position.ToString());
	}

	void OnClick_PastePos( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		string clipboard;
		g_Game.CopyFromClipboard(clipboard);
		vector pos = clipboard.BeautifiedToVector();
		if ( pos != vector.Zero && m_Waypoints.Count() > 0 )
			m_Waypoints[m_WaypointID].Position = pos;
	}

	void OnClick_CapturePosition( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		if ( m_Waypoints.Count() == 0 ) return;
		m_Waypoints[m_WaypointID].Position = g_Game.GetCurrentCameraPosition();
	}

	void OnChange_WaypointSpeed( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || m_Waypoints.Count() == 0 ) return;
		m_Waypoints[m_WaypointID].Speed = action.GetCurrent();
		UpdateDurationLabel();
	}

	void OnChange_HoldTime( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || m_Waypoints.Count() == 0 ) return;
		m_Waypoints[m_WaypointID].HoldTime = action.GetCurrent();
		UpdateDurationLabel();
	}

	void OnClick_WaypointSelectBox( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;

		int sel = m_WaypointSelectBox.GetSelection();
		if ( sel < 0 ) return;

		if ( m_Waypoints.Count() > 0 )
			FlushWaypointEdits();

		m_WaypointID = sel;
		UpdateUIWaypoint();
		TeleportToSelectedWaypoint();
	}

	void OnClick_AddWaypoint( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;

		if ( m_Waypoints.Count() > 0 )
			FlushWaypointEdits();

		JMCameraWaypoint wp = new JMCameraWaypoint();
		wp.Position = g_Game.GetCurrentCameraPosition();

		// Always capture the current camera orientation so lock-look direction is preserved
		if ( CurrentActiveCamera )
		{
			wp.Orientation         = CurrentActiveCamera.GetOrientation();
			wp.OrientationCaptured = true;
		}

		// Copy all non-position/orientation parameters from the previous waypoint
		if ( m_Waypoints.Count() > 0 )
		{
			JMCameraWaypoint prev = m_Waypoints[m_Waypoints.Count() - 1];
			wp.Speed          = prev.Speed;
			if ( wp.Speed <= 0 ) wp.Speed = 5.0;
			wp.HoldTime       = prev.HoldTime;
			wp.UseCatmull     = prev.UseCatmull;
			wp.TrackTarget    = prev.TrackTarget;
			wp.m_Easing       = prev.m_Easing;
			wp.Exposure       = prev.Exposure;
			wp.Vignette       = prev.Vignette;
			wp.Blur           = prev.Blur;
			wp.FOV            = prev.FOV;
			wp.ShakeIntensity = prev.ShakeIntensity;
			wp.ShakeFrequency = prev.ShakeFrequency;
		}

		m_Waypoints.Insert( wp );
		m_WaypointID = m_Waypoints.Count() - 1;
		UpdateUIWaypoint();
		UpdateDurationLabel();
	}

	void OnClick_DeleteWaypoint( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		if ( m_Waypoints.Count() == 0 ) return;

		m_Waypoints.Remove( m_WaypointID );

		if ( m_WaypointID >= m_Waypoints.Count() && m_WaypointID > 0 )
			m_WaypointID = m_Waypoints.Count() - 1;

		UpdateUIWaypoint();
		UpdateDurationLabel();
	}

	void OnClick_MoveWaypointUp( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		if ( m_WaypointID <= 0 || m_Waypoints.Count() < 2 ) return;

		FlushWaypointEdits();

		// Swap with the waypoint above
		JMCameraWaypoint tmp  = m_Waypoints[m_WaypointID - 1];
		m_Waypoints[m_WaypointID - 1] = m_Waypoints[m_WaypointID];
		m_Waypoints[m_WaypointID]     = tmp;
		m_WaypointID--;

		UpdateUIWaypoint();
		UpdateDurationLabel();
	}

	void OnClick_MoveWaypointDown( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		if ( m_WaypointID >= m_Waypoints.Count() - 1 ) return;

		FlushWaypointEdits();

		JMCameraWaypoint tmp  = m_Waypoints[m_WaypointID + 1];
		m_Waypoints[m_WaypointID + 1] = m_Waypoints[m_WaypointID];
		m_Waypoints[m_WaypointID]     = tmp;
		m_WaypointID++;

		UpdateUIWaypoint();
		UpdateDurationLabel();
	}

	void OnClick_ClearWaypoints( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		m_Waypoints.Clear();
		m_WaypointID = 0;
		UpdateUIWaypoint();
		UpdateDurationLabel();
	}

	void OnClick_CaptureOrientation( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		if ( m_Waypoints.Count() == 0 ) return;

		JMCameraWaypoint wp = m_Waypoints[m_WaypointID];
		if ( !action.IsChecked() )
		{
			wp.OrientationCaptured = false;
			return;
		}

		if ( CurrentActiveCamera )
			wp.Orientation = CurrentActiveCamera.GetOrientation();

		wp.OrientationCaptured = true;
	}

	void OnClick_TrackTarget( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || m_Waypoints.Count() == 0 ) return;
		m_Waypoints[m_WaypointID].TrackTarget = action.IsChecked();
	}

	void OnClick_EasingSelectBox( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || m_Waypoints.Count() == 0 ) return;
		m_Waypoints[m_WaypointID].m_Easing = m_EasingSelectBox.GetSelection();
	}

	// Per-waypoint effect handlers
	void OnChange_WP_Exposure( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || m_Waypoints.Count() == 0 ) return;
		m_Waypoints[m_WaypointID].Exposure = action.GetCurrent();
	}

	void OnChange_WP_Vignette( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || m_Waypoints.Count() == 0 ) return;
		m_Waypoints[m_WaypointID].Vignette = action.GetCurrent();
	}

	void OnChange_WP_Blur( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || m_Waypoints.Count() == 0 ) return;
		m_Waypoints[m_WaypointID].Blur = action.GetCurrent();
	}

	void OnChange_WP_FOV( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || m_Waypoints.Count() == 0 ) return;
		m_Waypoints[m_WaypointID].FOV = action.GetCurrent();
	}

	void OnChange_WP_ShakeIntensity( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || m_Waypoints.Count() == 0 ) return;
		m_Waypoints[m_WaypointID].ShakeIntensity = action.GetCurrent();
	}

	void OnChange_WP_ShakeFrequency( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || m_Waypoints.Count() == 0 ) return;
		m_Waypoints[m_WaypointID].ShakeFrequency = action.GetCurrent();
	}

	void OnClick_TravelModeSelectBox( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		m_Module.m_TravelMode = m_TravelModeSelectBox.GetSelection();
	}

	void OnChange_SpeedMult( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		m_Module.m_TravelSpeedMult = action.GetCurrent();
		UpdateDurationLabel();
	}

	void OnClick_GoToPositions( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		if ( m_Waypoints.Count() > 0 )
			FlushWaypointEdits();
		m_Module.GoToSelection( m_Waypoints );
	}

	void OnClick_PauseResume( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		m_Module.ToggleTravelPause();
		if ( m_BtnPauseResume )
		{
			if ( m_Module.IsTravelPaused() )
			{
				m_BtnPauseResume.SetButton( "Resume" );
				m_BtnPauseResume.SetIcon( JMConstants.ICON_PLAY );
			}
			else
			{
				m_BtnPauseResume.SetButton( "Pause" );
				m_BtnPauseResume.SetIcon( JMConstants.ICON_PAUSE );
			}
		}
	}

	// ----------------------------------------------------------------
	//  Path save/load handlers
	// ----------------------------------------------------------------

	void RefreshPathSelectBox()
	{
		if ( !m_PathSelectBox ) return;
		m_PathNames = m_Module.GetPathNames();
		// Belt + suspenders: UIActionSelectBox.SetSelections seeds an empty
		// array, but pre-seeding here gives the placeholder a UX-meaningful
		// label and survives if SetSelections' guard changes.
		if ( m_PathNames.Count() == 0 )
			m_PathNames.Insert( "(no paths)" );
		m_PathSelectBox.SetSelections( m_PathNames );
	}

	void OnClick_PathSelectBox( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		int idx = m_PathSelectBox.GetSelection();
		if ( idx >= 0 && m_PathNames.Count() > idx )
			m_PathName.SetText( m_PathNames[idx] );
	}

	void OnClick_SavePath( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		string name = m_PathName.GetText();
		if ( name == "" ) return;
		if ( m_Waypoints.Count() > 0 )
			FlushWaypointEdits();
		m_Module.m_Waypoints = m_Waypoints;
		m_Module.SaveCurrentPath( name );
		RefreshPathSelectBox();
	}

	void OnClick_LoadPath( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		string name = m_PathName.GetText();
		if ( name == "" ) return;
		m_Module.LoadPath( name );
		m_Waypoints  = m_Module.m_Waypoints;
		m_WaypointID = 0;
		UpdateUIWaypoint();
		UpdateDurationLabel();
	}

	void OnClick_DeletePath( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		string name = m_PathName.GetText();
		if ( name == "" ) return;
		m_Module.DeletePath( name );
		RefreshPathSelectBox();
	}

	// ----------------------------------------------------------------
	//  Bookmarks panel handlers
	// ----------------------------------------------------------------

	void RefreshBookmarkSelectBox()
	{
		if ( !m_BookmarkSelectBox ) return;
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

	string GetSelectedBookmarkName()
	{
		int idx = m_BookmarkSelectBox.GetSelection();
		if ( idx >= 0 && m_BookmarkNames && m_BookmarkNames.Count() > idx )
			return m_BookmarkNames[idx];
		return "";
	}

	void OnClick_SaveBookmark( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_CAMERA_MODULE_BOOKMARK_SAVE", "#STR_COT_CAMERA_MODULE_BOOKMARK_NAME", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "OnClick_SaveBookmark_Confirm" );
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

	void OnClick_TeleportBookmark( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
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

	// ----------------------------------------------------------------
	//  Shared helpers
	// ----------------------------------------------------------------

	bool GetCurrentCamera1stPersonADSHideScope()
	{
		if (CurrentActiveCamera)
			return CurrentActiveCamera.m_JM_1stPersonADS_HideScope;
		return false;
	}

	void SetEnableFullmapCamera(bool enable)
	{
		m_EnableFullmapCamera.SetChecked(enable);
	}

	// Teleport the active camera to the currently selected waypoint and apply its effects instantly
	private void TeleportToSelectedWaypoint()
	{
		if ( m_Waypoints.Count() == 0 || m_WaypointID < 0 ) return;
		if ( !CurrentActiveCamera ) return;

		JMCameraWaypoint wp = m_Waypoints[m_WaypointID];

		if ( wp.Position != vector.Zero )
			CurrentActiveCamera.SetPosition( wp.Position );

		if ( wp.OrientationCaptured )
		{
			CurrentActiveCamera.SetOrientation( wp.Orientation );
			JMCinematicCamera cine;
			if ( Class.CastTo(cine, CurrentActiveCamera) )
				cine.orientation = wp.Orientation;
		}

		// Apply waypoint effects instantly (no lerp)
		g_Game.SetEVValue( wp.Exposure );
		PPEffects.SetVignette( wp.Vignette, 0, 0, 0, 0 );
		m_Module.m_Exposure = wp.Exposure;
		m_Module.m_Vignette = wp.Vignette;
		m_Module.m_Blur     = wp.Blur;
		if ( wp.FOV > 0 )
		{
			CurrentActiveCamera.SetFOV( wp.FOV );
			m_Module.m_TargetFOV = wp.FOV;
		}
	}

	// Compute total estimated path duration (sum of all waypoint times + hold times, adjusted by speed)
	private void UpdateDurationLabel()
	{
		if ( !m_LabelDuration ) return;

		float total = 0;
		float speedMult = m_Module.m_TravelSpeedMult;
		if ( speedMult <= 0 ) speedMult = 1.0;

		int count = m_Waypoints.Count();
		for ( int i = 1; i < count; i++ )
		{
			JMCameraWaypoint wp = m_Waypoints[i];
			float segTime;
			if ( wp.Speed > 0 )
			{
				int j = i - 1;
				float dist = vector.Distance( m_Waypoints[j].Position, wp.Position );
				if ( dist < 0.01 ) dist = 0.01;
				segTime = ( dist / wp.Speed ) / speedMult;
			}
			else
			{
				segTime = wp.Time / speedMult;
			}
			total += segTime + wp.HoldTime;
		}

		string label;
		if ( count < 2 )
			label = "Est. Duration: --";
		else
			label = string.Format( "Est. Duration: %1s", Math.Round( total * 10 ) * 0.1 );

		m_LabelDuration.SetLabel( label );
	}

	// Flush the UI edit fields into the currently selected waypoint struct
	void FlushWaypointEdits()
	{
		if ( m_WaypointID < 0 || m_WaypointID >= m_Waypoints.Count() )
			return;

		JMCameraWaypoint wp = m_Waypoints[m_WaypointID];

		float speed = m_SliderWaypointSpeed.GetCurrent();
		if ( speed > 0 ) wp.Speed = speed;

		wp.HoldTime    = m_SliderHoldTime.GetCurrent();
		wp.UseCatmull  = m_ToggleCatmull.IsChecked();
		wp.Smooth      = !wp.UseCatmull;
		wp.TrackTarget = m_ToggleTrackTarget.IsChecked();
		wp.m_Easing    = m_EasingSelectBox.GetSelection();
	}

	// Refresh the waypoint selectbox and populate UI from the currently selected waypoint
	void UpdateUIWaypoint()
	{
		m_WaypointNames.Clear();
		int count = m_Waypoints.Count();
		for ( int i = 0; i < count; i++ )
			m_WaypointNames.Insert( string.Format("Waypoint %1", i + 1) );

		// Belt + suspenders: UIActionSelectBox.SetSelections also seeds an
		// empty array with "(empty)", but seeding here keeps the placeholder
		// UX-meaningful ("(no waypoints)") and survives SetSelections refactors.
		if ( m_WaypointNames.Count() == 0 )
			m_WaypointNames.Insert( "(no waypoints)" );

		if ( m_WaypointSelectBox )
			m_WaypointSelectBox.SetSelections( m_WaypointNames );

		if ( m_WaypointID < 0 )
			m_WaypointID = 0;
		else if ( m_WaypointID >= m_Waypoints.Count() )
			m_WaypointID = m_Waypoints.Count() - 1;

		if ( m_WaypointSelectBox && m_WaypointNames.Count() > 0 && m_WaypointID >= 0 )
			m_WaypointSelectBox.SetSelection( m_WaypointID, false );

		if ( m_WaypointID < 0 || m_Waypoints.Count() == 0 )
		{
			m_SliderWaypointSpeed.SetCurrent(5);
			m_SliderHoldTime.SetCurrent(0);
			m_ToggleCatmull.SetChecked(false);
			m_ToggleTrackTarget.SetChecked(false);
			m_CaptureOrientation.SetChecked(false);
			m_EasingSelectBox.SetSelection((int)JMCameraEasing.EASE_IN_OUT, false);
			m_WP_SliderExposure.SetCurrent(0);
			m_WP_SliderVignette.SetCurrent(0);
			m_WP_SliderBlur.SetCurrent(1);
			m_WP_SliderFOV.SetCurrent(1);
			m_WP_SliderShakeIntensity.SetCurrent(0);
			m_WP_SliderShakeFrequency.SetCurrent(1.0);
			return;
		}

		JMCameraWaypoint wp = m_Waypoints[m_WaypointID];
		float displaySpeed = wp.Speed;
		if ( displaySpeed <= 0 ) displaySpeed = 5.0;
		m_SliderWaypointSpeed.SetCurrent( displaySpeed );
		m_SliderHoldTime.SetCurrent( wp.HoldTime );
		m_ToggleCatmull.SetChecked( wp.UseCatmull );
		m_ToggleTrackTarget.SetChecked( wp.TrackTarget );
		m_CaptureOrientation.SetChecked( wp.OrientationCaptured );
		m_EasingSelectBox.SetSelection( (int)wp.m_Easing, false );
		m_WP_SliderExposure.SetCurrent( wp.Exposure );
		m_WP_SliderVignette.SetCurrent( wp.Vignette );
		m_WP_SliderBlur.SetCurrent( wp.Blur );
		m_WP_SliderFOV.SetCurrent( wp.FOV );
		m_WP_SliderShakeIntensity.SetCurrent( wp.ShakeIntensity );
		m_WP_SliderShakeFrequency.SetCurrent( wp.ShakeFrequency );
	}
}
