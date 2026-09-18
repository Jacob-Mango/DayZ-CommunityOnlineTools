//! #define scope in Enforce is per-file, NOT per compiled module - see COTModule.c.
#define COT_DEBUGLOGS

//! Every action that can be taken on a tracked object, as one context menu.
//!
//! Both entry points share this: right-clicking the world tag and right-clicking
//! a row in the ESP form's tracked list. They used to be two different menus
//! over two different action sets - the tag had an unfolding panel of controls,
//! the list had four items - which meant an action was either in one place, the
//! other, or written twice.
//!
//! Anything that needs a value is a page of choices rather than a text box: a
//! menu cannot be typed into, and for health, quantity and the rest the useful
//! values are a short list anyway. Anything genuinely free-form - an exact
//! position, an exact lock code - goes through the clipboard.
class JMESPActionMenu
{
	//! Pages. The id of a row is its whole routing: "p:" navigates, "a:" acts,
	//! and a trailing ":arg" carries the one parameter a page ever needs.
	static const string PAGE_MAIN      = "main";
	static const string PAGE_TRANSFORM = "transform";
	static const string PAGE_HEALTH    = "health";
	static const string PAGE_QUANTITY  = "quantity";
	static const string PAGE_LIQUID    = "liquid";
	static const string PAGE_FOOD      = "food";
	static const string PAGE_LOCK      = "lock";
	static const string PAGE_BUILD     = "build";
	static const string PAGE_PART      = "part";
	static const string PAGE_VEHICLE   = "vehicle";
	static const string PAGE_PLAYER    = "player";
	static const string PAGE_WEAPON    = "weapon";
	static const string PAGE_ATTACHMENTS  = "attachments";
	static const string PAGE_ATTACH_SLOT  = "attachslot";
	static const string PAGE_FIRE      = "fire";
	static const string PAGE_SCALE     = "scale";
	static const string PAGE_PARTHP    = "parthp";
	static const string PAGE_FUEL      = "fuel";
	static const string PAGE_COOLANT   = "coolant";
	static const string PAGE_FLAG      = "flag";
	static const string PAGE_COLOR     = "color";
	static const string PAGE_COPY      = "copy";
	static const string PAGE_COPY_PLAYER = "copyplayer";

	static const string PREFIX_PAGE   = "p:";
	static const string PREFIX_ACTION = "a:";

	//! Shock is what consciousness is made of - there is no "make unconscious"
	//! call - so it is set to either end of its range.
	static const float SHOCK_CONSCIOUS = 100;

	//! Body scales offered on the scale page. 1 is the only one that undoes the
	//! others, so it is listed first.
	static const string SCALE_VALUES = "1|0.5|0.75|1.5|2";

	//! The one menu that floats over the world.
	//!
	//! Both world entry points - a tag's right-click and a right-click on the
	//! object itself - raise this same instance, because only one menu can be
	//! open at a time anyway and two would have to be told about each other.
	//! The ESP form keeps its own, anchored inside its window so the list does
	//! not clip it.
	protected static ref JMESPActionMenu s_Shared;

	protected UIActionContextMenu m_Menu;
	protected JMESPMeta m_Meta;

	protected string m_Page;
	protected string m_PageArg;

	//! Where the menu was first opened. Every page after the first reopens at
	//! the same spot, so drilling in does not walk the menu across the screen.
	protected float m_X;
	protected float m_Y;

	protected bool HasEmptyAttachmentSlots( EntityAI entity )
	{
		if ( !entity || !entity.GetInventory() )
			return false;

		int count = entity.GetInventory().GetAttachmentSlotsCount();
		if ( count == 0 )
			return false;

		for ( int i = 0; i < count; ++i )
		{
			int slotId = entity.GetInventory().GetAttachmentSlotId( i );
			if ( slotId != InventorySlots.INVALID && InventorySlots.GetShowForSlotId( slotId ) )
			{
				if ( !entity.GetInventory().FindAttachment( slotId ) )
					return true;
			}
		}

		return false;
	}

	//! Whether this thing has a cargo space at all.
	//!
	//! The structure, not the contents: a client is not told what is inside a
	//! container it has never opened, so counting items here would hide the row
	//! on exactly the full crates it is for.
	protected bool HasCargo( EntityAI entity )
	{
		if ( !entity || !entity.GetInventory() )
			return false;

		return entity.GetInventory().GetCargo() != NULL;
	}

	//! The immobilised flag off whichever of the two creature families this is.
	//!
	//! Both carry it as a synced bool, so the row can report the state it is
	//! about to flip instead of guessing at it.
	protected bool CreatureIsImmobilized()
	{
		ZombieBase zombie;
		AnimalBase animal;

		if ( Class.CastTo( zombie, m_Meta.target ) )
			return zombie.COT_IsImmobilized();

		if ( Class.CastTo( animal, m_Meta.target ) )
			return animal.COT_IsImmobilized();

		return false;
	}

	//! Build the menu this instance drives. The caller keeps the returned
	//! control so it can register it as an overlay, and hands back a parent for
	//! it to live in - the ESP container for a world tag, the form's window for
	//! the tracked list.
	UIActionContextMenu Init( notnull Widget parent, notnull Widget anchor )
	{
		m_Menu = UIActionManager.CreateContextMenu( parent, anchor, this, "OnMenuClick" );

		return m_Menu;
	}

	//! The world menu, built on first use.
	//!
	//! Parented to the ESP container rather than to whatever raised it: that
	//! container is created once per client at mission load and outlives every
	//! tag, so a menu living in it cannot be torn out from under itself when
	//! the thing that opened it stops being tracked.
	static JMESPActionMenu Shared()
	{
		if ( !JMStatics.ESP_CONTAINER )
			return NULL;

		if ( s_Shared )
			return s_Shared;

		JMESPActionMenu menu = new JMESPActionMenu();

		if ( !menu.Init( JMStatics.ESP_CONTAINER, JMStatics.ESP_CONTAINER ) )
			return NULL;

		s_Shared = menu;

		return s_Shared;
	}

	UIActionContextMenu GetMenu()
	{
		return m_Menu;
	}

	JMESPMeta GetTarget()
	{
		return m_Meta;
	}

	void Open( JMESPMeta meta, float x, float y )
	{
		if ( !m_Menu || !meta || !meta.target )
			return;

		m_Meta    = meta;
		m_Page    = PAGE_MAIN;
		m_PageArg = "";
		m_X       = x;
		m_Y       = y;

		Show();
	}

	void Close()
	{
		if ( m_Menu )
			m_Menu.Close();
	}

	bool IsOpen()
	{
		return m_Menu && m_Menu.IsOpen();
	}

	// =========================================================================
	//  Page building
	// =========================================================================

	protected void Show()
	{
		if ( !m_Meta || !m_Meta.target )
			return;

		#ifdef COT_DEBUGLOGS
		Print("[COT_DBG] JMESPActionMenu.Show() page=" + m_Page + " arg=" + m_PageArg);
		#endif

		m_Menu.ClearItems();

		if ( m_Page == PAGE_TRANSFORM )
			BuildTransform();
		else if ( m_Page == PAGE_HEALTH )
			BuildHealth();
		else if ( m_Page == PAGE_QUANTITY )
			BuildQuantity();
		else if ( m_Page == PAGE_LIQUID )
			BuildLiquid();
		else if ( m_Page == PAGE_FOOD )
			BuildFood();
		else if ( m_Page == PAGE_LOCK )
			BuildLock();
		else if ( m_Page == PAGE_BUILD )
			BuildConstruction();
		else if ( m_Page == PAGE_PART )
			BuildPart();
		else if ( m_Page == PAGE_WEAPON )
			BuildWeapon();
		else if ( m_Page == PAGE_ATTACHMENTS )
			BuildAttachments();
		else if ( m_Page == PAGE_ATTACH_SLOT )
			BuildAttachSlot();
		else if ( m_Page == PAGE_FIRE )
			BuildFire();
		else if ( m_Page == PAGE_SCALE )
			BuildScale();
		else if ( m_Page == PAGE_PARTHP )
			BuildPartHealth();
		else if ( m_Page == PAGE_FUEL )
			BuildFuel();
		else if ( m_Page == PAGE_COOLANT )
			BuildCoolant();
		else if ( m_Page == PAGE_FLAG )
			BuildFlag();
		else if ( m_Page == PAGE_COLOR )
			BuildColor();
		else if ( m_Page == PAGE_COPY )
			BuildCopy();
		else if ( m_Page == PAGE_COPY_PLAYER )
			BuildCopyPlayer();
		else if ( m_Page == PAGE_VEHICLE )
			BuildVehicle();
		else if ( m_Page == PAGE_PLAYER )
			BuildPlayer();
		else
			BuildMain();

		m_Menu.ShowAt( m_X, m_Y );
	}

	//! `enabled` is (almost) always a permission check, sometimes ANDed with a
	//! state condition - either way, false means the row is not something
	//! this admin can do right now, and a row that only ever sits there
	//! grayed out teaches nothing. Skipped entirely rather than added
	//! disabled; the menu is rebuilt from scratch on every Show() (see
	//! ClearItems() above), so there is no stale row to worry about.
	protected void Add( string id, string label, string icon, bool enabled = true, int colour = 0 )
	{
		if ( !enabled )
			return;

		m_Menu.AddItem( id, label, icon, colour );
	}

	//! A row that opens another page. The trailing chevron is the whole point:
	//! a page and a command look identical without it.
	protected void AddPage( string page, string label, string icon, bool enabled = true )
	{
		if ( !enabled )
			return;

		m_Menu.AddItem( PREFIX_PAGE + page, label, icon, 0, true );
	}

	//! A row that reports a state as well as offering to change it: the glyph
	//! is the state, the label is the thing.
	protected void AddToggle( string id, string label, bool state, bool enabled = true )
	{
		string icon = JMConstants.Lucide( "square" );

		if ( state )
			icon = JMConstants.Lucide( "check" );

		Add( id, label, icon, enabled );
	}

	//! The five rows every "how much of it" page is made of.
	//!
	//! The two ends are named rather than numbered: "Full" and "100%" say the
	//! same thing, and only one of them reads as an answer to the question the
	//! page is asking.
	protected void AddPercentRows( string action, string fullLabel, string emptyLabel, string icon, bool enabled, int emptyColour = 0 )
	{
		Add( PREFIX_ACTION + action + ":100", fullLabel, icon, enabled );
		Add( PREFIX_ACTION + action + ":75", "75%", "", enabled );
		Add( PREFIX_ACTION + action + ":50", "50%", "", enabled );
		Add( PREFIX_ACTION + action + ":25", "25%", "", enabled );
		Add( PREFIX_ACTION + action + ":0", emptyLabel, "", enabled, emptyColour );
	}

	protected void AddBack()
	{
		Add( PREFIX_PAGE + PAGE_MAIN, "#STR_COT_ESP_MODULE_MENU_BACK", JMConstants.Lucide( "chevron-left" ) );
	}

	protected bool Perm( string permission )
	{
		return GetPermissionsManager().HasPermission( permission );
	}

	protected bool IsSelected()
	{
		return JM_GetSelected().IsObjectSelected( m_Meta.target );
	}

	protected float MaxHealth()
	{
		if ( !m_Meta.networkLow && !m_Meta.networkHigh && g_Game.IsMultiplayer() )
			return 0;

		return MiscGameplayFunctions.GetTypeMaxGlobalHealth( m_Meta.target.GetType() );
	}

	protected void BuildMain()
	{
		Object target = m_Meta.target;

		//! A map object baked into the terrain never got a network ID - a
		//! dynamically spawned/CE one always has one, even offline (see
		//! MaxHealth() below for the same test). Static ESP/tag actions
		//! (select, copy, delete, ...) mean nothing on it and are hidden;
		//! only the type-specific rows below (door open/close, ...) still
		//! apply, since a static house genuinely does have doors.
		bool isStatic = !m_Meta.networkLow && !m_Meta.networkHigh && g_Game.IsMultiplayer();

		if ( !isStatic )
		{
			if ( IsSelected() )
				Add( PREFIX_ACTION + "deselect", "#STR_COT_ESP_MODULE_MENU_DESELECT", JMConstants.Lucide( "square" ) );
			else
				Add( PREFIX_ACTION + "select", "#STR_COT_ESP_MODULE_MENU_SELECT", JMConstants.Lucide( "mouse-pointer-click" ) );
		}

		//! Neither transform nor the global health slider means anything on a
		//! placed building - it moves and takes damage through its construction
		//! parts (see PAGE_PARTHP below), not as one rigid object with a single
		//! position or a single health value.
		bool isBuilding = BaseBuildingBase.Cast( target ) != null;

		if ( !isBuilding )
			AddPage( PAGE_TRANSFORM, "#STR_COT_ESP_MODULE_PAGE_TRANSFORM", JMConstants.Lucide( "move-3d" ) );

		if ( !isBuilding && MaxHealth() > 0 )
			AddPage( PAGE_HEALTH, "#STR_COT_ESP_MODULE_PAGE_HEALTH", JMConstants.Lucide( "heart-pulse" ) );

		ItemBase item = ItemBase.Cast( target );

		if ( item )
		{
			if ( item.GetQuantityMax() > 0 )
				AddPage( PAGE_QUANTITY, "#STR_COT_ESP_MODULE_PAGE_QUANTITY", JMConstants.Lucide( "package" ) );

			if ( item.IsLiquidContainer() )
				AddPage( PAGE_LIQUID, "#STR_COT_ESP_MODULE_PAGE_LIQUID", JMConstants.Lucide( "droplet" ) );

			if ( item.IsInherited( Edible_Base ) )
				AddPage( PAGE_FOOD, "#STR_COT_ESP_MODULE_PAGE_FOOD", JMConstants.Lucide( "utensils" ) );

			if ( item.CanBeDisinfected() )
				AddToggle( PREFIX_ACTION + "disinfect", "#STR_COT_ESP_MODULE_ACTION_DISINFECTED", item.GetCleanness() > 0, Perm( "ESP.Object.SetCleanness" ) );

			//! Open and Close are no-op virtuals on a plain ItemBase, so this is
			//! limited to the containers that actually override them rather than
			//! offered everywhere and quietly doing nothing.
			//!
			//! The row names what it will do, not what the thing currently is:
			//! a menu entry is a verb, and "Open" on an already-open container
			//! reads as a statement rather than a button.
			if ( item.IsInherited( Container_Base ) )
			{
				if ( item.IsOpen() )
					Add( PREFIX_ACTION + "openclose", "#STR_COT_ESP_MODULE_ACTION_CLOSE", JMConstants.Lucide( "package" ), Perm( "ESP.Object.OpenClose" ) );
				else
					Add( PREFIX_ACTION + "openclose", "#STR_COT_ESP_MODULE_ACTION_OPEN", JMConstants.Lucide( "package-open" ), Perm( "ESP.Object.OpenClose" ) );
			}
		}

		if ( JMESPModule.GetCombinationLock( target ) )
			AddPage( PAGE_LOCK, "#STR_COT_ESP_MODULE_PAGE_LOCK", JMConstants.Lucide( "lock" ) );

		if ( m_Meta.GetConstructionParts() )
			AddPage( PAGE_BUILD, "#STR_COT_ESP_MODULE_PAGE_BUILD", JMConstants.Lucide( "hammer" ) );

		if ( target.IsInherited( Weapon_Base ) )
			AddPage( PAGE_WEAPON, "#STR_COT_ESP_MODULE_PAGE_WEAPON", JMConstants.Lucide( "crosshair" ) );

		if ( target.IsInherited( FireplaceBase ) )
			AddPage( PAGE_FIRE, "#STR_COT_ESP_MODULE_PAGE_FIRE", JMConstants.Lucide( "flame" ) );

		if ( target.IsInherited( Transport ) )
			AddPage( PAGE_VEHICLE, "#STR_COT_ESP_MODULE_PAGE_VEHICLE", JMConstants.Lucide( "car" ) );

		//! Recolors the target and, best-effort, any already-attached child
		//! that carries a color token of its own - offered on anything with
		//! an inventory, not just vehicles, since clothing and gear can be
		//! color-suffixed too. Never on a player - a character has no color
		//! token to change - and hidden entirely rather than shown-disabled
		//! when the classname has no variants: a page that only ever says "no
		//! variants" teaches nothing about what the tool can do.
		EntityAI colorEntity = EntityAI.Cast( target );
		if ( colorEntity && !PlayerBase.Cast( target ) && Perm( "ESP.Object.ChangeColor" ) )
		{
			JMObjectSpawnerModule objSpawner;
			if ( CF_Modules<JMObjectSpawnerModule>.Get( objSpawner ) && objSpawner.GetAvailableColorVariants( colorEntity ).Count() > 0 )
				AddPage( PAGE_COLOR, "#STR_COT_ESP_MODULE_PAGE_COLOR", JMConstants.Lucide( "palette" ) );
		}

		//! TargetPlayer, not m_Meta.player: the meta's own copy is filled from
		//! the target's identity and is null for most of the roster, so gating
		//! on it hid the whole player page for players this client could still
		//! name perfectly well.
		if ( TargetPlayer() )
			AddPage( PAGE_PLAYER, "#STR_COT_ESP_MODULE_PAGE_PLAYER", JMConstants.Lucide( "user" ) );

		Grenade_Base grenade = Grenade_Base.Cast( target );
		if ( grenade )
		{
			if ( grenade.IsPinned() )
				Add( PREFIX_ACTION + "unpingrenade", "#STR_COT_ESP_MODULE_MENU_UNPIN_GRENADE", JMConstants.Lucide( "bomb" ), Perm( "ESP.Object.SetQuantity" ) );
			else
				Add( PREFIX_ACTION + "pingrenade", "#STR_COT_ESP_MODULE_MENU_PIN_GRENADE", JMConstants.Lucide( "shield" ), Perm( "ESP.Object.SetQuantity" ) );
		}

		TrapBase trap = TrapBase.Cast( target );
		if ( trap )
		{
			//! Arming and setting it off are different things and a trap can be
			//! worth both in a session - a beartrap dropped by an admin is armed
			//! first and sprung later - so the toggle and the trigger are two
			//! rows rather than one that changes meaning.
			if ( trap.IsActive() )
				Add( PREFIX_ACTION + "disarmtrap", "#STR_COT_ESP_MODULE_MENU_DISARM_TRAP", JMConstants.Lucide( "shield-check" ), Perm( "ESP.Object.Trap" ) );
			else
				Add( PREFIX_ACTION + "armtrap", "#STR_COT_ESP_MODULE_MENU_ARM_TRAP", JMConstants.Lucide( "shield-alert" ), Perm( "ESP.Object.Trap" ) );

			Add( PREFIX_ACTION + "triggertrap", "#STR_COT_ESP_MODULE_MENU_TRIGGER_TRAP", JMConstants.Lucide( "flame" ), Perm( "ESP.Object.Trap" ), JMTheme.DANGER );
		}

		//! A flag pole's mast, which in vanilla is also its refresher timer.
		if ( target.IsInherited( TerritoryFlag ) )
			AddPage( PAGE_FLAG, "#STR_COT_ESP_MODULE_PAGE_FLAG", JMConstants.Lucide( "flag" ), Perm( "ESP.Object.Flag" ) );

		//! Infected and animals only. A player is a DayZPlayer, not a creature,
		//! and has the player page's freeze for the same job.
		if ( target.IsInherited( ZombieBase ) || target.IsInherited( AnimalBase ) )
			AddToggle( PREFIX_ACTION + "immobilize", "#STR_COT_ESP_MODULE_ACTION_IMMOBILIZED", CreatureIsImmobilized(), Perm( "ESP.Object.Immobilize" ) );

		Fence fence = Fence.Cast( target );
		if ( fence )
		{
			// Same eligibility vanilla's own gate action uses - a gate with
			// no hinges built yet, or one that's locked, has nothing here to
			// open or close.
			if ( fence.CanCloseFence() )
				Add( PREFIX_ACTION + "closefence", "#STR_COT_ESP_MODULE_MENU_CLOSE_FENCE", JMConstants.Lucide( "door-closed" ), Perm( "ESP.Object.BaseBuilding.Build" ) );
			else if ( fence.CanOpenFence() )
				Add( PREFIX_ACTION + "openfence", "#STR_COT_ESP_MODULE_MENU_OPEN_FENCE", JMConstants.Lucide( "door-open" ), Perm( "ESP.Object.BaseBuilding.Build" ) );
		}

		TentBase tent = TentBase.Cast( target );
		if ( tent )
		{
			if ( tent.IsOpen() )
				Add( PREFIX_ACTION + "closetent", "#STR_COT_ESP_MODULE_MENU_CLOSE_TENT", JMConstants.Lucide( "package" ), Perm( "ESP.Object.OpenClose" ) );
			else
				Add( PREFIX_ACTION + "opentent", "#STR_COT_ESP_MODULE_MENU_OPEN_TENT", JMConstants.Lucide( "package-open" ), Perm( "ESP.Object.OpenClose" ) );
		}

		//! A static map building only offers door actions when the right-click
		//! actually landed on one of its doors (m_DoorIndex, resolved from the
		//! raycast component at menu-open time) - a house with no door under
		//! the cursor gets none of this, same as any other static prop.
		Building house = Building.Cast( target );
		if ( house && m_Meta.m_DoorIndex >= 0 )
		{
			int doorIndex = m_Meta.m_DoorIndex;

			if ( house.IsDoorLocked( doorIndex ) )
				Add( PREFIX_ACTION + "unlockdoor", "#STR_COT_ESP_MODULE_MENU_UNLOCK_DOOR", JMConstants.Lucide( "lock-open" ), Perm( "ESP.Object.OpenClose" ) );
			else if ( house.CanDoorBeLocked( doorIndex ) )
				Add( PREFIX_ACTION + "lockdoor", "#STR_COT_ESP_MODULE_MENU_LOCK_DOOR", JMConstants.Lucide( "lock" ), Perm( "ESP.Object.OpenClose" ) );

			if ( house.IsDoorOpen( doorIndex ) )
			{
				if ( house.CanDoorBeClosed( doorIndex ) )
					Add( PREFIX_ACTION + "closedoor", "#STR_COT_ESP_MODULE_MENU_CLOSE_DOOR", JMConstants.Lucide( "door-closed" ), Perm( "ESP.Object.OpenClose" ) );
			}
			else if ( house.CanDoorBeOpened( doorIndex ) )
			{
				Add( PREFIX_ACTION + "opendoor", "#STR_COT_ESP_MODULE_MENU_OPEN_DOOR", JMConstants.Lucide( "door-open" ), Perm( "ESP.Object.OpenClose" ) );
			}
		}

		//! Weapons, vehicles and clothing all expose attachment slots through
		//! the same GameInventory API - one page covers all three rather than
		//! duplicating this per target type. A player has no attachment slots
		//! of its own here (its gear is reached through its inventory, not
		//! this menu), so it is excluded the same way color-change is above.
		EntityAI attachEntity = EntityAI.Cast( target );
		GameInventory attachInv;
		if ( attachEntity )
			attachInv = attachEntity.GetInventory();

		if ( attachInv && attachInv.GetAttachmentSlotsCount() > 0 && !PlayerBase.Cast( target ) )
			AddPage( PAGE_ATTACHMENTS, "#STR_COT_ESP_MODULE_PAGE_ATTACHMENTS", JMConstants.Lucide( "puzzle" ), Perm( "ESP.Object.SetAttachment" ) );

		if ( !isStatic )
		{
			AddPage( PAGE_COPY, "#STR_COT_ESP_MODULE_MENU_COPY", JMConstants.Lucide( "copy" ) );

			//! Anything that can hold cargo can have it emptied - a tent, a crate, a
			//! car boot. A player is skipped because the player page carries its own
			//! clear cargo, routed through the player module for the logging.
			if ( HasCargo( EntityAI.Cast( target ) ) && !TargetPlayer() )
				Add( PREFIX_ACTION + "objclearcargo", "#STR_COT_PLAYER_MODULE_ACTION_CLEAR_CARGO", JMConstants.Lucide( "package-x" ), Perm( "ESP.Object.ClearCargo" ), JMTheme.DANGER );

			Add( PREFIX_ACTION + "delete", "#STR_COT_GENERIC_DELETE", JMConstants.ICON_TRASH_CAN, Perm( "ESP.Object.Delete" ) && m_Meta.CanDelete(), JMTheme.DANGER );
		}
	}

	protected void BuildTransform()
	{
		AddBack();

		bool canMove = Perm( "ESP.Object.SetPosition" );
		bool canTurn = Perm( "ESP.Object.SetOrientation" );

		//! The two teleports the player and vehicle managers already offer over
		//! their own lists, over anything ESP tracks. Bringing a tracked player
		//! goes through the teleport module rather than a raw SetPosition, so it
		//! is logged and webhooked like every other teleport of a player.
		bool canBring = canMove;

		if ( TargetPlayer() )
			canBring = Perm( "Admin.Player.Teleport.Position" );

		Add( PREFIX_ACTION + "tpme", "#STR_COT_PLAYER_MODULE_TELEPORT_ME_TO", JMConstants.Lucide( "footprints" ), Perm( "Admin.Player.Teleport.Position" ) );
		Add( PREFIX_ACTION + "tphere", "#STR_COT_PLAYER_MODULE_TELEPORT_TO_ME", JMConstants.Lucide( "move-down-left" ), canBring );

		//! Greyed rather than dropped when there is nothing to take back: the
		//! row disappearing would move every entry under it between one opening
		//! of the menu and the next.
		Add( PREFIX_ACTION + "tpundo", "#STR_COT_TELEPORT_UNDO", JMConstants.Lucide( "undo-2" ), canBring && HasTeleportUndo() );
		Add( PREFIX_ACTION + "tpredo", "#STR_COT_TELEPORT_REDO", JMConstants.Lucide( "redo-2" ), canBring && HasTeleportRedo() );

		Add( PREFIX_ACTION + "copypos", "#STR_COT_ESP_MODULE_MENU_COPY_POS", JMConstants.Lucide( "copy" ) );
		Add( PREFIX_ACTION + "pastepos", "#STR_COT_ESP_MODULE_MENU_PASTE_POS", JMConstants.Lucide( "clipboard-paste" ), canMove );
		Add( PREFIX_ACTION + "ground", "#STR_COT_ESP_MODULE_MENU_SNAP_GROUND", JMConstants.Lucide( "arrow-down-to-line" ), canMove );

		Add( PREFIX_ACTION + "copyori", "#STR_COT_ESP_MODULE_MENU_COPY_ORI", JMConstants.Lucide( "compass" ) );
		Add( PREFIX_ACTION + "pasteori", "#STR_COT_ESP_MODULE_MENU_PASTE_ORI", JMConstants.Lucide( "clipboard-paste" ), canTurn );
		Add( PREFIX_ACTION + "facecam", "#STR_COT_ESP_MODULE_MENU_FACE_CAMERA", JMConstants.Lucide( "rotate-cw" ), canTurn );
	}

	protected void BuildHealth()
	{
		AddBack();

		bool canSet = Perm( "ESP.Object.SetHealth" );

		Add( PREFIX_ACTION + "health:100", "#STR_COT_ESP_MODULE_MENU_FULL", JMConstants.Lucide( "heart-pulse" ), canSet );
		Add( PREFIX_ACTION + "health:75", "75%", "", canSet );
		Add( PREFIX_ACTION + "health:50", "50%", "", canSet );
		Add( PREFIX_ACTION + "health:25", "25%", "", canSet );
		Add( PREFIX_ACTION + "health:0", "#STR_COT_ESP_MODULE_MENU_RUIN", "", canSet, JMTheme.DANGER );

		//! "All" because DoHeal always heals attachments and cargo recursively
		//! (HealEntityRecursive's defaults) - unlike the player manager's own
		//! Heal, which only reaches them with a separate permission.
		if ( m_Meta.target.IsInherited( Man ) || m_Meta.target.IsInherited( DayZCreature ) )
			Add( PREFIX_ACTION + "heal", "#STR_COT_ESP_MODULE_MENU_HEAL_ALL", JMConstants.Lucide( "heart-pulse" ), Perm( "ESP.Object.Heal" ) );
		else
			Add( PREFIX_ACTION + "heal", "#STR_COT_PLAYER_MODULE_ACTION_REPAIR", JMConstants.Lucide( "wrench" ), Perm( "ESP.Object.Heal" ) );

		if ( HasEmptyAttachmentSlots( EntityAI.Cast( m_Meta.target ) ) )
			Add( PREFIX_ACTION + "repairfill", "#STR_COT_ESP_MODULE_MENU_REPAIR_FILL", JMConstants.Lucide( "wrench" ), Perm( "ESP.Object.Heal" ) );
	}

	protected void BuildQuantity()
	{
		AddBack();

		bool canSet = Perm( "ESP.Object.SetQuantity" );

		Add( PREFIX_ACTION + "quantity:100", "#STR_COT_ESP_MODULE_MENU_FULL", JMConstants.Lucide( "package" ), canSet );
		Add( PREFIX_ACTION + "quantity:75", "75%", "", canSet );
		Add( PREFIX_ACTION + "quantity:50", "50%", "", canSet );
		Add( PREFIX_ACTION + "quantity:25", "25%", "", canSet );
		Add( PREFIX_ACTION + "quantity:0", "#STR_COT_ESP_MODULE_MENU_EMPTY", "", canSet );
	}

	protected void BuildLiquid()
	{
		AddBack();

		bool canSet = Perm( "ESP.Object.SetLiquid" );

		array<string> names = LiquidNames();
		array<int> values = LiquidValues();

		for ( int i = 0; i < names.Count(); ++i )
			Add( PREFIX_ACTION + "liquid:" + values[i].ToString(), names[i], "", canSet );
	}

	protected void BuildFood()
	{
		AddBack();

		bool canSet = Perm( "ESP.Object.SetFoodStage" );

		array<string> names = FoodStageNames();
		array<int> values = FoodStageValues();

		for ( int i = 0; i < names.Count(); ++i )
			Add( PREFIX_ACTION + "food:" + values[i].ToString(), names[i], "", canSet );
	}

	protected void BuildLock()
	{
		AddBack();

		CombinationLock combo = JMESPModule.GetCombinationLock( m_Meta.target );

		if ( !combo )
			return;

		if ( combo.IsLocked() )
			Add( PREFIX_ACTION + "unlock", "#STR_COT_ESP_MODULE_MENU_UNLOCK", JMConstants.Lucide( "lock-open" ), Perm( "ESP.Object.Lock" ) );
		else
			Add( PREFIX_ACTION + "lock", "#STR_COT_ESP_MODULE_MENU_LOCK", JMConstants.Lucide( "lock" ), Perm( "ESP.Object.Lock" ) );

		//! The same two verbs over every lock standing near this one. A base is
		//! a dozen separate entities, so "lock the base" cannot be addressed to
		//! the gate the menu was opened on.
		Add( PREFIX_ACTION + "lockall", "#STR_COT_ESP_MODULE_MENU_LOCK_ALL", JMConstants.Lucide( "lock" ), Perm( "ESP.Object.Lock" ) );
		Add( PREFIX_ACTION + "unlockall", "#STR_COT_ESP_MODULE_MENU_UNLOCK_ALL", JMConstants.Lucide( "lock-open" ), Perm( "ESP.Object.Lock" ) );

		Add( PREFIX_ACTION + "getcode", "#STR_COT_ESP_MODULE_ACTION_GET_CODE", JMConstants.ICON_STACK, Perm( "ESP.Object.GetCode" ) );
		Add( PREFIX_ACTION + "pastecode", "#STR_COT_ESP_MODULE_MENU_PASTE_CODE", JMConstants.Lucide( "clipboard-paste" ), Perm( "ESP.Object.SetCode" ) );
	}

	protected void BuildConstruction()
	{
		AddBack();

		map< string, ref JMConstructionPartData > parts = m_Meta.GetConstructionParts();

		if ( !parts )
			return;

		//! The two whole-object verbs, above the per-part rows they save.
		//! Dismantling everything takes the base part with it, which deletes the
		//! object - so it is tinted like the other things that cannot be undone.
		Add( PREFIX_ACTION + "buildall", "#STR_COT_ESP_MODULE_MENU_BUILD_ALL", JMConstants.Lucide( "construction" ), Perm( "ESP.Object.BaseBuilding.Build" ) );
		Add( PREFIX_ACTION + "dismantleall", "#STR_COT_ESP_MODULE_MENU_DISMANTLE_ALL", JMConstants.Lucide( "pickaxe" ), Perm( "ESP.Object.BaseBuilding.Dismantle" ), JMTheme.DANGER );
		Add( PREFIX_ACTION + "repairall", "#STR_COT_ESP_MODULE_MENU_REPAIR_ALL", JMConstants.Lucide( "wrench" ), Perm( "ESP.Object.BaseBuilding.Repair" ) );

		for ( int i = 0; i < parts.Count(); ++i )
		{
			string partName = parts.GetKey( i );
			JMConstructionPartData part = parts.Get( partName );

			//! A part that conflicts with something already built cannot be
			//! acted on at all, so it is not offered a page.
			bool enabled = part.m_State != JMConstructionPartState.CONFLICTING_PART;

			AddPage( PAGE_PART + ":" + partName, part.m_DisplayName, JMConstants.Lucide( "hammer" ), enabled );
		}
	}

	protected void BuildPart()
	{
		Add( PREFIX_PAGE + PAGE_BUILD, "#STR_COT_ESP_MODULE_MENU_BACK", JMConstants.Lucide( "chevron-left" ) );

		map< string, ref JMConstructionPartData > parts = m_Meta.GetConstructionParts();

		if ( !parts || !parts.Contains( m_PageArg ) )
			return;

		JMConstructionPartData part = parts.Get( m_PageArg );

		bool built = part.m_State == JMConstructionPartState.BUILT;

		if ( built )
		{
			Add( PREFIX_ACTION + "dismantle", "#STR_COT_ESP_MODULE_MENU_DISMANTLE", JMConstants.Lucide( "hammer" ), Perm( "ESP.Object.BaseBuilding.Dismantle" ) );
			Add( PREFIX_ACTION + "repairpart", "#STR_COT_ESP_MODULE_MENU_REPAIR", JMConstants.Lucide( "wrench" ), Perm( "ESP.Object.BaseBuilding.Repair" ) );

			//! Carries the part name forward as its own argument: the health
			//! page is a page like any other and has no memory of which row
			//! opened it.
			AddPage( PAGE_PARTHP + ":" + m_PageArg, "#STR_COT_ESP_MODULE_PAGE_PART_HEALTH", JMConstants.Lucide( "heart-pulse" ), Perm( "ESP.Object.BaseBuilding.SetHealth" ) );
		}
		else
		{
			Add( PREFIX_ACTION + "build", "#STR_COT_ESP_MODULE_MENU_BUILD", JMConstants.Lucide( "hammer" ), Perm( "ESP.Object.BaseBuilding.Build" ) );
		}
	}

	protected void BuildVehicle()
	{
		AddBack();

		//! Fuel and coolant are pages of fractions rather than one "fill it"
		//! row: an admin setting up a scenario wants a car with a quarter tank
		//! as often as a full one, and there was no way to ask for that.
		AddPage( PAGE_FUEL, "#STR_COT_ESP_MODULE_PAGE_FUEL", JMConstants.Lucide( "fuel" ), Perm( "ESP.Object.Car.Refuel" ) );

		//! Motorbike excluded - it has no coolant tank (see
		//! MotorbikeScript.c's COT_Refuel note).
		if ( m_Meta.target.IsInherited( CarScript ) )
			AddPage( PAGE_COOLANT, "#STR_COT_ESP_MODULE_PAGE_COOLANT", JMConstants.Lucide( "thermometer" ), Perm( "ESP.Object.Car.Refuel" ) );

		//! Trains run on rails and have nothing to be unstuck from.
	#ifndef DAYZ_1_29
		if ( m_Meta.target.IsInherited( CarScript ) || m_Meta.target.IsInherited( BoatScript ) || m_Meta.target.IsInherited( Motorbike ) )
	#else
		if ( m_Meta.target.IsInherited( CarScript ) || m_Meta.target.IsInherited( BoatScript ) )
	#endif
			Add( PREFIX_ACTION + "unstuck", "#STR_COT_ESP_MODULE_MENU_UNSTUCK", JMConstants.Lucide( "life-buoy" ), Perm( "ESP.Object.Car.Unstuck" ) );

		Add( PREFIX_ACTION + "heal", "#STR_COT_PLAYER_MODULE_ACTION_REPAIR", JMConstants.Lucide( "wrench" ), Perm( "ESP.Object.Heal" ) );

		if ( CommunityOnlineToolsBase.AreWheelsLocked( m_Meta.target ) )
			Add( PREFIX_ACTION + "unlockwheels", "#STR_COT_ESP_MODULE_MENU_UNLOCK_WHEELS", JMConstants.Lucide( "lock-open" ), Perm( "ESP.Object.Car.LockWheels" ) );
		else
			Add( PREFIX_ACTION + "lockwheels", "#STR_COT_ESP_MODULE_MENU_LOCK_WHEELS", JMConstants.Lucide( "lock" ), Perm( "ESP.Object.Car.LockWheels" ) );

		CarScript carScript = CarScript.Cast( m_Meta.target );
		if ( carScript )
		{
			if ( carScript.COT_AreCarDoorsOpen() )
				Add( PREFIX_ACTION + "closecardoors", "#STR_COT_ESP_MODULE_MENU_CLOSE_CAR_DOORS", JMConstants.Lucide( "car" ), Perm( "ESP.Object.OpenClose" ) );
			else
				Add( PREFIX_ACTION + "opencardoors", "#STR_COT_ESP_MODULE_MENU_OPEN_CAR_DOORS", JMConstants.Lucide( "car" ), Perm( "ESP.Object.OpenClose" ) );
		}

		if ( HasEmptyAttachmentSlots( EntityAI.Cast( m_Meta.target ) ) )
			Add( PREFIX_ACTION + "repairfill", "#STR_COT_ESP_MODULE_MENU_REPAIR_FILL", JMConstants.Lucide( "wrench" ), Perm( "ESP.Object.Heal" ) );

	#ifdef EXPANSIONMODVEHICLE
		//! Expansion used to bolt these onto the ESP tag itself with its own
		//! meta override. That hook is gone, so they are offered here and go
		//! through JMVehiclesModule - same permissions, logging and webhooks as
		//! the vehicle manager's own buttons.
		ExpansionVehicle vehicle;
		if ( ExpansionVehicle.Get( vehicle, EntityAI.Cast( m_Meta.target ) ) && vehicle.HasKey() )
		{
			if ( vehicle.IsLocked() )
				Add( PREFIX_ACTION + "vehunlock", "#STR_COT_ESP_MODULE_MENU_UNLOCK", JMConstants.Lucide( "lock-open" ), Perm( "Vehicles.Lock" ) );
			else
				Add( PREFIX_ACTION + "vehunlock", "#STR_COT_ESP_MODULE_MENU_LOCK", JMConstants.Lucide( "lock" ), Perm( "Vehicles.Lock" ) );

			Add( PREFIX_ACTION + "vehunpair", "#STR_COT_ESP_MODULE_MENU_UNPAIR", JMConstants.Lucide( "key-round" ), Perm( "Vehicles.UnPair" ) );
		}
	#endif
	}

	protected void BuildFuel()
	{
		Add( PREFIX_PAGE + PAGE_VEHICLE, "#STR_COT_ESP_MODULE_MENU_BACK", JMConstants.Lucide( "chevron-left" ) );

		bool canSet = Perm( "ESP.Object.Car.Refuel" );

		AddPercentRows( "fuel", "#STR_COT_ESP_MODULE_MENU_FULL", "#STR_COT_ESP_MODULE_MENU_EMPTY", JMConstants.Lucide( "fuel" ), canSet );

		//! The old Refuel, kept as one row: it tops up oil and brake fluid as
		//! well, which no per-fluid page above ever will.
		Add( PREFIX_ACTION + "refuel", "#STR_COT_ESP_MODULE_MENU_REFUEL_ALL", JMConstants.Lucide( "droplets" ), canSet );
	}

	protected void BuildCoolant()
	{
		Add( PREFIX_PAGE + PAGE_VEHICLE, "#STR_COT_ESP_MODULE_MENU_BACK", JMConstants.Lucide( "chevron-left" ) );

		AddPercentRows( "coolant", "#STR_COT_ESP_MODULE_MENU_FULL", "#STR_COT_ESP_MODULE_MENU_EMPTY", JMConstants.Lucide( "thermometer" ), Perm( "ESP.Object.Car.Refuel" ) );
	}

	//! How far up the mast the flag sits.
	//!
	//! A percentage rather than raise/lower because that is what the pole
	//! actually stores - vanilla's own raise action moves it one step at a time
	//! - and because the refresher time it carries moves with it.
	protected void BuildFlag()
	{
		AddBack();

		bool canSet = Perm( "ESP.Object.Flag" );

		//! A pole with no flag on it has nothing to raise. Greyed rather than
		//! dropped so the rows do not move between one opening and the next.
		EntityAI pole = EntityAI.Cast( m_Meta.target );

		if ( pole && !pole.FindAttachmentBySlotName( "Material_FPole_Flag" ) )
			canSet = false;

		AddPercentRows( "flag", "#STR_COT_ESP_MODULE_MENU_FLAG_RAISED", "#STR_COT_ESP_MODULE_MENU_FLAG_LOWERED", JMConstants.Lucide( "flag" ), canSet );
	}

	//! Damage one built part without dismantling it.
	//!
	//! Percentages rather than hit points: every part and every material tier
	//! has a different maximum, so a number would mean something different on
	//! each row.
	protected void BuildPartHealth()
	{
		Add( PREFIX_PAGE + PAGE_PART + ":" + m_PageArg, "#STR_COT_ESP_MODULE_MENU_BACK", JMConstants.Lucide( "chevron-left" ) );

		bool canSet = Perm( "ESP.Object.BaseBuilding.SetHealth" );

		Add( PREFIX_ACTION + "parthp:100", "#STR_COT_ESP_MODULE_MENU_FULL", JMConstants.Lucide( "heart-pulse" ), canSet );
		Add( PREFIX_ACTION + "parthp:75", "75%", "", canSet );
		Add( PREFIX_ACTION + "parthp:50", "50%", "", canSet );
		Add( PREFIX_ACTION + "parthp:25", "25%", "", canSet );
		Add( PREFIX_ACTION + "parthp:0", "#STR_COT_ESP_MODULE_MENU_RUIN", "", canSet, JMTheme.DANGER );
	}

	//! Everything that can be done to a firearm lying in the world.
	//!
	//! The jam row reports its state because a weapon syncs that flag; the two
	//! chamber rows do not, because chamber contents are not something a client
	//! can read off an object it is not holding - a row that claimed to know
	//! would be lying half the time.
	protected void BuildWeapon()
	{
		AddBack();

		Weapon_Base weapon;

		if ( !Class.CastTo( weapon, m_Meta.target ) )
			return;

		bool canSet = Perm( "ESP.Object.Weapon" );

		AddToggle( PREFIX_ACTION + "jam", "#STR_COT_ESP_MODULE_ACTION_JAMMED", weapon.IsJammed(), canSet );

		if ( weapon.IsChamberFull( 0 ) )
			Add( PREFIX_ACTION + "unchamber", "#STR_COT_ESP_MODULE_MENU_UNCHAMBER", JMConstants.Lucide( "circle" ), canSet );
		else
			Add( PREFIX_ACTION + "chamber", "#STR_COT_ESP_MODULE_MENU_CHAMBER", JMConstants.Lucide( "circle-dot" ), canSet );

		//! A weapon is one or the other: it takes a detachable magazine, or it
		//! holds its rounds itself. Offering both would put a dead row on every
		//! gun in the game.
		//!
		//! Exec_LoadMagazine already refills an already-attached magazine
		//! instead of stacking a second one in a slot that only holds one -
		//! the label just didn't say so. Same check, client-side, so the row
		//! reads "Refill" instead of "Attach" when that is what it will do.
		if ( WeaponAcceptsMagazine( weapon ) )
		{
			if ( WeaponHasMagazineAttached( weapon ) )
				Add( PREFIX_ACTION + "loadmag", "#STR_COT_ESP_MODULE_MENU_REFILL_MAG", JMConstants.Lucide( "package-plus" ), canSet );
			else
				Add( PREFIX_ACTION + "loadmag", "#STR_COT_ESP_MODULE_MENU_LOAD_MAG", JMConstants.Lucide( "package-plus" ), canSet );
		}
		else if ( weapon.HasInternalMagazine( -1 ) )
			Add( PREFIX_ACTION + "fillinternal", "#STR_COT_ESP_MODULE_MENU_FILL_INTERNAL", JMConstants.Lucide( "layers-plus" ), canSet );

		//! "Repair & Fill Slots" removed here on purpose - Refill/Attach
		//! Magazine above is the weapon's ammo-filling action; a separate
		//! Repair row that ALSO fills the (same) empty magazine slot was
		//! redundant with it for the one attachment that actually matters.
		//!
		//! The Attachments page itself is offered from BuildMain, not here -
		//! it is not weapon-specific (vehicles and clothing have attachment
		//! slots too), so every target with any gets the same one entry point.
	}

	// -------------------------------------------------------------------------
	//  Attachments: per-slot add/remove/swap. Works on anything with attachment
	//  slots - weapons, vehicles, clothing - not just weapons, since they all
	//  expose the same GameInventory slot API.
	//
	//  Candidates are gathered the same way JMObjectSpawnerModule's spawn-with-
	//  color fill does - every scope==2 CfgWeapons/CfgVehicles entry whose
	//  inventorySlot[] names this slot - rather than trying to duplicate the
	//  target's own attachments[] compatibility rules here. The list can be
	//  broader than what this specific target actually accepts; the engine's
	//  own CreateAttachmentEx() on the server is what actually enforces
	//  compatibility, so an incompatible pick just fails there instead of
	//  silently misattaching, same as every other create-by-classname action
	//  in this menu.
	// -------------------------------------------------------------------------

	//! Candidate classnames for whatever slot PAGE_ATTACH_SLOT is currently
	//! showing - built by BuildAttachSlot(), read by index when "setattach:N"
	//! is clicked. Rebuilt every time the page is (re)entered, so an index is
	//! only ever resolved against the list that produced it.
	protected ref array<string> m_AttachCandidates;

	//! One row per attachment slot the target's model declares, whether or
	//! not something currently fills it - an empty slot is exactly where
	//! "add" belongs, and CoT has no other way to discover slot names.
	protected void BuildAttachments()
	{
		AddBack();

		EntityAI target;
		if ( !Class.CastTo( target, m_Meta.target ) || !target.GetInventory() )
			return;

		int count = target.GetInventory().GetAttachmentSlotsCount();
		for ( int i = 0; i < count; i++ )
		{
			int slotId = target.GetInventory().GetAttachmentSlotId( i );
			if ( slotId == InventorySlots.INVALID || !InventorySlots.GetShowForSlotId( slotId ) )
				continue;

			string slotName = InventorySlots.GetSlotDisplayName( slotId );
			EntityAI attached = target.GetInventory().FindAttachment( slotId );

			string label = slotName + ": ";
			if ( attached )
				label += attached.GetDisplayName();
			else
				label += Widget.TranslateString( "#STR_COT_ESP_MODULE_ATTACH_EMPTY" );

			Add( PREFIX_PAGE + PAGE_ATTACH_SLOT + ":" + slotId, label, JMConstants.Lucide( "puzzle" ), true );
		}
	}

	//! m_PageArg is the slot id (see BuildAttachments) - "Remove" only shows
	//! when something is actually there to remove, same reasoning as every
	//! other conditional row in this menu.
	protected void BuildAttachSlot()
	{
		Add( PREFIX_PAGE + PAGE_ATTACHMENTS, "#STR_COT_ESP_MODULE_MENU_BACK", JMConstants.Lucide( "chevron-left" ) );

		EntityAI target;
		if ( !Class.CastTo( target, m_Meta.target ) || !target.GetInventory() || m_PageArg == "" )
			return;

		int slotId = m_PageArg.ToInt();

		bool canSet = Perm( "ESP.Object.SetAttachment" );

		if ( target.GetInventory().FindAttachment( slotId ) )
			Add( PREFIX_ACTION + "removeattach", "#STR_COT_ESP_MODULE_MENU_REMOVE", JMConstants.Lucide( "trash-2" ), canSet, JMTheme.DANGER );

		m_AttachCandidates = GetSlotCandidates( slotId );

		array< string > candidateLabels = new array< string >;
		for ( int i = 0; i < m_AttachCandidates.Count(); i++ )
			candidateLabels.Insert( GetClassDisplayName( m_AttachCandidates[i] ) );

		for ( int j = 0; j < m_AttachCandidates.Count(); j++ )
		{
			string label = candidateLabels[j];

			// Two different classnames can share one displayName (e.g. skin
			// variants) - fall back to the classname so the rows read as two
			// distinct choices instead of a duplicate that can't be told apart.
			if ( CountOf( candidateLabels, label ) > 1 )
				label = m_AttachCandidates[j];

			Add( PREFIX_ACTION + "setattach:" + j, label, JMConstants.Lucide( "puzzle" ), canSet );
		}
	}

	protected int CountOf( array< string > values, string needle )
	{
		int count = 0;
		for ( int i = 0; i < values.Count(); i++ )
		{
			if ( values[i] == needle )
				count++;
		}

		return count;
	}

	//! m_AttachCandidates stores classnames, not labels - SetAttachment/
	//! RemoveAttachment need the classname, so the row label is resolved
	//! separately here rather than baked into the candidate list itself.
	protected string GetClassDisplayName( string className )
	{
		string raw;

		if ( !g_Game.ConfigGetText( CFG_VEHICLESPATH + " " + className + " displayName", raw ) || raw == "" )
			g_Game.ConfigGetText( CFG_WEAPONSPATH + " " + className + " displayName", raw );

		if ( raw == "" )
			return className;

		return Widget.TranslateString( raw );
	}

	//! Every scope==2 item anywhere in CfgWeapons/CfgVehicles that declares
	//! this slot in its own inventorySlot[] - see the class-level note above
	//! for why this is deliberately a wide net rather than a weapon-specific
	//! compatibility table.
	protected ref array<string> GetSlotCandidates( int slotId )
	{
		array<string> result = new array<string>();

		TStringArray configPaths = new TStringArray();
		configPaths.Insert( CFG_VEHICLESPATH );
		configPaths.Insert( CFG_WEAPONSPATH );

		foreach ( string configPath : configPaths )
		{
			int childCount = g_Game.ConfigGetChildrenCount( configPath );

			for ( int i = 0; i < childCount; i++ )
			{
				string childName;
				g_Game.ConfigGetChildName( configPath, i, childName );

				string path = configPath + " " + childName;
				if ( g_Game.ConfigGetInt( path + " scope" ) != 2 )
					continue;

				string model;
				if ( !g_Game.ConfigGetText( path + " model", model ) || model == "" || model == "bmp" )
					continue;

				array<string> invSlots = new array<string>();
				g_Game.ConfigGetTextArray( path + " inventorySlot", invSlots );

				foreach ( string invSlot : invSlots )
				{
					if ( InventorySlots.GetSlotIdFromString( invSlot ) != slotId )
						continue;

					if ( result.Find( childName ) == -1 )
						result.Insert( childName );

					break;
				}
			}
		}

		return result;
	}

	//! A fireplace's whole lifecycle.
	//!
	//! Refuel is listed above ignite on purpose: a fire with nothing to burn
	//! goes straight back out on its first heating tick, so lighting an empty
	//! one looks like the button did nothing.
	protected void BuildFire()
	{
		AddBack();

		FireplaceBase fireplace;

		if ( !Class.CastTo( fireplace, m_Meta.target ) )
			return;

		bool canSet = Perm( "ESP.Object.Fireplace" );

		Add( PREFIX_ACTION + "firefuel", "#STR_COT_ESP_MODULE_MENU_ADD_FUEL", JMConstants.Lucide( "logs" ), canSet );

		if ( fireplace.IsBurning() )
			Add( PREFIX_ACTION + "extinguish", "#STR_COT_ESP_MODULE_MENU_EXTINGUISH", JMConstants.Lucide( "flame-kindling" ), canSet );
		else
			Add( PREFIX_ACTION + "ignite", "#STR_COT_ESP_MODULE_MENU_IGNITE", JMConstants.Lucide( "flame" ), canSet );

		Add( PREFIX_ACTION + "cookall", "#STR_COT_ESP_MODULE_MENU_COOK_ALL", JMConstants.Lucide( "utensils" ), canSet );

		AddToggle( PREFIX_ACTION + "stones", "#STR_COT_ESP_MODULE_ACTION_STONE_CIRCLE", fireplace.HasStoneCircle(), canSet );
		AddToggle( PREFIX_ACTION + "oven", "#STR_COT_ESP_MODULE_ACTION_OVEN", fireplace.IsOven(), canSet );
	}

	protected void BuildScale()
	{
		Add( PREFIX_PAGE + PAGE_PLAYER, "#STR_COT_ESP_MODULE_MENU_BACK", JMConstants.Lucide( "chevron-left" ) );

		bool canSet = Perm( "Admin.Player.Scale" );

		TStringArray values = new TStringArray;
		SCALE_VALUES.Split( "|", values );

		for ( int i = 0; i < values.Count(); i++ )
			Add( PREFIX_ACTION + "scale:" + values[i], values[i] + "x", "", canSet );
	}

	//! Reachable only when BuildMain already checked ESP.Object.ChangeColor
	//! to offer this page at all, so every row in it is enabled outright.
	protected void BuildColor()
	{
		AddBack();

		EntityAI entity = EntityAI.Cast( m_Meta.target );

		// Color is a whole-object property - a wheel or door opened this page
		// as a sub-part, but painting is always done on the thing it belongs
		// to, never the part itself.
		if ( entity && entity.GetHierarchyRoot() )
			entity = EntityAI.Cast( entity.GetHierarchyRoot() );

		array<string> tokens = new array<string>;

		JMObjectSpawnerModule objSpawner;
		if ( entity && CF_Modules<JMObjectSpawnerModule>.Get( objSpawner ) )
			tokens = objSpawner.GetAvailableColorVariants( entity );

		if ( tokens.Count() == 0 )
		{
			Add( "", "#STR_COT_ESP_MODULE_MENU_NO_COLOR_VARIANTS", "", false );
			return;
		}

		for ( int i = 0; i < tokens.Count(); i++ )
		{
			int wireIndex = JMObjectSpawnerModule.GetColorTokenIndex( tokens[i] );
			Add( PREFIX_ACTION + "changecolor:" + wireIndex, tokens[i], JMConstants.Lucide( "palette" ) );
		}
	}

	protected void BuildCopy()
	{
		AddBack();

		Add( PREFIX_ACTION + "copyraw",       "#STR_COT_ESP_MODULE_MENU_COPY_RAW",       JMConstants.Lucide( "copy" ) );
		Add( PREFIX_ACTION + "copyxml",       "#STR_COT_ESP_MODULE_MENU_COPY_XML",       JMConstants.Lucide( "file-code" ) );
		Add( PREFIX_ACTION + "copyspawnable", "#STR_COT_ESP_MODULE_MENU_COPY_SPAWNABLE", JMConstants.Lucide( "code-xml" ) );
		Add( PREFIX_ACTION + "copyexpansion", "#STR_COT_ESP_MODULE_MENU_COPY_EXPANSION", JMConstants.Lucide( "copy" ) );
#ifdef DZ_Expansion_Core
		Add( PREFIX_ACTION + "copyloadout",   "#STR_COT_ESP_MODULE_MENU_COPY_LOADOUT",   JMConstants.Lucide( "box" ) );
#endif
	}

	//! A player's own copy row - name, GUID, Steam64 - is identity data, not
	//! the classname/xml/spawnable family BuildCopy holds, so it gets its own
	//! page rather than being folded into that one.
	protected void BuildCopyPlayer()
	{
		AddBack();

		Add( PREFIX_ACTION + "copyname",  "#STR_COT_ESP_MODULE_MENU_COPY_NAME",  JMConstants.Lucide( "copy" ) );
		Add( PREFIX_ACTION + "copyguid",  "#STR_COT_ESP_MODULE_MENU_COPY_GUID",  JMConstants.Lucide( "copy" ) );
		Add( PREFIX_ACTION + "copysteam", "#STR_COT_ESP_MODULE_MENU_COPY_STEAM", JMConstants.Lucide( "copy" ) );
	}

	protected void BuildPlayer()
	{
		AddBack();

		JMPlayerInstance instance = TargetPlayer();

		if ( !instance )
			return;

		Add( PREFIX_ACTION + "spectate", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_SPECTATE", JMConstants.Lucide( "eye" ) );
		Add( PREFIX_ACTION + "heal", "#STR_COT_ESP_MODULE_MENU_HEAL_ALL", JMConstants.Lucide( "heart-pulse" ), Perm( "ESP.Object.Heal" ) );

		//! These are player states the player module already owns, so they are
		//! gated on ITS permissions and routed through it, webhooks and all.
		AddToggle( PREFIX_ACTION + "uncon", "#STR_COT_ESP_MODULE_ACTION_UNCONSCIOUS", instance.IsUnconscious(), Perm( "Admin.Player.Set.Shock" ) );
		AddToggle( PREFIX_ACTION + "freeze", "#STR_COT_ESP_MODULE_ACTION_FREEZE", instance.IsFrozen(), Perm( "Admin.Player.Freeze" ) );
		AddToggle( PREFIX_ACTION + "legs", "#STR_COT_ESP_MODULE_ACTION_BROKEN_LEGS", instance.HasBrokenLegs(), Perm( "Admin.Player.BrokenLegs" ) );
#ifndef DAYZ_1_29
		AddToggle( PREFIX_ACTION + "ragdoll", "#STR_COT_PLAYER_MODULE_ACTION_RAGDOLL", instance.IsRagdoll(), Perm( "Admin.Player.Ragdoll" ) );
#endif

		AddPage( PAGE_SCALE, "#STR_COT_ESP_MODULE_PAGE_SCALE", JMConstants.Lucide( "scaling" ), Perm( "Admin.Player.Scale" ) );
		AddPage( PAGE_COPY_PLAYER, "#STR_COT_ESP_MODULE_MENU_COPY", JMConstants.Lucide( "copy" ) );

		//! A menu cannot be typed into, so the message body comes off the
		//! clipboard - the same route the exact position and the exact lock code
		//! already take.
		Add( PREFIX_ACTION + "msg", "#STR_COT_ESP_MODULE_MENU_MESSAGE", JMConstants.Lucide( "message-square" ), Perm( "Admin.Player.Message" ) );

		//! Destructive, so tinted like delete is - the same bar this menu sets
		//! everywhere else for something that cannot be taken back.
		Add( PREFIX_ACTION + "clearcargo", "#STR_COT_PLAYER_MODULE_ACTION_CLEAR_CARGO", JMConstants.Lucide( "package-x" ), Perm( "Admin.Player.ClearCargo" ), JMTheme.DANGER );
		Add( PREFIX_ACTION + "strip", "#STR_COT_PLAYER_MODULE_ACTION_STRIP", JMConstants.Lucide( "shirt" ), Perm( "Admin.Player.Strip" ), JMTheme.DANGER );
		Add( PREFIX_ACTION + "kick", "#STR_COT_PLAYER_MODULE_ACTION_KICK", JMConstants.Lucide( "door-open" ), Perm( "Admin.Player.Kick" ), JMTheme.DANGER );
		Add( PREFIX_ACTION + "ban", "#STR_COT_PLAYER_MODULE_ACTION_BAN", JMConstants.Lucide( "gavel" ), Perm( "Admin.Player.Ban" ), JMTheme.DANGER );
	}

	// =========================================================================
	//  Routing
	// =========================================================================

	void OnMenuClick( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_Meta || !m_Meta.target )
			return;

		string id = m_Menu.GetLastClickedId();

		if ( id.IndexOf( PREFIX_PAGE ) == 0 )
		{
			GoToPage( id.Substring( 2, id.Length() - 2 ) );
			return;
		}

		if ( id.IndexOf( PREFIX_ACTION ) != 0 )
			return;

		RunAction( id.Substring( 2, id.Length() - 2 ), action );
	}

	//! "part:foo" is the only page that carries an argument; everything else is
	//! a bare page name.
	protected void GoToPage( string page )
	{
		int split = page.IndexOf( ":" );

		if ( split > -1 )
		{
			m_PageArg = page.Substring( split + 1, page.Length() - split - 1 );
			m_Page = page.Substring( 0, split );
		}
		else
		{
			m_PageArg = "";
			m_Page = page;
		}

		Show();
	}

	protected void RunAction( string id, UIActionBase action )
	{
		//! Value-carrying actions all look like "name:value".
		string name = id;
		string arg = "";

		int split = id.IndexOf( ":" );

		if ( split > -1 )
		{
			name = id.Substring( 0, split );
			arg = id.Substring( split + 1, id.Length() - split - 1 );
		}

		if ( name == "select" )
			DoSelect( true );
		else if ( name == "deselect" )
			DoSelect( false );
		else if ( name == "copytype" )
			DoCopyTypeWithAttachments();
		else if ( name == "copyraw" )
			DoCopyTypeRaw();
		else if ( name == "copyxml" )
			DoCopyTypesXml();
		else if ( name == "copyspawnable" )
			DoCopySpawnableTypes();
		else if ( name == "copyexpansion" )
			DoCopyTypeWithAttachments();
#ifdef DZ_Expansion_Core
		else if ( name == "copyloadout" )
			DoCopyExpLoadout();
#endif
		else if ( name == "delete" )
			DoDelete();
		else if ( name == "copypos" )
			g_Game.CopyToClipboard( m_Meta.target.GetPosition().ToString() );
		else if ( name == "copyori" )
			g_Game.CopyToClipboard( m_Meta.target.GetOrientation().ToString() );
		else if ( name == "tpme" )
			DoTeleportSelfTo();
		else if ( name == "tphere" )
			DoTeleportHere();
		else if ( name == "tpundo" )
			DoTeleportUndo();
		else if ( name == "tpredo" )
			DoTeleportRedo();
		else if ( name == "jam" )
			DoWeaponAction( JMESPObjectAction.SetJammed, Flip( WeaponIsJammed() ) );
		else if ( name == "chamber" )
			DoWeaponAction( JMESPObjectAction.SetChambered, 1 );
		else if ( name == "unchamber" )
			DoWeaponAction( JMESPObjectAction.SetChambered, 0 );
		else if ( name == "loadmag" )
			DoWeaponAction( JMESPObjectAction.LoadMagazine, 0 );
		else if ( name == "fillinternal" )
			DoWeaponAction( JMESPObjectAction.FillInternalMagazine, 0 );
		else if ( name == "removeattach" )
			DoRemoveAttachment();
		else if ( name == "setattach" )
			DoSetAttachment( arg.ToInt() );
		else if ( name == "ignite" )
			DoFireAction( JMESPObjectAction.SetBurning, 1 );
		else if ( name == "extinguish" )
			DoFireAction( JMESPObjectAction.SetBurning, 0 );
		else if ( name == "firefuel" )
			DoFireAction( JMESPObjectAction.FireplaceRefuel, 0 );
		else if ( name == "cookall" )
			DoFireAction( JMESPObjectAction.CookAll, 0 );
		else if ( name == "stones" )
			DoFireAction( JMESPObjectAction.SetStoneCircle, Flip( FireHasStones() ) );
		else if ( name == "oven" )
			DoFireAction( JMESPObjectAction.SetOven, Flip( FireIsOven() ) );
		else if ( name == "lockall" )
			DoLockAll( 1 );
		else if ( name == "unlockall" )
			DoLockAll( 0 );
		else if ( name == "parthp" )
			DoPartHealth( arg.ToFloat() * 0.01 );
		else if ( name == "scale" )
			DoScale( arg.ToFloat() );
		else if ( name == "msg" )
			DoMessage();
		else if ( name == "copyname" )
			DoCopyName();
		else if ( name == "copyguid" )
			DoCopyGUID();
		else if ( name == "copysteam" )
			DoCopySteam();
		else if ( name == "strip" )
			DoPlayerVerb( "strip" );
		else if ( name == "clearcargo" )
			DoPlayerVerb( "clearcargo" );
		else if ( name == "kick" )
			DoPlayerVerb( "kick" );
		else if ( name == "ban" )
			DoPlayerVerb( "ban" );
		else if ( name == "pastepos" )
			DoPastePosition();
		else if ( name == "pasteori" )
			DoPasteOrientation();
		else if ( name == "ground" )
			DoSnapToGround();
		else if ( name == "facecam" )
			DoFaceCamera();
		else if ( name == "health" )
			DoHealth( arg.ToFloat() );
		else if ( name == "heal" )
			DoHeal();
		else if ( name == "quantity" )
			DoQuantity( arg.ToFloat() );
		else if ( name == "liquid" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetLiquid, arg.ToInt(), 0, m_Meta.target );
		else if ( name == "food" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetFoodStage, arg.ToInt(), 0, m_Meta.target );
		else if ( name == "disinfect" )
			DoDisinfect();
		else if ( name == "openclose" )
			DoOpenClose();
		else if ( name == "lock" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetLock, 1, 0, m_Meta.target );
		else if ( name == "unlock" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetLock, 0, 0, m_Meta.target );
		else if ( name == "getcode" )
			m_Meta.module.ObjectAction( JMESPObjectAction.GetCode, 0, 0, m_Meta.target );
		else if ( name == "pastecode" )
			DoPasteCode();
		else if ( name == "build" )
			DoConstruction( 0 );
		else if ( name == "dismantle" )
			DoConstruction( 1 );
		else if ( name == "repairpart" )
			DoConstruction( 2 );
		else if ( name == "refuel" )
			DoRefuel();
		else if ( name == "fuel" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetFuel, 0, arg.ToFloat() * 0.01, m_Meta.target );
		else if ( name == "coolant" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetCoolant, 0, arg.ToFloat() * 0.01, m_Meta.target );
		else if ( name == "flag" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetFlagRaised, 0, arg.ToFloat() * 0.01, m_Meta.target );
		else if ( name == "lockwheels" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetLockWheels, 1, 0, m_Meta.target );
		else if ( name == "unlockwheels" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetLockWheels, 0, 0, m_Meta.target );
		else if ( name == "repairfill" )
			m_Meta.module.ObjectAction( JMESPObjectAction.RepairAndFillSlots, 0, 0, m_Meta.target );
		else if ( name == "changecolor" )
			m_Meta.module.ObjectAction( JMESPObjectAction.ChangeColor, arg.ToInt(), 0, m_Meta.target );
		else if ( name == "pingrenade" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetGrenadePin, 1, 0, m_Meta.target );
		else if ( name == "unpingrenade" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetGrenadePin, 0, 0, m_Meta.target );
		else if ( name == "triggertrap" )
			m_Meta.module.ObjectAction( JMESPObjectAction.TriggerTrap, 0, 0, m_Meta.target );
		else if ( name == "armtrap" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetTrapArmed, 1, 0, m_Meta.target );
		else if ( name == "disarmtrap" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetTrapArmed, 0, 0, m_Meta.target );
		else if ( name == "immobilize" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetImmobilized, Flip( CreatureIsImmobilized() ), 0, m_Meta.target );
		else if ( name == "objclearcargo" )
			m_Meta.module.ObjectAction( JMESPObjectAction.ClearCargo, 0, 0, m_Meta.target );
		else if ( name == "buildall" )
			m_Meta.module.ObjectAction( JMESPObjectAction.BuildAll, 0, 0, m_Meta.target );
		else if ( name == "dismantleall" )
			m_Meta.module.ObjectAction( JMESPObjectAction.DismantleAll, 0, 0, m_Meta.target );
		else if ( name == "repairall" )
			m_Meta.module.ObjectAction( JMESPObjectAction.RepairAll, 0, 0, m_Meta.target );
		else if ( name == "openfence" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetFenceOpen, 1, 0, m_Meta.target );
		else if ( name == "closefence" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetFenceOpen, 0, 0, m_Meta.target );
		else if ( name == "opentent" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetTentOpen, 1, 0, m_Meta.target );
		else if ( name == "closetent" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetTentOpen, 0, 0, m_Meta.target );
		else if ( name == "opendoor" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetHouseDoorOpen, 1, m_Meta.m_DoorIndex, m_Meta.target );
		else if ( name == "closedoor" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetHouseDoorOpen, 0, m_Meta.m_DoorIndex, m_Meta.target );
		else if ( name == "lockdoor" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetHouseDoorLocked, 1, m_Meta.m_DoorIndex, m_Meta.target );
		else if ( name == "unlockdoor" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetHouseDoorLocked, 0, m_Meta.m_DoorIndex, m_Meta.target );
		else if ( name == "opencardoors" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetCarDoors, 1, 0, m_Meta.target );
		else if ( name == "closecardoors" )
			m_Meta.module.ObjectAction( JMESPObjectAction.SetCarDoors, 0, 0, m_Meta.target );
		else if ( name == "unstuck" )
			DoUnstuck();
		else if ( name == "vehunlock" )
			DoVehicleLock();
		else if ( name == "vehunpair" )
			DoVehicleUnPair();
		else if ( name == "spectate" )
			DoSpectate( action );
		else if ( name == "uncon" )
			DoUnconscious();
		else if ( name == "freeze" )
			DoFreeze();
		else if ( name == "legs" )
			DoBrokenLegs();
#ifndef DAYZ_1_29
		else if ( name == "ragdoll" )
			DoRagdoll();
#endif
	}

	protected void DoCopyTypeRaw()
	{
		if ( !m_Meta || !m_Meta.target )
			return;

		g_Game.CopyToClipboard( m_Meta.GetType() );
	}

	//! <type name="..."/> block - the types.xml per-object entry.
	protected void DoCopyTypesXml()
	{
		if ( !m_Meta || !m_Meta.target )
			return;

		string xml = "<type name=\"" + m_Meta.GetType() + "\">\n";
		xml += "</type>";
		g_Game.CopyToClipboard( xml );
	}

	//! Full <spawnabletypes> XML for this one object, matching the
	//! format JMESPModule.CopyToClipboardSpawnableTypes produces for
	//! the selected-objects set - but scoped to m_Meta.target.
	protected void DoCopySpawnableTypes()
	{
		if ( !m_Meta || !m_Meta.target )
			return;

		string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\n";
		xml += "<spawnabletypes>\n";
		xml += "\t<type name=\"" + m_Meta.GetType() + "\">\n";

		EntityAI entity = EntityAI.Cast( m_Meta.target );
		if ( entity && entity.GetInventory() )
		{
			int attCount = entity.GetInventory().AttachmentCount();
			for ( int a = 0; a < attCount; ++a )
			{
				EntityAI att = entity.GetInventory().GetAttachmentFromIndex( a );
				if ( !att )
					continue;
				xml += "\t\t<attachments chance=\"1.00\">\n";
				xml += "\t\t\t<item name=\"" + att.GetType() + "\" chance=\"1.00\" />\n";
				xml += "\t\t</attachments>\n";
			}

			CargoBase cargo = entity.GetInventory().GetCargo();
			if ( cargo )
			{
				int cargoCount = cargo.GetItemCount();
				for ( int c = 0; c < cargoCount; ++c )
				{
					xml += "\t\t<cargo chance=\"1.00\">\n";
					xml += "\t\t\t<item name=\"" + cargo.GetItem( c ).GetType() + "\" />\n";
					xml += "\t\t</cargo>\n";
				}
			}
		}

		xml += "\t</type>\n";
		xml += "</spawnabletypes>\n";
		g_Game.CopyToClipboard( xml );
	}

#ifdef DZ_Expansion_Core
	protected void DoCopyExpLoadout()
	{
		if ( !m_Meta || !m_Meta.target )
			return;

		EntityAI entity = EntityAI.Cast( m_Meta.target );
		if ( !entity )
			return;

		ExpansionPrefab expPrefab = new ExpansionPrefab();

		if ( entity.IsMan() )
		{
			m_Meta.module.AddChildrenToExpLoadoutRecursive( expPrefab, entity );
		}
		else
		{
			expPrefab.ClassName = entity.GetType();
			m_Meta.module.AddToExpLoadoutRecursive( expPrefab, entity );
		}

		string loadoutJSON;
		string errorMsg;
		if ( JsonFileLoader<ExpansionPrefab>.MakeData( expPrefab, loadoutJSON, errorMsg ) )
			g_Game.CopyToClipboard( loadoutJSON );
		else
			COTCreateLocalAdminNotification( new StringLocaliser( errorMsg ) );
	}
#endif

	protected void DoCopyTypeWithAttachments()
	{
		if ( !m_Meta || !m_Meta.target )
			return;

		EntityAI entity = EntityAI.Cast( m_Meta.target );
		if ( !entity )
		{
			g_Game.CopyToClipboard( m_Meta.GetType() );
			return;
		}

		string copyStr = "";
		BuildClassnameWithAttachments( entity, copyStr );
		g_Game.CopyToClipboard( copyStr );
	}

	protected static void BuildClassnameWithAttachments( EntityAI entity, inout string result )
	{
		if ( !entity )
			return;

		if ( result != "" )
			result += " ";

		result += entity.GetType();

		if ( !entity.GetInventory() )
			return;

		int attCount = entity.GetInventory().AttachmentCount();
		for ( int i = 0; i < attCount; ++i )
		{
			EntityAI att = entity.GetInventory().GetAttachmentFromIndex( i );
			if ( att )
			{
				BuildClassnameWithAttachments( att, result );
			}
		}
	}

	protected void DoSelect( bool selected )
	{
		if ( m_Meta.widgetHandler )
		{
			if ( selected )
				m_Meta.widgetHandler.Select();
			else
				m_Meta.widgetHandler.Deselect();

			return;
		}

		//! No tag on screen for this object - the list can still select it.
		if ( selected )
			JMScriptInvokers.ADD_OBJECT.Invoke( m_Meta.target );
		else
			JMScriptInvokers.REMOVE_OBJECT.Invoke( m_Meta.target, m_Meta.networkLow, m_Meta.networkHigh );
	}

	protected void DoDelete()
	{
		if ( !Perm( "ESP.Object.Delete" ) )
			return;

		if ( IsMissionOffline() || ( !m_Meta.networkLow && !m_Meta.networkHigh ) )
			m_Meta.module.DeleteObject( m_Meta.target );
		else
			m_Meta.module.DeleteObject( m_Meta.networkLow, m_Meta.networkHigh );

		m_Meta.m_TargetDeleted = true;
	}

	//! Move the admin to the tracked object.
	protected void DoTeleportSelfTo()
	{
		if ( !Perm( "Admin.Player.Teleport.Position" ) )
			return;

		JMTeleportModule teleportModule = CF_Modules<JMTeleportModule>.Get();

		JMPlayerInstance self = GetPermissionsManager().GetClientPlayer();

		if ( !teleportModule || !self )
			return;

		teleportModule.Position( m_Meta.target.GetPosition(), { self.GetGUID() } );
	}

	//! Move the tracked object to the admin. A player goes through the teleport
	//! module so the move is logged as a teleport; anything else is a position
	//! set, which is what ESP has always called it.
	protected void DoTeleportHere()
	{
		Man self = g_Game.GetPlayer();

		if ( !self )
			return;

		vector pos = self.GetPosition();

		JMPlayerInstance targetPlayer = TargetPlayer();

		if ( targetPlayer )
		{
			if ( !Perm( "Admin.Player.Teleport.Position" ) )
				return;

			JMTeleportModule teleportModule = CF_Modules<JMTeleportModule>.Get();

			if ( !teleportModule )
				return;

			//! The teleport module records the history for a player, the same
			//! way it does for every other path into it - so this branch pushes
			//! nothing of its own.
			teleportModule.Position( pos, { targetPlayer.GetGUID() } );

			return;
		}

		if ( !Perm( "ESP.Object.SetPosition" ) )
			return;

		//! A player's move is recorded inside the teleport module, which every
		//! path into it shares. Anything else is moved by a raw position set,
		//! so this call site records its own.
		JMTeleportHistory.PushObject( m_Meta.target );

		m_Meta.module.SetPosition( pos, m_Meta.target );
	}

	//! The tracked player, however this client can name one.
	//!
	//! m_Meta.player comes from the target's identity, and a client only holds
	//! identities for players it can see - so it is null for most of the roster.
	//! Falling back to the shared lookup keeps this menu acting on the same
	//! player the player manager does, which is what makes one teleport history
	//! serve both.
	protected JMPlayerInstance TargetPlayer()
	{
		if ( m_Meta.player )
			return m_Meta.player;

		return JMTeleportHistory.PlayerOf( m_Meta.target );
	}

	protected bool HasTeleportUndo()
	{
		return JMTeleportHistory.Has( JMTeleportHistory.KeyFor( m_Meta.target ) );
	}

	protected bool HasTeleportRedo()
	{
		return JMTeleportHistory.HasRedo( JMTeleportHistory.KeyFor( m_Meta.target ) );
	}

	//! Put back a step an undo took away. Only ever reaches somewhere the target
	//! has actually been, so it cannot invent a move nobody made.
	protected void DoTeleportRedo()
	{
		JMTeleportHistory.RedoTarget( m_Meta.target, 0 );
	}

	// =========================================================================
	//  Weapons and fireplaces
	// =========================================================================

	//! The int that toggles a state.
	//!
	//! These actions carry their on/off as the ivalue of a generic action, and
	//! Enforce will not narrow a bool into an int parameter - so the negation
	//! has to become a number here rather than at the call site.
	protected int Flip( bool state )
	{
		if ( state )
			return 0;

		return 1;
	}

	//! Whether this weapon takes a detachable magazine at all.
	//!
	//! Read off the weapon's own magazines[] the same way the server loads one,
	//! so the row is offered exactly when the action behind it can do something.
	protected bool WeaponAcceptsMagazine( Weapon_Base weapon )
	{
		if ( !weapon )
			return false;

		TStringArray magazines = new TStringArray;
		g_Game.ConfigGetTextArray( "CfgWeapons " + weapon.GetType() + " magazines", magazines );

		if ( magazines.Count() == 0 )
			g_Game.ConfigGetTextArray( "CfgVehicles " + weapon.GetType() + " magazines", magazines );

		return magazines.Count() > 0;
	}

	//! Mirrors Exec_LoadMagazine's own "already wearing one" loop so the menu
	//! label agrees with what the click will actually do.
	protected bool WeaponHasMagazineAttached( Weapon_Base weapon )
	{
		if ( !weapon || !weapon.GetInventory() )
			return false;

		Magazine existing;

		for ( int i = 0; i < weapon.GetInventory().AttachmentCount(); i++ )
		{
			if ( Class.CastTo( existing, weapon.GetInventory().GetAttachmentFromIndex( i ) ) )
				return true;
		}

		return false;
	}

	protected bool WeaponIsJammed()
	{
		Weapon_Base weapon;

		if ( !Class.CastTo( weapon, m_Meta.target ) )
			return false;

		return weapon.IsJammed();
	}

	protected bool FireHasStones()
	{
		FireplaceBase fireplace;

		if ( !Class.CastTo( fireplace, m_Meta.target ) )
			return false;

		return fireplace.HasStoneCircle();
	}

	protected bool FireIsOven()
	{
		FireplaceBase fireplace;

		if ( !Class.CastTo( fireplace, m_Meta.target ) )
			return false;

		return fireplace.IsOven();
	}

	protected void DoWeaponAction( int action, int value )
	{
		if ( !Perm( "ESP.Object.Weapon" ) )
			return;

		m_Meta.module.ObjectAction( action, value, 0, m_Meta.target );
	}

	//! m_PageArg is the slot id BuildAttachSlot() was showing - see that
	//! method's own comment for why.
	protected void DoRemoveAttachment()
	{
		if ( !Perm( "ESP.Object.Weapon" ) || m_PageArg == "" )
			return;

		m_Meta.module.RemoveAttachment( m_Meta.target, m_PageArg.ToInt() );
	}

	//! `index` resolves against m_AttachCandidates as it stood when this page
	//! was last built - see BuildAttachSlot().
	protected void DoSetAttachment( int index )
	{
		if ( !Perm( "ESP.Object.Weapon" ) || m_PageArg == "" )
			return;

		if ( !m_AttachCandidates || index < 0 || index >= m_AttachCandidates.Count() )
			return;

		m_Meta.module.SetAttachment( m_Meta.target, m_PageArg.ToInt(), m_AttachCandidates[index] );
	}

	protected void DoFireAction( int action, int value )
	{
		if ( !Perm( "ESP.Object.Fireplace" ) )
			return;

		m_Meta.module.ObjectAction( action, value, 0, m_Meta.target );
	}

	protected void DoLockAll( int locked )
	{
		if ( !Perm( "ESP.Object.Lock" ) )
			return;

		m_Meta.module.ObjectAction( JMESPObjectAction.SetLockAll, locked, 0, m_Meta.target );
	}

	//! Set the health of the part whose page this was opened from.
	//!
	//! m_PageArg is the part name, carried through the page id - the health page
	//! is reached from one part's page and acts only on that part.
	protected void DoPartHealth( float health01 )
	{
		if ( !Perm( "ESP.Object.BaseBuilding.SetHealth" ) )
			return;

		BaseBuildingBase bb;

		if ( !Class.CastTo( bb, m_Meta.target ) || m_PageArg == "" )
			return;

		m_Meta.module.BaseBuilding_SetPartHealth( bb, m_PageArg, health01 );
	}

	// =========================================================================
	//  Player verbs
	//
	//  Routed through JMPlayerModule rather than reimplemented here, so each one
	//  is logged, webhooked and permissioned exactly as the same button in the
	//  player manager is.
	// =========================================================================

	protected JMPlayerModule PlayerModule()
	{
		JMPlayerModule playerModule;

		if ( !Class.CastTo( playerModule, GetModuleManager().GetModule( JMPlayerModule ) ) )
			return NULL;

		return playerModule;
	}

	protected void DoScale( float scale )
	{
		JMPlayerInstance instance = TargetPlayer();
		JMPlayerModule playerModule = PlayerModule();

		if ( !instance || !playerModule || !Perm( "Admin.Player.Scale" ) )
			return;

		playerModule.SetScale( scale, { instance.GetGUID() } );
	}

	//! Send whatever is on the clipboard as a message.
	//!
	//! A context menu cannot be typed into, and this file already says so: the
	//! genuinely free-form values - an exact position, an exact lock code - go
	//! through the clipboard, and a message body is one more of those.
	protected void DoMessage()
	{
		JMPlayerInstance instance = TargetPlayer();
		JMPlayerModule playerModule = PlayerModule();

		if ( !instance || !playerModule || !Perm( "Admin.Player.Message" ) )
			return;

		JMESPModule espModule = CF_Modules<JMESPModule>.Get();
		if ( espModule )
		{
			JMESPForm form = JMESPForm.Cast( espModule.GetForm() );
			if ( form )
			{
				form.PromptSendMessage( instance.GetGUID(), instance.GetName() );
			}
		}
	}

	protected void DoCopyName()
	{
		JMPlayerInstance instance = TargetPlayer();

		if ( !instance )
			return;

		g_Game.CopyToClipboard( instance.GetName() );
	}

	protected void DoCopyGUID()
	{
		JMPlayerInstance instance = TargetPlayer();

		if ( !instance )
			return;

		g_Game.CopyToClipboard( instance.GetGUID() );
	}

	protected void DoCopySteam()
	{
		JMPlayerInstance instance = TargetPlayer();

		if ( !instance )
			return;

		g_Game.CopyToClipboard( instance.GetSteam64ID() );
	}

	//! The four destructive player verbs, by name.
	//!
	//! One function rather than four near-identical ones: they differ only in
	//! the permission they need and the call they make, and every one of them
	//! resolves the same target the same way first.
	protected void DoPlayerVerb( string verb )
	{
		JMPlayerInstance instance = TargetPlayer();
		JMPlayerModule playerModule = PlayerModule();

		if ( !instance || !playerModule )
			return;

		array<string> guids = { instance.GetGUID() };

		if ( verb == "strip" && Perm( "Admin.Player.Strip" ) )
			playerModule.Strip( guids );
		else if ( verb == "clearcargo" && Perm( "Admin.Player.ClearCargo" ) )
			playerModule.ClearCargo( guids );
		else if ( verb == "kick" && Perm( "Admin.Player.Kick" ) )
			playerModule.Kick( guids, "" );
		else if ( verb == "ban" && Perm( "Admin.Player.Ban" ) )
			playerModule.Ban( guids, "" );
	}

	//! Put this target back where it was before its last move. Each click walks
	//! one more step back, up to JMTeleportHistory.MAX_ENTRIES.
	//!
	//! Routed by what the target IS, not by this being the ESP menu - so a
	//! player teleported from the player manager is undone from here, and a
	//! vehicle moved from here is undone from the vehicle list.
	protected void DoTeleportUndo()
	{
		JMTeleportHistory.UndoTarget( m_Meta.target, 0 );
	}

	protected void DoPastePosition()
	{
		if ( !Perm( "ESP.Object.SetPosition" ) )
			return;

		string clipboard;
		g_Game.CopyFromClipboard( clipboard );

		vector pos = clipboard.BeautifiedToVector();

		if ( pos == vector.Zero )
			return;

		//! Pasting coordinates is a teleport by another name, so it is a step
		//! the undo should be able to take back.
		JMTeleportHistory.PushObject( m_Meta.target );

		m_Meta.module.SetPosition( pos, m_Meta.target );
	}

	protected void DoPasteOrientation()
	{
		if ( !Perm( "ESP.Object.SetOrientation" ) )
			return;

		string clipboard;
		g_Game.CopyFromClipboard( clipboard );

		vector ori = clipboard.BeautifiedToVector();

		if ( ori == vector.Zero )
			return;

		m_Meta.module.SetOrientation( ori, m_Meta.target );
	}

	//! Drop the object onto the terrain under it. Started well above its own
	//! position so an object already sunk into the ground still finds the
	//! surface rather than the inside of itself.
	protected void DoSnapToGround()
	{
		if ( !Perm( "ESP.Object.SetPosition" ) )
			return;

		vector pos = m_Meta.target.GetPosition();

		vector from = pos + "0 20 0";
		vector to = pos - "0 200 0";

		vector contactPos;
		vector contactDir;
		int contactComponent;

		if ( !DayZPhysics.RaycastRV( from, to, contactPos, contactDir, contactComponent, NULL, NULL, m_Meta.target, false, true ) )
			return;

		m_Meta.module.SetPosition( contactPos, m_Meta.target );
	}

	protected void DoFaceCamera()
	{
		if ( !Perm( "ESP.Object.SetOrientation" ) )
			return;

		vector toCamera = g_Game.GetCurrentCameraPosition() - m_Meta.target.GetPosition();
		toCamera[1] = 0;

		if ( toCamera.Length() < 0.01 )
			return;

		vector ori = m_Meta.target.GetOrientation();
		vector angles = toCamera.VectorToAngles();

		ori[0] = angles[0];

		m_Meta.module.SetOrientation( ori, m_Meta.target );
	}

	protected void DoHealth( float percent )
	{
		if ( !Perm( "ESP.Object.SetHealth" ) )
			return;

		m_Meta.module.SetHealth( MaxHealth() * percent * 0.01, "", m_Meta.target );
	}

	protected void DoHeal()
	{
		if ( !Perm( "ESP.Object.Heal" ) )
			return;

		if ( ( m_Meta.target.IsInherited( Man ) || m_Meta.target.IsInherited( DayZCreature ) ) && m_Meta.target.IsDamageDestroyed() )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIFICATION_ERROR_CANNOT_HEAL_DEAD_CREATURE" ) );
			return;
		}

		m_Meta.module.Heal( m_Meta.target );
	}

	protected void DoQuantity( float percent )
	{
		ItemBase item = ItemBase.Cast( m_Meta.target );

		if ( !item )
			return;

		m_Meta.module.ObjectAction( JMESPObjectAction.SetQuantity, 0, item.GetQuantityMax() * percent * 0.01, m_Meta.target );
	}

	protected void DoDisinfect()
	{
		ItemBase item = ItemBase.Cast( m_Meta.target );

		if ( !item )
			return;

		int cleanness = 1;

		if ( item.GetCleanness() > 0 )
			cleanness = 0;

		m_Meta.module.ObjectAction( JMESPObjectAction.SetCleanness, cleanness, 0, m_Meta.target );
	}

	protected void DoOpenClose()
	{
		ItemBase item = ItemBase.Cast( m_Meta.target );

		if ( !item )
			return;

		int open = 1;

		if ( item.IsOpen() )
			open = 0;

		m_Meta.module.ObjectAction( JMESPObjectAction.SetOpen, open, 0, m_Meta.target );
	}

	//! A code is four free-form digits, which a menu has no way to ask for -
	//! so it comes in the way it went out, through the clipboard.
	protected void DoPasteCode()
	{
		string clipboard;
		g_Game.CopyFromClipboard( clipboard );

		clipboard = clipboard.Trim();

		if ( clipboard == "" )
			return;

		m_Meta.module.ObjectAction( JMESPObjectAction.SetCode, clipboard.ToInt(), 0, m_Meta.target );
	}

	protected void DoConstruction( int what )
	{
		BaseBuildingBase building = BaseBuildingBase.Cast( m_Meta.target );

		if ( !building || m_PageArg == "" )
			return;

		if ( what == 0 )
			m_Meta.module.BaseBuilding_Build( building, m_PageArg );
		else if ( what == 1 )
			m_Meta.module.BaseBuilding_Dismantle( building, m_PageArg );
		else
			m_Meta.module.BaseBuilding_Repair( building, m_PageArg );
	}

	protected void DoRefuel()
	{
		if ( !Perm( "ESP.Object.Car.Refuel" ) )
			return;

		m_Meta.module.Vehicle_Refuel( m_Meta.target );
	}

	protected void DoUnstuck()
	{
		if ( !Perm( "ESP.Object.Car.Unstuck" ) )
			return;

		m_Meta.module.Vehicle_Unstuck( m_Meta.target );
	}

	//! Both of these are network-id only requests, so a tracked object with no
	//! net id (offline, or scenery) has nothing to send and is skipped.
	protected void DoVehicleLock()
	{
	#ifdef EXPANSIONMODVEHICLE
		if ( !Perm( "Vehicles.Lock" ) )
			return;

		JMVehiclesModule vehiclesModule = CF_Modules<JMVehiclesModule>.Get();

		if ( !vehiclesModule || ( !m_Meta.networkLow && !m_Meta.networkHigh ) )
			return;

		vehiclesModule.RequestLockVehicleById( m_Meta.networkLow, m_Meta.networkHigh );
	#endif
	}

	protected void DoVehicleUnPair()
	{
	#ifdef EXPANSIONMODVEHICLE
		if ( !Perm( "Vehicles.UnPair" ) )
			return;

		JMVehiclesModule vehiclesModule = CF_Modules<JMVehiclesModule>.Get();

		if ( !vehiclesModule || ( !m_Meta.networkLow && !m_Meta.networkHigh ) )
			return;

		vehiclesModule.RequestUnPairVehicleById( m_Meta.networkLow, m_Meta.networkHigh );
	#endif
	}

	protected void DoSpectate( UIActionBase action )
	{
		JMPlayerModule playerModule = CF_Modules<JMPlayerModule>.Get();

		if ( !playerModule )
			return;

		playerModule.Click_Spectate( action, m_Meta.target );
	}

	protected void DoUnconscious()
	{
		JMPlayerModule playerModule = CF_Modules<JMPlayerModule>.Get();
		JMPlayerInstance instance = TargetPlayer();

		if ( !playerModule || !instance )
			return;

		if ( instance.IsUnconscious() )
			playerModule.SetShock( SHOCK_CONSCIOUS, { instance.GetGUID() } );
		else
			playerModule.SetShock( 0, { instance.GetGUID() } );
	}

	protected void DoFreeze()
	{
		JMPlayerModule playerModule = CF_Modules<JMPlayerModule>.Get();
		JMPlayerInstance instance = TargetPlayer();

		if ( !playerModule || !instance )
			return;

		playerModule.SetFreeze( !instance.IsFrozen(), { instance.GetGUID() } );
	}

	protected void DoBrokenLegs()
	{
		JMPlayerModule playerModule = CF_Modules<JMPlayerModule>.Get();
		JMPlayerInstance instance = TargetPlayer();

		if ( !playerModule || !instance )
			return;

		playerModule.SetBrokenLegs( !instance.HasBrokenLegs(), { instance.GetGUID() } );
	}

#ifndef DAYZ_1_29
	protected void DoRagdoll()
	{
		JMPlayerModule playerModule = CF_Modules<JMPlayerModule>.Get();
		JMPlayerInstance instance = TargetPlayer();

		if ( !playerModule || !instance )
			return;

		playerModule.SetRagdoll( !instance.IsRagdoll(), { instance.GetGUID() } );
	}
#endif

	// =========================================================================
	//  Value tables
	// =========================================================================

	protected array<string> LiquidNames()
	{
		array<string> names = {};
		names.Insert( "#STR_COT_ESP_MODULE_MENU_LIQUID_NONE" );
		names.Insert( "#STR_COT_OBJECT_LIQUID_WATER" );
		names.Insert( "#STR_COT_OBJECT_LIQUID_FRESHWATER" );
		names.Insert( "#STR_COT_OBJECT_LIQUID_SALTWATER" );
		names.Insert( "#STR_COT_OBJECT_LIQUID_CLEANWATER" );
		names.Insert( "#STR_COT_OBJECT_LIQUID_SNOW" );
		names.Insert( "#STR_COT_OBJECT_LIQUID_VODKA" );
		names.Insert( "#STR_COT_OBJECT_LIQUID_BEER" );
		names.Insert( "#STR_COT_OBJECT_LIQUID_GASOLINE" );
		names.Insert( "#STR_COT_OBJECT_LIQUID_DIESEL" );
		names.Insert( "#STR_COT_OBJECT_LIQUID_DISINFECTANT" );
		names.Insert( "#STR_COT_OBJECT_LIQUID_SALINE" );
		return names;
	}

	protected array<int> LiquidValues()
	{
		array<int> values = {};
		values.Insert( LIQUID_NONE );
		values.Insert( LIQUID_WATER );
		values.Insert( LIQUID_FRESHWATER );
		values.Insert( LIQUID_SALTWATER );
		values.Insert( LIQUID_CLEANWATER );
		values.Insert( LIQUID_SNOW );
		values.Insert( LIQUID_VODKA );
		values.Insert( LIQUID_BEER );
		values.Insert( LIQUID_GASOLINE );
		values.Insert( LIQUID_DIESEL );
		values.Insert( LIQUID_DISINFECTANT );
		values.Insert( LIQUID_SALINE );
		return values;
	}

	protected array<string> FoodStageNames()
	{
		array<string> names = {};
		names.Insert( "#STR_COT_PLAYER_MODULE_INV_STAGE_RAW" );
		names.Insert( "#STR_COT_PLAYER_MODULE_INV_STAGE_BAKED" );
		names.Insert( "#STR_COT_PLAYER_MODULE_INV_STAGE_BOILED" );
		names.Insert( "#STR_COT_PLAYER_MODULE_INV_STAGE_DRIED" );
		names.Insert( "#STR_COT_PLAYER_MODULE_INV_STAGE_BURNED" );
		names.Insert( "#STR_COT_PLAYER_MODULE_INV_STAGE_ROTTEN" );
		return names;
	}

	protected array<int> FoodStageValues()
	{
		array<int> values = {};
		values.Insert( FoodStageType.RAW );
		values.Insert( FoodStageType.BAKED );
		values.Insert( FoodStageType.BOILED );
		values.Insert( FoodStageType.DRIED );
		values.Insert( FoodStageType.BURNED );
		values.Insert( FoodStageType.ROTTEN );
		return values;
	}
}
