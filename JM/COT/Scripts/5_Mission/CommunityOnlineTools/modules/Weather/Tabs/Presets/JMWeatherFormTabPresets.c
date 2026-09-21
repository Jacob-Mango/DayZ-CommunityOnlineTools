//! "Presets" tab of JMWeatherForm - the preset selector, its read-only preview
//! and the dynamic-weather (next states) editor. Back-reference to the owning
//! form, same shape as JMPlayerRowWidget.Menu.
//!
//! ---------------------------------------------------------------------------
//!  Preset edit mode
//!
//!  With a preset picked here, every other tab edits THAT preset instead of the
//!  live world: the controls are loaded from it, the live poll stops writing
//!  over them, and Apply becomes a local preview. This tab owns the selection
//!  (m_SelectedPreset / m_IsCreatingPreset); the form asks it through
//!  IsPresetMode() / GetSelectedPreset() / IsCreatingPreset() and, until the tab
//!  has been opened, there is no selection and the answer is "live world".
//! ---------------------------------------------------------------------------
class JMWeatherFormTabPresets: JMFormTab
{
	protected JMWeatherForm m_Form;
	protected UIActionScroller m_ScrollerPresets;
	protected UIActionDropdown      m_SelectPreset;
	protected UIActionEditableText  m_EditPresetName;
	protected UIActionButton        m_ButtonPresetUse;
	protected UIActionButton        m_ButtonPresetSave;
	protected UIActionConfirmInline m_ButtonPresetRemove;

	//! What the selected preset will actually write when it is applied.
	//!
	//! The numeric rows are sliders so the values can be compared at a glance
	//! rather than read one by one; a preset stores -1 in every field it does
	//! not own, and those rows sit at zero and say so in their value text.
	protected ref UIActionCard m_PreviewCard;
	protected UIActionText   m_PreviewName;
	protected UIActionSlider m_PreviewOvercast;
	protected UIActionSlider m_PreviewFog;
	protected UIActionSlider m_PreviewRain;
	protected UIActionSlider m_PreviewSnow;
	protected UIActionSlider m_PreviewWind;
	protected UIActionSlider m_PreviewStorm;
	protected UIActionText   m_PreviewDate;
	protected UIActionText   m_PreviewTime;

	//! The fade and hold the preset applies with when it is used for real.
	protected UIActionText m_PreviewTransition;
	protected UIActionText m_PreviewDuration;

	// --- Dynamic weather ------------------------------------------------------
	//! The master switch is global and lives on its own card, so it can be
	//! flipped with no preset selected. It saves the moment it is changed -
	//! there is nothing else on that card to batch it with.
	protected UIActionToggleSwitch m_ToggleDynamic;

	//! Everything below belongs to the SELECTED preset, and the card holding it
	//! is hidden while there is nothing selected to speak for.
	protected ref UIActionCard m_NextStatesCard;
	protected UIActionText         m_TextDynamicScope;
	protected UIActionToggleSwitch m_ToggleInRotation;

	//! How long this preset holds and how long it takes to fade in, both as
	//! ranges the server draws from when it enters this preset.
	protected UIActionSliderRange m_RangeDuration;
	protected UIActionSliderRange m_RangeTransition;
	protected UIActionText   m_TextChanceTotal;
	protected UIActionButton m_ButtonDynamicSave;

	//! Next states are added one at a time from this dropdown rather than
	//! every preset being listed up front - a server with twenty presets would
	//! otherwise show nineteen sliders sitting at zero.
	protected UIActionDropdown m_DropAddState;
	protected UIActionButton   m_ButtonAddState;

	//! Names behind rows 1..n of the add dropdown. Row 0 is the prompt.
	protected autoptr TStringArray m_AddOptions;

	//! The chance rows are per selected preset and the preset list changes at
	//! runtime, so they are torn down and rebuilt rather than pre-allocated -
	//! a hidden child still owns its cell in a GridSpacer. m_ChanceHost stays
	//! put so the rows keep their place in the card; m_ChanceGrid is the part
	//! that gets replaced.
	protected Widget m_ChanceHost;
	protected Widget m_ChanceGrid;

	//! The chance rows are held as data, not read back off the widgets: they
	//! are destroyed and rebuilt whenever a row is added or removed, and a
	//! rebalance has to survive that.
	protected autoptr TStringArray m_ChanceTargets;
	protected autoptr array<int>   m_ChanceValues;
	protected autoptr array< ref UIActionSlider >      m_ChanceSliders;
	protected autoptr array< ref UIActionImageButton > m_ChanceDeletes;
	protected bool   m_IsCreatingPreset;
	protected string m_SelectedPreset;
	protected string m_RemovePreset;

	//! Fixed rows at the head of the preset selector.
	static const int ROW_LIVE_WORLD   = 0;
	static const int ROW_CREATE_NEW   = 1;
	static const int ROW_FIRST_PRESET = 2;

	//! Tops of the two dynamic timing ranges. A hold is edited in minutes and a
	//! fade in seconds, which is how each reads best.
	static const int DURATION_MAX_MINUTES   = 120;
	static const int TRANSITION_MAX_SECONDS = 600;

	//! Row splits, as fractions of the row. Each leaves more than enough room
	//! for the fixed-size control beside it: the chance row reserves a tenth
	//! for a 28px delete button, the add row a quarter for a 90px button.
	static const float CHANCE_SLIDER_W = 0.9;
	static const float ADD_DROPDOWN_W  = 0.75;
	static const int ADD_BUTTON_W = 90;
	static const int ADD_BUTTON_H = 30;

	void JMWeatherFormTabPresets( JMWeatherForm form )
	{
		m_Form = form;

		m_ChanceTargets = new TStringArray;
		m_ChanceValues  = new array<int>;
		m_ChanceSliders = new array< ref UIActionSlider >;
		m_ChanceDeletes = new array< ref UIActionImageButton >;
		m_AddOptions    = new TStringArray;
	}

	//! What the name field currently says - the name a Save Preset writes under.
	string GetEditedName()
	{
		if ( m_EditPresetName )
			return m_EditPresetName.GetText();

		return "";
	}

	string GetSelectedPreset()
	{
		return m_SelectedPreset;
	}

	bool IsCreatingPreset()
	{
		return m_IsCreatingPreset;
	}

	// -------------------------------------------------------------------------
	//  Selection state - read by the form and the other tabs
	// -------------------------------------------------------------------------

	bool IsPresetMode()
	{
		return ( m_SelectedPreset != "" || m_IsCreatingPreset );
	}

	protected void SetChanceTotalText( string key )
	{
		if ( m_TextChanceTotal )
			m_TextChanceTotal.SetText( Widget.TranslateString( key ) );
	}

	protected void SetPreviewPercent( UIActionSlider slider, float forecast )
	{
		SetPreviewSlider( slider, PreviewBarValue( forecast, 100.0 ), PreviewPercent( forecast ) );
	}

	//! SetCurrent repaints the value text from the slider format, so the custom
	//! text always has to be written afterwards.
	protected void SetPreviewSlider( UIActionSlider slider, float value, string text )
	{
		if ( !slider )
			return;

		slider.SetCurrent( value );
		slider.SetText( text );
	}

	protected void SetPreviewText( UIActionText action, string text )
	{
		if ( action )
			action.SetText( text );
	}

	void SetSelectedPreset( string preset )
	{
		m_IsCreatingPreset = false;
		m_SelectedPreset   = preset;

		if ( m_EditPresetName )
			m_EditPresetName.SetText( preset );

		UpdatePresetPreview();
		RefreshDynamicControls();

		m_Form.LoadEditorValues();
		m_Form.UpdateModeBanner();

		//! Straight after the controls are loaded, never before: the preview is
		//! built by reading them back.
		PreviewPreset();

		m_Form.UpdateStates();
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		m_ScrollerPresets = UIActionManager.CreateScroller( panel );
		Widget content = m_ScrollerPresets.GetContentWidget();

		UIActionCard manageCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_TAB_PRESETS" );
		Widget manageBody = UIActionManager.CreateGridSpacer( manageCard.GetContent(), 3, 1 );

		//! One dropdown, rather than the list of row widgets this used to
		//! build. Those were all created up front and hidden one by one, but a
		//! hidden child still owns its cell in a GridSpacer - so the saved
		//! presets drew scattered down the tab with gaps between them, and the
		//! tab scrolled for a hundred rows that were never filled.
		//!
		//! The popup is anchored to the form's root, not to the card, so it draws
		//! over the tab instead of being clipped by the scroller it lives in.
		m_SelectPreset = UIActionManager.CreateDropdown( manageBody, "#STR_COT_WEATHER_PRESET_LIST", m_Form.GetLayoutRoot(), this, "OnChange_PresetSelect", BuildPresetOptions() );
		m_SelectPreset.SetTooltip( "#STR_COT_WEATHER_PRESET_SELECT_DESC" );
		m_Form.AddOverlay( m_SelectPreset );

		m_EditPresetName = UIActionManager.CreateEditableText( manageBody, "#STR_COT_GENERIC_NAME", this );
		m_EditPresetName.SetOnlyNumbers( false );
		m_EditPresetName.SetText( "" );

		Widget actionRow = UIActionManager.CreateGridSpacer( manageBody, 1, 3 );
		m_ButtonPresetUse    = UIActionManager.CreateButton( actionRow, "#STR_COT_WEATHER_MODULE_PRESET_APPLY", this, "OnClick_PresetRun" );
		m_ButtonPresetSave   = UIActionManager.CreateButton( actionRow, "#STR_COT_WEATHER_MODULE_PRESET_SAVE", this, "" );
		if ( m_ButtonPresetSave ) m_ButtonPresetSave.SetOnClick( this, "OnClick_PresetUpdate" );
		m_ButtonPresetRemove = UIActionManager.CreateConfirmInline( actionRow, "#STR_COT_WEATHER_MODULE_PRESET_DELETE", this, "OnClick_PresetRemove" );
		UIActionIconGrid.ApplyDeletePreset( m_ButtonPresetRemove );

		// --- Preview ----------------------------------------------------------
		//! Hidden until a preset is selected. With nothing chosen the card had
		//! nothing to say and said it in nine blank rows.
		m_PreviewCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_PRESET_PREVIEW" );
		Widget previewBody = UIActionManager.CreateGridSpacer( m_PreviewCard.GetContent(), 11, 1 );

		m_PreviewName = UIActionManager.CreateText( previewBody, "#STR_COT_GENERIC_NAME", "" );

		m_PreviewOvercast = CreatePreviewSlider( previewBody, "#STR_COT_WEATHER_MODULE_OVERCAST", 100 );
		m_PreviewFog      = CreatePreviewSlider( previewBody, "#STR_COT_WEATHER_MODULE_FOG", 100 );
		m_PreviewRain     = CreatePreviewSlider( previewBody, "#STR_COT_WEATHER_MODULE_RAIN", 100 );
		m_PreviewSnow     = CreatePreviewSlider( previewBody, "#STR_COT_WEATHER_MODULE_SNOW", 100 );
		m_PreviewWind     = CreatePreviewSlider( previewBody, "#STR_COT_WEATHER_MODULE_WIND", JMWeatherForm.WIND_SCALE_MAX );
		m_PreviewStorm    = CreatePreviewSlider( previewBody, "#STR_COT_WEATHER_MODULE_STORM", 100 );

		m_PreviewDate = UIActionManager.CreateText( previewBody, "#STR_COT_GENERIC_DATE", "" );
		m_PreviewTime = UIActionManager.CreateText( previewBody, "#STR_COT_WEATHER_MODULE_START_TIME", "" );

		m_PreviewTransition = UIActionManager.CreateText( previewBody, "#STR_COT_GENERIC_TRANSITION", "" );
		m_PreviewDuration   = UIActionManager.CreateText( previewBody, "#STR_COT_GENERIC_DURATION", "" );

		// --- Dynamic weather --------------------------------------------------
		//! The master switch, on its own card. It is global, so it must stay
		//! reachable with no preset selected - which is exactly when the card
		//! below it is hidden.
		UIActionCard dynamicCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_DYNAMIC" );

		m_ToggleDynamic = UIActionManager.CreateToggleSwitch( dynamicCard.GetContent(), "#STR_COT_WEATHER_DYNAMIC_ENABLE", this, "OnChange_DynamicEnabled" );
		m_ToggleDynamic.SetTooltip( "#STR_COT_WEATHER_DYNAMIC_ENABLE_DESC" );

		//! Everything from here belongs to the selected preset.
		m_NextStatesCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_DYNAMIC_NEXT_CARD" );
		Widget dynamicBody = UIActionManager.CreateGridSpacer( m_NextStatesCard.GetContent(), 5, 1 );

		m_TextDynamicScope = UIActionManager.CreateText( dynamicBody, "#STR_COT_WEATHER_DYNAMIC_SCOPE", "" );

		m_ToggleInRotation = UIActionManager.CreateToggleSwitch( dynamicBody, "#STR_COT_WEATHER_DYNAMIC_ROTATION", this );
		m_ToggleInRotation.SetTooltip( "#STR_COT_WEATHER_DYNAMIC_ROTATION_DESC" );

		//! Both are ranges, and both are edited in the unit they read best in:
		//! a hold is minutes, a fade is seconds.
		m_RangeDuration = UIActionManager.CreateSliderRange( dynamicBody, "#STR_COT_WEATHER_DYNAMIC_DURATION", 1, DURATION_MAX_MINUTES );
		m_RangeDuration.SetFormat( "#STR_COT_FORMAT_MINUTE" );
		m_RangeDuration.SetStep( 1 );
		m_RangeDuration.SetTooltip( "#STR_COT_WEATHER_DYNAMIC_DURATION_DESC" );

		m_RangeTransition = UIActionManager.CreateSliderRange( dynamicBody, "#STR_COT_WEATHER_DYNAMIC_TRANSITION", 0, TRANSITION_MAX_SECONDS );
		m_RangeTransition.SetFormat( "#STR_COT_FORMAT_SECOND" );
		m_RangeTransition.SetStep( 10 );
		m_RangeTransition.SetTooltip( "#STR_COT_WEATHER_DYNAMIC_TRANSITION_DESC" );

		m_TextChanceTotal = UIActionManager.CreateText( dynamicBody, "#STR_COT_WEATHER_DYNAMIC_CHANCES", "" );
		m_TextChanceTotal.SetTooltip( "#STR_COT_WEATHER_DYNAMIC_CHANCES_DESC" );

		//! Widths are FRACTIONS of the row, not a measured flex split.
		//!
		//! UIActionFlexRow divides a row it has measured, and it cannot measure
		//! one that has not been drawn - which is the state every row is in on
		//! the frame it is built, and the state a rebuilt row returns to. When
		//! the pass had not taken, the child kept its authored full-row width
		//! and the button beside it wrapped onto a line of its own. Retrying
		//! the pass only narrowed the window; it did not close it.
		//!
		//! A fraction plus a fixed-size button needs no measurement at all, so
		//! it cannot be wrong on the first frame or at any resolution. Both
		//! scale together, so the reserve holds as the UI scales.
		Widget addRow = UIActionManager.CreateWrapSpacerCompact( m_NextStatesCard.GetContent(), WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_DropAddState = UIActionManager.CreateDropdown( addRow, "", m_Form.GetLayoutRoot(), this, "" );
		m_DropAddState.SetWidth( ADD_DROPDOWN_W );
		m_Form.AddOverlay( m_DropAddState );

		m_ButtonAddState = UIActionManager.CreateButton( addRow, "#STR_COT_WEATHER_DYNAMIC_ADD", this, "OnClick_AddState" );
		m_ButtonAddState.SetFixedSize( ADD_BUTTON_W, ADD_BUTTON_H );

		m_ChanceHost = UIActionManager.CreateGridSpacer( m_NextStatesCard.GetContent(), 1, 1 );

		m_ButtonDynamicSave = UIActionManager.CreateButton( m_NextStatesCard.GetContent(), "#STR_COT_WEATHER_DYNAMIC_SAVE", this, "OnClick_DynamicSave" );
		m_ButtonDynamicSave.SetColor( JMTheme.SUCCESS_FILL );
		m_ButtonDynamicSave.SetTooltip( "#STR_COT_WEATHER_DYNAMIC_SAVE_DESC" );

		UpdatePresetList();
		RefreshDynamicControls();

		m_ScrollerPresets.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_ScrollerPresets )
			m_ScrollerPresets.UpdateScroller();
	}

	//! A read-only slider.
	//!
	//! Disabling it would be the obvious way to say "not editable", but a
	//! disabled UIAction is painted under a 90%-opaque panel and the value
	//! would be unreadable. Instead it takes a callback that repaints the
	//! whole preview from the stored preset, so a drag snaps back the instant
	//! it starts and the control reads as output rather than as input.
	protected UIActionSlider CreatePreviewSlider( Widget parent, string label, float max )
	{
		UIActionSlider slider = UIActionManager.CreateSlider( parent, label, 0, max, this, "OnChange_PreviewLocked" );
		slider.SetStepValue( 1 );
		return slider;
	}

	//! The stored preset the tabs are editing. NULL while authoring a new one -
	//! there is nothing saved to load, so the world stays as the starting
	//! point - and NULL when not in preset mode at all.
	JMWeatherPreset EditingPreset()
	{
		if ( m_SelectedPreset == "" || !m_Form.m_Module || !m_Form.m_Module.HasSettings() )
			return NULL;

		return m_Form.m_Module.GetPreset( m_SelectedPreset );
	}

	//! Two fixed rows lead the list - live world, then create new - so a
	//! selector index is ROW_FIRST_PRESET ahead of the index into GetPresets().
	//!
	//! Live world is a row of its own rather than an absence of selection so
	//! the dropdown can never disagree with the banner about which mode the
	//! tabs are in.
	protected array<string> BuildPresetOptions()
	{
		array<string> options = new array<string>;

		//! UIActionDropdown prints entry text verbatim - unlike its label, it
		//! does not translate - so the stringtable keys are resolved here.
		options.Insert( Widget.TranslateString( "#STR_COT_WEATHER_PRESET_LIVE" ) );
		options.Insert( Widget.TranslateString( "#STR_COT_WEATHER_PRESET_NEW" ) );

		if ( !m_Form.m_Module || !m_Form.m_Module.HasSettings() )
			return options;

		array< ref JMWeatherPreset > presets = m_Form.m_Module.GetPresets();

		foreach ( JMWeatherPreset preset : presets )
			options.Insert( preset.Name );

		return options;
	}

	//! Which selector row the form's current state corresponds to.
	protected int SelectorRowFor()
	{
		if ( m_IsCreatingPreset )
			return ROW_CREATE_NEW;

		int index = PresetIndex( m_SelectedPreset );

		if ( index < 0 )
			return ROW_LIVE_WORLD;

		return index + ROW_FIRST_PRESET;
	}

	protected int PresetIndex( string name )
	{
		if ( name == "" || !m_Form.m_Module || !m_Form.m_Module.HasSettings() )
			return -1;

		array< ref JMWeatherPreset > presets = m_Form.m_Module.GetPresets();

		for ( int i = 0; i < presets.Count(); i++ )
		{
			if ( presets[i].Name == name )
				return i;
		}

		return -1;
	}

	protected JMWeatherPreset FindPreset( string name )
	{
		int index = PresetIndex( name );

		if ( index < 0 )
			return NULL;

		return m_Form.m_Module.GetPresets()[index];
	}

	// -------------------------------------------------------------------------
	//  Preset edit mode transitions
	// -------------------------------------------------------------------------

	//! Back to the live world: nothing selected, nothing being created, and the
	//! tabs reloaded from the server rather than left showing a preset that is
	//! no longer being edited.
	void ExitPresetMode()
	{
		m_IsCreatingPreset = false;
		m_SelectedPreset   = "";

		if ( m_EditPresetName )
			m_EditPresetName.SetText( "" );

		if ( m_SelectPreset )
			m_SelectPreset.SetSelection( ROW_LIVE_WORLD, false );

		UpdatePresetPreview();
		RefreshDynamicControls();

		m_Form.LoadEditorValues();
		m_Form.UpdateModeBanner();

		m_Form.UpdateStates();
	}

	void CreateNew()
	{
		m_IsCreatingPreset = true;
		m_SelectedPreset   = "";

		if ( m_EditPresetName )
			m_EditPresetName.SetText( "" );

		UpdatePresetPreview();
		RefreshDynamicControls();

		m_Form.LoadEditorValues();
		m_Form.UpdateModeBanner();

		m_Form.UpdateStates();
	}

	//! Show the selected preset on THIS CLIENT, immediately.
	//!
	//! Picking a preset is asking what it looks like, and answering that with a
	//! panel of numbers when the sky is right there is the long way round. It
	//! goes on locally, so nobody else's weather moves, and with the fades
	//! stripped, so it is the preset's END state rather than a slow drift
	//! toward it - PreviewSection does both.
	//!
	//! Weather only. The date and time are left alone: a preset that carries a
	//! time would otherwise throw this client's clock across the day just for
	//! being looked at, and time is not what "what does this weather look
	//! like" is asking.
	//!
	//! The server owns the weather and syncs it, so this lasts until the next
	//! sync - the right lifetime for a preview.
	protected void PreviewPreset()
	{
		if ( !IsPresetMode() )
			return;

		JMWeatherFormTabSky           sky           = m_Form.m_TabSkyCtrl;
		JMWeatherFormTabPrecipitation precipitation = m_Form.m_TabPrecipitationCtrl;
		JMWeatherFormTabWind          wind          = m_Form.m_TabWindCtrl;

		if ( sky )
		{
			sky.ApplyOvercast();
			sky.ApplyFog();
			sky.ApplyStorm();
		}

		if ( precipitation )
		{
			precipitation.ApplyRain();
			precipitation.ApplySnow();
		}

		if ( wind )
		{
			wind.ApplyWind();
			wind.ApplyWindFunction();
		}
	}

	//! Write the weather tabs into the named preset. Shared by the Presets tab
	//! button and the one in the banner, which have to mean the same thing.
	void SavePreset()
	{
		if ( !m_Form.m_Module )
			return;

		JMWeatherPreset preset = new JMWeatherPreset;
		m_Form.GetUIActionValues( preset );

		if ( preset.Name == "" )
			return;

		if ( m_IsCreatingPreset )
			m_Form.m_Module.CreatePreset( preset );
		else
			m_Form.m_Module.UpdatePreset( preset );

		m_IsCreatingPreset = false;
		m_SelectedPreset   = preset.Name;

		m_Form.UpdateModeBanner();
		m_Form.UpdateStates();
	}

	//! Asks for confirmation; the answer arrives on the form's RemovePreset_Yes
	//! (JMConfirmation callbacks are looked up on the form), which forwards to
	//! RemovePresetConfirmed.
	void RemovePreset( string preset )
	{
		m_RemovePreset = preset;

		m_Form.ConfirmAction( "#STR_COT_GENERIC_CONFIRM", "#STR_COT_WEATHER_PRESET_REMOVE_CONFIRM", "RemovePreset_Yes", "#STR_COT_GENERIC_YES", "#STR_COT_GENERIC_NO" );
	}

	void RemovePresetConfirmed()
	{
		if ( m_SelectedPreset == m_RemovePreset )
			m_SelectedPreset = "";

		if ( m_Form.m_Module )
			m_Form.m_Module.RemovePreset( m_RemovePreset );

		m_Form.UpdateStates();
	}

	void UpdatePresetList()
	{
		if ( !m_Form.m_Module || !m_Form.m_Module.HasSettings() )
			return;

		if ( m_SelectPreset )
		{
			m_SelectPreset.SetItems( BuildPresetOptions() );

			//! Rebuilding the options resets the selector to row 0, so the
			//! selection is restored by NAME. Restoring an index would follow
			//! whichever preset slid into that slot after a delete, and the
			//! event is suppressed so the restore does not re-enter
			//! SetSelectedPreset and clobber the edit field.
			m_SelectPreset.SetSelection( SelectorRowFor(), false );
		}

		UpdatePresetPreview();
		RefreshDynamicControls();

		if ( m_ScrollerPresets )
			m_ScrollerPresets.UpdateScroller();
	}

	// -------------------------------------------------------------------------
	//  Preview card
	// -------------------------------------------------------------------------

	//! Spell out what applying the selected preset would actually write.
	protected void UpdatePresetPreview()
	{
		if ( !m_PreviewName )
			return;

		JMWeatherPreset preset = FindPreset( m_SelectedPreset );

		//! Nothing selected - "Create new" - means there is no stored preset to
		//! describe, so the card goes away rather than standing there empty.
		if ( m_PreviewCard )
			m_PreviewCard.SetVisible( preset != NULL );

		if ( !preset )
			return;

		m_PreviewName.SetText( preset.Name );

		SetPreviewPercent( m_PreviewOvercast, preset.POvercast.Forecast );
		SetPreviewPercent( m_PreviewFog, preset.PFog.Forecast );
		SetPreviewPercent( m_PreviewRain, preset.PRain.Forecast );
		SetPreviewPercent( m_PreviewSnow, preset.PSnow.Forecast );

		//! Wind and storm each hold more than one number, so the bar tracks the
		//! headline value and the value text spells the rest out.
		SetPreviewSlider( m_PreviewWind, PreviewBarValue( preset.PWindMagnitude.Forecast, 1.0 ), PreviewWind( preset ) );
		SetPreviewSlider( m_PreviewStorm, PreviewBarValue( preset.Storm.Density, 100.0 ), PreviewStorm( preset ) );

		SetPreviewText( m_PreviewDate, PreviewDate( preset ) );
		SetPreviewText( m_PreviewTime, PreviewTime( preset ) );

		//! Stored on the phenomena rather than in a field of their own, so they
		//! are read back off overcast - the same place LoadEditorValues takes
		//! them from when it fills the Overview pickers.
		SetPreviewText( m_PreviewTransition, PreviewSeconds( preset.POvercast.Time ) );
		SetPreviewText( m_PreviewDuration, PreviewSeconds( preset.POvercast.MinDuration ) );
	}

	protected string PreviewSeconds( float seconds )
	{
		if ( seconds == -1 )
			return Widget.TranslateString( "#STR_COT_GENERIC_UNCHANGED" );

		return string.Format( Widget.TranslateString( "#STR_COT_FORMAT_SECOND" ), Math.Round( seconds ) );
	}

	//! An unset field has no bar to draw - the value text carries the meaning.
	protected float PreviewBarValue( float forecast, float scale )
	{
		if ( forecast == -1 )
			return 0;

		return forecast * scale;
	}

	void OnChange_PreviewLocked( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdatePresetPreview();
	}

	//! Every preset field carries -1 when the preset does not own it, and
	//! JMWeatherPreset skips those on Apply - so the preview says "unchanged"
	//! rather than printing a number that will never be written.
	protected string PreviewPercent( float forecast )
	{
		if ( forecast == -1 )
			return Widget.TranslateString( "#STR_COT_GENERIC_UNCHANGED" );

		return string.Format( Widget.TranslateString( "#STR_COT_FORMAT_PERCENTAGE" ), Math.Round( forecast * 100.0 ) );
	}

	protected string PreviewWind( JMWeatherPreset preset )
	{
		if ( preset.PWindMagnitude.Forecast == -1 )
			return Widget.TranslateString( "#STR_COT_GENERIC_UNCHANGED" );

		string text = string.Format( Widget.TranslateString( "#STR_COT_FORMAT_MPS" ), Math.Round( preset.PWindMagnitude.Forecast ) );

		//! Direction is radians in <-PI,+PI>, which means a genuine heading of
		//! -1 rad is indistinguishable from the "unchanged" marker. That is a
		//! property of the stored preset format, not of this readout: such a
		//! preset does not apply its direction either.
		if ( preset.PWindDirection.Forecast != -1 )
			text += "  " + JMWeatherFormTabWind.CardinalFor( JMWeatherFormTabWind.PI2DEG( preset.PWindDirection.Forecast ) );

		return text;
	}

	protected string PreviewStorm( JMWeatherPreset preset )
	{
		if ( preset.Storm.Density == -1 )
			return Widget.TranslateString( "#STR_COT_GENERIC_UNCHANGED" );

		string density   = string.Format( Widget.TranslateString( "#STR_COT_FORMAT_PERCENTAGE" ), Math.Round( preset.Storm.Density * 100.0 ) );
		string threshold = string.Format( Widget.TranslateString( "#STR_COT_FORMAT_PERCENTAGE" ), Math.Round( preset.Storm.Threshold * 100.0 ) );
		string interval  = string.Format( Widget.TranslateString( "#STR_COT_FORMAT_SECOND" ), Math.Round( preset.Storm.MinTimeBetweenLightning ) );

		return density + " / " + threshold + " / " + interval;
	}

	//! Date and time are one field to the engine - JMWeatherDate applies all
	//! five components or none, gated on Year - so both rows follow Year.
	protected string PreviewDate( JMWeatherPreset preset )
	{
		if ( preset.PDate.Year == -1 )
			return Widget.TranslateString( "#STR_COT_GENERIC_UNCHANGED" );

		return preset.PDate.Day.ToString() + "/" + preset.PDate.Month.ToString() + "/" + preset.PDate.Year.ToString();
	}

	protected string PreviewTime( JMWeatherPreset preset )
	{
		if ( preset.PDate.Year == -1 )
			return Widget.TranslateString( "#STR_COT_GENERIC_UNCHANGED" );

		return JMWeatherFormTabTime.FormatClock( preset.PDate.Hour, preset.PDate.Minute );
	}

	// -------------------------------------------------------------------------
	//  Dynamic weather
	// -------------------------------------------------------------------------

	void RefreshDynamicControls()
	{
		if ( !m_ToggleDynamic || !m_Form.m_Module || !m_Form.m_Module.HasSettings() )
			return;

		m_ToggleDynamic.SetChecked( m_Form.m_Module.IsDynamicEnabled() );

		JMWeatherPreset preset = m_Form.m_Module.GetPreset( m_SelectedPreset );

		//! Nothing selected means there is no preset to configure, so the whole
		//! per-preset card goes away rather than showing another preset's
		//! numbers or a row of defaults nothing would save.
		if ( m_NextStatesCard )
			m_NextStatesCard.SetVisible( preset != NULL );

		if ( preset )
		{
			if ( m_TextDynamicScope )
				m_TextDynamicScope.SetText( preset.Name );

			if ( m_ToggleInRotation )
				m_ToggleInRotation.SetChecked( preset.InRotation );

			if ( m_RangeDuration )
				m_RangeDuration.SetRange( SecondsToMinutes( preset.DurationMin ), SecondsToMinutes( preset.DurationMax ) );

			if ( m_RangeTransition )
				m_RangeTransition.SetRange( preset.TransitionMin, preset.TransitionMax );
		}

		LoadChanceModel();
		BuildChanceRows();
	}

	void OnChange_DynamicEnabled( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || !m_Form.m_Module || !m_ToggleDynamic )
			return;

		//! The master switch is global and sits alone on its card, so there is
		//! no Save to batch it into. An empty PresetName sends it by itself and
		//! leaves every preset's own settings alone.
		JMWeatherDynamicConfig config = new JMWeatherDynamicConfig;
		config.Enabled = m_ToggleDynamic.IsChecked();

		m_Form.m_Module.SetDynamicWeather( config );
	}

	//! Whole minutes, never zero - a hold of "0 minutes" is not a thing the
	//! chain can honour, and the slider starts at 1.
	protected int SecondsToMinutes( int seconds )
	{
		int minutes = seconds / 60;

		if ( minutes < 1 )
			minutes = 1;

		return minutes;
	}

	//! Read the selected preset's own next states.
	protected void LoadChanceModel()
	{
		m_ChanceTargets.Clear();
		m_ChanceValues.Clear();

		if ( !m_Form.m_Module || !m_Form.m_Module.HasSettings() )
			return;

		JMWeatherPreset preset = m_Form.m_Module.GetPreset( m_SelectedPreset );

		if ( !preset || !preset.NextStates )
			return;

		for ( int i = 0; i < preset.NextStates.Count(); i++ )
		{
			//! A candidate naming a preset that has since been deleted has
			//! nothing to draw a row for.
			if ( !m_Form.m_Module.HasPreset( preset.NextStates[i].To ) )
				continue;

			m_ChanceTargets.Insert( preset.NextStates[i].To );
			m_ChanceValues.Insert( preset.NextStates[i].Chance );
		}
	}

	//! One slider plus a delete button per state the admin has added.
	protected void BuildChanceRows()
	{
		if ( !m_ChanceHost )
			return;

		if ( m_ChanceGrid )
		{
			m_ChanceGrid.Unlink();
			m_ChanceGrid = NULL;
		}

		m_ChanceSliders.Clear();
		m_ChanceDeletes.Clear();

		int count = m_ChanceTargets.Count();

		if ( count > 0 )
		{
			m_ChanceGrid = UIActionManager.CreateGridSpacer( m_ChanceHost, count, 1 );

			UIActionSlider slider;
			UIActionImageButton remove;
			Widget row;

			for ( int i = 0; i < count; i++ )
			{
				//! Compact: margin and padding are both zero, so the children
				//! add up to exactly their own widths and nothing has to be
				//! held back for spacing between them.
				row = UIActionManager.CreateWrapSpacerCompact( m_ChanceGrid, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

				//! Delete first, so it sits ahead of the name it removes - a
				//! wrap spacer places children in creation order.
				remove = UIActionManager.CreateDeleteButton( row, this, "" );
				if ( remove ) remove.SetOnClick( this, "OnClick_RemoveState" );

				slider = UIActionManager.CreateSlider( row, m_ChanceTargets[i], 0, 100, this, "OnChange_Chance" );
				slider.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
				slider.SetStepValue( 1 );
				slider.SetCurrent( m_ChanceValues[i] );

				//! See the note on the add row: a fraction that leaves room for
				//! the fixed button beside it, rather than a measured split.
				slider.SetWidth( CHANCE_SLIDER_W );

				m_ChanceSliders.Insert( slider );
				m_ChanceDeletes.Insert( remove );
			}
		}

		RebuildAddOptions();
		UpdateChanceTotal();
		m_Form.UpdateStates();

		if ( m_ScrollerPresets )
			m_ScrollerPresets.UpdateScroller();
	}

	//! The add dropdown only offers states that are not already listed.
	protected void RebuildAddOptions()
	{
		m_AddOptions.Clear();

		array<string> options = new array<string>;
		options.Insert( Widget.TranslateString( "#STR_COT_WEATHER_DYNAMIC_ADD_HINT" ) );

		if ( m_Form.m_Module && m_Form.m_Module.HasSettings() && m_SelectedPreset != "" )
		{
			array< ref JMWeatherPreset > presets = m_Form.m_Module.GetPresets();

			for ( int i = 0; i < presets.Count(); i++ )
			{
				if ( m_ChanceTargets.Find( presets[i].Name ) != -1 )
					continue;

				m_AddOptions.Insert( presets[i].Name );
				options.Insert( presets[i].Name );
			}
		}

		if ( m_DropAddState )
		{
			m_DropAddState.SetItems( options );
			m_DropAddState.SetSelection( 0, false );
		}
	}

	protected void UpdateChanceTotal()
	{
		if ( !m_TextChanceTotal )
			return;

		if ( m_SelectedPreset == "" )
		{
			SetChanceTotalText( "#STR_COT_WEATHER_DYNAMIC_CHANCES_NONE" );
			return;
		}

		if ( m_ChanceTargets.Count() == 0 )
		{
			SetChanceTotalText( "#STR_COT_WEATHER_DYNAMIC_TOTAL_NONE" );
			return;
		}

		m_TextChanceTotal.SetText( string.Format( Widget.TranslateString( "#STR_COT_WEATHER_DYNAMIC_COUNT" ), m_ChanceTargets.Count() ) );
	}

	//! Repaint the sliders from the model after a rebalance. SetCurrent does
	//! not raise CHANGE, so this cannot re-enter the handler that called it.
	protected void PaintChanceSliders()
	{
		for ( int i = 0; i < m_ChanceSliders.Count(); i++ )
			m_ChanceSliders[i].SetCurrent( m_ChanceValues[i] );
	}

	//! The chances always add up to 100, so what is on screen is the real odds
	//! rather than a weight that only means something next to a total the admin
	//! has to add up themselves.
	//!
	//! The row that was just moved keeps its value and every other row is
	//! scaled into what is left. Rounding drift is pushed onto the last scaled
	//! row, so the column reads exactly 100 however the shares divide.
	protected void BalanceChances( int keep )
	{
		int count = m_ChanceValues.Count();

		if ( count == 0 )
			return;

		if ( count == 1 )
		{
			m_ChanceValues[0] = 100;
			return;
		}

		if ( keep < 0 || keep >= count )
		{
			NormalizeChances();
			return;
		}

		int i;
		int share;

		int kept = Math.Clamp( m_ChanceValues[keep], 0, 100 );
		m_ChanceValues[keep] = kept;

		int remainder   = 100 - kept;
		int othersTotal = 0;

		for ( i = 0; i < count; i++ )
		{
			if ( i != keep )
				othersTotal += m_ChanceValues[i];
		}

		int running = 0;
		int last    = -1;

		for ( i = 0; i < count; i++ )
		{
			if ( i == keep )
				continue;

			//! Multiplied out in floats first - an int division would round
			//! every share down and leave the column short of 100.
			if ( othersTotal > 0 )
				share = Math.Round( ( m_ChanceValues[i] * remainder * 1.0 ) / othersTotal );
			else
				share = Math.Round( ( remainder * 1.0 ) / ( count - 1 ) );

			m_ChanceValues[i] = share;
			running += share;
			last = i;
		}

		if ( last != -1 )
			m_ChanceValues[last] = Math.Clamp( m_ChanceValues[last] + ( remainder - running ), 0, 100 );
	}

	//! Scale every row to add up to 100, or split evenly when they are all at
	//! zero - which is what a freshly added state looks like.
	protected void NormalizeChances()
	{
		int count = m_ChanceValues.Count();

		if ( count == 0 )
			return;

		int i;
		int share;
		int total = 0;

		for ( i = 0; i < count; i++ )
			total += m_ChanceValues[i];

		int running = 0;

		for ( i = 0; i < count; i++ )
		{
			if ( total > 0 )
				share = Math.Round( ( m_ChanceValues[i] * 100.0 ) / total );
			else
				share = Math.Round( 100.0 / count );

			m_ChanceValues[i] = share;
			running += share;
		}

		m_ChanceValues[count - 1] = Math.Clamp( m_ChanceValues[count - 1] + ( 100 - running ), 0, 100 );
	}

	void OnChange_Chance( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UIActionSlider moved = UIActionSlider.Cast( action );

		if ( !moved )
			return;

		int index = m_ChanceSliders.Find( moved );

		if ( index < 0 )
			return;

		m_ChanceValues[index] = Math.Round( moved.GetCurrent() );

		BalanceChances( index );
		PaintChanceSliders();
	}

	void OnClick_AddState( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_DropAddState )
			return;

		//! Row 0 is the prompt, so the option list is offset by one.
		int index = m_DropAddState.GetSelection() - 1;

		if ( index < 0 || index >= m_AddOptions.Count() )
			return;

		action.AnimateFeedback();

		m_ChanceTargets.Insert( m_AddOptions[index] );
		m_ChanceValues.Insert( 0 );

		//! A new state comes in at zero, so the whole column is renormalised
		//! rather than balanced around it - otherwise it would keep the zero
		//! and never be worth adding.
		NormalizeChances();
		BuildChanceRows();
	}

	void OnClick_RemoveState( UIActionBase action )
	{
		UIActionImageButton clicked = UIActionImageButton.Cast( action );

		if ( !clicked )
			return;

		int index = m_ChanceDeletes.Find( clicked );

		if ( index < 0 )
			return;

		m_ChanceTargets.Remove( index );
		m_ChanceValues.Remove( index );

		NormalizeChances();
		BuildChanceRows();
	}

	void OnClick_DynamicSave( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module || !m_Form.m_Module.HasSettings() )
			return;

		action.AnimateFeedback();

		if ( m_SelectedPreset == "" )
			return;

		JMWeatherDynamicConfig config = new JMWeatherDynamicConfig;

		//! The payload names ONE preset, so nothing else has to be carried
		//! over - every other preset's settings live on the presets themselves
		//! and are never in this message to begin with.
		config.PresetName = m_SelectedPreset;

		if ( m_ToggleDynamic )
			config.Enabled = m_ToggleDynamic.IsChecked();

		if ( m_ToggleInRotation )
			config.InRotation = m_ToggleInRotation.IsChecked();

		if ( m_RangeDuration )
		{
			config.DurationMin = Math.Round( m_RangeDuration.GetRangeLow() ) * 60;
			config.DurationMax = Math.Round( m_RangeDuration.GetRangeHigh() ) * 60;
		}

		if ( m_RangeTransition )
		{
			config.TransitionMin = Math.Round( m_RangeTransition.GetRangeLow() );
			config.TransitionMax = Math.Round( m_RangeTransition.GetRangeHigh() );
		}

		JMWeatherNextState entry;

		for ( int j = 0; j < m_ChanceTargets.Count(); j++ )
		{
			//! A zero chance is the absence of a candidate. Storing it would
			//! only grow the file and slow every roll down.
			if ( m_ChanceValues[j] <= 0 )
				continue;

			entry = new JMWeatherNextState;
			entry.To     = m_ChanceTargets[j];
			entry.Chance = m_ChanceValues[j];

			config.NextStates.Insert( entry );
		}

		m_Form.m_Module.SetDynamicWeather( config );
	}

	void OnChange_PresetSelect( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || !m_SelectPreset )
			return;

		int index = m_SelectPreset.GetSelection();

		if ( index == ROW_LIVE_WORLD )
		{
			ExitPresetMode();
			return;
		}

		if ( index == ROW_CREATE_NEW || !m_Form.m_Module || !m_Form.m_Module.HasSettings() )
		{
			CreateNew();
			return;
		}

		array< ref JMWeatherPreset > presets = m_Form.m_Module.GetPresets();

		int preset = index - ROW_FIRST_PRESET;

		if ( preset < 0 || preset >= presets.Count() )
		{
			CreateNew();
			return;
		}

		SetSelectedPreset( presets[preset].Name );
	}

	void OnClick_PresetUpdate( UIActionBase action )
	{
		SavePreset();
	}

	void OnClick_PresetRun( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module || m_SelectedPreset == "" )
			return;

		action.AnimateFeedback();

		m_Form.m_Module.UsePreset( m_SelectedPreset );
	}

	void OnClick_PresetRemove( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || m_SelectedPreset == "" )
			return;

		RemovePreset( m_SelectedPreset );
	}

	// -------------------------------------------------------------------------
	//  Permissions / enable state
	// -------------------------------------------------------------------------

	void UpdateStates()
	{
		bool noPresetSelected = ( m_SelectedPreset == "" );

		m_Form.UpdateActionState( m_SelectPreset, JMConstants.PERM_WEATHER_PRESET_USE );
		m_Form.UpdateActionState( m_EditPresetName, JMConstants.PERM_WEATHER_PRESET_CREATE );
		m_Form.UpdateActionState( m_ButtonPresetSave, JMConstants.PERM_WEATHER_PRESET_CREATE );
		m_Form.UpdateActionState( m_ButtonPresetUse, JMConstants.PERM_WEATHER_PRESET_USE, noPresetSelected );
		m_Form.UpdateActionState( m_ButtonPresetRemove, JMConstants.PERM_WEATHER_PRESET_REMOVE, noPresetSelected );

		m_Form.UpdateActionState( m_ToggleDynamic, JMConstants.PERM_WEATHER_PRESET_DYNAMIC );
		m_Form.UpdateActionState( m_ToggleInRotation, JMConstants.PERM_WEATHER_PRESET_DYNAMIC, noPresetSelected );
		m_Form.UpdateActionState( m_RangeDuration, JMConstants.PERM_WEATHER_PRESET_DYNAMIC );
		m_Form.UpdateActionState( m_RangeTransition, JMConstants.PERM_WEATHER_PRESET_DYNAMIC );
		//! Every preset is already listed as a next state, so there is nothing
		//! the dropdown could offer.
		bool noStatesToAdd = ( m_AddOptions.Count() == 0 );

		m_Form.UpdateActionState( m_DropAddState, JMConstants.PERM_WEATHER_PRESET_DYNAMIC, noPresetSelected || noStatesToAdd );
		m_Form.UpdateActionState( m_ButtonAddState, JMConstants.PERM_WEATHER_PRESET_DYNAMIC, noPresetSelected || noStatesToAdd );

		for ( int d = 0; d < m_ChanceDeletes.Count(); d++ )
			m_Form.UpdateActionState( m_ChanceDeletes[d], JMConstants.PERM_WEATHER_PRESET_DYNAMIC );
		m_Form.UpdateActionState( m_ButtonDynamicSave, JMConstants.PERM_WEATHER_PRESET_DYNAMIC, noPresetSelected );

		for ( int c = 0; c < m_ChanceSliders.Count(); c++ )
			m_Form.UpdateActionState( m_ChanceSliders[c], JMConstants.PERM_WEATHER_PRESET_DYNAMIC );
	}
}
