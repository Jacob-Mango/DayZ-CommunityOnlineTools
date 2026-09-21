//! The item category filter - a filter button (id / group menu) plus a strip of the most recently used
//! categories - shared by every form that filters a list of item classes (Object Spawner, Loot Analysis).
//!
//! One table of categories, one grouped menu (All + four groups, each a page of UIActionFilterMenu), one chip
//! strip; a form owns a picker, points it at the strip's wrapper widget and is told which category is active:
//!
//!     m_Categories = new JMItemCategoryPicker( this, m_RecentWrapper, JMFilterRegistry.OBJECTS, this, "OnCategoryChanged" );
//!     ...
//!     void OnClick_CategoryFilter( UIEvent eid, UIActionBase action )
//!     {
//!         if ( eid == UIEvent.CLICK )
//!             m_Categories.Toggle( m_FilterButton.GetLayoutRoot() );
//!     }
//!
//!     void OnCategoryChanged( string categoryId )   // "" = everything, else the config base class to filter on
//!     {
//!         m_CurrentType = categoryId;
//!         UpdateList();
//!     }
//!
//! Mods add rows to the menu with JMFilterRegistry.Register( registryScope, ... ).
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

	//! How many recently used categories get a chip.
	static const int RECENT_MAX = 4;
	protected JMFormBase m_Form;
	protected Widget m_RecentWrapper;
	protected string m_RegistryScope;
	protected Class m_Target;
	protected string m_ChangeFn;
	protected string m_Current;
	protected ref UIActionFilterMenu m_Menu;
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

	//! Id / icon / label for every category, in menu order. The id is the config
	//! base class the list filters on; "" is everything.
	//!
	//! One table, read by both the menu and the recent chips, so the two can
	//! never drift apart.
	static void CategoryTable( out TStringArray ids, out TStringArray icons, out TStringArray labels )
	{
		ids = new TStringArray;
		icons = new TStringArray;
		labels = new TStringArray;

		AddCategory( ids, icons, labels, "",               JMConstants.Lucide( "layers" ),    "#STR_COT_OBJECT_CATEGORY_ALL" );
		AddCategory( ids, icons, labels, "edible_base",    JMConstants.ICON_MEAT,             "#STR_COT_OBJECT_CATEGORY_FOOD" );
		AddCategory( ids, icons, labels, "bottle_base",    JMConstants.ICON_BEER_BOTTLE,      "#STR_COT_OBJECT_CATEGORY_DRINKS" );
		AddCategory( ids, icons, labels, "transport",      JMConstants.ICON_JEEP,             "#STR_COT_OBJECT_CATEGORY_VEHICLES" );
		AddCategory( ids, icons, labels, "weapon_base",    JMConstants.ICON_FAMAS,            "#STR_COT_OBJECT_CATEGORY_FIREARMS" );
		AddCategory( ids, icons, labels, "meleeweapon",    JMConstants.ICON_GLADIUS,          "#STR_COT_OBJECT_CATEGORY_MELEE" );
		AddCategory( ids, icons, labels, "magazine_base",  JMConstants.ICON_MACHINE_GUN_MAG,  "#STR_COT_OBJECT_CATEGORY_AMMO" );
		AddCategory( ids, icons, labels, "clothing_base",  JMConstants.ICON_CLOTHES,          "#STR_COT_OBJECT_CATEGORY_CLOTHING" );
		AddCategory( ids, icons, labels, "headgear_base",  JMConstants.ICON_STAHLHELM,        "#STR_COT_OBJECT_CATEGORY_HEADGEAR" );
		AddCategory( ids, icons, labels, "mask_base",      JMConstants.ICON_BALACLAVA,        "#STR_COT_OBJECT_CATEGORY_MASKS" );
		AddCategory( ids, icons, labels, "glasses_base",   JMConstants.ICON_SUNGLASSES,       "#STR_COT_OBJECT_CATEGORY_GLASSES" );
		AddCategory( ids, icons, labels, "top_base",       JMConstants.ICON_T_SHIRT,          "#STR_COT_OBJECT_CATEGORY_TOPS" );
		AddCategory( ids, icons, labels, "pants_base",     JMConstants.ICON_TROUSERS,         "#STR_COT_OBJECT_CATEGORY_PANTS" );
		AddCategory( ids, icons, labels, "vest_base",      JMConstants.ICON_BELT,             "#STR_COT_OBJECT_CATEGORY_VESTS" );
		AddCategory( ids, icons, labels, "gloves_base",    JMConstants.ICON_GLOVES,           "#STR_COT_OBJECT_CATEGORY_GLOVES" );
		AddCategory( ids, icons, labels, "shoes_base",     JMConstants.ICON_TROUSERS,         "#STR_COT_OBJECT_CATEGORY_SHOES" );
		AddCategory( ids, icons, labels, "backpack_base",  JMConstants.ICON_LIGHT_BACKPACK,   "#STR_COT_OBJECT_CATEGORY_BACKPACKS" );
		AddCategory( ids, icons, labels, "container_base", JMConstants.ICON_KNAPSACK,         "#STR_COT_OBJECT_CATEGORY_CONTAINERS" );
		AddCategory( ids, icons, labels, "inventory_base", JMConstants.ICON_FULL_FOLDER,      "#STR_COT_OBJECT_CATEGORY_ITEMS" );
		AddCategory( ids, icons, labels, "itemmedical",    JMConstants.ICON_MEDICINES,        "#STR_COT_OBJECT_CATEGORY_MEDICAL" );
		AddCategory( ids, icons, labels, "tool_base",      JMConstants.ICON_SHARP_AXE,        "#STR_COT_OBJECT_CATEGORY_TOOLS" );
		AddCategory( ids, icons, labels, "trapbase",       JMConstants.ICON_TINKER,           "#STR_COT_OBJECT_CATEGORY_TRAPS" );
		AddCategory( ids, icons, labels, "electricdevice", JMConstants.ICON_ELECTRIC,         "#STR_COT_OBJECT_CATEGORY_ELECTRONICS" );
		AddCategory( ids, icons, labels, "tentbase",       JMConstants.ICON_CAMPING_TENT,     "#STR_COT_OBJECT_CATEGORY_TENTS" );
		AddCategory( ids, icons, labels, "grenade_base",   JMConstants.ICON_UNLIT_BOMB,       "#STR_COT_OBJECT_CATEGORY_EXPLOSIVES" );
		AddCategory( ids, icons, labels, "house",          JMConstants.ICON_HOME_GARAGE,      "#STR_COT_OBJECT_CATEGORY_BUILDINGS" );
		AddCategory( ids, icons, labels, "dz_lightai",     JMConstants.ICON_SHAMBLING_ZOMBIE, "#STR_COT_OBJECT_CATEGORY_AI" );
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
			members.Insert( "clothing_base" );
			members.Insert( "headgear_base" );
			members.Insert( "mask_base" );
			members.Insert( "glasses_base" );
			members.Insert( "top_base" );
			members.Insert( "pants_base" );
			members.Insert( "vest_base" );
			members.Insert( "gloves_base" );
			members.Insert( "shoes_base" );
			members.Insert( "backpack_base" );
			return members;
		}

		if ( groupId == GROUP_WEAPONS )
		{
			members.Insert( "weapon_base" );
			members.Insert( "meleeweapon" );
			members.Insert( "magazine_base" );
			members.Insert( "grenade_base" );
			members.Insert( "trapbase" );
			return members;
		}

		if ( groupId == GROUP_SURVIVAL )
		{
			members.Insert( "edible_base" );
			members.Insert( "bottle_base" );
			members.Insert( "itemmedical" );
			members.Insert( "tool_base" );
			members.Insert( "tentbase" );
			members.Insert( "container_base" );
			return members;
		}

		if ( groupId == GROUP_WORLD )
		{
			members.Insert( "transport" );
			members.Insert( "house" );
			members.Insert( "dz_lightai" );
			members.Insert( "electricdevice" );
			members.Insert( "inventory_base" );
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

		//! A chip press must never destroy the strip it landed on. Adding a
		//! category the strip does not hold yet is the only thing that changes
		//! its shape, and even then the rebuild is deferred a tick so it cannot
		//! happen inside the click that caused it - the engine answers a
		//! vanished press target by recentring the cursor.
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
			GetGame().GameScript.CallFunctionParams( m_Target, m_ChangeFn, null, new Param1<string>( id ) );
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
				return;

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
	}

	//! Only ever reached for MENU_ID_ALL - the four group rows above switch
	//! page instead, intercepted by UIActionFilterMenu before this runs.
	void OnRootChange( string id )
	{
		if ( id == MENU_ID_ALL )
			Select( "" );
	}

	void BuildGroupPage( UIActionFilterMenu menu )
	{
		TStringArray members = CategoryGroupMembers( menu.CurrentPage() );

		for ( int i = 0; i < members.Count(); i++ )
		{
			int color = 0;

			if ( members[i] == m_Current )
				color = JMTheme.ACCENT;

			menu.AddRow( members[i], CategoryLabel( members[i] ), CategoryIcon( members[i] ), color );
		}
	}

	void OnGroupChange( string id )
	{
		Select( id );
	}

	// -------------------------------------------------------------------------
	//  Recent chips
	// -------------------------------------------------------------------------

	//! Newest first, capped. Returns whether the strip's contents changed.
	//!
	//! Re-picking a category the strip ALREADY holds leaves the order alone.
	//! Promoting it to the front would slide every other chip sideways under
	//! the cursor that just pressed one, which is the last thing a shortcut
	//! bar should do. "All" gets no chip - it is the first row of the menu.
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
