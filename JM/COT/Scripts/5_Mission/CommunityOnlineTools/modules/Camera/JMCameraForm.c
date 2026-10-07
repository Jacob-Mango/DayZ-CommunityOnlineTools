class JMCameraForm: JMFormBase
{
	//! Scroller of the Effects tab. Every tab now has its own scroller - see
	//! m_TabScrollers - this one keeps its old name for sub-mods.
	protected UIActionScroller m_sclr_MainActions;

	//! DEPRECATED - the section selector was replaced by m_Tabs and is never
	//! created. Kept so sub-mods that null-check it still compile.
	protected UIActionSelectBox m_SelectBox;

	//! Tab captions, in tab order.
	protected ref array< string > m_SelectBoxText =
	{
		"#STR_COT_CAMERA_TAB_EFFECTS",
		"#STR_COT_CAMERA_TAB_TRAVELING",
		"#STR_COT_CAMERA_TAB_BOOKMARKS",
		"Settings"
	};

	//! Label/slider split shared by every effect slider.
	static const float SLIDER_LABEL_W = 0.4;
	static const float SLIDER_BODY_W  = 0.6;

	// ---- Tabs ----
	protected UIActionTabs m_Tabs;

	//! Tab indices - what the strip's AddTab() returned for each tab, never written as numbers.
	protected int m_TabIdEffects   = -1;
	protected int m_TabIdTravel    = -1;
	protected int m_TabIdBookmarks = -1;
	protected int m_TabIdSettings  = -1;

	//! One scroller per tab, in tab order.
	protected ref array< UIActionScroller > m_TabScrollers;

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
		m_TabScrollers  = new array< UIActionScroller >;
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

	//! An icon button in a card's title bar whose callback takes only the
	//! action - the SetOnClick signature - rather than (UIEvent, action) the
	//! premade UIActionCard.Add*Button helpers call. Public and static for the
	//! panels split out of this form.
	static UIActionImageButton AddCardClickAction( UIActionCard card, string icon, Class instance, string funcname, string tooltip = "" )
	{
		UIActionImageButton btn = UIActionManager.CreateIconButton( card.GetHeaderActions(), JMConstants.Lucide( icon ), instance, "" );
		if ( !btn )
			return null;

		btn.SetOnClick( instance, funcname );
		btn.SetFixedSize( JMFormBase.HEADER_ACTION_PX, JMFormBase.HEADER_ACTION_PX );

		if ( tooltip != "" )
			btn.SetTooltip( tooltip );

		return btn;
	}

	//! A delete confirm icon in a card's title bar. Fires CHANGE once confirmed.
	static UIActionConfirmInline AddCardDeleteConfirm( UIActionCard card, Class instance, string funcname, string tooltip = "" )
	{
		UIActionConfirmInline btn = UIActionManager.CreateDeleteConfirmIcon( card.GetHeaderActions(), instance, funcname );
		if ( !btn )
			return null;

		btn.SetFixedSize( JMFormBase.HEADER_ACTION_PX, JMFormBase.HEADER_ACTION_PX );
		btn.CenterIcon( JMFormBase.HEADER_ACTION_PX, 16 );

		if ( tooltip != "" )
			btn.SetTooltip( tooltip );

		return btn;
	}

	//! One full-width synced slider with the shared label/slider split.
	//! `format` "" leaves the slider's default format alone.
	static UIActionSlider CreateEffectSlider( Widget parent, string label, float min, float max, Class instance, string funcname, float current, float step, string format = "" )
	{
		UIActionSlider slider = UIActionManager.CreateSyncedSlider( parent, label, min, max, instance, funcname );
		slider.SetCurrent( current );

		if ( format != "" )
			slider.SetFormat( format );

		slider.SetStepValue( step );
		slider.SetWidth( 1.0 );
		slider.SetWidgetWidth( slider.GetLabelWidget(), SLIDER_LABEL_W );
		slider.SetWidgetWidth( slider.GetSliderWidget(), SLIDER_BODY_W );

		return slider;
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
		// ----------------------------------------------------------------------
		// Full pane, tabbed: a tab strip over one content frame per tab, each
		// with its own scroller and its sections as cards.
		//
		//   Effects    Controls, Depth of Field, Post-Process, Shake, Reset
		//   Traveling  JMCameraTravelPanel
		//   Bookmarks  saved camera positions
		//   Settings   options
		//
		// Every tab is built here rather than on first activation: Update()
		// drives the Effects sliders every frame, whichever tab is showing.
		// ----------------------------------------------------------------------

		m_RightTabStrip = layoutRoot.FindAnyWidget( "panel_right_tabs" );
		m_RightContent  = layoutRoot.FindAnyWidget( "panel_right_content" );

		Widget panelEffects   = layoutRoot.FindAnyWidget( "cam_tab_effects" );
		Widget panelTravel    = layoutRoot.FindAnyWidget( "cam_tab_travel" );
		Widget panelBookmarks = layoutRoot.FindAnyWidget( "cam_tab_bookmarks" );
		Widget panelSettings  = layoutRoot.FindAnyWidget( "cam_tab_settings" );

		//! Hoisted: an indexed string read as a call argument miscompiles.
		string labelEffects   = m_SelectBoxText[0];
		string labelTravel    = m_SelectBoxText[1];
		string labelBookmarks = m_SelectBoxText[2];
		string labelSettings  = m_SelectBoxText[3];

		m_Tabs = UIActionManager.CreateTabStrip( m_RightTabStrip, this, "OnClick_SelectBox" );
		m_TabIdEffects   = m_Tabs.AddTab( labelEffects,   JMConstants.Lucide( "aperture" ), panelEffects );
		m_TabIdTravel    = m_Tabs.AddTab( labelTravel,    JMConstants.Lucide( "route" ),    panelTravel );
		m_TabIdBookmarks = m_Tabs.AddTab( labelBookmarks, JMConstants.Lucide( "bookmark" ), panelBookmarks );
		m_TabIdSettings  = m_Tabs.AddTab( labelSettings,  JMConstants.Lucide( "settings" ), panelSettings );

		m_sclr_MainActions = CreateTabScroller( panelEffects );
		m_PanelEffects = UIActionManager.CreateGridSpacer( m_sclr_MainActions.GetContentWidget(), 1, 1 );
			InitCameraEffects();

		UIActionScroller travelScroller = CreateTabScroller( panelTravel );
		m_Travel.Build( travelScroller.GetContentWidget() );

		UIActionScroller bookmarkScroller = CreateTabScroller( panelBookmarks );
		m_PanelBookmarks = UIActionManager.CreateGridSpacer( bookmarkScroller.GetContentWidget(), 1, 1 );
			InitCameraBookmarks();

		UIActionScroller settingsScroller = CreateTabScroller( panelSettings );
		m_PanelSettings = UIActionManager.CreateGridSpacer( settingsScroller.GetContentWidget(), 1, 1 );
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

		// sendEvent = false: the panels are already in their initial state.
		m_Tabs.SetSelection( m_TabIdEffects, false );

		m_Travel.UpdateUIWaypoint();
		m_Travel.UpdateDurationLabel();
		UpdateTabScrollers();
	}

	protected UIActionScroller CreateTabScroller( Widget panel )
	{
		UIActionScroller scroller = UIActionManager.CreateScroller( panel );
		m_TabScrollers.Insert( scroller );
		return scroller;
	}

	protected void UpdateTabScrollers()
	{
		foreach ( UIActionScroller scroller : m_TabScrollers )
		{
			if ( scroller )
				scroller.UpdateScroller();
		}
	}

	protected override COT_ScriptedWidgetEventHandler GetTabStrip()
	{
		return m_Tabs;
	}

	// ----------------------------------------------------------------
	//  Panel init helpers
	// ----------------------------------------------------------------

	void InitCameraEffects()
	{
		// ---- Controls - live, so first -----------------------------------
		UIActionCard controlsCard = UIActionManager.CreateCard( m_PanelEffects, "#STR_COT_CAMERA_SECTION_CONTROLS" );
		Widget controls = controlsCard.GetContent();

		m_SliderSpeed = CreateEffectSlider( controls, "#STR_COT_CAMERA_MODULE_SPEED", 0.001, 10, this, "OnChange_Speed", JMCameraBase.s_CurrentSpeed, 0.001 );
		m_SliderFOV   = CreateEffectSlider( controls, "#STR_COT_CAMERA_MODULE_FOV", 0.001, 4, this, "OnChange_FOV", m_Module.m_CurrentFOV, 0.001 );

		// ---- Depth of field ------------------------------------------------
		UIActionCard dofCard = UIActionManager.CreateCard( m_PanelEffects, "#STR_COT_CAMERA_SECTION_DOF" );
		Widget dof = dofCard.GetContent();

		m_SliderBlurStrength  = CreateEffectSlider( dof, "#STR_COT_CAMERA_MODULE_BLUR", 0, 100, this, "OnChange_Blur", 0, 0.1, "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderFocusDistance = CreateEffectSlider( dof, "#STR_COT_CAMERA_MODULE_FOCUS", 0, 1000, this, "OnChange_Focus", 0, 0.1, "#STR_COT_FORMAT_METRE" );
		m_SliderFocalLength   = CreateEffectSlider( dof, "#STR_COT_CAMERA_MODULE_FOCAL_LENGTH", 0, 1000, this, "OnChange_FocalLength", 0, 0.1, "#STR_COT_FORMAT_METRE" );
		m_SliderFocalNear     = CreateEffectSlider( dof, "#STR_COT_CAMERA_MODULE_FOCAL_NEAR", 0, 1000, this, "OnChange_FocalNear", 0, 0.1, "#STR_COT_FORMAT_METRE" );

		// ---- Post-process --------------------------------------------------
		UIActionCard ppCard = UIActionManager.CreateCard( m_PanelEffects, "#STR_COT_CAMERA_SECTION_POSTPROCESS" );
		Widget pp = ppCard.GetContent();

		m_SliderExposure = CreateEffectSlider( pp, "#STR_COT_CAMERA_MODULE_EXPOSURE", -5, 5, this, "OnChange_Exposure", 0, 0.05, "#STR_COT_FORMAT_NONE" );
		m_SliderVignette = CreateEffectSlider( pp, "#STR_COT_CAMERA_MODULE_VIGNETTE", 0, 1, this, "OnChange_Vignette", 0, 0.01, "#STR_COT_FORMAT_PERCENTAGE" );

		// ---- Shake ---------------------------------------------------------
		UIActionCard shakeCard = UIActionManager.CreateCard( m_PanelEffects, "#STR_COT_CAMERA_SECTION_SHAKE" );
		Widget shake = shakeCard.GetContent();

		m_SliderShakeIntensity = CreateEffectSlider( shake, "#STR_COT_CAMERA_MODULE_SHAKE_INTENSITY", 0, 0.5, this, "OnChange_ShakeIntensity", 0, 0.005, "#STR_COT_FORMAT_NONE" );
		m_SliderShakeFrequency = CreateEffectSlider( shake, "#STR_COT_CAMERA_MODULE_SHAKE_FREQUENCY", 0.1, 10, this, "OnChange_ShakeFrequency", 1.0, 0.1, "#STR_COT_FORMAT_NONE" );

		UIActionButton btnResetFx = UIActionManager.CreateButton( m_PanelEffects, "#STR_COT_CAMERA_RESET", this, "" );
		if ( btnResetFx ) btnResetFx.SetOnClick( this, "OnClick_ResetEffects" );
		btnResetFx.SetIcon( JMConstants.Lucide( "rotate-ccw" ) );
		btnResetFx.SetTooltip( "#STR_COT_CAMERA_RESET_EVERY_SCREEN_EFFECT_BACK_TO" );
	}

	void InitCameraSettings()
	{
		UIActionCard card = UIActionManager.CreateCard( m_PanelSettings, "#STR_COT_CAMERA_SECTION_OPTIONS" );
		Widget col = card.GetContent();

		m_EnableFullmapCamera    = UIActionManager.CreateCheckbox( col, "#STR_COT_CAMERA_MODULE_FULLMAP_UPDATE",  this, "OnClick_EnableFullmap",         m_Module.m_EnableFullmapCamera );
		m_EnableFullmapCamera.SetTooltip( "#STR_COT_CAMERA_UPDATE_THE_FULLSCREEN_MAP_TO_FOLLOW" );
		m_1stPersonADS_HideScope = UIActionManager.CreateCheckbox( col, "#STR_COT_CAMERA_MODULE_HIDE_SCOPE",     this, "OnClick_1stPersonADS_HideScope", GetCurrentCamera1stPersonADSHideScope() );
		m_1stPersonADS_HideScope.SetTooltip( "#STR_COT_CAMERA_HIDE_THE_SCOPE_RETICLE_OVERLAY_WHILE" );
		m_HideGrass              = UIActionManager.CreateCheckbox( col, "#STR_COT_CAMERA_MODULE_HIDE_GRASS",     this, "OnClick_HideGrass",              m_Module.m_HideGrass );
		m_HideGrass.SetTooltip( "#STR_COT_CAMERA_DISABLE_GRASS_RENDERING_SO_DISTANT_OBJEC" );
	}

	void InitCameraBookmarks()
	{
		//! Save and delete act on the list, so they sit in the card's title
		//! bar; Go To is the action the tab exists for and gets the full row.
		UIActionCard card = UIActionManager.CreateCard( m_PanelBookmarks, "#STR_COT_CAMERA_SECTION_BOOKMARKS" );
		Widget col = card.GetContent();

		AddCardClickAction( card, "bookmark-plus", this, "OnClick_SaveBookmark", "#STR_COT_CAMERA_SAVE_THE_CURRENT_CAMERA_POSITION_AS" );
		m_DeleteBookmarkBtn = AddCardDeleteConfirm( card, this, "OnClick_DeleteBookmark", "#STR_COT_CAMERA_REMOVE_THE_SELECTED_BOOKMARK" );

		m_BookmarkNames     = m_Module.GetBookmarkNames();
		m_BookmarkSelectBox = UIActionManager.CreateSelectionBox( col, "#STR_COT_CAMERA_MODULE_BOOKMARKS", m_BookmarkNames, this, "OnClick_BookmarkSelectBox" );
		m_BookmarkSelectBox.SetSelectorWidth(1.0);

		UIActionButton btnGoToBm = UIActionManager.CreateButton( col, "#STR_COT_CAMERA_GO_TO",  this, "" );
		if ( btnGoToBm ) btnGoToBm.SetOnClick( this, "OnClick_TeleportBookmark" );
		btnGoToBm.SetIcon( JMConstants.Lucide( "navigation" ) );
		btnGoToBm.SetColor( JMTheme.SUCCESS_FILL );
		btnGoToBm.SetTooltip( "#STR_COT_CAMERA_TELEPORT_THE_CAMERA_TO_THE_SELECTED" );
	}

	// ----------------------------------------------------------------
	//  Update tick
	// ----------------------------------------------------------------

	override void OnResize( float w, float h )
	{
		super.OnResize( w, h );

		PinRightPanelGeometry( h );

		UpdateTabScrollers();
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

	//! The tab strip's callback. Kept under its old name - it used to be the
	//! section select box's.
	void OnClick_SelectBox( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		Exec_SelectBox();
	}

	//! The strip shows and hides the tab panels itself; what is left is
	//! dismissing popups from the tab being left and sizing the new one.
	void Exec_SelectBox()
	{
		CloseAllOverlays();
		UpdateTabScrollers();
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

		PlayerBase player = PlayerBase.Cast( g_Game.GetPlayer() );

		if ( !player || !player.COTHasAdminNVG() )
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
