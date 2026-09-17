//! "Settings" tab of JMESPForm - display mode, scanning radius/refresh rate,
//! safety limit and colour reset. Back-reference to the owning form, same
//! shape as JMPlayerRowWidget.Menu.
class JMESPFormTabSettings
{
	protected JMESPForm m_Form;

	//! Thinnest line the skeleton renderer can draw. The slider's own minimum is
	//! 0 so the fill previews the thickness proportionally, but the value can
	//! never settle below this.
	static const float MIN_LINE_THICKNESS = 1.0;

	//! Settings rows share one control column so the sliders and dropdowns in
	//! a card line up down a single edge instead of each starting wherever its
	//! own label happened to end.
	static const float SETTING_LABEL_W       = 0.34;
	static const float SETTING_ROW_REMAINDER = 0.64;
	static const float SETTING_CONTROL_W     = 0.62;

	//! Two-state BUTTONS, not switches: each one names the mode it is currently
	//! in, which a switch cannot do - "on" says nothing about whether that
	//! means a classname or a display name.
	protected UIActionButtonToggle m_UseClassName;
	protected UIActionToggleSwitch m_ShowDistance;
	protected UIActionButtonToggle m_DisableSafetyCheckbox;

	protected UIActionToggleSwitch m_AutoRefreshToggle;
	protected UIActionDropdown m_PlayerSkeletons;
	protected UIActionSlider m_SkeletonLineThickness;
	protected UIActionSlider m_RadiusSlider;
	protected UIActionSlider m_RefreshSlider;

	void JMESPFormTabSettings( JMESPForm form )
	{
		m_Form = form;
	}

	void Build( Widget parent )
	{
		UIActionScroller scroller = UIActionManager.CreateScroller( parent );
		Widget container = scroller.GetContentWidget();

		UIActionCard display = UIActionManager.CreateCard( container, "#STR_COT_ESP_MODULE_SECTION_DISPLAY" );
		Widget displayContent = display.GetContent();

		//! The button reads as the mode it is in, so the overlay's labelling is
		//! legible without toggling it to find out.
		Widget labelRow = UIActionManager.CreateWrapSpacer( displayContent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		UIActionText labelModeText = UIActionManager.CreateText( labelRow, "#STR_COT_ESP_MODULE_LABEL_MODE" );
		labelModeText.SetWidth( SETTING_LABEL_W );

		m_UseClassName = UIActionManager.CreateButtonToggle( labelRow, "#STR_COT_ESP_MODULE_LABEL_DISPLAY", "#STR_COT_ESP_MODULE_LABEL_CLASS", this, "OnClick_UseClassName" );
		m_UseClassName.SetWidth( SETTING_ROW_REMAINDER );
		m_UseClassName.SetToggle( JMESPWidgetHandler.UseClassName );
		m_UseClassName.SetTooltip( "#STR_COT_ESP_MODULE_TT_USE_CLASSNAME" );

		//! Distance is the one part of a tag that changes every frame; a wall of
		//! them reads faster with it off.
		m_ShowDistance = UIActionManager.CreateToggleSwitch( displayContent, "#STR_COT_ESP_MODULE_SHOW_DISTANCE", this, "OnClick_ShowDistance", JMESPWidgetHandler.ShowDistance );

		array<string> skeletonOpts = {"#STR_COT_ESP_MODULE_SKELETONS_NONE", "#STR_COT_ESP_MODULE_SKELETONS_OTHERS", "#STR_COT_ESP_MODULE_SKELETONS_ALL"};
		m_PlayerSkeletons = UIActionManager.CreateDropdown( displayContent, "#STR_COT_ESP_MODULE_DRAW_PLAYER_SKELETONS", parent, this, "OnChange_PlayerSkeletons", skeletonOpts );
		m_Form.RegisterOverlay( m_PlayerSkeletons );

		int idx = m_Form.m_Module.GetDrawPlayerSkeletonsEnabled();
		if ( idx )
			idx += m_Form.m_Module.DrawPlayerSkeletonsIncludingMyself;
		m_PlayerSkeletons.SetSelection( idx, false );

		//! A slider, not a dropdown: four steps on a continuum read better as a
		//! continuum, and the fill grows with the thickness it sets.
		//!
		//! The track spans 0..4 so a thickness of 1 reads as a quarter of it
		//! rather than as empty, but 0 is not a thickness anything can be drawn
		//! at, so the handler floors the value at MIN_LINE_THICKNESS.
		m_SkeletonLineThickness = UIActionManager.CreateSlider( displayContent, "#STR_COT_GENERIC_LINE_THICKNESS", 0.0, 4.0, this, "OnChange_Skeleton_LineThickness" );
		m_SkeletonLineThickness.SetCurrent( m_Form.m_Module.SkeletonLineThickness );
		m_SkeletonLineThickness.SetFormat( "#STR_COT_FORMAT_PIXEL" );
		m_SkeletonLineThickness.SetStepValue( 1.0 );
		m_SkeletonLineThickness.SetSliderWidth( SETTING_CONTROL_W );
		ApplyThicknessPreview();

		//! Hidden outright, not greyed: a thickness with no skeleton to apply
		//! to is not a setting the admin can act on, so it takes no room.
		m_SkeletonLineThickness.SetVisible( idx > 0 );

		UIActionCard scanning = UIActionManager.CreateCard( container, "#STR_COT_ESP_MODULE_SECTION_SCANNING" );
		Widget scanContent = scanning.GetContent();

		//! Label as its own text widget rather than the slider's built-in one,
		//! so this row has the identical shape to the Auto Refresh row below.
		Widget radiusRow = UIActionManager.CreateWrapSpacer( scanContent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		UIActionText radiusLabel = UIActionManager.CreateText( radiusRow, "#STR_COT_ESP_MODULE_RADIUS" );
		radiusLabel.SetWidth( SETTING_LABEL_W );

		m_RadiusSlider = UIActionManager.CreateSlider( radiusRow, "", 0, m_Form.m_Module.GetMaxRadius(), this, "OnChange_Range" );
		m_RadiusSlider.SetWidth( SETTING_ROW_REMAINDER );
		m_RadiusSlider.SetCurrent( m_Form.m_Module.ESPRadius );
		m_RadiusSlider.SetFormat( "#STR_COT_FORMAT_METRE_LONG" );
		m_RadiusSlider.SetStepValue( 10.0 );
		m_RadiusSlider.SetSliderWidth( 1.0 );

		//! Auto-refresh sits with the interval it governs rather than in the
		//! toolbar: the switch and the rate are one setting read together.
		Widget refreshRow = UIActionManager.CreateWrapSpacer( scanContent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_AutoRefreshToggle = UIActionManager.CreateToggleSwitch( refreshRow, "#STR_COT_ESP_MODULE_TOGGLE_AUTO_REFRESH", this, "OnClick_UpdateAtRate", m_Form.m_Module.GetState() == JMESPState.Update );
		m_AutoRefreshToggle.SetWidth( SETTING_LABEL_W );
		m_AutoRefreshToggle.SetTooltip( "#STR_COT_ESP_MODULE_TT_AUTO_REFRESH" );

		//! No label of its own: the switch to its left already names the
		//! setting, and a second "Refresh rate:" in the same row only competes
		//! with it. The slider still prints its own formatted value.
		m_RefreshSlider = UIActionManager.CreateSlider( refreshRow, "", 1.0, 10.0, this, "OnChange_UpdateRate" );
		m_RefreshSlider.SetWidth( SETTING_ROW_REMAINDER );
		m_RefreshSlider.SetCurrent( m_Form.m_Module.ESPUpdateTime );
		m_RefreshSlider.SetFormat( "#STR_COT_FORMAT_SECOND_LONG" );
		m_RefreshSlider.SetStepValue( 1.0 );

		//! Fills its own cell, which starts at SETTING_LABEL_W because the
		//! switch beside it is that wide - so the track begins on the same
		//! vertical line as the Radius track above.
		m_RefreshSlider.SetSliderWidth( 1.0 );

		Widget limitRow = UIActionManager.CreateWrapSpacer( scanContent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		UIActionText limitModeText = UIActionManager.CreateText( limitRow, "#STR_COT_ESP_MODULE_LIMIT_MODE" );
		limitModeText.SetWidth( SETTING_LABEL_W );

		m_DisableSafetyCheckbox = UIActionManager.CreateButtonToggle( limitRow, "#STR_COT_ESP_MODULE_LIMIT_SAFE", "#STR_COT_ESP_MODULE_LIMIT_ALL", this, "OnClick_DisableSafety" );
		m_DisableSafetyCheckbox.SetWidth( SETTING_ROW_REMAINDER );
		m_DisableSafetyCheckbox.SetToggle( m_Form.m_Module.GetFilterSafetyState() );
		m_DisableSafetyCheckbox.SetTooltip( "#STR_COT_ESP_MODULE_TT_DISABLE_SAFETY" );

		//! The pill itself carries the warning - it is a button, so SetColor
		//! paints the pill rather than the whole row.
		ApplySafetyStyle();

		UIActionCard colours = UIActionManager.CreateCard( container, "#STR_COT_ESP_MODULE_SECTION_COLOURS" );
		UIActionConfirmInline resetColours = UIActionManager.CreateConfirmInline( colours.GetContent(), "#STR_COT_ESP_MODULE_ACTION_RESET_COLOURS", this, "OnClick_ResetColours" );
		resetColours.SetIcon( JMConstants.Lucide( "rotate-ccw" ) );
		resetColours.SetTooltip( "#STR_COT_ESP_MODULE_TT_RESET_COLOURS" );

		scroller.UpdateScroller();
	}

	//! Whether the toolbar's ESP toggle should turn auto-refresh on when
	//! switching from Remove to an active state.
	bool IsAutoRefreshChecked()
	{
		return m_AutoRefreshToggle && m_AutoRefreshToggle.IsChecked();
	}

	//! Called from the form's 500ms UpdateUI timer.
	void RefreshSliders()
	{
		if ( m_RadiusSlider )  m_RadiusSlider.SetCurrent( m_Form.m_Module.ESPRadius );
		if ( m_RefreshSlider ) m_RefreshSlider.SetCurrent( m_Form.m_Module.ESPUpdateTime );

		if ( m_RefreshSlider )
		{
			//! Enable/disable only. The old code also repainted the track
			//! TEXT_PRIMARY, which is near-white - that is where the washed-out
			//! slider came from, and Disable() already carries the disabled
			//! styling.
			m_RefreshSlider.SetEnabledIf( m_AutoRefreshToggle && m_AutoRefreshToggle.IsChecked() );
		}
	}

	void UpdateMaxRange()
	{
		float maxRadius = m_Form.m_Module.GetMaxRadius();
		if ( m_Form.m_Module.ESPRadius > maxRadius )
			m_Form.m_Module.ESPRadius = maxRadius;
		if ( m_RadiusSlider )
		{
			m_RadiusSlider.SetMinMax( m_RadiusSlider.GetMin(), maxRadius );
			m_RadiusSlider.SetCurrent( m_Form.m_Module.ESPRadius );
		}
	}

	void OnChange_UpdateRate( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Form.m_Module.ESPUpdateTime = action.GetCurrent();
	}

	void OnChange_Range( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Form.m_Module.ESPRadius = action.GetCurrent();
	}

	void OnClick_DisableSafety( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.m_Module.SetFilterSafetyState( m_DisableSafetyCheckbox.IsToggled() );

		ApplySafetyStyle();
	}

	//! Red pill only while the limit is actually off.
	private void ApplySafetyStyle()
	{
		if ( !m_DisableSafetyCheckbox )
			return;

		if ( m_DisableSafetyCheckbox.IsToggled() )
		{
			m_DisableSafetyCheckbox.SetColor( JMTheme.DANGER_FILL );
			return;
		}

		m_DisableSafetyCheckbox.SetColor( JMTheme.SURFACE_RAISED );
	}

	void OnClick_ShowDistance( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMESPWidgetHandler.ShowDistance = m_ShowDistance.IsChecked();
	}

	void OnClick_UseClassName( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMESPWidgetHandler.UseClassName = m_UseClassName.IsToggled();
	}

	void OnClick_UpdateAtRate( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_Form.m_Module.GetState() != JMESPState.Remove )
		{
			if ( m_AutoRefreshToggle.IsChecked() )
			{
				if ( m_RefreshSlider ) m_RefreshSlider.Enable();
				m_Form.m_Module.UpdateState( JMESPState.Update );
			}
			else
			{
				if ( m_RefreshSlider ) m_RefreshSlider.Disable();
				m_Form.m_Module.UpdateState( JMESPState.View );
			}
		}

		m_Form.UpdateUI();
	}

	void OnChange_PlayerSkeletons( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		int idx = action.GetSelection();

		//! Skeletons are drawn off the player view types, so turning them on
		//! while no player category is enabled would draw nothing.
		JMESPViewTypeWidget aiRow;
		JMESPViewTypeWidget playerRow;

		if ( m_Form.m_TabFilters )
		{
			aiRow     = m_Form.m_TabFilters.GetTypeWidget( JMESPViewTypePlayerAI );
			playerRow = m_Form.m_TabFilters.GetTypeWidget( JMESPViewTypePlayer );
		}

		if ( idx > 0 && aiRow && playerRow && !playerRow.IsChecked() && !aiRow.IsChecked() )
			playerRow.SetChecked( true );

		m_Form.m_Module.SetDrawPlayerSkeletonsEnabled( idx > 0 );
		m_Form.m_Module.DrawPlayerSkeletonsIncludingMyself = idx > 1;

		if ( m_SkeletonLineThickness )
			m_SkeletonLineThickness.SetVisible( idx > 0 );

		if ( m_Form.m_TabFilters )
			m_Form.m_TabFilters.RefreshGroupHeaders();
	}

	void OnChange_Skeleton_LineThickness( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( action.GetCurrent() < MIN_LINE_THICKNESS )
			action.SetCurrent( MIN_LINE_THICKNESS );

		m_Form.m_Module.SkeletonLineThickness = action.GetCurrent();

		ApplyThicknessPreview();
	}

	//! Grow the slider's own bar with the thickness it sets: 1px maps to a
	//! quarter of the track, 4px to the whole of it.
	private void ApplyThicknessPreview()
	{
		if ( !m_SkeletonLineThickness )
			return;

		m_SkeletonLineThickness.SetFillThickness( m_SkeletonLineThickness.GetCurrent() / m_SkeletonLineThickness.GetMax() );
	}

	void OnClick_ResetColours( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Form.m_Module.ResetViewTypeColours();

		if ( m_Form.m_TabFilters )
			m_Form.m_TabFilters.RefreshAllSwatches();
	}
}
