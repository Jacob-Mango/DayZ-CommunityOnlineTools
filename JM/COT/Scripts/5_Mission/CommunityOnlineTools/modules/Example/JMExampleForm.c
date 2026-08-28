/*
 * JMExampleForm - canonical reference for every COT UI component.
 *
 * Tabs:
 *   0  "Basics"        - Button, NavButton, Checkbox, Text, EditableText,
 *                        EditableVector, Dropdown, Slider, ButtonToggle,
 *                        FeedbackButton, EditableTextPreview, RichText
 *   1  "Layout"        - ScrollableSection, SectionHeader, LabeledRow,
 *                        ButtonPair, Divider, CollapsibleSection, KeyValueList
 *   2  "Input"         - SearchBox, SearchBox+suggestions, Spinner, ColorPicker,
 *                        ToggleSwitch, TimePicker, SliderRange
 *   3  "Display"       - ProgressBar, Badge, LogView, DataTable, KeyValueList,
 *                        ContextMenu (right-click a DataTable row)
 *   4  "Selection"     - MultiSelectList, FilterBar, IconGrid, SelectBox, StagedIcon, Dropdown
 *   5  "Advanced"      - Tabs (nested), Paginator, StepList, ConfirmInline
 */
class JMExampleForm: JMFormBase
{
	protected JMExampleModule m_Module;

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	// ?? top-level tab bar ??????????????????????????????????????????????????????
	protected UIActionScroller m_OuterScroller;
	protected UIActionTabs    m_TopTabs;

	// ?? panel widgets (one per top-level tab) ?????????????????????????????????
	protected GridSpacerWidget m_PanelBasics;
	protected GridSpacerWidget m_PanelLayout;
	protected GridSpacerWidget m_PanelInput;
	protected GridSpacerWidget m_PanelDisplay;
	protected GridSpacerWidget m_PanelSelection;
	protected GridSpacerWidget m_PanelAdvanced;

	// ?? Basics ????????????????????????????????????????????????????????????????
	protected UIActionText              m_Text;
	protected UIActionEditableText      m_EditableText;
	protected UIActionEditableVector    m_EditableVector;
	protected UIActionButton            m_Button;
	protected UIActionNavigateButton    m_NavButton;
	protected ref array<string>         m_NavWords = { "Mid", "Extension", "When Train?" };
	protected int                       m_NavIdx;
	protected UIActionDropdownList      m_Dropdown;
	protected ref array<string>         m_DropdownItems = { "Jacob Mango", "Liquidrock", "Arkensor", "LieutenantMaster", "DannyDog" };
	protected UIActionSlider            m_Slider;
	protected UIActionCheckbox          m_Checkbox;
	protected UIActionSearchBox m_TextPreview;
	protected UIActionButtonToggle      m_ButtonToggle;
	protected UIActionFeedbackButton    m_FeedbackButton;
	protected UIActionEditableRichText  m_RichText;

	// ?? Layout ????????????????????????????????????????????????????????????????
	protected ref JMScrollableSection   m_ScrollSection;
	protected UIActionButton            m_BtnAdd;
	protected UIActionButton            m_BtnClear;
	protected UIActionKeyValueList      m_KVList;

	// ?? Input ?????????????????????????????????????????????????????????????????
	protected UIActionSearchBox         m_SearchBox;
	protected UIActionSearchBox         m_SmartSearch;
	protected ref array<string>         m_SmartSearchItems = { "Jacob Mango", "Liquidrock", "Arkensor", "LieutenantMaster", "DannyDog", "Survivor (76561198000000001)", "Bandit (76561198000000002)" };
	protected UIActionSpinner           m_Spinner;
	protected UIActionColorPicker       m_ColorPicker;
	protected UIActionToggleSwitch      m_ToggleSwitch;
	protected UIActionToggle            m_ToggleCheckbox;
	protected UIActionToggle            m_ToggleRadioA;
	protected UIActionToggle            m_ToggleRadioB;
	protected UIActionToggle            m_ToggleRadioC;
	protected UIActionTimePicker        m_TimePicker;
	protected UIActionSliderRange       m_SliderRange;

	// ?? Display ???????????????????????????????????????????????????????????????
	protected UIActionProgressBar       m_ProgressBar;
	protected UIActionBadge             m_Badge;
	protected UIActionLogView           m_LogView;
	protected UIActionDataTable         m_DataTable;
	protected UIActionContextMenu       m_ContextMenu;
	protected UIActionText              m_ContextMenuResult;

	// ?? Selection ?????????????????????????????????????????????????????????????
	protected UIActionMultiSelectList   m_MultiSelect;
	protected UIActionFilterBar         m_FilterBar;
	protected UIActionIconGrid          m_IconGrid;
	protected UIActionSelectBox         m_SelectBox;
	protected ref array<string>         m_SelectItems = { "Option A", "Option B", "Option C" };
	protected UIActionStagedIcon        m_StagedIcon;
	protected UIActionDropdown          m_DropdownIcons;

	// ?? Advanced ??????????????????????????????????????????????????????????????
	protected UIActionTabs              m_NestedTabs;
	protected GridSpacerWidget          m_NestedPanelA;
	protected GridSpacerWidget          m_NestedPanelB;
	protected UIActionPaginator         m_Paginator;
	protected UIActionStepList          m_StepList;
	protected UIActionConfirmInline     m_ConfirmInline;
	protected UIActionConfirmInline     m_ConfirmInlineIcon;

	// -- Preview Lab -----------------------------------------------------------
	//  Every tentative for why MapWidget / ItemPreviewWidget draw nothing inside
	//  COT while they draw fine in vanilla. One variant per row, each differing
	//  from the others in exactly ONE thing, so whichever row draws names the
	//  cause by itself. See InitPreviewLab.
	protected GridSpacerWidget    m_PanelPreviewLab;
	protected ref array<EntityAI> m_LabEntities;
	//! Copies parented to the WORKSPACE ROOT instead of into this form, to test
	//! the host rather than the layout. Kept so they can be cleared again.
	protected ref array<Widget>           m_LabDetached;
	protected UIActionText                m_LabStatus;

	//! T10 / T11: plain COT widgets that the render-layer menu pins to.
	protected Widget                      m_LabLayerAnchor;
	protected Widget                      m_LabMapAnchor;

	//! What the preview rows try to draw. A helmet: small, has a clean invView,
	//! and every world has it.
	static const string LAB_ITEM = "Mich2001Helmet";

	protected ref UIActionTooltip m_Tooltip;

	private JMPlayerModule m_PlayerModule;

	// ?? OnInit ????????????????????????????????????????????????????????????????

	override void OnInit()
	{
		Widget root = layoutRoot.FindAnyWidget( "panel" );
		m_OuterScroller = UIActionManager.CreateScroller( root );
		Widget outerContent = m_OuterScroller.GetContentWidget();

		// Top-level tab bar - drives visibility of the 6 content panels below.
		ref array<string> topLabels = { "Basics", "Layout", "Input", "Display", "Selection", "Advanced", "PreviewLab" };
		ref array<string> topIcons = { JMConstants.ICON_SETTINGS_KNOBS, JMConstants.ICON_STACK, JMConstants.ICON_MOVE, JMConstants.ICON_NOTEBOOK, JMConstants.ICON_RADAR_SWEEP, JMConstants.ICON_TINKER };
		//! Appended rather than written into the literal above: every other entry
		//! is a static const, and a CALL inside an array initialiser is not.
		topIcons.Insert( JMConstants.Lucide( "flask-conical" ) );
		m_TopTabs = UIActionManager.CreateTabs( outerContent, topLabels, topIcons, this, "OnChange_TopTab" );

		m_PanelBasics    = UIActionManager.CreateGridSpacer( outerContent, 16, 1 );
		m_PanelLayout    = UIActionManager.CreateGridSpacer( outerContent, 12, 1 );
		m_PanelInput     = UIActionManager.CreateGridSpacer( outerContent, 15, 1 );
		m_PanelDisplay   = UIActionManager.CreateGridSpacer( outerContent, 10, 1 );
		m_PanelSelection = UIActionManager.CreateGridSpacer( outerContent, 12, 1 );
		m_PanelAdvanced  = UIActionManager.CreateGridSpacer( outerContent, 11, 1 );
		m_PanelPreviewLab = UIActionManager.CreateGridSpacer( outerContent, 6, 1 );

		m_TopTabs.AddContent( m_PanelBasics    );
		m_TopTabs.AddContent( m_PanelLayout    );
		m_TopTabs.AddContent( m_PanelInput     );
		m_TopTabs.AddContent( m_PanelDisplay   );
		m_TopTabs.AddContent( m_PanelSelection );
		m_TopTabs.AddContent( m_PanelAdvanced  );
		m_TopTabs.AddContent( m_PanelPreviewLab );

		InitBasics();
		InitLayout();
		InitInput();
		InitDisplay();
		InitSelection();
		InitAdvanced();
		InitPreviewLab();

		if ( m_Button      ) m_Button.UpdatePermission(       "Admin.Example.Button"   );
		if ( m_Dropdown    ) m_Dropdown.UpdatePermission(     "Admin.Example.Dropdown" );
		if ( m_BtnAdd      ) m_BtnAdd.UpdatePermission(       "Admin.Example.Button"   );
		if ( m_BtnClear    ) m_BtnClear.UpdatePermission(     "Admin.Example.Button"   );
		if ( m_ConfirmInline ) m_ConfirmInline.UpdatePermission("Admin.Example.Button" );
		if ( m_ConfirmInlineIcon ) m_ConfirmInlineIcon.UpdatePermission("Admin.Example.Button" );

		if ( m_TopTabs ) m_TopTabs.SetSelection( 0 );

		m_Tooltip = UIActionManager.CreateTooltip( layoutRoot );

		Class.CastTo( m_PlayerModule, GetModuleManager().GetModule(JMPlayerModule) );
	}

	override void OnResize( float w, float h )
	{
		if ( m_OuterScroller )
			m_OuterScroller.UpdateScroller();
		if ( m_ScrollSection )
			m_ScrollSection.UpdateScroller();
	}

	// ?? Panel initialisers ????????????????????????????????????????????????????

	private void InitBasics()
	{
		Widget p = m_PanelBasics;

		UIActionManager.CreateSectionHeader( p, "Text & Display" );
		m_Text = UIActionManager.CreateText( p, "Label", "Value" );

		UIActionManager.CreateSectionHeader( p, "Inputs" );
		m_EditableText   = UIActionManager.CreateEditableText( p, "Editable:", this, "OnChange_EditableText" );
		m_EditableVector = UIActionManager.CreateEditableVector( p, "Position", this, "OnClick_EditableVector", "Set" );

		UIActionManager.CreateSectionHeader( p, "Buttons" );
		Widget btnRow = UIActionManager.CreateGridSpacer( p, 1, 2 );
		m_Button    = UIActionManager.CreateButton( btnRow, "Press Me", this, "OnClick_Button" );
		if ( m_Button ) m_Button.SetTooltip( "Triggers a multi-step confirmation chain", JMConstants.ICON_RADIOACTIVE, 0, ARGB(255,255,200,80) );
		m_NavButton = UIActionManager.CreateNavButton( btnRow, m_NavWords[0], JM_COT_ICON_ARROW_LEFT, JM_COT_ICON_ARROW_RIGHT, this, "OnClick_NavButton" );
		if ( m_NavButton ) m_NavButton.SetTooltip( "Navigate through the word list", "", ARGB(255,80,160,255) );

		m_ButtonToggle = UIActionManager.CreateButtonToggle( p, "Toggle: OFF", "Toggle: ON", this, "OnClick_Toggle" );
		if ( m_ButtonToggle ) m_ButtonToggle.SetTooltip( "Toggles between two states" );

		m_FeedbackButton = UIActionManager.CreateFeedbackButton( p, "Copy Steam ID", "Copied!", JMConstants.ICON_CHECK_MARK, this, "OnClick_FeedbackButton" );
		if ( m_FeedbackButton ) m_FeedbackButton.SetTooltip( "Fades its label out and back to confirm the click" );

		UIActionManager.CreateSectionHeader( p, "Dropdown & Slider" );
		m_Dropdown = UIActionManager.CreateDropdownBox( p, layoutRoot.FindAnyWidget("panel"), "Bestie:", m_DropdownItems, this, "OnClick_Dropdown" );
		if ( m_Dropdown )
		{
			m_Dropdown.SetPosition( 0.35 );
			m_Dropdown.SetWidth( 0.65 );
		}

		m_Slider = UIActionManager.CreateSlider( p, "Slidy:", 0, 100, this, "OnChange_Slider", 0.5 );
		if ( m_Slider )
		{
			m_Slider.SetCurrent( 50 );
			m_Slider.SetStepValue( 1 );
		}

		m_Checkbox = UIActionManager.CreateCheckbox( p, "Did My Homework", this, "OnClick_Checkbox" );

		UIActionManager.CreateSectionHeader( p, "Text Preview & RichText" );
		m_TextPreview = UIActionManager.CreateSearchBox( p, this, "OnChange_TextPreview", "", "", "Name:" );
		if ( m_TextPreview )
		{
			m_TextPreview.SetWidth( 0.5 );
			m_TextPreview.SetEditBoxWidth( 0.65 );
		}

		m_RichText = UIActionManager.CreateEditableRichText( p, "Notes:", this );
		if ( m_RichText )
		{
			m_RichText.SetWidth( 0.5 );
			m_RichText.SetEditBoxWidth( 0.65 );
		}
	}

	private void InitLayout()
	{
		Widget p = m_PanelLayout;

		UIActionManager.CreateSectionHeader( p, "CollapsibleSection" );
		UIActionCollapsibleSection sec = UIActionManager.CreateCollapsibleSection( p, "Click to expand/collapse" );
		if ( sec )
		{
			Widget secContent = sec.GetContent();
			UIActionManager.CreateText( secContent, "Hidden Row 1", "value" );
			UIActionManager.CreateText( secContent, "Hidden Row 2", "value" );
			sec.SetExpanded( false );
		}

		UIActionManager.CreateSectionHeader( p, "ScrollableSection + LabeledRow" );
		m_ScrollSection = UIActionManager.CreateScrollableSection( p );
		Widget listContent = null;
		if ( m_ScrollSection )
			listContent = m_ScrollSection.GetContent();
		UIActionManager.CreateSectionHeader( listContent, "Section A", JMUIStyle.DIVIDER_DARK );
		UIActionManager.CreateText( listContent, "Row A", "value-a" );
		UIActionManager.CreateText( listContent, "Row B", "value-b" );
		UIActionManager.CreateSectionHeader( listContent, "Section B" );
		Widget nameCell = UIActionManager.CreateLabeledRow( listContent, "Name" );
		UIActionManager.CreateEditableText( nameCell, "", this, "OnChange_EditableText" );
		Widget amtCell = UIActionManager.CreateLabeledRow( listContent, "Amount" );
		UIActionManager.CreateSlider( amtCell, "", 0, 100, this, "OnChange_Slider" );

		UIActionButton btnAdd;
		UIActionButton btnClear;
		UIActionManager.CreateButtonPair( p,
			"Add Row",    this, "OnClick_AddRow",   btnAdd,
			"Clear List", this, "OnClick_ClearList", btnClear );
		m_BtnAdd   = btnAdd;
		m_BtnClear = btnClear;

		UIActionManager.CreateDivider( p, JMUIStyle.DIVIDER_LIGHT );

		UIActionManager.CreateSectionHeader( p, "KeyValueList" );
		m_KVList = UIActionManager.CreateKeyValueList( p );
		m_KVList.SetValue( "Server",  "DayZ #42"  );
		m_KVList.SetValue( "Players", "32 / 60"   );
		m_KVList.SetValue( "Uptime",  "4h 12m"    );
		m_KVList.SetValue( "Map",     "Chernarus" );
	}

	private void InitInput()
	{
		Widget p = m_PanelInput;

		UIActionManager.CreateSectionHeader( p, "SearchBox" );
		m_SearchBox = UIActionManager.CreateSearchBox( p, this, "OnChange_Search", "Search..." );

		// SearchBox with an attached suggestion dropdown - same control the ban
		// form uses for its player picker. The list anchor must be a widget that
		// can host a floating popup (the form panel), not the row itself, so the
		// popup is not clipped by the scroller.
		UIActionManager.CreateSectionHeader( p, "SearchBox + suggestions" );
		m_SmartSearch = UIActionManager.CreateSearchBox( p, this, "OnChange_SmartSearch", "Search player / SteamID...", "", "", layoutRoot.FindAnyWidget( "panel" ), m_SmartSearchItems );
		if ( m_SmartSearch )
		{
			m_SmartSearch.SetMaxVisibleSuggestions( 6 );
			// SetAutoFilter( true ) is the default: the list filters itself against
			// the typed text. Pass false to keep every entry visible and do the
			// filtering yourself in the CHANGE callback via SetSuggestions().
			m_SmartSearch.SetAutoFilter( true );
		}

		UIActionManager.CreateSectionHeader( p, "Spinner" );
		m_Spinner = UIActionManager.CreateSpinner( p, "Quantity:", 1, 999, 1, this, "OnChange_Spinner" );
		if ( m_Spinner )
		{
			m_Spinner.SetIntegerOnly( true );
			m_Spinner.SetValue( 1 );
		}

		UIActionManager.CreateSectionHeader( p, "ColorPicker" );
		m_ColorPicker = UIActionManager.CreateColorPicker( p, "Highlight:", this, "OnChange_Color" );
		if ( m_ColorPicker )
			m_ColorPicker.SetColor( ARGB(255, 255, 128, 0) );

		UIActionManager.CreateSectionHeader( p, "ToggleSwitch" );
		m_ToggleSwitch = UIActionManager.CreateToggleSwitch( p, "God Mode", this, "OnClick_Toggle2" );

		UIActionManager.CreateSectionHeader( p, "Toggle (checkbox)" );
		m_ToggleCheckbox = UIActionManager.CreateToggle( p, "Enable feature", this, "OnChange_ToggleCheckbox" );
		if ( m_ToggleCheckbox )
			m_ToggleCheckbox.SetChecked( true );

		UIActionManager.CreateSectionHeader( p, "Toggle (radio / single-choice)" );
		m_ToggleRadioA = UIActionManager.CreateToggle( p, "Option A", this, "OnChange_ToggleRadio" );
		m_ToggleRadioB = UIActionManager.CreateToggle( p, "Option B", this, "OnChange_ToggleRadio" );
		m_ToggleRadioC = UIActionManager.CreateToggle( p, "Option C", this, "OnChange_ToggleRadio" );
		if ( m_ToggleRadioA ) m_ToggleRadioA.SetRound( true );
		if ( m_ToggleRadioB ) m_ToggleRadioB.SetRound( true );
		if ( m_ToggleRadioC ) m_ToggleRadioC.SetRound( true );
		if ( m_ToggleRadioA ) m_ToggleRadioA.SetChecked( true );

		UIActionManager.CreateSectionHeader( p, "TimePicker" );
		m_TimePicker = UIActionManager.CreateTimePicker( p, "Duration:", this, "OnChange_Time" );
		if ( m_TimePicker )
			m_TimePicker.SetTotalSeconds( 3600 );

		UIActionManager.CreateSectionHeader( p, "SliderRange" );
		m_SliderRange = UIActionManager.CreateSliderRange( p, "Health Range:", 0, 100, this, "OnChange_Range" );
		if ( m_SliderRange )
		{
			m_SliderRange.SetStep( 1 );
			m_SliderRange.SetRange( 20, 80 );
		}
	}

	private void InitDisplay()
	{
		Widget p = m_PanelDisplay;

		UIActionManager.CreateSectionHeader( p, "ProgressBar" );
		m_ProgressBar = UIActionManager.CreateProgressBar( p, "Loading:", 0.65 );

		Widget pbRow = UIActionManager.CreateGridSpacer( p, 1, 2 );
		UIActionManager.CreateButton( pbRow, "Fill +10%", this, "OnClick_ProgressAdd" );
		UIActionManager.CreateButton( pbRow, "Reset",     this, "OnClick_ProgressReset" );

		UIActionManager.CreateSectionHeader( p, "Badge" );
		m_Badge = UIActionManager.CreateBadge( p, "Connection", "ONLINE", ARGB(255, 40,180,80) );

		Widget badgeRow = UIActionManager.CreateGridSpacer( p, 1, 3 );
		UIActionManager.CreateButton( badgeRow, "Online",  this, "OnClick_BadgeOnline"  );
		UIActionManager.CreateButton( badgeRow, "Offline", this, "OnClick_BadgeOffline" );
		UIActionManager.CreateButton( badgeRow, "Banned",  this, "OnClick_BadgeBanned"  );

		UIActionManager.CreateSectionHeader( p, "LogView" );
		m_LogView = UIActionManager.CreateLogView( p, 50 );
		if ( m_LogView )
		{
			m_LogView.AppendLine( "Server started",           ARGB(255, 100,200,100) );
			m_LogView.AppendLine( "Player 'Test' joined",     ARGB(255, 180,220,255) );
			m_LogView.AppendLine( "RPC: DeleteVehicle fired", ARGB(255, 255,180, 80) );
		}

		Widget logRow = UIActionManager.CreateGridSpacer( p, 1, 2 );
		UIActionManager.CreateButton( logRow, "Append Line", this, "OnClick_LogAppend" );
		UIActionManager.CreateButton( logRow, "Clear Log",   this, "OnClick_LogClear"  );

		UIActionManager.CreateSectionHeader( p, "DataTable" );
		m_DataTable = UIActionManager.CreateDataTable( p, this, "OnClick_TableRow" );
		if ( m_DataTable )
		{
			ref array<string> cols   = { "Name",      "Status",  "Ping" };
			ref array<float>  widths = { 0.55,          0.25,     0.20  };
			m_DataTable.SetColumns( cols, widths );
			m_DataTable.AddRow( { "Jacob Mango",    "Admin",   "12ms"  } );
			m_DataTable.AddRow( { "Liquidrock",     "Offline", "---"   } );
			m_DataTable.AddRow( { "LieutenantMstr", "Online",  "44ms"  } );
		}

		UIActionManager.CreateSectionHeader( p, "ContextMenu" );
		m_ContextMenuResult = UIActionManager.CreateText( p, "Right-click a table row above.", "" );

		// The stub and the popup both hang off layoutRoot rather than off `p`:
		// the stub would otherwise eat a cell of the Display grid, and the popup
		// has to escape the tab panel or it gets hidden with it.
		m_ContextMenu = UIActionManager.CreateContextMenu( layoutRoot, layoutRoot, this, "OnClick_ContextMenu" );
	}

	private void InitSelection()
	{
		Widget p = m_PanelSelection;

		UIActionManager.CreateSectionHeader( p, "FilterBar (exclusive)" );
		ref array<string> filterLabels = { "All", "Online", "Offline", "Banned" };
		m_FilterBar = UIActionManager.CreateFilterBar( p, filterLabels, this, "OnChange_Filter" );
		if ( m_FilterBar )
			m_FilterBar.SetActive( 0 );

		UIActionManager.CreateSectionHeader( p, "MultiSelectList" );
		ref array<string> names = { "Jacob Mango", "Liquidrock", "Arkensor", "LieutenantMaster", "DannyDog" };
		m_MultiSelect = UIActionManager.CreateMultiSelectList( p, "Select Players:", names, this, "OnChange_MultiSelect" );

		UIActionManager.CreateSectionHeader( p, "IconGrid - Presets" );
		UIActionIconGrid presetGrid = UIActionManager.CreateIconGrid( p, this, "OnClick_Icon" );
		if ( presetGrid )
			presetGrid.AddDefaultPresets();

		UIActionManager.CreateSectionHeader( p, "IconGrid - All Icons" );
		m_IconGrid = UIActionManager.CreateIconGrid( p, this, "OnClick_Icon" );
		if ( m_IconGrid )
		{
			m_IconGrid.AddIcon( "aero_bike",        JMConstants.ICON_AERO_BIKE,        "Aero Bike"         );
			m_IconGrid.AddIcon( "airplane",         JMConstants.ICON_AIRPLANE,         "Airplane"          );
			m_IconGrid.AddIcon( "anticlockwise",    JMConstants.ICON_ANTICLOCKWISE,    "Anticlockwise"     );
			m_IconGrid.AddIcon( "apc",              JMConstants.ICON_APC,              "APC"               );
			m_IconGrid.AddIcon( "arrow_cursor",     JMConstants.ICON_ARROW_CURSOR,     "Arrow Cursor"      );
			m_IconGrid.AddIcon( "auto_repair",      JMConstants.ICON_AUTO_REPAIR,      "Auto Repair"       );
			m_IconGrid.AddIcon( "backward_time",    JMConstants.ICON_BACKWARD_TIME,    "Backward Time"     );
			m_IconGrid.AddIcon( "balaclava",        JMConstants.ICON_BALACLAVA,        "Balaclava"         );
			m_IconGrid.AddIcon( "baton",            JMConstants.ICON_BATON,            "Baton"             );
			m_IconGrid.AddIcon( "battery_0",        JMConstants.ICON_BATTERY_0,        "Battery 0%"        );
			m_IconGrid.AddIcon( "battery_100",      JMConstants.ICON_BATTERY_100,      "Battery 100%"      );
			m_IconGrid.AddIcon( "battery_25",       JMConstants.ICON_BATTERY_25,       "Battery 25%"       );
			m_IconGrid.AddIcon( "battery_50",       JMConstants.ICON_BATTERY_50,       "Battery 50%"       );
			m_IconGrid.AddIcon( "battery_75",       JMConstants.ICON_BATTERY_75,       "Battery 75%"       );
			m_IconGrid.AddIcon( "battery_minus",    JMConstants.ICON_BATTERY_MINUS,    "Battery Minus"     );
			m_IconGrid.AddIcon( "battery_pack_alt", JMConstants.ICON_BATTERY_PACK_ALT, "Battery Pack Alt"  );
			m_IconGrid.AddIcon( "battery_pack",     JMConstants.ICON_BATTERY_PACK,     "Battery Pack"      );
			m_IconGrid.AddIcon( "battery_plus",     JMConstants.ICON_BATTERY_PLUS,     "Battery Plus"      );
			m_IconGrid.AddIcon( "beer_bottle",      JMConstants.ICON_BEER_BOTTLE,      "Beer Bottle"       );
			m_IconGrid.AddIcon( "belt",             JMConstants.ICON_BELT,             "Belt"              );
			m_IconGrid.AddIcon( "bleeding_wound",   JMConstants.ICON_BLEEDING_WOUND,   "Bleeding Wound"    );
			m_IconGrid.AddIcon( "bus",              JMConstants.ICON_BUS,              "Bus"               );
			m_IconGrid.AddIcon( "camping_tent",     JMConstants.ICON_CAMPING_TENT,     "Camping Tent"      );
			m_IconGrid.AddIcon( "cancel",           JMConstants.ICON_CANCEL,           "Cancel"            );
			m_IconGrid.AddIcon( "canned_fish",      JMConstants.ICON_CANNED_FISH,      "Canned Fish"       );
			m_IconGrid.AddIcon( "car_door",         JMConstants.ICON_CAR_DOOR,         "Car Door"          );
			m_IconGrid.AddIcon( "car_key",          JMConstants.ICON_CAR_KEY,          "Car Key"           );
			m_IconGrid.AddIcon( "car_wheel",        JMConstants.ICON_CAR_WHEEL,        "Car Wheel"         );
			m_IconGrid.AddIcon( "check_mark",       JMConstants.ICON_CHECK_MARK,       "Check Mark"        );
			m_IconGrid.AddIcon( "city_car",         JMConstants.ICON_CITY_CAR,         "City Car"          );
			m_IconGrid.AddIcon( "clockwise",        JMConstants.ICON_CLOCKWISE,        "Clockwise"         );
			m_IconGrid.AddIcon( "clothes",          JMConstants.ICON_CLOTHES,          "Clothes"           );
			m_IconGrid.AddIcon( "cog",              JMConstants.ICON_COG,              "Cog"               );
			m_IconGrid.AddIcon( "coins",            JMConstants.ICON_COINS,            "Coins"             );
			m_IconGrid.AddIcon( "dripping_tube",    JMConstants.ICON_DRIPPING_TUBE,    "Dripping Tube"     );
			m_IconGrid.AddIcon( "electric",         JMConstants.ICON_ELECTRIC,         "Electric"          );
			m_IconGrid.AddIcon( "expand",           JMConstants.ICON_EXPAND,           "Expand"            );
			m_IconGrid.AddIcon( "famas",            JMConstants.ICON_FAMAS,            "FAMAS"             );
			m_IconGrid.AddIcon( "fast_backward",    JMConstants.ICON_FAST_BACKWARD,    "Fast Backward"     );
			m_IconGrid.AddIcon( "fast_forward",     JMConstants.ICON_FAST_FORWARD,     "Fast Forward"      );
			m_IconGrid.AddIcon( "flat_tire",        JMConstants.ICON_FLAT_TIRE,        "Flat Tire"         );
			m_IconGrid.AddIcon( "flatbed_covered",  JMConstants.ICON_FLATBED_COVERED,  "Flatbed Covered"   );
			m_IconGrid.AddIcon( "flying_flag",      JMConstants.ICON_FLYING_FLAG,      "Flying Flag"       );
			m_IconGrid.AddIcon( "fog",              JMConstants.ICON_FOG,              "Fog"               );
			m_IconGrid.AddIcon( "folded_paper",     JMConstants.ICON_FOLDED_PAPER,     "Folded Paper"      );
			m_IconGrid.AddIcon( "forklift",         JMConstants.ICON_FORKLIFT,         "Forklift"          );
			m_IconGrid.AddIcon( "fountain_pen",     JMConstants.ICON_FOUNTAIN_PEN,     "Fountain Pen"      );
			m_IconGrid.AddIcon( "full_folder",      JMConstants.ICON_FULL_FOLDER,      "Full Folder"       );
			m_IconGrid.AddIcon( "full_moto_helmet", JMConstants.ICON_FULL_MOTO_HELMET, "Full Moto Helmet"  );
			m_IconGrid.AddIcon( "gas_pump",         JMConstants.ICON_GAS_PUMP,         "Gas Pump"          );
			m_IconGrid.AddIcon( "gladius",          JMConstants.ICON_GLADIUS,          "Gladius"           );
			m_IconGrid.AddIcon( "glider",           JMConstants.ICON_GLIDER,           "Glider"            );
			m_IconGrid.AddIcon( "gloves",           JMConstants.ICON_GLOVES,           "Gloves"            );
			m_IconGrid.AddIcon( "graduate_cap",     JMConstants.ICON_GRADUATE_CAP,     "Graduate Cap"      );
			m_IconGrid.AddIcon( "hamburger_menu",   JMConstants.ICON_HAMBURGER_MENU,   "Hamburger Menu"    );
			m_IconGrid.AddIcon( "hammer_drop",      JMConstants.ICON_HAMMER_DROP,      "Hammer Drop"       );
			m_IconGrid.AddIcon( "health_decrease",  JMConstants.ICON_HEALTH_DECREASE,  "Health Decrease"   );
			m_IconGrid.AddIcon( "health_increase",  JMConstants.ICON_HEALTH_INCREASE,  "Health Increase"   );
			m_IconGrid.AddIcon( "health_normal",    JMConstants.ICON_HEALTH_NORMAL,    "Health Normal"     );
			m_IconGrid.AddIcon( "heart_beats",      JMConstants.ICON_HEART_BEATS,      "Heart Beats"       );
			m_IconGrid.AddIcon( "heart_minus",      JMConstants.ICON_HEART_MINUS,      "Heart Minus"       );
			m_IconGrid.AddIcon( "heart_plus",       JMConstants.ICON_HEART_PLUS,       "Heart Plus"        );
			m_IconGrid.AddIcon( "hearts",           JMConstants.ICON_HEARTS,           "Hearts"            );
			m_IconGrid.AddIcon( "helicopter",       JMConstants.ICON_HELICOPTER,       "Helicopter"        );
			m_IconGrid.AddIcon( "home_garage",      JMConstants.ICON_HOME_GARAGE,      "Home Garage"       );
			m_IconGrid.AddIcon( "horizontal_flip",  JMConstants.ICON_HORIZONTAL_FLIP,  "Horizontal Flip"   );
			m_IconGrid.AddIcon( "info",             JMConstants.ICON_INFO,             "Info"              );
			m_IconGrid.AddIcon( "jeep",             JMConstants.ICON_JEEP,             "Jeep"              );
			m_IconGrid.AddIcon( "knapsack",         JMConstants.ICON_KNAPSACK,         "Knapsack"          );
			m_IconGrid.AddIcon( "knee_bandage",     JMConstants.ICON_KNEE_BANDAGE,     "Knee Bandage"      );
			m_IconGrid.AddIcon( "lee_enfield",      JMConstants.ICON_LEE_ENFIELD,      "Lee Enfield"       );
			m_IconGrid.AddIcon( "light_backpack",   JMConstants.ICON_LIGHT_BACKPACK,   "Light Backpack"    );
			m_IconGrid.AddIcon( "lightning_storm",  JMConstants.ICON_LIGHTNING_STORM,  "Lightning Storm"   );
			m_IconGrid.AddIcon( "machine_gun_mag",  JMConstants.ICON_MACHINE_GUN_MAG,  "Machine Gun Mag"   );
			m_IconGrid.AddIcon( "meat",             JMConstants.ICON_MEAT,             "Meat"              );
			m_IconGrid.AddIcon( "medicines",        JMConstants.ICON_MEDICINES,        "Medicines"         );
			m_IconGrid.AddIcon( "moon",             JMConstants.ICON_MOON,             "Moon"              );
			m_IconGrid.AddIcon( "move",             JMConstants.ICON_MOVE,             "Move"              );
			m_IconGrid.AddIcon( "mp5k",             JMConstants.ICON_MP5K,             "MP5K"              );
			m_IconGrid.AddIcon( "mushroom_gills",   JMConstants.ICON_MUSHROOM_GILLS,   "Mushroom Gills"    );
			m_IconGrid.AddIcon( "notebook",         JMConstants.ICON_NOTEBOOK,         "Notebook"          );
			m_IconGrid.AddIcon( "open_folder",      JMConstants.ICON_OPEN_FOLDER,      "Open Folder"       );
			m_IconGrid.AddIcon( "pause",            JMConstants.ICON_PAUSE,            "Pause"             );
			m_IconGrid.AddIcon( "pencil",           JMConstants.ICON_PENCIL,           "Pencil"            );
			m_IconGrid.AddIcon( "pig",              JMConstants.ICON_PIG,              "Pig"               );
			m_IconGrid.AddIcon( "pin",              JMConstants.ICON_PIN,              "Pin"               );
			m_IconGrid.AddIcon( "pistol",           JMConstants.ICON_PISTOL,           "Pistol"            );
			m_IconGrid.AddIcon( "plain_circle",     JMConstants.ICON_PLAIN_CIRCLE,     "Plain Circle"      );
			m_IconGrid.AddIcon( "plain_square",     JMConstants.ICON_PLAIN_SQUARE,     "Plain Square"      );
			m_IconGrid.AddIcon( "play",             JMConstants.ICON_PLAY,             "Play"              );
			m_IconGrid.AddIcon( "position_marker",  JMConstants.ICON_POSITION_MARKER,  "Position Marker"   );
			m_IconGrid.AddIcon( "power_button",     JMConstants.ICON_POWER_BUTTON,     "Power Button"      );
			m_IconGrid.AddIcon( "radar_sweep",      JMConstants.ICON_RADAR_SWEEP,      "Radar Sweep"       );
			m_IconGrid.AddIcon( "radioactive",      JMConstants.ICON_RADIOACTIVE,      "Radioactive"       );
			m_IconGrid.AddIcon( "raining",          JMConstants.ICON_RAINING,          "Raining"           );
			m_IconGrid.AddIcon( "resize",           JMConstants.ICON_RESIZE,           "Resize"            );
			m_IconGrid.AddIcon( "revolver",         JMConstants.ICON_REVOLVER,         "Revolver"          );
			m_IconGrid.AddIcon( "sailboat",         JMConstants.ICON_SAILBOAT,         "Sailboat"          );
			m_IconGrid.AddIcon( "salmon",           JMConstants.ICON_SALMON,           "Salmon"            );
			m_IconGrid.AddIcon( "save_arrow",       JMConstants.ICON_SAVE_ARROW,       "Save Arrow"        );
			m_IconGrid.AddIcon( "sawed_off_shotgun",JMConstants.ICON_SAWED_OFF_SHOTGUN,"Sawed-Off Shotgun" );
			m_IconGrid.AddIcon( "scooter",          JMConstants.ICON_SCOOTER,          "Scooter"           );
			m_IconGrid.AddIcon( "settings_knobs",   JMConstants.ICON_SETTINGS_KNOBS,   "Settings Knobs"    );
			m_IconGrid.AddIcon( "shambling_zombie", JMConstants.ICON_SHAMBLING_ZOMBIE,  "Shambling Zombie"  );
			m_IconGrid.AddIcon( "sharp_axe",        JMConstants.ICON_SHARP_AXE,        "Sharp Axe"         );
			m_IconGrid.AddIcon( "shotgun_rounds",   JMConstants.ICON_SHOTGUN_ROUNDS,   "Shotgun Rounds"    );
			m_IconGrid.AddIcon( "skimmer_hat",      JMConstants.ICON_SKIMMER_HAT,      "Skimmer Hat"       );
			m_IconGrid.AddIcon( "snowing",          JMConstants.ICON_SNOWING,          "Snowing"           );
			m_IconGrid.AddIcon( "solar_time",       JMConstants.ICON_SOLAR_TIME,       "Solar Time"        );
			m_IconGrid.AddIcon( "speaker_off",      JMConstants.ICON_SPEAKER_OFF,      "Speaker Off"       );
			m_IconGrid.AddIcon( "speaker",          JMConstants.ICON_SPEAKER,          "Speaker"           );
			m_IconGrid.AddIcon( "stack",            JMConstants.ICON_STACK,            "Stack"             );
			m_IconGrid.AddIcon( "stahlhelm",        JMConstants.ICON_STAHLHELM,        "Stahlhelm"         );
			m_IconGrid.AddIcon( "stone_crafting",   JMConstants.ICON_STONE_CRAFTING,   "Stone Crafting"    );
			m_IconGrid.AddIcon( "stopwatch",        JMConstants.ICON_STOPWATCH,        "Stopwatch"         );
			m_IconGrid.AddIcon( "sun_cloud",        JMConstants.ICON_SUN_CLOUD,        "Sun Cloud"         );
			m_IconGrid.AddIcon( "sun",              JMConstants.ICON_SUN,              "Sun"               );
			m_IconGrid.AddIcon( "sunglasses",       JMConstants.ICON_SUNGLASSES,       "Sunglasses"        );
			m_IconGrid.AddIcon( "sunrise",          JMConstants.ICON_SUNRISE,          "Sunrise"           );
			m_IconGrid.AddIcon( "sunset",           JMConstants.ICON_SUNSET,           "Sunset"            );
			m_IconGrid.AddIcon( "t_shirt",          JMConstants.ICON_T_SHIRT,          "T-Shirt"           );
			m_IconGrid.AddIcon( "tank",             JMConstants.ICON_TANK,             "Tank"              );
			m_IconGrid.AddIcon( "thermometer_cold", JMConstants.ICON_THERMOMETER_COLD, "Thermometer Cold"  );
			m_IconGrid.AddIcon( "thermometer_hot",  JMConstants.ICON_THERMOMETER_HOT,  "Thermometer Hot"   );
			m_IconGrid.AddIcon( "tinker",           JMConstants.ICON_TINKER,           "Tinker"            );
			m_IconGrid.AddIcon( "tomato",           JMConstants.ICON_TOMATO,           "Tomato"            );
			m_IconGrid.AddIcon( "trash_can",        JMConstants.ICON_TRASH_CAN,        "Trash Can"         );
			m_IconGrid.AddIcon( "trousers",         JMConstants.ICON_TROUSERS,         "Trousers"          );
			m_IconGrid.AddIcon( "uncertainty",      JMConstants.ICON_UNCERTAINTY,      "Uncertainty"       );
			m_IconGrid.AddIcon( "unlit_bomb",       JMConstants.ICON_UNLIT_BOMB,       "Unlit Bomb"        );
			m_IconGrid.AddIcon( "vertical_flip",    JMConstants.ICON_VERTICAL_FLIP,    "Vertical Flip"     );
			m_IconGrid.AddIcon( "wax_tablet",       JMConstants.ICON_WAX_TABLET,       "Wax Tablet"        );
			m_IconGrid.AddIcon( "windsock",         JMConstants.ICON_WINDSOCK,         "Windsock"          );
			m_IconGrid.AddIcon( "winter_hat",       JMConstants.ICON_WINTER_HAT,       "Winter Hat"        );
			m_IconGrid.AddIcon( "world",            JMConstants.ICON_WORLD,            "World"             );
		}

		UIActionManager.CreateSectionHeader( p, "SelectBox" );
		m_SelectBox = UIActionManager.CreateSelectionBox( p, "Mode:", m_SelectItems, this, "OnClick_SelectBox" );
		if ( m_SelectBox )
			m_SelectBox.SetSelection( 0, false );

		UIActionManager.CreateSectionHeader( p, "StagedIcon - Battery (click to cycle)" );
		m_StagedIcon = UIActionManager.CreateStagedIcon( p, this, "OnChange_StagedIcon" );
		if ( m_StagedIcon )
		{
			m_StagedIcon.AddStage( JMConstants.ICON_BATTERY_0,   ARGB(255, 220,  50,  50) );
			m_StagedIcon.AddStage( JMConstants.ICON_BATTERY_25,  ARGB(255, 220, 130,  40) );
			m_StagedIcon.AddStage( JMConstants.ICON_BATTERY_50,  ARGB(255, 200, 190,  40) );
			m_StagedIcon.AddStage( JMConstants.ICON_BATTERY_75,  ARGB(255, 120, 190,  50) );
			m_StagedIcon.AddStage( JMConstants.ICON_BATTERY_100, ARGB(255,  50, 200,  80) );
			m_StagedIcon.SetCycleOnClick( true );
			m_StagedIcon.SetStage( 4, false );
			UpdateStagedIconTooltip();
		}

		UIActionManager.CreateSectionHeader( p, "Dropdown - Health Status" );
		m_DropdownIcons = UIActionManager.CreateDropdown( p, "Status:", layoutRoot, this, "OnChange_DropdownIcons" );
		if ( m_DropdownIcons )
		{
			m_DropdownIcons.AddEntry( "Healthy",  JMConstants.ICON_HEALTH_NORMAL,   ARGB(255,  50, 200,  80) );
			m_DropdownIcons.AddEntry( "Wounded",  JMConstants.ICON_HEALTH_DECREASE, ARGB(255, 220, 130,  40) );
			m_DropdownIcons.AddEntry( "Critical", JMConstants.ICON_HEART_BEATS,     ARGB(255, 220,  50,  50) );
			m_DropdownIcons.AddEntry( "Bleeding", JMConstants.ICON_BLEEDING_WOUND,  ARGB(255, 180,  40, 140) );
			m_DropdownIcons.AddEntry( "Unknown",  JMConstants.ICON_UNCERTAINTY,     ARGB(255, 140, 140, 140) );
			m_DropdownIcons.SetSelection( 0, false );
		}
	}

	private void InitAdvanced()
	{
		Widget p = m_PanelAdvanced;

		UIActionManager.CreateSectionHeader( p, "Nested Tabs" );
		ref array<string> nestedLabels = { "Tab A", "Tab B" };
		m_NestedTabs = UIActionManager.CreateTabs( p, nestedLabels, this, "OnChange_NestedTab" );

		m_NestedPanelA = UIActionManager.CreateGridSpacer( p, 3, 1 );
		UIActionManager.CreateText( m_NestedPanelA, "Tab A content", "" );
		UIActionManager.CreateText( m_NestedPanelA, "Row 2",         "value" );

		m_NestedPanelB = UIActionManager.CreateGridSpacer( p, 3, 1 );
		UIActionManager.CreateText( m_NestedPanelB, "Tab B content", "" );
		UIActionManager.CreateText( m_NestedPanelB, "Different data","value" );

		if ( m_NestedTabs )
		{
			m_NestedTabs.AddContent( m_NestedPanelA );
			m_NestedTabs.AddContent( m_NestedPanelB );
			m_NestedTabs.SetSelection( 0 );
		}

		UIActionManager.CreateDivider( p );

		UIActionManager.CreateSectionHeader( p, "Paginator" );
		m_Paginator = UIActionManager.CreatePaginator( p, this, "OnChange_Page" );
		if ( m_Paginator )
		{
			m_Paginator.SetPageCount( 5 );
			m_Paginator.SetPage( 0 );
		}

		/*
		UIActionManager.CreateSectionHeader( p, "StepList" );
		m_StepList = UIActionManager.CreateStepList( p, "Spawn Steps:", this, "OnChange_Steps" );
		if ( m_StepList )
		{
			m_StepList.AddItem( "Give AK-74"    );
			m_StepList.AddItem( "Give Magazine" );
			m_StepList.AddItem( "Give Vest"     );
		}
		*/

		Widget stepRow = UIActionManager.CreateGridSpacer( p, 1, 2 );
		UIActionManager.CreateButton( stepRow, "Add Step",   this, "OnClick_AddStep"    );
		UIActionManager.CreateButton( stepRow, "Clear Steps",this, "OnClick_ClearSteps" );

		UIActionManager.CreateSectionHeader( p, "ConfirmInline" );
		m_ConfirmInline = UIActionManager.CreateConfirmInline( p, "Delete Something", this, "OnConfirm_Delete", 4.0 );

		// Icon-only variant, same as the object spawner's delete button: trash-can
		// icon, red, and OK / X glyphs instead of "Confirm" / "Cancel" so the pair
		// still fits inside a 32x32 square.
		m_ConfirmInlineIcon = UIActionManager.CreateConfirmInline( p, "", this, "OnConfirm_DeleteIcon", 4.0 );
		UIActionIconGrid.ApplyDeletePreset( m_ConfirmInlineIcon );
		m_ConfirmInlineIcon.SetButton( "" );
		m_ConfirmInlineIcon.SetFixedSize( 32, 32 );
		m_ConfirmInlineIcon.CenterIcon( 32, 16 );
		m_ConfirmInlineIcon.SetTooltip( "Delete Something (icon variant)" );
	}

	// ?? Callbacks ?????????????????????????????????????????????????????????????

	// =========================================================================
	//  PREVIEW LAB
	//
	//  MapWidget and ItemPreviewWidget draw nothing anywhere in COT - the map
	//  module, the player tab map, the vehicle and entity maps, the object
	//  spawner preview, the inventory cells - while the same widget types draw
	//  fine in vanilla. Already ruled out, each by its own build:
	//
	//    * COT's imageset / style REGISTRATION. A build registering neither
	//      (JM_COT_NO_CUSTOM_STYLES) behaved identically. See JMWidgetStyles.
	//    * WHEN that registration happens - mission init vs before any mission.
	//    * Widget geometry: exact vs fractional sizing, explicit colour,
	//      priority, scaled, inheritalpha, model position.
	//
	//  So this tab stops arguing and puts the remaining candidates side by side.
	//  Each row is one tentative, labelled, differing from its neighbours in
	//  exactly one thing. Whichever row draws IS the answer:
	//
	//    T1  bare preview, exact px, no style      - what COT shipped
	//    T2  vanilla shape, VANILLA-styled parent  - if only this draws, a COT
	//                                                style in the chain is fatal
	//    T3  vanilla shape, COT-styled parent      - T2 vs T3 is the whole
	//                                                "our style breaks it" test
	//    T4  vanilla shape, NO style in the chain  - separates "COT style is
	//                                                fatal" from "a style is
	//                                                required at all"
	//    T5  wrapped in a WindowWidget             - vanilla's map chain shape
	//    T6  priority 900, opaque white            - pure draw-order test
	//    T7  vanilla shape, COT's old attributes   - shape vs attributes
	//    M1  MapWidget in a bare panel
	//    M2  the chain COT ships today
	//    M3  vanilla's day_z_map chain verbatim
	//
	//  RESULT OF ROUND ONE: T1-T7, M1-M3, H1 and H2 all drew nothing. M3 shows a
	//  grey rectangle, which is only its own rover_sim panel painting - not map
	//  tiles. So style, priority, sizing, colour, the wrapper shape and this
	//  form's chrome are all eliminated, and so is the workspace root, since H1
	//  and H2 put a copy there and it stayed blank too.
	//
	//  What that leaves, and what round two adds:
	//
	//    T8  the ENTITY: spawned at the player instead of at 0 0 0
	//    T9  the VIEW: index 0 and no model offset
	//    H3  the HOST: a real UIScriptedMenu, the one context every vanilla map
	//        and item preview is in and COT has never used
	//    H4  is it enough that a menu is OPEN?  ANSWERED: no. An open menu on its
	//        own changes nothing; the widget has to live inside the menu's tree,
	//        which H3 already demonstrates.
	//    H5  REMOVED. It re-parented COT's real window container into the menu,
	//        and the engine destroys a menu's children with the menu - taking
	//        the whole COT UI with it until the client restarts. H3 is the same
	//        proof without the hazard.
	//    V1  the CONTROL: the stock game's own map menu, no COT code at all
	//    V2  the CONTROL: the stock game's own inspect menu, no COT code at all
	// =========================================================================
	protected void InitPreviewLab()
	{
		m_LabEntities = new array<EntityAI>;
		m_LabDetached = new array<Widget>;

		UIActionCard itemCard = UIActionManager.CreateCard( m_PanelPreviewLab, "Item preview tentatives" );
		Widget itemContent = itemCard.GetContent();

		BuildPreviewTentative( itemContent, "T1  bare preview, exact px, no style (what COT shipped)", "JM/COT/GUI/layouts/previewlab/T1_preview_bare.layout" );
		BuildPreviewTentative( itemContent, "T2  vanilla shape, parent styled `blank` (VANILLA style)", "JM/COT/GUI/layouts/previewlab/T2_preview_panel_blank.layout" );
		BuildPreviewTentative( itemContent, "T3  vanilla shape, parent styled COTBox8Set (OUR style)", "JM/COT/GUI/layouts/previewlab/T3_preview_panel_cot.layout" );
		BuildPreviewTentative( itemContent, "T4  vanilla shape, no style anywhere in the chain", "JM/COT/GUI/layouts/previewlab/T4_preview_frame_nostyle.layout" );
		BuildPreviewTentative( itemContent, "T5  wrapped in a WindowWidget (vanilla map chain shape)", "JM/COT/GUI/layouts/previewlab/T5_preview_window.layout" );
		BuildPreviewTentative( itemContent, "T6  priority 900 + opaque white (draw-order test)", "JM/COT/GUI/layouts/previewlab/T6_preview_priority.layout" );
		BuildPreviewTentative( itemContent, "T7  vanilla shape, COT's old attrs (no colour/priority)", "JM/COT/GUI/layouts/previewlab/T7_preview_nocolor.layout" );

		// T1-T7 vary the WIDGET. These two vary the ENTITY behind it, which no
		// tentative has touched yet: every one so far spawned its copy at the
		// map origin and pushed the config's own view index.
		BuildPreviewTentative( itemContent, "T8  entity spawned at the PLAYER, not at 0 0 0", "JM/COT/GUI/layouts/previewlab/T2_preview_panel_blank.layout", true, false );
		BuildPreviewTentative( itemContent, "T9  SetView(0), no SetModelPosition call", "JM/COT/GUI/layouts/previewlab/T2_preview_panel_blank.layout", false, true );

		// T15 is copied from the one place in the base game where an
		// ItemPreviewWidget lives on the WORKSPACE rather than in a menu:
		// PluginItemDiagnostic, gui/layouts/debug/debug_item.layout. That chain
		// is a FrameWidget with priority 5, scaled 1 and inheritalpha 0, every
		// widget in it carries keepsafezone 0, and the plugin drives it with
		// nothing but SetItem and Show - no SetView, no SetModelPosition, both
		// of which COT calls everywhere.
		BuildVanillaPreviewTentative( itemContent, "T15  PluginItemDiagnostic chain, SetItem + Show only" );

		UIActionCard mapCard = UIActionManager.CreateCard( m_PanelPreviewLab, "Map tentatives" );
		Widget mapContent = mapCard.GetContent();

		BuildMapTentative( mapContent, "M1  MapWidget in a bare panel, nothing else", "JM/COT/GUI/layouts/previewlab/M1_map_bare.layout" );
		BuildMapTentative( mapContent, "M2  the chain COT ships today (Window Default / blank)", "JM/COT/GUI/layouts/previewlab/M2_map_cot_chain.layout" );
		BuildMapTentative( mapContent, "M3  vanilla day_z_map chain verbatim (rover_sim_black)", "JM/COT/GUI/layouts/previewlab/M3_map_vanilla.layout" );

		// M4 puts a MapWidget in that same workspace-proven chain. There is no
		// non-menu map anywhere in the base game to copy, so this is the debug
		// plugin's shape with the preview swapped for a map.
		BuildMapTentative( mapContent, "M4  PluginItemDiagnostic chain (keepsafezone 0, priority 5)", "JM/COT/GUI/layouts/previewlab/M4_map_vanilla_debug.layout" );

		UIActionCard hostCard = UIActionManager.CreateCard( m_PanelPreviewLab, "Host tentatives (outside this window)" );
		Widget hostContent = hostCard.GetContent();

		m_LabStatus = UIActionManager.CreateText( hostContent, "Detached copies:", "none" );

		Widget hostRow = UIActionManager.CreateGridSpacer( hostContent, 1, 3 );
		UIActionManager.CreateButton( hostRow, "H1  preview at workspace root", this, "Click_LabDetachPreview" );
		UIActionManager.CreateButton( hostRow, "H2  map at workspace root", this, "Click_LabDetachMap" );
		UIActionManager.CreateButton( hostRow, "Clear detached", this, "Click_LabClearDetached" );

		// H1 / H2 above still draw on the WORKSPACE - the same HUD-level context
		// COT's own windows use, so they only rule out this form's chrome. H3 is
		// the one that changes the host: a real UIScriptedMenu, which is where
		// every vanilla map and item preview lives without exception.
		Widget hostRow2 = UIActionManager.CreateGridSpacer( hostContent, 1, 1 );
		UIActionManager.CreateButton( hostRow2, "H3  open both inside a UIScriptedMenu", this, "Click_LabOpenMenu" );

		// V1 / V2 are the control group, and the only tentatives that contain no
		// COT code, no COT layout and no COT style at all: they open the stock
		// game's own map and inspect menus. Every tentative so far assumed the
		// fault is on COT's side of the line. These two test that assumption.
		// If the vanilla menus are blank as well, nothing in this mod can be the
		// cause and the search moves to the client, its settings or the world.
		// H3 proved the host matters. H4 and H5 split it in two: does a menu
		// simply have to be OPEN, or does the widget have to LIVE in the menu's
		// tree? Open the probe, then look back at T1-T9 and M1-M3 above - they
		// are the readout. The probe itself is invisible and ignores the mouse.
		// T10 is the narrow fix, prototyped without touching a single module.
		// The row below stays an ordinary COT widget on the workspace; only the
		// preview itself lives in a shared invisible menu, pinned to that row's
		// screen rectangle. If it draws here it will draw in any COT window.
		// The anchor is a real panel with a real height, not a bare spacer. An
		// empty spacer measures zero and the layer hides anything pinned to it.
		UIActionManager.CreateText( hostContent, "T10 / T11  render layer, pinned to the boxes below" );
		m_LabLayerAnchor = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/previewlab/anchor_box.layout", hostContent );
		UIActionManager.CreateButton( hostContent, "T10  attach an item preview to the box above", this, "Click_LabAttachLayer" );

		m_LabMapAnchor = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/previewlab/anchor_box.layout", hostContent );
		UIActionManager.CreateButton( hostContent, "T11  attach a MAP to the box above", this, "Click_LabAttachLayerMap" );
		UIActionManager.CreateButton( hostContent, "T10/T11  close the render layer", this, "Click_LabCloseLayer" );
		UIActionManager.CreateButton( hostContent, "T12  next sort step (default / 921 / 930 / 970)", this, "Click_LabToggleLayerSort" );

		// T16 is the whole thing, if the reading is right: MapWidget and
		// ItemPreviewWidget do not render when an ancestor carries a high sort,
		// and COT puts its window container at SORT_WINDOW (920). T10/T11 only
		// ever "worked" because the render layer dropped that container to 0 as
		// a side effect and left it there.
		UIActionManager.CreateButton( hostContent, "T16  toggle COT window sort (920 <-> 0)", this, "Click_LabToggleWindowSort" );

		Widget probeRow = UIActionManager.CreateGridSpacer( hostContent, 1, 1 );
		UIActionManager.CreateButton( probeRow, "H4  toggle an empty menu (COT windows stay put)", this, "Click_LabProbePlain" );

		Widget vanillaRow = UIActionManager.CreateGridSpacer( hostContent, 1, 2 );
		UIActionManager.CreateButton( vanillaRow, "V1  stock game map menu (MENU_MAP)", this, "Click_LabVanillaMap" );
		UIActionManager.CreateButton( vanillaRow, "V2  stock game inspect menu (MENU_INSPECT)", this, "Click_LabVanillaInspect" );
	}

	//! T15: the base game's own workspace preview, reproduced exactly - both the
	//! widget chain and the two-call script that drives it.
	protected void BuildVanillaPreviewTentative( Widget parent, string label )
	{
		UIActionManager.CreateText( parent, label );

		Widget host = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/previewlab/T15_preview_vanilla_debug.layout", parent );

		if ( !host )
			return;

		ItemPreviewWidget preview;
		if ( !Class.CastTo( preview, host.FindAnyWidget( "lab_preview" ) ) )
			return;

		EntityAI ent = LabSpawnEntity( true );
		if ( !ent )
			return;

		//! Deliberately only these two calls. PluginItemDiagnostic never touches
		//! the view index or the model position, and COT always does.
		preview.SetItem( ent );
		preview.Show( true );
	}

	//! One preview row: a label, then the variant layout fed a local entity.
	//!
	//! atPlayer  - spawn the copy at the player instead of at the map origin, in
	//!             case the engine will not draw an entity sitting outside any
	//!             streamed area.
	//! plainView - push view index 0 and skip SetModelPosition, in case the
	//!             config's own view index or the model offset is the problem.
	protected void BuildPreviewTentative( Widget parent, string label, string layoutPath, bool atPlayer = false, bool plainView = false )
	{
		UIActionManager.CreateText( parent, label, "" );

		Widget row = UIActionManager.CreatePanel( parent, 0x00000000, 72 );
		Widget host = g_Game.GetWorkspace().CreateWidgets( layoutPath, row );

		if ( !host )
		{
			Print( "[COT-LAB] layout failed to load: " + layoutPath );
			return;
		}

		ItemPreviewWidget preview;

		// The bare tentative IS the preview; the rest wrap it.
		if ( !Class.CastTo( preview, host ) )
			Class.CastTo( preview, host.FindAnyWidget( "lab_preview" ) );

		if ( !preview )
		{
			Print( "[COT-LAB] no lab_preview in " + layoutPath );
			return;
		}

		EntityAI ent = LabSpawnEntity( atPlayer );
		if ( !ent )
			return;

		preview.SetItem( ent );

		if ( plainView )
		{
			preview.SetView( 0 );
		}
		else
		{
			preview.SetView( ent.GetViewIndex() );
			preview.SetModelPosition( Vector( 0, 0, 1 ) );
			preview.SetModelOrientation( vector.Zero );
		}

		preview.Show( true );
	}

	//! One map row: a label, then the variant layout centred on the player.
	protected void BuildMapTentative( Widget parent, string label, string layoutPath )
	{
		UIActionManager.CreateText( parent, label, "" );

		Widget row = UIActionManager.CreatePanel( parent, 0x00000000, 208 );
		Widget host = g_Game.GetWorkspace().CreateWidgets( layoutPath, row );

		if ( !host )
		{
			Print( "[COT-LAB] layout failed to load: " + layoutPath );
			return;
		}

		//! NOT named `map`: that is the name of Enforce's own map<K,V> template,
		//! and a local shadowing a type name does not compile.
		MapWidget labMap;
		if ( !Class.CastTo( labMap, host.FindAnyWidget( "lab_map" ) ) )
		{
			Print( "[COT-LAB] no lab_map in " + layoutPath );
			return;
		}

		LabCenterMap( labMap );
	}

	//! A client-local copy of LAB_ITEM, spawned the same way the player form's
	//! inventory cells do it - this lab has to reproduce their conditions, not
	//! improve on them.
	protected EntityAI LabSpawnEntity( bool atPlayer = false )
	{
		vector spawnAt = vector.Zero;

		if ( atPlayer )
		{
			Man self = g_Game.GetPlayer();
			if ( self )
				spawnAt = self.GetPosition();
		}

		EntityAI ent = EntityAI.Cast( g_Game.CreateObject( LAB_ITEM, spawnAt, true, false, false ) );

		if ( !ent )
		{
			Print( "[COT-LAB] could not spawn " + LAB_ITEM );
			return NULL;
		}

		dBodyActive( ent, ActiveState.INACTIVE );
		dBodyDynamic( ent, false );
		ent.DisableSimulation( true );

		m_LabEntities.Insert( ent );

		return ent;
	}

	protected void LabCenterMap( MapWidget labMap )
	{
		vector center = "7500 0 7500";

		Man player = g_Game.GetPlayer();
		if ( player )
			center = player.GetPosition();

		labMap.SetScale( 0.4 );
		labMap.SetMapPos( center );
	}

	void Click_LabDetachPreview( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		// NULL parent = straight onto the workspace, the way COT's own windows
		// container is created. Sorted above everything so nothing can cover it.
		Widget host = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/previewlab/T2_preview_panel_blank.layout", NULL );
		if ( !host )
			return;

		host.SetFlags( WidgetFlags.HEXACTPOS | WidgetFlags.VEXACTPOS | WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE, true );
		host.SetPos( 40, 300 );
		host.SetSize( 128, 128 );
		host.SetSort( 9000 );

		ItemPreviewWidget preview;
		if ( Class.CastTo( preview, host.FindAnyWidget( "lab_preview" ) ) )
		{
			EntityAI ent = LabSpawnEntity();
			if ( ent )
			{
				preview.SetItem( ent );
				preview.SetView( ent.GetViewIndex() );
				preview.SetModelPosition( Vector( 0, 0, 1 ) );
				preview.Show( true );
			}
		}

		m_LabDetached.Insert( host );
		LabRefreshStatus();
	}

	void Click_LabDetachMap( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		Widget host = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/previewlab/M1_map_bare.layout", NULL );
		if ( !host )
			return;

		host.SetFlags( WidgetFlags.HEXACTPOS | WidgetFlags.VEXACTPOS | WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE, true );
		host.SetPos( 200, 300 );
		host.SetSize( 400, 400 );
		host.SetSort( 9000 );

		MapWidget labMap;
		if ( Class.CastTo( labMap, host.FindAnyWidget( "lab_map" ) ) )
			LabCenterMap( labMap );

		m_LabDetached.Insert( host );
		LabRefreshStatus();
	}

	void Click_LabOpenMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		g_Game.GetUIManager().EnterScriptedMenu( JMPreviewLabMenu.MENU_ID, NULL );
	}

	//! H4: a menu is open and nothing else changes. If the in-form tentatives
	//! light up while this is on, the render pass is gated on the menu stack
	//! alone and COT's widgets never needed to move.
	void Click_LabProbePlain( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		ToggleLabProbe( false );
	}

	//! Hand a plain COT widget to the render layer and drive the preview it
	//! gives back exactly the way a module would drive one of its own.
	void Click_LabAttachLayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_LabLayerAnchor )
		{
			Print( "[COT-LAB] T10: no anchor widget" );
			return;
		}

		ItemPreviewWidget preview = JMPreviewLayerMenu.Attach( m_LabLayerAnchor );

		if ( !preview )
		{
			Print( "[COT-LAB] T10: render layer gave back no preview" );
			return;
		}

		EntityAI ent = LabSpawnEntity( true );
		if ( !ent )
			return;

		preview.SetItem( ent );
		preview.SetView( ent.GetViewIndex() );
		preview.SetModelPosition( Vector( 0, 0, 1 ) );
		preview.SetModelOrientation( vector.Zero );
		preview.Show( true );

		float ax, ay, aw, ah;
		m_LabLayerAnchor.GetScreenPos( ax, ay );
		m_LabLayerAnchor.GetScreenSize( aw, ah );
		Print( "[COT-LAB] T10 attached at " + ax + "," + ay + " size=" + aw + "x" + ah + " item=" + ent.GetType() );
	}

	//! T11: the map half of the render layer, which is the one that matters for
	//! the map view and the player position map.
	void Click_LabAttachLayerMap( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_LabMapAnchor )
		{
			Print( "[COT-LAB] T11: no anchor widget" );
			return;
		}

		MapWidget layerMap = JMPreviewLayerMenu.AttachMap( m_LabMapAnchor );

		if ( !layerMap )
		{
			Print( "[COT-LAB] T11: render layer gave back no map" );
			return;
		}

		LabCenterMap( layerMap );

		float mw, mh;
		m_LabMapAnchor.GetScreenSize( mw, mh );
		Print( "[COT-LAB] T11 map attached, anchor size=" + mw + "x" + mh );
	}

	//! An open layer menu blocks every other scripted menu, so there has to be
	//! a way to shut it that does not involve restarting the client.
	void Click_LabCloseLayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPreviewLayerMenu.CloseLayer();
		Print( "[COT-LAB] render layer closed" );
	}

	//! T12: the layer draws under COT's windows at its default sort and above
	//! them when raised - but raising it seems to stop the widgets drawing at
	//! all. This flips between the two so both can be seen back to back.
	//! Takes effect the next time the layer opens, so close it first.
	void Click_LabToggleLayerSort( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		int step = JMPreviewLayerMenu.s_SortStep + 1;

		if ( step >= JMPreviewLayerMenu.SORT_STEPS.Count() )
			step = 0;

		JMPreviewLayerMenu.s_SortStep = step;
		JMPreviewLayerMenu.CloseLayer();

		Print( "[COT-LAB] T12 sort step is now " + step + " = " + JMPreviewLayerMenu.SORT_STEPS[step] + " - re-attach to apply" );
	}

	//! Flip the window container between COT's sort and 0, live. Everything
	//! above should blank out and come back with it.
	void Click_LabToggleWindowSort( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !JMStatics.WINDOWS_CONTAINER )
			return;

		int sort = JMStatics.WINDOWS_CONTAINER.GetSort();

		if ( sort == 0 )
			sort = JMUILayout.SORT_WINDOW;
		else
			sort = 0;

		JMStatics.WINDOWS_CONTAINER.SetSort( sort );

		Print( "[COT-LAB] T16 window container sort is now " + sort );
	}

	protected void ToggleLabProbe( bool adopt )
	{
		UIManager ui = g_Game.GetUIManager();

		if ( ui.IsMenuOpen( JMPreviewLabProbeMenu.MENU_ID ) )
		{
			ui.FindMenu( JMPreviewLabProbeMenu.MENU_ID ).Close();
			Print( "[COT-LAB] probe menu closed" );
			return;
		}

		ui.EnterScriptedMenu( JMPreviewLabProbeMenu.MENU_ID, NULL );
	}

	//! Pure vanilla: MapMenu, gui/layouts/day_z_map.layout, no COT anywhere.
	//! This is exactly what ActionUnfoldMap does when a map is unfolded in game.
	void Click_LabVanillaMap( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIManager ui = g_Game.GetUIManager();

		if ( ui.IsMenuOpen( MENU_MAP ) )
		{
			ui.FindMenu( MENU_MAP ).Close();
			return;
		}

		UIScriptedMenu menu = ui.EnterScriptedMenu( MENU_MAP, NULL );
		bool mapOpened = ( menu != NULL );
		Print( "[COT-LAB] V1 vanilla map menu opened=" + mapOpened.ToString() );
	}

	//! Pure vanilla: InspectMenuNew, the same menu the stock inventory opens
	//! when an item is inspected, fed a locally spawned copy of the lab item.
	void Click_LabVanillaInspect( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		EntityAI ent = LabSpawnEntity( true );
		if ( !ent )
		{
			Print( "[COT-LAB] V2: could not spawn " + LAB_ITEM );
			return;
		}

		InspectMenuNew inspect = InspectMenuNew.Cast( g_Game.GetUIManager().EnterScriptedMenu( MENU_INSPECT, NULL ) );

		if ( !inspect )
		{
			Print( "[COT-LAB] V2: inspect menu did not open" );
			return;
		}

		inspect.SetItem( ent );
		Print( "[COT-LAB] V2 vanilla inspect menu opened on " + ent.GetType() );
	}

	void Click_LabClearDetached( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		foreach ( Widget host : m_LabDetached )
		{
			if ( host )
				host.Unlink();
		}

		m_LabDetached.Clear();
		LabRefreshStatus();
	}

	protected void LabRefreshStatus()
	{
		if ( !m_LabStatus )
			return;

		if ( m_LabDetached.Count() == 0 )
			m_LabStatus.SetText( "none" );
		else
			m_LabStatus.SetText( m_LabDetached.Count().ToString() );
	}

	void OnChange_TopTab( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		// Popups anchored outside the tab panel do not get hidden along with it,
		// so a tab switch has to dismiss them explicitly.
		if ( m_ContextMenu )
			m_ContextMenu.Close();
		if ( m_DropdownIcons )
			m_DropdownIcons.Close();

		if ( m_OuterScroller )
			m_OuterScroller.UpdateScroller();
	}

	void OnChange_NestedTab( UIEvent eid, UIActionBase action )
	{
	}

	// Basics
	void OnChange_EditableText( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		UIActionEditableText et = UIActionEditableText.Cast( action );
		if ( et ) et.Disable();
	}

	void OnClick_EditableVector( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		PlayerBase player = PlayerBase.Cast( g_Game.GetPlayer() );
		if ( !player || !player.GetIdentity() ) return;
		if ( !m_PlayerModule )
			Class.CastTo( m_PlayerModule, GetModuleManager().GetModule(JMPlayerModule) );
		if ( m_PlayerModule )
			m_PlayerModule.TeleportTo( action.GetValue(), { player.GetIdentity().GetId() } );
	}

	void OnClick_Button( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		CreateConfirmation_Two( JMConfirmationType.INFO, "Are you sure?", "Kick Selected Player(s)",
			"#STR_COT_GENERIC_YES", "OnFunnyButtons_01", "#STR_COT_GENERIC_NO", "" );
	}

	void OnFunnyButtons_01( JMConfirmation conf )
	{
		CreateConfirmation_Two( JMConfirmationType.INFO, "Are you sure?", "It will kick you included",
			"#STR_COT_GENERIC_YES", "OnFunnyButtons_02", "#STR_COT_GENERIC_NO", "" );
	}

	void OnFunnyButtons_02( JMConfirmation conf )
	{
		CreateConfirmation_Two( JMConfirmationType.INFO, "Please think twice", "You cannot undo this action!",
			"#STR_COT_GENERIC_YES", "OnConfirmationButton", "#STR_COT_GENERIC_YES", "OnConfirmationButton" );
	}

	void OnConfirmationButton( JMConfirmation conf )
	{
		array<string> players = JM_GetSelected().GetPlayers();
		if ( players.Count() == 0 )
		{
			PlayerBase player = PlayerBase.Cast( g_Game.GetPlayer() );
			if ( player && player.GetIdentity() )
				players.Insert( player.GetIdentity().GetId() );
		}
		if ( m_PlayerModule )
			m_PlayerModule.Kick( players, "I am so sorry :c" );
	}

	void OnClick_NavButton( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK || eid == UIEvent.CLICK_RIGHTSIDE )
			m_NavIdx++;
		else if ( eid == UIEvent.CLICK_LEFTSIDE )
			m_NavIdx--;
		else
			return;
		m_NavIdx = Math.Clamp( m_NavIdx, 0, m_NavWords.Count() - 1 );
		m_NavButton.SetButton( m_NavWords[m_NavIdx] );
	}

	void OnClick_Toggle( UIEvent eid, UIActionBase action )
	{
	}

	//! The button swaps its own face; the callback only does the actual work.
	void OnClick_FeedbackButton( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		g_Game.CopyToClipboard( "76561198000000000" );
	}

	void OnClick_Checkbox( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		if ( action.IsChecked() )
		{
			action.SetLabel( "\"Did\" My Homework" );
			COTCreateLocalAdminNotification( new StringLocaliser( "Liar" ) );
		}
		else
		{
			action.SetLabel( "Did My Homework" );
		}
	}

	void OnChange_Slider( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		float c = ( m_Slider.GetCurrent() / 100.0 ) * 255.0;
		m_Slider.SetColor( ARGB(255, c, c, c) );
		m_Slider.SetAlpha( 1.0 );
	}

	void OnClick_Dropdown( UIEvent eid, UIActionBase action )
	{
	}

	void OnChange_TextPreview( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		m_TextPreview.SetTextPreview( "" );
		string s = m_TextPreview.GetText();
		if ( s != "" )
		{
			s.ToLower();
			string kw = "not bob";
			if ( kw.IndexOf(s) == 0 )
				m_TextPreview.SetTextPreview( kw );
		}
	}

	// Layout
	void OnClick_AddRow( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_ScrollSection ) return;
		Widget c = m_ScrollSection.GetContent();
		int n = 0;
		Widget ch = c.GetChildren();
		while ( ch ) { n++; ch = ch.GetSibling(); }
		UIActionManager.CreateText( c, "Dynamic Row " + n, "added" );
		m_ScrollSection.UpdateScroller();
	}

	void OnClick_ClearList( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_ScrollSection ) return;
		UIActionManager.ClearChildren( m_ScrollSection.GetContent() );
		m_ScrollSection.UpdateScroller();
	}

	// Input
	void OnChange_Search( UIEvent eid, UIActionBase action )
	{
		// eid == CHANGE: text changed; eid == CLICK: cleared
		string text = "";
		if ( eid != UIEvent.CLICK )
			text = m_SearchBox.GetText();
		// Filter logic would go here - update a list, table, etc.
	}

	// Suggestion dropdown: CHANGE fires on keystroke and when a row is picked,
	// CLICK fires when the x clears the field.
	void OnChange_SmartSearch( UIEvent eid, UIActionBase action )
	{
		if ( !m_SmartSearch )
			return;

		if ( eid == UIEvent.CLICK )
		{
			m_SmartSearch.SetTextPreview( "" );
			return;
		}

		if ( eid != UIEvent.CHANGE )
			return;

		string text = m_SmartSearch.GetText();

		// Ghost-text autocomplete: show the first prefix match behind the caret,
		// committed with TAB / ENTER. Cleared when nothing matches.
		string preview = "";
		if ( text != "" )
		{
			string needle = text;
			needle.ToLower();

			for ( int i = 0; i < m_SmartSearchItems.Count(); i++ )
			{
				string candidate = m_SmartSearchItems[i];
				string lower = candidate;
				lower.ToLower();

				if ( lower.IndexOf( needle ) == 0 )
				{
					preview = candidate;
					break;
				}
			}
		}

		m_SmartSearch.SetTextPreview( preview );

		// Filter logic would go here - update a list, table, etc.
	}

	void OnChange_Spinner( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		// m_Spinner.GetValueInt() -> use the value
	}

	void OnChange_Color( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		int col = m_ColorPicker.GetColor();
		// Apply color wherever needed
	}

	void OnClick_Toggle2( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		// m_ToggleSwitch.IsChecked()
	}

	void OnChange_ToggleCheckbox( UIEvent eid, UIActionBase action )
	{
		// m_ToggleCheckbox.IsChecked()
	}

	void OnChange_ToggleRadio( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;

		UIActionToggle toggled = UIActionToggle.Cast( action );
		if ( !toggled || !toggled.IsChecked() )
			return;

		// Uncheck all others to enforce single-selection.
		if ( m_ToggleRadioA && m_ToggleRadioA != toggled ) m_ToggleRadioA.SetCheckedSilent( false );
		if ( m_ToggleRadioB && m_ToggleRadioB != toggled ) m_ToggleRadioB.SetCheckedSilent( false );
		if ( m_ToggleRadioC && m_ToggleRadioC != toggled ) m_ToggleRadioC.SetCheckedSilent( false );
	}

	void OnChange_Time( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		int secs = m_TimePicker.GetTotalSeconds();
		// Use secs as ban duration, etc.
	}

	void OnChange_Range( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		float lo = m_SliderRange.GetRangeLow();
		float hi = m_SliderRange.GetRangeHigh();
		// Filter entities in range lo..hi
	}

	// Display
	void OnClick_ProgressAdd( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		m_ProgressBar.SetProgress( Math.Clamp( m_ProgressBar.GetProgress() + 0.1, 0, 1 ) );
	}

	void OnClick_ProgressReset( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		m_ProgressBar.SetProgress( 0 );
	}

	void OnClick_BadgeOnline( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		m_Badge.SetStatus( "ONLINE", ARGB(255, 40,180, 80) );
	}

	void OnClick_BadgeOffline( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		m_Badge.SetStatus( "OFFLINE", ARGB(255, 120,120,120) );
	}

	void OnClick_BadgeBanned( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		m_Badge.SetStatus( "BANNED", ARGB(255, 200, 40, 40) );
	}

	void OnClick_LogAppend( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		static int s_LogCount;
		s_LogCount++;
		int r37  = s_LogCount * 37;
		int r73  = s_LogCount * 73;
		int r113 = s_LogCount * 113;
		int col = ARGB( 255,
			50 + ( r37  - (r37  / 180) * 180 ),
			50 + ( r73  - (r73  / 180) * 180 ),
			50 + ( r113 - (r113 / 180) * 180 ) );
		m_LogView.AppendLine( "Log entry #" + s_LogCount, col );
	}

	void OnClick_LogClear( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		m_LogView.Clear();
	}

	void OnClick_TableRow( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK_RIGHT )
		{
			ShowTableContextMenu();
			return;
		}

		if ( eid != UIEvent.CLICK ) return;
		int row = m_DataTable.GetSelectedRow();
		// Row selected - do something with the row index
	}

	//! Rebuilt on every open so the entries can reflect the row that was hit -
	//! a menu built once at init cannot be row-specific.
	private void ShowTableContextMenu()
	{
		if ( !m_ContextMenu || !m_DataTable )
			return;

		int row = m_DataTable.GetSelectedRow();
		if ( row < 0 )
			return;

		m_ContextMenu.ClearItems();
		m_ContextMenu.AddItem( "inspect", "Inspect", JMConstants.Lucide( "eye"   ) );
		m_ContextMenu.AddItem( "copy",    "Copy",    JMConstants.ICON_STACK        );
		m_ContextMenu.AddItem( "remove",  "Remove",  JMConstants.Lucide( "trash-2" ), JMTheme.DANGER );

		// Second row of the demo table is "Offline" - shown greyed rather than
		// dropped, so the menu keeps the same shape whatever row was clicked.
		m_ContextMenu.SetItemEnabled( "inspect", row != 1 );

		m_ContextMenu.ShowAt( m_DataTable.GetLastRightClickX(), m_DataTable.GetLastRightClickY() );
	}

	void OnClick_ContextMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;

		string id  = m_ContextMenu.GetLastClickedId();
		int    row = m_DataTable.GetSelectedRow();

		if ( m_ContextMenuResult )
			m_ContextMenuResult.SetText( "Picked \"" + id + "\" on row " + row );
	}

	// Selection
	void OnChange_Filter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		array<string> active = m_FilterBar.GetActiveFilters();
		// Rebuild list based on active filters
	}

	void OnChange_MultiSelect( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		array<string> selected = m_MultiSelect.GetSelectedItems();
		// Use selected player list
	}

	void OnClick_Icon( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		string id = m_IconGrid.GetLastClickedId();
		// Spawn or select item with id
	}

	void OnClick_SelectBox( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		int sel = m_SelectBox.GetSelection();
	}

	private static ref array<string> s_BatteryIcons  = { "", "", "", "", "" };
	private static ref array<int>    s_BatteryColors = { 0, 0, 0, 0, 0 };
	private static ref array<string> s_BatteryLabels = { "Empty (0%)", "Low (25%)", "Half (50%)", "Good (75%)", "Full (100%)" };

	private void UpdateStagedIconTooltip()
	{
		if ( !m_StagedIcon )
			return;

		int stage = m_StagedIcon.GetStage();

		string icons[5];
		icons[0] = JMConstants.ICON_BATTERY_0;
		icons[1] = JMConstants.ICON_BATTERY_25;
		icons[2] = JMConstants.ICON_BATTERY_50;
		icons[3] = JMConstants.ICON_BATTERY_75;
		icons[4] = JMConstants.ICON_BATTERY_100;

		int colors[5];
		colors[0] = ARGB(255, 220,  50,  50);
		colors[1] = ARGB(255, 220, 130,  40);
		colors[2] = ARGB(255, 200, 190,  40);
		colors[3] = ARGB(255, 120, 190,  50);
		colors[4] = ARGB(255,  50, 200,  80);

		string labels[5];
		labels[0] = "Battery: Empty (0%)";
		labels[1] = "Battery: Low (25%)";
		labels[2] = "Battery: Half (50%)";
		labels[3] = "Battery: Good (75%)";
		labels[4] = "Battery: Full (100%) - This battery is at maximum charge and ready for use. It was last recharged during the morning cycle and has experienced no significant discharge events since then. Estimated remaining runtime under standard load conditions is approximately fourteen hours, assuming no high-draw peripherals are connected. Consider rotating with backup units.";

		if ( stage >= 0 && stage < 5 )
			m_StagedIcon.SetTooltip( labels[stage], icons[stage], colors[stage], colors[stage] );
	}

	void OnChange_StagedIcon( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		UpdateStagedIconTooltip();
		if ( m_StagedIcon )
			m_StagedIcon.RefreshTooltip();
	}

	void OnChange_DropdownIcons( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		string selected = m_DropdownIcons.GetSelectedText();
	}

	// Advanced
	void OnChange_Page( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		int page = m_Paginator.GetCurrentPage();
		// Rebuild visible slice: items[page * PAGE_SIZE .. (page+1) * PAGE_SIZE]
	}

	void OnChange_Steps( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		array<string> steps = m_StepList.GetItems();
	}

	void OnClick_AddStep( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		m_StepList.AddItem( "New Step " + ( m_StepList.GetItems().Count() + 1 ) );
	}

	void OnClick_ClearSteps( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		m_StepList.Clear();
	}

	void OnConfirm_Delete( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CHANGE )
		{
			// User confirmed - execute the destructive action here
		}
	}

	void OnConfirm_DeleteIcon( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CHANGE )
		{
			// User confirmed - execute the destructive action here
		}
	}
}
