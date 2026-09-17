//! One collapsible group of ESP category filters.
//!
//! Carried as the header button's UIActionData so the click handler knows which
//! group it just folded without a lookup - the same trick JMRoleManagerForm's
//! permission tree uses for its branch rows.
class JMESPFilterGroup: UIActionData
{
	//! All four are prefixed: bare Header/Label/Rows collide with existing
	//! Enforce type names, which the compiler rejects as
	//! "Variable name 'X' already used as type name".
	string GroupLabel;
	string GroupIcon;

	UIActionButton       GroupHeader;
	UIActionImageButton  GroupChevron;
	UIActionToggleSwitch GroupToggle;
	UIActionFoldPanel    GroupFold;

	//! Weak: the rows are owned by JMESPForm.m_ESPTypeList.
	ref array<JMESPViewTypeWidget> GroupRows;

	void JMESPFilterGroup()
	{
		GroupRows = new array<JMESPViewTypeWidget>;
	}
}

//! Archetype B: a fixed toolbar over a tabbed pane.
//!
//! The old form spent 245px of a 560px window on four rows of controls that are
//! set once and then left alone, which left the category list - the thing the
//! form is actually for - with barely half the height. Everything that is not
//! "turn ESP on" or "find something" now lives in the Settings tab, and the ~30
//! view types are grouped by their own inheritance tree instead of being one
//! flat column chunked 100 rows at a time.
class JMESPForm: JMFormBase
{
	//! Height of panel_top in esp_form.layout. The tab strip below is pinned
	//! against it in OnResize; keep the two in step.
	static const int TOOLBAR_HEIGHT = 44;

	//! Thinnest line the skeleton renderer can draw. The slider's own minimum is
	//! 0 so the fill previews the thickness proportionally, but the value can
	//! never settle below this.
	static const float MIN_LINE_THICKNESS = 1.0;

	//! The object list is sized to its CONTENT between these two bounds, not to
	//! whatever height is left over.
	//!
	//! It used to take (window - toolbar - tabs - chrome), which on a tall
	//! window is ~500px of viewport whether the list holds two rows or two
	//! thousand. Inside a size-to-content card that stretched the card to
	//! match, so an empty list pushed the Selection card off the bottom behind
	//! a screen of void.
	static const int OBJECT_LIST_MIN_ROWS = 4;
	static const int OBJECT_LIST_MAX_ROWS = 12;

	//! Vertical space the rest of the tab needs, so a short window shrinks the
	//! list rather than hiding the card under it.
	static const int OBJECT_LIST_CHROME = 260;

	//! Settings rows share one control column so the sliders and dropdowns in
	//! a card line up down a single edge instead of each starting wherever its
	//! own label happened to end.
	static const float SETTING_LABEL_W       = 0.34;
	static const float SETTING_ROW_REMAINDER = 0.64;
	static const float SETTING_CONTROL_W     = 0.62;


	static const int TAB_FILTERS  = 0;
	static const int TAB_OBJECTS  = 1;
	static const int TAB_SETTINGS = 2;

	//! Filter groups, in display order.
	static const int GROUP_LIVING    = 0;
	static const int GROUP_VEHICLES  = 1;
	static const int GROUP_WEAPONS   = 2;
	static const int GROUP_ITEMS     = 3;
	static const int GROUP_IMMOVABLE = 4;
	static const int GROUP_OTHER     = 5;
	static const int GROUP_COUNT     = 6;

	//! Fraction of the group header row given to the name button. The chevron
	//! ahead of it is fixed-size and sits in the same left gutter the category
	//! rows put their palette button in, so the two columns line up.
	static const float GROUP_LABEL_W = 0.80;

	static const string CAT_MENU_ONLY     = "only";
	static const string CAT_MENU_GROUP_ON = "groupon";
	static const string CAT_MENU_GROUP_OFF= "groupoff";
	static const string CAT_MENU_COLOUR   = "colour";
	static const string CAT_MENU_RESET    = "resetcolour";
	static const string CAT_MENU_COPYPERM = "copyperm";


	protected autoptr array< ref JMESPViewTypeWidget > m_ESPTypeList;
	protected ref map<typename, JMESPViewTypeWidget> m_ESPTypeWidgetsByType;
	protected ref array< ref JMESPFilterGroup > m_FilterGroups;

	// -- Toolbar (panel_top) --------------------------------------------------
	protected ref UIActionFlexRow m_ToolbarRow;
	protected UIActionButton m_ToggleButton;
	protected UIActionSearchBox m_SearchBox;

	//! Every spawnable classname, scanned once and handed to the search box as
	//! its suggestion set. Null until the first keystroke - the walk covers
	//! four config trees and is far too slow to pay for at form open.
	protected ref TStringArray m_ClassnameSuggestions;

	// -- Filters tab ----------------------------------------------------------
	protected UIActionScroller m_ESPListScroller;
	protected UIActionSearchBox m_CategorySearch;
	protected UIActionContextMenu m_CategoryMenu;
	protected JMESPViewTypeWidget m_CategoryMenuTarget;

	//! ONE picker for the whole list, kept hidden and used only for its ARGB
	//! popup, which is anchored to whichever row's palette button opened it.
	//! Hiding a UIActionBase does not take it off the GUI update queue, so it
	//! still repositions its popup and still closes on an outside click.
	protected UIActionColorPicker m_CategoryPicker;
	protected JMESPViewTypeWidget m_ColourTarget;

	// -- Objects tab ----------------------------------------------------------
	protected UIActionScroller m_ESPSelectedObjects;
	protected UIActionItemList m_ObjectList;
	protected UIActionSearchBox m_ObjectSearch;
	protected UIActionCard m_ObjectsCard;
	protected UIActionImageButton m_ObjectListLockButton;

	//! Manual hold, set from the lock button. Independent of the automatic hold
	//! that focus applies, so releasing the mouse does not silently unpin a
	//! list the admin pinned on purpose.
	protected bool m_ObjectListPinned;

	//! What the lock button is currently drawing, so the icon is only swapped
	//! when the state actually changes rather than twice a second.
	protected bool m_ObjectListHeldShown;

	//! Last height OnResize reported, so the list can be re-sized when its row
	//! count changes without waiting for another resize.
	protected float m_ContentHeight;
	//! The same menu the world tags use, so a row in this list and the tag over
	//! the object it names offer the identical set of actions.
	protected ref JMESPActionMenu m_ObjectMenu;
	//! Parallel to the list's rows - the list itself only holds strings.
	protected ref array<JMESPMeta> m_ObjectRows;

	protected UIActionButton m_ExportButton;
	protected UIActionDropdown m_ExportTypeList;
	protected UIActionButton m_MoveToCursorButton;
	protected UIActionConfirmInline m_DeleteButton;

	//! Bulk selection, on both tabs. Feedback buttons because what they change
	//! is a list somewhere else on screen - see the note where they are built.
	protected UIActionFeedbackButton m_SelectAll;
	protected UIActionFeedbackButton m_DeselectAll;
	protected UIActionFeedbackButton m_EspSelectAll;
	protected UIActionFeedbackButton m_EspDeselectAll;

	// -- Settings tab ---------------------------------------------------------
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

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMESPModule m_Module;
	protected JMLoadoutModule m_LoadoutModule;

	protected UIActionTabs m_Tabs;
	protected Widget m_TabFiltersPanel;
	protected Widget m_TabObjectsPanel;
	protected Widget m_TabSettingsPanel;

	void JMESPForm()
	{
		m_ESPTypeList = new array< ref JMESPViewTypeWidget >;
		m_ESPTypeWidgetsByType = new map<typename, JMESPViewTypeWidget>;
		m_FilterGroups = new array< ref JMESPFilterGroup >;
		m_ObjectRows = new array<JMESPMeta>;

		JMScriptInvokers.ESP_VIEWTYPE_CHANGED.Insert( OnESPViewTypeChanged );
	}

	void ~JMESPForm()
	{
		JMScriptInvokers.ESP_VIEWTYPE_CHANGED.Remove( OnESPViewTypeChanged );
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		JMESPWidgetHandler.espMenu = this;

		InitWidgetsTop();
		InitWidgetsBottom();

		Class.CastTo( m_LoadoutModule, GetModuleManager().GetModule( JMLoadoutModule ) );
	}

	// =========================================================================
	//  Toolbar
	// =========================================================================

	protected void InitWidgetsTop()
	{
		Widget top = layoutRoot.FindAnyWidget( "panel_top" );

		m_ToolbarRow = UIActionManager.CreateFlexRow( top, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		Widget row = m_ToolbarRow.GetContent();

		m_ToggleButton = UIActionManager.CreateButton( row, "#STR_COT_ESP_MODULE_TOGGLE", this, "OnClick_UpdateESP" );
		m_ToggleButton.SetFlex( 0, 150, 150 );
		m_ToggleButton.SetIcon( JMConstants.Lucide( "eye" ) );
		m_ToggleButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_TOGGLE_ESP" );
		m_ToolbarRow.Add( m_ToggleButton );
		RegisterPermission( m_ToggleButton, "ESP.View" );

		//! UIActionSearchBox carries its own inline clear, so there is no
		//! separate 'x' button beside it any more.
		//! Placeholder names what it does, not what it is. "Filter:" said
		//! nothing about WHAT was being filtered, in a form that has two other
		//! search boxes filtering two other things.
		m_SearchBox = UIActionManager.CreateSearchBox( row, this, "OnChange_Filter", "#STR_COT_ESP_MODULE_CLASS_FILTER", m_Module.Filter );
		m_SearchBox.SetFlex( 1.0, 120 );
		m_SearchBox.SetTooltip( "#STR_COT_ESP_MODULE_TT_CLASS_FILTER" );
		m_ToolbarRow.Add( m_SearchBox );

		//! Anchored to the window root so the suggestion list floats over the
		//! tabs below instead of being clipped by the toolbar strip.
		if ( m_Window )
		{
			m_SearchBox.InitSuggestionList( m_Window.GetWidgetRoot() );
			m_SearchBox.SetMaxVisibleSuggestions( 10 );
			RegisterOverlay( m_SearchBox );
		}

		m_ToolbarRow.SetGap( 10 );

		//! Paint it now rather than waiting for the first UpdateUI tick, which
		//! is up to 500ms away - long enough to see the placeholder label.
		ApplyToggleFace();
	}

	protected void InitWidgetsBottom()
	{
		m_TabFiltersPanel  = layoutRoot.FindAnyWidget( "esp_filters_panel" );
		m_TabObjectsPanel  = layoutRoot.FindAnyWidget( "esp_objects_panel" );
		m_TabSettingsPanel = layoutRoot.FindAnyWidget( "esp_settings_panel" );

		ref array<string> tabLabels = { "#STR_COT_ESP_TAB_FILTERS", "#STR_COT_ESP_TAB_VISIBLE", "#STR_COT_ESP_TAB_SETTINGS" };
		ref array<string> tabIcons  = { JMConstants.Lucide( "filter" ), JMConstants.Lucide( "eye" ), JMConstants.Lucide( "settings" ) };

		m_Tabs = UIActionManager.CreateTabs( layoutRoot.FindAnyWidget( "panel_bottom_tabs" ), tabLabels, tabIcons, this, "OnChange_Tab" );

		m_Tabs.AddContent( m_TabFiltersPanel );
		m_Tabs.AddContent( m_TabObjectsPanel );
		m_Tabs.AddContent( m_TabSettingsPanel );

		InitTabState( 3 );

		m_Tabs.SetSelection( TAB_FILTERS, false );

		BuildTabIfNeeded( TAB_FILTERS );
	}

	// =========================================================================
	//  Filters tab
	// =========================================================================

	private void ESPFilters( Widget parent )
	{
		m_ESPListScroller = UIActionManager.CreateScroller( parent );
		Widget container = m_ESPListScroller.GetContentWidget();

		//! The header rows sit loose above the folds rather than inside a card:
		//! UIActionCard already spends the one Size-To-Content-V the chain is
		//! allowed, and every group below it is a self-measuring fold.
		Widget headerGrid = UIActionManager.CreateGridSpacer( container, 1, 2 );

		m_CategorySearch = UIActionManager.CreateSearchBox( headerGrid, this, "OnChange_CategoryFilter", "#STR_COT_ESP_MODULE_CATEGORY_FILTER" );
		m_CategorySearch.SetTooltip( "#STR_COT_ESP_MODULE_TT_CATEGORY_FILTER" );

		//! Feedback buttons: flipping a list of switches somewhere below the
		//! fold is an invisible result from where the button is, so the button
		//! says so itself rather than leaving the admin to scroll and check.
		Widget rowSelectors = UIActionManager.CreateGridSpacer( headerGrid, 1, 2 );
		m_EspSelectAll = UIActionManager.CreateFeedbackButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_SELECT_ALL", "#STR_COT_SELECTED", JMConstants.ICON_CHECK_MARK, this, "OnClick_ESPSelectAll" );
		m_EspSelectAll.SetIcon( JMConstants.ICON_SELECT_ALL );
		m_EspSelectAll.SetTooltip( "#STR_COT_ESP_MODULE_TT_ESP_SELECT_ALL" );
		m_EspDeselectAll = UIActionManager.CreateFeedbackButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_DESELECT_ALL", "#STR_COT_CLEARED", JMConstants.ICON_CHECK_MARK, this, "OnClick_ESPDeselectAll" );
		m_EspDeselectAll.SetIcon( JMConstants.ICON_DESELECT_ALL );
		m_EspDeselectAll.SetTooltip( "#STR_COT_ESP_MODULE_TT_ESP_DESELECT_ALL" );

		m_CategoryPicker = UIActionManager.CreateColorPicker( container, "#STR_COT_ESP_MODULE_CATEGORY_COLOUR", this, "OnChange_CategoryColour" );
		m_CategoryPicker.SetVisible( false );
		RegisterOverlay( m_CategoryPicker );

		UIActionManager.CreatePanel( container, JMTheme.DIVIDER_MEDIUM, 1 );

		BuildFilterGroups( container );

		m_ESPListScroller.UpdateScroller();
	}

	private void BuildFilterGroups( Widget container )
	{
		TStringArray groupLabels = {
			"#STR_COT_ESP_GROUP_LIVING",
			"#STR_COT_ESP_GROUP_VEHICLES",
			"#STR_COT_ESP_GROUP_WEAPONS",
			"#STR_COT_ESP_GROUP_ITEMS",
			"#STR_COT_ESP_GROUP_IMMOVABLE",
			"#STR_COT_ESP_GROUP_OTHER"
		};

		//! One line, not a block: Enforce cannot span an initializer that holds
		//! calls across lines.
		TStringArray groupIcons = { JMConstants.Lucide( "users" ), JMConstants.Lucide( "car" ), JMConstants.Lucide( "swords" ), JMConstants.Lucide( "package" ), JMConstants.Lucide( "mountain" ), JMConstants.Lucide( "shapes" ) };

		for ( int g = 0; g < GROUP_COUNT; g++ )
		{
			JMESPFilterGroup group = new JMESPFilterGroup();
			group.GroupLabel = groupLabels[g];
			group.GroupIcon  = groupIcons[g];

			//! WrapSpacer, not GridSpacer: a GridSpacer divides its width into
			//! equal columns and ignores a child's SetWidth, which is what put
			//! the chevron halfway across the row. Both halves fold the same
			//! group, so it does not matter which one is hit.
			Widget headerRow = UIActionManager.CreateWrapSpacer( container, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

			//! Same leading pad as the rows below, so the chevron column and the
			//! palette column line up instead of both hugging the scroller edge.
			Widget headerInset = UIActionManager.CreateSpacer( headerRow );
			UIActionManager.SetFixedSize( headerInset, JMESPViewTypeWidget.ROW_INSET_PX, JMESPViewTypeWidget.GUTTER_PX );

			group.GroupChevron = UIActionManager.CreateIconButton( headerRow, JMConstants.ICON_CHEVRON_DOWN, this, "OnClick_ToggleGroup" );
			group.GroupChevron.SetFixedSize( JMESPViewTypeWidget.GUTTER_PX, JMESPViewTypeWidget.GUTTER_PX );
			group.GroupChevron.SetData( group );

			//! Sits in the same column as the rows' switches, so a group can be
			//! turned on or off without unfolding it and the whole list reads
			//! as one switch column.
			group.GroupToggle = UIActionManager.CreateToggleSwitch( headerRow, "", this, "OnClick_GroupToggle", false );
			group.GroupToggle.SetFixedSize( JMESPViewTypeWidget.SWITCH_PX, JMESPViewTypeWidget.GUTTER_PX );
			group.GroupToggle.SetThumbIcon( group.GroupIcon );
			group.GroupToggle.SetData( group );

			//! No icon on the header button: the switch immediately to its left
			//! already carries the group icon on its thumb, and repeating it two
			//! cells apart reads as two different things rather than one.
			group.GroupHeader = UIActionManager.CreateButton( headerRow, group.GroupLabel, this, "OnClick_ToggleGroup" );
			group.GroupHeader.SetFlat( true );
			group.GroupHeader.SetWidth( GROUP_LABEL_W );
			group.GroupHeader.SetData( group );

			group.GroupFold = UIActionManager.CreateFoldPanel( container, this, "OnChange_GroupFold", false );

			//! A hairline under every group, so the list reads as blocks rather
			//! than one unbroken column of switches.
			UIActionManager.CreatePanel( container, JMTheme.DIVIDER_MEDIUM, 1 );

			m_FilterGroups.Insert( group );
		}

		auto viewTypes = m_Module.GetViewTypes();

		foreach ( auto viewType: viewTypes )
		{
			JMESPFilterGroup owner = m_FilterGroups[GroupIndexFor( viewType )];

			JMESPViewTypeWidget rScript = new JMESPViewTypeWidget();
			rScript.Build( owner.GroupFold.GetContent(), viewType, this );

			m_ESPTypeList.Insert( rScript );
			m_ESPTypeWidgetsByType[viewType.Type()] = rScript;
			owner.GroupRows.Insert( rScript );

			RegisterPermission( rScript, "ESP.View." + viewType.Permission );
		}

		//! Living and Vehicles open, the long tails shut: those two are what an
		//! admin reaches for first, and six open groups is a page of scrolling
		//! before the first row of anything.
		for ( int o = 0; o < m_FilterGroups.Count(); o++ )
			SetGroupExpanded( m_FilterGroups[o], o < GROUP_WEAPONS, false );

		RefreshGroupHeaders();
	}

	//! Most-specific first: the weapon, item and immovable trees are all
	//! JMESPViewType subclasses, so testing the bases first would swallow every
	//! leaf. Anything a sub-mod adds that matches nothing lands in Other rather
	//! than vanishing from the list.
	private int GroupIndexFor( JMESPViewType viewType )
	{
		typename t = viewType.Type();

		if ( t.IsInherited( JMESPViewTypeWeapon ) )
			return GROUP_WEAPONS;

		if ( t.IsInherited( JMESPViewTypeItemBase ) )
			return GROUP_ITEMS;

		if ( t.IsInherited( JMESPViewTypeImmovable ) )
			return GROUP_IMMOVABLE;

		if ( t.IsInherited( JMESPViewTypeCar ) || t.IsInherited( JMESPViewTypeBoat ) || t.IsInherited( JMESPViewTypeTrain ) )
			return GROUP_VEHICLES;

		if ( t.IsInherited( JMESPViewTypePlayer ) || t.IsInherited( JMESPViewTypePlayerAI ) )
			return GROUP_LIVING;

		if ( t.IsInherited( JMESPViewTypeInfected ) || t.IsInherited( JMESPViewTypeAnimal ) )
			return GROUP_LIVING;

		return GROUP_OTHER;
	}

	//! Glyph baked into a category row's switch thumb. Keyed on typename the
	//! same way GroupIndexFor is, and for the same reason: a sub-mod's own view
	//! type falls through to the generic marker instead of rendering blank.
	//!
	//! Most-specific first - every weapon and item leaf also passes the base
	//! test, so the bases have to be asked last.
	static string IconForViewType( JMESPViewType viewType )
	{
		typename t = viewType.Type();

		if ( t == JMESPViewTypePlayer )   return JMConstants.Lucide( "user" );
		if ( t == JMESPViewTypePlayerAI ) return JMConstants.Lucide( "bot" );
		if ( t == JMESPViewTypeInfected ) return JMConstants.Lucide( "skull" );
		if ( t == JMESPViewTypeAnimal )   return JMConstants.Lucide( "rabbit" );

		if ( t == JMESPViewTypeCar )   return JMConstants.Lucide( "car" );
		if ( t == JMESPViewTypeBoat )  return JMConstants.Lucide( "ship" );
		if ( t == JMESPViewTypeTrain ) return JMConstants.Lucide( "train-front" );

		if ( t == JMESPViewTypeArchery )         return JMConstants.Lucide( "bow-arrow" );
		if ( t == JMESPViewTypePistol )          return JMConstants.Lucide( "target" );
		if ( t == JMESPViewTypeLauncher )        return JMConstants.Lucide( "rocket" );
		if ( t == JMESPViewTypeRifle )           return JMConstants.Lucide( "crosshair" );
		if ( t == JMESPViewTypeBoltRifle )       return JMConstants.Lucide( "crosshair" );
		if ( t == JMESPViewTypeBoltActionRifle ) return JMConstants.Lucide( "crosshair" );

		if ( t == JMESPViewTypeTent )         return JMConstants.Lucide( "tent" );
		if ( t == JMESPViewTypeBaseBuilding ) return JMConstants.Lucide( "blocks" );
		if ( t == JMESPViewTypeFood )         return JMConstants.Lucide( "apple" );
		if ( t == JMESPViewTypeExplosive )    return JMConstants.Lucide( "bomb" );
		if ( t == JMESPViewTypeBook )         return JMConstants.Lucide( "book" );
		if ( t == JMESPViewTypeContainer )    return JMConstants.Lucide( "box" );
		if ( t == JMESPViewTypeTransmitter )  return JMConstants.Lucide( "radio" );
		if ( t == JMESPViewTypeClothing )     return JMConstants.Lucide( "shirt" );
		if ( t == JMESPViewTypeMagazine )     return JMConstants.Lucide( "layers" );
		if ( t == JMESPViewTypeAmmo )         return JMConstants.Lucide( "shell" );
		if ( t == JMESPViewTypeUnknown )      return JMConstants.Lucide( "circle-help" );

		if ( t == JMESPViewTypeBuilding )    return JMConstants.Lucide( "building" );
		if ( t == JMESPViewTypeRock )        return JMConstants.Lucide( "gem" );
		if ( t == JMESPViewTypeTree )        return JMConstants.Lucide( "tree-pine" );
		if ( t == JMESPViewTypeBush )        return JMConstants.Lucide( "shrub" );
		if ( t == JMESPViewTypePlainObject ) return JMConstants.Lucide( "square" );

		if ( t.IsInherited( JMESPViewTypeWeapon ) )    return JMConstants.Lucide( "swords" );
		if ( t.IsInherited( JMESPViewTypeItemBase ) )  return JMConstants.Lucide( "package" );
		if ( t.IsInherited( JMESPViewTypeImmovable ) ) return JMConstants.Lucide( "mountain" );

		return JMConstants.Lucide( "map-pin" );
	}

	private void SetGroupExpanded( JMESPFilterGroup group, bool expanded, bool animate = true )
	{
		if ( !group || !group.GroupFold )
			return;

		group.GroupFold.SetExpanded( expanded, animate );

		//! Swapped rather than turned: UIActionImageButton drives its image's
		//! rotation itself for the spin animation, so a static angle written
		//! here would be overwritten the next time one played.
		if ( group.GroupChevron )
		{
			if ( expanded )
				group.GroupChevron.SetImage( JMConstants.ICON_CHEVRON_DOWN );
			else
				group.GroupChevron.SetImage( JMConstants.ICON_CHEVRON_RIGHT );
		}
	}

	//! Header text carries the enabled/total count, and a group whose every row
	//! is hidden - by the search or by permission - goes away with them.
	private void RefreshGroupHeaders()
	{
		foreach ( JMESPFilterGroup group : m_FilterGroups )
		{
			int visible = 0;
			int enabled = 0;

			foreach ( JMESPViewTypeWidget row : group.GroupRows )
			{
				if ( row.IsFiltered() || !row.IsPermitted() )
					continue;

				visible++;

				if ( row.IsChecked() )
					enabled++;
			}

			bool anyVisible = visible > 0;

			group.GroupHeader.SetVisible( anyVisible );
			group.GroupFold.SetVisible( anyVisible );

			if ( group.GroupChevron )
				group.GroupChevron.SetVisible( anyVisible );

			//! The group switch reads as ON only when every visible row under it
			//! is on, so a part-enabled group does not claim to be enabled.
			if ( group.GroupToggle )
			{
				group.GroupToggle.SetVisible( anyVisible );
				group.GroupToggle.SetChecked( visible > 0 && enabled >= visible );
			}

			group.GroupHeader.SetButton( string.Format( "%1  (%2/%3)", Widget.TranslateString( group.GroupLabel ), enabled, group.GroupRows.Count() ) );
		}
	}

	void OnClick_ToggleGroup( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMESPFilterGroup group;
		if ( !Class.CastTo( group, action.GetData() ) )
			return;

		SetGroupExpanded( group, !group.GroupFold.IsExpanded() );
	}

	//! Quick-toggle the whole group from its header. The switch has already
	//! flipped itself, so its new state is the state to push down.
	void OnClick_GroupToggle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMESPFilterGroup group;
		if ( !Class.CastTo( group, action.GetData() ) )
			return;

		SetGroupChecked( group, group.GroupToggle.IsChecked() );

		RefreshGroupHeaders();
	}

	//! A fold reports CHANGE on every frame its height moves, which is exactly
	//! when the scroller's extent is wrong.
	void OnChange_GroupFold( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_ESPListScroller )
			m_ESPListScroller.UpdateScroller();
	}

	void OnChange_CategoryFilter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		ApplyCategoryFilter( action.GetText() );
	}

	private void ApplyCategoryFilter( string text )
	{
		string needle = text;
		needle.ToLower();

		foreach ( JMESPFilterGroup group : m_FilterGroups )
		{
			int matches = 0;

			foreach ( JMESPViewTypeWidget row : group.GroupRows )
			{
				bool hit = row.MatchesFilter( needle );

				row.SetFiltered( !hit );

				if ( hit )
					matches++;
			}

			//! A search that hits inside a shut group has to open it, or the
			//! result is invisible. Clearing the search leaves every group
			//! where the user last put it.
			if ( needle != "" )
				SetGroupExpanded( group, matches > 0, false );
		}

		RefreshGroupHeaders();

		if ( m_ESPListScroller )
			m_ESPListScroller.UpdateScroller();
	}

	//! Called by a row when its switch changes, so the group count keeps up.
	void OnChange_CategoryRow()
	{
		RefreshGroupHeaders();
	}

	//! Called by a row's colour picker. The module owns the write so the choice
	//! reaches the JSON as well as the live view type.
	void ApplyCategoryColour( JMESPViewTypeWidget row, int colour )
	{
		if ( !row )
			return;

		m_Module.SetViewTypeColour( row.GetViewType(), colour );
		row.RefreshSwatch();
	}

	//! Raise the shared ARGB popup over `anchor` - the palette button of the
	//! row that asked - and point it at that row's colour.
	void OpenCategoryColourPopup( JMESPViewTypeWidget row, Widget anchor )
	{
		if ( !m_CategoryPicker || !row || !row.GetViewType() )
			return;

		m_ColourTarget = row;

		m_CategoryPicker.SetPopupAnchorWidget( anchor );
		m_CategoryPicker.SetColor( row.GetViewType().Colour );
		m_CategoryPicker.Open();
	}

	void OnChange_CategoryColour( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( !m_ColourTarget )
			return;

		ApplyCategoryColour( m_ColourTarget, m_CategoryPicker.GetColor() );
	}

	// -------------------------------------------------------------------------
	//  Category row context menu
	// -------------------------------------------------------------------------

	void OnRightClick_CategoryRow( JMESPViewTypeWidget row )
	{
		if ( !row )
			return;

		if ( !m_CategoryMenu )
		{
			if ( !m_Window )
				return;

			m_CategoryMenu = UIActionManager.CreateContextMenu( layoutRoot, m_Window.GetWidgetRoot(), this, "OnClick_CategoryMenu" );
			RegisterOverlay( m_CategoryMenu );

			if ( !m_CategoryMenu )
				return;
		}

		m_CategoryMenuTarget = row;

		m_CategoryMenu.ClearItems();
		m_CategoryMenu.AddItem( CAT_MENU_ONLY,      "#STR_COT_ESP_MODULE_MENU_ONLY_THIS",  JMConstants.Lucide( "scan" ) );
		m_CategoryMenu.AddItem( CAT_MENU_GROUP_ON,  "#STR_COT_ESP_MODULE_MENU_GROUP_ON",   JMConstants.ICON_SELECT_ALL );
		m_CategoryMenu.AddItem( CAT_MENU_GROUP_OFF, "#STR_COT_ESP_MODULE_MENU_GROUP_OFF",  JMConstants.ICON_DESELECT_ALL );
		m_CategoryMenu.AddItem( CAT_MENU_COLOUR,    "#STR_COT_ESP_MODULE_CATEGORY_COLOUR", JMConstants.Lucide( "palette" ) );
		m_CategoryMenu.AddItem( CAT_MENU_RESET,     "#STR_COT_ESP_MODULE_MENU_RESET_ONE",  JMConstants.Lucide( "rotate-ccw" ) );
		m_CategoryMenu.AddItem( CAT_MENU_COPYPERM,  "#STR_COT_ESP_MODULE_MENU_COPY_PERM",  JMConstants.ICON_STACK );

		int mx;
		int my;
		GetMousePos( mx, my );

		m_CategoryMenu.ShowAt( mx, my );
	}

	void OnClick_CategoryMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_CategoryMenuTarget )
			return;

		string id = m_CategoryMenu.GetLastClickedId();

		if ( id == CAT_MENU_ONLY )
			EnableOnly( m_CategoryMenuTarget );

		if ( id == CAT_MENU_GROUP_ON )
			SetGroupChecked( GroupOf( m_CategoryMenuTarget ), true );

		if ( id == CAT_MENU_GROUP_OFF )
			SetGroupChecked( GroupOf( m_CategoryMenuTarget ), false );

		if ( id == CAT_MENU_COLOUR )
			m_CategoryMenuTarget.OpenColourEditor();

		if ( id == CAT_MENU_RESET )
			ResetOneColour( m_CategoryMenuTarget );

		if ( id == CAT_MENU_COPYPERM )
			g_Game.CopyToClipboard( "ESP.View." + m_CategoryMenuTarget.GetViewType().Permission );

		RefreshGroupHeaders();
	}

	private JMESPFilterGroup GroupOf( JMESPViewTypeWidget row )
	{
		if ( !row || !row.GetViewType() )
			return NULL;

		return m_FilterGroups[GroupIndexFor( row.GetViewType() )];
	}

	private void SetGroupChecked( JMESPFilterGroup group, bool state )
	{
		if ( !group )
			return;

		foreach ( JMESPViewTypeWidget row : group.GroupRows )
		{
			if ( !row.IsFiltered() && row.IsPermitted() )
				row.SetChecked( state );
		}
	}

	private void EnableOnly( JMESPViewTypeWidget target )
	{
		foreach ( JMESPViewTypeWidget row : m_ESPTypeList )
		{
			if ( row.IsPermitted() )
				row.SetChecked( row == target );
		}
	}

	private void ResetOneColour( JMESPViewTypeWidget row )
	{
		JMESPViewType viewType = row.GetViewType();

		if ( !viewType )
			return;

		m_Module.ResetViewTypeColour( viewType );
		row.RefreshSwatch();
	}

	// =========================================================================
	//  Objects tab
	// =========================================================================

	private void ESPSelectedObjects( Widget parent )
	{
		m_ESPSelectedObjects = UIActionManager.CreateScroller( parent );
		Widget container = m_ESPSelectedObjects.GetContentWidget();

		//! The list and the things you do to a selection are two different
		//! jobs, so they get a card each rather than a loose search row over a
		//! bare list. The count lives in the card's own title, which is where
		//! the eye already is and costs no row of its own.
		//!
		//! Selection first, tracked objects under it: the actions are a fixed
		//! three rows while the list grows to whatever the scan returned, so
		//! putting the list first pushes the buttons off the bottom of the tab.
		UIActionCard card = UIActionManager.CreateCard( container, "#STR_COT_ESP_MODULE_SELECTION_HEADER" );
		Widget cardContent = card.GetContent();

		//! Three even two-up rows, so the card reads as a block of paired
		//! actions instead of a ragged mix of full and half width buttons.
		Widget rowSelectors = UIActionManager.CreateGridSpacer( cardContent, 1, 2 );
		m_SelectAll = UIActionManager.CreateFeedbackButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_SELECT_ALL", "#STR_COT_SELECTED", JMConstants.ICON_CHECK_MARK, this, "OnClick_SelectAll" );
		m_SelectAll.SetIcon( JMConstants.ICON_SELECT_ALL );
		m_SelectAll.SetTooltip( "#STR_COT_ESP_MODULE_TT_OBJECT_SELECT_ALL" );
		m_DeselectAll = UIActionManager.CreateFeedbackButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_DESELECT_ALL", "#STR_COT_CLEARED", JMConstants.ICON_CHECK_MARK, this, "OnClick_DeselectAll" );
		m_DeselectAll.SetIcon( JMConstants.ICON_DESELECT_ALL );
		m_DeselectAll.SetTooltip( "#STR_COT_ESP_MODULE_TT_OBJECT_DESELECT_ALL" );

		Widget rowMisc = UIActionManager.CreateGridSpacer( cardContent, 1, 2 );
		m_MoveToCursorButton = UIActionManager.CreateButton( rowMisc, "#STR_COT_ESP_MODULE_ACTION_MOVE_TO_CURSOR", this, "OnClick_MoveToCursor" );
		m_MoveToCursorButton.SetIcon( JMConstants.Lucide( "mouse-pointer-click" ) );
		m_MoveToCursorButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_MOVE_TO_CURSOR" );
		RegisterPermission( m_MoveToCursorButton, "ESP.Object.MoveToCursor" );

		m_DeleteButton = UIActionManager.CreateConfirmInline( rowMisc, "#STR_COT_GENERIC_DELETE", this, "OnClick_DeleteSelected" );
		UIActionIconGrid.ApplyDeletePreset( m_DeleteButton );
		m_DeleteButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_DELETE_SELECTED" );
		RegisterPermission( m_DeleteButton, "ESP.Object.DeleteAll" );

		//! Format first, then the button that acts on it - the row reads in the
		//! order it is used.
		Widget rowExports = UIActionManager.CreateGridSpacer( cardContent, 1, 2 );

		TStringArray exportChoices = {
			"#STR_COT_ESP_MODULE_EXPORT_RAW",
			"#STR_COT_ESP_MODULE_EXPORT_SPAWNABLETYPES",
		#ifdef DZ_Expansion_Market
			"#STR_COT_ESP_MODULE_EXPORT_EXP_MARKET",
		#endif
		#ifdef DZ_Expansion_Core
			"#STR_COT_ESP_MODULE_EXPORT_EXP_LOADOUT",
		#endif
			"#STR_COT_LOADOUT_MODULE_NAME"
		};
		m_ExportTypeList = UIActionManager.CreateDropdown( rowExports, "", parent, this, "OnClick_ExportType", exportChoices );
		RegisterOverlay( m_ExportTypeList );

		//! Without this the dropdown opens blank, which reads as "no format
		//! chosen" even though Raw is what the button would actually do.
		m_ExportTypeList.SetSelection( COT_ESPMode.COPYLISTRAW, false );

		m_ExportButton = UIActionManager.CreateButton( rowExports, "#STR_COT_TO_CLIPBOARD", this, "OnClick_CopyToClipboard" );
		m_ExportButton.SetIcon( JMConstants.ICON_STACK );
		m_ExportButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_EXPORT_SELECTED" );

		m_ObjectsCard = UIActionManager.CreateCard( container, "#STR_COT_ESP_MODULE_OBJECTS_CARD" );
		Widget objectsContent = m_ObjectsCard.GetContent();

		m_ObjectsCard.AddRefreshButton( this, "OnClick_RefreshObjects" );

		m_ObjectListLockButton = m_ObjectsCard.AddCardHeaderAction( JMConstants.Lucide( "lock-open" ), this, "OnClick_ToggleListLock", "#STR_COT_ESP_MODULE_LIST_LOCK" );

		m_ObjectSearch = UIActionManager.CreateSearchBox( objectsContent, this, "OnChange_ObjectFilter", "#STR_COT_ESP_MODULE_OBJECT_FILTER" );
		m_ObjectSearch.SetTooltip( "#STR_COT_ESP_MODULE_TT_OBJECT_FILTER" );

		//! Virtualized: only the visible rows exist as widgets, so a scan that
		//! comes back with a few thousand objects costs a few thousand strings
		//! rather than a few thousand layouts.
		m_ObjectList = UIActionManager.CreateItemList( objectsContent, this, "OnClick_ObjectRow" );
		ApplyObjectListHeight();

		RefreshObjectList();

		m_ESPSelectedObjects.UpdateScroller();
	}

	//! Built from the module's tracked list rather than from a walk of
	//! JMESPMeta.s_JM_All: the linked list also holds metas queued for creation
	//! and destruction, which are not on screen.
	void RefreshObjectList( bool force = false )
	{
		if ( !m_ObjectList )
			return;

		RefreshListLockIndicator();

		if ( !force && IsObjectListHeld() )
			return;

		array< ref JMESPMeta > active = m_Module.GetActiveObjects();

		string needle = "";
		if ( m_ObjectSearch )
			needle = m_ObjectSearch.GetText();
		needle.ToLower();

		m_ObjectRows.Clear();

		TStringArray names = new TStringArray;
		TStringArray subs  = new TStringArray;

		vector camera = g_Game.GetCurrentCameraPosition();

		foreach ( JMESPMeta meta : active )
		{
			if ( !meta || !meta.target )
				continue;

			string label = meta.GetName();
			string classname = meta.GetType();

			if ( needle != "" )
			{
				string labelLower = label;
				labelLower.ToLower();

				string classLower = classname;
				classLower.ToLower();

				if ( labelLower.IndexOf( needle ) < 0 && classLower.IndexOf( needle ) < 0 )
					continue;
			}

			int distance = Math.Round( vector.Distance( camera, meta.target.GetPosition() ) );

			string category = classname;
			if ( meta.type )
				category = Widget.TranslateString( meta.type.Localisation );

			names.Insert( label );
			subs.Insert( string.Format( "%1  -  %2m", category, distance ) );

			m_ObjectRows.Insert( meta );
		}

		m_ObjectList.SetItems( names, subs );

		ApplyObjectListHeight();

		RefreshObjectCardTitle( names.Count() );
	}

	//! Card title carries the count, and says so when the list is being held.
	private void RefreshObjectCardTitle( int count )
	{
		if ( !m_ObjectsCard )
			return;

		string title = string.Format( "%1 (%2)", Widget.TranslateString( "#STR_COT_ESP_MODULE_OBJECTS_CARD" ), count );

		if ( m_ObjectListHeldShown )
			title = string.Format( "%1  -  %2", title, Widget.TranslateString( "#STR_COT_ESP_MODULE_LIST_HELD" ) );

		m_ObjectsCard.SetLabel( title );
	}

	void OnClick_RefreshObjects( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		action.AnimateSpin();

		//! An explicit refresh beats the hold - that is what it is for.
		RefreshObjectList( true );
	}

	//! True while the list must not be rebuilt under the admin.
	//!
	//! A 500ms rebuild re-sorts and re-indexes every row, which makes the list
	//! impossible to work: the entry under the cursor is gone by the time it is
	//! clicked. Touching the list holds it automatically, and the lock button
	//! holds it indefinitely.
	bool IsObjectListHeld()
	{
		if ( m_ObjectListPinned )
			return true;

		return m_ObjectList && m_ObjectList.IsFocused();
	}

	void OnClick_ToggleListLock( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_ObjectListPinned = !m_ObjectListPinned;

		RefreshListLockIndicator();

		//! Unpinning is a request to see current data now, not in half a
		//! second.
		if ( !m_ObjectListPinned )
			RefreshObjectList( true );
	}

	//! Icon and title marker follow the EFFECTIVE hold, so a list held by the
	//! cursor says so just as plainly as one held by the button.
	private void RefreshListLockIndicator()
	{
		bool held = IsObjectListHeld();

		if ( held == m_ObjectListHeldShown )
			return;

		m_ObjectListHeldShown = held;

		if ( !m_ObjectListLockButton )
			return;

		if ( held )
		{
			m_ObjectListLockButton.SetImage( JMConstants.Lucide( "lock" ) );
			m_ObjectListLockButton.SetColor( JMTheme.WARNING );
		}
		else
		{
			m_ObjectListLockButton.SetImage( JMConstants.Lucide( "lock-open" ) );
			m_ObjectListLockButton.SetColor( JMTheme.TEXT_SECONDARY );
		}

		RefreshObjectCardTitle( m_ObjectRows.Count() );
	}

	//! Height of the object list: as many rows as it actually holds, clamped to
	//! [MIN_ROWS, MAX_ROWS] and then to whatever the window can spare. An empty
	//! list is four rows tall, not half the form.
	private void ApplyObjectListHeight()
	{
		if ( !m_ObjectList )
			return;

		int rows = Math.Clamp( m_ObjectList.GetItemCount(), OBJECT_LIST_MIN_ROWS, OBJECT_LIST_MAX_ROWS );

		float listHeight = rows * UIActionItemList.ROW_H;

		float available = m_ContentHeight - TOOLBAR_HEIGHT - TAB_STRIP_HEIGHT - OBJECT_LIST_CHROME;
		float floorPx   = OBJECT_LIST_MIN_ROWS * UIActionItemList.ROW_H;

		if ( available < floorPx )
			available = floorPx;

		if ( listHeight > available )
			listHeight = available;

		m_ObjectList.SetViewportHeight( listHeight );
	}

	void OnChange_ObjectFilter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		RefreshObjectList( true );
	}

	void OnClick_ObjectRow( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		int idx = m_ObjectList.GetSelectedIndex();

		if ( idx < 0 || idx >= m_ObjectRows.Count() )
			return;

		JMESPMeta meta = m_ObjectRows[idx];

		if ( !meta || !meta.widgetHandler )
			return;

		if ( m_ObjectList.GetLastClickButton() == MouseState.RIGHT )
		{
			ShowObjectMenu( meta );
			return;
		}

		meta.widgetHandler.Select();
	}

	private void ShowObjectMenu( JMESPMeta meta )
	{
		if ( !m_ObjectMenu )
		{
			if ( !m_Window )
				return;

			m_ObjectMenu = new JMESPActionMenu();

			//! Anchored to the window root, not to the list: a menu parented to
			//! the list would be clipped by it on the bottom rows.
			UIActionContextMenu menu = m_ObjectMenu.Init( layoutRoot, m_Window.GetWidgetRoot() );

			if ( !menu )
			{
				m_ObjectMenu = NULL;
				return;
			}

			RegisterOverlay( menu );
		}

		int mx;
		int my;
		GetMousePos( mx, my );

		m_ObjectMenu.Open( meta, mx, my );
	}

	// =========================================================================
	//  Settings tab
	// =========================================================================

	private void ESPSettings( Widget parent )
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
		RegisterOverlay( m_PlayerSkeletons );

		int idx = m_Module.GetDrawPlayerSkeletonsEnabled();
		if ( idx )
			idx += m_Module.DrawPlayerSkeletonsIncludingMyself;
		m_PlayerSkeletons.SetSelection( idx, false );

		//! A slider, not a dropdown: four steps on a continuum read better as a
		//! continuum, and the fill grows with the thickness it sets.
		//!
		//! The track spans 0..4 so a thickness of 1 reads as a quarter of it
		//! rather than as empty, but 0 is not a thickness anything can be drawn
		//! at, so the handler floors the value at MIN_LINE_THICKNESS.
		m_SkeletonLineThickness = UIActionManager.CreateSlider( displayContent, "#STR_COT_GENERIC_LINE_THICKNESS", 0.0, 4.0, this, "OnChange_Skeleton_LineThickness" );
		m_SkeletonLineThickness.SetCurrent( m_Module.SkeletonLineThickness );
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
		//! so this row has the identical shape to the Auto Refresh row below -
		//! [ SETTING_LABEL_W ][ SETTING_ROW_REMAINDER, track filling it ]. Built
		//! that way the two tracks start AND end on the same vertical lines by
		//! construction; matching them by hand-picked track fractions only ever
		//! lines up one of the two edges, because a track is right-aligned
		//! inside its own widget.
		Widget radiusRow = UIActionManager.CreateWrapSpacer( scanContent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		UIActionText radiusLabel = UIActionManager.CreateText( radiusRow, "#STR_COT_ESP_MODULE_RADIUS" );
		radiusLabel.SetWidth( SETTING_LABEL_W );

		m_RadiusSlider = UIActionManager.CreateSlider( radiusRow, "", 0, m_Module.GetMaxRadius(), this, "OnChange_Range" );
		m_RadiusSlider.SetWidth( SETTING_ROW_REMAINDER );
		m_RadiusSlider.SetCurrent( m_Module.ESPRadius );
		m_RadiusSlider.SetFormat( "#STR_COT_FORMAT_METRE_LONG" );
		m_RadiusSlider.SetStepValue( 10.0 );
		m_RadiusSlider.SetSliderWidth( 1.0 );

		//! Auto-refresh sits with the interval it governs rather than in the
		//! toolbar: the switch and the rate are one setting read together.
		Widget refreshRow = UIActionManager.CreateWrapSpacer( scanContent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_AutoRefreshToggle = UIActionManager.CreateToggleSwitch( refreshRow, "#STR_COT_ESP_MODULE_TOGGLE_AUTO_REFRESH", this, "OnClick_UpdateAtRate", m_Module.GetState() == JMESPState.Update );
		m_AutoRefreshToggle.SetWidth( SETTING_LABEL_W );
		m_AutoRefreshToggle.SetTooltip( "#STR_COT_ESP_MODULE_TT_AUTO_REFRESH" );

		//! No label of its own: the switch to its left already names the
		//! setting, and a second "Refresh rate:" in the same row only competes
		//! with it. The slider still prints its own formatted value.
		m_RefreshSlider = UIActionManager.CreateSlider( refreshRow, "", 1.0, 10.0, this, "OnChange_UpdateRate" );
		m_RefreshSlider.SetWidth( SETTING_ROW_REMAINDER );
		m_RefreshSlider.SetCurrent( m_Module.ESPUpdateTime );
		m_RefreshSlider.SetFormat( "#STR_COT_FORMAT_SECOND_LONG" );
		m_RefreshSlider.SetStepValue( 1.0 );

		//! Fills its own cell, which starts at SETTING_LABEL_W because the
		//! switch beside it is that wide - so the track begins on the same
		//! vertical line as the Radius track above. Anything short of 1.0 here
		//! would also open a gap between the switch and the rate it governs,
		//! since the track is right-aligned inside the slider widget.
		m_RefreshSlider.SetSliderWidth( 1.0 );

		Widget limitRow = UIActionManager.CreateWrapSpacer( scanContent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		UIActionText limitModeText = UIActionManager.CreateText( limitRow, "#STR_COT_ESP_MODULE_LIMIT_MODE" );
		limitModeText.SetWidth( SETTING_LABEL_W );

		m_DisableSafetyCheckbox = UIActionManager.CreateButtonToggle( limitRow, "#STR_COT_ESP_MODULE_LIMIT_SAFE", "#STR_COT_ESP_MODULE_LIMIT_ALL", this, "OnClick_DisableSafety" );
		m_DisableSafetyCheckbox.SetWidth( SETTING_ROW_REMAINDER );
		m_DisableSafetyCheckbox.SetToggle( m_Module.GetFilterSafetyState() );
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

	void OnClick_ResetColours( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Module.ResetViewTypeColours();

		foreach ( JMESPViewTypeWidget row : m_ESPTypeList )
			row.RefreshSwatch();
	}

	// =========================================================================
	//  Tabs & lifecycle
	// =========================================================================

	private void BuildTabIfNeeded( int tabIdx )
	{
		if ( !ShouldBuildTab( tabIdx ) )
			return;

		switch ( tabIdx )
		{
			case TAB_FILTERS:  ESPFilters( m_TabFiltersPanel );          break;
			case TAB_OBJECTS:  ESPSelectedObjects( m_TabObjectsPanel );  break;
			case TAB_SETTINGS: ESPSettings( m_TabSettingsPanel );        break;
		}

		UpdateUI();
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

		if ( IsTabActive( TAB_OBJECTS ) )
			RefreshObjectList();
	}

	override void OnResize( float w, float h )
	{
		super.OnResize( w, h );

		PinStripGeometry( layoutRoot.FindAnyWidget( "panel_bottom_tabs" ), layoutRoot.FindAnyWidget( "panel_bottom_content" ), h - TOOLBAR_HEIGHT, TAB_STRIP_HEIGHT );

		if ( m_ToolbarRow )
			m_ToolbarRow.Layout();

		m_ContentHeight = h;

		ApplyObjectListHeight();

		if ( m_ESPListScroller )    m_ESPListScroller.UpdateScroller();
		if ( m_ESPSelectedObjects ) m_ESPSelectedObjects.UpdateScroller();
	}

	override void OnShow()
	{
		super.OnShow();

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateUI, 500, true );

		UpdateUI();
	}

	override void OnHide()
	{
		super.OnHide();

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( UpdateUI );
	}

	//! Kept because JMESPModule.OnClientPermissionsUpdated calls it. The
	//! per-control work it used to stand in for is now done by the base's
	//! registered-permission walk.
	void DisableToggleableOptions()
	{
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();

		RefreshGroupHeaders();

		UpdateUI();
	}

	void OnESPViewTypeChanged( JMESPViewType viewType )
	{
		UpdateMaxRange();
	}

	//! Paint the toolbar toggle to say, in one glance, whether ESP is currently
	//! drawing and what clicking will do about it.
	//!
	//! The old face said "Clear ESP" / "Show ESP", which are both descriptions
	//! of the ACTION - so the only way to read the current state was to work
	//! backwards from the verb. Now the label names the state ("ESP is on"),
	//! the icon shows it (open eye / struck-through eye), the fill colour
	//! reinforces it (success while on, neutral while off) and the tooltip
	//! carries the action ("click to hide"). State on the button, action in the
	//! tooltip - the opposite way round from before.
	protected void ApplyToggleFace()
	{
		if ( !m_ToggleButton )
			return;

		if ( m_Module.GetState() != JMESPState.Remove )
		{
			m_ToggleButton.SetButton( "#STR_COT_ESP_MODULE_STATE_ON" );
			m_ToggleButton.SetIcon( JMConstants.Lucide( "eye" ) );
			m_ToggleButton.SetColor( JMTheme.SUCCESS_FILL );
			m_ToggleButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_STATE_ON" );
		}
		else
		{
			m_ToggleButton.SetButton( "#STR_COT_ESP_MODULE_STATE_OFF" );
			m_ToggleButton.SetIcon( JMConstants.Lucide( "eye-off" ) );
			m_ToggleButton.SetColor( JMTheme.BUTTON_FILL );
			m_ToggleButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_STATE_OFF" );
		}
	}

	//! Runs on a 500ms timer while the form is open, so every widget it touches
	//! can belong to a tab that has never been built.
	void UpdateUI()
	{
		if ( m_RadiusSlider )  m_RadiusSlider.SetCurrent( m_Module.ESPRadius );
		if ( m_RefreshSlider ) m_RefreshSlider.SetCurrent( m_Module.ESPUpdateTime );

		if ( IsTabActive( TAB_OBJECTS ) )
			RefreshObjectList();

		ApplyToggleFace();

		if ( m_RefreshSlider )
		{
			//! Enable/disable only. The old code also repainted the track
			//! TEXT_PRIMARY, which is near-white - that is where the washed-out
			//! slider came from, and Disable() already carries the disabled
			//! styling.
			m_RefreshSlider.SetEnabledIf( m_AutoRefreshToggle && m_AutoRefreshToggle.IsChecked() );
		}
	}

	//! Feed the search box its suggestion set, building it on the first call.
	//!
	//! This used to walk all four config trees on EVERY keystroke to find a
	//! single closest match to show as ghost text. The same walk now runs once
	//! and hands the whole set to UIActionSearchBox, which prefix-filters it
	//! live and offers the matches as a real pickable list.
	void UpdateList()
	{
		if ( !m_SearchBox )
			return;

		if ( m_ClassnameSuggestions )
			return;

		m_ClassnameSuggestions = new TStringArray;

		TStringArray configs = new TStringArray;
		configs.Insert( CFG_VEHICLESPATH );
		configs.Insert( CFG_WEAPONSPATH );
		configs.Insert( CFG_MAGAZINESPATH );
		configs.Insert( CFG_NONAI_VEHICLES );

		for ( int nConfig = 0; nConfig < configs.Count(); nConfig++ )
		{
			string strConfigPath = configs.Get( nConfig );

			int nClasses = g_Game.ConfigGetChildrenCount( strConfigPath );

			//! The first twenty entries of the vehicles tree are the abstract
			//! bases, which are not spawnable and never worth suggesting.
			int nClassStart = 0;
			if ( nConfig == 0 ) nClassStart = 20;

			for ( int nClass = nClassStart; nClass < nClasses; nClass++ )
			{
				string strName;

				g_Game.ConfigGetChildName( strConfigPath, nClass, strName );

				int scope = g_Game.ConfigGetInt( strConfigPath + " " + strName + " scope" );

				if ( scope == 0 )
					continue;

				if ( !g_Game.ConfigIsExisting( strConfigPath + " " + strName + " model" ) )
					continue;

				m_ClassnameSuggestions.Insert( strName );
			}
		}

		m_SearchBox.SetSuggestions( m_ClassnameSuggestions );
	}

	// =========================================================================
	//  Handlers
	// =========================================================================

	void OnClick_UpdateESP( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !GetPermissionsManager().HasPermission( "ESP.View" ) )
			return;

		if ( m_Module.GetState() != JMESPState.Remove )
		{
			m_Module.UpdateState( JMESPState.Remove );
			m_Module.Log( "Clearing ESP" );
		} else
		{
			if ( m_AutoRefreshToggle && m_AutoRefreshToggle.IsChecked() )
			{
				m_Module.UpdateState( JMESPState.Update );
				m_Module.Log( "ESP updating" );
			}
			else
			{
				m_Module.UpdateState( JMESPState.View );
				m_Module.Log( "Viewing ESP" );
			}
		}

		UpdateUI();
	}

	void OnChange_Filter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateList();

		m_Module.Filter = action.GetText();
	}

	void OnChange_UpdateRate( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Module.ESPUpdateTime = action.GetCurrent();
	}

	void OnChange_Range( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Module.ESPRadius = action.GetCurrent();
	}

	void OnClick_DisableSafety( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetFilterSafetyState( m_DisableSafetyCheckbox.IsToggled() );

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

	void UpdateMaxRange()
	{
		float maxRadius = m_Module.GetMaxRadius();
		if ( m_Module.ESPRadius > maxRadius )
			m_Module.ESPRadius = maxRadius;
		if ( m_RadiusSlider )
		{
			m_RadiusSlider.SetMinMax( m_RadiusSlider.GetMin(), maxRadius );
			m_RadiusSlider.SetCurrent( m_Module.ESPRadius );
		}
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

		if ( m_Module.GetState() != JMESPState.Remove )
		{
			if ( m_AutoRefreshToggle.IsChecked() )
			{
				if ( m_RefreshSlider ) m_RefreshSlider.Enable();
				m_Module.UpdateState( JMESPState.Update );
			}
			else
			{
				if ( m_RefreshSlider ) m_RefreshSlider.Disable();
				m_Module.UpdateState( JMESPState.View );
			}
		}

		UpdateUI();
	}

	void OnChange_PlayerSkeletons( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		int idx = action.GetSelection();

		//! Skeletons are drawn off the player view types, so turning them on
		//! while no player category is enabled would draw nothing.
		JMESPViewTypeWidget aiRow = m_ESPTypeWidgetsByType[JMESPViewTypePlayerAI];
		JMESPViewTypeWidget playerRow = m_ESPTypeWidgetsByType[JMESPViewTypePlayer];

		if ( idx > 0 && aiRow && playerRow && !playerRow.IsChecked() && !aiRow.IsChecked() )
			playerRow.SetChecked( true );

		m_Module.SetDrawPlayerSkeletonsEnabled( idx > 0 );
		m_Module.DrawPlayerSkeletonsIncludingMyself = idx > 1;

		if ( m_SkeletonLineThickness )
			m_SkeletonLineThickness.SetVisible( idx > 0 );

		RefreshGroupHeaders();
	}

	void OnClick_ExportType( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_ExportTypeList.GetSelection() == COT_ESPMode.CREATELOADOUT )
			m_ExportButton.SetButton( "#STR_COT_SAVE_AS" );
		else
			m_ExportButton.SetButton( "#STR_COT_TO_CLIPBOARD" );
	}

	void OnChange_Skeleton_LineThickness( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( action.GetCurrent() < MIN_LINE_THICKNESS )
			action.SetCurrent( MIN_LINE_THICKNESS );

		m_Module.SkeletonLineThickness = action.GetCurrent();

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

	void OnClick_DuplicateSelected( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !GetPermissionsManager().HasPermission( "ESP.Object.DuplicateAll" ) )
			return;

		m_Module.DuplicateSelected();
	}

	void OnClick_ESPDeselectAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		//! Only what is on screen: a search narrowed the list on purpose, and
		//! toggling categories the admin cannot even see is a surprise.
		foreach ( JMESPViewTypeWidget espType: m_ESPTypeList )
		{
			if ( !espType.IsFiltered() && espType.IsPermitted() )
				espType.SetChecked( false );
		}

		RefreshGroupHeaders();
	}

	void OnClick_ESPSelectAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		foreach ( JMESPViewTypeWidget espType: m_ESPTypeList )
		{
			if ( !espType.IsFiltered() && espType.IsPermitted() )
				espType.SetChecked( true );
		}

		RefreshGroupHeaders();
	}

	void OnClick_DeleteSelected( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( !GetPermissionsManager().HasPermission( "ESP.Object.DeleteAll" ) )
			return;

		CreateAdvancedObjectConfirm( "DeleteSelected", "DeleteSelected" );
	}

	void DeleteSelected()
	{
		m_Module.DeleteSelected();

		RefreshObjectList();
	}

	void OnClick_SelectAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		auto node = JMESPMeta.s_JM_All.m_Head;
		while ( node )
		{
			if ( node.m_Value.widgetRoot.IsVisible() )
				node.m_Value.widgetHandler.Select();

			node = node.m_Next;
		}
	}

	void OnClick_DeselectAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		auto node = JMESPMeta.s_JM_All.m_Head;
		while ( node )
		{
			node.m_Value.widgetHandler.Deselect();
			node = node.m_Next;
		}

		JM_GetSelected().ClearObjects();
	}

	void OnClick_MoveToCursor( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !GetPermissionsManager().HasPermission( "ESP.Object.MoveToCursor" ) )
			return;

		CreateAdvancedObjectConfirm( "MoveToCursor", "MoveToCursor" );
	}

	void MoveToCursor()
	{
		vector dir = g_Game.GetCurrentCameraDirection();
		vector from = g_Game.GetCurrentCameraPosition();
		vector to = from + ( dir * 1000 );
		vector contact_pos;
		vector contact_dir;
		int contact_component;

		if ( DayZPhysics.RaycastRV( from, to, contact_pos, contact_dir, contact_component, NULL, NULL, NULL, false, true ) )
			m_Module.MoveToCursor( contact_pos );
	}

	void OnClick_CopyToClipboard( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		switch ( m_ExportTypeList.GetSelection() )
		{
			case COT_ESPMode.COPYLISTRAW:
				m_Module.CopyToClipboardRaw();
			break;
			case COT_ESPMode.COPYLISTSPAWNABLETYPES:
				m_Module.CopyToClipboardSpawnableTypes();
			break;
		#ifdef DZ_Expansion_Market
			case COT_ESPMode.COPYLISTEXPMARKET:
				m_Module.CopyToClipboardMarket();
			break;
		#endif
		#ifdef DZ_Expansion_Core
			case COT_ESPMode.COPYLISTEXPLOADOUT:
				m_Module.CopyToClipboardExpLoadout( JMSelectedObject );
			break;
		#endif
			case COT_ESPMode.CREATELOADOUT:
				CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_ESP_MODULE_LOADOUT_MESSAGE_HEADER", "#STR_COT_ESP_MODULE_LOADOUT_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CREATE", "CreateLoadout_Confirm" );
			break;
		}
	}

	void CreateLoadout_Confirm( JMConfirmation confirmation )
	{
		string name = confirmation.GetEditBoxValue();
		if ( name == string.Empty )
			return;

		if ( !m_LoadoutModule )
			Class.CastTo( m_LoadoutModule, GetModuleManager().GetModule( JMLoadoutModule ) );

		m_LoadoutModule.Create( name );
	}

	protected string m_PendingMsgPlayerGUID;

	void PromptSendMessage( string playerGUID, string playerName )
	{
		m_PendingMsgPlayerGUID = playerGUID;
		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_MESSAGE_HEADER", Widget.TranslateString( "#STR_COT_ESP_MODULE_MESSAGE_PROMPT" ) + " " + playerName + ":", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "OnSendMessage_Confirm" );
	}

	void OnSendMessage_Confirm( JMConfirmation confirmation )
	{
		string msgText = confirmation.GetEditBoxValue();
		if ( msgText == "" || m_PendingMsgPlayerGUID == "" )
			return;

		JMPlayerModule playerModule = JMPlayerModule.Cast( GetModuleManager().GetModule( JMPlayerModule ) );
		if ( playerModule )
		{
			playerModule.DoMessage( { m_PendingMsgPlayerGUID }, msgText );
		}
		m_PendingMsgPlayerGUID = "";
	}
}
