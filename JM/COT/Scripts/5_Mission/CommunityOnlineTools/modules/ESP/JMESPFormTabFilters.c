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

	//! Weak: the rows are owned by JMESPFormTabFilters.m_ESPTypeList.
	ref array<JMESPViewTypeWidget> GroupRows;

	void JMESPFilterGroup()
	{
		GroupRows = new array<JMESPViewTypeWidget>;
	}
}

//! "Filters" tab of JMESPForm - the ~30 view types grouped by their own
//! inheritance tree, each foldable, with a search box, bulk select/deselect
//! and a per-row context menu (colour, only-this, group on/off, copy
//! permission). Back-reference to the owning form, same shape as
//! JMPlayerRowWidget.Menu.
//!
//! JMESPViewTypeWidget.c holds a JMESPForm-typed back-reference (not this
//! class) and calls OnRightClick_CategoryRow/OnChange_CategoryRow/
//! OpenCategoryColourPopup on it directly - JMESPForm forwards those three
//! into this class, so JMESPViewTypeWidget.c never needed to change.
class JMESPFormTabFilters
{
	protected JMESPForm m_Form;

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

	protected autoptr array< ref JMESPViewTypeWidget > m_ESPTypeList = new array< ref JMESPViewTypeWidget >;
	protected ref map<typename, JMESPViewTypeWidget> m_ESPTypeWidgetsByType = new map<typename, JMESPViewTypeWidget>;
	protected ref array< ref JMESPFilterGroup > m_FilterGroups = new array< ref JMESPFilterGroup >;

	protected UIActionScroller m_ESPListScroller;
	protected UIActionSearchBox m_CategorySearch;
	protected UIActionContextMenu m_CategoryMenu;
	protected JMESPViewTypeWidget m_CategoryMenuTarget;

	//! ONE picker for the whole list, kept hidden and used only for its ARGB
	//! popup, which is anchored to whichever row's palette button opened it.
	protected UIActionColorPicker m_CategoryPicker;
	protected JMESPViewTypeWidget m_ColourTarget;

	protected UIActionFeedbackButton m_EspSelectAll;
	protected UIActionFeedbackButton m_EspDeselectAll;

	void JMESPFormTabFilters( JMESPForm form )
	{
		m_Form = form;
	}

	void Build( Widget parent )
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
		m_Form.RegisterOverlay( m_CategoryPicker );

		UIActionManager.CreatePanel( container, JMTheme.DIVIDER_MEDIUM, 1 );

		BuildFilterGroups( container );

		m_ESPListScroller.UpdateScroller();
	}

	void OnResize()
	{
		if ( m_ESPListScroller )
			m_ESPListScroller.UpdateScroller();
	}

	array< ref JMESPViewTypeWidget > GetTypeList()
	{
		return m_ESPTypeList;
	}

	JMESPViewTypeWidget GetTypeWidget( typename t )
	{
		return m_ESPTypeWidgetsByType[t];
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

		auto viewTypes = m_Form.m_Module.GetViewTypes();

		foreach ( auto viewType: viewTypes )
		{
			JMESPFilterGroup owner = m_FilterGroups[GroupIndexFor( viewType )];

			JMESPViewTypeWidget rScript = new JMESPViewTypeWidget();
			rScript.Build( owner.GroupFold.GetContent(), viewType, m_Form );

			m_ESPTypeList.Insert( rScript );
			m_ESPTypeWidgetsByType[viewType.Type()] = rScript;
			owner.GroupRows.Insert( rScript );

			m_Form.RegisterPermission( rScript, JMConstants.PERM_ESP_VIEW + "." + viewType.Permission );
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
	void RefreshGroupHeaders()
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

	//! Refresh every row's colour swatch - called from the Settings tab's
	//! Reset Colours action.
	void RefreshAllSwatches()
	{
		foreach ( JMESPViewTypeWidget row : m_ESPTypeList )
			row.RefreshSwatch();
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

		m_Form.m_Module.SetViewTypeColour( row.GetViewType(), colour );
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
			CF_Window window = m_Form.GetWindow();
			if ( !window )
				return;

			m_CategoryMenu = UIActionManager.CreateContextMenu( m_Form.GetLayoutRoot(), window.GetWidgetRoot(), this, "OnClick_CategoryMenu" );
			m_Form.RegisterOverlay( m_CategoryMenu );

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

		m_Form.m_Module.ResetViewTypeColour( viewType );
		row.RefreshSwatch();
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
}
