//! The item category filter - a filter button (id / group menu) plus a strip of recently used
//! categories - shared by forms that filter item classes (Object Spawner, Loot Analysis).
//! A form owns a picker, points it at the strip's wrapper widget, calls Toggle( button ) on
//! click and is told the active category id ("" = everything). Mods add menu rows with JMFilterRegistry.Register.
class JMItemCategoryPicker
{
	//! Categories are presented in groups, each opening a submenu, the way the
	//! teleport filter is organised. Flat, the 27 rows needed two columns to fit
	//! the window and still read as a wall of icons.
	static const string GROUP_GEAR     = "grp_gear";
	static const string GROUP_WEAPONS  = "grp_weapons";
	static const string GROUP_SURVIVAL = "grp_survival";
	static const string GROUP_WORLD    = "grp_world";

	//! The "everything" category is the empty string, which the menu cannot use
	//! as an id - an empty GetLastClickedId() is also what a menu that has never
	//! been clicked reports. Only the root row needs this: every group member id
	//! (CategoryGroupMembers) is already a non-empty string usable as-is.
	static const string MENU_ID_ALL = "__all";
	static const string PAGE_ROOT   = "root";

	//! Id of the optional checkbox at the bottom of the root page (see SetFooterToggle).
	static const string MENU_ID_FOOTER = "__footer";

	//! How many recently used categories get a chip.
	static const int RECENT_MAX = 4;
	protected JMFormBase m_Form;
	protected Widget m_RecentWrapper;
	protected string m_RegistryScope;
	protected Class m_Target;
	protected string m_ChangeFn;
	protected string m_Current;
	protected ref UIActionFilterMenu m_Menu;
	protected string m_FooterLabel;
	protected bool m_FooterOn;
	protected Class m_FooterTarget;
	protected string m_FooterFn;
	protected ref TStringArray m_RecentIds = new TStringArray;
	protected ref array<ref UIActionButton> m_RecentButtons = new array<ref UIActionButton>;

	//! `recentWrapper` hosts the chip strip (null = no chips). `changeFn` is `void Fn( string categoryId )` on
	//! `target`, called whenever the active category changes. `current` is the category active at start.
	void JMItemCategoryPicker( JMFormBase form, Widget recentWrapper, string registryScope, Class target, string changeFn, string current = "" )
	{
		m_Form = form;
		m_RecentWrapper = recentWrapper;
		m_RegistryScope = registryScope;
		m_Target = target;
		m_ChangeFn = changeFn;
		m_Current = current;
	}

	// -------------------------------------------------------------------------
	//  State
	// -------------------------------------------------------------------------

	//! Adds a checkbox as the last row of the root page, for a filter that is not a
	//! category. `fn` is `void Fn( bool on )` on `target`, called when it is toggled.
	void SetFooterToggle( string label, bool on, Class target, string fn )
	{
		m_FooterLabel = label;
		m_FooterOn = on;
		m_FooterTarget = target;
		m_FooterFn = fn;
	}

	//! The active category id ("" = everything).
	string GetCurrent()
	{
		return m_Current;
	}

	//! Change the active category from code (no change callback) and repaint.
	void SetCurrent( string id )
	{
		m_Current = id;

		if ( m_Menu )
			m_Menu.Refresh();

		PaintRecent();
	}

	// -------------------------------------------------------------------------
	//  Categories
	// -------------------------------------------------------------------------

	//! Id / icon / label for every category, in menu order. The id is the config base
	//! class or script type (case sensitive!) the list filters on; "" is everything. One table read by both the menu and
	//! the recent chips, so the two cannot drift apart.
	static void CategoryTable( out TStringArray ids, out TStringArray icons, out TStringArray labels )
	{
		ids = new TStringArray;
		icons = new TStringArray;
		labels = new TStringArray;

		AddCategory( ids, icons, labels, "",               JMConstants.Lucide( "layers" ),    "#STR_COT_OBJECT_CATEGORY_ALL" );
		AddCategory( ids, icons, labels, "Edible_Base",    JMConstants.ICON_MEAT,             "#STR_COT_OBJECT_CATEGORY_FOOD" );
		AddCategory( ids, icons, labels, "Bottle_Base",    JMConstants.ICON_BEER_BOTTLE,      "#STR_COT_OBJECT_CATEGORY_DRINKS" );
		AddCategory( ids, icons, labels, "Transport",      JMConstants.ICON_JEEP,             "#STR_COT_OBJECT_CATEGORY_VEHICLES" );
		AddCategory( ids, icons, labels, "Weapon_Base",    JMConstants.ICON_FAMAS,            "#STR_COT_OBJECT_CATEGORY_FIREARMS" );
		//AddCategory( ids, icons, labels, "meleeweapon",    JMConstants.ICON_GLADIUS,          "#STR_COT_OBJECT_CATEGORY_MELEE" );  //! what's this supposed to be? doesn't exist!
		AddCategory( ids, icons, labels, "Magazine_Base",  JMConstants.ICON_MACHINE_GUN_MAG,  "#STR_COT_OBJECT_CATEGORY_AMMO" );
		AddCategory( ids, icons, labels, "Clothing_Base",  JMConstants.ICON_CLOTHES,          "#STR_COT_OBJECT_CATEGORY_CLOTHING" );
		AddCategory( ids, icons, labels, "HeadGear_Base",  JMConstants.ICON_STAHLHELM,        "#STR_COT_OBJECT_CATEGORY_HEADGEAR" );
		AddCategory( ids, icons, labels, "Mask_Base",      JMConstants.ICON_BALACLAVA,        "#STR_COT_OBJECT_CATEGORY_MASKS" );
		AddCategory( ids, icons, labels, "Glasses_Base",   JMConstants.ICON_SUNGLASSES,       "#STR_COT_OBJECT_CATEGORY_GLASSES" );
		AddCategory( ids, icons, labels, "Top_Base",       JMConstants.ICON_T_SHIRT,          "#STR_COT_OBJECT_CATEGORY_TOPS" );
		AddCategory( ids, icons, labels, "Pants_Base",     JMConstants.ICON_TROUSERS,         "#STR_COT_OBJECT_CATEGORY_PANTS" );
		AddCategory( ids, icons, labels, "Vest_Base",      JMConstants.ICON_BELT,             "#STR_COT_OBJECT_CATEGORY_VESTS" );
		AddCategory( ids, icons, labels, "Gloves_Base",    JMConstants.ICON_GLOVES,           "#STR_COT_OBJECT_CATEGORY_GLOVES" );
		AddCategory( ids, icons, labels, "Shoes_Base",     JMConstants.ICON_TROUSERS,         "#STR_COT_OBJECT_CATEGORY_SHOES" );
		AddCategory( ids, icons, labels, "Backpack_Base",  JMConstants.ICON_LIGHT_BACKPACK,   "#STR_COT_OBJECT_CATEGORY_BACKPACKS" );
		AddCategory( ids, icons, labels, "Container_Base", JMConstants.ICON_KNAPSACK,         "#STR_COT_OBJECT_CATEGORY_CONTAINERS" );
		AddCategory( ids, icons, labels, "Inventory_Base", JMConstants.ICON_FULL_FOLDER,      "#STR_COT_OBJECT_CATEGORY_ITEMS" );
		//AddCategory( ids, icons, labels, "itemmedical",    JMConstants.ICON_MEDICINES,        "#STR_COT_OBJECT_CATEGORY_MEDICAL" );  //! what's this supposed to be? doesn't exist!
		AddCategory( ids, icons, labels, "ToolBase",       JMConstants.ICON_SHARP_AXE,        "#STR_COT_OBJECT_CATEGORY_TOOLS" );
		AddCategory( ids, icons, labels, "TrapBase",       JMConstants.ICON_TINKER,           "#STR_COT_OBJECT_CATEGORY_TRAPS" );
		//AddCategory( ids, icons, labels, "electricdevice", JMConstants.ICON_ELECTRIC,         "#STR_COT_OBJECT_CATEGORY_ELECTRONICS" );  //! what's this supposed to be? doesn't exist!
		AddCategory( ids, icons, labels, "BaseBuildingBase", JMConstants.Lucide( "hammer" ),  "#STR_COT_ESP_MODULE_VIEW_TYPE_BASE_BUILDING" );
		AddCategory( ids, icons, labels, "TentBase",       JMConstants.ICON_CAMPING_TENT,     "#STR_COT_OBJECT_CATEGORY_TENTS" );
		AddCategory( ids, icons, labels, "Grenade_Base",   JMConstants.ICON_UNLIT_BOMB,       "#STR_COT_OBJECT_CATEGORY_EXPLOSIVES" );
		AddCategory( ids, icons, labels, "House",          JMConstants.ICON_HOME_GARAGE,      "#STR_COT_OBJECT_CATEGORY_BUILDINGS" );
		AddCategory( ids, icons, labels, "DZ_LightAI",     JMConstants.ICON_SHAMBLING_ZOMBIE, "#STR_COT_OBJECT_CATEGORY_AI" );
	}

	static void CategoryGroupTable( out TStringArray ids, out TStringArray icons, out TStringArray labels )
	{
		ids = new TStringArray;
		icons = new TStringArray;
		labels = new TStringArray;

		AddCategory( ids, icons, labels, GROUP_GEAR,     JMConstants.ICON_CLOTHES, "#STR_COT_OBJECT_CATEGORY_GROUP_GEAR" );
		AddCategory( ids, icons, labels, GROUP_WEAPONS,  JMConstants.ICON_FAMAS,   "#STR_COT_OBJECT_CATEGORY_GROUP_WEAPONS" );
		AddCategory( ids, icons, labels, GROUP_SURVIVAL, JMConstants.ICON_MEAT,    "#STR_COT_OBJECT_CATEGORY_GROUP_SURVIVAL" );
		AddCategory( ids, icons, labels, GROUP_WORLD,    JMConstants.ICON_JEEP,    "#STR_COT_OBJECT_CATEGORY_GROUP_WORLD" );
	}

	//! Every id here must also appear in CategoryTable, and between them the four
	//! groups must cover it entirely - a category in neither place can no longer
	//! be reached from the filter button at all.
	static TStringArray CategoryGroupMembers( string groupId )
	{
		TStringArray members = new TStringArray;

		if ( groupId == GROUP_GEAR )
		{
			members.Insert( "Clothing_Base" );
			members.Insert( "HeadGear_Base" );
			members.Insert( "Mask_Base" );
			members.Insert( "Glasses_Base" );
			members.Insert( "Top_Base" );
			members.Insert( "Pants_Base" );
			members.Insert( "Vest_Base" );
			members.Insert( "Gloves_Base" );
			members.Insert( "Shoes_Base" );
			members.Insert( "Backpack_Base" );
			return members;
		}

		if ( groupId == GROUP_WEAPONS )
		{
			members.Insert( "Weapon_Base" );
			//members.Insert( "meleeweapon" );
			members.Insert( "Magazine_Base" );
			members.Insert( "Grenade_Base" );
			members.Insert( "TrapBase" );
			return members;
		}

		if ( groupId == GROUP_SURVIVAL )
		{
			members.Insert( "Edible_Base" );
			members.Insert( "Bottle_Base" );
			//members.Insert( "itemmedical" );
			members.Insert( "ToolBase" );
			members.Insert( "BaseBuildingBase" );
			members.Insert( "TentBase" );
			members.Insert( "Container_Base" );
			return members;
		}

		if ( groupId == GROUP_WORLD )
		{
			members.Insert( "Transport" );
			members.Insert( "House" );
			members.Insert( "DZ_LightAI" );
			//members.Insert( "electricdevice" );
			members.Insert( "Inventory_Base" );
			return members;
		}

		return members;
	}

	static void AddCategory( TStringArray ids, TStringArray icons, TStringArray labels, string id, string icon, string label )
	{
		ids.Insert( id );
		icons.Insert( icon );
		labels.Insert( label );
	}

	static string GroupOfCategory( string categoryId )
	{
		TStringArray gids, gicons, glabels;
		CategoryGroupTable( gids, gicons, glabels );

		for ( int i = 0; i < gids.Count(); i++ )
		{
			TStringArray members = CategoryGroupMembers( gids[i] );

			if ( members.Find( categoryId ) >= 0 )
				return gids[i];
		}

		return "";
	}

	static string CategoryLabel( string id )
	{
		TStringArray ids, icons, labels;
		CategoryTable( ids, icons, labels );

		for ( int i = 0; i < ids.Count(); i++ )
		{
			if ( ids[i] == id )
				return labels[i];
		}

		return "";
	}

	static string CategoryIcon( string id )
	{
		TStringArray ids, icons, labels;
		CategoryTable( ids, icons, labels );

		for ( int i = 0; i < ids.Count(); i++ )
		{
			if ( ids[i] == id )
				return icons[i];
		}

		return JMConstants.Lucide( "layers" );
	}

	//! A category was picked (menu, chip): remember it, keep the chip strip in step and tell the form.
	void Select( string id )
	{
		m_Current = id;

		//! A chip press must never destroy the strip it landed on. Adding a category the
		//! strip lacks changes its shape, so the rebuild is deferred a tick; the engine
		//! answers a vanished press target by recentring the cursor.
		if ( PushRecent( id ) )
			g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Call( RebuildRecent );
		else
			PaintRecent();

		//! The menu marks what is active, so it follows a chip click as well as
		//! its own - a chip press leaves the menu open on whatever page it
		//! was showing, and that page's colours are now stale.
		if ( m_Menu )
			m_Menu.Refresh();

		if ( m_Target && m_ChangeFn != "" )
		{
			auto param = new Param1<string>( id );
			GetGame().GameScript.CallFunctionParams( m_Target, m_ChangeFn, null, param );
		}
	}

	// -------------------------------------------------------------------------
	//  Menu
	// -------------------------------------------------------------------------

	//! Open the category menu under `button` (closes it if already open). Categories
	//! are one UIActionFilterMenu - a root page (All + the four groups) and one page
	//! per group - rather than a menu plus a second one for the group's members.
	//! Picking a category is a command, not a toggle, so every page closes on pick.
	void Toggle( notnull Widget button )
	{
		if ( !m_Menu )
		{
			m_Menu = UIActionManager.CreateOverlayFilterMenu( m_Form, button, m_RegistryScope );

			if ( !m_Menu )
			{
				Error("[JMItemCategoryPicker] Toggle failed: Could not create UIActionFilterMenu (m_Menu is null)!");
				return;
			}

			m_Menu.AddPage( PAGE_ROOT, this, "BuildRootPage", "", false, "", this, "OnRootChange", true );
			m_Menu.AddPage( GROUP_GEAR,     this, "BuildGroupPage", PAGE_ROOT, false, "", this, "OnGroupChange", true );
			m_Menu.AddPage( GROUP_WEAPONS,  this, "BuildGroupPage", PAGE_ROOT, false, "", this, "OnGroupChange", true );
			m_Menu.AddPage( GROUP_SURVIVAL, this, "BuildGroupPage", PAGE_ROOT, false, "", this, "OnGroupChange", true );
			m_Menu.AddPage( GROUP_WORLD,    this, "BuildGroupPage", PAGE_ROOT, false, "", this, "OnGroupChange", true );
		}

		m_Menu.ToggleAt( button );
	}

	void BuildRootPage( UIActionFilterMenu menu )
	{
		//! 0 means "use the menu default", so only the active row names a
		//! colour of its own. A group is marked when the active category is
		//! one of its members, which is the only cue while its page is shut.
		int allColor = 0;

		if ( m_Current == "" )
			allColor = JMTheme.ACCENT;

		menu.AddRow( MENU_ID_ALL, CategoryLabel( "" ), CategoryIcon( "" ), allColor );

		TStringArray gids, gicons, glabels;
		CategoryGroupTable( gids, gicons, glabels );

		string activeGroup = GroupOfCategory( m_Current );

		for ( int i = 0; i < gids.Count(); i++ )
		{
			int color = 0;

			if ( gids[i] == activeGroup )
				color = JMTheme.ACCENT;

			menu.AddRow( gids[i], glabels[i], gicons[i], color, true, gids[i] );
		}

		if ( m_FooterLabel != "" )
		{
			menu.AddToggleRow( MENU_ID_FOOTER, m_FooterLabel, m_FooterOn );
			menu.KeepRowOpen( MENU_ID_FOOTER );
		}
	}

	//! Only ever reached for MENU_ID_ALL - the four group rows above switch
	//! page instead, intercepted by UIActionFilterMenu before this runs.
	void OnRootChange( string id )
	{
		if ( id == MENU_ID_ALL )
		{
			Select( "" );
		}
		else if ( id == MENU_ID_FOOTER )
		{
			m_FooterOn = !m_FooterOn;

			if ( m_FooterTarget && m_FooterFn != "" )
			{
				auto param = new Param1<bool>( m_FooterOn );
				GetGame().GameScript.CallFunctionParams( m_FooterTarget, m_FooterFn, null, param );
			}
		}
	}

	void BuildGroupPage( UIActionFilterMenu menu )
	{
		TStringArray members = CategoryGroupMembers( menu.CurrentPage() );

		for ( int i = 0; i < members.Count(); i++ )
		{
			string memberId = members[i];
			string memberLabel = CategoryLabel( memberId );
			string memberIcon = CategoryIcon( memberId );
			int color = 0;

			if ( memberId == m_Current )
				color = JMTheme.ACCENT;

			menu.AddRow( memberId, memberLabel, memberIcon, color );
		}
	}

	void OnGroupChange( string id )
	{
		Select( id );
	}

	// -------------------------------------------------------------------------
	//  Recent chips
	// -------------------------------------------------------------------------

	//! Newest first, capped. Returns whether the strip's contents changed. Re-picking a
	//! category the strip ALREADY holds leaves the order alone, as promoting it would slide
	//! every other chip under the cursor. "All" gets no chip - it is the first menu row.
	protected bool PushRecent( string id )
	{
		if ( id == "" )
			return false;

		if ( m_RecentIds.Find( id ) >= 0 )
			return false;

		m_RecentIds.InsertAt( id, 0 );

		while ( m_RecentIds.Count() > RECENT_MAX )
			m_RecentIds.Remove( m_RecentIds.Count() - 1 );

		return true;
	}

	//! Rebuild the chip strip. Only ever needed when the SET of recent
	//! categories changed - a plain re-selection repaints instead.
	void RebuildRecent()
	{
		if ( !m_RecentWrapper )
			return;

		//! Drop the script references BEFORE the widgets that own them go, so a
		//! chip is never left alive with a layoutRoot that has been unlinked.
		m_RecentButtons.Clear();

		Widget child = m_RecentWrapper.GetChildren();

		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}

		if ( m_RecentIds.Count() == 0 )
			return;

		Widget strip = UIActionManager.CreateWrapSpacerCompact( m_RecentWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		//! Each chip takes an equal FRACTION of the row. The icon grid this
		//! replaced was given an exact pixel width computed from the chip count
		//! and an authored cell size, which had to stay in step with both and
		//! spilled out of its band when it did not.
		float chipW = 0.99 / m_RecentIds.Count();

		for ( int i = 0; i < m_RecentIds.Count(); i++ )
		{
			UIActionButton chip = UIActionManager.CreateButton( strip, CategoryLabel( m_RecentIds[i] ), this, "" );
			if ( chip ) chip.SetOnClick( this, "OnClick_RecentChip" );

			if ( !chip )
				continue;

			chip.SetWidth( chipW );
			chip.SetIcon( CategoryIcon( m_RecentIds[i] ) );

			m_RecentButtons.Insert( chip );
		}

		PaintRecent();
	}

	//! Mark whichever chip matches the active category.
	protected void PaintRecent()
	{
		for ( int i = 0; i < m_RecentButtons.Count(); i++ )
		{
			if ( i >= m_RecentIds.Count() )
				return;

			//! A WASH rather than the solid accent: UIActionButton has no way to
			//! recolour its own label, and the default one is unreadable on a
			//! saturated fill.
			if ( m_RecentIds[i] == m_Current )
				m_RecentButtons[i].SetColor( JMTheme.ACCENT_WASH_STRONG );
			else
				m_RecentButtons[i].SetColor( JMTheme.INK_700 );
		}
	}

	//! Which chip was pressed, from the action that fired. The buttons carry no
	//! id of their own - their position in m_RecentButtons IS their position in
	//! m_RecentIds, because the two are built in one pass.
	void OnClick_RecentChip( UIActionBase action )
	{
		UIActionButton chip;

		if ( !Class.CastTo( chip, action ) )
			return;

		int index = m_RecentButtons.Find( chip );

		if ( index < 0 || index >= m_RecentIds.Count() )
			return;

		Select( m_RecentIds[index] );
	}
}
