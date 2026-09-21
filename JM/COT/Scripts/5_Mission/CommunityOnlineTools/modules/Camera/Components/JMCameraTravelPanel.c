//! The camera form's Traveling panel: waypoint list and editing, per-waypoint camera effects, travel mode and playback, and saved paths.
class JMCameraTravelPanel
{
	protected JMCameraForm m_Form;

	// ---- Traveling panel ----
	protected GridSpacerWidget m_PanelTraveling;

	// Waypoint list
	protected UIActionSelectBox      m_WaypointSelectBox;
	protected ref TStringArray       m_WaypointNames;

	// Edit-selected-waypoint controls
	protected UIActionSlider         m_SliderWaypointSpeed;
	protected UIActionSlider         m_SliderHoldTime;
	protected UIActionCheckbox       m_ToggleCatmull;
	protected UIActionCheckbox       m_ToggleTrackTarget;
	protected UIActionCheckbox       m_CaptureOrientation;
	protected UIActionSelectBox      m_EasingSelectBox;
	protected ref TStringArray       m_EasingNames;

	// Per-waypoint screen effects + shake
	protected UIActionSlider         m_WP_SliderExposure;
	protected UIActionSlider         m_WP_SliderVignette;
	protected UIActionSlider         m_WP_SliderBlur;
	protected UIActionSlider         m_WP_SliderFOV;
	protected UIActionSlider         m_WP_SliderShakeIntensity;
	protected UIActionSlider         m_WP_SliderShakeFrequency;

	// Playback buttons
	protected UIActionButton         m_BtnTravel;
	protected UIActionButton         m_BtnPauseResume;

	// Path controls
	protected UIActionSelectBox      m_TravelModeSelectBox;
	protected ref TStringArray       m_TravelModeNames;
	protected UIActionSlider         m_SliderSpeedMult;
	protected UIActionText           m_LabelDuration;

	// Path save/load
	protected UIActionEditableText   m_PathName;
	protected UIActionSelectBox      m_PathSelectBox;
	protected ref TStringArray       m_PathNames;

	// Working waypoint list (shared with module)
	protected int                              m_WaypointID;
	protected ref array< ref JMCameraWaypoint > m_Waypoints;

	void JMCameraTravelPanel( JMCameraForm form )
	{
		m_Form = form;

		m_Waypoints       = new array< ref JMCameraWaypoint >;
		m_PathNames       = new TStringArray;
		m_WaypointNames   = new TStringArray;
		m_EasingNames     = new TStringArray;
		m_TravelModeNames = new TStringArray;

		// Waypoint SelectBox is created during OnCreate, before any waypoints
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

	//! Shows the panel when the form's section selector is on it.
	void Show( bool show )
	{
		m_PanelTraveling.Show( show );
	}

	//! Pulls the waypoints, travel mode and saved paths in from the module - the
	//! module keeps them across the form being closed and reopened.
	void LoadFromModule()
	{
		JMCameraModule module = m_Form.GetModule();

		m_Waypoints = module.m_Waypoints;

		m_TravelModeSelectBox.SetSelection((int)module.m_TravelMode, false);
		m_SliderSpeedMult.SetCurrent(module.m_TravelSpeedMult);

		RefreshPathSelectBox();
	}

	//! Hands the working waypoint list back to the module when the form hides.
	void SaveToModule()
	{
		if ( m_Waypoints.Count() > 0 )
			FlushWaypointEdits();

		m_Form.GetModule().m_Waypoints = m_Waypoints;
	}

	//! Creates the panel inside the form's scroller content and fills it.
	void Build( Widget actions )
	{
		m_PanelTraveling = UIActionManager.CreateGridSpacer( actions, 1, 1 );

		Widget col = UIActionManager.CreateGridSpacer( m_PanelTraveling, 1, 1 );

		// ---- Waypoint List ----
		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_WAYPOINT_EDITOR", "" );
		UIActionManager.CreateDivider( col, JMTheme.DIVIDER_DARK, 2 );

		m_WaypointSelectBox = UIActionManager.CreateSelectionBox( col, "#STR_COT_CAMERA_SELECT", m_WaypointNames, this, "OnClick_WaypointSelectBox" );
		m_WaypointSelectBox.SetSelectorWidth(1.0);

		// Add / Remove / Move Up / Move Down in a 2x2 grid
		Widget gridListButtons = UIActionManager.CreateGridSpacer( col, 2, 2 );
		UIActionButton btnAddWp = UIActionManager.CreateButton( gridListButtons, "#STR_COT_GENERIC_ADD",    this, ""     );
		if ( btnAddWp ) btnAddWp.SetOnClick( this, "OnClick_AddWaypoint" );
		btnAddWp.SetTooltip( "#STR_COT_CAMERA_ADD_A_NEW_WAYPOINT_AT_THE" );
		UIActionConfirmInline delWaypointBtn = UIActionManager.CreateConfirmInline( gridListButtons, "#STR_COT_GENERIC_DELETE", this, "OnClick_DeleteWaypoint" );
		UIActionIconGrid.ApplyDeletePreset( delWaypointBtn );
		delWaypointBtn.SetTooltip( "#STR_COT_CAMERA_REMOVE_THE_SELECTED_WAYPOINT" );
		UIActionButton btnUp   = UIActionManager.CreateButton( gridListButtons, "Up",             this, ""   );
		if ( btnUp ) btnUp.SetOnClick( this, "OnClick_MoveWaypointUp" );
		btnUp.SetTooltip( "#STR_COT_CAMERA_MOVE_THE_SELECTED_WAYPOINT_EARLIER_IN" );
		UIActionButton btnDown = UIActionManager.CreateButton( gridListButtons, "#STR_COT_CAMERA_DOWN",           this, "" );
		if ( btnDown ) btnDown.SetOnClick( this, "OnClick_MoveWaypointDown" );
		btnDown.SetTooltip( "#STR_COT_CAMERA_MOVE_THE_SELECTED_WAYPOINT_LATER_IN" );

		UIActionButton btnClear = UIActionManager.CreateButton( col, "#STR_COT_CAMERA_CLEAR_ALL", this, "" );
		if ( btnClear ) btnClear.SetOnClick( this, "OnClick_ClearWaypoints" );
		btnClear.SetTooltip( "#STR_COT_CAMERA_REMOVE_EVERY_WAYPOINT_FROM_THE_CURRENT" );

		// ---- Edit Selected Waypoint ----
		UIActionManager.CreateText( col, "#STR_COT_CAMERA_EDIT_SELECTED_WAYPOINT", "" );
		UIActionManager.CreateDivider( col, JMTheme.DIVIDER_DARK, 2 );

		UIActionButton btnCapPos = UIActionManager.CreateButton( col, "#STR_COT_CAMERA_CAPTURE_POSITION", this, "" );
		if ( btnCapPos ) btnCapPos.SetOnClick( this, "OnClick_CapturePosition" );
		btnCapPos.SetTooltip( "#STR_COT_CAMERA_UPDATE_THE_SELECTED_WAYPOINT_WITH_THE" );

		m_SliderWaypointSpeed = UIActionManager.CreateSyncedSlider( col, "Speed (m/s)", 0.1, 50, this, "OnChange_WaypointSpeed" );
		m_SliderWaypointSpeed.SetCurrent( 5 );
		m_SliderWaypointSpeed.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_SliderWaypointSpeed.SetStepValue( 0.1 );
		m_SliderWaypointSpeed.SetWidth( 1.0 );
		m_SliderWaypointSpeed.SetWidgetWidth( m_SliderWaypointSpeed.GetLabelWidget(), 0.5 );
		m_SliderWaypointSpeed.SetWidgetWidth( m_SliderWaypointSpeed.GetSliderWidget(), 0.5 );

		m_SliderHoldTime = UIActionManager.CreateSyncedSlider( col, "Hold Time", 0, 10, this, "OnChange_HoldTime" );
		m_SliderHoldTime.SetCurrent( 0 );
		m_SliderHoldTime.SetFormat( "#STR_COT_FORMAT_SECOND" );
		m_SliderHoldTime.SetStepValue( 0.5 );
		m_SliderHoldTime.SetWidth( 1.0 );
		m_SliderHoldTime.SetWidgetWidth( m_SliderHoldTime.GetLabelWidget(), 0.5 );
		m_SliderHoldTime.SetWidgetWidth( m_SliderHoldTime.GetSliderWidget(), 0.5 );

		// Catmull + Track Target on the same row
		Widget gridCatmullTrack = UIActionManager.CreateGridSpacer( col, 1, 2 );
		m_ToggleCatmull     = UIActionManager.CreateCheckbox( gridCatmullTrack, "#STR_COT_CAMERA_CATMULL_ROM",  this );
		m_ToggleCatmull.SetTooltip( "#STR_COT_CAMERA_SMOOTH_THE_TRAVEL_PATH_WITH_CATMULL" );
		m_ToggleTrackTarget = UIActionManager.CreateCheckbox( gridCatmullTrack, "#STR_COT_CAMERA_TRACK_TARGET", this, "OnClick_TrackTarget" );
		m_ToggleTrackTarget.SetTooltip( "#STR_COT_CAMERA_KEEP_THE_CAMERA_POINTED_AT_THE" );

		m_CaptureOrientation = UIActionManager.CreateCheckbox( col, "#STR_COT_CAMERA_MODULE_CAPTURE_ORIENT", this, "OnClick_CaptureOrientation" );
		m_CaptureOrientation.SetTooltip( "#STR_COT_CAMERA_STORE_THE_CURRENT_CAMERA_ROTATION_WHEN" );

		m_EasingSelectBox = UIActionManager.CreateSelectionBox( col, "#STR_COT_CAMERA_EASING", m_EasingNames, this, "OnClick_EasingSelectBox" );
		m_EasingSelectBox.SetSelectorWidth(1.0);
		m_EasingSelectBox.SetSelection((int)JMCameraEasing.EASE_IN_OUT, false);

		// ---- Per-Waypoint Screen Effects ----
		UIActionManager.CreateText( col, "#STR_COT_CAMERA_WAYPOINT_EFFECTS", "" );
		UIActionManager.CreateDivider( col, JMTheme.DIVIDER_DARK, 2 );

		m_WP_SliderExposure = UIActionManager.CreateSyncedSlider( col, "Exposure (EV)", -5, 5, this, "OnChange_WP_Exposure" );
		m_WP_SliderExposure.SetCurrent( 0 );
		m_WP_SliderExposure.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_WP_SliderExposure.SetStepValue( 0.05 );
		m_WP_SliderExposure.SetWidth( 1.0 );
		m_WP_SliderExposure.SetWidgetWidth( m_WP_SliderExposure.GetLabelWidget(), 0.4 );
		m_WP_SliderExposure.SetWidgetWidth( m_WP_SliderExposure.GetSliderWidget(), 0.6 );

		m_WP_SliderVignette = UIActionManager.CreateSyncedSlider( col, "Vignette", 0, 1, this, "OnChange_WP_Vignette" );
		m_WP_SliderVignette.SetCurrent( 0 );
		m_WP_SliderVignette.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_WP_SliderVignette.SetStepValue( 0.01 );
		m_WP_SliderVignette.SetWidth( 1.0 );
		m_WP_SliderVignette.SetWidgetWidth( m_WP_SliderVignette.GetLabelWidget(), 0.4 );
		m_WP_SliderVignette.SetWidgetWidth( m_WP_SliderVignette.GetSliderWidget(), 0.6 );

		m_WP_SliderBlur = UIActionManager.CreateSyncedSlider( col, "Blur", 0, 100, this, "OnChange_WP_Blur" );
		m_WP_SliderBlur.SetCurrent( 1 );
		m_WP_SliderBlur.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_WP_SliderBlur.SetStepValue( 0.1 );
		m_WP_SliderBlur.SetWidth( 1.0 );
		m_WP_SliderBlur.SetWidgetWidth( m_WP_SliderBlur.GetLabelWidget(), 0.4 );
		m_WP_SliderBlur.SetWidgetWidth( m_WP_SliderBlur.GetSliderWidget(), 0.6 );

		m_WP_SliderFOV = UIActionManager.CreateSyncedSlider( col, "FOV", 0.001, 4, this, "OnChange_WP_FOV" );
		m_WP_SliderFOV.SetCurrent( 1 );
		m_WP_SliderFOV.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_WP_SliderFOV.SetStepValue( 0.001 );
		m_WP_SliderFOV.SetWidth( 1.0 );
		m_WP_SliderFOV.SetWidgetWidth( m_WP_SliderFOV.GetLabelWidget(), 0.4 );
		m_WP_SliderFOV.SetWidgetWidth( m_WP_SliderFOV.GetSliderWidget(), 0.6 );

		m_WP_SliderShakeIntensity = UIActionManager.CreateSyncedSlider( col, "Shake Intensity", 0, 0.5, this, "OnChange_WP_ShakeIntensity" );
		m_WP_SliderShakeIntensity.SetCurrent( 0 );
		m_WP_SliderShakeIntensity.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_WP_SliderShakeIntensity.SetStepValue( 0.005 );
		m_WP_SliderShakeIntensity.SetWidth( 1.0 );
		m_WP_SliderShakeIntensity.SetWidgetWidth( m_WP_SliderShakeIntensity.GetLabelWidget(), 0.4 );
		m_WP_SliderShakeIntensity.SetWidgetWidth( m_WP_SliderShakeIntensity.GetSliderWidget(), 0.6 );

		m_WP_SliderShakeFrequency = UIActionManager.CreateSyncedSlider( col, "Shake Frequency", 0.1, 10, this, "OnChange_WP_ShakeFrequency" );
		m_WP_SliderShakeFrequency.SetCurrent( 1.0 );
		m_WP_SliderShakeFrequency.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_WP_SliderShakeFrequency.SetStepValue( 0.1 );
		m_WP_SliderShakeFrequency.SetWidth( 1.0 );
		m_WP_SliderShakeFrequency.SetWidgetWidth( m_WP_SliderShakeFrequency.GetLabelWidget(), 0.4 );
		m_WP_SliderShakeFrequency.SetWidgetWidth( m_WP_SliderShakeFrequency.GetSliderWidget(), 0.6 );

		// Clipboard shortcuts
		Widget gridPosActions = UIActionManager.CreateGridSpacer( col, 1, 2 );
		UIActionButton btnCopyPos  = UIActionManager.CreateButton( gridPosActions, "#STR_COT_CAMERA_COPY_POS",  this, ""  );
		if ( btnCopyPos ) btnCopyPos.SetOnClick( this, "OnClick_CopyPos" );
		btnCopyPos.SetIcon( JMConstants.ICON_STACK );
		btnCopyPos.SetTooltip( "#STR_COT_CAMERA_COPY_THIS_WAYPOINT_S_WORLD_POSITION" );
		UIActionButton btnPastePos = UIActionManager.CreateButton( gridPosActions, "#STR_COT_CAMERA_PASTE_POS", this, "" );
		if ( btnPastePos ) btnPastePos.SetOnClick( this, "OnClick_PastePos" );
		btnPastePos.SetTooltip( "#STR_COT_CAMERA_PASTE_A_WORLD_POSITION_FROM_THE" );

		UIActionButton btnLookAt = UIActionManager.CreateButton( col, "#STR_COT_CAMERA_LOOK_AT", this, "" );
		if ( btnLookAt ) btnLookAt.SetOnClick( this, "OnClick_LookAtSelection" );
		btnLookAt.SetTooltip( "#STR_COT_CAMERA_ROTATE_THIS_WAYPOINT_SO_THE_CAMERA" );

		// ---- Path Controls ----
		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_PATH_CONTROLS", "" );
		UIActionManager.CreateDivider( col, JMTheme.DIVIDER_DARK, 2 );

		m_TravelModeSelectBox = UIActionManager.CreateSelectionBox( col, "#STR_COT_CAMERA_MODE", m_TravelModeNames, this, "OnClick_TravelModeSelectBox" );
		m_TravelModeSelectBox.SetSelectorWidth(1.0);
		m_TravelModeSelectBox.SetSelection(0, false);

		m_SliderSpeedMult = UIActionManager.CreateSyncedSlider( col, "Speed Mult.", 0.1, 4, this, "OnChange_SpeedMult" );
		m_SliderSpeedMult.SetCurrent( 1.0 );
		m_SliderSpeedMult.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_SliderSpeedMult.SetStepValue( 0.1 );
		m_SliderSpeedMult.SetWidth( 1.0 );
		m_SliderSpeedMult.SetWidgetWidth( m_SliderSpeedMult.GetLabelWidget(), 0.4 );
		m_SliderSpeedMult.SetWidgetWidth( m_SliderSpeedMult.GetSliderWidget(), 0.6 );

		m_LabelDuration = UIActionManager.CreateText( col, "#STR_COT_CAMERA_EST_DURATION", "" );

		Widget gridPlayback = UIActionManager.CreateGridSpacer( col, 1, 2 );
		m_BtnTravel = UIActionManager.CreateButton( gridPlayback, "#STR_COT_CAMERA_TRAVEL", this, "" );
		if ( m_BtnTravel ) m_BtnTravel.SetOnClick( this, "OnClick_GoToPositions" );
		m_BtnTravel.SetIcon( JMConstants.ICON_PLAY );
		m_BtnTravel.SetTooltip( "#STR_COT_CAMERA_START_TRAVELLING_THROUGH_THE_WAYPOINTS" );
		m_BtnPauseResume = UIActionManager.CreateButton( gridPlayback, "#STR_COT_CAMERA_PAUSE", this, "" );
		if ( m_BtnPauseResume ) m_BtnPauseResume.SetOnClick( this, "OnClick_PauseResume" );
		m_BtnPauseResume.SetIcon( JMConstants.ICON_PAUSE );
		m_BtnPauseResume.SetTooltip( "#STR_COT_CAMERA_PAUSE_OR_RESUME_THE_CURRENT_TRAVEL" );

		// ---- Saved Paths ----
		UIActionManager.CreateText( col, "#STR_COT_CAMERA_SECTION_SAVED_PATHS", "" );
		UIActionManager.CreateDivider( col, JMTheme.DIVIDER_DARK, 2 );

		m_PathNames     = m_Form.GetModule().GetPathNames();
		m_PathSelectBox = UIActionManager.CreateSelectionBox( col, "#STR_COT_CAMERA_MODULE_SAVED_PATHS", m_PathNames, this, "OnClick_PathSelectBox" );
		m_PathSelectBox.SetSelectorWidth(1.0);

		m_PathName = UIActionManager.CreateEditableText( col, "#STR_COT_CAMERA_MODULE_PATH_NAME", this );

		Widget gridPathActions = UIActionManager.CreateGridSpacer( col, 1, 3 );
		UIActionButton btnSavePath = UIActionManager.CreateButton( gridPathActions, "#STR_COT_CAMERA_SAVE",           this, ""   );
		if ( btnSavePath ) btnSavePath.SetOnClick( this, "OnClick_SavePath" );
		btnSavePath.SetTooltip( "#STR_COT_CAMERA_SAVE_THE_CURRENT_WAYPOINT_PATH_UNDER" );
		UIActionButton btnLoadPath = UIActionManager.CreateButton( gridPathActions, "#STR_COT_CAMERA_LOAD",          this, ""   );
		if ( btnLoadPath ) btnLoadPath.SetOnClick( this, "OnClick_LoadPath" );
		btnLoadPath.SetTooltip( "#STR_COT_CAMERA_LOAD_THE_SELECTED_SAVED_PATH_INTO" );
		UIActionConfirmInline delPathBtn = UIActionManager.CreateConfirmInline( gridPathActions, "#STR_COT_GENERIC_DELETE", this, "OnClick_DeletePath" );
		UIActionIconGrid.ApplyDeletePreset( delPathBtn );
		delPathBtn.SetTooltip( "#STR_COT_CAMERA_DELETE_THE_SELECTED_SAVED_PATH" );
	}

	// ----------------------------------------------------------------
	//  Traveling panel handlers
	// ----------------------------------------------------------------

	void OnClick_LookAtSelection( UIActionBase action )
	{
		m_Form.GetModule().LookAtSelection();
	}

	void OnClick_CopyPos( UIActionBase action )
	{
		if ( m_Waypoints.Count() > 0 )
			COTFeedback.Copy(m_Waypoints[m_WaypointID].Position.ToString());
	}

	void OnClick_PastePos( UIActionBase action )
	{
		string clipboard;
		g_Game.CopyFromClipboard(clipboard);
		vector pos = clipboard.BeautifiedToVector();
		if ( pos != vector.Zero && m_Waypoints.Count() > 0 )
			m_Waypoints[m_WaypointID].Position = pos;
	}

	void OnClick_CapturePosition( UIActionBase action )
	{
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

	void OnClick_AddWaypoint( UIActionBase action )
	{
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

	void OnClick_MoveWaypointUp( UIActionBase action )
	{
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

	void OnClick_MoveWaypointDown( UIActionBase action )
	{
		if ( m_WaypointID >= m_Waypoints.Count() - 1 ) return;

		FlushWaypointEdits();

		JMCameraWaypoint tmp  = m_Waypoints[m_WaypointID + 1];
		m_Waypoints[m_WaypointID + 1] = m_Waypoints[m_WaypointID];
		m_Waypoints[m_WaypointID]     = tmp;
		m_WaypointID++;

		UpdateUIWaypoint();
		UpdateDurationLabel();
	}

	void OnClick_ClearWaypoints( UIActionBase action )
	{
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
		m_Form.GetModule().m_TravelMode = m_TravelModeSelectBox.GetSelection();
	}

	void OnChange_SpeedMult( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		m_Form.GetModule().m_TravelSpeedMult = action.GetCurrent();
		UpdateDurationLabel();
	}

	void OnClick_GoToPositions( UIActionBase action )
	{
		if ( m_Waypoints.Count() > 0 )
			FlushWaypointEdits();
		m_Form.GetModule().GoToSelection( m_Waypoints );
	}

	void OnClick_PauseResume( UIActionBase action )
	{
		m_Form.GetModule().ToggleTravelPause();
		if ( m_BtnPauseResume )
		{
			if ( m_Form.GetModule().IsTravelPaused() )
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
		m_PathNames = m_Form.GetModule().GetPathNames();
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

	void OnClick_SavePath( UIActionBase action )
	{
		string name = m_PathName.GetText();
		if ( name == "" ) return;
		if ( m_Waypoints.Count() > 0 )
			FlushWaypointEdits();
		m_Form.GetModule().m_Waypoints = m_Waypoints;
		m_Form.GetModule().SaveCurrentPath( name );
		RefreshPathSelectBox();
	}

	void OnClick_LoadPath( UIActionBase action )
	{
		string name = m_PathName.GetText();
		if ( name == "" ) return;
		m_Form.GetModule().LoadPath( name );
		m_Waypoints  = m_Form.GetModule().m_Waypoints;
		m_WaypointID = 0;
		UpdateUIWaypoint();
		UpdateDurationLabel();
	}

	void OnClick_DeletePath( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		string name = m_PathName.GetText();
		if ( name == "" ) return;
		m_Form.GetModule().DeletePath( name );
		RefreshPathSelectBox();
	}

	// Teleport the active camera to the currently selected waypoint and apply its effects instantly
	protected void TeleportToSelectedWaypoint()
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
		m_Form.GetModule().m_Exposure = wp.Exposure;
		m_Form.GetModule().m_Vignette = wp.Vignette;
		m_Form.GetModule().m_Blur     = wp.Blur;
		if ( wp.FOV > 0 )
		{
			CurrentActiveCamera.SetFOV( wp.FOV );
			m_Form.GetModule().m_TargetFOV = wp.FOV;
		}
	}

	// Compute total estimated path duration (sum of all waypoint times + hold times, adjusted by speed)
	void UpdateDurationLabel()
	{
		if ( !m_LabelDuration ) return;

		float total = 0;
		float speedMult = m_Form.GetModule().m_TravelSpeedMult;
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
