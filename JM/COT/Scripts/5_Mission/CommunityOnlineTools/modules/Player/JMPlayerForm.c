//! #define scope in Enforce is per-file, NOT per compiled module - see COTModule.c.
#define COT_DEBUGLOGS

class JMPlayerForm: JMFormBase
{
	private autoptr array< JMPlayerRowWidget > m_PlayerList;

	private UIActionText m_PlayerListCount;
	private UIActionText m_PlayerListPeak;
	private UIActionText m_PlayerListSelected;
	private UIActionSearchBox m_PlayerListFilter;

	//! Slot count of the server, when the client knows it - a direct connect
	//! never went through the browser, so it has no host data to read.
	private int m_MaxPlayers;

	//! High-water mark of the roster. Static so it survives the form being
	//! rebuilt (permissions change, menu reopened) and keeps counting for the
	//! whole session rather than restarting at the current headcount.
	private static int s_PeakPlayerCount;

	private UIActionImageButtonToggle m_PlayerListSort;
	private UIActionImageButton m_PlayerListSelectAll;
	private UIActionImageButton m_PlayerListDeSelectAll;
	private UIActionImageButton m_PlayerListFilters;

	//! The advanced-filter dropdown, and the state its entries toggle. All on
	//! by default: an admin who has never touched this sees the whole roster.
	private UIActionContextMenu m_FilterMenu;
	private bool m_FilterShowDead     = true;
	private bool m_FilterShowUncon    = true;
	private bool m_FilterShowHurt     = true;
	private bool m_FilterShowSick     = true;
	private bool m_FilterShowAdmins   = true;
	private bool m_FilterShowCheaters = true;

	private UIActionImageButton m_PlayerPrefSave;
	private UIActionImageButton m_PlayerPrefLoad;

	private UIActionScroller m_PlayerListScroller;
	private Widget m_PlayerListRows;

	private UIActionScroller m_ActionListScroller;
	//! protected so modded-class fragments (EgoLand tab injection) can append
	//! new tab panels onto the scroller without a fragile widget-tree walk.
	protected Widget m_ActionsWrapper;

	//! The right-hand panel is a tab strip over a scroller, not one long
	//! scroller. The strip has to sit OUTSIDE the scroller or it scrolls away
	//! with the content, which is why player_form.layout splits panel_right
	//! into panel_right_tabs and panel_right_content.
	//! protected so modded-class fragments can call AddContent / SetTabs / SetSelection.
	protected UIActionTabs m_Tabs;
	private Widget m_TabGeneral;
	private Widget m_TabPosition;
	private Widget m_TabInventory;
	private Widget m_TabStatistics;
	private Widget m_TabActions;

	static const int TAB_GENERAL    = 0;
	static const int TAB_POSITION   = 1;
	static const int TAB_INVENTORY  = 2;
	static const int TAB_STATISTICS = 3;
	static const int TAB_ACTIONS    = 4;

	//! A root-level layout entry below zero is an EMPTY equipment slot rather
	//! than an item. Slot i is encoded as INV_GHOST_BASE - i, which leaves -1
	//! meaning what it always meant: no item.
	static const int INV_GHOST_BASE = -2;

	private UIActionFeedbackButton m_GUID;
	private UIActionFeedbackButton m_Name;
	private UIActionFeedbackButton m_Steam64ID;
	private UIActionButton m_SteamProfile;

	#ifdef GAMELABS
	private UIActionFeedbackButton m_CFToolsID;
	private UIActionButton m_CFProfile;
	#endif

	//! Read-only mirror of the four JMPlayerVariables flags worth seeing at a
	//! glance. The checkboxes that set them live on the Actions tab; these say
	//! what is on without making the admin go and look.
	private Widget m_FlagStrip;
	//! Right-click menu for the player list, and the player it was opened on.
	//! The GUID is captured at open time and every item acts on THAT player, so
	//! the menu can never run on a different one than the row under the pointer
	//! - the selection set can change underneath it while it is up.
	private UIActionContextMenu m_PlayerMenu;
	private string              m_PlayerMenuGUID;

	//! Scale prompt, lazily built the first time it is needed - see m_PlayerMenu.
	//! One shared slider serves all three routing modes; which target set the
	//! next confirm applies to is remembered here rather than baked into three
	//! separate prompt ids.
	private UIActionValuePrompt m_ScalePrompt;
	private int                 m_ScalePendingMode;
	static const int SCALE_MODE_MULTI  = 0;
	static const int SCALE_MODE_SINGLE = 1;
	static const int SCALE_MODE_SELF   = 2;

	private UIActionBadge m_BadgeGodMode;
	private UIActionBadge m_BadgeFrozen;
	private UIActionBadge m_BadgeInvisible;
	private UIActionBadge m_BadgeUnconscious;
	private UIActionText  m_IdentityRole;

	//! Jumps to the Role Manager with this player already selected. Reading the
	//! role here and having to go and FIND the same player in another window to
	//! change it was the one identity row with no way to act on what it said.
	private UIActionImageButton m_IdentityRoleEdit;

	private UIActionStagedIcon m_BadgeStatusDead;
	private UIActionStagedIcon m_BadgeStatusUnconscious;
	private UIActionStagedIcon m_BadgeStatusBrokenLeg;
	private UIActionStagedIcon m_BadgeStatusBleeding;
	private UIActionStagedIcon m_BadgeStatusSick;

	//! The whole Active Statuses card. Hidden outright while the player is in
	//! none of the five states - an empty card is a heading over a blank strip,
	//! which reads as a panel that failed to load rather than as "nothing wrong".
	private Widget m_StatusCardRoot;

	//! Right-click menu for one Active Statuses glyph, and the guid it was
	//! opened against. Separate from m_PlayerMenu: that one belongs to a list
	//! row and can be open over a different player than the card is showing.
	private UIActionContextMenu m_StatusMenu;
	private string m_StatusMenuGUID;

	//! protected, not private: DayZ-Expansion's `modded class JMPlayerForm`
	//! (DayZExpansion_AI, DayZExpansion_Hardline) re-sorts these widgets via
	//! GetLayoutRoot().SetSort() when injecting its faction/reputation rows.
	//! A modded class cannot touch a private member of the class it mods.
	protected UIActionImageButton m_ApplyStats;
	private UIActionSlider m_Health;
	private bool m_HealthUpdated;
	private UIActionSlider m_Blood;
	private bool m_BloodUpdated;
	private UIActionSlider m_Energy;
	private bool m_EnergyUpdated;
	private UIActionSlider m_Water;
	private bool m_WaterUpdated;
	private UIActionSlider m_Shock;
	private bool m_ShockUpdated;
	//! protected, not private: DayZ-Expansion's `modded class JMPlayerForm` uses
	//! m_Stamina.GetLayoutRoot().GetParent() as the anchor to inject its own rows.
	protected UIActionSlider m_Stamina;
	private bool m_StaminaUpdated;
	private UIActionSlider m_HeatComfort;
	private UIActionSlider m_HeatBuffer;
	private bool m_HeatBufferUpdated;
	private UIActionCheckbox m_BloodyHands;
	private UIActionCheckbox m_GodMode;
	private UIActionCheckbox m_Freeze;
	private UIActionCheckbox m_Invisibility;
	private UIActionCheckbox m_UnlimitedAmmo;
	private UIActionCheckbox m_AdminNVG;
	private UIActionCheckbox m_UnlimitedStamina;
	private UIActionCheckbox m_BrokenLegs;
	private UIActionCheckbox m_ReceiveDmgDealt;
	private UIActionCheckbox m_CannotBeTargetedByAI;
	private UIActionCheckbox m_RemoveCollision;

    private UIActionImageButton m_CopyRotationPlayer;
    private UIActionImageButton m_CopyPositionPlayer;
    private UIActionImageButton m_PastePositionPlayer;
	private UIActionButton m_TeleportToMe;
	private UIActionButton m_TeleportMeTo;

	private UIActionEditableText m_PositionX;
	private bool m_PositionXUpdated;
	private UIActionEditableText m_PositionY;
	private bool m_PositionYUpdated;
	private UIActionEditableText m_PositionZ;
	private bool m_PositionZUpdated;
	private UIActionImageButton m_PositionRefresh;
	private UIActionImageButton m_Position;

	//! protected, not private: DayZ-Expansion's `modded class JMPlayerForm`
	//! (DayZExpansion_AI, DayZExpansion_Hardline, DayZExpansion_PersonalStorage)
	//! does, inside its InitActionWidgetsStats override:
	//!
	//!     m_ApplyStats.GetLayoutRoot().SetSort(sort + 1);
	//!     m_RefreshStats.GetLayoutRoot().SetSort(sort + 2);
	//!
	//! COT dropped this button at some point and never kept the name, so all
	//! three of those addons have failed to compile against COT ever since.
	//!
	//! It is a real button sitting beside Apply, NOT an alias of the Position
	//! tab's refresh icon: SetSort reorders a widget among ITS OWN siblings, so
	//! pointing this at the position toolbar would have Expansion silently
	//! shuffling the coordinate icons instead. Both now live in the variables
	//! card's title bar rather than in the slider grid, which makes Expansion's
	//! SetSort a harmless no-op - the pair is already last in reading order -
	//! and leaves the member names it compiles against intact.
	protected UIActionImageButton m_RefreshStats;

	//! Recenters the Position tab map on the selected player. Panning is only
	//! useful if something puts the view back.
	private UIActionImageButton m_PositionRecenter;

	//! Vehicle Manager's map made generic (UIActionMap) - markers, hover and
	//! click routing are handled there instead of by hand here.
	private UIActionMap m_PositionMap;

	//! Whether the deferred first-open recentre has already been scheduled.
	private bool m_PositionMapCentered;

	//! Widths of the three controls in a history row, as FRACTIONS of the row.
	//!
	//! Never a fixed pixel size beside a fraction: a WrapSpacer wraps the moment
	//! its children total more than the row, and mixing the two units makes that
	//! edge impossible to reason about. These sum to less than 1.
	static const float HISTORY_GO_W  = 0.70;
	static const float HISTORY_BTN_W = 0.13;

	//! The last few places the selected player was moved away FROM, most recent
	//! first, one button per entry - a click sends them back to that row.
	//!
	//! The rows are REBUILT rather than updated: the list is at most
	//! JMTeleportHistory.MAX_ENTRIES long, and an undo several steps deep drops
	//! every row above the one it lands on in a single go.
	private UIActionCard m_TeleportHistoryCard;
	private UIActionImageButton m_TeleportHistoryClear;
	private UIActionImageButton m_TeleportHistoryRedo;
	private Widget m_TeleportHistoryHost;
	private Widget m_TeleportHistoryGrid;
	private ref array<ref UIActionButton> m_TeleportHistoryButtons;

	//! The two icon buttons beside each row, in the same order as the row
	//! buttons above - each array's index IS the step it acts on, because all
	//! three are filled in one pass.
	private ref array<ref UIActionImageButton> m_TeleportHistoryCopy;
	private ref array<ref UIActionImageButton> m_TeleportHistoryFocus;

	private UIActionText m_TeleportHistoryEmpty;

	//! What the panel above was last built for. RefreshTeleports runs ten times
	//! a second, and rebuilding five buttons on every one of those would churn
	//! widgets for nothing - and a rebuild under the cursor steals the press.
	private string m_TeleportHistoryGuid;
	private int    m_TeleportHistoryCount;

	//! Timestamp of the newest row the panel was built from.
	//!
	//! The count alone is not enough to notice a change: once five steps are
	//! recorded the list is capped, so a sixth teleport pushes one off the
	//! bottom and leaves the count at five.
	private int    m_TeleportHistoryTop;

	// -- Statistics tab ----------------------------------------------------
	private UIActionKeyValueList m_StatsSession;
	private UIActionKeyValueList m_StatsCombat;
	private UIActionProgressBar m_StatsHeadshotBar;
	private UIActionText m_StatsCombatNotice;

#ifdef DZ_Expansion_Core
	//! Third Statistics card, built only against an Expansion server. Its rows
	//! are whatever the server sent, so a build with only some of the Expansion
	//! mods loaded gets a shorter card rather than a card full of "-".
	private UIActionKeyValueList m_StatsExpansion;
	private Widget m_StatsExpansionRoot;
#endif
	private UIActionImageButton m_StatsRefreshButton;

	//! Last history the server pushed for the selected player.
	private ref JMPlayerStats m_PlayerStats;

	private JMAntiCheatModule m_AntiCheatModule;

	// -- Inventory tab -----------------------------------------------------
	//! Preview entities and the classname each was spawned from, indexed by
	//! ITEM index - the position in m_InventoryItems, not the position on
	//! screen. Keying by item means expanding or collapsing a container moves
	//! no entities at all.
	private ref array<EntityAI> m_InvCellEntities;
	private ref array<string>   m_InvCellTypes;

	//! The cells currently on screen, and the item index each one is showing.
	//! Parallel, both rebuilt whenever the tree opens or closes.
	private ref array<Widget> m_InvCells;
	private ref array<int>    m_InvCellItem;

	//! Item indices of the containers the admin has opened. A container is
	//! closed until asked for, so a full loadout opens as a dozen cells rather
	//! than eighty.
	private ref array<int> m_InvExpanded;

	//! Cells and bands are positioned by hand onto this panel. See
	//! LayoutInventoryLevel for why it is a bare canvas and not a spacer.
	private Widget m_InventoryCanvas;

	//! Columns the last layout pass fitted, and the width it fitted them into.
	//! A resize only has to rebuild when one of these actually moves.
	private int   m_InvColumns;
	private float m_InvLayoutWidth;

	//! Drawn size of one cell, in screen pixels, and that against the size the
	//! layout declares. Everything the layout pass computes is in screen pixels
	//! - see MeasureInventoryCell for why it cannot be in anything else.
	private float m_InvCellPixels;
	private float m_InvScale;

	private int m_InvSelectedIndex;
	private int m_InvHoveredIndex;

	//! Preview entities spawned during the current rebuild. The budget is per
	//! rebuild, not per level, so a deep tree cannot walk past it.
	private int m_InvPreviewsBuilt;
	private UIActionContextMenu m_InventoryMenu;
	private UIActionValuePrompt m_InventoryPrompt;

	//! The container "Delete all" was clicked on. The confirmation is answered
	//! later and the table can be rebuilt in the meantime, so the container is
	//! remembered by its network ID, not by its row.
	private int m_InvGroupPendingLow;
	private int m_InvGroupPendingHigh;
	private string m_InvGroupPendingName;

	// -- Inventory item preview --------------------------------------------
	//! Vanilla's own hover tooltip layout, driven by the same static
	//! InspectMenuNew.UpdateItemInfo the inventory screen uses.
	private Widget            m_InvTooltip;
	private ItemPreviewWidget m_InvTooltipPreview;

	//! Client-local stand-ins for the hovered and the inspected item. See
	//! SpawnLocalPreview for why these are copies and not the real entities.
	private EntityAI m_InvHoverEntity;
	private string   m_InvHoverType;
	private EntityAI m_InvInspectEntity;
	private bool     m_InvInspectOpen;
	//! Only ever says that the server stopped listing - the per-item count it
	//! used to carry is what the grid itself shows.
	private UIActionText m_InventoryNotice;
	private UIActionImageButton m_InventoryRefresh;
	private UIActionImageButton m_InventoryClearCargo;

	//! The survivor's equipment slots, in the order the vanilla inventory draws
	//! them, with the ghost icon each empty one shows. Read once out of config;
	//! see EnsureInventorySlotTable.
	private ref array<string> m_InvSlotNames;
	private ref array<string> m_InvSlotIcons;
	private bool m_InvSlotsBuilt;

	//! The listing the table was last built from. Row index maps straight into
	//! this, so the selected row IS the item handle on the client side.
	private ref array< ref JMPlayerInventoryItem > m_InventoryItems;

	//! Context-menu item ids. Strings rather than an enum because that is what
	//! UIActionContextMenu round-trips.
	//! Quick actions on the row the pointer is over. Ids, not indices: the menu
	//! is rebuilt per click and the freeze entry changes label with the player.
	//! Advanced list filters. Each one hides a category of player rather than
	//! selecting one: they start all-on, so the list is complete until an admin
	//! deliberately narrows it.
	static const string LIST_FILTER_DEAD     = "f_dead";
	static const string LIST_FILTER_UNCON    = "f_uncon";
	static const string LIST_FILTER_HURT     = "f_hurt";
	static const string LIST_FILTER_SICK     = "f_sick";
	static const string LIST_FILTER_ADMINS   = "f_admins";
	static const string LIST_FILTER_CHEATERS = "f_cheaters";

	static const string ROW_MENU_HEAL       = "heal";
	static const string ROW_MENU_TP_TO_ME   = "tp_to_me";
	static const string ROW_MENU_TP_ME_TO   = "tp_me_to";
	static const string ROW_MENU_TP_UNDO    = "tp_undo";
	static const string ROW_MENU_TP_REDO    = "tp_redo";
	static const string ROW_MENU_FOCUS_MAP  = "focus_map";
	static const string ROW_MENU_COPY_GUID  = "copy_guid";
	static const string ROW_MENU_COPY_STEAM = "copy_steam";
	static const string ROW_MENU_MESSAGE    = "message";
	static const string ROW_MENU_CLEARCARGO = "clear_cargo";
	static const string ROW_MENU_STRIP      = "strip";
	static const string ROW_MENU_KICK       = "kick";
	static const string ROW_MENU_BAN        = "ban";
	static const string ROW_MENU_FREEZE     = "freeze";
	static const string ROW_MENU_RAGDOLL    = "ragdoll";
	static const string ROW_MENU_SPECTATE   = "spectate";

	//! Status repairs. Listed on the row menu only while the player is actually
	//! in that state - an admin scanning a roster wants the fix for what is
	//! wrong with this one, not the full catalogue of what could be.
	static const string ROW_MENU_FIX_LEGS   = "fix_legs";
	static const string ROW_MENU_STOP_BLEED = "stop_bleed";
	static const string ROW_MENU_CURE       = "cure";
	static const string ROW_MENU_WAKE       = "wake";

	static const string INV_MENU_INSPECT = "inspect";
	static const string INV_MENU_TAKE   = "take";
	static const string INV_MENU_DELETE = "delete";

	//! Item-specific entries. Unlike the four above they are only listed when
	//! the row that was hit can actually take them - a "Set Liquid" greyed out
	//! on every rifle in the loadout would be noise on the common case, and
	//! these are properties of the item, not of the admin's permissions.
	//!
	//! Doubles as the prompt id the confirm callback branches on: one menu
	//! entry raises exactly one prompt, so a second set of ids would only be a
	//! second thing to keep in step.
	static const string INV_MENU_UNJAM       = "unjam";
	static const string INV_MENU_QUANTITY    = "quantity";
	static const string INV_MENU_HEALTH      = "health";
	static const string INV_MENU_TEMPERATURE = "temperature";
	static const string INV_MENU_STATE       = "state";
	static const string INV_MENU_LIQUID      = "liquid";

	//! Row the context menu was opened on, captured at open time: the delete
	//! confirmation is asynchronous and the selection can move under it.
	private int m_InventoryPendingRow;

	//! The item an open prompt is about, held as its network ID rather than as
	//! a row: answering a prompt takes as long as the admin wants, and the
	//! listing behind it can be rebuilt by any refresh in the meantime, which
	//! would leave a row index pointing at somebody else's boots.
	private int m_InvPromptNetLow;
	private int m_InvPromptNetHigh;

	//! Shock value the unconscious chip wakes a player with. The slider it
	//! shares its permission with runs 0..100, and full is the only value that
	//! is certain to clear the engine own consciousness threshold.
	static const float SHOCK_CONSCIOUS = 100;

	//! Identity row geometry, in the row's own fractions. The value column stops
	//! short of the right edge so the card's inner padding stays visible down the
	//! whole column, and the short value + its profile button split what is left
	//! of the row after the label.
	static const float IDENTITY_LABEL_WIDTH  = 0.22;
	static const float IDENTITY_VALUE_WIDTH  = 0.76;
	static const float IDENTITY_SHORT_WIDTH  = 0.40;
	static const float IDENTITY_BUTTON_LEFT  = 0.64;
	static const float IDENTITY_BUTTON_WIDTH = 0.34;

	//! The role row gives up its last few percent to the edit button beside it.
	static const float IDENTITY_ROLE_VALUE_WIDTH = 0.92;
	static const float IDENTITY_ROLE_EDIT_LEFT   = 0.93;

	//! The edit glyph is SHORTER than the row it sits in - every other identity
	//! value fills its row, so only this one has slack to distribute. Its
	//! position is a fraction of the parent, so the pixel gap above and below is
	//! converted here rather than hardcoded; left at the layout default of 0 the
	//! button hung two pixels above the text it belongs to.
	//! ( IDENTITY_ROW_HEIGHT 32 - HEADER_ACTION_PX 28 ) / 2, over the row height.
	//! Written out rather than computed: these are another class's static consts
	//! and a const initialiser is not the place to depend on their evaluation
	//! order. Revisit if either constant moves.
	static const float IDENTITY_ROLE_EDIT_TOP = 0.0625;

	//! Drawn height of the embedded map, in layout pixels.
	//! Cells that get a 3D preview. Every preview is a real client-local
	//! entity, so this is a frame-time and memory budget, not a display
	//! preference. Past it the cell is still there and still operable - it just
	//! shows the item's name instead of its model.
	static const int INV_MAX_PREVIEWS = 96;

	static const int INV_CELL_SIZE = 72;

	//! How far the cell tree is drawn before it is assumed to be malformed.
	//! Deeper than any real loadout: player > backpack > case > box > mag.
	static const int INV_MAX_DEPTH = 8;

	//! Geometry of a container's band: the inset its contents are drawn at, the
	//! strip its label sits in, and the breathing room after it.
	static const int INV_BAND_PAD   = 8;
	static const int INV_BAND_LABEL = 26;
	static const int INV_BAND_GAP   = 6;

	//! Header action size. Smaller than the cards' HEADER_ACTION_PX because the
	//! band strip is a band strip, not a card title bar - three 28px buttons
	//! would be taller than the row they label.
	static const int INV_BAND_ACTION_PX = 22;

	//! Glyph size inside one of those buttons.
	//!
	//! The pill's image is an EXACT 16px in UIActionImageButton.layout - it does
	//! NOT scale with the button - so a button shrunk below the card size keeps
	//! a full-size glyph and ends up as a solid disc with a picture jammed edge
	//! to edge. Kept near the layout's own ratio of 16 in 28.
	static const int INV_BAND_ICON_PX = 12;

	//! How a cell prints its quantity. See InventoryQuantityMode.
	static const int INV_QTY_HIDDEN = 0;
	static const int INV_QTY_COUNT  = 1;
	static const int INV_QTY_BAR    = 2;

	static const int MAP_HEIGHT = 220;

	//! Zoom the map returns to when it is recentered.
	static const float MAP_DEFAULT_SCALE = 0.15;

	//! Zoom the map takes when it is pointed at one recorded step. Closer than
	//! the default: the point of focusing a step is to see WHERE it is, and at
	//! the overview scale three positions in the same town are one dot.
	static const float MAP_FOCUS_SCALE = 0.06;

	static const int MAP_MARK_TARGET = 0xFFFFFF00;
	static const int MAP_MARK_SELF   = 0xFF00FF00;

	//! Whether the coordinates on screen came from the player's entity on this
	//! client rather than from the roster snapshot the server pushes.
	private bool m_PositionIsLive;

	//! Coordinate precision. The roster copy of a position is a snapshot sent a
	//! few times a second; when the entity itself is loaded here it can be read
	//! directly, which is both current and worth printing to millimetres.
	static const int COORD_DECIMALS_SYNCED = 2;
	static const int COORD_DECIMALS_LIVE   = 3;


	private UIActionButton m_RepairTransport;
	private UIActionButton m_SpectatePlayer;
	private UIActionButton m_HealPlayer;
	private UIActionButton m_StripPlayer;
	private UIActionButton m_DryPlayer;

	private UIActionButton m_VomitPlayer;
	private UIActionButton m_SetScalePlayer;

	// Disease Add/Remove controls (status-aware dropdown)
	private UIActionDropdown m_DiseaseAgent;
	private UIActionButton m_DiseaseAdd;
	private UIActionButton m_DiseaseRemove;
	private UIActionImageButton m_DiseaseClear;
	private ref array< int > m_DiseaseAgentIds;
	private string m_LastSelectedGuid;

	// Bleed-from-body-part controls
	private UIActionDropdown m_BleedingPart;
	private UIActionButton m_BleedApply;
	private UIActionImageButton m_BleedClear;

	private UIActionFeedbackButton m_CopyExpLoadout;

	private UIActionButton m_KillPlayer;
	private UIActionButton m_SendMessage;
	private UIActionButton m_KickPlayer;
	private UIActionButton m_BanPlayer;

	//! Roles the user has collapsed. Kept as names rather than as row indices
	//! because the row pool is reused - the row that carried a group before a
	//! refresh is probably carrying a different one after it.
	private ref TStringArray m_CollapsedRoles;

	//! Role -> the GUIDs listed under it as of the last rebuild, so a header's
	//! checkbox and count act on exactly the rows it is sitting above and not on
	//! members the filter took out.
	private ref map< string, ref TStringArray > m_RoleMembers;

	//! The one group currently folding or unfolding. Collapse is not a state
	//! change followed by a repaint: the rows have to still exist while they
	//! shrink, so the collapse is only committed to m_CollapsedRoles once the
	//! animation has run out.
	private ref array< JMPlayerRowWidget > m_AnimRows;
	private JMPlayerRowWidget m_AnimHeader;
	private string m_AnimRole;
	private bool   m_AnimExpanding;
	private float  m_AnimTime;

	//! protected, not private: DayZ-Expansion's `modded class JMPlayerForm` reads
	//! this to pull per-player faction/reputation for its injected rows.
	protected JMPlayerInstance m_SelectedInstance;

	//! protected, not private: DayZ-Expansion's `modded class JMPlayerForm` calls
	//! m_Module.SetExpansionFaction() / SetExpansionReputation() on it.
	protected JMPlayerModule m_Module;

	private int m_LastChangeTime;
	
	private autoptr TStringArray m_HeatBufferStates = {"", "+", "++", "+++"};
	private autoptr TStringArray m_PlayersPref1 = {};
	private autoptr TStringArray m_PlayersPref2 = {};

	// Default inject count - high enough to cross every disease's activation threshold
	const int DISEASE_INJECT_COUNT = 200;

	private bool m_AutoSelect = true;  //! Auto-select player when first shown

	void JMPlayerForm()
	{
		m_PlayerList = new array< JMPlayerRowWidget >;
		m_InventoryItems = new array< ref JMPlayerInventoryItem >;
		m_InventoryPendingRow = -1;

		m_InvSlotsBuilt   = false;
		m_InvCells        = new array<Widget>;
		m_InvCellItem     = new array<int>;
		m_InvCellEntities = new array<EntityAI>;
		m_InvCellTypes    = new array<string>;
		m_InvExpanded     = new array<int>;
		m_InvSelectedIndex = -1;
		m_InvHoveredIndex  = -1;
		m_CollapsedRoles = new TStringArray;
		m_TeleportHistoryButtons = new array<ref UIActionButton>;
		m_TeleportHistoryCopy    = new array<ref UIActionImageButton>;
		m_TeleportHistoryFocus   = new array<ref UIActionImageButton>;
		m_TeleportHistoryCount   = -1;
		m_RoleMembers = new map< string, ref TStringArray >;
		m_AnimRows = new array< JMPlayerRowWidget >;

		JMScriptInvokers.MENU_PLAYER_CHECKBOX.Insert( OnPlayer_Checked );
		JMScriptInvokers.MENU_PLAYER_BUTTON.Insert( OnPlayer_Button );
	}

	void ~JMPlayerForm()
	{
		if (!g_Game)
			return;

		JMScriptInvokers.MENU_PLAYER_CHECKBOX.Remove( OnPlayer_Checked );
		JMScriptInvokers.MENU_PLAYER_BUTTON.Remove( OnPlayer_Button );

		// The tooltip is parented to the workspace, not to this form, so it
		// outlives the form unless it is taken down by hand.
		if ( m_InvTooltip )
			m_InvTooltip.Unlink();

		DestroyInventoryCellEntities();
		DestroyLocalPreview( m_InvHoverEntity );
		DestroyLocalPreview( m_InvInspectEntity );
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnClientPermissionsUpdated()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnClientPermissionsUpdated");
		#endif

		super.OnClientPermissionsUpdated();

		UpdateUI();
	}

	override void OnInit()
	{
		InitWidgetsLeft();
		InitWidgetsRight();

		RefreshStats(true);  //! Show correct state on reinit
	}

	override void OnResize( float w, float h )
	{
		PinRightPanelGeometry( h );

		if ( m_PlayerListScroller ) m_PlayerListScroller.UpdateScroller();
		if ( m_ActionListScroller ) m_ActionListScroller.UpdateScroller();

		// The cell grid is positioned by hand, so a resize is the one thing it
		// cannot ride out - but only a resize that changes how many cells fit
		// per row is worth rebuilding for.
		if ( m_InventoryCanvas && m_InventoryItems && m_InventoryItems.Count() > 0 && m_InvCellPixels > 0 )
		{
			float width = InventoryCanvasWidth();

			// Column count for the cells, width for the bands: a band is drawn
			// to the full width, so it has to be rebuilt even when the same
			// number of cells still fits.
			if ( InventoryColumnsFor( width ) != m_InvColumns || Math.AbsFloat( width - m_InvLayoutWidth ) > 4 )
				RebuildInventoryTable( false );
		}
	}

	//! One roster counter: a Lucide glyph where the word label used to be, the
	//! number immediately after it, and the word itself demoted to the tooltip
	//! so the meaning is still one hover away.
	//!
	//! The number is drawn left, next to its glyph, rather than on the cell's
	//! right edge - three icon/number pairs read as three pairs that way, where
	//! right-aligned numbers read as a column that has drifted away from its
	//! labels. STAT_TEXT_OFFSET is the player list's own name indent, so the
	//! glyph-to-text gap up here matches the checkbox-to-name gap down there.
	private UIActionText CreateRosterStat( Widget parent, string icon, string tooltip, float width, bool valueLeftAligned = true )
	{
		UIActionText stat = UIActionManager.CreateText( parent, "" );
		if ( !stat )
			return NULL;

		stat.SetIcon( JMConstants.Lucide( icon ) );
		stat.SetValueLeftAligned( valueLeftAligned );
		stat.SetLabelOffset( JMPlayerRowWidget.TEXT_OFFSET_PLAYER );
		stat.SetTooltip( tooltip );
		stat.SetWidth( width );

		return stat;
	}

	private void InitWidgetsLeft()
	{
		m_LeftPanel = layoutRoot.FindAnyWidget( "panel_left" );

		Widget leftPanelGrid = UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "panel_left_top" ), 3, 1 );

		// -- Roster stats ---------------------------------------------------
		//  Four numbers instead of one: how many are on the server right now
		//  (against the slot count where the client knows it), the high-water
		//  mark for the session, how many rows survived the filter, and how many
		//  are ticked. A single count could not answer "is the list short because
		//  the server is empty, or because I am still filtering?".
		//
		//  All three on one line: the words "Online:"/"Peak:"/"Selected:" are what
		//  forced two rows, and each of them is a single noun a glyph can carry. The label becomes the icon, the value keeps the cell's right
		//  edge, and the row that frees up goes back to the list. Every one carries
		//  a tooltip, because an icon says nothing until it is hovered.
		//
		//  A WrapSpacer rather than a 1x4 grid, because the four values are not
		//  four equal numbers: online is the only one that carries a slot count
		//  with it ("127 / 128"), so an even quarter of the column clips it while
		//  the other three sit on empty space. The fractions below hand that space
		//  over, and have to sum to under 1.0 - a WrapSpacer wraps its last child
		//  at exactly 1.0.
		m_MaxPlayers = 0;
		GetServersResultRow hostData = g_Game.GetHostData();
		if ( hostData )
			m_MaxPlayers = hostData.m_MaxPlayers;

		//! Not three equal thirds. Online is the only one carrying a slot count
		//! ("61 / 128"), and an even third clipped it against the peak cell that
		//! followed. It takes the space the other two do not need, and those two
		//! right-align their value so they sit as a pair on the right edge
		//! instead of drifting into the room online just took.
		Widget countRowGrid = UIActionManager.CreateWrapSpacerCompact( leftPanelGrid );
		m_PlayerListCount 	= CreateRosterStat( countRowGrid, "users",        "#STR_COT_PLAYER_MODULE_TT_STAT_ONLINE",   0.46 );
		m_PlayerListPeak 	= CreateRosterStat( countRowGrid, "trending-up",  "#STR_COT_PLAYER_MODULE_TT_STAT_PEAK",     0.26, false );
		m_PlayerListSelected 	= CreateRosterStat( countRowGrid, "square-check", "#STR_COT_PLAYER_MODULE_TT_STAT_SELECTED", 0.26, false );

		// -- Toolbar --------------------------------------------------------
		//  Icon-only pills, all four on one line: save and load a selection,
		//  select all and deselect all are the same kind of verb over the same
		//  list, so they read as one strip rather than two stacked pairs - and
		//  the row that frees up carries the second stats line above.
		//  Each carries a tooltip, because an icon-only button says nothing until
		//  it is hovered.
		//
		//  GridSpacer rather than WrapSpacer: a WrapSpacer reserves margin and
		//  padding around every child, so four children at 0.24 each overflow and
		//  the fourth wraps onto its own line.
		Widget toolbarRowGrid = UIActionManager.CreateGridSpacer( leftPanelGrid, 1, 5 );
		m_PlayerPrefSave 	= UIActionManager.CreateIconButton( toolbarRowGrid, JMConstants.Lucide( "save" ), this, "OnClick_PlayerPrefSave" );
		m_PlayerPrefLoad 	= UIActionManager.CreateIconButton( toolbarRowGrid, JMConstants.Lucide( "folder-open" ), this, "OnClick_PlayerPrefLoad" );
		m_PlayerListSelectAll 	= UIActionManager.CreateIconButton( toolbarRowGrid, JMConstants.ICON_SELECT_ALL, this, "Event_SelectAllPlayerList" );
		m_PlayerListDeSelectAll = UIActionManager.CreateIconButton( toolbarRowGrid, JMConstants.ICON_DESELECT_ALL, this, "Event_DeSelectAllPlayerList" );
		m_PlayerListFilters     = UIActionManager.CreateIconButton( toolbarRowGrid, JMConstants.Lucide( "list-filter" ), this, "OnClick_PlayerListFilters" );

		if ( m_PlayerPrefSave )        m_PlayerPrefSave.SetTooltip( "#STR_COT_GENERIC_SAVE" );
		if ( m_PlayerPrefLoad )        m_PlayerPrefLoad.SetTooltip( "#STR_COT_GENERIC_LOAD" );
		if ( m_PlayerListSelectAll )   m_PlayerListSelectAll.SetTooltip( "#STR_COT_ESP_MODULE_ACTION_SELECT_ALL" );
		if ( m_PlayerListDeSelectAll ) m_PlayerListDeSelectAll.SetTooltip( "#STR_COT_ESP_MODULE_ACTION_DESELECT_ALL" );
		if ( m_PlayerListFilters )     m_PlayerListFilters.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_ADVANCED_FILTERS" );

		//! The grid sizes its row to the tallest child, so shrinking all four
		//! shrinks the row - and the list below grows by the difference.
		if ( m_PlayerListFilters )     m_PlayerListFilters.SetFixedHeight( HEADER_CONTROL_HEIGHT );
		if ( m_PlayerPrefSave )        m_PlayerPrefSave.SetFixedHeight( HEADER_CONTROL_HEIGHT );
		if ( m_PlayerPrefLoad )        m_PlayerPrefLoad.SetFixedHeight( HEADER_CONTROL_HEIGHT );
		if ( m_PlayerListSelectAll )   m_PlayerListSelectAll.SetFixedHeight( HEADER_CONTROL_HEIGHT );
		if ( m_PlayerListDeSelectAll ) m_PlayerListDeSelectAll.SetFixedHeight( HEADER_CONTROL_HEIGHT );

		// -- Filter row -----------------------------------------------------
		//  Sort direction and the filter share a line: both narrow the list down
		//  to what you are looking for, and the sort is one glyph wide. The
		//  compact spacer is the one with no margin or padding, so the two
		//  fractions here are the whole story - they still have to sum to under
		//  1.0, because a WrapSpacer wraps its last child at exactly 1.0.
		Widget filterPanelGrid = UIActionManager.CreateWrapSpacerCompact( leftPanelGrid );

		m_PlayerListSort = UIActionManager.CreateImageButtonToggle( filterPanelGrid, JMConstants.Lucide( "arrow-down-a-z" ), JMConstants.Lucide( "arrow-down-z-a" ), this, "Event_UpdatePlayerList", 0.13 );
		if ( m_PlayerListSort )
		{
			m_PlayerListSort.SetTooltip( "#STR_COT_PLAYER_MODULE_LEFT_SORT_TOOLTIP" );

			//! The toggle's layout is a 32px square - the toolbar pill height,
			//! not the search box's. The search box's visible chrome is 22px
			//! centred inside a 30px root, so both are pinned to 22 here: same
			//! drawn height, and with equal roots the spacer aligns their tops.
			m_PlayerListSort.SetFixedHeight( HEADER_CONTROL_HEIGHT );
		}

		m_PlayerListFilter = UIActionManager.CreateSearchBox( filterPanelGrid, this, "Event_UpdatePlayerList", "#STR_COT_GENERIC_SEARCH" );
		if ( m_PlayerListFilter )
		{
			m_PlayerListFilter.SetWidth( 0.85 );
			m_PlayerListFilter.SetFixedHeight( HEADER_CONTROL_HEIGHT );
		}

		m_PlayerListScroller 	= UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel_left_bottom" ) );
		m_PlayerListRows 		= UIActionManager.CreateActionRows( m_PlayerListScroller.GetContentWidget() );
		
		for ( int i = 0; i < 2; i++ )
		{
			GridSpacerWidget gsw;
			if ( !Class.CastTo( gsw, m_PlayerListRows.FindAnyWidget( "Content_Row_0" + i ) ) )
				continue;

			gsw.Show( false );

			for ( int j = 0; j < 100; j++ )
			{
				Widget prWidget = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/player_widget.layout", gsw );
				
				if ( !prWidget )
					continue;

				JMPlayerRowWidget prScript;
				prWidget.GetScript( prScript );

				if ( !prScript )
					continue;

				prScript.SetPlayer( "" );
				prScript.Menu = this;

				m_PlayerList.Insert( prScript );
			}
		}

		m_PlayerListScroller.UpdateScroller();
	}

	private void InitWidgetsRight()
	{
		m_RightPanel = layoutRoot.FindAnyWidget( "panel_right" );
		m_RightPanelDisable = layoutRoot.FindAnyWidget( "panel_right_disable" );
		m_RightTabStrip = layoutRoot.FindAnyWidget( "panel_right_tabs" );
		m_RightContent = layoutRoot.FindAnyWidget( "panel_right_content" );

		// Captions are one word each on purpose - UIActionTabs centres the label
		// across the whole button while the icon sits at a fixed x=10, so a wide
		// caption grows leftwards into its own icon.
		ref array<string> tabLabels = { "#STR_COT_PLAYER_MODULE_TAB_GENERAL", "#STR_COT_PLAYER_MODULE_TAB_POSITION", "#STR_COT_PLAYER_MODULE_TAB_INVENTORY", "#STR_COT_PLAYER_MODULE_TAB_STATISTICS", "#STR_COT_PLAYER_MODULE_TAB_ACTIONS" };
		ref array<string> tabIcons = { JMConstants.Lucide( "user" ), JMConstants.Lucide( "map-pin" ), JMConstants.Lucide( "backpack" ), JMConstants.Lucide( "chart-column" ), JMConstants.Lucide( "zap" ) };

		m_Tabs = UIActionManager.CreateTabs( m_RightTabStrip, tabLabels, tabIcons, this, "OnChange_Tab" );

		m_ActionListScroller = UIActionManager.CreateScroller( m_RightContent );

		// The tab panels are direct children of the scroller content, with no
		// grid in between: a GridSpacer reserves a row for a hidden child, so
		// four hidden tabs would pad the visible one down the page.
		m_ActionsWrapper = m_ActionListScroller.GetContentWidget();

		// Row counts are the number of direct children each builder adds, with
		// headroom. Over-provisioning is free under Size-To-Content-V; running
		// short silently clips the last rows with no error anywhere.
		m_TabGeneral    = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 8, 1 );
		m_TabPosition   = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 8, 1 );
		m_TabInventory  = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 6, 1 );
		m_TabStatistics = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 8, 1 );
		m_TabActions    = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 14, 1 );

		m_Tabs.AddContent( m_TabGeneral );
		m_Tabs.AddContent( m_TabPosition );
		m_Tabs.AddContent( m_TabInventory );
		m_Tabs.AddContent( m_TabStatistics );
		m_Tabs.AddContent( m_TabActions );

		InitTabState( 5 );

		// sendEvent = false: OnChange_Tab pokes at widgets no builder has run
		// for yet on the very first pass.
		m_Tabs.SetSelection( TAB_GENERAL, false );

		// Only the tab that is about to be on screen gets built. The other four
		// cost nothing until the user asks for them, which is most of what the
		// form used to spend on being opened. Everything that touches a widget
		// from a tab it does not own null-guards it - that convention is what
		// makes deferring safe, so keep it when adding to this form.
		BuildTabIfNeeded( TAB_GENERAL );

		// No geometry pin here: the true content height is only known once
		// JMWindowBase calls OnResize. Until then the layout's own 0.06 / 0.94
		// split holds, which is close enough to be invisible.
		m_ActionListScroller.UpdateScroller();
	}

	//! Build a tab's contents the first time it is asked for. The bookkeeping
	//! lives in JMFormBase.ShouldBuildTab(); only the cases are ours.
	//! protected virtual: a modded-class fragment that injects extra tabs can
	//! override this, call super.BuildTabIfNeeded() for the base cases and
	//! handle its own tab indices in the extra cases.
	protected void BuildTabIfNeeded( int tabIdx )
	{
		if ( !ShouldBuildTab( tabIdx ) )
			return;

		switch ( tabIdx )
		{
			case TAB_GENERAL:
				InitActionWidgetsIdentity( m_TabGeneral );
				InitActionWidgetsStats( m_TabGeneral );
				//! Last card on the tab. It is the one card here that is empty
				//! most of the time - a healthy player has no statuses - and a
				//! card that usually says nothing should not sit above the
				//! identity and the stats an admin opened the tab for.
				InitActionWidgetsStatuses( m_TabGeneral );
				break;

			case TAB_POSITION:
				InitActionWidgetsPosition( m_TabPosition );
				break;

			case TAB_INVENTORY:
				InitActionWidgetsInventory( m_TabInventory );
				break;

			case TAB_STATISTICS:
				InitActionWidgetsStatistics( m_TabStatistics );
				break;

			case TAB_ACTIONS:
				InitActionWidgetsFlags( m_TabActions );
				InitActionWidgetsDiseases( m_TabActions );
				InitActionWidgetsQuick( m_TabActions );
				break;
		}

		// A tab built after the form already has a selection has missed every
		// UpdateUI/RefreshStats pass so far, so it would come up blank and
		// unpermissioned. Catch it up before it is shown.
		if ( m_SelectedInstance )
		{
			UpdateUI();
			RefreshStats( true );
		}
	}

	//! Feeds JMFormBase.IsTabActive(), which is what the 10Hz refreshes gate on.
	//! UIActionTabs is a 5_Mission type and the base lives in 4_World, so the
	//! selection has to be handed up rather than read down.
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

		// Every one of these anchors its popup to the window root rather than to
		// the tab panel, so UIActionTabs' Show(false) never reaches them - an
		// open list would float over whichever tab came next. They are all
		// registered at creation, so the base walks them.
		CloseAllOverlays();

		HideInventoryTooltip();

		int sel = m_Tabs.GetSelection();

		BuildTabIfNeeded( sel );

		if ( sel == TAB_POSITION )
			OnEnterPositionTab();
		else if ( sel == TAB_INVENTORY )
			RequestInventory();
		else if ( sel == TAB_STATISTICS )
			RequestStatistics();
		else if ( sel == TAB_ACTIONS )
		{
			if ( m_LastSelectedGuid != "" )
			{
				m_Module.RequestDiseaseMask( m_LastSelectedGuid );
				m_Module.RequestBleedingState( m_LastSelectedGuid );
			}
		}

		//! Extension hook - called with the final selected index after all
		//! built-in tab logic has run. Modded-class fragments override this
		//! to react to extra tab indices (e.g. requesting data on tab entry)
		//! without duplicating or replacing the base switch above.
		OnTabChanged( sel );

		if ( m_ActionListScroller )
			m_ActionListScroller.UpdateScroller();
	}

	//! Override in a modded-class fragment to react to any tab index,
	//! including indices beyond the base form's five tabs.
	protected void OnTabChanged( int tabIdx )
	{
	}

	//! Identity card: a section header, a strip of read-only state badges, then
	//! one labelled row per identifier.
	//!
	//! The value of each identifier is the CAPTION of its copy button - both
	//! UpdateUI (SetButton) and the Click_Copy* handlers (GetButton) go through
	//! that, so the button is the value store as well as the affordance. Do not
	//! "clean this up" into a separate text widget without moving all six call
	//! sites with it.
	private Widget InitActionWidgetsIdentity( Widget actionsParent )
	{
		// header + 2 badge rows + name + guid + steam + role [+ cftools] + divider
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 10, 1 );

		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_IDENTITY" );
		Widget section0 = section0Card.GetContent();
		// Two rows of two, not one row of four. UIActionBadge reserves a fixed
		// 80px pill on the right of whatever cell it lands in, so four cells
		// across this panel leave about forty pixels for the label and every
		// flag name is cut to four characters. Two per row gives the label the
		// room it needs, and this tab has vertical space to spare.
		Widget flagRowTop = UIActionManager.CreatePanel( section0, 0x00000000, IDENTITY_ROW_HEIGHT );
		m_FlagStrip = UIActionManager.CreateGridSpacer( flagRowTop, 1, 2 );
		m_BadgeGodMode     = UIActionManager.CreateBadge( m_FlagStrip, "#STR_COT_PLAYER_MODULE_BADGE_GODMODE", "", JMTheme.SUCCESS_DIM );
		m_BadgeFrozen      = UIActionManager.CreateBadge( m_FlagStrip, "#STR_COT_PLAYER_MODULE_BADGE_FROZEN", "", JMTheme.SUCCESS_DIM );

		Widget flagRowBottom = UIActionManager.CreatePanel( section0, 0x00000000, IDENTITY_ROW_HEIGHT );
		Widget flagStripBottom = UIActionManager.CreateGridSpacer( flagRowBottom, 1, 2 );
		m_BadgeInvisible   = UIActionManager.CreateBadge( flagStripBottom, "#STR_COT_PLAYER_MODULE_BADGE_INVISIBLE", "", JMTheme.SUCCESS_DIM );
		m_BadgeUnconscious = UIActionManager.CreateBadge( flagStripBottom, "#STR_COT_PLAYER_MODULE_BADGE_UNCONSCIOUS", "", JMTheme.SUCCESS_DIM );

		// The four flags are not just a readout. An admin who has just read
		// "GODMODE ON" is usually about to turn it off, and making them cross to
		// the Actions tab for a checkbox that sets what they are already looking
		// at is the long way round. The chips toggle it in place; the checkboxes
		// stay, and both drive the same module call.
		MakeFlagBadgeClickable( m_BadgeGodMode,     "Click_BadgeGodMode",     "#STR_COT_PLAYER_MODULE_TT_GODMODE"   );
		MakeFlagBadgeClickable( m_BadgeFrozen,      "Click_BadgeFrozen",      "#STR_COT_PLAYER_MODULE_TT_FREEZE"    );
		MakeFlagBadgeClickable( m_BadgeInvisible,   "Click_BadgeInvisible",   "#STR_COT_PLAYER_MODULE_TT_INVISIBLE" );
		MakeFlagBadgeClickable( m_BadgeUnconscious, "Click_BadgeUnconscious", ""                                    );

		// One row = ONE fixed-height panel holding the label and the value
		// button, placed by fraction. NOT a nested grid per row: a grid row whose
		// height is content-driven, holding a panel whose height is also
		// content-driven, resolves circularly and the rows blow out to hundreds
		// of pixels each - which is what put the GUID row a full screen below the
		// Name row and gave the tab a scrollbar with nothing in it.
		Widget rowName = UIActionManager.CreatePanel( section0, 0x00000000, IDENTITY_ROW_HEIGHT );
		UIActionManager.CreateText( rowName, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_NAME", "" );
		m_Name = UIActionManager.CreateFeedbackButton( rowName, "", "#STR_COT_COPIED", JMConstants.ICON_CHECK_MARK, this, "Click_CopyPlayerName" );
		m_Name.SetWidth( IDENTITY_VALUE_WIDTH );
		m_Name.SetPosition( IDENTITY_LABEL_WIDTH );
		m_Name.SetIcon( JMConstants.ICON_STACK );
		m_Name.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_COPY_NAME" );

		Widget rowGuid = UIActionManager.CreatePanel( section0, 0x00000000, IDENTITY_ROW_HEIGHT );
		UIActionManager.CreateText( rowGuid, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_GUID", "" );
		m_GUID = UIActionManager.CreateFeedbackButton( rowGuid, "", "#STR_COT_COPIED", JMConstants.ICON_CHECK_MARK, this, "Click_CopyPlayerGUID" );
		m_GUID.SetWidth( IDENTITY_VALUE_WIDTH );
		m_GUID.SetPosition( IDENTITY_LABEL_WIDTH );
		m_GUID.SetIcon( JMConstants.ICON_STACK );
		m_GUID.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_COPY_GUID" );

		Widget rowSteam = UIActionManager.CreatePanel( section0, 0x00000000, IDENTITY_ROW_HEIGHT );
		UIActionManager.CreateText( rowSteam, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_STEAMID", "" );
		m_Steam64ID = UIActionManager.CreateFeedbackButton( rowSteam, "", "#STR_COT_COPIED", JMConstants.ICON_CHECK_MARK, this, "Click_CopyPlayerSteam64ID" );
		m_Steam64ID.SetWidth( IDENTITY_SHORT_WIDTH );
		m_Steam64ID.SetPosition( IDENTITY_LABEL_WIDTH );
		m_Steam64ID.SetIcon( JMConstants.ICON_STACK );
		m_Steam64ID.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_COPY_STEAMID" );
		m_SteamProfile = UIActionManager.CreateButton( rowSteam, "#STR_COT_PLAYER_MODULE_OPEN_PROFILE_WEB", this, "Click_OpenPlayerSteamProfile" );
		m_SteamProfile.SetWidth( IDENTITY_BUTTON_WIDTH );
		m_SteamProfile.SetPosition( IDENTITY_BUTTON_LEFT );
		m_SteamProfile.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_OPEN_STEAM_PROFILE" );

		// Which role bucket the player list filed this player under. It is the
		// one piece of identity the admin cannot read off the card otherwise,
		// and the General tab is exactly where "who is this" belongs.
		// A row, not a bare CreateText, because the edit button has to sit on
		// the same line as the value: the label/value pair stops short of the
		// right edge and the button takes what is left.
		Widget rowRole = UIActionManager.CreatePanel( section0, 0x00000000, IDENTITY_ROW_HEIGHT );
		m_IdentityRole = UIActionManager.CreateText( rowRole, "#STR_COT_PLAYER_MODULE_IDENTITY_ROLE", "" );
		m_IdentityRole.SetWidth( IDENTITY_ROLE_VALUE_WIDTH );

		m_IdentityRoleEdit = UIActionManager.CreateEditButton( rowRole, this, "Click_EditPlayerRoles", "#STR_COT_PLAYER_MODULE_TT_EDIT_ROLES" );
		m_IdentityRoleEdit.SetFixedSize( HEADER_ACTION_PX, HEADER_ACTION_PX );
		m_IdentityRoleEdit.SetPosition( IDENTITY_ROLE_EDIT_LEFT, IDENTITY_ROLE_EDIT_TOP );

		#ifdef GAMELABS
		Widget rowCF = UIActionManager.CreatePanel( section0, 0x00000000, IDENTITY_ROW_HEIGHT );
		UIActionManager.CreateText( rowCF, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_CFTOOLS", "" );
		m_CFToolsID = UIActionManager.CreateFeedbackButton( rowCF, "", "#STR_COT_COPIED", JMConstants.ICON_CHECK_MARK, this, "Click_CopyCFToolsID" );
		m_CFToolsID.SetWidth( IDENTITY_SHORT_WIDTH );
		m_CFToolsID.SetPosition( IDENTITY_LABEL_WIDTH );
		m_CFToolsID.SetIcon( JMConstants.ICON_STACK );
		m_CFProfile = UIActionManager.CreateButton( rowCF, "#STR_COT_PLAYER_MODULE_OPEN_PROFILE_WEB", this, "Click_OpenPlayerCFProfile" );
		m_CFProfile.SetWidth( IDENTITY_BUTTON_WIDTH );
		m_CFProfile.SetPosition( IDENTITY_BUTTON_LEFT );
		#endif

		ShowIdentityWidgets();

		return parent;
	}

	//! Glyphs, not chips. Each status is one bit of information - it is on or it
	//! is not - and a labelled pill spent a whole cell saying a word the icon
	//! says at a glance. The same glyphs the player list uses, so a row badge
	//! and this card read as the same vocabulary; the word each one stood for
	//! survives as its tooltip.
	//!
	//! Only the bad states are here. "Healthy" was a sixth glyph that meant
	//! "none of the other five", which is what an empty card already says, and
	//! it was the one entry with no action behind a right-click.
	private Widget InitActionWidgetsStatuses( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 2, 1 );

		UIActionCard statusCard = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_ACTIVE_STATUSES" );
		Widget statusContent = statusCard.GetContent();
		Widget statusRow = UIActionManager.CreatePanel( statusContent, 0x00000000, IDENTITY_ROW_HEIGHT );
		Widget statusGrid = UIActionManager.CreateGridSpacer( statusRow, 1, 5 );

		m_BadgeStatusDead        = CreateStatusIcon( statusGrid, "skull",          "#STR_COT_PLAYER_MODULE_STATUS_DEAD",        JMTheme.DANGER,  "" );
		m_BadgeStatusUnconscious = CreateStatusIcon( statusGrid, "bed",            "#STR_COT_PLAYER_MODULE_STATUS_UNCONSCIOUS", JMTheme.WARNING, "Click_StatusUnconscious" );
		m_BadgeStatusBrokenLeg   = CreateStatusIcon( statusGrid, "bone-fracture",  "#STR_COT_PLAYER_MODULE_STATUS_BROKEN_LEGS", JMTheme.WARNING, "Click_StatusBrokenLegs" );
		m_BadgeStatusBleeding    = CreateStatusIcon( statusGrid, "droplet",        "#STR_COT_PLAYER_MODULE_STATUS_BLEEDING",    JMTheme.DANGER,  "Click_StatusBleeding" );
		m_BadgeStatusSick        = CreateStatusIcon( statusGrid, "thermometer",    "#STR_COT_PLAYER_MODULE_STATUS_SICK",        JMTheme.DANGER,  "Click_StatusSick" );

		// Hidden until RefreshIdentityBadges finds a state to show. Built here
		// all the same: the card is cheap and rebuilding it per refresh would
		// drop the glyph the pointer is over mid right-click.
		m_StatusCardRoot = parent;
		m_StatusCardRoot.Show( false );

		return parent;
	}

	//! Repaint the identity badges from the synced flag bitmask. Called from
	//! RefreshStats, so it must stay cheap - four comparisons and at most four
	//! SetStatus calls.
	private void RefreshIdentityBadges()
	{
		if ( !m_SelectedInstance )
			return;

		if ( m_BadgeGodMode )
			SetFlagBadge( m_BadgeGodMode, m_SelectedInstance.HasGodMode() );

		if ( m_BadgeFrozen )
			SetFlagBadge( m_BadgeFrozen, m_SelectedInstance.IsFrozen() );

		if ( m_BadgeInvisible )
			SetFlagBadge( m_BadgeInvisible, m_SelectedInstance.HasInvisibility() );

		if ( m_BadgeUnconscious )
			SetFlagBadge( m_BadgeUnconscious, m_SelectedInstance.IsUnconscious() );

		bool isDead = m_SelectedInstance.IsDead();
		bool isUncon = m_SelectedInstance.IsUnconscious();
		bool hasBrokenLeg = m_SelectedInstance.HasBrokenLegs();
		bool isBleeding = m_SelectedInstance.IsBleeding();
		bool isSick = m_SelectedInstance.IsSick();

		if ( m_BadgeStatusDead )
			m_BadgeStatusDead.SetVisible( isDead );
		if ( m_BadgeStatusUnconscious )
			m_BadgeStatusUnconscious.SetVisible( isUncon && !isDead );
		if ( m_BadgeStatusBrokenLeg )
			m_BadgeStatusBrokenLeg.SetVisible( hasBrokenLeg && !isDead );
		if ( m_BadgeStatusBleeding )
			m_BadgeStatusBleeding.SetVisible( isBleeding && !isDead );
		if ( m_BadgeStatusSick )
			m_BadgeStatusSick.SetVisible( isSick && !isDead );

		// Death hides the other four, so it alone decides the card when it is
		// set - otherwise any of the remaining states keeps it on screen.
		bool anyStatus = isDead;
		if ( !isDead )
			anyStatus = isUncon || hasBrokenLeg || isBleeding || isSick;

		if ( m_StatusCardRoot && m_StatusCardRoot.IsVisible() != anyStatus )
		{
			m_StatusCardRoot.Show( anyStatus );

			// The card sits in the General tab scroller, so appearing or going
			// away changes the height of everything under it.
			if ( m_ActionListScroller )
				m_ActionListScroller.UpdateScroller();
		}
	}

	//! One status glyph for the Active Statuses card. A single stage: these do
	//! not cycle, they are shown or hidden by RefreshIdentityBadges.
	//!
	//! `callback` answers UIEvent.CLICK_RIGHT and opens the repair menu for that
	//! status. Pass "" for a status with nothing to undo - death is the only one.
	private UIActionStagedIcon CreateStatusIcon( Widget parent, string icon, string tooltip, int color, string callback )
	{
		UIActionStagedIcon status = UIActionManager.CreateStagedIcon( parent, this, callback );
		if ( !status )
			return NULL;

		status.AddStage( JMConstants.Lucide( icon ), color );

		//! Resolved here rather than handed to the tooltip as a key: the tooltip
		//! sizes its panel from the string's length, and a "#STR_..." key is not
		//! the length of the word it stands for.
		status.SetTooltip( Widget.TranslateString( tooltip ) );

		return status;
	}

	void Click_StatusUnconscious( UIEvent eid, UIActionBase action )
	{
		OpenStatusMenu( eid, action, ROW_MENU_WAKE );
	}

	void Click_StatusBrokenLegs( UIEvent eid, UIActionBase action )
	{
		OpenStatusMenu( eid, action, ROW_MENU_FIX_LEGS );
	}

	void Click_StatusBleeding( UIEvent eid, UIActionBase action )
	{
		OpenStatusMenu( eid, action, ROW_MENU_STOP_BLEED );
	}

	void Click_StatusSick( UIEvent eid, UIActionBase action )
	{
		OpenStatusMenu( eid, action, ROW_MENU_CURE );
	}

	//! Right-clicking a status glyph offers the one action that clears it. A
	//! one-entry menu on purpose: the glyph already named the problem, so the
	//! menu only has to name the fix and give it a deliberate second click.
	private void OpenStatusMenu( UIEvent eid, UIActionBase action, string repairId )
	{
		if ( eid != UIEvent.CLICK_RIGHT || !m_SelectedInstance || !m_Window )
			return;

		UIActionStagedIcon icon = UIActionStagedIcon.Cast( action );
		if ( !icon )
			return;

		if ( !m_StatusMenu )
		{
			m_StatusMenu = UIActionManager.CreateContextMenu( layoutRoot, m_Window.GetWidgetRoot(), this, "OnClick_StatusMenu" );
			RegisterOverlay( m_StatusMenu );

			if ( !m_StatusMenu )
				return;
		}

		m_StatusMenuGUID = m_SelectedInstance.GetGUID();

		m_StatusMenu.ClearItems();
		AddStatusRepairItem( m_StatusMenu, repairId );

		if ( m_StatusMenu.GetItemCount() == 0 )
			return;

		SetStatusRepairPermissions( m_StatusMenu );

		m_StatusMenu.ShowAt( icon.GetLastRightClickX(), icon.GetLastRightClickY() );
	}

	void OnClick_StatusMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_StatusMenu )
			return;

		RunStatusRepair( m_StatusMenu.GetLastClickedId(), m_StatusMenuGUID );
	}

	//! An active flag reads as a warning, not as a success: every one of these
	//! is a state the player should not normally be in.
	private void SetFlagBadge( UIActionBadge badge, bool active )
	{
		if ( active )
			badge.SetStatus( "#STR_COT_PLAYER_MODULE_BADGE_ON", JMTheme.DANGER );
		else
			badge.SetStatus( "#STR_COT_PLAYER_MODULE_BADGE_OFF", JMTheme.SUCCESS_DIM );
	}

	private void MakeFlagBadgeClickable( UIActionBadge badge, string callback, string tooltip )
	{
		if ( !badge )
			return;

		badge.SetCallback( this, callback );
		badge.SetClickable( true );

		if ( tooltip != "" )
			badge.SetTooltip( tooltip );
	}

	//! Flip one flag from its chip.
	//!
	//! The new value is the negation of what the SELECTED PLAYER has right now,
	//! not of what the chip is painted as: the paint is a frame of synced state
	//! and can lag a refresh behind, and a toggle that read its own label would
	//! send the value the player already has.
	//!
	//! The badge is repainted immediately rather than waiting for the next sync,
	//! so the click has an answer; RefreshIdentityBadges overwrites it with the
	//! truth a moment later, including when the server refused the change.
	void Click_BadgeGodMode( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_SelectedInstance )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Godmode" ) )
			return;

		bool godMode = !m_SelectedInstance.HasGodMode();

		UpdateLastChangeTime();

		m_Module.SetGodMode( godMode, JM_GetSelected().GetPlayersOrSelf() );

		if ( m_GodMode )
			m_GodMode.SetChecked( godMode );

		SetFlagBadge( m_BadgeGodMode, godMode );
	}

	void Click_BadgeFrozen( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_SelectedInstance )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Freeze" ) )
			return;

		bool frozen = !m_SelectedInstance.IsFrozen();

		UpdateLastChangeTime();

		m_Module.SetFreeze( frozen, JM_GetSelected().GetPlayersOrSelf() );

		if ( m_Freeze )
			m_Freeze.SetChecked( frozen );

		SetFlagBadge( m_BadgeFrozen, frozen );
	}

	void Click_BadgeInvisible( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_SelectedInstance )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Invisibility" ) )
			return;

		bool invisible = !m_SelectedInstance.HasInvisibility();

		UpdateLastChangeTime();

		m_Module.SetInvisible( invisible, JM_GetSelected().GetPlayersOrSelf() );

		if ( m_Invisibility )
			m_Invisibility.SetChecked( invisible );

		SetFlagBadge( m_BadgeInvisible, invisible );
	}

	//! There is no "set unconscious" call anywhere in the module, because
	//! consciousness IS shock: the engine drops a player as shock reaches zero
	//! and brings them round as it climbs back. So this chip is the shock slider
	//! driven to one end or the other, and it is gated on the shock permission
	//! for exactly that reason.
	void Click_BadgeUnconscious( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_SelectedInstance )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Shock" ) )
			return;

		bool wake = m_SelectedInstance.IsUnconscious();
		float shock = 0;

		if ( wake )
			shock = SHOCK_CONSCIOUS;

		UpdateLastChangeTime();

		m_Module.SetShock( shock, JM_GetSelected().GetPlayersOrSelf() );

		if ( m_Shock )
			m_Shock.SetCurrent( shock );

		SetFlagBadge( m_BadgeUnconscious, !wake );
	}

	private void ShowIdentityWidgets()
	{
		m_Name.Show();
		m_GUID.Show();
		m_Steam64ID.Show();
		m_SteamProfile.Show();

		#ifdef GAMELABS
		m_CFToolsID.Show();
		m_CFProfile.Show();
		#endif
	}

	private void HideIdentityWidgets()
	{
		// Nothing is selected, so there are no statuses to be active.
		if ( m_StatusCardRoot )
			m_StatusCardRoot.Show( false );

		m_Name.Hide();
		m_GUID.Hide();
		m_Steam64ID.Hide();
		m_SteamProfile.Hide();

		#ifdef GAMELABS
		m_CFToolsID.Hide();
		m_CFProfile.Hide();
		#endif
	}

	private Widget InitActionWidgetsPosition( Widget actionsParent )
	{
		// header + toolbar + vector + map header + map slot + teleport header
		// + teleport row + history header + history rows + divider
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 12, 1 );

		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_POSITION" );
		Widget section0 = section0Card.GetContent();

		// -- Card quick actions ---------------------------------------------
		//  Every verb that acts on the three coordinate fields lives in the
		//  card's own title bar - the same place the Inventory and Session
		//  cards put theirs. Refresh re-reads the player's live coordinates
		//  into the boxes, the clipboard trio copies or pastes them, and the
		//  map pin pushes whatever is typed back to the server. Left to right
		//  that reads reload, clipboard tools, apply.
		//
		//  Icon-only, so each keeps the label it lost as its tooltip, and the
		//  three clipboard buttons answer a click by flashing a check mark.
		m_PositionRefresh     = section0Card.AddRefreshButton( this, "Click_RefreshTeleports", "#STR_COT_PLAYER_MODULE_TT_REFRESH_COORDS" );
		m_CopyRotationPlayer  = section0Card.AddCardHeaderAction( JMConstants.Lucide( "rotate-3d" ), this, "Click_CopyPlayerRotation", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_ORIENTATION_COPY" );
		m_CopyPositionPlayer  = section0Card.AddCopyButton( this, "Click_CopyPlayerPostion", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_COPY" );
		m_PastePositionPlayer = section0Card.AddPasteButton( this, "Click_PastePlayerPostion", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_PASTE" );
		m_Position            = section0Card.AddCardHeaderAction( JMConstants.Lucide( "map-pin-check" ), this, "Click_SetPosition", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_TO_COORDINATES" );

		Widget positionActionsVec = UIActionManager.CreateGridSpacer( section0, 1, 3 );

		m_PositionX = UIActionManager.CreateEditableText( positionActionsVec, "X:", this, "Change_PositionX" );
		m_PositionY = UIActionManager.CreateEditableText( positionActionsVec, "Y:", this, "Change_PositionY" );
		m_PositionZ = UIActionManager.CreateEditableText( positionActionsVec, "Z:", this, "Change_PositionZ" );

		m_PositionX.SetOnlyNumbers( true );
		m_PositionY.SetOnlyNumbers( true );
		m_PositionZ.SetOnlyNumbers( true );

		UpdatePositionSourceHint();

		// -- Map ------------------------------------------------------------
		//  Built here rather than lazily into a reserved slot: every panel this
		//  form can reserve space with is a "Size To Content V" UIPanel, and a
		//  MapWidget reports no content height, so the slot collapses to
		//  nothing and the map is created inside a zero-height box - present,
		//  handled and invisible. Its own layout carries an exact height, so
		//  parenting it directly to the tab grid is both simpler and correct,
		//  and it lands between the two section headers instead of being
		//  appended to the end of the grid on first open.
		//
		//  It starts hidden and is only shown while the tab is up: a MapWidget
		//  left visible keeps rendering behind everything else.
		UIActionCard section1Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_MAP" );
		Widget section1 = section1Card.GetContent();

		// Recentre acts on the map as a whole, not on anything drawn inside it,
		// so it belongs in the map card's title bar rather than in a toolbar
		// row that would cost a line of height above the map itself.
		m_PositionRecenter = section1Card.AddCardHeaderAction( JMConstants.Lucide( "locate-fixed" ), this, "Click_RecenterMap" );
		m_PositionRecenter.SetFixedSize( HEADER_ACTION_PX, HEADER_ACTION_PX );
		m_PositionRecenter.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_RECENTER_MAP" );

		InitActionWidgetsMap( section1 );

		UIActionCard section2Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_TELEPORT" );
		Widget section2 = section2Card.GetContent();
		Widget teleportActions = UIActionManager.CreateGridSpacer( section2, 1, 2 );
		m_TeleportToMe   = UIActionManager.CreateButton( teleportActions, "#STR_COT_PLAYER_MODULE_TELEPORT_TO_ME", this, "Click_TeleportToMe"     );
		m_TeleportToMe.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_TELEPORT_TO_ME" );
		m_TeleportMeTo   = UIActionManager.CreateButton( teleportActions, "#STR_COT_PLAYER_MODULE_TELEPORT_ME_TO", this, "Click_TeleportMeTo"     );
		m_TeleportMeTo.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_TELEPORT_ME_TO" );

		// -- Teleport history ------------------------------------------------
		//  Where this player has been moved away from, newest first. Recorded on
		//  this client as the admin issues the moves - it is a record of what
		//  THIS session did, not a server-side audit trail, so it is empty for
		//  teleports another admin issued or ones from before this connection.
		m_TeleportHistoryCard = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_TELEPORT_HISTORY" );
		m_TeleportHistoryHost = m_TeleportHistoryCard.GetContent();

		//  Clearing is card-level - it wipes every row at once - so it sits in
		//  the title bar with the other card-level wipes and keeps the caption
		//  it lost as an icon-only button's tooltip.
		//  Redo is card-level too: it acts on the stack, not on any one row -
		//  the rows are the steps still ahead of the target, and a redone step
		//  is by definition not one of them.
		m_TeleportHistoryRedo = m_TeleportHistoryCard.AddCardHeaderAction( JMConstants.Lucide( "redo-2" ), this, "Click_RedoTeleport", "#STR_COT_TELEPORT_REDO" );
		m_TeleportHistoryRedo.SetFixedSize( HEADER_ACTION_PX, HEADER_ACTION_PX );

		m_TeleportHistoryClear = m_TeleportHistoryCard.AddDeleteButton( this, "Click_ClearTeleportHistory", "#STR_COT_TELEPORT_HISTORY_CLEAR" );
		m_TeleportHistoryClear.SetFixedSize( HEADER_ACTION_PX, HEADER_ACTION_PX );
		m_TeleportHistoryClear.SetColor( JMTheme.DANGER_FILL );

		RebuildTeleportHistory();

		return parent;
	}

	//! Rebuild the history rows for whoever is selected.
	//!
	//! Cheap enough to call outright - it is at most five buttons - but never
	//! from inside one of their own click handlers: the engine answers a
	//! vanished press target by recentring the cursor, so a rebuild caused by a
	//! click is deferred a tick.
	protected void RebuildTeleportHistory()
	{
		if ( !m_TeleportHistoryHost )
			return;

		//! Drop the script references BEFORE the widgets they wrap, so a button
		//! is never left alive holding a layoutRoot that has been unlinked.
		m_TeleportHistoryButtons.Clear();
		m_TeleportHistoryCopy.Clear();
		m_TeleportHistoryFocus.Clear();
		m_TeleportHistoryEmpty = NULL;

		if ( m_TeleportHistoryGrid )
		{
			m_TeleportHistoryGrid.Unlink();
			m_TeleportHistoryGrid = NULL;
		}

		array<ref JMTeleportHistoryEntry> entries = JMTeleportHistory.Entries( JMTeleportHistory.PlayerKey( m_LastSelectedGuid ) );

		int count = 0;

		if ( entries )
			count = entries.Count();

		m_TeleportHistoryGuid  = m_LastSelectedGuid;
		m_TeleportHistoryCount = count;
		m_TeleportHistoryTop   = 0;

		//! Greyed rather than hidden: a header action that comes and goes moves
		//! the two beside it every time the stack changes.
		if ( m_TeleportHistoryRedo )
		{
			if ( JMTeleportHistory.HasRedo( JMTeleportHistory.PlayerKey( m_LastSelectedGuid ) ) )
				m_TeleportHistoryRedo.Enable();
			else
				m_TeleportHistoryRedo.Disable();
		}

		if ( count > 0 )
			m_TeleportHistoryTop = entries[0].Time;

		if ( count == 0 )
		{
			m_TeleportHistoryGrid  = UIActionManager.CreateGridSpacer( m_TeleportHistoryHost, 1, 1 );
			m_TeleportHistoryEmpty = UIActionManager.CreateText( m_TeleportHistoryGrid, "#STR_COT_TELEPORT_HISTORY_EMPTY", "" );

			return;
		}

		m_TeleportHistoryGrid = UIActionManager.CreateGridSpacer( m_TeleportHistoryHost, count, 1 );

		for ( int i = 0; i < count; i++ )
		{
			//! A WrapSpacer packs left to right in creation order, so this IS
			//! the order on screen: the step, then copy, then focus.
			Widget rowSpacer = UIActionManager.CreateWrapSpacerCompact( m_TeleportHistoryGrid, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

			//! "1. 7521 305 8134  (12s)" - the step, where it was, how long ago.
			string label = string.Format( "%1. %2  (%3)", i + 1, entries[i].FormatPosition(), entries[i].FormatAge() );

			UIActionButton row = UIActionManager.CreateButton( rowSpacer, label, this, "OnClick_TeleportHistory" );
			row.SetWidth( HISTORY_GO_W );
			row.SetTooltip( "#STR_COT_TELEPORT_HISTORY_ROW_DESC" );

			//! Copies in the same "<x, y, z>" shape the coordinate box does, so
			//! it pastes straight back into that box or into an ESP paste.
			UIActionImageButton copy = UIActionManager.CreateIconButton( rowSpacer, JMConstants.Lucide( "copy" ), this, "OnClick_TeleportHistoryCopy" );
			copy.SetWidth( HISTORY_BTN_W );
			copy.SetTooltip( "#STR_COT_TELEPORT_HISTORY_COPY" );

			//! Shows where the step was WITHOUT moving anyone - the one way to
			//! tell three sets of coordinates apart is to look at them on a map.
			UIActionImageButton focus = UIActionManager.CreateIconButton( rowSpacer, JMConstants.Lucide( "locate-fixed" ), this, "OnClick_TeleportHistoryFocus" );
			focus.SetWidth( HISTORY_BTN_W );
			focus.SetTooltip( "#STR_COT_TELEPORT_HISTORY_FOCUS" );

			m_TeleportHistoryButtons.Insert( row );
			m_TeleportHistoryCopy.Insert( copy );
			m_TeleportHistoryFocus.Insert( focus );
		}
	}

	//! The entry a row's controls act on, or NULL if the panel has moved on
	//! since the button was built.
	protected JMTeleportHistoryEntry TeleportHistoryEntryAt( int index )
	{
		return JMTeleportHistory.EntryAt( JMTeleportHistory.PlayerKey( m_LastSelectedGuid ), index );
	}

	//! Rebuild only when there is something new to show.
	//!
	//! Called off the ten-a-second refresh, so it compares first: who is
	//! selected, how many steps are recorded, and when the newest of them was
	//! recorded. The last of those is what catches a push at the cap, where a
	//! new step drops one off the bottom and the count does not move.
	protected void SyncTeleportHistory()
	{
		if ( !m_TeleportHistoryHost )
			return;

		if ( m_LastSelectedGuid != m_TeleportHistoryGuid )
		{
			RebuildTeleportHistory();

			return;
		}

		string key = JMTeleportHistory.PlayerKey( m_LastSelectedGuid );

		if ( JMTeleportHistory.Count( key ) != m_TeleportHistoryCount )
		{
			RebuildTeleportHistory();

			return;
		}

		JMTeleportHistoryEntry newest = JMTeleportHistory.EntryAt( key, 0 );

		if ( newest && newest.Time != m_TeleportHistoryTop )
			RebuildTeleportHistory();
	}

	//! Send the selected player back to the place this row names.
	//!
	//! The rows ABOVE the one clicked go with it - they are places the player
	//! only passed through on the way here, and leaving them in would make the
	//! next undo walk forwards. See JMTeleportHistory.Pop.
	void OnClick_TeleportHistory( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionButton row;

		if ( !Class.CastTo( row, action ) )
			return;

		int index = m_TeleportHistoryButtons.Find( row );

		if ( index < 0 )
			return;

		if ( !JMTeleportHistory.UndoPlayer( m_LastSelectedGuid, index ) )
			return;

		//! Deferred: this rebuild destroys the very button the press landed on.
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Call( RebuildTeleportHistory );
	}

	void OnClick_TeleportHistoryCopy( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionImageButton button;

		if ( !Class.CastTo( button, action ) )
			return;

		int index = m_TeleportHistoryCopy.Find( button );

		if ( index < 0 )
			return;

		JMTeleportHistoryEntry entry = TeleportHistoryEntryAt( index );

		if ( !entry )
			return;

		//! Same shape as Click_CopyPlayerPostion, so the two are interchangeable
		//! wherever a position is pasted.
		g_Game.CopyToClipboard( "<" + entry.Position[0] + ", " + entry.Position[1] + ", " + entry.Position[2] + ">" );

		//! No toast: the button swaps to a check mark where the click happened.
		button.ShowFeedback();
	}

	void OnClick_TeleportHistoryFocus( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionImageButton button;

		if ( !Class.CastTo( button, action ) )
			return;

		int index = m_TeleportHistoryFocus.Find( button );

		if ( index < 0 )
			return;

		JMTeleportHistoryEntry entry = TeleportHistoryEntryAt( index );

		if ( !entry )
			return;

		FocusMap( entry.Position );
	}

	//! Point the position tab's map at a coordinate, closer in than the default
	//! so the step is placed rather than just centred.
	//!
	//! Split out of RecenterMap because the two differ only in what they centre
	//! on, and the map is a MapWidget either way - nothing is moved, nothing is
	//! sent, the view just changes.
	private void FocusMap( vector position )
	{
		if ( !m_PositionMap || position == vector.Zero )
			return;

		m_PositionMap.CenterOn( position, MAP_FOCUS_SCALE );
	}

	void Click_RedoTeleport( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !JMTeleportHistory.RedoPlayer( m_LastSelectedGuid, 0 ) )
			return;

		//! Deferred: a redo puts a row back on the panel this button lives in.
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Call( RebuildTeleportHistory );
	}

	void Click_ClearTeleportHistory( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMTeleportHistory.Clear( JMTeleportHistory.PlayerKey( m_LastSelectedGuid ) );

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Call( RebuildTeleportHistory );
	}

	//! Vitals and temperatures.
	//!
	//! Name, signature and return type are frozen: DayZ-Expansion modded forms
	//! override this exact shape, call super, then inject their own rows into
	//! m_Stamina.GetLayoutRoot().GetParent() - the slider grid below - and
	//! re-sort them against m_ApplyStats and m_RefreshStats. Which tab this is
	//! called with does not matter to that; the anchor is internal.
	private Widget InitActionWidgetsStats( Widget actionsParent )
	{
		// header + sliders + temp header + temps + apply + divider
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 8, 1 );

		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_HEADER" );
		Widget section0 = section0Card.GetContent();
		// 6 sliders in 5 rows of 2 - two spare cells, which is where Expansion
		// puts its faction / reputation rows plus their spacer panel.
		Widget actions = UIActionManager.CreateGridSpacer( section0, 5, 2 );

		m_Health = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_HEALTH", 0, 100, this, "Click_SetHealth" );
		m_Water = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_WATER", 0, PlayerConstants.SL_WATER_MAX, this, "Click_SetWater" );

		m_Shock = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_SHOCK", 0, 100, this, "Click_SetShock" );
		m_Energy = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_FOOD", 0, PlayerConstants.SL_ENERGY_MAX, this, "Click_SetEnergy" );

		m_Blood = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_BLOOD", 0, 5000, this, "Click_SetBlood" );
		m_Stamina = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_STAMINA", 0, CfgGameplayHandler.GetStaminaMax(), this, "Click_SetStamina" );

		UIActionCard section1Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_TEMPERATURES" );
		Widget section1 = section1Card.GetContent();
		Widget actionsTemps = UIActionManager.CreateGridSpacer( section1, 1, 2 );
		m_HeatComfort = UIActionManager.CreateSlider( actionsTemps, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_HEATCOMFORT", -0.75, 0.75, this );
		m_HeatComfort.SetFormat("");
		m_HeatComfort.Disable(); // You cant force the game, so this slider is READ ONLY
		m_HeatComfort.SetDisableAlpha(0.0);

		m_HeatBuffer = UIActionManager.CreateSlider( actionsTemps, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_HEATBUFFER", 0, 3, this, "Click_SetHeatBuffer" );
		m_HeatBuffer.SetStepValue(0.1);

		m_Health.SetSliderWidth(0.5);
		m_Shock.SetSliderWidth(0.5);
		m_Blood.SetSliderWidth(0.5);
		m_Energy.SetSliderWidth(0.5);
		m_Water.SetSliderWidth(0.5);
		m_Stamina.SetSliderWidth(0.5);
		m_HeatComfort.SetSliderWidth(0.5);
		m_HeatBuffer.SetSliderWidth(0.5);

		// Apply and Refresh act on the whole card - every slider in it at once -
		// so they sit in its title bar with the other card-level quick actions
		// instead of costing a full row of height under the last slider.
		// Icon-only, so both keep the caption they lost as their tooltip.
		//
		// Created on the VARIABLES card, not the temperatures one: that is the
		// panel the sliders belong to and the one an admin reads them off.
		m_RefreshStats = section0Card.AddRefreshButton( this, "Click_RefreshVitals", "#STR_COT_PLAYER_MODULE_TT_REFRESH_STATS" );
		m_ApplyStats   = section0Card.AddApplyButton( this, "Click_ApplyStats", "#STR_COT_PLAYER_MODULE_APPLY" );

		return parent;
	}

	//! The eleven state toggles. Split out of InitActionWidgetsStats so they can
	//! live on the Actions tab next to kick and ban, which is where an admin
	//! looks for them. Expansion never touched this grid, only the slider one.
	private Widget InitActionWidgetsFlags( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_FLAGS" );
		Widget section0 = section0Card.GetContent();
		// 11 checkboxes - 6 rows x 2 cols leaves 1 empty cell, instead of 5,2 which truncates.
		Widget actions2 = UIActionManager.CreateGridSpacer( section0, 6, 2 );

		m_RemoveCollision = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_REMOVE_COLLISION", this, "Click_RemoveCollision", false, 1, JMConstants.Lucide( "ghost" ) );
		m_RemoveCollision.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_REMOVE_COLLISION" );
		m_GodMode = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_GODMODE", this, "Click_GodMode", false, 1, JMConstants.Lucide( "crown" ) );
		m_GodMode.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_GODMODE" );

		m_Freeze = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_FREEZE", this, "Click_Freeze", false, 1, JMConstants.Lucide( "snowflake" ) );
		m_Freeze.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_FREEZE" );
		m_Invisibility = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_INVISIBLE", this, "Click_Invisible", false, 1, JMConstants.Lucide( "eye-off" ) );
		m_Invisibility.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_INVISIBLE" );

		m_BloodyHands = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_BLOODY_HANDS", this, "Click_BloodyHands", false, 1, JMConstants.Lucide( "droplet" ) );
		m_BloodyHands.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_BLOODY_HANDS" );
		m_CannotBeTargetedByAI = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_IGNORED_BY_AI", this, "Click_CannotBeTargetedByAI", false, 1, JMConstants.Lucide( "bot-off" ) );
		m_CannotBeTargetedByAI.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_IGNORED_BY_AI" );

		m_BrokenLegs = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_BROKEN_LEGS", this, "Click_SetBrokenLegs", false, 1, JMConstants.Lucide( "bone-fracture" ) );
		m_BrokenLegs.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_BROKEN_LEGS" );
		m_UnlimitedStamina = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_UNLIMITED_STAMINA", this, "Click_UnlimitedStamina", false, 1, JMConstants.Lucide( "zap" ) );
		m_UnlimitedStamina.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_UNLIMITED_STAMINA" );

		m_ReceiveDmgDealt = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_RECEIVE_DAMAGE_DEALT", this, "Click_SetReceiveDamageDealt", false, 1, JMConstants.Lucide( "swords" ) );
		m_ReceiveDmgDealt.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_RECEIVE_DAMAGE_DEALT" );
		m_UnlimitedAmmo = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_UNLIMITED_AMMO", this, "Click_UnlimitedAmmo", false, 1, JMConstants.Lucide( "infinity" ) );
		m_UnlimitedAmmo.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_UNLIMITED_AMMO" );

		m_AdminNVG = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_NVG", this, "Click_AdminNVG", false, 1, JMConstants.Lucide( "eye" ) );
		m_AdminNVG.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_NVG" );

		return parent;
	}

	private Widget InitActionWidgetsDiseases( Widget actionsParent )
	{
		// disease header + dropdown + buttons + bleed header + dropdown + apply
		// + divider
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 8, 1 );

		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_DISEASES_HEADER" );
		Widget section0 = section0Card.GetContent();
		// Static agent id list mirrors PluginTransmissionAgents eAgents bitmask
		m_DiseaseAgentIds = {
			eAgents.CHOLERA,
			eAgents.INFLUENZA,
			eAgents.SALMONELLA,
			eAgents.BRAIN,
			eAgents.FOOD_POISON,
			eAgents.CHEMICAL_POISON,
			eAgents.WOUND_AGENT,
			eAgents.NERVE_AGENT,
			eAgents.HEAVYMETAL
		};

		// Dropdown shows each disease with a green or red icon depending on whether the
		// currently selected player is infected. Click "Add" to inject, "Remove" to clear.
		m_DiseaseAgent = UIActionManager.CreateDropdown( section0, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_DISEASES_AGENT", m_Window.GetWidgetRoot(), this, "" );
		RegisterOverlay( m_DiseaseAgent );
		m_DiseaseAgent.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_DISEASE_AGENT" );

		// Default-state rows: all green (no data until selection pushes mask)
		RebuildDiseaseDropdown( 0, 0, 0, 0, 0, 0, 0, 0, 0 );

		// Add and Remove act on whichever agent the dropdown is showing, so they
		// stay in the body under it. Clear-all does not read the dropdown at all
		// - it wipes the card - so it moves up into the title bar with the other
		// card-level destructive actions, and keeps the caption it lost as the
		// tooltip an icon-only button needs.
		m_DiseaseClear = section0Card.AddDeleteButton( this, "Click_ClearAllDiseases" );
		m_DiseaseClear.SetFixedSize( HEADER_ACTION_PX, HEADER_ACTION_PX );
		m_DiseaseClear.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_DISEASE_CLEAR" );
		m_DiseaseClear.SetColor( JMTheme.DANGER_FILL );

		Widget actions = UIActionManager.CreateGridSpacer( section0, 1, 2 );
		m_DiseaseAdd = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_DISEASE_ADD", this, "Click_AddDisease" );
		m_DiseaseAdd.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_DISEASE_ADD" );
		m_DiseaseRemove = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_DISEASE_REMOVE", this, "Click_RemoveDisease" );
		m_DiseaseRemove.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_DISEASE_REMOVE" );

		// ---- Bleed-from-body-part ----
		UIActionCard section1Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_BLEEDING" );
		Widget section1 = section1Card.GetContent();

		// Stop-all does not read the part dropdown - it clears the whole card -
		// so it belongs in the title bar next to the other card-level wipes,
		// exactly where Diseases puts its own. Bound straight to the existing
		// Click_StopBleeding: "stop every bleed on this player" already had a
		// handler and a confirmation, it just had no button on this card.
		m_BleedClear = section1Card.AddDeleteButton( this, "Click_StopBleeding" );
		m_BleedClear.SetFixedSize( HEADER_ACTION_PX, HEADER_ACTION_PX );
		m_BleedClear.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_BLEED_CLEAR" );
		m_BleedClear.SetColor( JMTheme.DANGER_FILL );

		m_BleedingPart = UIActionManager.CreateDropdown( section1, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_BLEEDING_PART", m_Window.GetWidgetRoot(), this, "" );
		RegisterOverlay( m_BleedingPart );
		m_BleedingPart.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_BLEEDING_PART" );

		// Populate with zone list (default fallback entries until server push arrives)
		m_BleedingPart.AddEntry( BleedPartLabel( "All" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "Head" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "Torso" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "LeftArm" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "RightArm" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "LeftLeg" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "RightLeg" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "LeftFoot" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "RightFoot" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.SetSelection( 0, false );

		Widget bleedActions = UIActionManager.CreateGridSpacer( section1, 1, 1 );
		m_BleedApply = UIActionManager.CreateButton( bleedActions, "#STR_COT_PLAYER_MODULE_BLEED_APPLY", this, "Click_ApplyBleed" );
		m_BleedApply.SetWidth( 1.0 );
		m_BleedApply.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_BLEED_APPLY" );

		return parent;
	}

	private Widget InitActionWidgetsQuick( Widget actionsParent )
	{
		// header + friendly + gap + misc + gap + danger header + danger + divider
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 9, 1 );

		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_HEADER" );
		Widget section0 = section0Card.GetContent();
		// Friendly actions at the top - 3 buttons in one row (Stop Bleeding moved to per-part Bleed dropdown)
		Widget actionsFriendly = UIActionManager.CreateGridSpacer( section0, 1, 3 );
		m_HealPlayer = UIActionManager.CreateButton( actionsFriendly, "#STR_COT_PLAYER_MODULE_ACTION_HEAL",    this, "Click_HealPlayer"    );
		m_HealPlayer.SetWidth( 0.33 );
		m_HealPlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_HEAL" );
		m_RepairTransport = UIActionManager.CreateButton( actionsFriendly, "#STR_COT_PLAYER_MODULE_ACTION_REPAIR", this, "Click_RepairTransport" );
		m_RepairTransport.SetWidth( 0.33 );
		m_RepairTransport.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_REPAIR" );
		m_DryPlayer = UIActionManager.CreateButton( actionsFriendly, "#STR_COT_PLAYER_MODULE_ACTION_DRY",     this, "Click_DryPlayer"     );
		m_DryPlayer.SetWidth( 0.33 );
		m_DryPlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_DRY" );

		// Misc actions inbetween - 4 or 5 buttons; #ifdef adds a row if Expansion is loaded
		UIActionManager.CreatePanel( section0, 0x00000000, 10 );
	#ifdef DZ_Expansion_Core
		Widget actionsMisc = UIActionManager.CreateGridSpacer( section0, 3, 2 );
	#else
		Widget actionsMisc = UIActionManager.CreateGridSpacer( section0, 2, 2 );
	#endif
		m_SpectatePlayer = UIActionManager.CreateButton( actionsMisc, "#STR_COT_PLAYER_MODULE_ACTION_SPECTATE", this, "Click_SpectatePlayer" );
		m_SpectatePlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_SPECTATE" );
		m_SendMessage 	 = UIActionManager.CreateButton( actionsMisc, "#STR_COT_PLAYER_MODULE_ACTION_MESSAGE",  this, "Click_SendMessage"    );
		m_SendMessage.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_MESSAGE" );
		m_VomitPlayer 	 = UIActionManager.CreateButton( actionsMisc, "#STR_COT_PLAYER_MODULE_ACTION_VOMIT",  this, "Click_VomitPlayer"    );
		m_VomitPlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_VOMIT" );
		m_SetScalePlayer = UIActionManager.CreateButton( actionsMisc, "#STR_COT_PLAYER_MODULE_ACTION_SCALE",  this, "Click_ScalePlayer"    );
		m_SetScalePlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_SCALE" );
	#ifdef DZ_Expansion_Core
		m_CopyExpLoadout = UIActionManager.CreateFeedbackButton( actionsMisc, "Exp.Loadout", "#STR_COT_COPIED", JMConstants.ICON_CHECK_MARK, this, "Click_CopyExpLoadout" );
		// This one can fail, so the swap is fired from the callback instead.
		m_CopyExpLoadout.SetAutoFeedback( false );
	#endif

		// Destructive actions at the bottom - 4 buttons in 2 rows x 2 cols
		UIActionManager.CreatePanel( section0, 0x00000000, 10 );
		UIActionCard section1Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_DANGER" );
		Widget section1 = section1Card.GetContent();
		section1Card.SetRingColor( JMTheme.DANGER );
		Widget actionsDestructive = UIActionManager.CreateGridSpacer( section1, 2, 2 );
		m_KillPlayer  = UIActionManager.CreateButton( actionsDestructive, "#STR_COT_PLAYER_MODULE_ACTION_KILL",  this, "Click_KillPlayer"  );
		m_KillPlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_KILL" );
		m_StripPlayer = UIActionManager.CreateButton( actionsDestructive, "#STR_COT_PLAYER_MODULE_ACTION_STRIP", this, "Click_StripPlayer" );
		m_StripPlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_STRIP" );
		m_KickPlayer  = UIActionManager.CreateButton( actionsDestructive, "#STR_COT_PLAYER_MODULE_ACTION_KICK",  this, "Click_KickPlayer"  );
		m_KickPlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_KICK" );
		m_BanPlayer   = UIActionManager.CreateButton( actionsDestructive, "#STR_COT_PLAYER_MODULE_ACTION_BAN",   this, "Click_BanPlayer"   );
		m_BanPlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_BAN" );

		m_KillPlayer.SetColor(JMTheme.DANGER_FILL);
		m_StripPlayer.SetColor(JMTheme.DANGER_FILL);
		m_KickPlayer.SetColor(JMTheme.DANGER_FILL);
		m_BanPlayer.SetColor(JMTheme.DANGER_FILL);

		return parent;
	}

	//! Inventory tab: a flat table of everything the player is carrying, with a
	//! right-click menu per row.
	//!
	//! A table rather than an icon grid because DayZ has no 2D item icon - the
	//! only way to draw an item picture is an ItemPreviewWidget fed a real
	//! client-local entity, and spawning one per row for an 80-item loadout,
	//! rebuilt on every refresh, is a frame-time problem. Nesting is shown by
	//! indenting the name instead.
	private Widget InitActionWidgetsInventory( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 6, 1 );

		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_INVENTORY" );
		Widget section0 = section0Card.GetContent();

		// Both controls act on the whole listing, so they live in the card's
		// own title bar rather than in a toolbar row above the grid - a row of
		// their own cost a full line of height and pushed the items down.
		// Created refresh-first: the strip packs right, so the first child is
		// the leftmost and the destructive one ends up on the outside edge.
		m_InventoryRefresh = section0Card.AddRefreshButton( this, "Click_RefreshInventory", "#STR_COT_PLAYER_MODULE_TT_INV_REFRESH" );

		// ClearCargo has existed on the module since forever with no UI anywhere.
		// It belongs on this tab; Strip stays with the destructive actions.
		// Icon-only, so the caption it lost becomes its tooltip. Always targets
		// m_SelectedInstance - the player list's own checkbox selection is a
		// separate concept and has nothing to do with the inventory on screen.
		m_InventoryClearCargo = section0Card.AddDeleteButton( this, "Click_ClearCargoOnly", "#STR_COT_PLAYER_MODULE_TT_INV_CLEAR_CARGO" );
		m_InventoryClearCargo.SetColor( JMTheme.DANGER_FILL );


		// A wrap spacer, not a grid with a fixed column count: the form is
		// resizable, and a wrap spacer reflows the cells to whatever width the
		// tab currently has, the way the vanilla inventory does. It sizes to its
		// content vertically, so the tab's existing scroller handles overflow
		// with no extra work.
		m_InventoryCanvas = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/player_inventory_canvas.layout", section0 );

		// Hidden unless the server stopped short of the whole loadout - a count
		// of what is already on screen is not worth a permanent row. Created
		// after the canvas so the row it would occupy is below the grid and not
		// a gap above it.
		m_InventoryNotice = UIActionManager.CreateText( section0, "", "" );
		m_InventoryNotice.GetLayoutRoot().Show( false );

		// Anchored to the window root, not to this tab panel: a popup parented
		// into the panel would be clipped by the scroller and hidden with the
		// tab. OnChange_Tab closes it explicitly for the same reason.
		m_InventoryMenu = UIActionManager.CreateContextMenu( layoutRoot, m_Window.GetWidgetRoot(), this, "OnClick_InventoryMenu" );
		RegisterOverlay( m_InventoryMenu );

		// Same anchoring, same reason: the prompt the menu raises has to float
		// over the tab rather than scroll with it.
		m_InventoryPrompt = UIActionManager.CreateValuePrompt( layoutRoot, m_Window.GetWidgetRoot(), this, "OnConfirm_InventoryPrompt" );
		RegisterOverlay( m_InventoryPrompt );

		// Vanilla's inventory tooltip, verbatim. It is filled by the static
		// InspectMenuNew.UpdateItemInfo, so nothing here depends on ItemManager
		// or on the inventory menu being open - which it never is while COT has
		// the screen.
		//
		// Created at the TOP of the workspace with no section0, not inside the COT
		// window: as a child of the window it was clipped at the window's edge
		// and its position was relative to the window, so it drifted as soon as
		// the form was dragged. A tooltip follows the cursor across the whole
		// screen, so it has to be positioned in screen space.
		m_InvTooltip = g_Game.GetWorkspace().CreateWidgets( "gui/layouts/inventory_new/day_z_inventory_new_tooltip.layout", NULL );
		if ( m_InvTooltip )
		{
			m_InvTooltip.Show( false );
			m_InvTooltip.SetSort( 9998 );
			Class.CastTo( m_InvTooltipPreview, m_InvTooltip.FindAnyWidget( "ItemFrameWidget" ) );
		}

		return parent;
	}

	//! Ask the server for a fresh listing of the selected player.
	//!
	//! Only ever called on tab entry, on a selection change while the tab is up,
	//! from the refresh button, and after an operation - never on a timer. An
	//! unrequested listing per connected admin per second is real bandwidth for
	//! a panel that may not even be open.
	private void RequestInventory()
	{
		if ( !m_Module || !m_SelectedInstance )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.AccessInventory" ) )
			return;

		m_Module.RequestInventory( m_SelectedInstance.GetGUID() );
	}

	//! Server pushed a listing. Dropped when it is for someone else: the
	//! selection can move while the request is in flight.
	void OnInventoryUpdated( string guid, array< ref JMPlayerInventoryItem > items, bool truncated )
	{
		if ( !m_SelectedInstance || guid != m_SelectedInstance.GetGUID() )
			return;

		// Item indices only mean anything within one listing, so which
		// containers were open is carried across by NET ID instead - stable
		// across a refresh unless the tree itself changed, which is what a
		// routine edit (health, quantity, temperature, ...) never does. Losing
		// the expansion on every refresh made editing anything inside an open
		// bag or vest close it right back up.
		array<int> expandedLow  = {};
		array<int> expandedHigh = {};

		foreach ( int expandedIndex : m_InvExpanded )
		{
			if ( expandedIndex < 0 || expandedIndex >= m_InventoryItems.Count() )
				continue;

			expandedLow.Insert( m_InventoryItems[expandedIndex].NetIdLow );
			expandedHigh.Insert( m_InventoryItems[expandedIndex].NetIdHigh );
		}

		m_InventoryItems.Clear();
		foreach ( JMPlayerInventoryItem item : items )
			m_InventoryItems.Insert( item );

		m_InvExpanded.Clear();

		for ( int i = 0; i < m_InventoryItems.Count(); i++ )
		{
			int netLow  = m_InventoryItems[i].NetIdLow;
			int netHigh = m_InventoryItems[i].NetIdHigh;

			for ( int e = 0; e < expandedLow.Count(); e++ )
			{
				if ( expandedLow[e] == netLow && expandedHigh[e] == netHigh )
				{
					m_InvExpanded.Insert( i );
					break;
				}
			}
		}

		RebuildInventoryTable( truncated );
	}

	//! Rebuild the cell grid from the cached listing.
	//!
	//! Cells are recreated wholesale but the PREVIEW ENTITIES are reused: an
	//! entity is only respawned when what that index is showing changes. A
	//! refresh after a repair or a take otherwise means deleting and recreating
	//! up to ninety-six objects for a listing that barely moved.
	private void RebuildInventoryTable( bool truncated )
	{
		if ( !m_InventoryCanvas )
			return;

		HideInventoryTooltip();

		int count = m_InventoryItems.Count();

		// Drop the tail first, so the entities for items that no longer exist
		// are released before anything is rebuilt.
		while ( m_InvCellEntities.Count() > count )
		{
			int last = m_InvCellEntities.Count() - 1;
			DestroyLocalPreview( m_InvCellEntities[last] );
			m_InvCellEntities.Remove( last );
			m_InvCellTypes.Remove( last );
		}

		while ( m_InvCellEntities.Count() < count )
		{
			m_InvCellEntities.Insert( NULL );
			m_InvCellTypes.Insert( "" );
		}

		// Cells and bands are both plain children of the canvas, so clearing the
		// canvas is the whole teardown.
		Widget child = m_InventoryCanvas.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}

		m_InvCells.Clear();
		m_InvCellItem.Clear();
		m_InvSelectedIndex = -1;
		m_InvHoveredIndex  = -1;
		m_InvPreviewsBuilt = 0;

		// Measured, not assumed: the workspace scales what a layout declares,
		// so a cell drawn on screen is only 72 pixels wide at a UI scale of 1.
		m_InvCellPixels = MeasureInventoryCell();
		m_InvScale      = m_InvCellPixels / INV_CELL_SIZE;

		float width = InventoryCanvasWidth();
		m_InvLayoutWidth = width;
		m_InvColumns     = InventoryColumnsFor( width );

		float originX, originY;
		m_InventoryCanvas.GetScreenPos( originX, originY );

		float used = LayoutInventoryLevel( -1, originX, originY, width, 0 );

		// The canvas carries no spacer of its own, so its height is whatever the
		// layout pass just measured - and that is what gives the tab's scroller
		// something to scroll.
		float cw, ch;
		m_InventoryCanvas.GetScreenSize( cw, ch );
		m_InventoryCanvas.SetScreenSize( cw, Math.Max( used, m_InvCellPixels ) );

		m_InventoryCanvas.Update();

		DumpPreviewDiagnostics( m_InvPreviewsBuilt );

		if ( !m_InventoryNotice )
			return;

		if ( truncated )
			m_InventoryNotice.SetText( string.Format( Widget.TranslateString( "#STR_COT_PLAYER_MODULE_INV_TRUNCATED" ), m_InventoryItems.Count().ToString() ) );

		m_InventoryNotice.GetLayoutRoot().Show( truncated );
	}

	//! Drawn width of the cell area, in screen pixels.
	//!
	//! Falls back up the parent chain and then to a sane default: a listing can
	//! arrive before the tab has ever been laid out, and a width of zero would
	//! collapse the grid to a single column.
	private float InventoryCanvasWidth()
	{
		float w, h;
		m_InventoryCanvas.GetScreenSize( w, h );

		if ( w < m_InvCellPixels && m_InventoryCanvas.GetParent() )
			m_InventoryCanvas.GetParent().GetScreenSize( w, h );

		if ( w < m_InvCellPixels )
			w = m_InvCellPixels * 6;

		return w;
	}

	//! Drawn width of one cell, in screen pixels.
	//!
	//! The whole layout pass works in screen pixels and places everything with
	//! SetScreenPos / SetScreenSize. It has to: a layout declares its sizes in
	//! units the workspace then scales by the player's UI scale, and stepping
	//! the grid by the DECLARED 72 while the engine draws the cell at the SCALED
	//! width is exactly how the cells ended up overlapping each other. Screen
	//! pixels are the one space where the number that positions a cell and the
	//! number that sizes it mean the same thing.
	private float MeasureInventoryCell()
	{
		Widget probe = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/player_inventory_cell.layout", m_InventoryCanvas );
		if ( !probe )
			return INV_CELL_SIZE;

		probe.Update();

		float w, h;
		probe.GetScreenSize( w, h );

		probe.Unlink();

		if ( w < 1 )
			return INV_CELL_SIZE;

		return w;
	}

	private int InventoryColumnsFor( float width )
	{
		int columns = Math.Floor( width / m_InvCellPixels );

		if ( columns < 1 )
			columns = 1;

		return columns;
	}

	//! Say what the first preview cell actually is, so a blank grid can be told
	//! apart from a grid of previews drawn at zero size or with no entity
	//! behind them - the inventory preview is the other widget the engine
	//! renders in its own pass.
	private void DumpPreviewDiagnostics( int previews )
	{
		Print( "[COT-PREVIEW] rebuild: cells=" + m_InvCells.Count() + " previews=" + previews + " entities=" + m_InvCellEntities.Count() );

		if ( m_InvCells.Count() == 0 )
			return;

		ItemPreviewWidget first;
		if ( !Class.CastTo( first, m_InvCells[0].FindAnyWidget( "cell_preview" ) ) )
		{
			Print( "[COT-PREVIEW] first cell has no cell_preview widget" );
			return;
		}

		float px, py, pw, ph;
		first.GetScreenPos( px, py );
		first.GetScreenSize( pw, ph );

		// The item is printed by TYPE, not by reference: Enforce has no implicit
		// conversion from a class reference to string, so concatenating the
		// EntityAI itself does not compile.
		string itemState = "NULL";
		if ( first.GetItem() )
			itemState = first.GetItem().GetType();

		Print( "[COT-PREVIEW] first: visible=" + first.IsVisible().ToString() + " pos=" + px + "," + py + " size=" + pw + "x" + ph + " item=" + itemState );

		// The three states this has to tell apart, and what each looks like:
		//
		//   size 0x0 or item=NULL   the preview never had anything to draw -
		//                           geometry or the local entity, our bug.
		//   sane size, item set     the engine IS drawing it and something is
		//                           on top of it, or the model is out of frame.
		//   an ancestor invisible
		//   or alpha 0              the chain hides it, whatever the leaf says.
		//
		// So walk up and print the chain: a widget that reports itself visible
		// inside an invisible parent is the classic false positive here.
		Widget walk = first.GetParent();
		int depth = 0;

		while ( walk && depth < 8 )
		{
			Print( "[COT-PREVIEW]   parent[" + depth + "] " + walk.GetName() + " visible=" + walk.IsVisible().ToString() + " alpha=" + walk.GetAlpha() );

			walk = walk.GetParent();
			depth++;
		}
	}

	//! Does anything in the listing name this item as its parent?
	private bool HasInventoryChildren( int index )
	{
		foreach ( JMPlayerInventoryItem child : m_InventoryItems )
		{
			if ( child.ParentIndex == index )
				return true;
		}

		return false;
	}

	//! Place one level of the tree, and answer how tall it came out.
	//!
	//! Everything is positioned by hand onto one flat canvas rather than handed
	//! to a spacer. A band has to be exactly as tall as what it contains, and
	//! the items after it have to carry on underneath - which is the one thing a
	//! wrap spacer will not do: it sized the band against the whole panel, so
	//! one open container swallowed the rest of the list.
	//!
	//! Cells and bands are all siblings on the canvas; a band is simply the
	//! ground drawn under a run of cells, created BEFORE them so it stays
	//! behind. Only an opened container is descended into, so a listing that
	//! describes a cycle cannot be walked twice; the depth guard covers a
	//! ParentIndex chain that loops back on itself.
	private float LayoutInventoryLevel( int parentIndex, float originX, float originY, float width, int depth )
	{
		if ( depth > INV_MAX_DEPTH )
			return 0;

		int columns = InventoryColumnsFor( width );

		float y   = originY;
		int   col = 0;

		array<int> order = InventoryLevelOrder( parentIndex );

		// Two passes, not one. Interleaving a band with the cells - dropping it
		// in right under the container it belongs to - splits the grid in half
		// the moment anything in the first row is opened, and at the root that
		// grid is the equipment slot layout: an admin loses the shape of the
		// loadout as soon as they look inside a backpack. So every cell of this
		// level is placed first, and the open containers spill out underneath
		// the complete grid, in the order they appear in it.
		for ( int i = 0; i < order.Count(); i++ )
		{
			int entry = order[i];

			Widget cell;
			if ( entry >= 0 )
				cell = BuildInventoryCell( entry );
			else
				cell = BuildInventoryGhostCell( INV_GHOST_BASE - entry );

			if ( cell )
				cell.SetScreenPos( originX + ( col * m_InvCellPixels ), y );

			col++;
			if ( col >= columns )
			{
				col = 0;
				y   = y + m_InvCellPixels;
			}
		}

		// Close the last, partly filled row before anything goes below it.
		if ( col > 0 )
			y = y + m_InvCellPixels;

		for ( int b = 0; b < order.Count(); b++ )
		{
			int open = order[b];

			// An empty slot holds nothing, so there is nothing to open.
			if ( open < 0 )
				continue;

			if ( m_InvExpanded.Find( open ) < 0 )
				continue;

			if ( !HasInventoryChildren( open ) )
				continue;

			y = y + LayoutInventoryBand( open, originX, y, width, depth );
		}

		return y - originY;
	}

	//! What one level draws, in the order it draws it.
	//!
	//! Below the root that is simply listing order: the server walked the tree
	//! PREORDER, so a container's contents already arrive in the order it holds
	//! them.
	//!
	//! The ROOT is different. An admin reads a loadout by SLOT, and an EMPTY
	//! slot is information - "no vest", "no backpack" - that a list of whatever
	//! happens to be worn cannot express at all. So the root is the survivor's
	//! full equipment list in the vanilla order, with the worn item dropped into
	//! each slot and the slot's own ghost icon where there is nothing. Anything
	//! the slot table does not account for - the item in hands, a slot added by
	//! a mod - follows after it rather than being dropped.
	private array<int> InventoryLevelOrder( int parentIndex )
	{
		array<int> order = new array<int>;

		if ( parentIndex != -1 )
		{
			for ( int c = 0; c < m_InventoryItems.Count(); c++ )
			{
				if ( m_InventoryItems[c].ParentIndex == parentIndex )
					order.Insert( c );
			}

			return order;
		}

		EnsureInventorySlotTable();

		// Config gave us nothing to lay the root out against - fall back to the
		// flat listing rather than drawing an empty tab.
		if ( m_InvSlotNames.Count() == 0 )
		{
			for ( int f = 0; f < m_InventoryItems.Count(); f++ )
			{
				if ( m_InventoryItems[f].ParentIndex == -1 )
					order.Insert( f );
			}

			return order;
		}

		array<int> placed = new array<int>;

		for ( int s = 0; s < m_InvSlotNames.Count(); s++ )
		{
			int worn = InventoryItemInSlot( m_InvSlotNames[s] );

			if ( worn < 0 )
				order.Insert( INV_GHOST_BASE - s );

			if ( worn < 0 )
				continue;

			order.Insert( worn );
			placed.Insert( worn );
		}

		for ( int r = 0; r < m_InventoryItems.Count(); r++ )
		{
			if ( m_InventoryItems[r].ParentIndex != -1 )
				continue;

			if ( placed.Find( r ) >= 0 )
				continue;

			order.Insert( r );
		}

		return order;
	}

	//! Index of the root-level item worn in this slot, or -1.
	//!
	//! Matched on slot ID rather than on the strings: the listing's slot name
	//! comes back out of InventorySlots and the table's comes out of CfgSlots,
	//! and resolving both to the id the engine assigned is the one comparison
	//! that cannot be tripped by a difference in case.
	private int InventoryItemInSlot( string slotName )
	{
		int wanted = InventorySlots.GetSlotIdFromString( slotName );

		for ( int i = 0; i < m_InventoryItems.Count(); i++ )
		{
			if ( m_InventoryItems[i].ParentIndex != -1 )
				continue;

			// What is in the hands is not an attachment and carries no slot
			// name, but Hands is a slot on the survivor and this is what is in
			// it. Nothing else at the root can be slotless.
			if ( slotName == "Hands" && m_InventoryItems[i].SlotName == "" )
				return i;

			if ( m_InventoryItems[i].SlotName == "" )
				continue;

			if ( InventorySlots.GetSlotIdFromString( m_InventoryItems[i].SlotName ) == wanted )
				return i;
		}

		return -1;
	}

	//! Read the survivor's equipment slots out of config, once.
	//!
	//! playerSlots is the same list, in the same order, that the vanilla
	//! inventory draws down its left edge - so a loadout looks here the way it
	//! looks in the game's own screen. Reading it rather than hardcoding it also
	//! means a mod that adds a slot to the survivor gets one here for free.
	private void EnsureInventorySlotTable()
	{
		if ( m_InvSlotsBuilt )
			return;

		m_InvSlotsBuilt = true;
		m_InvSlotNames  = new array<string>;
		m_InvSlotIcons  = new array<string>;

		array<string> slotClasses = new array<string>;
		g_Game.ConfigGetTextArray( "CfgVehicles SurvivorBase InventoryEquipment playerSlots", slotClasses );

		for ( int i = 0; i < slotClasses.Count(); i++ )
		{
			string path = "CfgSlots " + slotClasses[i];

			if ( !g_Game.ConfigIsExisting( path ) )
				continue;

			string name = "";
			g_Game.ConfigGetText( path + " name", name );

			if ( name == "" )
				continue;

			// Already in the "set:<set> image:<image>" form an ImageWidget
			// takes. A slot with no ghost icon still gets a cell - an empty
			// tile in the right place still says the slot is empty.
			string icon = "";
			g_Game.ConfigGetText( path + " ghostIcon", icon );

			m_InvSlotNames.Insert( name );
			m_InvSlotIcons.Insert( icon );
		}
	}

	//! An equipment slot with nothing in it: the slot's own ghost glyph on a
	//! sunken tile.
	//!
	//! It is inserted with an item index of -1, which every handler that
	//! resolves a widget back through InventoryCellIndexOf already treats as
	//! "not a cell" - so an empty slot cannot be selected, opened, hovered for
	//! a tooltip, or acted on by the context menu without a single extra guard.
	private Widget BuildInventoryGhostCell( int slotIndex )
	{
		Widget cell = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/player_inventory_cell.layout", m_InventoryCanvas );
		if ( !cell )
			return NULL;

		m_InvCells.Insert( cell );
		m_InvCellItem.Insert( -1 );

		Widget fill = cell.FindAnyWidget( "cell_fill" );
		if ( fill )
			fill.SetColor( JMTheme.SURFACE_SUNKEN );

		Widget ring = cell.FindAnyWidget( "cell_ring" );
		if ( ring )
			ring.SetColor( JMTheme.BORDER_SUBTLE );

		// The health dot is on by default in the layout, and nothing is worn
		// here to have any.
		HideInventoryCellPart( cell, "cell_health_ring" );
		HideInventoryCellPart( cell, "cell_health_dot" );

		if ( slotIndex < 0 || slotIndex >= m_InvSlotIcons.Count() )
			return cell;

		ImageWidget ghost;
		if ( !Class.CastTo( ghost, cell.FindAnyWidget( "cell_ghost" ) ) )
			return cell;

		if ( m_InvSlotIcons[slotIndex] == "" )
			return cell;

		ghost.LoadImageFile( 0, m_InvSlotIcons[slotIndex] );
		ghost.Show( true );

		return cell;
	}

	private void HideInventoryCellPart( Widget cell, string name )
	{
		Widget part = cell.FindAnyWidget( name );
		if ( part )
			part.Show( false );
	}

	//! The ground under an open container: everything that item is holding, on
	//! its own tinted panel, directly below the cell it came out of. Answers the
	//! height it used, the gap after it included.
	private float LayoutInventoryBand( int index, float x, float y, float width, int depth )
	{
		// Created before its contents, because widgets draw in creation order
		// and this one is the backdrop.
		Widget band = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/player_inventory_group.layout", m_InventoryCanvas );
		if ( !band )
			return 0;

		// The ground itself is just DARKER than the grid - a tinted panel
		// behind eighty cells is noise. What the band is holding its contents
		// by is said once, in its own header: a paperclip for attachments, a
		// box for cargo, in the colour that separates the two.
		bool attached = InventoryGroupIsAttachments( index );

		int accent = JMTheme.ACCENT;
		if ( attached )
			accent = JMTheme.ACCENT_TEAL;

		Widget stripe = band.FindAnyWidget( "group_accent" );
		if ( stripe )
			stripe.SetColor( accent );

		ImageWidget icon;
		if ( Class.CastTo( icon, band.FindAnyWidget( "group_icon" ) ) )
		{
			if ( attached )
				icon.LoadImageFile( 0, JMConstants.Lucide( "paperclip" ) );
			else
				icon.LoadImageFile( 0, JMConstants.Lucide( "package" ) );

			// The stripe already carries the attached / cargo colour; a tinted
			// glyph on top of it just made the header harder to read.
			icon.SetColor( JMTheme.TEXT_PRIMARY );
		}

		TextWidget label;
		if ( Class.CastTo( label, band.FindAnyWidget( "group_label" ) ) )
			label.SetText( InventoryGroupLabel( index ) );

		BuildInventoryBandActions( band, index );

		float pad   = INV_BAND_PAD * m_InvScale;
		float strip = INV_BAND_LABEL * m_InvScale;

		float innerX     = x + pad;
		float innerY     = y + strip + pad;
		float innerWidth = width - ( pad * 2 );

		float contentHeight = LayoutInventoryLevel( index, innerX, innerY, innerWidth, depth + 1 );

		float bandHeight = strip + ( pad * 2 ) + contentHeight;

		band.SetScreenPos( x, y );
		band.SetScreenSize( width, bandHeight );

		return bandHeight + ( INV_BAND_GAP * m_InvScale );
	}

	//! The three whole-band operations, in the band's own header strip. They act
	//! on what the band DRAWS - everything this container holds directly - which
	//! is the scope an admin looking at an open backpack means by "all of it",
	//! and the reason they are here rather than on the container's own cell.
	//!
	//! Real UIActionImageButtons, the same ones the card headers use, so they
	//! carry the hover fill, the border and the tooltip every other icon button
	//! in COT has. They were raw ImageWidgets routed through OnMouseButtonUp,
	//! which drew the right glyph and answered a click but was dead on hover.
	//!
	//! Created per rebuild rather than cached: the band and everything on it is
	//! destroyed with the canvas on every refresh, so a cached button would be a
	//! dangling widget by the next listing.
	//!
	//! An action the admin lacks the permission for is not created at all.
	//! Hidden rather than greyed, unlike the context menu: the menu keeps one
	//! shape so a missing entry reads as a missing PERMISSION, but a band header
	//! is three unlabelled glyphs over someone else's backpack, and a dead one
	//! there just reads as broken.
	private void BuildInventoryBandActions( Widget band, int index )
	{
		//! Packed right and centred in the strip. The spacer lays a right-aligned
		//! run out from the right edge INWARDS, so the FIRST button created is
		//! the rightmost one - delete is declared first here to land on the
		//! outside edge, away from the two harmless actions, which is where
		//! every other destructive control in COT sits.
		WrapSpacerWidget host;
		if ( !Class.CastTo( host, band.FindAnyWidget( "group_actions" ) ) )
			return;

		host.SetContentAlignmentH( WidgetAlignment.WA_RIGHT );
		host.SetContentAlignmentV( WidgetAlignment.WA_CENTER );

		if ( GetPermissionsManager().HasPermission( "Admin.Player.Inventory.Delete" ) )
		{
			UIActionImageButton wipe = UIActionManager.CreateDeleteButton( host, this, "Click_BandDeleteAll", "#STR_COT_PLAYER_MODULE_TT_INV_GROUP_DELETE" );

			//! CreateDeleteButton paints the pill in the bright DANGER accent,
			//! which is the colour COT uses for TEXT and rings, not for a filled
			//! control. Every other delete button in the mod overrides it to the
			//! deeper DANGER_FILL right after creating it - see the Inventory
			//! card's own clear-cargo button - so this one does too.
			wipe.SetColor( JMTheme.DANGER_FILL );

			PrepareInventoryBandAction( wipe, index, "" );
		}

		if ( GetPermissionsManager().HasPermission( "Admin.Player.Inventory.Repair" ) )
		{
			UIActionImageButton repair = UIActionManager.CreateIconButton( host, JMConstants.Lucide( "wrench" ), this, "Click_BandRepairAll" );
			PrepareInventoryBandAction( repair, index, "#STR_COT_PLAYER_MODULE_TT_INV_GROUP_REPAIR" );
		}

		if ( GetPermissionsManager().HasPermission( "Admin.Player.Inventory.Take" ) )
		{
			UIActionImageButton take = UIActionManager.CreateIconButton( host, JMConstants.Lucide( "hand" ), this, "Click_BandTakeAll" );
			PrepareInventoryBandAction( take, index, "#STR_COT_PLAYER_MODULE_TT_INV_GROUP_TAKE" );
		}
	}

	//! Size the button to the header strip and pin the container it belongs to
	//! onto it. The INDEX is what travels, not the item: the callback re-reads
	//! m_InventoryItems, which is the array the listing rebuilt.
	private void PrepareInventoryBandAction( UIActionImageButton button, int index, string tooltip )
	{
		if ( !button )
			return;

		button.SetFixedSize( INV_BAND_ACTION_PX, INV_BAND_ACTION_PX );

		//! See INV_BAND_ICON_PX: the glyph is an exact size in the layout and
		//! has to be brought down by hand, or the button is all picture and no
		//! pill and the hover fill has nowhere to show.
		Widget glyph = button.GetLayoutRoot().FindAnyWidget( "action_image" );
		if ( glyph )
		{
			glyph.SetSize( INV_BAND_ICON_PX, INV_BAND_ICON_PX );
			glyph.Update();
		}

		button.SetData( new JMStringData( index.ToString() ) );

		if ( tooltip != "" )
			button.SetTooltip( tooltip );
	}

	//! Which container a band action was created for, or -1.
	private int InventoryBandActionIndex( UIActionBase action )
	{
		if ( !action )
			return -1;

		JMStringData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return -1;

		return data.Value.ToInt();
	}

	void Click_BandTakeAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		RunInventoryGroupAction( InventoryBandActionIndex( action ), JMInventoryGroupOp.TAKE_ALL );
	}

	void Click_BandRepairAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		RunInventoryGroupAction( InventoryBandActionIndex( action ), JMInventoryGroupOp.REPAIR_ALL );
	}

	void Click_BandDeleteAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		RunInventoryGroupAction( InventoryBandActionIndex( action ), JMInventoryGroupOp.DELETE_ALL );
	}

	private void RunInventoryGroupAction( int index, int op )
	{
		if ( !m_SelectedInstance )
			return;

		if ( index < 0 || index >= m_InventoryItems.Count() )
			return;

		JMPlayerInventoryItem container = m_InventoryItems[index];

		// Deleting a whole container's contents is the one irreversible action
		// here, and the glyph that starts it is 14 pixels wide - so it asks.
		if ( op == JMInventoryGroupOp.DELETE_ALL )
		{
			m_InvGroupPendingLow  = container.NetIdLow;
			m_InvGroupPendingHigh = container.NetIdHigh;
			m_InvGroupPendingName = container.GetDisplayName();

			CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_PLAYER_MODULE_INV_GROUP_DELETE_TITLE", string.Format( Widget.TranslateString( "#STR_COT_PLAYER_MODULE_INV_GROUP_DELETE_BODY" ), InventoryChildCount( index ).ToString(), m_InvGroupPendingName ), "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "InventoryGroupDeleteConfirm" );
			return;
		}

		m_Module.InventoryGroupOp( m_SelectedInstance.GetGUID(), container.NetIdLow, container.NetIdHigh, op );
	}

	void InventoryGroupDeleteConfirm( JMConfirmation confirmation = NULL )
	{
		if ( !m_SelectedInstance )
			return;

		m_Module.InventoryGroupOp( m_SelectedInstance.GetGUID(), m_InvGroupPendingLow, m_InvGroupPendingHigh, JMInventoryGroupOp.DELETE_ALL );
	}

	//! "Field Backpack (6)" - whose contents these are, and how many.
	private string InventoryGroupLabel( int index )
	{
		int count = InventoryChildCount( index );

		return m_InventoryItems[index].GetDisplayName() + " (" + count.ToString() + ")";
	}

	private int InventoryChildCount( int index )
	{
		int count = 0;

		foreach ( JMPlayerInventoryItem child : m_InventoryItems )
		{
			if ( child.ParentIndex == index )
				count++;
		}

		return count;
	}

	//! True when everything in this band is attached rather than carried. A
	//! mixed container is drawn as cargo - the per-cell badge separates the two
	//! inside it.
	private bool InventoryGroupIsAttachments( int index )
	{
		foreach ( JMPlayerInventoryItem child : m_InventoryItems )
		{
			if ( child.ParentIndex != index )
				continue;

			if ( !child.IsAttachment() )
				return false;
		}

		return true;
	}

	private void ToggleInventoryExpanded( int index )
	{
		int at = m_InvExpanded.Find( index );

		if ( at >= 0 )
			m_InvExpanded.Remove( at );
		else
			m_InvExpanded.Insert( index );

		RebuildInventoryTable( false );
	}

	//! One cell, unpositioned - the layout pass places it. NULL only when the
	//! layout itself failed to load.
	private Widget BuildInventoryCell( int index )
	{
		JMPlayerInventoryItem item = m_InventoryItems[index];

		Widget cell = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/player_inventory_cell.layout", m_InventoryCanvas );
		if ( !cell )
			return NULL;

		m_InvCells.Insert( cell );
		m_InvCellItem.Insert( index );

		bool allowPreview = m_InvPreviewsBuilt < INV_MAX_PREVIEWS;
		if ( allowPreview )
			m_InvPreviewsBuilt++;

		// Roots sit on the raised surface, anything inside a container on the
		// sunken one, so a cell says whether it is loose gear or the contents of
		// something before any badge is read.
		Widget fill = cell.FindAnyWidget( "cell_fill" );
		if ( fill )
		{
			if ( item.Depth > 0 )
				fill.SetColor( JMTheme.SURFACE_SUNKEN );
			else
				fill.SetColor( JMTheme.SURFACE_RAISED );
		}

		// Containers carry a chevron: closed points right, open points down,
		// exactly like the role groups in the player list.
		ImageWidget expand;
		if ( Class.CastTo( expand, cell.FindAnyWidget( "cell_expand" ) ) )
		{
			bool hasChildren = HasInventoryChildren( index );
			expand.Show( hasChildren );

			if ( hasChildren )
			{
				if ( m_InvExpanded.Find( index ) >= 0 )
					expand.LoadImageFile( 0, JMConstants.Lucide( "chevron-down" ) );
				else
					expand.LoadImageFile( 0, JMConstants.Lucide( "chevron-right" ) );
			}
		}

		PaintInventoryCellQuantity( cell, item );
		PaintInventoryCellHealth( cell, item );
		PaintInventoryCellRing( cell, index, false );

		// The cache key carries the child count as well as the classname: a
		// rifle that lost its optic between refreshes is still the same
		// classname, and would otherwise keep drawing the optic.
		string key = InventoryPreviewKey( index );

		if ( allowPreview && m_InvCellTypes[index] != key )
		{
			DestroyLocalPreview( m_InvCellEntities[index] );
			m_InvCellEntities[index] = SpawnLocalPreview( item );

			// Weapons only: their attachments are drawn ON the model, so a rifle
			// without them is the wrong picture. Cargo never shows through a
			// container's model, and rebuilding every container's subtree for an
			// icon would spawn the whole loadout once per cell.
			if ( item.IsWeapon() )
				BuildLocalPreviewChildren( index, m_InvCellEntities[index], true );

			FinishLocalPreview( m_InvCellEntities[index] );

			m_InvCellTypes[index] = key;
		}

		ItemPreviewWidget preview;
		if ( !Class.CastTo( preview, cell.FindAnyWidget( "cell_preview" ) ) )
			return cell;

		EntityAI entity = m_InvCellEntities[index];

		if ( !entity )
		{
			// Over the preview budget, or the classname does not resolve to
			// something spawnable. Fall back to the name so the cell still says
			// what it is.
			TextWidget name;
			if ( Class.CastTo( name, cell.FindAnyWidget( "cell_name" ) ) )
			{
				name.SetText( item.GetDisplayName() );
				name.Show( true );
			}

			return cell;
		}

		// Exactly the call sequence vanilla's own inventory icon uses, in the
		// same order. The engine frames the model itself from the config's
		// bounding box and invView - which is why an item here now fills its
		// cell the way it does in the game's inventory. COT used to push the
		// model back with SetModelPosition to "fit" it, and that was the whole
		// reason these previews came out a fraction of vanilla's size: it was
		// overriding the auto-fit, not helping it.
		preview.SetForceFlipEnable( true );
		preview.SetForceFlip( false );
		preview.SetItem( entity );
		preview.SetModelOrientation( vector.Zero );
		preview.SetView( entity.GetViewIndex() );

		preview.Show( true );

		return cell;
	}

	//! What a cell's preview entity was built from. Two items of the same type
	//! holding different things must not share one cached model, and neither
	//! may two states of the SAME item: health, quantity, temperature and food
	//! stage are all baked into the spawned entity at SpawnLocalPreview time,
	//! not re-applied afterward, so a key that ignored them kept showing the
	//! item's state as of whenever the preview first spawned - pristine, full
	//! and room temperature - no matter how many edits landed on it since.
	private string InventoryPreviewKey( int index )
	{
		JMPlayerInventoryItem item = m_InventoryItems[index];

		return item.Type + "|" + InventoryChildCount( index ).ToString() + "|" + item.Health.ToString() + "|" + item.Quantity.ToString() + "|" + item.Temperature.ToString() + "|" + item.Stage.ToString();
	}

	//! Quantity is drawn the way the item's own config asks for it - vanilla's
	//! rule, out of QuantityConversions: a magazine counts rounds, a config that
	//! sets quantityBar (or that has a max of one, which is a percentage in
	//! disguise) gets a bar, anything else with a max prints the number. A
	//! bandage stack reads "4/4"; a pear and a chemlight are bars.
	private int InventoryQuantityMode( JMPlayerInventoryItem item )
	{
		if ( item.QuantityMax <= 0 )
			return INV_QTY_HIDDEN;

		if ( item.IsMagazine() )
			return INV_QTY_COUNT;

		if ( item.QuantityMax <= 1 )
			return INV_QTY_BAR;

		if ( g_Game.ConfigGetInt( "CfgVehicles " + item.Type + " quantityBar" ) > 0 )
			return INV_QTY_BAR;

		return INV_QTY_COUNT;
	}

	private void PaintInventoryCellQuantity( Widget cell, JMPlayerInventoryItem item )
	{
		int mode = InventoryQuantityMode( item );

		bool asBar   = mode == INV_QTY_BAR;
		bool asCount = mode == INV_QTY_COUNT;

		Widget track = cell.FindAnyWidget( "cell_quantity_track" );
		if ( track )
			track.Show( asBar );

		TextWidget label;
		if ( Class.CastTo( label, cell.FindAnyWidget( "cell_quantity" ) ) )
		{
			label.Show( asCount );

			if ( asCount )
				label.SetText( item.GetQuantityLabel() );
		}

		Widget fill = cell.FindAnyWidget( "cell_quantity_fill" );
		if ( !fill )
			return;

		fill.Show( asBar );

		if ( !asBar )
			return;

		float ratio = Math.Clamp( item.Quantity / item.QuantityMax, 0.0, 1.0 );

		// Against the track's own drawn width, not a constant: the bar has to
		// end where the track ends at every UI scale.
		float tw, th;
		float w, h;
		fill.GetScreenSize( w, h );

		if ( track )
		{
			track.GetScreenSize( tw, th );
			if ( tw > 0 )
				w = tw;
		}

		fill.SetScreenSize( Math.Max( 2, w * ratio ), h );

		// A nearly empty canteen is worth spotting without reading anything;
		// everything else is the neutral accent so the bar does not compete with
		// the condition dot for attention.
		if ( ratio <= 0.15 )
			fill.SetColor( JMTheme.WARNING );
		else
			fill.SetColor( JMTheme.ACCENT );
	}

	//! Condition is a dot in the top-right corner, on vanilla's own bands, so
	//! "red" means the same thing here as it does in the player's own inventory.
	private void PaintInventoryCellHealth( Widget cell, JMPlayerInventoryItem item )
	{
		Widget dot = cell.FindAnyWidget( "cell_health_dot" );
		if ( !dot )
			return;

		dot.SetColor( InventoryHealthColour( item ) );
	}

	//! The condition dot uses VANILLA's colours, straight from
	//! ItemManager.GetItemHealthColor - green / yellow-green / yellow / amber /
	//! red - so a dot here means exactly what the same colour means in the
	//! player's own inventory. Those constants carry no alpha, hence the OR.
	//!
	//! The level comes from the server, where the item's own healthLevels
	//! config decided it. The threshold fallback is only for a listing from an
	//! older build, which sends no level at all.
	private int InventoryHealthColour( JMPlayerInventoryItem item )
	{
		if ( item.HealthLevel >= 0 )
			return ItemManager.GetItemHealthColor( item.HealthLevel ) | 0xFF000000;

		if ( item.IsRuined() )
			return Colors.COLOR_RUINED | 0xFF000000;

		if ( item.Health > 70 )
			return Colors.COLOR_PRISTINE | 0xFF000000;

		if ( item.Health > 50 )
			return Colors.COLOR_WORN | 0xFF000000;

		if ( item.Health > 30 )
			return Colors.COLOR_DAMAGED | 0xFF000000;

		if ( item.Health > 0 )
			return Colors.COLOR_BADLY_DAMAGED | 0xFF000000;

		return Colors.COLOR_RUINED | 0xFF000000;
	}

	//! Index of the cell a widget belongs to, or -1. Cells are a handful of
	//! widgets each, so anything the pointer lands on is a descendant.
	private int InventoryCellIndexOf( Widget w )
	{
		if ( !w || !m_InvCells )
			return -1;

		foreach ( int i, Widget cell : m_InvCells )
		{
			Widget walk = w;
			while ( walk )
			{
				if ( walk == cell )
					return m_InvCellItem[i];

				walk = walk.GetParent();
			}
		}

		return -1;
	}

	//! Screen position of the cell showing this item, or -1 when it is not on
	//! screen. The inverse of InventoryCellIndexOf.
	private int InventoryCellSlotOf( int itemIndex )
	{
		// Empty equipment slots are stored as -1, which is also "nothing is
		// selected" - without this an unselected grid would match the first
		// ghost cell and repaint it as though it were the selection.
		if ( itemIndex < 0 )
			return -1;

		foreach ( int i, int shown : m_InvCellItem )
		{
			if ( shown == itemIndex )
				return i;
		}

		return -1;
	}

	private void SelectInventoryCell( int index )
	{
		int previous = InventoryCellSlotOf( m_InvSelectedIndex );
		if ( previous >= 0 )
			PaintInventoryCellRing( m_InvCells[previous], m_InvSelectedIndex, false );

		m_InvSelectedIndex = index;

		int slot = InventoryCellSlotOf( index );
		if ( slot >= 0 )
			PaintInventoryCellRing( m_InvCells[slot], index, true );
	}

	//! Selection wins, then the OPEN state: a container that is showing its
	//! contents keeps a lit edge, so the run of child cells after it can be
	//! traced back to what they came out of.
	private void PaintInventoryCellRing( Widget cell, int index, bool selected )
	{
		Widget ring = cell.FindAnyWidget( "cell_ring" );
		if ( !ring )
			return;

		Widget inner = cell.FindAnyWidget( "cell_ring_inner" );

		bool expanded = index >= 0 && m_InvExpanded.Find( index ) >= 0;

		if ( selected )
		{
			ring.SetColor( JMTheme.BORDER_RING );
			PaintInventoryCellInnerRing( inner, expanded, JMTheme.BORDER_RING );
			return;
		}

		if ( expanded )
		{
			ring.SetColor( JMTheme.BORDER_ACCENT );
			PaintInventoryCellInnerRing( inner, true, JMTheme.BORDER_ACCENT );
			return;
		}

		ring.SetColor( JMTheme.BORDER_SUBTLE );
		PaintInventoryCellInnerRing( inner, false, JMTheme.BORDER_SUBTLE );
	}

	//! The second, inset ring, drawn one pixel inside the first.
	//!
	//! COT's 8-slice border texture is a single pixel wide and there is no
	//! heavier variant of it, so an open container is marked by drawing the ring
	//! TWICE rather than by authoring another image set. Two concentric strokes
	//! read as one thick edge, which is what separates the container being
	//! looked into from the rest of the grid.
	private void PaintInventoryCellInnerRing( Widget inner, bool show, int color )
	{
		if ( !inner )
			return;

		inner.Show( show );

		if ( show )
			inner.SetColor( color );
	}

	void Click_RefreshInventory( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		SpinRefreshIcon( action );
		RequestInventory();
	}

	//! Turn the glyph of whichever refresh button was clicked.
	//!
	//! Every one of these actions can complete with nothing on screen changing -
	//! a listing that came back identical, a stat block that has not moved - and
	//! a button that produces no feedback reads as a button that did not work.
	//! Two revolutions, matching every other refresh button in COT.
	private void SpinRefreshIcon( UIActionBase action )
	{
		UIActionImageButton button;
		if ( Class.CastTo( button, action ) )
			button.TriggerSpin( 2 );
	}

	void Click_RefreshVitals( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		SpinRefreshIcon( action );
		RefreshStats( true );
	}

	//! Unlike the player-list quick actions, this button lives inside the
	//! inventory panel itself - it always means the player whose inventory is
	//! open (m_SelectedInstance), never whatever else is checkbox-selected in
	//! the list behind it. Going through JM_GetSelected() here meant clicking
	//! it could wipe cargo on a different player than the one on screen.
	void Click_ClearCargoOnly( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_SelectedInstance )
			return;

		CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_PLAYER_MODULE_INV_CLEAR_CARGO", "#STR_COT_PLAYER_MODULE_INV_CLEAR_CARGO_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "ClearCargoOnlyConfirm" );
	}

	void ClearCargoOnlyConfirm( JMConfirmation confirmation = NULL )
	{
		if ( !m_SelectedInstance )
			return;

		m_Module.ClearCargo( { m_SelectedInstance.GetGUID() } );
		RequestInventory();
	}

	// -------------------------------------------------------------------------
	//  Player list quick actions
	// -------------------------------------------------------------------------

	//! A row was right-clicked. Built fresh every time because the freeze entry
	//! reads the player state, and greyed rather than dropped where a permission
	//! is missing so the menu keeps one shape whoever opens it.
	void OnPlayerRow_RightClick( string guid, int x, int y )
	{
		if ( guid == "" || !m_Module )
			return;

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( guid );
		if ( !instance )
			return;

		if ( !m_PlayerMenu )
		{
			if ( !m_Window )
				return;

			//! Anchored to the window root, not to the list: a menu parented to
			//! the scroller would be clipped by it a row from the bottom.
			m_PlayerMenu = UIActionManager.CreateContextMenu( layoutRoot, m_Window.GetWidgetRoot(), this, "OnClick_PlayerMenu" );
			RegisterOverlay( m_PlayerMenu );

			if ( !m_PlayerMenu )
				return;
		}

		m_PlayerMenuGUID = guid;

		m_PlayerMenu.ClearItems();

		//! The repairs come first and only for the states the player is in, so
		//! the entry an admin opened the menu for is the one under the cursor.
		AddStatusRepairItems( m_PlayerMenu, instance );

		//! Two entries are meaningless on your own row and are left out rather
		//! than greyed: "teleport me to" would be a no-op walk to where you are
		//! already standing, and spectating yourself is what the game already
		//! does. "Teleport to me" stays - it is how an admin recalls a character
		//! they left somewhere, and it reads the same either way.
		bool isSelf = ( guid == GetPermissionsManager().GetClientGUID() );

		m_PlayerMenu.AddItem( ROW_MENU_HEAL,     "#STR_COT_PLAYER_MODULE_ACTION_HEAL",     JMConstants.Lucide( "heart-pulse" ) );
		m_PlayerMenu.AddItem( ROW_MENU_TP_TO_ME, "#STR_COT_PLAYER_MODULE_TELEPORT_TO_ME",  JMConstants.Lucide( "move-down-left" ) );

		if ( !isSelf )
			m_PlayerMenu.AddItem( ROW_MENU_TP_ME_TO, "#STR_COT_PLAYER_MODULE_TELEPORT_ME_TO",  JMConstants.Lucide( "footprints" ) );

		//! Greyed rather than dropped when there is nothing to take back: an
		//! entry that comes and goes moves every row under it between one
		//! opening of the menu and the next.
		m_PlayerMenu.AddItem( ROW_MENU_TP_UNDO, "#STR_COT_TELEPORT_UNDO", JMConstants.Lucide( "undo-2" ) );
		m_PlayerMenu.AddItem( ROW_MENU_TP_REDO, "#STR_COT_TELEPORT_REDO", JMConstants.Lucide( "redo-2" ) );

		//! Moves the Position tab's map, not the player - the one way to tell
		//! two sets of coordinates apart is to look at them.
		m_PlayerMenu.AddItem( ROW_MENU_FOCUS_MAP, "#STR_COT_TELEPORT_HISTORY_FOCUS", JMConstants.Lucide( "locate-fixed" ) );

		if ( instance.IsFrozen() )
			m_PlayerMenu.AddItem( ROW_MENU_FREEZE, "#STR_COT_PLAYER_MODULE_ACTION_UNFREEZE", JMConstants.Lucide( "snowflake" ) );
		else
			m_PlayerMenu.AddItem( ROW_MENU_FREEZE, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_FREEZE", JMConstants.Lucide( "snowflake" ) );

#ifdef DAYZ_1_30
		if ( instance.IsRagdoll() )
			m_PlayerMenu.AddItem( ROW_MENU_RAGDOLL, "#STR_COT_PLAYER_MODULE_ACTION_UNRAGDOLL", JMConstants.Lucide( "bone" ) );
		else
			m_PlayerMenu.AddItem( ROW_MENU_RAGDOLL, "#STR_COT_PLAYER_MODULE_ACTION_RAGDOLL", JMConstants.Lucide( "bone" ) );
#endif

		if ( !isSelf )
			m_PlayerMenu.AddItem( ROW_MENU_SPECTATE, "#STR_COT_PLAYER_MODULE_ACTION_SPECTATE", JMConstants.Lucide( "eye" ) );

		m_PlayerMenu.AddItem( ROW_MENU_COPY_GUID,  "#STR_COT_ESP_MODULE_MENU_COPY_GUID", JMConstants.Lucide( "copy" ) );
		m_PlayerMenu.AddItem( ROW_MENU_COPY_STEAM, "#STR_COT_ESP_MODULE_MENU_COPY_STEAM", JMConstants.Lucide( "copy" ) );

		//! A context menu cannot be typed into, so the body comes off the
		//! clipboard - the same route the ESP menu takes for the same reason.
		m_PlayerMenu.AddItem( ROW_MENU_MESSAGE, "#STR_COT_ESP_MODULE_MENU_MESSAGE", JMConstants.Lucide( "message-square" ) );

		//! Destructive, and tinted like it. These four are the reason the row
		//! menu is worth having at all - an admin dealing with someone should
		//! not have to change tab to act.
		m_PlayerMenu.AddItem( ROW_MENU_CLEARCARGO, "#STR_COT_PLAYER_MODULE_ACTION_CLEAR_CARGO", JMConstants.Lucide( "package-x" ), JMTheme.DANGER );
		m_PlayerMenu.AddItem( ROW_MENU_STRIP,      "#STR_COT_PLAYER_MODULE_ACTION_STRIP",       JMConstants.Lucide( "shirt" ),     JMTheme.DANGER );

		if ( !isSelf )
		{
			m_PlayerMenu.AddItem( ROW_MENU_KICK, "#STR_COT_PLAYER_MODULE_ACTION_KICK", JMConstants.Lucide( "door-open" ), JMTheme.DANGER );
			m_PlayerMenu.AddItem( ROW_MENU_BAN,  "#STR_COT_PLAYER_MODULE_ACTION_BAN",  JMConstants.Lucide( "gavel" ),     JMTheme.DANGER );
		}

		//! SetItemEnabled on an id the menu is not carrying is a no-op, so the
		//! two self-only omissions above need no special case here.
		m_PlayerMenu.SetItemEnabled( ROW_MENU_HEAL,     GetPermissionsManager().HasPermission( "Admin.Player.Heal" ) );
		m_PlayerMenu.SetItemEnabled( ROW_MENU_TP_TO_ME, GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Position" ) );
		m_PlayerMenu.SetItemEnabled( ROW_MENU_TP_ME_TO, GetPermissionsManager().HasPermission( "Admin.Player.Teleport.SenderTo" ) );
		m_PlayerMenu.SetItemEnabled( ROW_MENU_TP_UNDO,  GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Position" ) && JMTeleportHistory.Has( JMTeleportHistory.PlayerKey( instance.GetGUID() ) ) );
		m_PlayerMenu.SetItemEnabled( ROW_MENU_TP_REDO,  GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Position" ) && JMTeleportHistory.HasRedo( JMTeleportHistory.PlayerKey( instance.GetGUID() ) ) );
		m_PlayerMenu.SetItemEnabled( ROW_MENU_MESSAGE,    GetPermissionsManager().HasPermission( "Admin.Player.Message" ) );
		m_PlayerMenu.SetItemEnabled( ROW_MENU_CLEARCARGO, GetPermissionsManager().HasPermission( "Admin.Player.ClearCargo" ) );
		m_PlayerMenu.SetItemEnabled( ROW_MENU_STRIP,      GetPermissionsManager().HasPermission( "Admin.Player.Strip" ) );
		m_PlayerMenu.SetItemEnabled( ROW_MENU_KICK,       GetPermissionsManager().HasPermission( "Admin.Player.Kick" ) );
		m_PlayerMenu.SetItemEnabled( ROW_MENU_BAN,        GetPermissionsManager().HasPermission( "Admin.Player.Ban" ) );
		m_PlayerMenu.SetItemEnabled( ROW_MENU_FREEZE,   GetPermissionsManager().HasPermission( "Admin.Player.Freeze" ) );
#ifdef DAYZ_1_30
		m_PlayerMenu.SetItemEnabled( ROW_MENU_RAGDOLL,  GetPermissionsManager().HasPermission( "Admin.Player.Ragdoll" ) );
#endif
		m_PlayerMenu.SetItemEnabled( ROW_MENU_SPECTATE, GetPermissionsManager().HasPermission( "Admin.Player.Spectate" ) );

		SetStatusRepairPermissions( m_PlayerMenu );

		m_PlayerMenu.ShowAt( x, y );
	}

	//! GUID a row-menu message prompt is pending for, between ShowAt() (in
	//! RunRowMessage) and the confirmation's own callback - same pattern as
	//! JMESPForm.m_PendingMsgPlayerGUID / PromptSendMessage, which already
	//! solved this identically for the world right-click player menu.
	private string m_PendingRowMsgGuid;

	//! Opens a text-entry prompt for one player, same as the ESP module's own
	//! player menu already does. Used to read from the clipboard instead - "a
	//! context menu cannot be typed into" - but a context menu can raise a
	//! confirmation dialog just fine, which is exactly what JMESPForm's
	//! PromptSendMessage does, so this matches it instead of asking the admin
	//! to pre-copy the message text before right-clicking.
	private void RunRowMessage( string guid )
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Message" ) )
			return;

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( guid );
		if ( !instance )
			return;

		m_PendingRowMsgGuid = guid;

		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_MESSAGE_HEADER", Widget.TranslateString( "#STR_COT_ESP_MODULE_MESSAGE_PROMPT" ) + " " + instance.GetName() + ":", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "OnRowMessage_Confirm" );
	}

	private void OnRowMessage_Confirm( JMConfirmation confirmation = NULL )
	{
		if ( !confirmation )
			return;

		string msgText = confirmation.GetEditBoxValue();
		if ( msgText == "" || m_PendingRowMsgGuid == "" )
			return;

		m_Module.DoMessage( { m_PendingRowMsgGuid }, msgText );
		m_PendingRowMsgGuid = "";
	}

	//! The repair entries for whatever is currently wrong with a player, added
	//! to `menu` in the order the list badges use. A healthy player contributes
	//! nothing, which is the point: the menu is a list of what can be fixed.
	//!
	//! A corpse gets none of them - reviving is not one of these calls, and
	//! stopping the bleeding of a dead man is not an action worth offering.
	private void AddStatusRepairItems( UIActionContextMenu menu, JMPlayerInstance instance )
	{
		if ( !menu || !instance || instance.IsDead() )
			return;

		if ( instance.IsUnconscious() )
			AddStatusRepairItem( menu, ROW_MENU_WAKE );

		if ( instance.HasBrokenLegs() )
			AddStatusRepairItem( menu, ROW_MENU_FIX_LEGS );

		if ( instance.IsBleeding() )
			AddStatusRepairItem( menu, ROW_MENU_STOP_BLEED );

		if ( instance.IsSick() )
			AddStatusRepairItem( menu, ROW_MENU_CURE );
	}

	//! One repair entry, so the row menu and the status glyphs word the same fix
	//! the same way. An unknown id adds nothing.
	private void AddStatusRepairItem( UIActionContextMenu menu, string id )
	{
		if ( !menu )
			return;

		if ( id == ROW_MENU_WAKE )
			menu.AddItem( ROW_MENU_WAKE, "#STR_COT_PLAYER_MODULE_ACTION_WAKE", JMConstants.Lucide( "bed" ) );
		else if ( id == ROW_MENU_FIX_LEGS )
			menu.AddItem( ROW_MENU_FIX_LEGS, "#STR_COT_PLAYER_MODULE_ACTION_FIX_LEGS", JMConstants.Lucide( "bone-fracture" ) );
		else if ( id == ROW_MENU_STOP_BLEED )
			menu.AddItem( ROW_MENU_STOP_BLEED, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_STOP_BLEEDING", JMConstants.Lucide( "droplet" ) );
		else if ( id == ROW_MENU_CURE )
			menu.AddItem( ROW_MENU_CURE, "#STR_COT_PLAYER_MODULE_ACTION_CURE", JMConstants.Lucide( "thermometer" ) );
	}

	//! Grey rather than drop: SetItemEnabled on an id the menu does not carry is
	//! a no-op, so this can be called unconditionally after every build.
	private void SetStatusRepairPermissions( UIActionContextMenu menu )
	{
		if ( !menu )
			return;

		menu.SetItemEnabled( ROW_MENU_WAKE,       GetPermissionsManager().HasPermission( "Admin.Player.Set.Shock" ) );
		menu.SetItemEnabled( ROW_MENU_FIX_LEGS,   GetPermissionsManager().HasPermission( "Admin.Player.BrokenLegs" ) );
		menu.SetItemEnabled( ROW_MENU_STOP_BLEED, GetPermissionsManager().HasPermission( "Admin.Player.Bleed.Add" ) );
		menu.SetItemEnabled( ROW_MENU_CURE,       GetPermissionsManager().HasPermission( "Admin.Player.Disease.Remove" ) );
	}

	//! Run one status repair against one guid. Shared by the player-list row
	//! menu and the Active Statuses card, which offer the same four fixes.
	//! Returns false for an id that is not a repair so the caller can go on
	//! matching its own entries.
	private bool RunStatusRepair( string id, string guid )
	{
		if ( guid == "" || !m_Module )
			return false;

		if ( id == ROW_MENU_WAKE )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Shock" ) )
				return true;

			UpdateLastChangeTime();
			m_Module.SetShock( SHOCK_CONSCIOUS, { guid } );

			return true;
		}

		if ( id == ROW_MENU_FIX_LEGS )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.BrokenLegs" ) )
				return true;

			UpdateLastChangeTime();
			m_Module.SetBrokenLegs( false, { guid } );

			return true;
		}

		if ( id == ROW_MENU_STOP_BLEED )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Bleed.Add" ) )
				return true;

			UpdateLastChangeTime();
			m_Module.StopBleeding( { guid } );

			return true;
		}

		if ( id == ROW_MENU_CURE )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Disease.Remove" ) )
				return true;

			UpdateLastChangeTime();
			m_Module.RemoveAllDiseases( { guid } );

			return true;
		}

		return false;
	}

	//! Every branch addresses m_PlayerMenuGUID alone. None of these go through
	//! the confirmation dialog the equivalent buttons use: a context menu IS the
	//! confirmation - it was opened on one named row and names the action.
	void OnClick_PlayerMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_PlayerMenu || !m_Module )
			return;

		string id   = m_PlayerMenu.GetLastClickedId();
		string guid = m_PlayerMenuGUID;

		if ( guid == "" )
			return;

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( guid );
		if ( !instance )
			return;

		//! Enforce has no block scope, so everything the branches need is
		//! declared once here rather than inside the branch that uses it.
		vector toMe = vector.Zero;
		PlayerBase spectated;

		if ( RunStatusRepair( id, guid ) )
			return;

		if ( id == ROW_MENU_HEAL )
		{
			if ( GetPermissionsManager().HasPermission( "Admin.Player.Heal" ) )
				m_Module.Heal( { guid } );

			return;
		}

		if ( id == ROW_MENU_TP_TO_ME )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Position" ) )
				return;

			toMe = GetCurrentPosition();

			if ( toMe != vector.Zero )
				m_Module.TeleportTo( toMe, { guid } );

			return;
		}

		if ( id == ROW_MENU_TP_REDO )
		{
			JMTeleportHistory.RedoPlayer( guid, 0 );

			return;
		}

		if ( id == ROW_MENU_FOCUS_MAP )
		{
			JMPlayerInstance focusInstance = GetPermissionsManager().GetPlayer( guid );

			if ( focusInstance )
				FocusMap( focusInstance.GetPosition() );

			return;
		}

		if ( id == ROW_MENU_COPY_GUID )
		{
			g_Game.CopyToClipboard( guid );

			return;
		}

		if ( id == ROW_MENU_COPY_STEAM )
		{
			JMPlayerInstance steamInstance = GetPermissionsManager().GetPlayer( guid );

			if ( steamInstance )
				g_Game.CopyToClipboard( steamInstance.GetSteam64ID() );

			return;
		}

		if ( id == ROW_MENU_MESSAGE )
		{
			RunRowMessage( guid );

			return;
		}

		if ( id == ROW_MENU_CLEARCARGO )
		{
			if ( GetPermissionsManager().HasPermission( "Admin.Player.ClearCargo" ) )
				m_Module.ClearCargo( { guid } );

			return;
		}

		if ( id == ROW_MENU_STRIP )
		{
			if ( GetPermissionsManager().HasPermission( "Admin.Player.Strip" ) )
				m_Module.Strip( { guid } );

			return;
		}

		if ( id == ROW_MENU_KICK )
		{
			if ( GetPermissionsManager().HasPermission( "Admin.Player.Kick" ) )
				m_Module.Kick( { guid }, "" );

			return;
		}

		if ( id == ROW_MENU_BAN )
		{
			if ( GetPermissionsManager().HasPermission( "Admin.Player.Ban" ) )
				m_Module.Ban( { guid }, "" );

			return;
		}

		if ( id == ROW_MENU_TP_UNDO )
		{
			//! One step per use, so repeated right-clicks walk back through the
			//! whole history rather than needing five separate menu entries.
			JMTeleportHistory.UndoPlayer( guid, 0 );

			return;
		}

		if ( id == ROW_MENU_TP_ME_TO )
		{
			if ( GetPermissionsManager().HasPermission( "Admin.Player.Teleport.SenderTo" ) )
				m_Module.TeleportSenderTo( guid );

			return;
		}

		if ( id == ROW_MENU_FREEZE )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Freeze" ) )
				return;

			UpdateLastChangeTime();
			m_Module.SetFreeze( !instance.IsFrozen(), { guid } );

			return;
		}

#ifdef DAYZ_1_30
		if ( id == ROW_MENU_RAGDOLL )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Ragdoll" ) )
				return;

			UpdateLastChangeTime();
			m_Module.SetRagdoll( !instance.IsRagdoll(), { guid } );

			return;
		}
#endif

		if ( id == ROW_MENU_SPECTATE )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Spectate" ) )
				return;

			spectated = instance.PlayerObject;
			m_Module.Click_Spectate( action, spectated, guid );
		}
	}

	//! Cells are plain widgets under the form's own handler, so their pointer
	//! events arrive here rather than through a UIAction callback. Returns false
	//! for anything that is not a cell so the rest of the form still sees it.
	//!
	//! WHICH EVENT DOES WHAT, and why it matters:
	//!
	//!   down  - selection, and the right-click menu. Cheap, and touches no
	//!           widget the pointer is currently over.
	//!   up    - the container toggle, because that REBUILDS THE GRID. Doing it
	//!           on the way down destroys the very widget the press started on,
	//!           and the engine answers a vanished press target by recentring
	//!           the cursor - the pointer jumps to the middle of the screen
	//!           mid-click.
	//!
	//! OnClick is deliberately not the toggle either: a PanelWidget is not a
	//! ButtonWidget and does not reliably raise a click at all, which is what
	//! left containers that never opened in the first place.
	private bool OnInventoryCellPressed( Widget w, int button )
	{
		int index = InventoryCellIndexOf( w );
		if ( index < 0 )
			return false;

		SelectInventoryCell( index );

		if ( button == MouseState.RIGHT )
		{
			ShowInventoryContextMenu();
			return true;
		}

		return true;
	}

	//! The button came back up over a cell. Selection already happened on the
	//! way down, so the context menu and every operation still act on the
	//! container itself; this only opens or closes it.
	private bool OnInventoryCellReleased( Widget w, int button )
	{
		if ( button != MouseState.LEFT )
			return false;

		int index = InventoryCellIndexOf( w );
		if ( index < 0 )
			return false;

		if ( HasInventoryChildren( index ) )
			ToggleInventoryExpanded( index );

		return true;
	}

	//! Selection only. See OnInventoryCellPressed for why the toggle is not
	//! here as well.
	private bool OnInventoryCellSelect( Widget w )
	{
		int index = InventoryCellIndexOf( w );
		if ( index < 0 )
			return false;

		SelectInventoryCell( index );

		return true;
	}

	private bool OnInventoryCellHover( Widget w )
	{
		int index = InventoryCellIndexOf( w );
		if ( index < 0 )
			return false;

		if ( index == m_InvHoveredIndex )
			return true;

		m_InvHoveredIndex = index;
		ShowInventoryTooltip();
		return true;
	}

	//! DayZ cannot draw an item that is not a real entity, and these items are
	//! in ANOTHER player's inventory on the server - they are not replicated to
	//! this client unless the admin happens to be standing on top of them. So
	//! the preview is a client-local copy spawned from the classname with the
	//! health and quantity the listing carried applied on top. It is never the
	//! real item and no operation ever goes near it; only the network ID does,
	//! and only server-side.
	private EntityAI SpawnLocalPreview( JMPlayerInventoryItem item )
	{
		EntityAI ent = EntityAI.Cast( g_Game.CreateObject( item.Type, vector.Zero, true, false, false ) );
		if ( !ent )
			return NULL;

		dBodyActive( ent, ActiveState.INACTIVE );
		dBodyDynamic( ent, false );

		if ( item.Health > 0 )
			ent.SetHealth( "", "", item.Health );

		ItemBase asItem;
		if ( Class.CastTo( asItem, ent ) )
		{
			if ( item.QuantityMax > 0 )
				asItem.SetQuantity( item.Quantity, false );

			//! SetTemperatureEx dereferences m_TAC, which EntityAI only builds
			//! server-side (InitItemVariables). This preview is client-local, so
			//! the call must be skipped there or it NULLs on entities like Apple.
			if ( g_Game.IsServer() )
				asItem.SetTemperatureEx( new TemperatureData( item.Temperature ) );

			Edible_Base asFood;
			if ( item.HasFoodStage() && Class.CastTo( asFood, asItem ) && asFood.GetFoodStage() )
				asFood.GetFoodStage().ChangeFoodStage( item.Stage );
		}

		return ent;
	}

	//! Freeze a preview tree once it is fully built.
	//!
	//! Simulation is switched off LAST, not at spawn: a weapon whose simulation
	//! is already off does not run the state change that seats a magazine, so a
	//! rifle built the other way round came up with its optic and no mag.
	private void FinishLocalPreview( EntityAI ent )
	{
		if ( !ent )
			return;

		ent.DisableSimulation( true );
	}

	private void DestroyLocalPreview( EntityAI ent )
	{
		if ( ent )
			g_Game.ObjectDelete( ent );
	}

	//! Spawn a child loose and hand it to the parent with a LOCAL inventory
	//! operation - the path EntityAI itself exposes for client-side work, and
	//! the one that takes a magazine when CreateAttachment will not.
	private EntityAI AttachLocalPreviewChild( EntityAI parent, JMPlayerInventoryItem child, int slotId )
	{
		EntityAI ent = EntityAI.Cast( g_Game.CreateObject( child.Type, vector.Zero, true, false, false ) );
		if ( !ent )
			return NULL;

		bool taken = false;

		if ( slotId >= 0 )
			taken = parent.LocalTakeEntityAsAttachmentEx( ent, slotId );

		if ( !taken )
			taken = parent.LocalTakeEntityAsAttachment( ent );

		if ( !taken )
		{
			g_Game.ObjectDelete( ent );
			return NULL;
		}

		return ent;
	}

	//! Rebuild an item's attachments and cargo onto its local copy.
	//!
	//! The listing is a PREORDER flat array - a parent always appears before its
	//! children, and a whole subtree is contiguous - so one forward pass is
	//! enough and the subtree ends at the first entry back at or above the
	//! root's depth. Without this a rifle inspects as a bare receiver with no
	//! optic, no magazine and no suppressor.
	private void BuildLocalPreviewChildren( int rootIndex, EntityAI rootEntity, bool attachmentsOnly = false )
	{
		if ( !rootEntity || rootIndex < 0 || rootIndex >= m_InventoryItems.Count() )
			return;

		int built  = 0;
		int failed = 0;
		string failures = "";

		map<int, EntityAI> spawned = new map<int, EntityAI>;
		spawned.Insert( rootIndex, rootEntity );

		JMPlayerInventoryItem child;
		EntityAI parentEntity;
		EntityAI childEntity;
		ItemBase asItem;

		// No early exit on depth. The subtree IS contiguous under a PREORDER
		// walk, but bailing out on the first entry back at the root's depth
		// meant one unexpected ordering dropped the rest of the subtree - which
		// is how a rifle came up with its optic but no magazine. Membership in
		// `spawned` already restricts this to the root's descendants, and the
		// listing is capped at 256 entries, so just walk the whole thing.
		for ( int i = rootIndex + 1; i < m_InventoryItems.Count(); i++ )
		{
			child = m_InventoryItems[i];

			if ( !spawned.Contains( child.ParentIndex ) )
				continue;

			parentEntity = spawned.Get( child.ParentIndex );
			if ( !parentEntity || !parentEntity.GetInventory() )
				continue;

			// An icon only ever needs what is drawn ON the model. Pulling a
			// backpack's contents in as well would spawn the whole loadout once
			// per container cell.
			if ( attachmentsOnly && !child.IsAttachment() )
				continue;

			childEntity = NULL;

			int slotId = -1;
			if ( child.SlotName != "" )
				slotId = InventorySlots.GetSlotIdFromString( child.SlotName );

			// By slot id first, so a thing that can live in more than one slot
			// lands in the one it was actually in. By type second, because a
			// magazine's slot name does not always resolve to an id the target
			// weapon accepts. Then the LOCAL inventory operation, which is the
			// client-side path vanilla itself uses and the one that seats a
			// magazine the CreateAttachment calls refuse. Cargo last.
			if ( slotId >= 0 )
				childEntity = parentEntity.GetInventory().CreateAttachmentEx( child.Type, slotId );

			if ( !childEntity && child.IsAttachment() )
				childEntity = parentEntity.GetInventory().CreateAttachment( child.Type );

			if ( !childEntity && child.IsAttachment() )
				childEntity = AttachLocalPreviewChild( parentEntity, child, slotId );

			if ( !childEntity && !attachmentsOnly )
				childEntity = parentEntity.GetInventory().CreateInInventory( child.Type );

			if ( !childEntity )
			{
				failed++;
				failures = failures + " " + child.Type + "/" + child.SlotName;
				continue;
			}

			built++;

			if ( child.Health > 0 )
				childEntity.SetHealth( "", "", child.Health );

			if ( child.QuantityMax > 0 && Class.CastTo( asItem, childEntity ) )
				asItem.SetQuantity( child.Quantity, false );

			FinishLocalPreview( childEntity );

			spawned.Insert( i, childEntity );
		}

		// Loud on purpose: a missing attachment is invisible in the picture and
		// impossible to tell apart from an item that simply has none.
		if ( failed > 0 )
			Print( "[COT-PREVIEW] children of " + m_InventoryItems[rootIndex].Type + ": built=" + built + " failed=" + failed + " ->" + failures );
	}

	private void ShowInventoryTooltip()
	{
		if ( !m_InvTooltip )
			return;

		int row = m_InvHoveredIndex;
		if ( row < 0 || row >= m_InventoryItems.Count() )
			return;

		JMPlayerInventoryItem item = m_InventoryItems[row];

		// The cell already owns a copy of this item for its own preview, so the
		// tooltip borrows it rather than spawning a second one. Only a cell past
		// the preview budget needs its own, and then just one at a time.
		EntityAI entity = NULL;
		if ( row < m_InvCellEntities.Count() )
			entity = m_InvCellEntities[row];

		if ( !entity )
		{
			if ( item.Type != m_InvHoverType )
			{
				DestroyLocalPreview( m_InvHoverEntity );
				m_InvHoverEntity = SpawnLocalPreview( item );

				if ( item.IsWeapon() )
					BuildLocalPreviewChildren( row, m_InvHoverEntity, true );

				FinishLocalPreview( m_InvHoverEntity );

				m_InvHoverType   = item.Type;
			}

			entity = m_InvHoverEntity;
		}

		if ( !entity )
			return;

		InspectMenuNew.UpdateItemInfo( m_InvTooltip, entity );

		if ( m_InvTooltipPreview )
		{
			m_InvTooltipPreview.SetItem( entity );
			m_InvTooltipPreview.SetView( entity.GetViewIndex() );
		}

		PlaceInventoryTooltip();
		m_InvTooltip.Show( true );
	}

	//! Put the tooltip beside the cursor and keep it on screen. Vanilla does the
	//! same clamping in ItemManager.PrepareTooltip; that path is unusable here
	//! because it lives on the inventory menu, which is not open.
	private void PlaceInventoryTooltip()
	{
		int screenW, screenH;
		GetScreenSize( screenW, screenH );

		float tw, th;
		m_InvTooltip.GetScreenSize( tw, th );

		// The cursor, not the coordinates the enter event carried: those are
		// relative to the row widget that was entered, so they were always a
		// handful of pixels and the tooltip parked itself in the top-left
		// corner of the screen.
		int mx, my;
		GetMousePos( mx, my );

		int x = mx + 18;
		int y = my + 18;

		if ( x + tw > screenW - 10 )
			x = screenW - 10 - tw;

		if ( y + th > screenH - 10 )
			y = screenH - 10 - th;

		// The layout root is declared with exact pos flags, but it is parented
		// to the window root here rather than to the inventory screen, so set
		// them explicitly rather than trusting an inherited default.
		m_InvTooltip.SetFlags( WidgetFlags.HEXACTPOS | WidgetFlags.VEXACTPOS, true );
		m_InvTooltip.SetPos( x, y );
	}

	private void HideInventoryTooltip()
	{
		if ( m_InvTooltip )
			m_InvTooltip.Show( false );

		m_InvHoveredIndex = -1;

		// Only the spare copy is ours to delete - the cells own theirs.
		DestroyLocalPreview( m_InvHoverEntity );
		m_InvHoverEntity = NULL;
		m_InvHoverType   = "";
	}

	//! Release every cell's preview entity. They are real objects in the world,
	//! invisible and simulation-disabled but real, so a form that comes and goes
	//! all session cannot be allowed to leak one per item per open.
	private void DestroyInventoryCellEntities()
	{
		if ( !m_InvCellEntities )
			return;

		foreach ( EntityAI entity : m_InvCellEntities )
			DestroyLocalPreview( entity );

		m_InvCellEntities.Clear();
		m_InvCellTypes.Clear();
	}

	//! Rebuilt on every open: the entries and their enabled state depend on both
	//! the row that was hit and the permissions the admin actually holds.
	private void ShowInventoryContextMenu()
	{
		if ( !m_InventoryMenu )
			return;

		int row = m_InvSelectedIndex;
		if ( row < 0 || row >= m_InventoryItems.Count() )
			return;

		JMPlayerInventoryItem menuItem = m_InventoryItems[row];

		m_InventoryMenu.ClearItems();
		m_InventoryMenu.AddItem( INV_MENU_INSPECT, "#STR_COT_PLAYER_MODULE_INV_INSPECT", JMConstants.Lucide( "eye" ) );
		m_InventoryMenu.AddItem( INV_MENU_TAKE,   "#STR_COT_PLAYER_MODULE_INV_TAKE",   JMConstants.Lucide( "hand" ) );
		m_InventoryMenu.AddItem( INV_MENU_HEALTH, "#STR_COT_PLAYER_MODULE_INV_SET_HEALTH", JMConstants.Lucide( "heart-pulse" ) );

		// The item-specific block, between the operations every row has and the
		// destructive one at the bottom. Listed only when the item can take
		// them: an entry is here because of what the item IS, so a greyed
		// "Unjam" on a pair of boots would say nothing an admin could act on.
		bool canModify = GetPermissionsManager().HasPermission( "Admin.Player.Inventory.Modify" );

		if ( menuItem.IsJammed() )
			m_InventoryMenu.AddItem( INV_MENU_UNJAM, "#STR_COT_PLAYER_MODULE_INV_UNJAM", JMConstants.Lucide( "unlink" ) );

		if ( menuItem.HasQuantity() )
			m_InventoryMenu.AddItem( INV_MENU_QUANTITY, "#STR_COT_PLAYER_MODULE_INV_SET_QUANTITY", JMConstants.Lucide( "gauge" ) );

		// Temperature is only meaningful on the two kinds of item the game
		// itself lets it matter for - the same pair the object spawner enables
		// its own temperature slider for.
		if ( menuItem.IsLiquidContainer() || menuItem.HasFoodStage() )
			m_InventoryMenu.AddItem( INV_MENU_TEMPERATURE, "#STR_COT_PLAYER_MODULE_INV_SET_TEMPERATURE", JMConstants.Lucide( "thermometer" ) );

		if ( menuItem.HasFoodStage() )
			m_InventoryMenu.AddItem( INV_MENU_STATE, "#STR_COT_PLAYER_MODULE_INV_SET_STATE", JMConstants.Lucide( "flame" ) );

		if ( menuItem.IsLiquidContainer() )
			m_InventoryMenu.AddItem( INV_MENU_LIQUID, "#STR_COT_PLAYER_MODULE_INV_SET_LIQUID", JMConstants.Lucide( "droplet" ) );

		m_InventoryMenu.AddItem( INV_MENU_DELETE, "#STR_COT_PLAYER_MODULE_INV_DELETE", JMConstants.ICON_TRASH_CAN, JMTheme.DANGER );

		// Greyed rather than dropped, so the menu keeps the same shape whoever
		// opens it and a missing permission reads as a permission problem.
		m_InventoryMenu.SetItemEnabled( INV_MENU_TAKE,   GetPermissionsManager().HasPermission( "Admin.Player.Inventory.Take" ) );
		m_InventoryMenu.SetItemEnabled( INV_MENU_DELETE, GetPermissionsManager().HasPermission( "Admin.Player.Inventory.Delete" ) );

		m_InventoryMenu.SetItemEnabled( INV_MENU_HEALTH,      canModify );
		m_InventoryMenu.SetItemEnabled( INV_MENU_UNJAM,       canModify );
		m_InventoryMenu.SetItemEnabled( INV_MENU_QUANTITY,    canModify );
		m_InventoryMenu.SetItemEnabled( INV_MENU_TEMPERATURE, canModify );
		m_InventoryMenu.SetItemEnabled( INV_MENU_STATE,       canModify );
		m_InventoryMenu.SetItemEnabled( INV_MENU_LIQUID,      canModify );

		int menuX, menuY;
		GetMousePos( menuX, menuY );
		m_InventoryMenu.ShowAt( menuX, menuY );
	}

	void OnClick_InventoryMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_SelectedInstance )
			return;

		int row = m_InvSelectedIndex;
		if ( row < 0 || row >= m_InventoryItems.Count() )
			return;

		string id = m_InventoryMenu.GetLastClickedId();
		JMPlayerInventoryItem item = m_InventoryItems[row];

		if ( id == INV_MENU_INSPECT )
		{
			InspectInventoryItem( row, item );
			return;
		}

		if ( id == INV_MENU_TAKE )
		{
			m_Module.InventoryTake( m_SelectedInstance.GetGUID(), item.NetIdLow, item.NetIdHigh );
			return;
		}

		if ( id == INV_MENU_DELETE )
		{
			// The confirmation is asynchronous, so remember which row it was
			// about - the table can be rebuilt before the answer comes back.
			m_InventoryPendingRow = row;
			CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_PLAYER_MODULE_INV_DELETE_CONFIRM_TITLE", string.Format( Widget.TranslateString( "#STR_COT_PLAYER_MODULE_INV_DELETE_CONFIRM_BODY" ), item.GetDisplayName() ), "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "InventoryDeleteConfirm" );
			return;
		}

		// Unjam is the one item edit with nothing to ask: there is exactly one
		// thing the admin can mean by it, so it goes straight out.
		if ( id == INV_MENU_UNJAM )
		{
			m_Module.InventoryModify( m_SelectedInstance.GetGUID(), item.NetIdLow, item.NetIdHigh, JMInventoryModifyOp.UNJAM, 0 );
			return;
		}

		if ( !m_InventoryPrompt )
			return;

		// Every prompt below is answered later, so the item is pinned by
		// network ID now - see m_InvPromptNetLow.
		m_InvPromptNetLow  = item.NetIdLow;
		m_InvPromptNetHigh = item.NetIdHigh;

		if ( id == INV_MENU_QUANTITY )
		{
			ShowInventoryQuantityPrompt( item );
			return;
		}

		if ( id == INV_MENU_HEALTH )
		{
			ShowInventoryHealthPrompt( item );
			return;
		}

		if ( id == INV_MENU_TEMPERATURE )
		{
			ShowInventoryTemperaturePrompt( item );
			return;
		}

		if ( id == INV_MENU_STATE )
		{
			ShowInventoryStatePrompt( item );
			return;
		}

		if ( id == INV_MENU_LIQUID )
		{
			ShowInventoryLiquidPrompt( item );
		}
	}

	//! Rounds for a magazine, quantity for everything else. Whole steps either
	//! way: half a bandage is not a thing an admin ever means to set.
	private void ShowInventoryQuantityPrompt( JMPlayerInventoryItem item )
	{
		string format = "%1";
		if ( item.QuantityMax > 0 )
			format = "%1 / " + Math.Round( item.QuantityMax ).ToString();

		m_InventoryPrompt.ShowSlider( INV_MENU_QUANTITY, "#STR_COT_PLAYER_MODULE_INV_SET_QUANTITY", item.GetDisplayName(), item.QuantityMin, item.QuantityMax, item.Quantity, 1, format );
	}

	//! Same range and the same degree format the object spawner's temperature
	//! slider uses, so the two read alike.
	private void ShowInventoryTemperaturePrompt( JMPlayerInventoryItem item )
	{
		m_InventoryPrompt.ShowSlider( INV_MENU_TEMPERATURE, "#STR_COT_PLAYER_MODULE_INV_SET_TEMPERATURE", item.GetDisplayName(), GameConstants.STATE_COLD_LVL_FOUR, GameConstants.STATE_HOT_LVL_FOUR, item.Temperature, 1, "#STR_COT_FORMAT_DEGREE", UIActionValuePrompt.SLIDER_COLOR_TEMPERATURE );
	}

	//! Replaces the old one-shot "repair to full" action: an admin can now set
	//! any health, not only max, coloured the same way the condition dot and
	//! the object spawner's own health slider are.
	//!
	//! Range is the ITEM's own MaxHealth, not a flat 0-100 - most carried items
	//! happen to top out at 100, but not all of them do, and a slider that
	//! could not reach an item's real ceiling would silently cap it below full.
	private void ShowInventoryHealthPrompt( JMPlayerInventoryItem item )
	{
		float max = item.MaxHealth;
		if ( max <= 0 )
			max = 100;

		m_InventoryPrompt.ShowSlider( INV_MENU_HEALTH, "#STR_COT_PLAYER_MODULE_INV_SET_HEALTH", item.GetDisplayName(), 0, max, item.Health, 1, "%1", UIActionValuePrompt.SLIDER_COLOR_HEALTH );
	}

	//! Only the stages this item's own config defines.
	//!
	//! The full enum would offer a boiled steak a "dried" it has no visual,
	//! nutrition or transition data for, and the server would take it: nothing
	//! below ChangeFoodStage checks that the stage was ever configured.
	private void ShowInventoryStatePrompt( JMPlayerInventoryItem item )
	{
		array<string> labels = {};
		array<int>    values = {};

		int stage;
		for ( stage = FoodStageType.RAW; stage < FoodStageType.COUNT; stage++ )
		{
			string stageName = FoodStage.GetFoodStageName( stage );
			if ( stageName == "" )
				continue;

			if ( !GetGame().ConfigIsExisting( "CfgVehicles " + item.Type + " Food FoodStages " + stageName ) )
				continue;

			labels.Insert( FoodStageLabel( stage ) );
			values.Insert( stage );
		}

		if ( labels.Count() == 0 )
			return;

		m_InventoryPrompt.ShowOptions( INV_MENU_STATE, "#STR_COT_PLAYER_MODULE_INV_SET_STATE", item.GetDisplayName(), labels, values, item.Stage );
	}

	//! Only the liquids this container's config accepts.
	//!
	//! The list is built from the game's own liquid definitions rather than
	//! from a hard-coded table, so a mod that adds one gets it for free, and
	//! filtered by the container's mask because the server rejects anything
	//! outside it anyway.
	private void ShowInventoryLiquidPrompt( JMPlayerInventoryItem item )
	{
		int mask = item.GetLiquidContainerMask();
		if ( mask == 0 )
			return;

		array<string> labels = {};
		array<int>    values = {};

		foreach ( int liquidType, LiquidInfo info : Liquid.m_LiquidInfosByType )
		{
			if ( liquidType == LIQUID_NONE )
				continue;

			if ( ( liquidType & mask ) == 0 )
				continue;

			string label = info.m_LiquidDisplayName;
			if ( label == "" )
				label = info.m_LiquidClassName;

			labels.Insert( label );
			values.Insert( liquidType );
		}

		if ( labels.Count() == 0 )
			return;

		m_InventoryPrompt.ShowOptions( INV_MENU_LIQUID, "#STR_COT_PLAYER_MODULE_INV_SET_LIQUID", item.GetDisplayName(), labels, values, item.LiquidType );
	}

	private string FoodStageLabel( int stage )
	{
		switch ( stage )
		{
		case FoodStageType.RAW:    return "#STR_COT_PLAYER_MODULE_INV_STAGE_RAW";
		case FoodStageType.BAKED:  return "#STR_COT_PLAYER_MODULE_INV_STAGE_BAKED";
		case FoodStageType.BOILED: return "#STR_COT_PLAYER_MODULE_INV_STAGE_BOILED";
		case FoodStageType.DRIED:  return "#STR_COT_PLAYER_MODULE_INV_STAGE_DRIED";
		case FoodStageType.BURNED: return "#STR_COT_PLAYER_MODULE_INV_STAGE_BURNED";
		case FoodStageType.ROTTEN: return "#STR_COT_PLAYER_MODULE_INV_STAGE_ROTTEN";
		}

		return FoodStage.GetFoodStageName( stage );
	}

	//! One callback for all four prompts; which one answered is the id it was
	//! raised with. The item is whatever the network ID still resolves to on
	//! the server - the listing on screen may have moved on, and the server
	//! re-checks the item against the op before applying it either way.
	void OnConfirm_InventoryPrompt( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_SelectedInstance || !m_InventoryPrompt )
			return;

		string id = m_InventoryPrompt.GetPromptId();

		if ( id == INV_MENU_QUANTITY )
		{
			m_Module.InventoryModify( m_SelectedInstance.GetGUID(), m_InvPromptNetLow, m_InvPromptNetHigh, JMInventoryModifyOp.QUANTITY, m_InventoryPrompt.GetSliderValue() );
			return;
		}

		if ( id == INV_MENU_HEALTH )
		{
			m_Module.InventoryModify( m_SelectedInstance.GetGUID(), m_InvPromptNetLow, m_InvPromptNetHigh, JMInventoryModifyOp.HEALTH, m_InventoryPrompt.GetSliderValue() );
			return;
		}

		if ( id == INV_MENU_TEMPERATURE )
		{
			m_Module.InventoryModify( m_SelectedInstance.GetGUID(), m_InvPromptNetLow, m_InvPromptNetHigh, JMInventoryModifyOp.TEMPERATURE, m_InventoryPrompt.GetSliderValue() );
			return;
		}

		if ( id == INV_MENU_STATE )
		{
			m_Module.InventoryModify( m_SelectedInstance.GetGUID(), m_InvPromptNetLow, m_InvPromptNetHigh, JMInventoryModifyOp.FOOD_STAGE, m_InventoryPrompt.GetSelectedValue() );
			return;
		}

		if ( id == INV_MENU_LIQUID )
		{
			m_Module.InventoryModify( m_SelectedInstance.GetGUID(), m_InvPromptNetLow, m_InvPromptNetHigh, JMInventoryModifyOp.LIQUID_TYPE, m_InventoryPrompt.GetSelectedValue() );
		}
	}

	//! Open DayZ's own inspect screen on a local copy of the item.
	//!
	//! MENU_INSPECT is an ordinary UIScriptedMenu and COT is not a menu at all -
	//! it draws straight onto the workspace - so entering it stacks on top of
	//! the COT window and its own close button returns the admin here.
	private void InspectInventoryItem( int row, JMPlayerInventoryItem item )
	{
		// The tooltip and the inspect screen would otherwise be looking at the
		// same entity, and closing the tooltip deletes it out from under the
		// menu.
		HideInventoryTooltip();

		CloseInventoryInspect();

		m_InvInspectEntity = SpawnLocalPreview( item );

		if ( !m_InvInspectEntity )
			return;

		BuildLocalPreviewChildren( row, m_InvInspectEntity );

		// Only now: a weapon with its simulation already off never seats the
		// magazine that was just handed to it.
		FinishLocalPreview( m_InvInspectEntity );

		InspectMenuNew inspect = InspectMenuNew.Cast( g_Game.GetUIManager().EnterScriptedMenu( MENU_INSPECT, NULL ) );
		if ( !inspect )
		{
			DestroyLocalPreview( m_InvInspectEntity );
			m_InvInspectEntity = NULL;
			return;
		}

		inspect.SetItem( m_InvInspectEntity );

		m_InvInspectOpen = true;
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( WatchInventoryInspect, 250, true );
	}

	//! The inspect screen is entered without a parent menu, so nothing in the
	//! vanilla chain tears it down for us: the menu blur is left on the world
	//! after it closes and the local copy would leak. Poll for the menu going
	//! away and clean up behind it.
	private void WatchInventoryInspect()
	{
		if ( !m_InvInspectOpen )
			return;

		if ( g_Game.GetUIManager().FindMenu( MENU_INSPECT ) )
			return;

		CloseInventoryInspect();

		// UIScriptedMenu.UnlockControls hands the cursor back to a PARENT MENU
		// and hides it when there is not one. COT is drawn straight onto the
		// workspace and is not a menu at all, so closing the inspect screen
		// leaves the admin with a COT window up and no pointer until something
		// else happens to re-show it.
		RestoreCursorAfterMenu();
	}

	//! Put the pointer back after a vanilla menu closed over the top of COT.
	//! Only the cursor: the menu balanced its own ChangeGameFocus on the way
	//! out, so COT's focus count is still whatever opening the window set.
	private void RestoreCursorAfterMenu()
	{
		if ( !m_IsShown )
			return;

		g_Game.GetUIManager().ShowUICursor( true );
	}

	private void CloseInventoryInspect()
	{
		if ( m_InvInspectOpen )
		{
			g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( WatchInventoryInspect );

			// Entered with no parent menu, so the engine never clears the menu
			// blur on the way out and the world stays smeared.
			PPEffects.SetBlurMenu( 0 );

			m_InvInspectOpen = false;
		}

		DestroyLocalPreview( m_InvInspectEntity );
		m_InvInspectEntity = NULL;
	}

	//! Everything known about one item, as a block of lines. No longer used by
	//! the inspect action - that opens the vanilla screen now - but kept as the
	//! text form of an entry, and it is the only place the classname is spelt
	//! out, which is what an admin needs for a types.xml entry or a spawn
	//! command.
	private string BuildInspectText( JMPlayerInventoryItem item )
	{
		string text = item.GetDisplayName();
		text = text + "\n" + item.Type;
		text = text + "\n";
		text = text + "\n" + Widget.TranslateString( "#STR_COT_PLAYER_MODULE_INV_COL_SLOT" ) + ": " + Widget.TranslateString( item.GetLocationLabel() );

		int health = Math.Round( item.Health );
		text = text + "\n" + Widget.TranslateString( "#STR_COT_PLAYER_MODULE_INV_COL_HP" ) + ": " + health.ToString();
		text = text + "\n" + Widget.TranslateString( "#STR_COT_PLAYER_MODULE_INV_COL_QTY" ) + ": " + item.GetQuantityLabel();

		if ( item.IsRuined() )
			text = text + "\n" + Widget.TranslateString( "#STR_COT_PLAYER_MODULE_INV_RUINED" );

		return text;
	}

	void InventoryDeleteConfirm( JMConfirmation confirmation = NULL )
	{
		if ( !m_SelectedInstance )
			return;

		if ( m_InventoryPendingRow < 0 || m_InventoryPendingRow >= m_InventoryItems.Count() )
			return;

		JMPlayerInventoryItem item = m_InventoryItems[m_InventoryPendingRow];
		m_InventoryPendingRow = -1;

		m_Module.InventoryDelete( m_SelectedInstance.GetGUID(), item.NetIdLow, item.NetIdHigh );
	}

	//! Statistics tab: session history on top, combat below.
	//!
	//! The two halves come from different places and different permissions -
	//! session history from this module, kill stats from the anti-cheat module
	//! behind Admin.AntiCheat.View - so they refresh independently and either can
	//! be empty on its own.
	private Widget InitActionWidgetsStatistics( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 12, 1 );

		// Refresh acts on both cards below it, so it sits in the session card
		// title bar - the same place the Position and Inventory cards put
		// theirs, spelled with the same glyph. It used to be a loose icon inside
		// the card body nudged to x=0.9, which cost a row of height and drifted
		// against the values column as the form was resized.
		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_SESSION" );
		Widget section0 = section0Card.GetContent();
		m_StatsRefreshButton = section0Card.AddRefreshButton( this, "Click_RefreshStatistics" );
		m_StatsRefreshButton.SetFixedSize( HEADER_ACTION_PX, HEADER_ACTION_PX );
		m_StatsRefreshButton.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_REFRESH_STATISTICS" );

		m_StatsSession = UIActionManager.CreateKeyValueList( section0 );

		UIActionManager.CreatePanel( section0, 0x00000000, 10 );
		UIActionCard section1Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_COMBAT" );
		Widget section1 = section1Card.GetContent();
		m_StatsCombat = UIActionManager.CreateKeyValueList( section1 );

		// The bar is a summary of the two rows above it, so it gets its own
		// breathing room instead of reading as a fifth table row.
		UIActionManager.CreatePanel( section1, 0x00000000, 8 );
		m_StatsHeadshotBar = UIActionManager.CreateProgressBar( section1, "#STR_COT_PLAYER_MODULE_STAT_HEADSHOT_RATIO", 0 );

		// Shown instead of the combat rows when the admin lacks the anti-cheat
		// permission. A silently empty section reads as "this player has never
		// fired a shot", which is a different and wrong answer.
		m_StatsCombatNotice = UIActionManager.CreateText( section1, "", "" );

	#ifdef DZ_Expansion_Core
		UIActionManager.CreatePanel( parent, 0x00000000, 10 );
		UIActionCard section2Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_EXPANSION" );
		m_StatsExpansion = UIActionManager.CreateKeyValueList( section2Card.GetContent() );

		// Hidden until the server answers with at least one row: Expansion being
		// COMPILED against is not the same as any of its features being on, and
		// an empty card would claim the player has no money, faction, reputation
		// or group when the truth is that nothing asked.
		m_StatsExpansionRoot = section2Card.GetLayoutRoot();
		if ( m_StatsExpansionRoot )
			m_StatsExpansionRoot.Show( false );
	#endif

		return parent;
	}

	void Click_RefreshStatistics( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		SpinRefreshIcon( action );
		RequestStatistics();
	}

	//! Pull both halves. Called on tab entry, on a selection change while the tab
	//! is up, and never on a timer.
	private void RequestStatistics()
	{
		if ( !m_Module || !m_SelectedInstance )
			return;

		if ( GetPermissionsManager().HasPermission( "Admin.Player.Statistics.View" ) )
			m_Module.RequestPlayerStats( m_SelectedInstance.GetGUID() );

	#ifdef DZ_Expansion_Core
		//! Same permission and the same one-shot request as the session block -
		//! none of it is on a timer.
		if ( GetPermissionsManager().HasPermission( "Admin.Player.Statistics.View" ) )
			m_Module.RequestExpansionInfo( m_SelectedInstance.GetGUID() );
	#endif

		RequestKillStats();
		RefreshStatisticsPanel();
	}

	//! Ask the anti-cheat module for its kill table.
	//!
	//! On a listen host RequestFlags() returns without sending anything - there
	//! is no wire to send over - so the server-side map is read directly instead.
	//! Only the request differs; both paths end up in RefreshStatisticsPanel.
	private void RequestKillStats()
	{
		if ( !m_AntiCheatModule )
			Class.CastTo( m_AntiCheatModule, GetModuleManager().GetModule( JMAntiCheatModule ) );

		if ( !m_AntiCheatModule )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.AntiCheat.View" ) )
			return;

		if ( !IsMissionHost() )
			m_AntiCheatModule.RequestFlags();
	}

	//! The Expansion block came back. Declared unconditionally - the module
	//! calls it from an RPC handler that is compiled either way - but it only
	//! has anywhere to put the rows on an Expansion build.
	void OnExpansionInfoUpdated( string guid, array< string > ids, array< string > values )
	{
	#ifdef DZ_Expansion_Core
		if ( !m_SelectedInstance || guid != m_SelectedInstance.GetGUID() )
			return;

		if ( !m_StatsExpansion || !ids || !values )
			return;

		m_StatsExpansion.Clear();

		int count = ids.Count();
		if ( values.Count() < count )
			count = values.Count();

		for ( int i = 0; i < count; i++ )
			m_StatsExpansion.SetValue( ExpansionInfoLabel( ids[i] ), values[i], ExpansionInfoIcon( ids[i] ) );

		if ( m_StatsExpansionRoot )
			m_StatsExpansionRoot.Show( count > 0 );

		if ( m_ActionListScroller )
			m_ActionListScroller.UpdateScroller();
	#endif
	}

#ifdef DZ_Expansion_Core
	//! Row ids are resolved to a label and a glyph HERE rather than sent as
	//! text, so the server never decides what language the admin reads and an
	//! id the client does not recognise still renders as itself.
	private string ExpansionInfoLabel( string id )
	{
		if ( id == JMPlayerModule.EXP_INFO_MONEY )
			return "#STR_COT_PLAYER_MODULE_STAT_EXP_MONEY";

		if ( id == JMPlayerModule.EXP_INFO_FACTION )
			return "#STR_COT_PLAYER_MODULE_STAT_EXP_FACTION";

		if ( id == JMPlayerModule.EXP_INFO_REPUTATION )
			return "#STR_COT_PLAYER_MODULE_STAT_EXP_REPUTATION";

		if ( id == JMPlayerModule.EXP_INFO_GROUP )
			return "#STR_COT_PLAYER_MODULE_STAT_EXP_GROUP";

		return id;
	}

	private string ExpansionInfoIcon( string id )
	{
		if ( id == JMPlayerModule.EXP_INFO_MONEY )
			return JMConstants.Lucide( "banknote" );

		if ( id == JMPlayerModule.EXP_INFO_FACTION )
			return JMConstants.Lucide( "flag" );

		if ( id == JMPlayerModule.EXP_INFO_REPUTATION )
			return JMConstants.Lucide( "star" );

		if ( id == JMPlayerModule.EXP_INFO_GROUP )
			return JMConstants.Lucide( "users" );

		return JMConstants.Lucide( "info" );
	}
#endif

	void OnPlayerStatsUpdated( string guid, JMPlayerStats stats )
	{
		if ( !m_SelectedInstance || guid != m_SelectedInstance.GetGUID() )
			return;

		m_PlayerStats = stats;
		RefreshStatisticsPanel();
	}

	private void RefreshStatisticsPanel()
	{
		RefreshSessionStats();
		RefreshCombatStats();
	}

	private void RefreshSessionStats()
	{
		if ( !m_StatsSession )
			return;

		// Every row is written whether or not there is data behind it. A section
		// that renders a different NUMBER of rows depending on what arrived reads
		// as a broken panel; one that renders "-" reads as "nothing recorded",
		// which is the actual answer for a player whose history starts today.
		string playtime    = "-";
		string sessions    = "-";
		string firstSeen   = "-";
		string lastSeen    = "-";
		string deaths      = "-";
		string longestLife = "-";
		string currentLife = "-";

		if ( m_PlayerStats )
		{
			playtime    = JMPlayerStats.FormatDuration( m_PlayerStats.TotalPlaytimeSec );
			sessions    = m_PlayerStats.SessionCount.ToString();
			firstSeen   = JMPlayerStats.FormatSince( m_PlayerStats.FirstSeenUnix );
			lastSeen    = JMPlayerStats.FormatSince( m_PlayerStats.LastSeenUnix );
			deaths      = m_PlayerStats.Deaths.ToString();
			longestLife = JMPlayerStats.FormatDuration( m_PlayerStats.LongestLifeSec );
			currentLife = JMPlayerStats.FormatDuration( m_PlayerStats.GetCurrentLifeSeconds() );
		}

		// The icon is only read the first time a key is seen, so passing it on
		// every refresh costs nothing.
		m_StatsSession.SetValue( "#STR_COT_PLAYER_MODULE_STAT_PLAYTIME", playtime, JMConstants.Lucide( "hourglass" ) );
		m_StatsSession.SetValue( "#STR_COT_PLAYER_MODULE_STAT_SESSIONS", sessions, JMConstants.Lucide( "log-in" ) );
		m_StatsSession.SetValue( "#STR_COT_PLAYER_MODULE_STAT_FIRSTSEEN", firstSeen, JMConstants.Lucide( "calendar-plus" ) );
		m_StatsSession.SetValue( "#STR_COT_PLAYER_MODULE_STAT_LASTSEEN", lastSeen, JMConstants.Lucide( "calendar-check" ) );
		m_StatsSession.SetValue( "#STR_COT_PLAYER_MODULE_STAT_DEATHS", deaths, JMConstants.Lucide( "skull" ) );
		m_StatsSession.SetValue( "#STR_COT_PLAYER_MODULE_STAT_LONGEST_LIFE", longestLife, JMConstants.Lucide( "heart-pulse" ) );
		m_StatsSession.SetValue( "#STR_COT_PLAYER_MODULE_STAT_CURRENT_LIFE", currentLife, JMConstants.Lucide( "heart" ) );
	}

	private void RefreshCombatStats()
	{
		if ( !m_StatsCombat || !m_SelectedInstance )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.AntiCheat.View" ) )
		{
			if ( m_StatsCombatNotice )
				m_StatsCombatNotice.SetText( "#STR_COT_PLAYER_MODULE_STAT_NO_PERMISSION" );
			return;
		}

		JMAntiCheatKillStats stats = GetSelectedKillStats();

		string kills   = "-";
		string hitStr  = "-";
		string zones   = "-";
		string avgStr  = "-";
		string maxStr  = "-";
		float headshotRatio = 0;

		if ( stats )
		{
			kills = stats.TotalKills.ToString();

			// Math.Round returns a float, so rounding straight into ToString
			// gives "50.0%". Land it in an int first.
			float hit = stats.GetHitRatio();
			if ( hit >= 0 )
			{
				int hitPercent = Math.Round( hit * 100 );
				hitStr = hitPercent.ToString() + "%";
			}

			zones = stats.BodyHead.ToString() + " / " + stats.BodyTorso.ToString();

			int avgDist = Math.Round( stats.GetAvgDistance() );
			int maxDist = Math.Round( stats.DistanceMax );
			avgStr = avgDist.ToString() + "m";
			maxStr = maxDist.ToString() + "m";

			if ( stats.TotalKills > 0 )
				headshotRatio = stats.BodyHead / (float)stats.TotalKills;
		}

		if ( m_StatsCombatNotice )
		{
			if ( stats )
				m_StatsCombatNotice.SetText( "" );
			else
				m_StatsCombatNotice.SetText( "#STR_COT_PLAYER_MODULE_STAT_NO_COMBAT" );
		}

		m_StatsCombat.SetValue( "#STR_COT_PLAYER_MODULE_STAT_KILLS", kills, JMConstants.Lucide( "swords" ) );
		m_StatsCombat.SetValue( "#STR_COT_PLAYER_MODULE_STAT_HITRATIO", hitStr, JMConstants.Lucide( "target" ) );
		m_StatsCombat.SetValue( "#STR_COT_PLAYER_MODULE_STAT_HEADSHOTS", zones, JMConstants.Lucide( "scan-face" ) );
		m_StatsCombat.SetValue( "#STR_COT_PLAYER_MODULE_STAT_AVGDIST", avgStr, JMConstants.Lucide( "ruler" ) );
		m_StatsCombat.SetValue( "#STR_COT_PLAYER_MODULE_STAT_MAXDIST", maxStr, JMConstants.Lucide( "ruler-dimension-line" ) );

		if ( m_StatsHeadshotBar )
			m_StatsHeadshotBar.SetProgress( headshotRatio );
	}

	//! Kill stats for the selected player, from whichever side of the wire has
	//! them. RequestFlags never fills the client cache on a listen host.
	private JMAntiCheatKillStats GetSelectedKillStats()
	{
		if ( !m_AntiCheatModule || !m_SelectedInstance )
			return NULL;

		string guid = m_SelectedInstance.GetGUID();

		map< string, ref JMAntiCheatKillStats > source = m_AntiCheatModule.GetClientKillStats();
		if ( IsMissionHost() )
			source = m_AntiCheatModule.GetServerKillStats();

		if ( !source || !source.Contains( guid ) )
			return NULL;

		return source.Get( guid );
	}

	//! Built eagerly with the rest of the tab (see InitActionWidgetsMap), but
	//! kept hidden until the Position tab is actually up - a MapWidget left
	//! visible behind another tab keeps rendering.
	private void OnEnterPositionTab()
	{
		if ( m_PositionMap )
			m_PositionMap.Show();

		RebuildTeleportHistory();

		RecenterMap();

		//! First open only: CenterOn lands before the widget has been laid out
		//! on the very first show and is discarded - the map came up on the
		//! world origin and stayed there until the recentre button was pressed
		//! by hand. Every later visit re-enters an already-laid-out map, which
		//! is why this only ever looked broken the first time.
		if ( !m_PositionMapCentered )
		{
			m_PositionMapCentered = true;
			g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( RecenterMap, 100, false );
		}
	}

	private void InitActionWidgetsMap( Widget parent )
	{
		m_PositionMap = UIActionManager.CreateMap( parent, this, "OnClick_PositionMap", MAP_HEIGHT );

		if ( !m_PositionMap )
			return;

		m_PositionMap.SetScale( MAP_DEFAULT_SCALE );
		m_PositionMap.Hide();
	}

	//! Put the view back on the selected player. Only ever called on a selection
	//! change, on entering the tab, or from the recenter button - never from the
	//! marker refresh, or the admin could not pan or zoom at all.
	private void RecenterMap()
	{
		if ( !m_PositionMap || !m_SelectedInstance )
			return;

		// A remote player's position is only as fresh as the last client refresh,
		// and before the first one it is the world origin - which is open sea,
		// so the map would come up as a flat blue square and read as broken.
		// Fall back to the admin's own position until the target has a real one.
		vector center = m_SelectedInstance.GetPosition();
		if ( center == vector.Zero )
		{
			JMPlayerInstance self = GetPermissionsManager().GetClientPlayer();
			if ( self )
				center = self.GetPosition();
		}

		m_PositionMap.CenterOn( center, MAP_DEFAULT_SCALE );
	}

	//! Draw the selected player and the admin, and nothing else. The COT map
	//! module already exists for a whole-server view; sixty marks a second here
	//! would only duplicate it.
	//!
	//! Both live under stable ids and get moved in place rather than cleared
	//! and redrawn every poll - UIActionMap.AddMarker replaces the marker
	//! already under an id, so this only ever touches its own two widgets.
	private void UpdateMapMarkers()
	{
		if ( !m_PositionMap || !m_SelectedInstance )
			return;

		if ( !IsTabActive( TAB_POSITION ) )
			return;

		JMPlayerInstance self = GetPermissionsManager().GetClientPlayer();

		//! Looking at your own row is ONE player, so it gets one mark. Drawing
		//! both put the admin name and "You" on top of each other at the same
		//! coordinate, which read as two players standing in the same spot.
		bool targetIsSelf = false;
		if ( self )
			targetIsSelf = ( self.GetGUID() == m_SelectedInstance.GetGUID() );

		if ( !targetIsSelf )
			m_PositionMap.AddMarker( "position_target", m_SelectedInstance.GetPosition(), m_SelectedInstance.GetName(), MAP_MARK_TARGET, UIActionMap.ICON_DOT, "position" );
		else
			m_PositionMap.RemoveMarker( "position_target" );

		if ( self )
			m_PositionMap.AddMarker( "position_self", self.GetPosition(), "#STR_COT_PLAYER_MODULE_MAP_YOU", MAP_MARK_SELF, UIActionMap.ICON_DOT, "position" );

		GetCommunityOnlineTools().RefreshClientPositions();
	}

	//! Ground clicks: a double press fills the coordinate fields and runs the
	//! ordinary teleport-to-coordinates path, confirmation dialog included.
	void OnClick_PositionMap( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.DOUBLE_CLICK )
			return;

		if ( !m_PositionMap )
			return;

		vector target = m_PositionMap.GetLastClickWorldPos();

		m_PositionX.SetText( target[0].ToString() );
		m_PositionY.SetText( target[1].ToString() );
		m_PositionZ.SetText( target[2].ToString() );

		Click_SetPosition( UIEvent.CLICK, NULL );
	}

	void Click_RecenterMap( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		RecenterMap();
	}

	void OnClick_PlayerPrefSave( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateConfirmation_Three( JMConfirmationType.INFO, "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_HEADER", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_SAVE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_GROUP 01", "OnClick_SavePlayerListPref01", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_GROUP 02", "OnClick_SavePlayerListPref02" );
	}

	void OnClick_SavePlayerListPref01()
	{
		m_PlayersPref1 = new TStringArray;
		SavePlayerListPref(m_PlayersPref1);
		
		if (m_PlayersPref1.Count() == 0 && m_PlayersPref2.Count() == 0)
			m_PlayerPrefLoad.Disable();
		else
			m_PlayerPrefLoad.Enable();
	}

	void OnClick_SavePlayerListPref02()
	{
		m_PlayersPref2 = new TStringArray;
		SavePlayerListPref(m_PlayersPref2);
		
		if (m_PlayersPref1.Count() == 0 && m_PlayersPref2.Count() == 0)
			m_PlayerPrefLoad.Disable();
		else
			m_PlayerPrefLoad.Enable();
	}

	void OnClick_PlayerPrefLoad( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateConfirmation_Three( JMConfirmationType.INFO, "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_HEADER", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_LOAD_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_GROUP 01", "OnClick_LoadPlayerListPref01", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_GROUP 02", "OnClick_LoadPlayerListPref02" );
	}

	void OnClick_LoadPlayerListPref01()
	{
		LoadPlayerListPref(m_PlayersPref1);
	}

	void OnClick_LoadPlayerListPref02()
	{
		LoadPlayerListPref(m_PlayersPref2);
	}

	void SavePlayerListPref(out TStringArray playerArr)
	{
		foreach(JMPlayerRowWidget player: m_PlayerList)
		{
			if (player.IsChecked())
				playerArr.Insert(player.GetGUID());
		}
	}

	void LoadPlayerListPref(out TStringArray playerArr)
	{
		foreach(JMPlayerRowWidget player: m_PlayerList)
		{
			bool state = playerArr.Find(player.GetGUID()) != -1;
			
			if (state)
				JM_GetSelected().AddPlayer(player.GetGUID());
			else
				JM_GetSelected().RemovePlayer(player.GetGUID());
			
			player.SetChecked(state);
		}
		
		UpdateUI();
		UpdatePlayerCount();
	}

	//! Open the advanced-filter dropdown under the toolbar button.
	//!
	//! A context menu rather than a panel of checkboxes: the whole row - glyph
	//! and word together - is already one hit area here, which is exactly the
	//! behaviour asked for, and it floats over the list instead of costing the
	//! list a permanent strip of height it only needs while being changed.
	void OnClick_PlayerListFilters( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_FilterMenu )
		{
			m_FilterMenu = UIActionManager.CreateContextMenu( layoutRoot, m_Window.GetWidgetRoot(), this, "OnClick_FilterMenu" );
			RegisterOverlay( m_FilterMenu );

			if ( !m_FilterMenu )
				return;

			//! Every row here is a toggle, so a click is never "done with this
			//! menu". It closes on the button, on Escape, or on a click that
			//! lands anywhere outside the list.
			m_FilterMenu.SetCloseOnClick( false );

			if ( m_PlayerListFilters )
				m_FilterMenu.SetOwnerWidget( m_PlayerListFilters.GetLayoutRoot() );
		}

		//! Toggle: a second click on the button puts the menu away rather than
		//! reopening it in place, which is what a dropdown is expected to do.
		if ( m_FilterMenu.IsOpen() )
		{
			m_FilterMenu.Close();
			return;
		}

		RebuildFilterMenu();

		float fx, fy, fh, fw;
		m_PlayerListFilters.GetLayoutRoot().GetScreenPos( fx, fy );
		m_PlayerListFilters.GetLayoutRoot().GetScreenSize( fw, fh );

		m_FilterMenu.ShowAt( fx, fy + fh );
	}

	//! Rebuild every entry so each one carries its current state in its colour.
	//! An active filter is normal text; a disabled one is dimmed, which is the
	//! whole read-out - there is no separate tick, the row IS the checkbox.
	private void RebuildFilterMenu()
	{
		if ( !m_FilterMenu )
			return;

		m_FilterMenu.ClearItems();

		AddFilterItem( LIST_FILTER_DEAD,     "#STR_COT_PLAYER_MODULE_FILTER_SHOW_DEAD",     "skull",         m_FilterShowDead );
		AddFilterItem( LIST_FILTER_UNCON,    "#STR_COT_PLAYER_MODULE_FILTER_SHOW_UNCON",    "bed",           m_FilterShowUncon );
		AddFilterItem( LIST_FILTER_HURT,     "#STR_COT_PLAYER_MODULE_FILTER_SHOW_HURT",     "droplet",       m_FilterShowHurt );
		AddFilterItem( LIST_FILTER_SICK,     "#STR_COT_PLAYER_MODULE_FILTER_SHOW_SICK",     "thermometer",   m_FilterShowSick );
		AddFilterItem( LIST_FILTER_ADMINS,   "#STR_COT_PLAYER_MODULE_FILTER_SHOW_ADMINS",   "shield",        m_FilterShowAdmins );
		AddFilterItem( LIST_FILTER_CHEATERS, "#STR_COT_PLAYER_MODULE_FILTER_SHOW_CHEATERS", "flag",          m_FilterShowCheaters );
	}

	private void AddFilterItem( string id, string label, string icon, bool enabled )
	{
		m_FilterMenu.AddItem( id, label, JMConstants.Lucide( icon ), FilterItemColor( enabled ) );
	}

	//! Repaint the entries without touching the widget tree. Rebuilding rows
	//! here would destroy the very row the click landed on, and the engine
	//! answers a vanished press target by recentring the cursor.
	private void RefreshFilterMenuColors()
	{
		if ( !m_FilterMenu )
			return;

		m_FilterMenu.SetItemTextColor( LIST_FILTER_DEAD,     FilterItemColor( m_FilterShowDead ) );
		m_FilterMenu.SetItemTextColor( LIST_FILTER_UNCON,    FilterItemColor( m_FilterShowUncon ) );
		m_FilterMenu.SetItemTextColor( LIST_FILTER_HURT,     FilterItemColor( m_FilterShowHurt ) );
		m_FilterMenu.SetItemTextColor( LIST_FILTER_SICK,     FilterItemColor( m_FilterShowSick ) );
		m_FilterMenu.SetItemTextColor( LIST_FILTER_ADMINS,   FilterItemColor( m_FilterShowAdmins ) );
		m_FilterMenu.SetItemTextColor( LIST_FILTER_CHEATERS, FilterItemColor( m_FilterShowCheaters ) );
	}

	private int FilterItemColor( bool enabled )
	{
		if ( enabled )
			return JMTheme.TEXT_PRIMARY;

		return JMTheme.TEXT_DISABLED;
	}

	//! Flip the filter the clicked row stands for, repaint the menu so the new
	//! state is visible without closing it, and rebuild the list behind it.
	void OnClick_FilterMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_FilterMenu )
			return;

		string id = m_FilterMenu.GetLastClickedId();

		if ( id == LIST_FILTER_DEAD )
			m_FilterShowDead = !m_FilterShowDead;
		else if ( id == LIST_FILTER_UNCON )
			m_FilterShowUncon = !m_FilterShowUncon;
		else if ( id == LIST_FILTER_HURT )
			m_FilterShowHurt = !m_FilterShowHurt;
		else if ( id == LIST_FILTER_SICK )
			m_FilterShowSick = !m_FilterShowSick;
		else if ( id == LIST_FILTER_ADMINS )
			m_FilterShowAdmins = !m_FilterShowAdmins;
		else if ( id == LIST_FILTER_CHEATERS )
			m_FilterShowCheaters = !m_FilterShowCheaters;
		else
			return;

		RefreshFilterMenuColors();
		UpdatePlayerList();
	}

	//! Does this player survive the advanced filters? Dead is tested before the
	//! rest because a corpse is not separately unconscious or bleeding as far as
	//! the list is concerned - it is one row, and "show dead" owns it.
	private bool PassesListFilters( JMPlayerInstance player )
	{
		bool playerDead = player.IsDead();

		if ( playerDead )
			return m_FilterShowDead;

		if ( !m_FilterShowUncon && player.IsUnconscious() )
			return false;

		if ( !m_FilterShowHurt && ( player.IsBleeding() || player.HasBrokenLegs() ) )
			return false;

		if ( !m_FilterShowSick && player.IsSick() )
			return false;

		if ( !m_FilterShowAdmins && player.HasPermission( "COT" ) )
			return false;

		if ( !m_FilterShowCheaters && JMAntiCheatStatus.IsFlagged( player.GetGUID() ) )
			return false;

		return true;
	}

	void Event_UpdatePlayerList( UIEvent eid, UIActionBase action )
	{
		UpdatePlayerList();
	}

	void Event_DeSelectAllPlayerList( UIEvent eid, UIActionBase action )
	{
		SelectAllPlayerList(false);
	}

	void Event_SelectAllPlayerList( UIEvent eid, UIActionBase action )
	{
		SelectAllPlayerList(true);
	}	

	void Click_StripPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateConfirmation_Three( JMConfirmationType.INFO, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_CLEAR_INVENTORY", "", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_EVERYTHING", "StripConfirm", "#STR_COT_CARGO_ONLY", "ClearCargoConfirm" );
	}

	void StripConfirm(JMConfirmation confirmation = NULL)
	{
		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_CLEAR_INVENTORY", "StripMulti", "StripSingle", "StripSelf");
	}

	void StripMulti(JMConfirmation confirmation = NULL)
	{
		m_Module.Strip( JM_GetSelected().GetPlayers() );
	}

	void StripSingle(JMConfirmation confirmation = NULL)
	{
		m_Module.Strip( {JM_GetSelected().GetPlayersOrSelf()[0]} );
	}

	void StripSelf(JMConfirmation confirmation = NULL)
	{
		m_Module.Strip( {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

	void ClearCargoConfirm(JMConfirmation confirmation = NULL)
	{
		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_CLEAR_INVENTORY", "ClearCargoMulti", "ClearCargoSingle", "ClearCargoSelf");
	}

	void ClearCargoMulti(JMConfirmation confirmation = NULL)
	{
		m_Module.ClearCargo( JM_GetSelected().GetPlayers() );
	}

	void ClearCargoSingle(JMConfirmation confirmation = NULL)
	{
		m_Module.ClearCargo( {JM_GetSelected().GetPlayersOrSelf()[0]} );
	}

	void ClearCargoSelf(JMConfirmation confirmation = NULL)
	{
		m_Module.ClearCargo( {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

	void Click_DryPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_DRY", "DryMulti", "DrySingle", "DrySelf", false);
	}

	void DryMulti(JMConfirmation confirmation = NULL)
	{
		m_Module.Dry( JM_GetSelected().GetPlayers() );
	}

	void DrySingle(JMConfirmation confirmation = NULL)
	{
		m_Module.Dry( {JM_GetSelected().GetPlayersOrSelf()[0]} );
	}

	void DrySelf(JMConfirmation confirmation = NULL)
	{
		m_Module.Dry( {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

	void Click_KillPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_KILL", "KillPlayerMulti", "KillPlayerSingle", "KillPlayerSelf");
	}

	void KillPlayerMulti(JMConfirmation confirmation = NULL)
	{
		m_Module.SetHealth( 0, JM_GetSelected().GetPlayers() );
	}

	void KillPlayerSingle(JMConfirmation confirmation = NULL)
	{
		m_Module.SetHealth( 0, {JM_GetSelected().GetPlayersOrSelf()[0]} );
	}

	void KillPlayerSelf(JMConfirmation confirmation = NULL)
	{
		m_Module.SetHealth( 0, {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

	void Click_VomitPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_VOMIT", "VomitPlayerMultiConfirm", "VomitPlayerSingleConfirm", "VomitPlayerSelfConfirm");
	}

	void VomitPlayerMultiConfirm()
	{
		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_SET_VOMIT_DURATION_HEADER", "#STR_COT_PLAYER_MODULE_SET_VOMIT_DURATION_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "VomitPlayerMulti" );
	}

	void VomitPlayerSingleConfirm()
	{
		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_SET_VOMIT_DURATION_HEADER", "#STR_COT_PLAYER_MODULE_SET_VOMIT_DURATION_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "VomitPlayerSingle" );
	}

	void VomitPlayerSelfConfirm()
	{
		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_SET_VOMIT_DURATION_HEADER", "#STR_COT_PLAYER_MODULE_SET_VOMIT_DURATION_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "VomitPlayerSelf" );
	}

	void VomitPlayerMulti(JMConfirmation confirmation)
	{
		float value;
		if (confirmation.GetEditBoxValueFloat(value, 1, 120))
			m_Module.Vomit(value, JM_GetSelected().GetPlayers() );
	}

	void VomitPlayerSingle(JMConfirmation confirmation)
	{
		float value;
		if (confirmation.GetEditBoxValueFloat(value, 1, 120))
			m_Module.Vomit(value, {JM_GetSelected().GetPlayers()[0]} );
	}

	void VomitPlayerSelf(JMConfirmation confirmation)
	{
		float value;
		if (confirmation.GetEditBoxValueFloat(value, 1, 120))
			m_Module.Vomit(value, {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

	void Click_ScalePlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_SETSCALE", "ScalePlayerMultiConfirm", "ScalePlayerSingleConfirm", "ScalePlayerSelfConfirm", false);
	}

	void ScalePlayerMultiConfirm()
	{
		ShowScalePrompt( SCALE_MODE_MULTI );
	}

	void ScalePlayerSingleConfirm()
	{
		ShowScalePrompt( SCALE_MODE_SINGLE );
	}

	void ScalePlayerSelfConfirm()
	{
		ShowScalePrompt( SCALE_MODE_SELF );
	}

	//! Lazily built the first time it is needed, exactly like m_PlayerMenu -
	//! nothing before this ever needed a scale slider.
	private void ShowScalePrompt( int mode )
	{
		if ( !m_ScalePrompt )
		{
			if ( !m_Window )
				return;

			m_ScalePrompt = UIActionManager.CreateValuePrompt( layoutRoot, m_Window.GetWidgetRoot(), this, "OnConfirm_ScalePrompt" );
			RegisterOverlay( m_ScalePrompt );

			if ( !m_ScalePrompt )
				return;
		}

		m_ScalePendingMode = mode;

		// Range matches the server's own clamp in Exec_SetScale/RPC_SetScale -
		// a slider that could ask for more than the server will ever apply
		// would just teach the wrong ceiling.
		m_ScalePrompt.ShowSlider( "scale", "#STR_COT_PLAYER_MODULE_SET_SCALE_HEADER", "#STR_COT_PLAYER_MODULE_ACTION_SCALE", 0.1, 10, 1.0, 0.05, "%1" );
	}

	void OnConfirm_ScalePrompt( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_ScalePrompt )
			return;

		if ( m_ScalePrompt.GetPromptId() != "scale" )
			return;

		float value = m_ScalePrompt.GetSliderValue();

		if ( m_ScalePendingMode == SCALE_MODE_MULTI )
			m_Module.SetScale( value, JM_GetSelected().GetPlayers() );
		else if ( m_ScalePendingMode == SCALE_MODE_SINGLE )
			m_Module.SetScale( value, {JM_GetSelected().GetPlayers()[0]} );
		else
			m_Module.SetScale( value, {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

	void Click_SendMessage( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_SEND_MESSAGE", "SendMessageMultiConfirm", "SendMessageSingleConfirm", "SendMessageSelfConfirm", false);
	}

	void SendMessageMultiConfirm()
	{
		CreateConfirmation_Three( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_CONFIRM_MESSAGE", "SendMessageMulti", "#STR_COT_CONFIRM_NOTIFICATION", "SendNotifMulti" );
	}

	void SendMessageSingleConfirm()
	{
		CreateConfirmation_Three( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_CONFIRM_MESSAGE", "SendMessageSingle", "#STR_COT_CONFIRM_NOTIFICATION", "SendNotifSingle" );
	}

	void SendMessageSelfConfirm()
	{
		CreateConfirmation_Three( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_CONFIRM_MESSAGE", "SendMessageSelf", "#STR_COT_CONFIRM_NOTIFICATION", "SendNotifSelf" );
	}

	void SendMessageMulti(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text != "" )
			m_Module.DoMessage( JM_GetSelected().GetPlayers(), text);
	}

	void SendMessageSingle(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text != "" )
			m_Module.DoMessage( {JM_GetSelected().GetPlayers(true)[0]}, text);
	}

	void SendMessageSelf(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text != "" )
			m_Module.DoMessage( {GetPermissionsManager().GetClientPlayer().GetGUID()}, text);
	}

	void SendNotifMulti(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text != "" )
			m_Module.DoNotif( JM_GetSelected().GetPlayers(), text);
	}

	void SendNotifSingle(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text != "" )
			m_Module.DoNotif( {JM_GetSelected().GetPlayers(true)[0]}, text);
	}

	void SendNotifSelf(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text != "" )
			m_Module.DoNotif( {GetPermissionsManager().GetClientPlayer().GetGUID()}, text);
	}

	void Click_CopyExpLoadout( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

	#ifdef DZ_Expansion_Core
		JMESPModule module = CF_Modules<JMESPModule>.Get();
		if (module.CopyToClipboardExpLoadout(JMPlayerInstance))
			m_CopyExpLoadout.ShowFeedback();
	#endif
	}

	void Click_KickPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_KICK", "KickPlayerMultiConfirm", "KickPlayerSingleConfirm", "KickPlayerSelfConfirm", false);
	}

	void KickPlayerMultiConfirm(JMConfirmation confirmation = NULL)
	{
		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_KICK_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_KICK_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_KICK", "KickPlayerMulti" );
	}

	void KickPlayerSingleConfirm(JMConfirmation confirmation = NULL)
	{
		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_KICK_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_KICK_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_KICK", "KickPlayerSingle" );
	}

	void KickPlayerSelfConfirm(JMConfirmation confirmation = NULL)
	{
		COTCreateLocalAdminNotification( new StringLocaliser( "#STR_COT_PLAYER_MODULE_CANT_KICK_SELF" ) );
	}

	void KickPlayerMulti(JMConfirmation confirmation)
	{
		m_Module.Kick( JM_GetSelected().GetPlayers(), confirmation.GetEditBoxValue() );
	}

	void KickPlayerSingle(JMConfirmation confirmation)
	{
		m_Module.Kick( {JM_GetSelected().GetPlayers()[0]}, confirmation.GetEditBoxValue() );
	}

	void Click_BanPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_BAN", "BanPlayerMultiConfirm", "BanPlayerSingleConfirm", "BanPlayerSelfConfirm", false);
	}

	void BanPlayerMultiConfirm()
	{
		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_BAN_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_BAN_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_BAN", "BanPlayerMulti" );
	}

	void BanPlayerSingleConfirm()
	{
		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_BAN_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_BAN_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_BAN", "BanPlayerSingle" );
	}

	void BanPlayerSelfConfirm(JMConfirmation confirmation)
	{
		COTCreateLocalAdminNotification( new StringLocaliser( "#STR_COT_PLAYER_MODULE_CANT_BAN_SELF" ) );
	}

	void BanPlayerMulti(JMConfirmation confirmation)
	{
		m_Module.Ban( JM_GetSelected().GetPlayers(), confirmation.GetEditBoxValue(), -1 );
	}

	void BanPlayerSingle(JMConfirmation confirmation)
	{
		m_Module.Ban( {JM_GetSelected().GetPlayers()[0]}, confirmation.GetEditBoxValue(), -1 );
	}

	void Click_StopBleeding( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_STOP_BLEEDING", "StopBleedingMulti", "StopBleedingSingle", "StopBleedingSelf", false);
	}

	void StopBleedingMulti(JMConfirmation confirmation = NULL)
	{
		m_Module.StopBleeding( JM_GetSelected().GetPlayers() );
	}

	void StopBleedingSingle(JMConfirmation confirmation = NULL)
	{
		m_Module.StopBleeding( {JM_GetSelected().GetPlayersOrSelf()[0]} );
	}

	void StopBleedingSelf(JMConfirmation confirmation = NULL)
	{
		m_Module.StopBleeding( {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

	void Click_AddDisease( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		int agentIdx = m_DiseaseAgent.GetSelection();
		if ( agentIdx < 0 || agentIdx >= m_DiseaseAgentIds.Count() )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_DISEASES_ADD", "AddDiseaseMulti", "AddDiseaseSingle", "AddDiseaseSelf", false);
	}

	void AddDiseaseMulti( JMConfirmation confirmation = NULL )
	{
		int agentIdx = m_DiseaseAgent.GetSelection();
		int agent = m_DiseaseAgentIds[agentIdx];
		m_Module.AddDisease( agent, DISEASE_INJECT_COUNT, JM_GetSelected().GetPlayers() );
	}

	void AddDiseaseSingle( JMConfirmation confirmation = NULL )
	{
		int agentIdx = m_DiseaseAgent.GetSelection();
		int agent = m_DiseaseAgentIds[agentIdx];
		m_Module.AddDisease( agent, DISEASE_INJECT_COUNT, { JM_GetSelected().GetPlayersOrSelf()[0] } );
	}

	void AddDiseaseSelf( JMConfirmation confirmation = NULL )
	{
		int agentIdx = m_DiseaseAgent.GetSelection();
		int agent = m_DiseaseAgentIds[agentIdx];
		m_Module.AddDisease( agent, DISEASE_INJECT_COUNT, { GetPermissionsManager().GetClientPlayer().GetGUID() } );
	}

	void Click_RemoveDisease( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		int agentIdx = m_DiseaseAgent.GetSelection();
		if ( agentIdx < 0 || agentIdx >= m_DiseaseAgentIds.Count() )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_DISEASES_REMOVE", "RemoveDiseaseMulti", "RemoveDiseaseSingle", "RemoveDiseaseSelf", false);
	}

	void RemoveDiseaseMulti( JMConfirmation confirmation = NULL )
	{
		int agentIdx = m_DiseaseAgent.GetSelection();
		int agent = m_DiseaseAgentIds[agentIdx];
		m_Module.RemoveDisease( agent, JM_GetSelected().GetPlayers() );
	}

	void RemoveDiseaseSingle( JMConfirmation confirmation = NULL )
	{
		int agentIdx = m_DiseaseAgent.GetSelection();
		int agent = m_DiseaseAgentIds[agentIdx];
		m_Module.RemoveDisease( agent, { JM_GetSelected().GetPlayersOrSelf()[0] } );
	}

	void RemoveDiseaseSelf( JMConfirmation confirmation = NULL )
	{
		int agentIdx = m_DiseaseAgent.GetSelection();
		int agent = m_DiseaseAgentIds[agentIdx];
		m_Module.RemoveDisease( agent, { GetPermissionsManager().GetClientPlayer().GetGUID() } );
	}

	// ---------------- Bleed-from-body-part handlers ----------------

	void Click_ApplyBleed( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		string sel = m_BleedingPart.GetSelectedText();
		if ( sel == "" )
			return;

		if ( sel == "All" )
			CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_BLEEDING_STOP", "ApplyBleedMulti", "ApplyBleedSingle", "ApplyBleedSelf", false);
		else
			CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_BLEEDING_APPLY", "ApplyBleedMulti", "ApplyBleedSingle", "ApplyBleedSelf", false);
	}

	void ApplyBleedMulti( JMConfirmation confirmation = NULL )
	{
		string sel = m_BleedingPart.GetSelectedText();
		if ( sel == "All" )
			m_Module.StopBleeding( JM_GetSelected().GetPlayers() );
		else
			m_Module.AddBleedingPart( sel, JM_GetSelected().GetPlayers() );
	}

	void ApplyBleedSingle( JMConfirmation confirmation = NULL )
	{
		string sel = m_BleedingPart.GetSelectedText();
		if ( sel == "All" )
			m_Module.StopBleeding( { JM_GetSelected().GetPlayersOrSelf()[0] } );
		else
			m_Module.AddBleedingPart( sel, { JM_GetSelected().GetPlayersOrSelf()[0] } );
	}

	void ApplyBleedSelf( JMConfirmation confirmation = NULL )
	{
		string sel = m_BleedingPart.GetSelectedText();
		if ( sel == "All" )
			m_Module.StopBleeding( { GetPermissionsManager().GetClientPlayer().GetGUID() } );
		else
			m_Module.AddBleedingPart( sel, { GetPermissionsManager().GetClientPlayer().GetGUID() } );
	}

	// ---------------- Disease dropdown rebuild ----------------

	private void RebuildDiseaseDropdown( int cholera, int influenza, int salmonella, int brain, int foodPoison, int chemPoison, int wound, int nerve, int heavyMetal )
	{
		// Arrives from a server response, which does not wait for the Actions
		// tab to have been opened.
		if ( !m_DiseaseAgent )
			return;

		if ( !m_DiseaseAgent )
			return;

		// Don't rebuild while the user is interacting with the popup - clearing
		// and re-adding entries pops each row's widget creation in the dropdown
		// which gives the appearance of an auto-close. Defer until the next
		// user-triggered action.
		if ( m_DiseaseAgent.IsOpen() )
			return;

		// Cache selection so we can restore it after rebuild - otherwise
		// calling SetSelection() forces the dropdown closed even with sendEvent=false,
		// which makes the form feel like it's auto-refreshing the popup.
		int prevSel = m_DiseaseAgent.GetSelection();

		m_DiseaseAgent.ClearEntries();

		// (count, label, agentId)
		int counts[9];
		counts[0] = cholera;
		counts[1] = influenza;
		counts[2] = salmonella;
		counts[3] = brain;
		counts[4] = foodPoison;
		counts[5] = chemPoison;
		counts[6] = wound;
		counts[7] = nerve;
		counts[8] = heavyMetal;

		string labels[9];
		labels[0] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_CHOLERA";
		labels[1] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_INFLUENZA";
		labels[2] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_SALMONELLA";
		labels[3] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_BRAIN_KURU";
		labels[4] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_FOOD_POISONING";
		labels[5] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_CHEMICAL_POISONING";
		labels[6] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_WOUND_INFECTION";
		labels[7] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_NERVE_AGENT";
		labels[8] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_HEAVY_METAL_POISONING";

		int COLOR_OK = ARGB(255, 73, 184, 117);   // green = not infected
		int COLOR_BAD = ARGB(255, 217, 90, 90);  // red = infected

		for ( int i = 0; i < 9; i++ )
		{
			string icon;
			int colour;
			if ( counts[i] > 0 )
			{
				icon = JMConstants.ICON_HEALTH_DECREASE;
				colour = COLOR_BAD;
			}
			else
			{
				icon = JMConstants.ICON_HEALTH_NORMAL;
				colour = COLOR_OK;
			}

			m_DiseaseAgent.AddEntry( labels[i] + " (" + counts[i].ToString() + ")", icon, colour );
		}

		// Restore prior selection without firing CHANGE (which would also close the popup).
		// If prior selection was -1 or out of range, fall back to 0.
		int restore = prevSel;
		if ( restore < 0 || restore >= 9 )
			restore = 0;
		m_DiseaseAgent.SetSelection( restore, false );

		//! This arrives from a server RPC round-trip kicked off by OnChange_Tab,
		//! strictly after OnChange_Tab's own one-shot UpdateScroller() call has
		//! already measured and settled the Actions tab's content height. Nine
		//! freshly-added dropdown rows can change that height, so the scroller
		//! needs telling again or it's stuck showing the pre-response size
		//! (typically "no scrollbar yet" on the very first open, before this
		//! response has ever arrived).
		if ( m_ActionListScroller )
			m_ActionListScroller.UpdateScroller();
	}

	// Called from the module when the server pushes the selected player's mask.
	void OnDiseaseMaskUpdated( string guid, int cholera, int influenza, int salmonella, int brain, int foodPoison, int chemPoison, int wound, int nerve, int heavyMetal )
	{
		if ( guid == m_LastSelectedGuid )
			RebuildDiseaseDropdown( cholera, influenza, salmonella, brain, foodPoison, chemPoison, wound, nerve, heavyMetal );
	}

	// Called from the module when the server pushes the selected player's
	// bleeding state. Rebuilds the body-part dropdown with red icons on the
	// parts that are actively bleeding.
	void OnBleedingStateUpdated( string guid, array< string > names, array< int > bits, int bleedingBits, array< int > activePartIndices )
	{
		if ( guid == m_LastSelectedGuid )
			RebuildBleedDropdown( names, bits, activePartIndices );
	}

	//! Maps a raw bleed zone identifier - the static fallback list above, or a
	//! zone name the server pushes over RPC - to its localization key. A zone
	//! name this list does not recognize comes back unchanged rather than
	//! blanked, since the dropdown still has to show something for it.
	private string BleedPartLabel( string zoneName )
	{
		if ( zoneName == "All" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_ALL";
		if ( zoneName == "Head" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_HEAD";
		if ( zoneName == "Torso" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_TORSO";
		if ( zoneName == "LeftArm" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_LEFT_ARM";
		if ( zoneName == "RightArm" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_RIGHT_ARM";
		if ( zoneName == "LeftLeg" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_LEFT_LEG";
		if ( zoneName == "RightLeg" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_RIGHT_LEG";
		if ( zoneName == "LeftFoot" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_LEFT_FOOT";
		if ( zoneName == "RightFoot" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_RIGHT_FOOT";

		return zoneName;
	}

	private void RebuildBleedDropdown( array< string > names, array< int > bits, array< int > activePartIndices )
	{
		if ( !m_BleedingPart )
			return;

		if ( !m_BleedingPart )
			return;

		int prevSel = m_BleedingPart.GetSelection();

		m_BleedingPart.ClearEntries();

		int COLOR_OK  = ARGB(255, 73, 184, 117);
		int COLOR_BAD = ARGB(255, 217, 90, 90);

		// First entry: "All" - green = no bleeders, red = bleeders present.
		bool anyBleeding = activePartIndices.Count() > 0;
		if ( anyBleeding )
			m_BleedingPart.AddEntry( BleedPartLabel( "All" ), JMConstants.ICON_BLEEDING_WOUND, COLOR_BAD );
		else
			m_BleedingPart.AddEntry( BleedPartLabel( "All" ), JMConstants.ICON_BLEEDING_WOUND, COLOR_OK );

		for ( int i = 0; i < names.Count(); i++ )
		{
			string nm = names.Get( i );
			bool active = false;
			for ( int a = 0; a < activePartIndices.Count(); a++ )
			{
				if ( activePartIndices.Get( a ) == i )
				{
					active = true;
					break;
				}
			}
			if ( active )
				m_BleedingPart.AddEntry( BleedPartLabel( nm ), JMConstants.ICON_BLEEDING_WOUND, COLOR_BAD );
			else
				m_BleedingPart.AddEntry( BleedPartLabel( nm ), JMConstants.ICON_BLEEDING_WOUND, COLOR_OK );
		}

		int restore = prevSel;
		int maxEntries = names.Count() + 1;
		if ( restore < 0 || restore >= maxEntries )
			restore = 0;
		m_BleedingPart.SetSelection( restore, false );

		//! Same reasoning as RebuildDiseaseDropdown's own trailing call - this
		//! also arrives after OnChange_Tab's scroller measurement already ran.
		if ( m_ActionListScroller )
			m_ActionListScroller.UpdateScroller();
	}

	void Click_ClearAllDiseases( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_DISEASES_CLEAR", "ClearAllDiseasesMulti", "ClearAllDiseasesSingle", "ClearAllDiseasesSelf", false);
	}

	void ClearAllDiseasesMulti( JMConfirmation confirmation = NULL )
	{
		m_Module.RemoveAllDiseases( JM_GetSelected().GetPlayers() );
	}

	void ClearAllDiseasesSingle( JMConfirmation confirmation = NULL )
	{
		m_Module.RemoveAllDiseases( { JM_GetSelected().GetPlayersOrSelf()[0] } );
	}

	void ClearAllDiseasesSelf( JMConfirmation confirmation = NULL )
	{
		m_Module.RemoveAllDiseases( { GetPermissionsManager().GetClientPlayer().GetGUID() } );
	}

	void Click_HealPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_HEAL", "HealMulti", "HealSingle", "HealSelf", false);
	}

	void HealMulti(JMConfirmation confirmation = NULL)
	{
		m_Module.Heal( JM_GetSelected().GetPlayers() );
	}

	void HealSingle(JMConfirmation confirmation = NULL)
	{
		m_Module.Heal( {JM_GetSelected().GetPlayersOrSelf()[0]} );
	}

	void HealSelf(JMConfirmation confirmation = NULL)
	{
		m_Module.Heal( {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

	void Click_SpectatePlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		TStringArray players = JM_GetSelected().GetPlayers();
		string guid = players[0];

		if (guid == GetPermissionsManager().GetClientGUID() && CurrentActiveCamera)
		{
			//! Selected player is ourself and we are in spectator cam, or we are in freecam and previous camera was spectator cam

			if (CurrentActiveCamera.IsInherited(JMSpectatorCamera) || (COT_PreviousActiveCamera && COT_PreviousActiveCamera.IsInherited(JMSpectatorCamera)))
			{
				//! We are spectating something else. Stop spectating.
				m_Module.EndSpectating();
				return;
			}
		}

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer(guid);
		PlayerBase player;
		if (instance)
			player = instance.PlayerObject;

		m_Module.Click_Spectate(action, player, guid);
	}

	void Click_RepairTransport( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.RepairTransport( JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_SetPosition( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_TO_COORDINATES", "SetPositionMulti", "SetPositionSingle", "SetPositionSelf", false);
	}

	vector GetPosition()
	{
		vector pos = vector.Zero;
		pos[0] = m_PositionX.GetText().ToFloat();
		pos[1] = m_PositionY.GetText().ToFloat();
		pos[2] = m_PositionZ.GetText().ToFloat();
		return pos;
	}

	void SetPositionMulti(JMConfirmation confirmation = NULL)
	{
		UpdateLastChangeTime();
		m_Module.TeleportTo(GetPosition(), JM_GetSelected().GetPlayers() );
	}

	void SetPositionSingle(JMConfirmation confirmation = NULL)
	{
		UpdateLastChangeTime();
		m_Module.TeleportTo(GetPosition(), {JM_GetSelected().GetPlayersOrSelf()[0]} );
	}

	void SetPositionSelf(JMConfirmation confirmation = NULL)
	{
		UpdateLastChangeTime();
		m_Module.TeleportTo(GetPosition(), {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

	void Click_TeleportToMe( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_TO_ME", "TeleportToMeMulti", "TeleportToMeSingle", "TeleportToMeSelf", false);
	}

	void TeleportToMeMulti(JMConfirmation confirmation = NULL)
	{
		vector pos = GetCurrentPosition();
		if (pos != vector.Zero)
			m_Module.TeleportTo(pos, JM_GetSelected().GetPlayers() );
	}

	void TeleportToMeSingle(JMConfirmation confirmation = NULL)
	{
		vector pos = GetCurrentPosition();
		if (pos != vector.Zero)
			m_Module.TeleportTo(pos, {JM_GetSelected().GetPlayersOrSelf()[0]} );
	}

	void TeleportToMeSelf(JMConfirmation confirmation = NULL)
	{
		vector pos = GetCurrentPosition();
		if (pos != vector.Zero)
			m_Module.TeleportTo(pos, {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

    void Click_CopyPlayerName( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		//! No toast: the button swaps to "Copied!" where the click happened.
		g_Game.CopyToClipboard(m_Name.GetButton());
	}

    void Click_CopyPlayerGUID( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		g_Game.CopyToClipboard(m_GUID.GetButton());
	}

    void Click_CopyPlayerSteam64ID( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		g_Game.CopyToClipboard(m_Steam64ID.GetButton());
	}

    void Click_OpenPlayerSteamProfile( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		g_Game.OpenURL("https://steamcommunity.com/profiles/" + m_Steam64ID.GetButton());
	}

	#ifdef GAMELABS
	void Click_CopyCFToolsID( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		g_Game.CopyToClipboard(m_CFToolsID.GetButton());
	}

    void Click_OpenPlayerCFProfile( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		g_Game.OpenURL(m_SelectedInstance.PlayerObject.GetUpstreamIdentityHotlink());
	}
	#endif

    void Click_CopyPlayerPostion( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		g_Game.CopyToClipboard("<" + m_PositionX.GetText() + ", " + m_PositionY.GetText() + ", " + m_PositionZ.GetText() + ">");

		m_CopyPositionPlayer.ShowFeedback();
	}

    void Click_CopyPlayerRotation( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_SelectedInstance )
			return;

		vector rotation = m_SelectedInstance.GetOrientation();

		//! Same deal as the position: the entity is here when the player is in
		//! our bubble, and reading it beats the rounded roster snapshot.
		PlayerBase loaded = m_SelectedInstance.PlayerObject;
		if ( loaded )
			rotation = loaded.GetOrientation();

		g_Game.CopyToClipboard("<" + FormatCoordinate( rotation[0], COORD_DECIMALS_LIVE ) + ", " + FormatCoordinate( rotation[1], COORD_DECIMALS_LIVE ) + ", " + FormatCoordinate( rotation[2], COORD_DECIMALS_LIVE ) + ">");

		m_CopyRotationPlayer.ShowFeedback();
	}

	void Click_PastePlayerPostion( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		string clipboard;
		g_Game.CopyFromClipboard(clipboard);

		vector pos = clipboard.BeautifiedToVector();

		// The copies answer a click with a check mark, so the paste has to
		// answer one too - otherwise the only button in the strip that can
		// fail is also the only one that says nothing when it works. A
		// clipboard holding anything that is not a vector flashes a red cross
		// instead: the fields are left alone and nothing is sent.
		if (pos == vector.Zero)
		{
			m_PastePositionPlayer.SetFeedbackIcon( JMConstants.Lucide( "x" ) );
			m_PastePositionPlayer.SetFeedbackColor( JMTheme.DANGER_FILL );
			m_PastePositionPlayer.ShowFeedback();
			return;
		}

		m_PositionX.SetText(FormatCoordinate( pos[0], COORD_DECIMALS_LIVE ));
		m_PositionY.SetText(FormatCoordinate( pos[1], COORD_DECIMALS_LIVE ));
		m_PositionZ.SetText(FormatCoordinate( pos[2], COORD_DECIMALS_LIVE ));

		Click_SetPosition(eid, action);

		m_PastePositionPlayer.SetFeedbackIcon( JMConstants.ICON_CHECK_MARK );
		m_PastePositionPlayer.SetFeedbackColor( JMTheme.SUCCESS_DIM );
		m_PastePositionPlayer.ShowFeedback();
	}

	void Click_TeleportMeTo( UIEvent eid, UIActionBase action )
	{
		if ( JM_GetSelected().GetPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		m_Module.TeleportSenderTo( JM_GetSelected().GetPlayers()[0] );
	}

	void Click_RefreshTeleports( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_PositionRefresh.TriggerSpin( 2 );
		RefreshTeleports(true);
		RefreshStats(true);

		//! Outright, not through SyncTeleportHistory: nothing about the panel
		//! has to have changed for the ages on its rows to be stale.
		RebuildTeleportHistory();
	}
	
	void Change_PositionX( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE && eid != UIEvent.MOUSEWHEEL )
			return;

		UpdateLastChangeTime();

		m_PositionXUpdated = true;
	}
	
	void Change_PositionY( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE && eid != UIEvent.MOUSEWHEEL )
			return;

		UpdateLastChangeTime();

		m_PositionYUpdated = true;
	}
	
	void Change_PositionZ( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE && eid != UIEvent.MOUSEWHEEL )
			return;

		UpdateLastChangeTime();

		m_PositionZUpdated = true;
	}

	void RefreshStats(bool force = false)
	{
		if ( !m_SelectedInstance )
			return;
		
		if (g_Game.IsClient() && m_SelectedInstance.GetDataLastUpdatedTime() < m_LastChangeTime)
			return;

		// The coordinate fields only exist on the Position tab, and this runs ten
		// times a second. Everything below is a cheap SetCurrent / SetChecked and
		// stays ungated.
		if ( IsTabActive( TAB_POSITION ) )
		{
			RefreshTeleports(force);
			SyncTeleportHistory();
		}

		RefreshIdentityBadges();

		if (force)
		{
			m_HealthUpdated = false;
			m_BloodUpdated = false;
			m_EnergyUpdated = false;
			m_WaterUpdated = false;
			m_ShockUpdated = false;
			m_StaminaUpdated = false;
			m_HeatBufferUpdated = false;
		}

		if ( m_Health && !m_HealthUpdated )
			m_Health.SetCurrent( m_SelectedInstance.GetHealth() );
		
		if ( m_Blood && !m_BloodUpdated )
			m_Blood.SetCurrent( m_SelectedInstance.GetBlood() );
		
		if ( m_Energy && !m_EnergyUpdated )
			m_Energy.SetCurrent( m_SelectedInstance.GetEnergy() );
		
		if ( m_Water && !m_WaterUpdated )
			m_Water.SetCurrent( m_SelectedInstance.GetWater() );
		
		if ( m_Shock && !m_ShockUpdated )
			m_Shock.SetCurrent( m_SelectedInstance.GetShock() );
		
		if ( m_Stamina && !m_StaminaUpdated )
			m_Stamina.SetCurrent( m_SelectedInstance.GetStamina() );
		
		if ( m_HeatComfort )
			m_HeatComfort.SetCurrent( m_SelectedInstance.GetHeatComfort() );

		if ( m_HeatBuffer && !m_HeatBufferUpdated )
		{
			m_HeatBuffer.SetCurrent( m_SelectedInstance.GetHeatBuffer() / 10 );
			int heatbufferState = m_HeatBuffer.GetCurrent();
			m_HeatBuffer.SetText(m_HeatBufferStates[heatbufferState] + " "+ m_HeatBuffer.GetCurrent());
		}

		if ( m_BloodyHands )
			m_BloodyHands.SetChecked( m_SelectedInstance.HasBloodyHands() );
		
		if ( m_GodMode )
			m_GodMode.SetChecked( m_SelectedInstance.HasGodMode() );
		
		if ( m_Freeze )
			m_Freeze.SetChecked( m_SelectedInstance.IsFrozen() );
		
		if ( m_Invisibility )
			m_Invisibility.SetChecked( m_SelectedInstance.HasInvisibility() );
		
		if ( m_UnlimitedAmmo )
			m_UnlimitedAmmo.SetChecked( m_SelectedInstance.HasUnlimitedAmmo() );
		
		if ( m_UnlimitedStamina )
			m_UnlimitedStamina.SetChecked( m_SelectedInstance.HasUnlimitedStamina() );
		
		if ( m_AdminNVG )
			m_AdminNVG.SetChecked( m_SelectedInstance.HasAdminNVG() );

		if ( m_BrokenLegs )
			m_BrokenLegs.SetChecked( m_SelectedInstance.HasBrokenLegs() );
		
		if ( m_ReceiveDmgDealt )
			m_ReceiveDmgDealt.SetChecked( m_SelectedInstance.GetReceiveDmgDealt() );
		
		if ( m_CannotBeTargetedByAI )
			m_CannotBeTargetedByAI.SetChecked( m_SelectedInstance.GetCannotBeTargetedByAI() );
		
		if ( m_RemoveCollision )
			m_RemoveCollision.SetChecked( m_SelectedInstance.GetRemoveCollision() );

		if ( m_Health.GetCurrent() >= PlayerConstants.SL_HEALTH_HIGH )
		{
			m_Health.SetColor( Colors.COLOR_PRISTINE );
			m_Health.SetAlpha( 1.0 );
		}
		else if ( m_Health.GetCurrent() >= PlayerConstants.SL_HEALTH_NORMAL )
		{
			m_Health.SetColor( JMTheme.VALUE_OK );
		}
		else if ( m_Health.GetCurrent() >= PlayerConstants.SL_HEALTH_LOW )
		{
			m_Health.SetColor( JMTheme.VALUE_CAUTION );
		}
		else
		{
			m_Health.SetColor( JMTheme.VALUE_BAD );
		}

		if ( m_Blood.GetCurrent() >= PlayerConstants.SL_BLOOD_HIGH )
		{
			m_Blood.SetColor( Colors.COLOR_PRISTINE );
			m_Blood.SetAlpha( 1.0 );
		}
		else if ( m_Blood.GetCurrent() >= PlayerConstants.SL_BLOOD_NORMAL )
		{
			m_Blood.SetColor( JMTheme.VALUE_OK );
		}
		else if ( m_Blood.GetCurrent() >= PlayerConstants.SL_BLOOD_LOW )
		{
			m_Blood.SetColor( JMTheme.VALUE_CAUTION );
		}
		else
		{
			m_Blood.SetColor( JMTheme.VALUE_BAD );
		}

		if ( m_Energy.GetCurrent() >= PlayerConstants.SL_ENERGY_HIGH )
		{
			m_Energy.SetColor( Colors.COLOR_PRISTINE );
			m_Energy.SetAlpha( 1.0 );
		}
		else if ( m_Energy.GetCurrent() >= PlayerConstants.SL_ENERGY_NORMAL )
		{
			m_Energy.SetColor( JMTheme.VALUE_OK );
		}
		else if ( m_Energy.GetCurrent() >= PlayerConstants.SL_ENERGY_LOW )
		{
			m_Energy.SetColor( JMTheme.VALUE_CAUTION );
		}
		else
		{
			m_Energy.SetColor( JMTheme.VALUE_BAD );
		}

		if ( m_Water.GetCurrent() >= PlayerConstants.SL_WATER_HIGH )
		{
			m_Water.SetColor( Colors.COLOR_PRISTINE );
			m_Water.SetAlpha( 1.0 );
		}
		else if ( m_Water.GetCurrent() >= PlayerConstants.SL_WATER_NORMAL )
		{
			m_Water.SetColor( JMTheme.VALUE_OK );
		}
		else if ( m_Water.GetCurrent() >= PlayerConstants.SL_WATER_LOW )
		{
			m_Water.SetColor( JMTheme.VALUE_CAUTION );
		}
		else
		{
			m_Water.SetColor( JMTheme.VALUE_BAD );
		}

		if ( m_Shock.GetCurrent() >= 75 )
		{
			m_Shock.SetColor( Colors.COLOR_PRISTINE );
			m_Shock.SetAlpha( 1.0 );
		}
		else if ( m_Shock.GetCurrent() >= PlayerConstants.CONSCIOUS_THRESHOLD )
		{
			m_Shock.SetColor( JMTheme.VALUE_OK );
		}
		else if ( m_Shock.GetCurrent() < PlayerConstants.UNCONSCIOUS_THRESHOLD )
		{
			m_Shock.SetColor( JMTheme.VALUE_BAD );
		}
		else
		{
			m_Shock.SetColor( JMTheme.VALUE_CAUTION );
		}

		if ( m_Stamina.GetCurrent() >= 75 )
		{
			m_Stamina.SetColor( Colors.COLOR_PRISTINE );
			m_Stamina.SetAlpha( 1.0 );
		}
		else if ( m_Stamina.GetCurrent() >= 50 )
		{
			m_Stamina.SetColor( JMTheme.VALUE_OK );
		}
		else if ( m_Stamina.GetCurrent() >= 25 )
		{
			m_Stamina.SetColor( JMTheme.VALUE_CAUTION );
		}
		else
		{
			m_Stamina.SetColor( JMTheme.VALUE_BAD );
		}

		// COLD
		if ( m_HeatComfort.GetCurrent() <= PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_CRITICAL )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_COLD_LVL_FOUR );
		}
		else if ( m_HeatComfort.GetCurrent() <= PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_WARNING )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_COLD_LVL_TWO );
		}
		else if ( m_HeatComfort.GetCurrent() <= PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_EMPTY )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_COLD_LVL_ONE );
		}
		// NEUTRAL
		else if ( m_HeatComfort.GetCurrent() > PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_EMPTY && m_HeatComfort.GetCurrent() < PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_WARNING )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_NEUTAL );
		}
		// HOT
		else if ( m_HeatComfort.GetCurrent() >= PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_CRITICAL )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_HOT_LVL_FOUR );
		}
		else if ( m_HeatComfort.GetCurrent() >= PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_WARNING )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_HOT_LVL_TWO );
		}
		else if ( m_HeatComfort.GetCurrent() >= PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_EMPTY )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_HOT_LVL_ONE );
		}
		m_HeatComfort.SetAlpha( 1.0 );

		if ( m_HeatBuffer.GetCurrent() >= 3 )
		{
			m_HeatBuffer.SetColor( Colors.COLOR_PRISTINE );
		}
		else if ( m_HeatBuffer.GetCurrent() >= 2 )
		{
			m_HeatBuffer.SetColor( Colors.COLOR_WORN );
		}
		else if ( m_HeatBuffer.GetCurrent() >= 1 )
		{
			m_HeatBuffer.SetColor( Colors.COLOR_WORN );
		}
		else
		{
			m_HeatBuffer.SetColor(0x00FFFFFF);
		}
		m_HeatBuffer.SetAlpha( 1.0 );
	}

	void RefreshTeleports(bool force = false)
	{
		if ( !m_PositionX )
			return;

		if ( !m_SelectedInstance )
			return;

		if ( IsMissionOffline() )
			m_SelectedInstance.Update();

		vector position = m_SelectedInstance.GetPosition();
		int decimals = COORD_DECIMALS_SYNCED;

		//! Null unless the entity is loaded on this client - the server only
		//! sends a reference, and a player outside our network bubble has none.
		//! When it is there it is both fresher and finer than the roster
		//! snapshot, which is rounded down to what the sync is worth.
		PlayerBase loaded = m_SelectedInstance.PlayerObject;
		bool isLive = loaded != NULL;

		if ( isLive )
		{
			position = loaded.GetPosition();
			decimals = COORD_DECIMALS_LIVE;
		}

		if ( isLive != m_PositionIsLive )
		{
			m_PositionIsLive = isLive;
			UpdatePositionSourceHint();
		}

		if (force)
		{
			m_PositionXUpdated = false;
			m_PositionYUpdated = false;
			m_PositionZUpdated = false;
		}

		if ( m_PositionX && !m_PositionXUpdated )
			m_PositionX.SetText( FormatCoordinate( position[0], decimals ) );

		if ( m_PositionY && !m_PositionYUpdated )
			m_PositionY.SetText( FormatCoordinate( position[1], decimals ) );

		if ( m_PositionZ && !m_PositionZUpdated )
			m_PositionZ.SetText( FormatCoordinate( position[2], decimals ) );
	}

	//! Says where the three numbers came from: "11969.70" and "11969.703" are
	//! otherwise the same reading with a stray digit, and only one of them is
	//! accurate enough to teleport something back onto a roof.
	private void UpdatePositionSourceHint()
	{
		if ( !m_PositionX )
			return;

		string hint = "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_SOURCE_SYNCED";

		if ( m_PositionIsLive )
			hint = "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_SOURCE_LIVE";

		if ( m_PositionX )
			m_PositionX.SetTooltip( hint );

		if ( m_PositionY )
			m_PositionY.SetTooltip( hint );

		if ( m_PositionZ )
			m_PositionZ.SetTooltip( hint );
	}

	//! float.ToString() prints about six significant digits, which on a
	//! five-digit map coordinate is a whole metre of slop - 11969.7 is every
	//! position between 11969.65 and 11969.75. Print the fraction from an
	//! integer instead so the digits that survived the read are all shown.
	private string FormatCoordinate( float value, int decimals )
	{
		int scale = 1;
		for ( int i = 0; i < decimals; i++ )
			scale *= 10;

		bool negative = value < 0;
		if ( negative )
			value = -value;

		int whole = Math.Floor( value );
		int fraction = Math.Round( ( value - whole ) * scale );

		// Rounding the fraction up to the scale is a carry into the whole part.
		if ( fraction >= scale )
		{
			whole++;
			fraction -= scale;
		}

		string text = fraction.ToString();
		while ( text.Length() < decimals )
			text = "0" + text;

		text = whole.ToString() + "." + text;

		if ( negative )
			text = "-" + text;

		return text;
	}

	void Click_ApplyStats( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_APPLY", "ApplyStatsMulti", "ApplyStatsSingle", "ApplyStatsSelf", false);
	}

	void ApplyStatsMulti(JMConfirmation confirmation = NULL)
	{
		ApplyStats(JM_GetSelected().GetPlayers());
	}

	void ApplyStatsSingle(JMConfirmation confirmation = NULL)
	{
		ApplyStats({JM_GetSelected().GetPlayers()[0]});
	}

	void ApplyStatsSelf(JMConfirmation confirmation = NULL)
	{
		ApplyStats({GetPermissionsManager().GetClientPlayer().GetGUID()});
	}

	void ApplyStats(TStringArray guids)
	{
		UpdateLastChangeTime();

		if ( m_HealthUpdated )
		{
			m_HealthUpdated = false;

			if ( m_Health )
				m_Module.SetHealth( m_Health.GetCurrent(), guids );
		}

		if ( m_BloodUpdated )
		{
			m_BloodUpdated = false;

			if ( m_Blood )
				m_Module.SetBlood( m_Blood.GetCurrent(), guids );
		}

		if ( m_EnergyUpdated )
		{
			m_EnergyUpdated = false;

			if ( m_Energy )
				m_Module.SetEnergy( m_Energy.GetCurrent(), guids );
		}

		if ( m_WaterUpdated )
		{
			m_WaterUpdated = false;

			if ( m_Water )
				m_Module.SetWater( m_Water.GetCurrent(), guids );
		}

		if ( m_ShockUpdated )
		{
			m_ShockUpdated = false;

			if ( m_Shock )
				m_Module.SetShock( m_Shock.GetCurrent(), guids );
		}

		if ( m_StaminaUpdated )
		{
			m_StaminaUpdated = false;

			if ( m_Stamina )
				m_Module.SetStamina( m_Stamina.GetCurrent(), guids );
		}

		if ( m_HeatBufferUpdated )
		{
			m_HeatBufferUpdated = false;

			if ( m_HeatBuffer )
				m_Module.SetHeatBuffer( m_HeatBuffer.GetCurrent() * 10, guids );
		}

		//! Apply is otherwise silent: the sliders already read the value they
		//! were dragged to, so nothing on screen changes when the RPC goes out
		//! and an admin cannot tell a click that landed from one that missed.
		//! The check mark on the pill is the receipt.
		if ( m_ApplyStats )
			m_ApplyStats.AnimateFeedback();
	}

	void Click_SetHealth( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateLastChangeTime();
	
		if ( m_Health.GetCurrent() >= PlayerConstants.SL_HEALTH_HIGH )
		{
			m_Health.SetColor( Colors.COLOR_PRISTINE );
			m_Health.SetAlpha( 1.0 );
		}
		else if ( m_Health.GetCurrent() >= PlayerConstants.SL_HEALTH_NORMAL )
		{
			m_Health.SetColor( JMTheme.VALUE_OK );
		}
		else if ( m_Health.GetCurrent() >= PlayerConstants.SL_HEALTH_LOW )
		{
			m_Health.SetColor( JMTheme.VALUE_CAUTION );
		}
		else
		{
			m_Health.SetColor( JMTheme.VALUE_BAD );
		}

		m_HealthUpdated = true;
	}

	void Click_SetBlood( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateLastChangeTime();

		if ( m_Blood.GetCurrent() >= PlayerConstants.SL_BLOOD_HIGH )
		{
			m_Blood.SetColor( Colors.COLOR_PRISTINE );
			m_Blood.SetAlpha( 1.0 );
		}
		else if ( m_Blood.GetCurrent() >= PlayerConstants.SL_BLOOD_NORMAL )
		{
			m_Blood.SetColor( JMTheme.VALUE_OK );
		}
		else if ( m_Blood.GetCurrent() >= PlayerConstants.SL_BLOOD_LOW )
		{
			m_Blood.SetColor( JMTheme.VALUE_CAUTION );
		}
		else
		{
			m_Blood.SetColor( JMTheme.VALUE_BAD );
		}

		m_BloodUpdated = true;
	}

	void Click_SetEnergy( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateLastChangeTime();

		if ( m_Energy.GetCurrent() >= PlayerConstants.SL_ENERGY_HIGH )
		{
			m_Energy.SetColor( Colors.COLOR_PRISTINE );
			m_Energy.SetAlpha( 1.0 );
		}
		else if ( m_Energy.GetCurrent() >= PlayerConstants.SL_ENERGY_NORMAL )
		{
			m_Energy.SetColor( JMTheme.VALUE_OK );
		}
		else if ( m_Energy.GetCurrent() >= PlayerConstants.SL_ENERGY_LOW )
		{
			m_Energy.SetColor( JMTheme.VALUE_CAUTION );
		}
		else
		{
			m_Energy.SetColor( JMTheme.VALUE_BAD );
		}

		m_EnergyUpdated = true;
	}

	void Click_SetWater( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateLastChangeTime();

		if ( m_Water.GetCurrent() >= PlayerConstants.SL_WATER_HIGH )
		{
			m_Water.SetColor( Colors.COLOR_PRISTINE );
			m_Water.SetAlpha( 1.0 );
		}
		else if ( m_Water.GetCurrent() >= PlayerConstants.SL_WATER_NORMAL )
		{
			m_Water.SetColor( JMTheme.VALUE_OK );
		}
		else if ( m_Water.GetCurrent() >= PlayerConstants.SL_WATER_LOW )
		{
			m_Water.SetColor( JMTheme.VALUE_CAUTION );
		}
		else
		{
			m_Water.SetColor( JMTheme.VALUE_BAD );
		}

		m_WaterUpdated = true;
	}

	void Click_SetShock( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateLastChangeTime();

		if ( m_Shock.GetCurrent() >= 75 )
		{
			m_Shock.SetColor( Colors.COLOR_PRISTINE );
			m_Shock.SetAlpha( 1.0 );
		}
		else if ( m_Shock.GetCurrent() >= PlayerConstants.CONSCIOUS_THRESHOLD )
		{
			m_Shock.SetColor( JMTheme.VALUE_OK );
		}
		else if ( m_Shock.GetCurrent() < PlayerConstants.UNCONSCIOUS_THRESHOLD )
		{
			m_Shock.SetColor( JMTheme.VALUE_BAD );
		}
		else
		{
			m_Shock.SetColor( JMTheme.VALUE_CAUTION );
		}

		m_ShockUpdated = true;
	}

	void Click_SetStamina( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateLastChangeTime();

		if ( m_Stamina.GetCurrent() >= 75 )
		{
			m_Stamina.SetColor( Colors.COLOR_PRISTINE );
			m_Stamina.SetAlpha( 1.0 );
		}
		else if ( m_Stamina.GetCurrent() >= 50 )
		{
			m_Stamina.SetColor( JMTheme.VALUE_OK );
		}
		else if ( m_Stamina.GetCurrent() >= 25 )
		{
			m_Stamina.SetColor( JMTheme.VALUE_CAUTION );
		}
		else
		{
			m_Stamina.SetColor( JMTheme.VALUE_BAD );
		}

		m_StaminaUpdated = true;
	}

	void Click_SetHeatBuffer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateLastChangeTime();

		if ( m_HeatBuffer.GetCurrent() >= 3 )
		{
			m_HeatBuffer.SetColor( Colors.COLOR_PRISTINE );
		}
		else if ( m_HeatBuffer.GetCurrent() >= 2 )
		{
			m_HeatBuffer.SetColor( Colors.COLOR_WORN );
		}
		else if ( m_HeatBuffer.GetCurrent() >= 1 )
		{
			m_HeatBuffer.SetColor( Colors.COLOR_WORN );
		}
		else
		{
			m_HeatBuffer.SetColor(0x00FFFFFF);
		}

		int heatbufferState = m_HeatBuffer.GetCurrent();
		m_HeatBuffer.SetText(m_HeatBufferStates[heatbufferState] +" "+ m_HeatBuffer.GetCurrent());

		m_HeatBufferUpdated = true;
		m_HeatBuffer.SetAlpha( 1.0 );
		
		// COLD
		if ( m_HeatComfort.GetCurrent() <= PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_CRITICAL )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_COLD_LVL_FOUR );
		}
		else if ( m_HeatComfort.GetCurrent() <= PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_WARNING )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_COLD_LVL_TWO );
		}
		else if ( m_HeatComfort.GetCurrent() <= PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_EMPTY )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_COLD_LVL_ONE );
		}
		// NEUTRAL
		else if ( m_HeatComfort.GetCurrent() > PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_EMPTY && m_HeatComfort.GetCurrent() > PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_WARNING )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_NEUTAL );
		}
		// HOT
		else if ( m_HeatComfort.GetCurrent() >= PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_EMPTY )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_HOT_LVL_ONE );
		}
		else if ( m_HeatComfort.GetCurrent() >= PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_WARNING )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_HOT_LVL_TWO );
		}
		else if ( m_HeatComfort.GetCurrent() >= PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_CRITICAL )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_HOT_LVL_FOUR );
		}
		m_HeatComfort.SetAlpha( 1.0 );
	}

	void Click_BloodyHands( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetBloodyHands( m_BloodyHands.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_GodMode( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetGodMode( m_GodMode.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_Freeze( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetFreeze( m_Freeze.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_SetBrokenLegs( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetBrokenLegs( m_BrokenLegs.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_SetReceiveDamageDealt( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetReceiveDamageDealt( m_ReceiveDmgDealt.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_CannotBeTargetedByAI( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetCannotBeTargetedByAI( m_CannotBeTargetedByAI.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_Invisible( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetInvisible( m_Invisibility.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_RemoveCollision( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetRemoveCollision( m_RemoveCollision.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_UnlimitedAmmo( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetUnlimitedAmmo( m_UnlimitedAmmo.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_AdminNVG( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetAdminNVG( m_AdminNVG.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_UnlimitedStamina( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetUnlimitedStamina( m_UnlimitedStamina.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	override void HideUI()
	{
		//! Do NOT concatenate "this" into a Print() here - that implicit
		//! object-to-string conversion inside this override crashes the real
		//! DayZ Tools script compiler natively at module load (confirmed by
		//! bisection: this file alone, with only this pattern active,
		//! reproduces the crash). The custom lint tool does not catch it.
		#ifdef COT_DEBUGLOGS
		Print( "[COT_DBG] JMPlayerForm::HideUI enter" );
		#endif

		ShowIdentityWidgets();

		m_RightPanelDisable.Show( true );

		m_SelectedInstance = NULL;

		//! Nothing in the right-hand panel means no row owns it either.
		m_LastSelectedGuid = "";
		ApplyListFocus();

		#ifdef COT_DEBUGLOGS
		Print( "[COT_DBG] JMPlayerForm::HideUI exit" );
		#endif
	}

	override void ShowUI()
	{
		#ifdef COT_DEBUGLOGS
		Print( "[COT_DBG] JMPlayerForm::ShowUI enter" );
		#endif

		m_RightPanelDisable.Show( false );

		#ifdef COT_DEBUGLOGS
		Print( "[COT_DBG] JMPlayerForm::ShowUI exit" );
		#endif
	}

	void UpdateUI()
	{
		if ( JM_GetSelected().NumPlayers(m_AutoSelect) == 0 )
		{
			HideUI();
			return;
		}

		m_AutoSelect = false;

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( JM_GetSelected().GetPlayers(false)[0] );
		
		if ( !instance )
		{
			HideUI();
			return;
		}

		if ( IsMissionOffline() )
		{
			instance.Update();
		}

		ShowUI();

		RegisterPermission( m_PositionX,             "Admin.Player.Teleport.Position"     );
		RegisterPermission( m_PositionY,             "Admin.Player.Teleport.Position"     );
		RegisterPermission( m_PositionZ,             "Admin.Player.Teleport.Position"     );
		RegisterPermission( m_Position,              "Admin.Player.Teleport.Position"     );
		RegisterPermission( m_TeleportToMe,          "Admin.Player.Teleport.Position"     );
		RegisterPermission( m_Health,                "Admin.Player.Set.Health"            );
		RegisterPermission( m_Blood,                 "Admin.Player.Set.Blood"             );
		RegisterPermission( m_Shock,                 "Admin.Player.Set.Shock"             );
		RegisterPermission( m_Energy,                "Admin.Player.Set.Energy"            );
		RegisterPermission( m_Water,                 "Admin.Player.Set.Water"             );
		RegisterPermission( m_Stamina,               "Admin.Player.Set.Stamina"           );
		RegisterPermission( m_HeatBuffer,            "Admin.Player.Set.HeatBuffer"        );
		RegisterPermission( m_BloodyHands,           "Admin.Player.Set.BloodyHands"       );
		RegisterPermission( m_GodMode,               "Admin.Player.Godmode"               );
		RegisterPermission( m_Freeze,                "Admin.Player.Freeze"                );
		RegisterPermission( m_Invisibility,          "Admin.Player.Invisibility"          );

		// The identity chips are those same three toggles plus shock, so they
		// answer to the same permissions - an admin who cannot set a flag must
		// not be handed a second way to try.
		RegisterPermission( m_BadgeGodMode,          "Admin.Player.Godmode"               );
		RegisterPermission( m_BadgeFrozen,           "Admin.Player.Freeze"                );
		RegisterPermission( m_BadgeInvisible,        "Admin.Player.Invisibility"          );
		RegisterPermission( m_BadgeUnconscious,      "Admin.Player.Set.Shock"             );
		RegisterPermission( m_UnlimitedAmmo,         "Admin.Player.UnlimitedAmmo"         );
		RegisterPermission( m_AdminNVG,              "Admin.Player.AdminNVG"              );
		RegisterPermission( m_UnlimitedStamina,      "Admin.Player.UnlimitedStamina"      );
		RegisterPermission( m_BrokenLegs,            "Admin.Player.BrokenLegs"            );
		RegisterPermission( m_ReceiveDmgDealt,       "Admin.Player.ReceiveDamageDealt"    );
		RegisterPermission( m_CannotBeTargetedByAI,  "Admin.Player.CannotBeTargetedByAI"  );
		RegisterPermission( m_RemoveCollision,       "Admin.Player.RemoveCollision"       );
		RegisterPermission( m_HealPlayer,            "Admin.Player.Heal"                  );
		RegisterPermission( m_RepairTransport,       "Admin.Transport.Repair"             );
		RegisterPermission( m_DryPlayer,             "Admin.Player.Dry"                   );
		RegisterPermission( m_SpectatePlayer,        "Admin.Player.Spectate"              );
		RegisterPermission( m_VomitPlayer,           "Admin.Player.Vomit"                 );
		RegisterPermission( m_SetScalePlayer,        "Admin.Player.Scale"                 );
		RegisterPermission( m_KillPlayer,            "Admin.Player.Set.Health"            );
		RegisterPermission( m_StripPlayer,           "Admin.Player.Strip"                 );
		RegisterPermission( m_KickPlayer,            "Admin.Player.Kick"                  );
		RegisterPermission( m_BanPlayer,             "Admin.Player.Ban"                   );

		//! Roles are edited in the Role Manager, so the jump is only offered to
		//! an admin who can actually open it.
		RegisterPermission( m_IdentityRoleEdit,      "Admin.Roles.View"                   );

		RegisterPermission( m_InventoryRefresh,      "Admin.Player.AccessInventory"       );
		RegisterPermission( m_InventoryClearCargo,   "Admin.Player.ClearCargo"            );

		RegisterPermission( m_DiseaseAgent,          "Admin.Player.Disease.Add"           );
		RegisterPermission( m_DiseaseAdd,            "Admin.Player.Disease.Add"           );
		RegisterPermission( m_DiseaseRemove,         "Admin.Player.Disease.Remove"        );
		RegisterPermission( m_DiseaseClear,          "Admin.Player.Disease.Remove"        );
		RegisterPermission( m_BleedingPart,          "Admin.Player.Bleed.Add"             );
		RegisterPermission( m_BleedApply,            "Admin.Player.Bleed.Add"             );
		RegisterPermission( m_BleedClear,            "Admin.Player.StopBleeding"          );

		if ( JM_GetSelected().NumPlayers(false) == 1 )
		{
			RegisterPermission( m_TeleportMeTo, "Admin.Player.Teleport.SenderTo" );
		} else if ( m_TeleportMeTo ) {
			m_TeleportMeTo.Disable();
		}

		ShowIdentityWidgets();
		m_GUID.SetButton( instance.GetGUID() );
		m_Name.SetButton( instance.GetName() );
		m_Steam64ID.SetButton( instance.GetSteam64ID() );

		if ( m_IdentityRole )
			m_IdentityRole.SetText( RoleDisplayName( GetPrimaryRole( instance ) ) );

		#ifdef GAMELABS
		m_CFToolsID.SetButton(instance.PlayerObject.GetUpstreamIdentity());
		#endif

		if ( IsMissionOffline() )
		{
			if ( m_TeleportToMe )
				m_TeleportToMe.Disable();
			if ( m_TeleportMeTo )
				m_TeleportMeTo.Disable();
			if ( m_SpectatePlayer )
				m_SpectatePlayer.Disable();
		}

		if ( m_SelectedInstance != instance )
		{
			m_SelectedInstance = instance;
		}

		// Push disease mask + bleeding state sync on selection change so dropdown
		// icons update for both panels.
		// UpdateUI is called from a lot more than a selection change -
		// OnClientPermissionsUpdated runs it every time the server pushes
		// permissions - so nothing below may fire an RPC unconditionally.
		// Unguarded, the Statistics panel rebuilt roughly once a second and read
		// as flashing, and the disease/bleeding requests went out just as often.
		string selectedGuid = instance.GetGUID();
		bool selectionChanged = ( selectedGuid != m_LastSelectedGuid );
		m_LastSelectedGuid = selectedGuid;

		if ( selectionChanged )
		{
			m_Module.RequestDiseaseMask( m_LastSelectedGuid );
			m_Module.RequestBleedingState( m_LastSelectedGuid );

			RecenterMap();

			// A NEW player's net ids have nothing to do with the last one's, so
			// last player's open containers cannot mean anything here.
			m_InvExpanded.Clear();

			// Only while the tab is up: every other selection change would
			// otherwise pull data nobody is looking at.
			if ( IsTabActive( TAB_INVENTORY ) )
				RequestInventory();

			// The history belongs to the player, not to the character, so it
			// has to be dropped rather than left showing the previous numbers.
			m_PlayerStats = NULL;

			if ( IsTabActive( TAB_STATISTICS ) )
				RequestStatistics();
			else
				RefreshStatisticsPanel();
		}

		ApplyListFocus();

		RefreshStats();
	}

	override void OnShow()
	{
		super.OnShow();

		g_Game.GetCallQueue( CALL_CATEGORY_GAMEPLAY ).CallLater( UpdatePlayerList, 1500, true );
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( RefreshStats, 100, true, false );
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateMapMarkers, 1000, true );
		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Insert( UpdateGroupAnimation );

		UpdateUI();

		UpdatePlayerList();
	}

	override void OnHide() 
	{
		super.OnHide();

		g_Game.GetCallQueue( CALL_CATEGORY_GAMEPLAY ).Remove( UpdatePlayerList );
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( RefreshStats );
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( UpdateMapMarkers );
		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Remove( UpdateGroupAnimation );

		// A MapWidget left visible behind a hidden form keeps rendering.
		if ( m_PositionMap )
			m_PositionMap.Hide();

		// Local preview entities are real objects in the world, invisible and
		// simulation-disabled but real. Leaking one per form open is not
		// acceptable.
		HideInventoryTooltip();
		CloseInventoryInspect();
		DestroyInventoryCellEntities();

		// The overlays that float outside the form - the prompt holds the game's
		// inputs down while it is up - are closed by super.OnHide() through the
		// registry, above.

		//! A form hidden mid-fold would come back with half-height rows.
		FinishGroupAnimation();
	}

	//! The position map's marker widgets need reprojecting onto its current
	//! pan/zoom every frame the tab is up - see UIActionMap.TickMarkers().
	override void Update()
	{
		super.Update();

		if ( m_PositionMap )
			m_PositionMap.TickMarkers();
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
		{
			return false;
		}

		return OnInventoryCellSelect( w );
	}

	//! Both buttons arrive here: OnClick never fires for the right one, and it
	//! is not guaranteed to fire for a plain panel on the left one either.
	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( OnInventoryCellPressed( w, button ) )
			return true;

		return super.OnMouseButtonDown( w, x, y, button );
	}

	//! The container toggle lands here rather than on the press - see
	//! OnInventoryCellPressed.
	override bool OnMouseButtonUp( Widget w, int x, int y, int button )
	{
		if ( OnInventoryCellReleased( w, button ) )
			return true;

		return super.OnMouseButtonUp( w, x, y, button );
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( OnInventoryCellHover( w ) )
			return true;

		return super.OnMouseEnter( w, x, y );
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		// A cell is several widgets, so the pointer crosses a boundary moving
		// within one. Only drop the tooltip when it has actually left the cell.
		if ( m_InvHoveredIndex >= 0 && InventoryCellIndexOf( enterW ) != m_InvHoveredIndex )
		{
			HideInventoryTooltip();
			return true;
		}

		return super.OnMouseLeave( w, enterW, x, y );
	}

	//! The position map's own double-click (teleport-to-cursor) is handled by
	//! OnClick_PositionMap via UIActionMap's UIEvent.DOUBLE_CLICK now, not here.
	override bool OnDoubleClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
			return false;

		return super.OnDoubleClick( w, x, y, button );
	}

	//! Mark the one row whose player fills the right-hand panel. Cheap enough to
	//! run over the whole pool: the list is a fixed 200 rows and this only fires
	//! on a selection change or a list rebuild, not per frame.
	private void ApplyListFocus()
	{
		foreach ( JMPlayerRowWidget row: m_PlayerList )
		{
			if ( !row )
				continue;

			row.SetFocused( m_LastSelectedGuid != "" && row.GetGUID() == m_LastSelectedGuid );
		}
	}

	void OnPlayer_Checked( string guid, bool checked )
	{
		for ( int i = 0; i < m_PlayerList.Count(); i++ )
		{
			if ( m_PlayerList[i].GetGUID() == guid )
			{
				m_PlayerList[i].SetChecked( checked );
			}
		}

		RefreshRoleHeaders();

		UpdateUI();

		UpdatePlayerCount();
	}

	void OnPlayer_Button( string guid, bool check )
	{
		for ( int i = 0; i < m_PlayerList.Count(); i++ )
		{
			if ( m_PlayerList[i].GetGUID() == guid )
			{
				m_PlayerList[i].SetChecked( check );
			} else
			{
				m_PlayerList[i].SetChecked( false );
			}
		}

		RefreshRoleHeaders();

		UpdateUI();

		UpdatePlayerCount();
	}

	//! Sorts by display name. array<T>.Sort() only knows how to order plain
	//! strings, not an object by a derived key, so the name list is sorted on
	//! its own and matched back to its owner afterward.
	//!
	//! Matched by CONSUMING one remaining player per sorted name, not by a bare
	//! string comparison against the whole list: two players sharing a display
	//! name (a fake test player happens to share a name with someone real, or
	//! any two real players named the same) used to match the SAME source index
	//! for both of them, which left one array slot written twice and another
	//! never written at all - a player silently dropped from the list, and no
	//! error anywhere to say why.
	private void SortPlayersArray( out array< JMPlayerInstance > players, bool isReversed )
	{
		int count = players.Count();

		TStringArray pNames = new TStringArray;
		for ( int i = 0; i < count; i++ )
			pNames.Insert( players[i].GetName() );

		pNames.Sort( isReversed );

		array< JMPlayerInstance > remaining = new array< JMPlayerInstance >;
		remaining.Copy( players );

		array< JMPlayerInstance > sorted = new array< JMPlayerInstance >;

		for ( i = 0; i < count; i++ )
		{
			string wantName = pNames[i];

			for ( int j = 0; j < remaining.Count(); j++ )
			{
				if ( !remaining[j] )
					continue;

				if ( remaining[j].GetName() != wantName )
					continue;

				sorted.Insert( remaining[j] );
				remaining[j] = NULL;
				break;
			}
		}

		players.Clear();
		players.Copy( sorted );
	}

	void UpdatePlayerCount()
	{
		//! The roster, not the local entity list: this is the same set of players
		//! the list below is built from, so the two can never disagree.
		int online;
		JMPermissionManager permissions = GetPermissionsManager();
		if ( permissions )
			online = permissions.GetPlayers().Count();

		if ( online > s_PeakPlayerCount )
			s_PeakPlayerCount = online;

		string onlineText = "" + online;
		if ( m_MaxPlayers > 0 )
			onlineText = onlineText + " / " + m_MaxPlayers;

		m_PlayerListCount.SetText( onlineText );
		m_PlayerListPeak.SetText( "" + s_PeakPlayerCount );
		m_PlayerListSelected.SetText( "" + JM_GetSelected().GetPlayers(false).Count() );
	}

	void SelectAllPlayerList(bool state = true)
	{
		//! The filtered roster rather than the visible rows: a collapsed group
		//! has no rows on screen, and "select all" that quietly skipped it would
		//! be selecting less than the list says it is holding.
		foreach ( string memberRole, TStringArray memberGuids : m_RoleMembers )
		{
			foreach ( string guid : memberGuids )
			{
				if ( guid == string.Empty )
					continue;

				if ( state )
					JM_GetSelected().AddPlayer( guid );
				else
					JM_GetSelected().RemovePlayer( guid );
			}
		}

		foreach ( JMPlayerRowWidget row: m_PlayerList )
		{
			if ( !row || row.IsHeader() || row.GetGUID() == string.Empty )
				continue;

			row.SetChecked( state );
		}

		RefreshRoleHeaders();

		UpdateUI();
		UpdatePlayerCount();
	}

	// -------------------------------------------------------------------------
	//  Role grouping
	//
	//  The list is grouped by role: a header per role, its members underneath,
	//  and the header collapses the group. Grouping happens after filtering, so
	//  a header's count is the number of rows actually under it and a group
	//  nothing matched does not show up at all.
	// -------------------------------------------------------------------------

	//! Every player carries "everyone", so anything past it is a role somebody
	//! granted on purpose. A player with nothing else granted groups under
	//! "everyone" itself rather than under a made-up bucket name.
	static const string ROLE_UNGROUPED = "everyone";

	//! Open the Role Manager on the selected player.
	//!
	//! The window is created by the module, exactly as the sidebar creates it,
	//! so an already-open Role Manager is reused rather than duplicated - and
	//! the deep link is handed over AFTER Show(), because the form does not
	//! exist until the window has built it.
	void Click_EditPlayerRoles( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_SelectedInstance )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Roles.View" ) )
			return;

		JMRoleManagerModule roleModule;
		if ( !Class.CastTo( roleModule, GetModuleManager().GetModule( JMRoleManagerModule ) ) )
			return;

		roleModule.Show();

		JMRoleManagerForm roleForm;
		if ( !Class.CastTo( roleForm, roleModule.GetForm() ) )
			return;

		roleForm.OpenPlayer( m_SelectedInstance.GetGUID() );
	}

	private string GetPrimaryRole( JMPlayerInstance player )
	{
		array< string > roles = player.GetRoles();

		//! Do not assume "everyone" sits at index 0 - a hand-edited player
		//! file (Roles written directly, not through the Role Manager UI)
		//! can list roles in any order or omit "everyone" altogether.
		if ( roles )
		{
			for ( int i = 0; i < roles.Count(); i++ )
			{
				if ( roles[i] != ROLE_UNGROUPED )
					return roles[i];
			}
		}

		return ROLE_UNGROUPED;
	}

	//! Roles are stored lowercase because that is how the permission files name
	//! them; a header is a heading, so it gets a capital.
	private string RoleDisplayName( string role )
	{
		if ( role.Length() == 0 )
			return role;

		string head = role.Substring( 0, 1 );
		head.ToUpper();

		return head + role.Substring( 1, role.Length() - 1 );
	}

	bool IsRoleCollapsed( string role )
	{
		return m_CollapsedRoles.Find( role ) >= 0;
	}

	//! Does the role itself grant the COT permission? A player row's shield
	//! means "granted to this person"; the same shield on a header means every
	//! player under it is staff by virtue of being in the group.
	private bool IsAdminRole( string role )
	{
		JMRole roleData = GetPermissionsManager().GetRole( role );
		if ( !roleData )
			return false;

		JMPermissionType permType;
		return roleData.HasPermission( JMPlayerRowWidget.PERMISSION_ADMIN, permType );
	}

	private bool IsRoleFullySelected( string role )
	{
		TStringArray guids;
		if ( !m_RoleMembers.Find( role, guids ) )
			return false;

		if ( guids.Count() == 0 )
			return false;

		foreach ( string guid : guids )
		{
			if ( !JM_GetSelected().IsSelected( guid ) )
				return false;
		}

		return true;
	}

	//! Re-derive every header's box from the selection set. A member row
	//! toggling does not know which header aggregates it, so the headers are
	//! recomputed rather than kept in step incrementally.
	private void RefreshRoleHeaders()
	{
		foreach ( JMPlayerRowWidget headerRow: m_PlayerList )
		{
			if ( !headerRow )
				continue;

			if ( !headerRow.IsHeader() )
				continue;

			headerRow.SetChecked( IsRoleFullySelected( headerRow.GetRole() ) );
		}
	}

	//! Called by a header row's chevron, or by a click anywhere else on it.
	//!
	//! Expanding and collapsing are not symmetrical. To expand, the rows have to
	//! be built before they can be animated in, so the state flips first and the
	//! rebuild runs before the animation. To collapse, the rows have to survive
	//! until the animation is over, so the state flips at the END - see
	//! FinishGroupAnimation.
	void OnRoleHeader_Toggled( string role )
	{

		//! A second click while the last fold is still running would leave the
		//! first one's rows stranded at whatever height they had reached.
		FinishGroupAnimation();

		int at = m_CollapsedRoles.Find( role );

		if ( at >= 0 )
		{
			m_CollapsedRoles.Remove( at );
			UpdatePlayerList();
			BeginGroupAnimation( role, true );
			return;
		}

		BeginGroupAnimation( role, false );
	}

	// -------------------------------------------------------------------------
	//  Fold animation
	//
	//  The rows sit in a GridSpacer, which lays them out by their size - so
	//  driving a row's height to zero IS the animation: everything below it
	//  slides up to fill the gap. Alpha rides along so a half-height row does
	//  not read as a clipped one.
	// -------------------------------------------------------------------------

	static const float GROUP_ANIM_DURATION = 0.16;

	private bool IsGroupAnimating()
	{
		return m_AnimRole != "";
	}

	private void BeginGroupAnimation( string role, bool expanding )
	{
		m_AnimRows.Clear();
		m_AnimHeader = NULL;
		m_AnimRole = role;
		m_AnimExpanding = expanding;
		m_AnimTime = 0;

		TStringArray guids;
		m_RoleMembers.Find( role, guids );

		foreach ( JMPlayerRowWidget row: m_PlayerList )
		{
			if ( !row )
				continue;

			if ( row.IsHeader() )
			{
				if ( row.GetRole() == role )
					m_AnimHeader = row;

				continue;
			}

			if ( !guids )
				continue;

			if ( guids.Find( row.GetGUID() ) < 0 )
				continue;

			m_AnimRows.Insert( row );
		}

		//! Nothing to fold - an empty group still has to flip its chevron.

		if ( m_AnimRows.Count() == 0 )
		{
			FinishGroupAnimation();
			return;
		}

		if ( expanding )
			ApplyGroupAnimation( 0 );
		else
			ApplyGroupAnimation( 1.0 );
	}

	private void ApplyGroupAnimation( float t )
	{
		foreach ( JMPlayerRowWidget row: m_AnimRows )
		{
			if ( row )
				row.SetRevealProgress( t );
		}

		if ( m_AnimHeader )
			m_AnimHeader.SetChevronProgress( t );
	}

	//! Snap to the end state and commit it. Safe to call when nothing is
	//! running, which is what makes it usable as an "interrupt whatever is
	//! going on" from OnHide and from a second click.
	private void FinishGroupAnimation()
	{
		if ( !IsGroupAnimating() )
			return;

		string role = m_AnimRole;
		bool wasExpanding = m_AnimExpanding;

		m_AnimRole = "";
		m_AnimHeader = NULL;
		m_AnimRows.Clear();
		m_AnimTime = 0;

		if ( wasExpanding )
		{
			//! Rows are already in the list; all the animation was doing was
			//! growing them into place.
			UpdatePlayerList();
			return;
		}

		if ( m_CollapsedRoles.Find( role ) < 0 )
			m_CollapsedRoles.Insert( role );

		UpdatePlayerList();
	}

	void UpdateGroupAnimation( float timeSlice )
	{
		if ( !IsGroupAnimating() )
			return;

		m_AnimTime += timeSlice;

		float t = m_AnimTime / GROUP_ANIM_DURATION;

		if ( t >= 1.0 )
		{
			FinishGroupAnimation();
			return;
		}

		if ( m_AnimExpanding )
			ApplyGroupAnimation( t );
		else
			ApplyGroupAnimation( 1.0 - t );
	}

	//! Called by a header row's checkbox. Acts on the members the header counted,
	//! which is the filtered set - not on every holder of the role.
	void OnRoleHeader_Checked( string role, bool checked )
	{
		TStringArray guids;
		if ( !m_RoleMembers.Find( role, guids ) )
			return;

		foreach ( string guid : guids )
		{
			if ( checked )
				JM_GetSelected().AddPlayer( guid );
			else
				JM_GetSelected().RemovePlayer( guid );
		}

		UpdateUI();
		UpdatePlayerList();
	}

	//! The pool is split across Content_Row_0N grids of 100 rows each, and a grid
	//! is only revealed once the emit actually reaches it. Returns false when the
	//! grid a row would need does not exist, which is the end of the pool.
	private bool RevealRowBlock( int entryId, inout int contentID )
	{
		if ( entryId % 100 != 0 )
			return true;

		GridSpacerWidget spacer;
		if ( !Class.CastTo( spacer, m_PlayerListRows.FindAnyWidget( "Content_Row_0" + contentID ) ) )
			return false;

		spacer.Show( true );
		contentID++;

		return true;
	}

	void UpdatePlayerList()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "UpdatePlayerList" );
		#endif

		if ( !IsMissionOffline() )
			GetCommunityOnlineTools().RefreshClients();

		//! A rebuild resets every row to its resting height, which would cut a
		//! fold off halfway. The 1500ms refresh can wait out the ~160ms fold.
		if ( IsGroupAnimating() )
			return;

		int contentID;
		GridSpacerWidget parentSpacer;
		while ( Class.CastTo( parentSpacer, m_PlayerListRows.FindAnyWidget( "Content_Row_0" + contentID ) ) )
		{
			contentID++;
			parentSpacer.Show( false );
		}

		bool isReversed = m_PlayerListSort.IsToggled();

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();
		SortPlayersArray( players, isReversed );

		string closestMatch;

		bool isFiltering;
		COT_String strSearch = m_PlayerListFilter.GetText();
		bool requireAllKeywords;
		TStringArray keywords = strSearch.KeywordSearch_Prepare(requireAllKeywords);
		if (strSearch != string.Empty)
			isFiltering = true;

		//! Bucket the survivors of the filter by role, remembering the order the
		//! roles were first seen so the sort below has something to sort.
		TStringArray roleOrder = new TStringArray;
		map< string, ref array< JMPlayerInstance > > groups = new map< string, ref array< JMPlayerInstance > >;

		m_RoleMembers.Clear();

		array< JMPlayerInstance > bucket;
		TStringArray bucketGuids;

		//! One row per guid. Rows are keyed by guid everywhere downstream - the
		//! checkbox, the focus bar and the row menu all match on it - so a guid
		//! emitted twice produced rows that ticked, focused and acted as one,
		//! which reads as the list being broken rather than as a roster with a
		//! duplicate in it.
		map< string, bool > seenGuids = new map< string, bool >;

		foreach ( JMPlayerInstance cPlayer: players )
		{
			string cGuid = cPlayer.GetGUID();

			if ( cGuid == "" || seenGuids.Contains( cGuid ) )
				continue;

			seenGuids.Insert( cGuid, true );

			COT_String pName = cPlayer.GetName();
			pName.ToLower();

			if ( isFiltering )
			{
				if ( !pName.KeywordSearchImplEx( strSearch, keywords, requireAllKeywords, closestMatch ) )
					continue;
			}

			if ( !PassesListFilters( cPlayer ) )
				continue;

			string role = GetPrimaryRole( cPlayer );

			if ( !groups.Find( role, bucket ) )
			{
				bucket = new array< JMPlayerInstance >;
				groups.Insert( role, bucket );
				roleOrder.Insert( role );

				bucketGuids = new TStringArray;
				m_RoleMembers.Insert( role, bucketGuids );
			}

			m_RoleMembers.Find( role, bucketGuids );

			bucket.Insert( cPlayer );
			bucketGuids.Insert( cPlayer.GetGUID() );
		}

		roleOrder.Sort( isReversed );

		//! You are always the first row of your own category, whichever way the
		//! sort is pointing. An admin acting on themselves should not have to
		//! hunt their own name out of sixty, and the alphabetical order that
		//! decides everyone else has no opinion about that.
		//!
		//! The guid list is reordered in step: the header's "all selected" test
		//! reads it positionally against the bucket.
		string selfGUID = GetPermissionsManager().GetClientGUID();
		if ( selfGUID != "" )
		{
			array< JMPlayerInstance > selfBucket;
			TStringArray selfGuids;

			foreach ( string selfRole: roleOrder )
			{
				selfBucket = groups.Get( selfRole );
				if ( !selfBucket )
					continue;

				int selfIdx = -1;
				for ( int si = 0; si < selfBucket.Count(); si++ )
				{
					if ( selfBucket[si].GetGUID() == selfGUID )
					{
						selfIdx = si;
						break;
					}
				}

				if ( selfIdx <= 0 )
					continue;

				JMPlayerInstance selfInstance = selfBucket[selfIdx];
				selfBucket.Remove( selfIdx );
				selfBucket.InsertAt( selfInstance, 0 );

				if ( m_RoleMembers.Find( selfRole, selfGuids ) && selfIdx < selfGuids.Count() )
				{
					selfGuids.Remove( selfIdx );
					selfGuids.InsertAt( selfGUID, 0 );
				}

				break;
			}
		}

		//! Every pool row starts blank, so anything the emit below does not reach
		//! is hidden rather than left showing whoever it carried last time.
		int entryId;
		int maxThreshold = m_PlayerList.Count();

		while ( entryId < maxThreshold )
		{
			m_PlayerList[entryId].SetPlayer( "" );
			entryId++;
		}

		entryId = 0;
		contentID = 0;

		foreach ( string roleName: roleOrder )
		{
			if ( entryId >= maxThreshold )
				break;

			if ( !RevealRowBlock( entryId, contentID ) )
				break;

			array< JMPlayerInstance > members = groups.Get( roleName );
			bool collapsed = IsRoleCollapsed( roleName );

			//! Negated into its own local rather than passed as `!collapsed`.
			//! Enforce did not carry the negation through the argument list - the
			//! header was emitted with collapsed=true and arrived at the row with
			//! expanded=true, which is why a collapsed group kept a down arrow.
			bool expanded = true;
			if ( collapsed )
				expanded = false;

			m_PlayerList[entryId].SetRoleHeader( roleName, RoleDisplayName( roleName ), members.Count(), expanded, IsRoleFullySelected( roleName ), IsAdminRole( roleName ) );
			entryId++;

			if ( collapsed )
				continue;

			foreach ( JMPlayerInstance member: members )
			{
				if ( entryId >= maxThreshold )
					break;

				if ( !RevealRowBlock( entryId, contentID ) )
					break;

				m_PlayerList[entryId].SetPlayer( member.GetGUID() );
				entryId++;
			}
		}

		m_PlayerListFilter.SetTextPreview(closestMatch);

		//! Rows are a reused pool, so the row that carried the focused player
		//! before this rebuild is probably showing somebody else now.
		ApplyListFocus();

		UpdatePlayerCount();

		m_PlayerListScroller.UpdateScroller();
	}

	override void OnFocus()
	{
		//CF_DumpWidgets( m_LeftPanel );
	}

	override void OnUnfocus()
	{

	}

	void UpdateLastChangeTime()
	{
		m_LastChangeTime = g_Game.GetTime();
	}

}