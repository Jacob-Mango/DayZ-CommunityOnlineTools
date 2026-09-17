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
	//! Public (not protected): JMWeatherFormTabTime reaches this class-qualified
	//! (JMWeatherForm.m_DaysInMonth), which needs more than protected.
	static const int m_DaysInMonth [ 12 ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	static const int TAB_OVERVIEW      = 0;
	static const int TAB_SKY           = 1;
	static const int TAB_PRECIPITATION = 2;
	static const int TAB_WIND          = 3;
	static const int TAB_TIME          = 4;
	static const int TAB_PRESETS       = 5;

	static const int TAB_COUNT = 6;

	//! UNUSED - kept only so an external reference does not break. The Presets
	//! tab no longer pre-builds rows; see InitTabPresets.
	//!
	//! Rows of preset widgets built into the Presets tab. 100 was the old
	//! ceiling and no server has come close to it.
	static const int PRESET_ROWS = 100;

	//! Defaults for the two global timing fields. These were what Easy Mode
	//! forced; with Easy Mode gone they are simply what the form starts at.
	static const float DEFAULT_TRANSITION = 0;
	static const float DEFAULT_DURATION   = 240;

	//! Seconds between lightning strikes when nothing else has been chosen.
	static const float DEFAULT_LIGHTNING_INTERVAL = 25;
	static const float DEFAULT_SANDSTORM_DURATION = 30;
	static const float DEFAULT_SANDSTORM_FADEIN = 10;
	static const float DEFAULT_SANDSTORM_OVERCAST = 0.8;
	static const float DEFAULT_SANDSTORM_WINDMAGNITUDE = 18.0;

	//! Top of the wind scale, shared by the Overview readout and the Wind tab.
	static const float WIND_SCALE_MAX = 20;

	//! Height of the mode banner, in the same layout units as the base class's
	//! TAB_STRIP_HEIGHT - the two are pinned together, see PinWeatherGeometry.
	static const int BANNER_HEIGHT = 26;

	//! Fixed rows at the head of the preset selector.
	static const int ROW_LIVE_WORLD   = 0;
	static const int ROW_CREATE_NEW   = 1;
	static const int ROW_FIRST_PRESET = 2;

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

	//! Rain and snowfall thresholds a Clear puts back. The quick actions for
	//! rain and snow move the floor down to 0.5 so their precipitation starts
	//! immediately; left there, the world keeps raining at half overcast long
	//! after the admin asked for clear skies. These are the vanilla
	//! cfgweather.xml values, so Clear restores normal behaviour rather than
	//! inventing a policy of its own.
	static const float CLEAR_THRESHOLD_MIN = 0.6;
	static const float CLEAR_THRESHOLD_MAX = 1.0;

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

	//! UIActionIconCellLabeled is authored at 110x36, but the WrapSpacer adds
	//! its own padding between children on top of that. Under-reserving it by
	//! even a pixel wraps the last chip onto a second row - 116 was too tight
	//! and put 3 chips on one line and 2 on the next.
	static const int QUICK_CELL_W = 160;

	//! Row height in pixels: the 36px chip plus breathing room. This is the
	//! number the card sizes itself to, so it has to be exact.
	static const int QUICK_ROW_H  = 44;

	//! Public enough (no modifier), not protected: JMWeatherFormTabSky/
	//! Precipitation/Wind/Time reach this through their back-reference.
	JMWeatherModule m_Module;

	protected UIActionTabs m_Tabs;

	protected Widget m_TabOverview;
	protected Widget m_TabSky;
	protected Widget m_TabPrecipitation;
	protected Widget m_TabWind;
	protected Widget m_TabTime;
	protected Widget m_TabPresets;

	//! One class per tab, in its own file - only this form constructs/
	//! dispatches to them. Overview and Presets stay on this form: Overview's
	//! GlobalTransition/GlobalDuration/quick-actions and Presets' preview
	//! mechanism (PreviewSection) are read by every other tab, so splitting
	//! them out would just move the coupling instead of removing it.
	ref JMWeatherFormTabSky           m_TabSkyCtrl;
	ref JMWeatherFormTabPrecipitation m_TabPrecipitationCtrl;
	ref JMWeatherFormTabWind          m_TabWindCtrl;
	ref JMWeatherFormTabTime          m_TabTimeCtrl;

	//! Which mode the weather tabs are in, shown across the full width so it is
	//! answered before anything is touched rather than after.
	protected Widget     m_ModeBanner;
	protected TextWidget m_ModeBannerText;

	//! Save and leave, right there in the banner that says you are editing a
	//! preset - the two things you want from that state without going back to
	//! the Presets tab to find them.
	protected ButtonWidget m_BannerSave;
	protected ButtonWidget m_BannerClose;
	protected Widget       m_BannerSaveHover;
	protected Widget       m_BannerCloseHover;

	protected UIActionScroller m_ScrollerOverview;
	protected UIActionScroller m_ScrollerPresets;

	// --- Overview: live state -------------------------------------------------
	//! Raw widgets from weather_state_row.layout rather than UIActions.
	//!
	//! "now -> next" is one line of five fixed-size pieces at known offsets,
	//! which is exactly what the sidebar footer does and exactly what the
	//! spacer-based UIAction widgets will not do - every one of them claims a
	//! full row.
	protected ImageWidget m_StateIconNow;
	protected TextWidget  m_StateTextNow;
	protected ImageWidget m_StateArrow;
	protected ImageWidget m_StateIconNext;
	protected TextWidget  m_StateTextNext;
	protected TextWidget  m_StateHint;

	//! Sandstorm/EVR storm badge, right of the now->next display. Neither
	//! state replicates to clients the way the vanilla forecast phenomena do
	//! (see JMWeatherModule.RequestSpecialWeatherStatus), so it is refreshed
	//! from a polled RPC rather than read straight off g_Game.GetWeather().
	protected ImageWidget m_StateSpecialIcon;
	protected TextWidget  m_StateSpecialText;
	//! g_Game.GetTickTime() timestamp of the last poll, not a countdown - this
	//! Update() override takes no timeslice parameter to decrement one with.
	protected float m_LastSpecialWeatherPoll;

	//! How often the Overview tab asks the server whether a sandstorm/EVR
	//! storm is active. Once a second is plenty for a status badge - this
	//! isn't a control an admin is dragging in real time.
	static const float SPECIAL_WEATHER_POLL_INTERVAL = 1.0;

	protected UIActionText m_StateTextClock;

	//! Range bars, not progress bars: the span between what a phenomenon is
	//! doing now and what it is heading towards says more than a single fill,
	//! and it is the same picture the "now -> next" row draws in words.
	//!
	//! They take no callback and are rewritten every frame, so a drag snaps
	//! straight back - they read as live output rather than as controls.
	protected UIActionSliderRange m_StateOvercast;
	protected UIActionSliderRange m_StateRain;
	protected UIActionSliderRange m_StateSnow;
	protected UIActionSliderRange m_StateFog;
	protected UIActionSliderRange m_StateWind;

	protected UIActionIconGrid m_QuickWeather;
	protected UIActionIconGrid m_QuickTime;

	protected UIActionImageButton m_ButtonRefresh;
	protected UIActionButton      m_ButtonApplyAll;
	protected UIActionToggleSwitch m_ToggleFreezeTime;
	protected UIActionToggleSwitch m_ToggleAutoRefresh;

	protected UIActionTimePicker m_PickerTransition;
	protected UIActionTimePicker m_PickerDuration;

	// --- Presets --------------------------------------------------------------
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

	//! Bitmask tracking which sections the admin has touched since last refresh.
	protected int m_DirtySectionsMask;

	void SetSectionDirty( int sectionId, bool dirty = true )
	{
		if ( dirty )
			m_DirtySectionsMask = m_DirtySectionsMask | ( 1 << sectionId );
		else
			m_DirtySectionsMask = m_DirtySectionsMask & ~( 1 << sectionId );

		UpdateApplyButtonStates();
	}

	bool IsSectionDirty( int sectionId )
	{
		return ( m_DirtySectionsMask & ( 1 << sectionId ) ) != 0;
	}

	protected void ClearDirtySections()
	{
		m_DirtySectionsMask = 0;
		UpdateApplyButtonStates();
	}

	//! Was called from SetSectionDirty()/ClearDirtySections() with no definition
	//! anywhere in the codebase - a pre-existing dangling reference found while
	//! splitting this form's tabs into their own classes, unrelated to that
	//! split. Left as a no-op placeholder rather than guessed at, since there is
	//! no existing per-section Apply-button visual state to restore.
	protected void UpdateApplyButtonStates()
	{
	}

	static autoptr TStringArray CARDINAL_DIRECTIONS = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};



	void JMWeatherForm()
	{
		m_ChanceTargets = new TStringArray;
		m_ChanceValues  = new array<int>;
		m_ChanceSliders = new array< ref UIActionSlider >;
		m_ChanceDeletes = new array< ref UIActionImageButton >;
		m_AddOptions    = new TStringArray;
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	static float PI2DEG( float value )
	{
		return ( value * Math.RAD2DEG ) + 180;
	}

	static float DEG2PI( float value )
	{
		return ( value * Math.DEG2RAD ) - Math.PI;
	}

	// -------------------------------------------------------------------------
	//  Construction
	// -------------------------------------------------------------------------

	override void OnInit()
	{
		m_RightPanel        = layoutRoot.FindAnyWidget( "panel_right" );
		m_RightPanelDisable = layoutRoot.FindAnyWidget( "panel_right_disable" );
		m_RightTabStrip     = layoutRoot.FindAnyWidget( "panel_right_tabs" );
		m_RightContent      = layoutRoot.FindAnyWidget( "panel_right_content" );

		m_TabOverview      = layoutRoot.FindAnyWidget( "tab_overview" );
		m_TabSky           = layoutRoot.FindAnyWidget( "tab_sky" );
		m_TabPrecipitation = layoutRoot.FindAnyWidget( "tab_precipitation" );
		m_TabWind          = layoutRoot.FindAnyWidget( "tab_wind" );
		m_TabTime          = layoutRoot.FindAnyWidget( "tab_time" );
		m_TabPresets       = layoutRoot.FindAnyWidget( "tab_presets" );

		m_ModeBanner     = layoutRoot.FindAnyWidget( "mode_banner" );
		m_ModeBannerText = TextWidget.Cast( layoutRoot.FindAnyWidget( "mode_banner_text" ) );

		m_BannerSave       = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "mode_banner_save" ) );
		m_BannerClose      = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "mode_banner_close" ) );
		m_BannerSaveHover  = layoutRoot.FindAnyWidget( "mode_banner_save_hover" );
		m_BannerCloseHover = layoutRoot.FindAnyWidget( "mode_banner_close_hover" );

		LoadBannerIcon( "mode_banner_save_icon", JMConstants.Lucide( "save" ) );
		LoadBannerIcon( "mode_banner_close_icon", JMConstants.Lucide( "x" ) );

		ref array<string> tabLabels = { "#STR_COT_WEATHER_TAB_OVERVIEW", "#STR_COT_WEATHER_TAB_SKY", "#STR_COT_WEATHER_TAB_PRECIPITATION", "#STR_COT_WEATHER_MODULE_WIND", "#STR_COT_WEATHER_TAB_TIME", "#STR_COT_WEATHER_TAB_PRESETS" };
		ref array<string> tabIcons  = { JMConstants.Lucide( "gauge" ), JMConstants.Lucide( "cloud-sun" ), JMConstants.Lucide( "cloud-rain" ), JMConstants.Lucide( "wind" ), JMConstants.Lucide( "calendar-clock" ), JMConstants.Lucide( "bookmark" ) };

		m_Tabs = UIActionManager.CreateTabs( m_RightTabStrip, tabLabels, tabIcons, this, "OnChange_Tab" );

		m_Tabs.AddContent( m_TabOverview );
		m_Tabs.AddContent( m_TabSky );
		m_Tabs.AddContent( m_TabPrecipitation );
		m_Tabs.AddContent( m_TabWind );
		m_Tabs.AddContent( m_TabTime );
		m_Tabs.AddContent( m_TabPresets );

		InitTabState( TAB_COUNT );

		m_Tabs.SetSelection( TAB_OVERVIEW, false );

		BuildTabIfNeeded( TAB_OVERVIEW );
	}

	private void BuildTabIfNeeded( int tabIdx )
	{
		if ( !ShouldBuildTab( tabIdx ) )
			return;

		switch ( tabIdx )
		{
			case TAB_OVERVIEW:
				InitTabOverview();
				break;

			case TAB_SKY:
				m_TabSkyCtrl = new JMWeatherFormTabSky( this );
				m_TabSkyCtrl.Build( m_TabSky );
				break;

			case TAB_PRECIPITATION:
				m_TabPrecipitationCtrl = new JMWeatherFormTabPrecipitation( this );
				m_TabPrecipitationCtrl.Build( m_TabPrecipitation );
				break;

			case TAB_WIND:
				m_TabWindCtrl = new JMWeatherFormTabWind( this );
				m_TabWindCtrl.Build( m_TabWind );
				break;

			case TAB_TIME:
				m_TabTimeCtrl = new JMWeatherFormTabTime( this );
				m_TabTimeCtrl.Build( m_TabTime );
				break;

			case TAB_PRESETS:
				InitTabPresets();
				break;
		}

		//! A tab built after the form was already open has missed every refresh
		//! that ran before it existed - and in preset mode it has to come up
		//! showing the preset, not the world.
		LoadEditorValues();
		UpdateModeBanner();
		UpdateStates();
	}

	override int GetActiveTabIndex()
	{
		if ( !m_Tabs )
			return -1;

		return m_Tabs.GetSelection();
	}

	void OnChange_Tab( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CloseAllOverlays();

		BuildTabIfNeeded( GetActiveTabIndex() );

		UpdateStates();
	}

	// -------------------------------------------------------------------------
	//  Overview
	// -------------------------------------------------------------------------

	protected void InitTabOverview()
	{
		m_ScrollerOverview = UIActionManager.CreateScroller( m_TabOverview );
		Widget content = m_ScrollerOverview.GetContentWidget();

		// --- Live state -------------------------------------------------------
		UIActionCard stateCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_STATE" );
		m_ButtonRefresh = stateCard.AddRefreshButton( this, "OnClick_Refresh" );

		Widget stateBody = UIActionManager.CreateGridSpacer( stateCard.GetContent(), 3, 1 );

		Widget stateRow = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/weather_state_row.layout", stateBody );

		if ( stateRow )
		{
			m_StateIconNow   = ImageWidget.Cast( stateRow.FindAnyWidget( "wsr_icon_now" ) );
			m_StateTextNow   = TextWidget.Cast( stateRow.FindAnyWidget( "wsr_text_now" ) );
			m_StateArrow     = ImageWidget.Cast( stateRow.FindAnyWidget( "wsr_arrow" ) );
			m_StateIconNext  = ImageWidget.Cast( stateRow.FindAnyWidget( "wsr_icon_next" ) );
			m_StateTextNext  = TextWidget.Cast( stateRow.FindAnyWidget( "wsr_text_next" ) );
			m_StateHint      = TextWidget.Cast( stateRow.FindAnyWidget( "wsr_hint" ) );
			m_StateSpecialIcon = ImageWidget.Cast( stateRow.FindAnyWidget( "wsr_special_icon" ) );
			m_StateSpecialText = TextWidget.Cast( stateRow.FindAnyWidget( "wsr_special_text" ) );

			if ( m_StateSpecialIcon )
				m_StateSpecialIcon.Show( false );
			if ( m_StateSpecialText )
				m_StateSpecialText.Show( false );

			if ( m_StateHint )
				m_StateHint.SetColor( JMTheme.TEXT_MUTED );

			if ( m_StateArrow )
			{
				m_StateArrow.LoadImageFile( 0, JMConstants.Lucide( "move-right" ) );
				m_StateArrow.SetImage( 0 );
				m_StateArrow.SetColor( JMTheme.TEXT_MUTED );
			}

		}

		m_StateTextClock = UIActionManager.CreateText( stateBody, "#STR_COT_WEATHER_MODULE_START_TIME", "" );

		Widget bars = UIActionManager.CreateGridSpacer( stateBody, 5, 1 );

		m_StateOvercast = CreateStateRange( bars, "#STR_COT_WEATHER_MODULE_OVERCAST" );
		m_StateRain     = CreateStateRange( bars, "#STR_COT_WEATHER_MODULE_RAIN" );
		m_StateSnow     = CreateStateRange( bars, "#STR_COT_WEATHER_MODULE_SNOW" );
		m_StateFog      = CreateStateRange( bars, "#STR_COT_WEATHER_MODULE_FOG" );

		//! Wind is the one readout not measured in <0,1> - it is m/s, on the
		//! same 0-20 scale the Wind tab's slider uses.
		m_StateWind = UIActionManager.CreateSliderRange( bars, "#STR_COT_WEATHER_MODULE_WIND", 0, WIND_SCALE_MAX );
		m_StateWind.SetFormat( "#STR_COT_FORMAT_MPS" );
		m_StateWind.SetStep( 0.1 );

		// --- Quick actions ----------------------------------------------------
		UIActionCard quickCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_QUICKACTIONS" );

		//! Straight into the card body, with NO grid spacer of its own.
		//! card_content is already a GridSpacer, and a GridSpacer re-sizes its
		//! children to the cell on every layout pass - so an icon grid nested a
		//! second time had both the exact width and the exact height that
		//! ForceContentWidth set overwritten, which is what made the strip wrap
		//! to two rows and then stretch down the card.
		Widget quickBody = quickCard.GetContent();

		//! Each strip sits in a panel of a FIXED PIXEL HEIGHT, and the icon grid
		//! goes inside that rather than straight into the card.
		//!
		//! ForceContentWidth deliberately leaves the grid root height
		//! fractional - it is written for a horizontal scroller host. card_content
		//! is a GridSpacer with Size To Content V, which resolves a fractional
		//! height against the whole tab, so the card grew to fill the page and
		//! pushed the conditions card off the top. A pixel-height panel gives the
		//! row a real number to size to.
		Widget quickWeatherRow = UIActionManager.CreatePanel( quickBody, 0x00000000, QUICK_ROW_H );

		m_QuickWeather = UIActionManager.CreateIconGridHorizontal( quickWeatherRow, this, "OnClick_QuickWeather" );
		m_QuickWeather.UseLabeledCells( true );

		//! Each chip fires a command and is done; none of them is a state the
		//! form stays in. A latched blue cell would claim the weather still IS
		//! whatever was last pressed, which stops being true the moment any
		//! slider or the server's own controller moves.
		m_QuickWeather.SetMomentary( true );
		m_QuickWeather.AddIcon( "clear",  JMConstants.Lucide( "sun" ),             "#STR_COT_WEATHER_CLEAR" );
		m_QuickWeather.AddIcon( "cloudy", JMConstants.Lucide( "cloud-sun" ),       "#STR_COT_WEATHER_CLOUDY" );
		m_QuickWeather.AddIcon( "rainy",  JMConstants.Lucide( "cloud-rain" ),      "#STR_COT_WEATHER_MODULE_RAINY" );
		m_QuickWeather.AddIcon( "snowy",  JMConstants.Lucide( "cloud-snow" ),      "#STR_COT_WEATHER_MODULE_SNOWY" );
		m_QuickWeather.AddIcon( "storm",  JMConstants.Lucide( "cloud-lightning" ), "#STR_COT_WEATHER_MODULE_STORM" );

		//! Without this the inner WrapSpacer sizes itself to the whole cell it
		//! was handed and the strip grows to fill the card vertically. Has to
		//! run AFTER the icons exist. Labeled cells are authored at 110x36.
		m_QuickWeather.ForceContentWidth( QUICK_CELL_W * 5, QUICK_ROW_H );

		Widget quickTimeRow = UIActionManager.CreatePanel( quickBody, 0x00000000, QUICK_ROW_H );

		m_QuickTime = UIActionManager.CreateIconGridHorizontal( quickTimeRow, this, "OnClick_QuickTime" );
		m_QuickTime.UseLabeledCells( true );
		m_QuickTime.SetMomentary( true );
		m_QuickTime.AddIcon( "dawn",  JMConstants.Lucide( "sunrise" ), "#STR_COT_WEATHER_DAWN" );
		m_QuickTime.AddIcon( "day",   JMConstants.Lucide( "sun" ),     "#STR_COT_WEATHER_DAY" );
		m_QuickTime.AddIcon( "dusk",  JMConstants.Lucide( "sunset" ),  "#STR_COT_WEATHER_DUSK" );
		m_QuickTime.AddIcon( "night", JMConstants.Lucide( "moon" ),    "#STR_COT_WEATHER_NIGHT" );

		m_QuickTime.ForceContentWidth( QUICK_CELL_W * 4, QUICK_ROW_H );

		// --- Timing + session toggles ----------------------------------------
		UIActionCard timingCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_TIMING" );
		Widget timingBody = UIActionManager.CreateGridSpacer( timingCard.GetContent(), 5, 1 );

		//! Both are durations in seconds and both routinely run to minutes or
		//! hours, which a HH:MM:SS field states far better than a raw second
		//! count in a spinner.
		m_PickerTransition = UIActionManager.CreateTimePicker( timingBody, "#STR_COT_GENERIC_TRANSITION", this );
		m_PickerTransition.SetMaxHours( 1 );
		m_PickerTransition.SetTotalSeconds( DEFAULT_TRANSITION );
		m_PickerTransition.SetTooltip( "#STR_COT_WEATHER_TIMING_TRANSITION_DESC" );

		m_PickerDuration = UIActionManager.CreateTimePicker( timingBody, "#STR_COT_GENERIC_DURATION", this );
		m_PickerDuration.SetMaxHours( 6 );
		m_PickerDuration.SetTotalSeconds( DEFAULT_DURATION );
		m_PickerDuration.SetTooltip( "#STR_COT_WEATHER_TIMING_DURATION_DESC" );

		m_ToggleFreezeTime = UIActionManager.CreateToggleSwitch( timingBody, "#STR_COT_WEATHER_FREEZETIME", this, "OnClick_FreezeTime" );
		m_ToggleFreezeTime.SetTooltip( "#STR_COT_WEATHER_FREEZETIME_DESC" );
		if ( m_Module )
			m_ToggleFreezeTime.SetChecked( m_Module.IsTimeFrozen() );

		m_ToggleAutoRefresh = UIActionManager.CreateToggleSwitch( timingBody, "#STR_COT_WEATHER_MODULE_AUTOREFRESH", this );
		m_ToggleAutoRefresh.SetTooltip( "#STR_COT_WEATHER_AUTOREFRESH_DESC" );
		m_ToggleAutoRefresh.SetChecked( true );

		m_ButtonApplyAll = UIActionManager.CreateButton( timingBody, "#STR_COT_WEATHER_APPLY_ALL", this, "OnClick_ApplyAll" );
		m_ButtonApplyAll.SetColor( JMTheme.SUCCESS_FILL );
		m_ButtonApplyAll.SetTooltip( "#STR_COT_WEATHER_APPLY_ALL_DESC" );

		m_ScrollerOverview.UpdateScroller();
	}

	// -------------------------------------------------------------------------
	//  Presets
	// -------------------------------------------------------------------------

	protected void InitTabPresets()
	{
		m_ScrollerPresets = UIActionManager.CreateScroller( m_TabPresets );
		Widget content = m_ScrollerPresets.GetContentWidget();

		UIActionCard manageCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_TAB_PRESETS" );
		Widget manageBody = UIActionManager.CreateGridSpacer( manageCard.GetContent(), 3, 1 );

		//! One dropdown, rather than the list of PRESET_ROWS row widgets this
		//! used to build. Those were all created up front and hidden one by
		//! one, but a hidden child still owns its cell in a GridSpacer - so the
		//! saved presets drew scattered down the tab with gaps between them,
		//! and the tab scrolled for a hundred rows that were never filled.
		//!
		//! The popup is anchored to layoutRoot, not to the card, so it draws
		//! over the tab instead of being clipped by the scroller it lives in.
		m_SelectPreset = UIActionManager.CreateDropdown( manageBody, "#STR_COT_WEATHER_PRESET_LIST", layoutRoot, this, "OnChange_PresetSelect", BuildPresetOptions() );
		m_SelectPreset.SetTooltip( "#STR_COT_WEATHER_PRESET_SELECT_DESC" );
		RegisterOverlay( m_SelectPreset );

		m_EditPresetName = UIActionManager.CreateEditableText( manageBody, "#STR_COT_GENERIC_NAME", this );
		m_EditPresetName.SetOnlyNumbers( false );
		m_EditPresetName.SetText( "" );

		Widget actionRow = UIActionManager.CreateGridSpacer( manageBody, 1, 3 );
		m_ButtonPresetUse    = UIActionManager.CreateButton( actionRow, "#STR_COT_WEATHER_MODULE_PRESET_APPLY", this, "OnClick_PresetRun" );
		m_ButtonPresetSave   = UIActionManager.CreateButton( actionRow, "#STR_COT_WEATHER_MODULE_PRESET_SAVE", this, "OnClick_PresetUpdate" );
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
		m_PreviewWind     = CreatePreviewSlider( previewBody, "#STR_COT_WEATHER_MODULE_WIND", WIND_SCALE_MAX );
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

		m_DropAddState = UIActionManager.CreateDropdown( addRow, "", layoutRoot, this, "" );
		m_DropAddState.SetWidth( ADD_DROPDOWN_W );
		RegisterOverlay( m_DropAddState );

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

	//! A read-only slider.
	//!
	//! Disabling it would be the obvious way to say "not editable", but a
	//! disabled UIAction is painted under a 90%-opaque panel and the value
	//! would be unreadable. Instead it takes a callback that repaints the
	//! whole preview from the stored preset, so a drag snaps back the instant
	//! it starts and the control reads as output rather than input.
	protected UIActionSlider CreatePreviewSlider( Widget parent, string label, float max )
	{
		UIActionSlider slider = UIActionManager.CreateSlider( parent, label, 0, max, this, "OnChange_PreviewLocked" );
		slider.SetStepValue( 1 );
		return slider;
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

		if ( !m_Module || !m_Module.HasSettings() )
			return options;

		array< ref JMWeatherPreset > presets = m_Module.GetPresets();

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
		if ( name == "" || !m_Module || !m_Module.HasSettings() )
			return -1;

		array< ref JMWeatherPreset > presets = m_Module.GetPresets();

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

		return m_Module.GetPresets()[index];
	}

	// -------------------------------------------------------------------------
	//  Shared builders
	// -------------------------------------------------------------------------

	//! Every 0-1 phenomenon is edited as a whole percentage. The engine wants
	//! the fraction, so the read side multiplies by 0.01 - see ReadPercent.
	//! `instance` defaults to this form (Overview has no percent sliders of its
	//! own, but the default keeps this call-compatible); every other tab class
	//! passes itself so the CHANGE callback fires on the object that owns it.
	UIActionSlider CreatePercentSlider( Widget parent, string label, string callback, Class instance = null )
	{
		if ( !instance )
			instance = this;

		UIActionSlider slider = UIActionManager.CreateSlider( parent, label, 0, 100, instance, callback );
		slider.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		slider.SetStepValue( 1 );
		return slider;
	}

	float ReadPercent( UIActionSlider slider )
	{
		if ( !slider )
			return 0;

		return slider.GetCurrent() * 0.01;
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

		PinWeatherGeometry( h );

		if ( m_ScrollerOverview )      m_ScrollerOverview.UpdateScroller();
		if ( m_ScrollerPresets )       m_ScrollerPresets.UpdateScroller();

		if ( m_TabSkyCtrl )           m_TabSkyCtrl.OnResize();
		if ( m_TabPrecipitationCtrl ) m_TabPrecipitationCtrl.OnResize();
		if ( m_TabWindCtrl )          m_TabWindCtrl.OnResize();
		if ( m_TabTimeCtrl )          m_TabTimeCtrl.OnResize();
	}

	//! JMFormBase pins the content directly under the tab strip, which would
	//! draw it straight over the banner - and it does so on every resize, so a
	//! split written into the layout file alone would not survive. The three
	//! bands are pinned together here instead.
	protected void PinWeatherGeometry( float contentHeight )
	{
		if ( !m_RightTabStrip || !m_RightContent || !m_ModeBanner )
			return;

		//! Below this the split says nothing, and before the first real resize
		//! the height can still arrive as a fraction.
		if ( contentHeight <= TAB_STRIP_HEIGHT + BANNER_HEIGHT )
			return;

		m_RightTabStrip.SetFlags( WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE, true );
		m_RightTabStrip.SetPos( 0, 0 );
		m_RightTabStrip.SetSize( 1, TAB_STRIP_HEIGHT );

		m_ModeBanner.SetFlags( WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE, true );
		m_ModeBanner.SetPos( 0, TAB_STRIP_HEIGHT );
		m_ModeBanner.SetSize( 1, BANNER_HEIGHT );

		m_RightContent.SetFlags( WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE, true );
		m_RightContent.SetPos( 0, TAB_STRIP_HEIGHT + BANNER_HEIGHT );
		m_RightContent.SetSize( 1, contentHeight - TAB_STRIP_HEIGHT - BANNER_HEIGHT );
	}

	override void OnSettingsUpdated()
	{
		if ( !m_Module || !m_Module.IsLoaded() )
			return;

		UpdatePresetList();
		RefreshDynamicControls();
	}

	override void Update()
	{
		RefreshLiveState();
		RefreshTimingWarnings();
		PollSpecialWeatherStatus();

		//! Auto-refresh pulls the LIVE WORLD into the editing controls several
		//! times a second. In preset mode those controls hold the preset, so
		//! the poll would wipe out the edit as fast as it was made.
		if ( IsPresetMode() )
			return;

		if ( m_ToggleAutoRefresh && m_ToggleAutoRefresh.IsChecked() )
			RefreshValues( true );
	}

	//! Flag the two timing values that are legal but surprising.
	//!
	//! Neither is an error and neither is blocked - a snap change is sometimes
	//! exactly what an admin wants. They are called out because the result does
	//! not look like a setting at all: a zero transition reads as the weather
	//! glitching, and a zero hold reads as the weather ignoring what was just
	//! applied. Both are worth a sentence before the admin goes looking for a
	//! bug that is not there.
	protected void RefreshTimingWarnings()
	{
		if ( m_PickerTransition )
			m_PickerTransition.SetWarning( m_PickerTransition.GetTotalSeconds() == 0, "#STR_COT_WEATHER_WARN_TRANSITION" );

		if ( m_PickerDuration )
			m_PickerDuration.SetWarning( m_PickerDuration.GetTotalSeconds() == 0, "#STR_COT_WEATHER_WARN_DURATION" );
	}

	// -------------------------------------------------------------------------
	//  Live state readout
	// -------------------------------------------------------------------------

	//! Current weather, what it is heading towards, and how long until the
	//! controller recomputes. All of it comes from JMWorldConditions so this
	//! panel and the sidebar can never disagree.
	protected void RefreshLiveState()
	{
		if ( !m_StateTextNow )
			return;

		int year, month, day, hour, minute;
		g_Game.GetWorld().GetDate( year, month, day, hour, minute );

		bool isNight = JMWorldConditions.IsNight( hour );

		int nowKind  = JMWorldConditions.GetWeatherKind();
		int nextKind = JMWorldConditions.GetForecastWeatherKind();

		m_StateTextNow.SetText( Widget.TranslateString( JMWorldConditions.GetWeatherLabel( nowKind ) ) );
		//! The countdown is part of the Next label rather than a widget of its
		//! own, so it stays glued to the state it belongs to however long the
		//! translated weather name turns out to be.
		string nextLabel = Widget.TranslateString( JMWorldConditions.GetWeatherLabel( nextKind ) );

		string countdown = JMWorldConditions.FormatSecondsToChange( JMWorldConditions.GetSecondsToWeatherChange() );

		//! The engine exposes exactly ONE number here - GetNextChange, the wait
		//! until the weather controller picks its next forecast. How long the
		//! CURRENT weather lasts and how long until the NEXT one are therefore
		//! the same number, so it is printed once, worded as a wait ("in 2m"),
		//! and the second line says what that wait means for the current state
		//! rather than repeating the figure.
		if ( countdown != "" )
			nextLabel += "   " + string.Format( Widget.TranslateString( "#STR_COT_WEATHER_IN" ), countdown );

		m_StateTextNext.SetText( nextLabel );

		if ( m_StateHint )
		{
			if ( countdown != "" )
				m_StateHint.SetText( Widget.TranslateString( "#STR_COT_WEATHER_STATE_HOLD" ) );
			else
				m_StateHint.SetText( Widget.TranslateString( "#STR_COT_WEATHER_STATE_NOCHANGE" ) );
		}

		LoadStateIcon( m_StateIconNow, JMWorldConditions.GetWeatherIcon( nowKind, isNight ) );
		LoadStateIcon( m_StateIconNext, JMWorldConditions.GetWeatherIcon( nextKind, isNight ) );

		int tod = JMWorldConditions.GetTimeOfDay( hour );

		string clock = Widget.TranslateString( JMWorldConditions.GetTimeOfDayLabel( tod ) );
		clock += "  " + FormatClock( hour, minute );
		clock += "  " + day + "/" + month + "/" + year;

		//! The countdown lives on the state row now, beside the arrow it
		//! belongs to, so it is not repeated here.
		m_StateTextClock.SetText( clock );

		Weather weather = g_Game.GetWeather();
		if ( !weather )
			return;

		SetStateRange( m_StateOvercast, weather.GetOvercast(), 100.0, 100.0 );
		SetStateRange( m_StateRain, weather.GetRain(), 100.0, 100.0 );
		SetStateRange( m_StateSnow, weather.GetSnowfall(), 100.0, 100.0 );
		SetStateRange( m_StateFog, weather.GetFog(), 100.0, 100.0 );
		SetStateRange( m_StateWind, weather.GetWindMagnitude(), 1.0, WIND_SCALE_MAX );

		RefreshSpecialWeatherBadge();
	}

	//! Asks the server whether a sandstorm or (Namalsk) EVR storm is active,
	//! throttled to once a second - see JMWeatherModule.RequestSpecialWeatherStatus.
	protected void PollSpecialWeatherStatus()
	{
		if ( !m_Module )
			return;

		float now = g_Game.GetTickTime();
		if ( now - m_LastSpecialWeatherPoll < SPECIAL_WEATHER_POLL_INTERVAL )
			return;

		m_LastSpecialWeatherPoll = now;
		m_Module.RequestSpecialWeatherStatus();
	}

	//! Paints the badge from whatever JMWeatherModule last heard back from
	//! PollSpecialWeatherStatus's RPC - hidden when neither is active.
	protected void RefreshSpecialWeatherBadge()
	{
		if ( !m_StateSpecialIcon || !m_StateSpecialText || !m_Module )
			return;

		string label = "";
		string icon = "";

		//! EVR takes the row when both happen to be active at once - it is
		//! the more dangerous of the two and the one an admin is less likely
		//! to already know about from the sky itself.
		if ( m_Module.m_LastEVRStormActive )
		{
			label = "#STR_COT_WEATHER_EVR_ACTIVE";
			icon = JMConstants.Lucide( "radiation" );
		}
		else if ( m_Module.m_LastSandstormActive )
		{
			label = "#STR_COT_WEATHER_SANDSTORM_ACTIVE";
			icon = JMConstants.Lucide( "wind" );
		}

		bool active = label != "";

		m_StateSpecialIcon.Show( active );
		m_StateSpecialText.Show( active );

		if ( !active )
			return;

		m_StateSpecialText.SetText( Widget.TranslateString( label ) );
		m_StateSpecialText.SetColor( JMTheme.WARNING );
		m_StateSpecialIcon.LoadImageFile( 0, icon );
		m_StateSpecialIcon.SetImage( 0 );
		m_StateSpecialIcon.SetColor( JMTheme.WARNING );
	}

	//! A 0-100 percentage range bar for one phenomenon.
	protected UIActionSliderRange CreateStateRange( Widget parent, string label )
	{
		UIActionSliderRange range = UIActionManager.CreateSliderRange( parent, label, 0, 100 );
		range.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		range.SetStep( 1 );
		return range;
	}

	protected void LoadStateIcon( ImageWidget widget, string path )
	{
		if ( !widget )
			return;

		widget.LoadImageFile( 0, path );
		widget.SetImage( 0 );
	}

	//! Paint one phenomenon as the span between where it is and where it is
	//! going. A range wants its ends ordered, and a forecast is as likely to be
	//! below the current value as above it, so they are sorted before use.
	//!
	//! Sorting throws away the one thing the admin most wants to know, though -
	//! WHICH end is the forecast - because the bar looks identical either way
	//! round. So the direction is handed to the slider separately and its
	//! arrows drift toward the target.
	protected void SetStateRange( UIActionSliderRange range, WeatherPhenomenon phenomenon, float scale, float ceiling )
	{
		if ( !range || !phenomenon )
			return;

		float actual   = phenomenon.GetActual() * scale;
		float forecast = phenomenon.GetForecast() * scale;

		float low  = Math.Clamp( Math.Min( actual, forecast ), 0, ceiling );
		float high = Math.Clamp( Math.Max( actual, forecast ), 0, ceiling );

		range.SetRange( low, high );

		//! Settled weather has nowhere to go, and arrows on a bar that is not
		//! moving would be a lie.
		int direction = 0;

		if ( forecast > actual )
			direction = 1;
		else if ( forecast < actual )
			direction = -1;

		range.SetFlowDirection( direction );
	}

	protected string FormatClock( int hour, int minute )
	{
		string h = hour.ToString();
		if ( hour < 10 )
			h = "0" + h;

		string m = minute.ToString();
		if ( minute < 10 )
			m = "0" + m;

		return h + ":" + m;
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

	//! Public: every per-tab Set*Values()/Apply*() calls this to paint a
	//! percent slider from a phenomenon's actual/forecast value.
	void SetPercentSlider( UIActionSlider slider, JMWeatherPhenomenon phenomenon, bool actual )
	{
		if ( !slider )
			return;

		if ( actual )
			slider.SetCurrent( phenomenon.Actual * 100.0 );
		else if ( phenomenon.Forecast != -1 )
			slider.SetCurrent( phenomenon.Forecast * 100.0 );
	}

	// -------------------------------------------------------------------------
	//  Reading the controls into a preset
	// -------------------------------------------------------------------------

	void GetUIActionValues( out JMWeatherPreset preset )
	{
		float transition = GlobalTransition();
		float duration   = GlobalDuration();

		if ( m_EditPresetName )
			preset.Name = m_EditPresetName.GetText();

		if ( m_TabTimeCtrl )
			m_TabTimeCtrl.ReadInto( preset );

		if ( m_TabSkyCtrl )
			m_TabSkyCtrl.ReadInto( preset, transition, duration );

		if ( m_TabPrecipitationCtrl )
			m_TabPrecipitationCtrl.ReadInto( preset, transition, duration );

		if ( m_TabWindCtrl )
			m_TabWindCtrl.ReadInto( preset, transition, duration );
	}

	//! Public: read by every per-tab Apply*()/ReadInto(), which is why these
	//! stay on the form even though the pickers themselves are Overview-owned.
	float GlobalTransition()
	{
		if ( m_PickerTransition )
			return m_PickerTransition.GetTotalSeconds();

		return DEFAULT_TRANSITION;
	}

	float GlobalDuration()
	{
		if ( m_PickerDuration )
			return m_PickerDuration.GetTotalSeconds();

		return DEFAULT_DURATION;
	}

	//! Compass point for a heading in degrees, shared with the preset preview
	//! and with JMWeatherFormTabWind.
	static string CardinalFor( float degrees )
	{
		int index = Math.Floor( ( ( degrees + 22.5 ) / 45 ) );

		if ( index > CARDINAL_DIRECTIONS.Count() - 1 )
			index -= CARDINAL_DIRECTIONS.Count();
		else if ( index < 0 )
			index = 0;

		return CARDINAL_DIRECTIONS[index];
	}

	// -------------------------------------------------------------------------
	//  Apply - per-section Apply buttons/logic moved to their owning tab
	//  class (JMWeatherFormTabSky/Precipitation/Wind/Time.c); Apply All stays
	//  here since it is Overview's own button and has to reach every section.
	// -------------------------------------------------------------------------

	//! Writes every section the admin has touched, and only those. A section
	//! nobody edited is left to the mission's own weather controller.
	void OnClick_ApplyAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Module )
			return;

		action.AnimateFeedback();

		//! Dirty tracking exists so a live Apply All does not write sections
		//! the admin never touched. A preset preview has no such risk - it
		//! changes nothing but this client's own view - and the admin asked to
		//! see the preset, which means all of it.
		if ( IsPresetMode() )
		{
			if ( m_TabSkyCtrl && JMPermissions.Has( JMConstants.PERM_WEATHER_OVERCAST ) )        m_TabSkyCtrl.ApplyOvercast();
			if ( m_TabSkyCtrl && JMPermissions.Has( JMConstants.PERM_WEATHER_FOG ) )             m_TabSkyCtrl.ApplyFog();
			if ( m_TabSkyCtrl && JMPermissions.Has( JMConstants.PERM_WEATHER_STORM ) )           m_TabSkyCtrl.ApplyStorm();
			if ( m_TabSkyCtrl && JMPermissions.Has( JMConstants.PERM_WEATHER_SANDSTORM ) )       m_TabSkyCtrl.ApplySandstorm();
			if ( m_TabPrecipitationCtrl && JMPermissions.Has( JMConstants.PERM_WEATHER_RAIN ) )  m_TabPrecipitationCtrl.ApplyRain();
			if ( m_TabPrecipitationCtrl && JMPermissions.Has( JMConstants.PERM_WEATHER_SNOW ) )  m_TabPrecipitationCtrl.ApplySnow();
			if ( m_TabWindCtrl && JMPermissions.Has( JMConstants.PERM_WEATHER_WIND ) )           m_TabWindCtrl.ApplyWind();
			if ( m_TabWindCtrl && JMPermissions.Has( JMConstants.PERM_WEATHER_WIND_FUNCPARAMS ) ) m_TabWindCtrl.ApplyWindFunction();
			if ( m_TabTimeCtrl && JMPermissions.Has( JMConstants.PERM_WEATHER_DATE ) )           m_TabTimeCtrl.ApplyTime();
			return;
		}

		if ( m_TabSkyCtrl && IsSectionDirty( SECTION_OVERCAST ) && JMPermissions.Has( JMConstants.PERM_WEATHER_OVERCAST ) )        m_TabSkyCtrl.ApplyOvercast();
		if ( m_TabSkyCtrl && IsSectionDirty( SECTION_FOG ) && JMPermissions.Has( JMConstants.PERM_WEATHER_FOG ) )                  m_TabSkyCtrl.ApplyFog();
		if ( m_TabSkyCtrl && IsSectionDirty( SECTION_STORM ) && JMPermissions.Has( JMConstants.PERM_WEATHER_STORM ) )              m_TabSkyCtrl.ApplyStorm();
		if ( m_TabSkyCtrl && IsSectionDirty( SECTION_SANDSTORM ) && JMPermissions.Has( JMConstants.PERM_WEATHER_SANDSTORM ) )      m_TabSkyCtrl.ApplySandstorm();
		if ( m_TabPrecipitationCtrl && IsSectionDirty( SECTION_RAIN ) && JMPermissions.Has( JMConstants.PERM_WEATHER_RAIN ) )      m_TabPrecipitationCtrl.ApplyRain();
		if ( m_TabPrecipitationCtrl && IsSectionDirty( SECTION_SNOW ) && JMPermissions.Has( JMConstants.PERM_WEATHER_SNOW ) )      m_TabPrecipitationCtrl.ApplySnow();
		if ( m_TabWindCtrl && IsSectionDirty( SECTION_WIND ) && JMPermissions.Has( JMConstants.PERM_WEATHER_WIND ) )               m_TabWindCtrl.ApplyWind();
		if ( m_TabWindCtrl && IsSectionDirty( SECTION_WIND_FUNCTION ) && JMPermissions.Has( JMConstants.PERM_WEATHER_WIND_FUNCPARAMS ) ) m_TabWindCtrl.ApplyWindFunction();
		if ( m_TabTimeCtrl && IsSectionDirty( SECTION_TIME ) && JMPermissions.Has( JMConstants.PERM_WEATHER_DATE ) )               m_TabTimeCtrl.ApplyTime();
	}

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

	// -------------------------------------------------------------------------
	//  Quick actions
	// -------------------------------------------------------------------------

	void OnClick_QuickWeather( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Module || !m_QuickWeather )
			return;

		string id = m_QuickWeather.GetLastClickedId();

		float transition = GlobalTransition();
		float duration   = GlobalDuration();

		//! Clear is the undo button of this strip, and an undo that fades is
		//! not an undo. It lands instantly whatever the Transition field says,
		//! so the rain the admin is trying to stop stops now rather than over
		//! the next few minutes.
		if ( id == "clear" )
			transition = 0;

		//! Enforce has no block scope, so every quick action's working values
		//! are named once here rather than per branch.
		float overcast = 0;
		float rain     = 0;
		float snow     = 0;
		float fog      = 0;
		float density  = 0;
		float windMin  = Math.RandomFloatInclusive( 0, 0.8 );

		if ( id == "clear" )
		{
			overcast = 0;
		}
		else if ( id == "cloudy" )
		{
			overcast = Math.RandomFloatInclusive( 0.5, 0.8 );
			if ( m_TabSkyCtrl ) density = m_TabSkyCtrl.ReadStormDensity();
		}
		else if ( id == "rainy" )
		{
			overcast = Math.RandomFloatInclusive( 0.7, 1.0 );
			rain     = Math.RandomFloatInclusive( 0.5, 1.0 );
			fog      = Math.RandomFloatInclusive( 0.0, 1.0 - windMin );
			if ( m_TabSkyCtrl ) density = m_TabSkyCtrl.ReadStormDensity();
		}
		else if ( id == "snowy" )
		{
			overcast = Math.RandomFloatInclusive( 0.7, 1.0 );
			snow     = Math.RandomFloatInclusive( 0.5, 1.0 );
			fog      = Math.RandomFloatInclusive( 0.0, 1.0 - windMin );
			if ( m_TabSkyCtrl ) density = m_TabSkyCtrl.ReadStormDensity();
		}
		else if ( id == "storm" )
		{
			overcast = Math.RandomFloatInclusive( 0.7, 1.0 );
			rain     = Math.RandomFloatInclusive( 0.5, 1.0 );
			fog      = Math.RandomFloatInclusive( 0.0, 1.0 - windMin );
			density  = 1.0;
		}
		else
		{
			return;
		}

		m_Module.SetOvercast( overcast, transition, duration );
		m_Module.SetRain( rain, transition, duration );
		m_Module.SetSnow( snow, transition, duration );
		m_Module.SetFog( fog, transition, duration );
		m_Module.SetDynamicFog( 0, 0, 0 );

		if ( id == "rainy" )
			m_Module.SetRainThresholds( 0.5, 1.0, transition );

		if ( id == "snowy" )
			m_Module.SetSnowThresholds( 0.5, 1.0, transition );

		float lightningInterval = DEFAULT_LIGHTNING_INTERVAL;
		float stormThreshold    = 0;
		if ( m_TabSkyCtrl )
		{
			lightningInterval = m_TabSkyCtrl.StormLightningInterval();
			stormThreshold    = m_TabSkyCtrl.ReadStormThreshold();
		}

		if ( id == "storm" )
			m_Module.SetStorm( density, 0.7, lightningInterval );
		else if ( density > 0 )
			m_Module.SetStorm( density, Math.Max( stormThreshold, 0.7 ), lightningInterval );

		if ( id == "clear" )
		{
			//! Setting rain and snow to 0 is not enough on its own. The rainy
			//! and snowy actions drop the overcast threshold to 0.5 so their
			//! precipitation starts at once, and a threshold is a standing
			//! rule - left at 0.5 the world starts raining again as soon as
			//! the cloud cover climbs back past half. Clear puts both rules
			//! back to vanilla.
			m_Module.SetRainThresholds( CLEAR_THRESHOLD_MIN, CLEAR_THRESHOLD_MAX, transition );
			m_Module.SetSnowThresholds( CLEAR_THRESHOLD_MIN, CLEAR_THRESHOLD_MAX, transition );

			//! Storm is a separate system with no getter, so nothing else here
			//! would ever turn the lightning off again.
			m_Module.SetStorm( 0, 1.0, lightningInterval );

			m_Module.SetWindMagnitude( 0, transition, duration );
			m_Module.SetWindDirection( 0, transition, duration );
			m_Module.SetWindFunctionParams( 0, 1, 30 );
		}
		else
		{
			m_Module.SetWindMagnitude( Math.RandomFloatInclusive( 0, 20 ), transition, duration );

			//! Wind direction is an ANGLE IN RADIANS, documented as <-PI, +PI>.
			m_Module.SetWindDirection( Math.RandomFloatInclusive( -Math.PI, Math.PI ), transition, duration );

			m_Module.SetWindFunctionParams( windMin, Math.RandomFloatInclusive( windMin, 1 ), Math.RandomFloatInclusive( 0, 30 ) );
		}

		//! A quick action is a live-world command even in preset mode, but the
		//! controls are holding a preset there - so the world it just set is
		//! not read back into them.
		if ( !IsPresetMode() )
			RefreshValues();
	}

	void OnClick_QuickTime( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Module || !m_QuickTime )
			return;

		string id = m_QuickTime.GetLastClickedId();

		int hour = -1;

		if ( id == "dawn" )
			hour = 6;
		else if ( id == "day" )
			hour = 12;
		else if ( id == "dusk" )
			hour = 18;
		else if ( id == "night" )
			hour = 0;

		if ( hour < 0 )
			return;

		//! Only the time of day changes - the calendar date is whatever the
		//! world already says, not whatever the Time tab happens to be showing.
		int year, month, day, currentHour, minute;
		g_Game.GetWorld().GetDate( year, month, day, currentHour, minute );

		m_Module.SetDate( year, month, day, hour, 0 );

		if ( !IsPresetMode() )
			RefreshValues();
	}

	// -------------------------------------------------------------------------
	//  Session toggles
	// -------------------------------------------------------------------------

	void OnClick_FreezeTime( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE && eid != UIEvent.CLICK )
			return;

		if ( m_Module && m_ToggleFreezeTime )
			m_Module.SetFreezeTime( m_ToggleFreezeTime.IsChecked() );
	}

	void OnClick_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_ButtonRefresh )
			m_ButtonRefresh.TriggerSpin( 2 );

		//! An explicit refresh discards local edits rather than protecting
		//! them - and reloads whatever the tabs are meant to be showing, which
		//! in preset mode is the stored preset, not the server.
		ClearDirty();

		LoadEditorValues();
	}

	protected void ClearDirty()
	{
		ClearDirtySections();
	}

	// -------------------------------------------------------------------------
	//  Preset edit mode
	//
	//  With a preset picked on the Presets tab, every other tab edits THAT
	//  preset instead of the live world: the controls are loaded from it, the
	//  live poll stops writing over them, and Apply becomes a local preview.
	// -------------------------------------------------------------------------

	bool IsPresetMode()
	{
		return ( m_SelectedPreset != "" || m_IsCreatingPreset );
	}

	//! The stored preset the tabs are editing. NULL while authoring a new one -
	//! there is nothing saved to load, so the world stays as the starting
	//! point - and NULL when not in preset mode at all.
	protected JMWeatherPreset EditingPreset()
	{
		if ( m_SelectedPreset == "" || !m_Module || !m_Module.HasSettings() )
			return NULL;

		return m_Module.GetPreset( m_SelectedPreset );
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
		if ( m_PickerTransition && preset.POvercast.Time != -1 )
			m_PickerTransition.SetTotalSeconds( preset.POvercast.Time );

		if ( m_PickerDuration && preset.POvercast.MinDuration != -1 )
			m_PickerDuration.SetTotalSeconds( preset.POvercast.MinDuration );
	}

	protected void LoadBannerIcon( string name, string path )
	{
		ImageWidget icon = ImageWidget.Cast( layoutRoot.FindAnyWidget( name ) );

		if ( !icon )
			return;

		icon.LoadImageFile( 0, path );
		icon.SetImage( 0 );
		icon.SetColor( JMTheme.TEXT_ON_ACCENT );
	}

	//! Raw widgets, so their clicks arrive here rather than at a UIAction.
	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( button != MouseState.LEFT )
			return false;

		if ( w == m_BannerClose )
		{
			ExitPresetMode();
			return true;
		}

		if ( w == m_BannerSave )
		{
			SavePreset();
			return true;
		}

		return super.OnClick( w, x, y, button );
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( w == m_BannerSave && m_BannerSaveHover )
			m_BannerSaveHover.Show( true );

		if ( w == m_BannerClose && m_BannerCloseHover )
			m_BannerCloseHover.Show( true );

		return super.OnMouseEnter( w, x, y );
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( w == m_BannerSave && m_BannerSaveHover )
			m_BannerSaveHover.Show( false );

		if ( w == m_BannerClose && m_BannerCloseHover )
			m_BannerCloseHover.Show( false );

		return super.OnMouseLeave( w, enterW, x, y );
	}

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

		LoadEditorValues();
		UpdateModeBanner();

		UpdateStates();
	}

	protected void UpdateModeBanner()
	{
		if ( !m_ModeBanner || !m_ModeBannerText )
			return;

		//! Both buttons only mean anything while a preset is being edited.
		if ( m_BannerSave )
			m_BannerSave.Show( IsPresetMode() );

		if ( m_BannerClose )
			m_BannerClose.Show( IsPresetMode() );

		if ( m_IsCreatingPreset )
		{
			m_ModeBanner.SetColor( JMTheme.ACCENT );
			m_ModeBannerText.SetText( Widget.TranslateString( "#STR_COT_WEATHER_MODE_NEW" ) );
			return;
		}

		if ( m_SelectedPreset != "" )
		{
			m_ModeBanner.SetColor( JMTheme.ACCENT );
			m_ModeBannerText.SetText( string.Format( Widget.TranslateString( "#STR_COT_WEATHER_MODE_PRESET" ), m_SelectedPreset ) );
			return;
		}

		//! The live state gets a banner too. Reserving the band either way
		//! means no reflow when the mode changes, and "which of these am I
		//! about to change" is worth answering in both directions.
		m_ModeBanner.SetColor( JMTheme.SURFACE_RAISED );
		m_ModeBannerText.SetText( Widget.TranslateString( "#STR_COT_WEATHER_MODE_LIVE" ) );
	}

	// -------------------------------------------------------------------------
	//  Presets
	// -------------------------------------------------------------------------

	void CreateNew()
	{
		m_IsCreatingPreset = true;
		m_SelectedPreset   = "";

		if ( m_EditPresetName )
			m_EditPresetName.SetText( "" );

		UpdatePresetPreview();
		RefreshDynamicControls();

		LoadEditorValues();
		UpdateModeBanner();

		UpdateStates();
	}

	void SetSelectedPreset( string preset )
	{
		m_IsCreatingPreset = false;
		m_SelectedPreset   = preset;

		if ( m_EditPresetName )
			m_EditPresetName.SetText( preset );

		UpdatePresetPreview();
		RefreshDynamicControls();

		LoadEditorValues();
		UpdateModeBanner();

		//! Straight after the controls are loaded, never before: the preview is
		//! built by reading them back.
		PreviewPreset();

		UpdateStates();
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

		if ( m_TabSkyCtrl )
		{
			m_TabSkyCtrl.ApplyOvercast();
			m_TabSkyCtrl.ApplyFog();
			m_TabSkyCtrl.ApplyStorm();
		}

		if ( m_TabPrecipitationCtrl )
		{
			m_TabPrecipitationCtrl.ApplyRain();
			m_TabPrecipitationCtrl.ApplySnow();
		}

		if ( m_TabWindCtrl )
		{
			m_TabWindCtrl.ApplyWind();
			m_TabWindCtrl.ApplyWindFunction();
		}
	}

	void RemovePreset( string preset )
	{
		m_RemovePreset = preset;

		CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_GENERIC_CONFIRM", "#STR_COT_WEATHER_PRESET_REMOVE_CONFIRM", "#STR_COT_GENERIC_NO", "", "#STR_COT_GENERIC_YES", "RemovePreset_Yes" );
	}

	void RemovePreset_Yes( JMConfirmation confirmation )
	{
		if ( m_SelectedPreset == m_RemovePreset )
			m_SelectedPreset = "";

		if ( m_Module )
			m_Module.RemovePreset( m_RemovePreset );

		UpdateStates();
	}

	void UpdatePresetList()
	{
		if ( !m_Module || !m_Module.HasSettings() )
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

	protected void SetPreviewText( UIActionText action, string text )
	{
		if ( action )
			action.SetText( text );
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

	protected void SetPreviewPercent( UIActionSlider slider, float forecast )
	{
		SetPreviewSlider( slider, PreviewBarValue( forecast, 100.0 ), PreviewPercent( forecast ) );
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
			text += "  " + CardinalFor( PI2DEG( preset.PWindDirection.Forecast ) );

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

		return FormatClock( preset.PDate.Hour, preset.PDate.Minute );
	}

	protected void RefreshDynamicControls()
	{
		if ( !m_ToggleDynamic || !m_Module || !m_Module.HasSettings() )
			return;

		m_ToggleDynamic.SetChecked( m_Module.IsDynamicEnabled() );

		JMWeatherPreset preset = m_Module.GetPreset( m_SelectedPreset );

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
		if ( eid != UIEvent.CHANGE || !m_Module || !m_ToggleDynamic )
			return;

		//! The master switch is global and sits alone on its card, so there is
		//! no Save to batch it into. An empty PresetName sends it by itself and
		//! leaves every preset's own settings alone.
		JMWeatherDynamicConfig config = new JMWeatherDynamicConfig;
		config.Enabled = m_ToggleDynamic.IsChecked();

		m_Module.SetDynamicWeather( config );
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

		if ( !m_Module || !m_Module.HasSettings() )
			return;

		JMWeatherPreset preset = m_Module.GetPreset( m_SelectedPreset );

		if ( !preset || !preset.NextStates )
			return;

		for ( int i = 0; i < preset.NextStates.Count(); i++ )
		{
			//! A candidate naming a preset that has since been deleted has
			//! nothing to draw a row for.
			if ( !m_Module.HasPreset( preset.NextStates[i].To ) )
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
				remove = UIActionManager.CreateDeleteButton( row, this, "OnClick_RemoveState" );

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
		UpdateStates();

		if ( m_ScrollerPresets )
			m_ScrollerPresets.UpdateScroller();
	}

	//! The add dropdown only offers states that are not already listed.
	protected void RebuildAddOptions()
	{
		m_AddOptions.Clear();

		array<string> options = new array<string>;
		options.Insert( Widget.TranslateString( "#STR_COT_WEATHER_DYNAMIC_ADD_HINT" ) );

		if ( m_Module && m_Module.HasSettings() && m_SelectedPreset != "" )
		{
			array< ref JMWeatherPreset > presets = m_Module.GetPresets();

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

	protected void SetChanceTotalText( string key )
	{
		if ( m_TextChanceTotal )
			m_TextChanceTotal.SetText( Widget.TranslateString( key ) );
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

	void OnClick_RemoveState( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

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
		if ( eid != UIEvent.CLICK || !m_Module || !m_Module.HasSettings() )
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

		m_Module.SetDynamicWeather( config );
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

		if ( index == ROW_CREATE_NEW || !m_Module || !m_Module.HasSettings() )
		{
			CreateNew();
			return;
		}

		array< ref JMWeatherPreset > presets = m_Module.GetPresets();

		int preset = index - ROW_FIRST_PRESET;

		if ( preset < 0 || preset >= presets.Count() )
		{
			CreateNew();
			return;
		}

		SetSelectedPreset( presets[preset].Name );
	}

	void OnClick_PresetUpdate( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		SavePreset();
	}

	//! Write the weather tabs into the named preset. Shared by the Presets tab
	//! button and the one in the banner, which have to mean the same thing.
	void SavePreset()
	{
		if ( !m_Module )
			return;

		JMWeatherPreset preset = new JMWeatherPreset;
		GetUIActionValues( preset );

		if ( preset.Name == "" )
			return;

		if ( m_IsCreatingPreset )
			m_Module.CreatePreset( preset );
		else
			m_Module.UpdatePreset( preset );

		m_IsCreatingPreset = false;
		m_SelectedPreset   = preset.Name;

		UpdateModeBanner();
		UpdateStates();
	}

	void OnClick_PresetRun( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Module || m_SelectedPreset == "" )
			return;

		action.AnimateFeedback();

		m_Module.UsePreset( m_SelectedPreset );
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
		bool noPresetSelected = ( m_SelectedPreset == "" );

		UpdateActionState( m_ButtonRefresh, "Weather.View" );
		UpdateActionState( m_ButtonApplyAll, "Weather.View" );
		UpdateActionState( m_ToggleFreezeTime, "Weather.FreezeTime" );
		UpdateActionState( m_ToggleAutoRefresh, "Weather.View" );

		UpdateActionState( m_PickerTransition, "Weather.View" );
		UpdateActionState( m_PickerDuration, "Weather.View" );

		if ( m_TabTimeCtrl )          m_TabTimeCtrl.UpdateStates();
		if ( m_TabSkyCtrl )           m_TabSkyCtrl.UpdateStates();
		if ( m_TabPrecipitationCtrl ) m_TabPrecipitationCtrl.UpdateStates();
		if ( m_TabWindCtrl )          m_TabWindCtrl.UpdateStates();

		UpdateActionState( m_SelectPreset, "Weather.Preset.Use" );
		UpdateActionState( m_EditPresetName, "Weather.Preset.Create" );
		UpdateActionState( m_ButtonPresetSave, "Weather.Preset.Create" );
		UpdateActionState( m_ButtonPresetUse, "Weather.Preset.Use", noPresetSelected );
		UpdateActionState( m_ButtonPresetRemove, "Weather.Preset.Remove", noPresetSelected );

		UpdateActionState( m_ToggleDynamic, "Weather.Preset.Dynamic" );
		UpdateActionState( m_ToggleInRotation, "Weather.Preset.Dynamic", noPresetSelected );
		UpdateActionState( m_RangeDuration, "Weather.Preset.Dynamic" );
		UpdateActionState( m_RangeTransition, "Weather.Preset.Dynamic" );
		//! Every preset is already listed as a next state, so there is nothing
		//! the dropdown could offer.
		bool noStatesToAdd = ( m_AddOptions.Count() == 0 );

		UpdateActionState( m_DropAddState, "Weather.Preset.Dynamic", noPresetSelected || noStatesToAdd );
		UpdateActionState( m_ButtonAddState, "Weather.Preset.Dynamic", noPresetSelected || noStatesToAdd );

		for ( int d = 0; d < m_ChanceDeletes.Count(); d++ )
			UpdateActionState( m_ChanceDeletes[d], "Weather.Preset.Dynamic" );
		UpdateActionState( m_ButtonDynamicSave, "Weather.Preset.Dynamic", noPresetSelected );

		for ( int c = 0; c < m_ChanceSliders.Count(); c++ )
			UpdateActionState( m_ChanceSliders[c], "Weather.Preset.Dynamic" );

		//! The quick-action grids answer to their own per-action permissions,
		//! which the grid cannot express per cell - so the whole strip follows
		//! the umbrella permission.
		UpdateActionState( m_QuickWeather, "Weather.QuickAction" );
		UpdateActionState( m_QuickTime, "Weather.QuickAction.Date" );

		if ( m_RightPanelDisable )
			m_RightPanelDisable.Show( false );
	}
}
