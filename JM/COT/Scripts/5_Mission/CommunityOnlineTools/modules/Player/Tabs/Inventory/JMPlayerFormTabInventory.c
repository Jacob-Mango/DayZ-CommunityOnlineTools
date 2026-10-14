//! "Inventory" tab of JMPlayerForm - the selected player's equipment as a tree
//! of cells with previews, a context menu and item prompts. Back-reference to
//! the owning form, same shape as JMPlayerRowWidget.Menu.
class JMPlayerFormTabInventory: JMFormTab
{
	protected JMPlayerForm m_Form;

	// -- Inventory tab -----------------------------------------------------
	//! Preview entities and their spawn classnames, indexed by ITEM index (position
	//! in m_InventoryItems, not on screen), so expanding a container moves none.
	ref array<EntityAI> m_InvCellEntities;
	ref array<string>   m_InvCellTypes;

	//! The cells currently on screen, and the item index each one is showing.
	//! Parallel, both rebuilt whenever the tree opens or closes.
	ref array<Widget> m_InvCells;
	ref array<int>    m_InvCellItem;

	//! Item indices of the containers the admin has opened. A container is
	//! closed until asked for, so a full loadout opens as a dozen cells rather
	//! than eighty.
	ref array<int> m_InvExpanded;

	//! Cells and bands are positioned by hand onto this panel. See
	//! LayoutInventoryLevel for why it is a bare canvas and not a spacer.
	Widget m_InventoryCanvas;

	//! Columns the last layout pass fitted, and the width it fitted them into.
	//! A resize only has to rebuild when one of these actually moves.
	int   m_InvColumns;
	float m_InvLayoutWidth;

	//! Drawn size of one cell, in screen pixels, and that against the size the
	//! layout declares. Everything the layout pass computes is in screen pixels
	//! - see MeasureInventoryCell for why it cannot be in anything else.
	float m_InvCellPixels;
	float m_InvScale;
	int m_InvSelectedIndex;
	int m_InvHoveredIndex;

	//! Preview entities spawned during the current rebuild. The budget is per
	//! rebuild, not per level, so a deep tree cannot walk past it.
	int m_InvPreviewsBuilt;
	UIActionContextMenu m_InventoryMenu;
	UIActionValuePrompt m_InventoryPrompt;

	//! The container "Delete all" was clicked on. The confirmation is answered
	//! later and the table can be rebuilt in the meantime, so the container is
	//! remembered by its network ID, not by its row.
	int m_InvGroupPendingLow;
	int m_InvGroupPendingHigh;
	string m_InvGroupPendingName;

	// -- Inventory item preview --------------------------------------------
	//! Vanilla's own hover tooltip layout, driven by the same static
	//! InspectMenuNew.UpdateItemInfo the inventory screen uses.
	Widget            m_InvTooltip;
	ItemPreviewWidget m_InvTooltipPreview;

	//! Client-local stand-ins for the hovered and the inspected item. See
	//! SpawnLocalPreview for why these are copies and not the real entities.
	EntityAI m_InvHoverEntity;
	string   m_InvHoverType;
	EntityAI m_InvInspectEntity;
	bool     m_InvInspectOpen;

	//! Only ever says that the server stopped listing - the per-item count it
	//! used to carry is what the grid itself shows.
	UIActionText m_InventoryNotice;
	UIActionImageButton m_InventoryRefresh;
	UIActionImageButton m_InventoryClearCargo;

	//! The survivor's equipment slots, in the order the vanilla inventory draws
	//! them, with the ghost icon each empty one shows. Read once out of config;
	//! see EnsureInventorySlotTable.
	ref array<string> m_InvSlotNames;
	ref array<string> m_InvSlotIcons;
	bool m_InvSlotsBuilt;

	//! The listing the table was last built from. Row index maps straight into
	//! this, so the selected row IS the item handle on the client side.
	ref array< ref JMPlayerInventoryItem > m_InventoryItems;

	//! Row the context menu was opened on, captured at open time: the delete
	//! confirmation is asynchronous and the selection can move under it.
	int m_InventoryPendingRow;

	//! The item an open prompt is about, held as its network ID rather than as
	//! a row: answering a prompt takes as long as the admin wants, and the
	//! listing behind it can be rebuilt by any refresh in the meantime, which
	//! would leave a row index pointing at somebody else's boots.
	int m_InvPromptNetLow;
	int m_InvPromptNetHigh;

	//! A root-level layout entry below zero is an EMPTY equipment slot rather
	//! than an item. Slot i is encoded as INV_GHOST_BASE - i, which leaves -1
	//! meaning what it always meant: no item.
	static const int INV_GHOST_BASE = -2;
	static const string INV_MENU_INSPECT = "inspect";
	static const string INV_MENU_TAKE   = "take";
	static const string INV_MENU_DELETE = "delete";

	//! Item-specific entries, listed only when the row can take them (a greyed
	//! "Set Liquid" on every rifle is noise). Doubles as the prompt id the confirm
	//! callback branches on: one entry raises one prompt.
	static const string INV_MENU_UNJAM       = "unjam";
	static const string INV_MENU_QUANTITY    = "quantity";
	static const string INV_MENU_HEALTH      = "health";
	static const string INV_MENU_TEMPERATURE = "temperature";
	static const string INV_MENU_STATE       = "state";
	static const string INV_MENU_LIQUID      = "liquid";

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

	//! Header action size. Smaller than the cards' HEADER_ACTION_PX: three 28px
	//! buttons would be taller than the band strip they label.
	static const int INV_BAND_ACTION_PX = 22;

	//! Glyph size in those buttons. The pill's image is an EXACT 16px in the layout
	//! and does NOT scale, so shrunk buttons need a smaller glyph; near 16 in 28.
	static const int INV_BAND_ICON_PX = 12;

	//! How a cell prints its quantity. See InventoryQuantityMode.
	static const int INV_QTY_HIDDEN = 0;
	static const int INV_QTY_COUNT  = 1;
	static const int INV_QTY_BAR    = 2;

	//! Direct children this tab's builder adds to its panel, with headroom. Over-
	//! provisioning is free under Size-To-Content-V; running short silently clips the
	//! last rows with no error anywhere. The form sizes the panel from this.
	static const int PANEL_ROWS = 6;

	void JMPlayerFormTabInventory( JMPlayerForm form )
	{
		m_Form = form;

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
	}

	//! Does anything in the listing name this item as its parent?
	bool HasInventoryChildren( int index )
	{
		foreach ( JMPlayerInventoryItem child : m_InventoryItems )
		{
			if ( child.ParentIndex == index )
				return true;
		}

		return false;
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		InitActionWidgetsInventory( panel );
	}

	override void OnFocus()
	{
		RequestInventory();
	}

	override void OnUnfocus()
	{
		HideInventoryTooltip();
	}

	override void OnDestruct()
	{
		ReleaseLocalEntities();
	}

	//! The cell grid is positioned by hand, so a resize is the one thing it
	//! cannot ride out - but only a resize that changes how many cells fit
	//! per row is worth rebuilding for.
	override void OnResize( float w, float h )
	{
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

	//! The form was hidden. Local preview entities are real objects in the world,
	//! invisible and simulation-disabled but real. Leaking one per form open is
	//! not acceptable.
	void OnFormHide()
	{
		HideInventoryTooltip();
		CloseInventoryInspect();
		DestroyInventoryCellEntities();
	}

	//! Everything that outlives the widgets: the tooltip is parented to the
	//! workspace, not to the form, and the previews are world entities. Safe to
	//! call more than once.
	void ReleaseLocalEntities()
	{
		if ( m_InvTooltip )
		{
			m_InvTooltip.Unlink();
			m_InvTooltip = null;
		}

		DestroyInventoryCellEntities();
		DestroyLocalPreview( m_InvHoverEntity );
		DestroyLocalPreview( m_InvInspectEntity );
	}

	void BindPermissions()
	{
		m_Form.BindPermission( m_InventoryRefresh,      JMConstants.PERM_PLAYER_ACCESSINVENTORY       );
		m_Form.BindPermission( m_InventoryClearCargo,   JMConstants.PERM_PLAYER_CLEARCARGO            );
	}

	//! A NEW player's net ids have nothing to do with the last one's, so
	//! last player's open containers cannot mean anything here.
	void OnSelectionChanged()
	{
		m_InvExpanded.Clear();

		// Only while the tab is up: every other selection change would
		// otherwise pull data nobody is looking at.
		if ( m_Form.IsTabActive( GetTabId() ) )
			RequestInventory();
	}

	//! A cell is several widgets, so the pointer crosses a boundary moving
	//! within one. Only drop the tooltip when it has actually left the cell.
	bool OnMouseLeaveCell( Widget enterW )
	{
		if ( m_InvHoveredIndex >= 0 && InventoryCellIndexOf( enterW ) != m_InvHoveredIndex )
		{
			HideInventoryTooltip();
			return true;
		}

		return false;
	}

	//! Inventory tab: a table of everything carried, right-click menu per row.
	//! No 2D item icon exists, so a picture needs an ItemPreviewWidget on a
	//! client-local entity; one per row for 80 items on every refresh costs frame
	//! time. Nesting is shown by indenting the name.
	Widget InitActionWidgetsInventory( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 6, 1 );

		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_INVENTORY" );
		Widget section0 = section0Card.GetContent();

		// Both act on the whole listing, so they sit in the card title bar (a
		// toolbar row cost a line of height). Refresh first: the strip packs right,
		// so the destructive one ends up on the outside edge.
		m_InventoryRefresh = section0Card.AddRefreshButton( this, "Click_RefreshInventory", "#STR_COT_PLAYER_MODULE_TT_INV_REFRESH" );

		// ClearCargo belongs on this tab; Strip stays with the destructive actions.
		// Icon-only, so its caption is the tooltip. Always targets m_SelectedInstance,
		// not the player list's checkbox selection.
		m_InventoryClearCargo = section0Card.AddDeleteButton( this, "Click_ClearCargoOnly", "#STR_COT_PLAYER_MODULE_TT_INV_CLEAR_CARGO" );
		m_InventoryClearCargo.SetColor( JMTheme.DANGER_FILL );


		// A wrap spacer, not a fixed-column grid: the form is resizable and it
		// reflows cells like vanilla. It sizes to content vertically, so the tab's
		// scroller handles overflow.
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
		m_InventoryMenu = UIActionManager.CreateOverlayMenu( m_Form, this, "OnClick_InventoryMenu" );

		// Same anchoring, same reason: the prompt the menu raises has to float
		// over the tab rather than scroll with it.
		m_InventoryPrompt = UIActionManager.CreateOverlayPrompt( m_Form, this, "OnConfirm_InventoryPrompt" );

		// Vanilla's inventory tooltip, filled by static InspectMenuNew.UpdateItemInfo
		// (no ItemManager or open inventory menu needed). Created at the workspace
		// TOP, not in the COT window: as a child it was clipped at the window edge
		// and drifted when dragged; it follows the cursor so needs screen space.
		m_InvTooltip = g_Game.GetWorkspace().CreateWidgets( "gui/layouts/inventory_new/day_z_inventory_new_tooltip.layout", NULL );
		if ( m_InvTooltip )
		{
			m_InvTooltip.Show( false );
			m_InvTooltip.SetSort( 9998 );
			Class.CastTo( m_InvTooltipPreview, m_InvTooltip.FindAnyWidget( "ItemFrameWidget" ) );
		}

		return parent;
	}

	//! Ask the server for a fresh listing of the selected player. Called on tab
	//! entry, selection change, refresh and after an operation - never on a timer
	//! (per-admin per-second listings are real bandwidth for a maybe-closed panel).
	void RequestInventory()
	{
		if ( !m_Form.m_Module || !m_Form.m_SelectedInstance )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_ACCESSINVENTORY ) )
			return;

		m_Form.m_Module.RequestInventory( m_Form.m_SelectedInstance.GetGUID() );
	}

	//! Server pushed a listing. Dropped when it is for someone else: the
	//! selection can move while the request is in flight.
	void OnInventoryUpdated( string guid, array< ref JMPlayerInventoryItem > items, bool truncated )
	{
		if ( !m_Form.m_SelectedInstance || guid != m_Form.m_SelectedInstance.GetGUID() )
			return;

		// Item indices only mean something within one listing, so open containers
		// are carried across by NET ID (stable unless the tree changed). Else any
		// edit inside an open bag or vest would close it again.
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

	//! Rebuild the cell grid from the cached listing. Cells are recreated but
	//! PREVIEW ENTITIES are reused, respawned only when an index's content changes
	//! (else a refresh recreates up to 96 objects for a barely changed listing).
	void RebuildInventoryTable( bool truncated )
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
		UIActionManager.ClearChildren( m_InventoryCanvas );

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
			m_InventoryNotice.SetTextFormat( "#STR_COT_PLAYER_MODULE_INV_TRUNCATED" , m_InventoryItems.Count().ToString() );

		m_InventoryNotice.GetLayoutRoot().Show( truncated );
	}

	//! Drawn width of the cell area, in screen pixels. Falls back to the parent, then
	//! a default: a listing can arrive before first layout and zero width would
	//! collapse the grid to one column.
	float InventoryCanvasWidth()
	{
		float w, h;
		m_InventoryCanvas.GetScreenSize( w, h );

		if ( w < m_InvCellPixels && m_InventoryCanvas.GetParent() )
			m_InventoryCanvas.GetParent().GetScreenSize( w, h );

		if ( w < m_InvCellPixels )
			w = m_InvCellPixels * 6;

		return w;
	}

	//! Drawn width of one cell, in screen pixels. The layout pass uses screen pixels
	//! throughout (SetScreenPos/SetScreenSize): stepping by the DECLARED 72 while
	//! the engine draws at the UI-SCALED width made cells overlap.
	float MeasureInventoryCell()
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

	int InventoryColumnsFor( float width )
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
	void DumpPreviewDiagnostics( int previews )
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

		// Tells apart: size 0x0 or item=NULL (nothing to draw, our bug); sane size and
		// item set (drawn but covered or out of frame); invisible/alpha 0 ancestor
		// (hides it whatever the leaf says). So print the parent chain.
		Widget walk = first.GetParent();
		int depth = 0;

		while ( walk && depth < 8 )
		{
			Print( "[COT-PREVIEW]   parent[" + depth + "] " + walk.GetName() + " visible=" + walk.IsVisible().ToString() + " alpha=" + walk.GetAlpha() );

			walk = walk.GetParent();
			depth++;
		}
	}

	//! Place one level of the tree, answering its height. Hand-positioned on a flat
	//! canvas (a spacer sized a band against the whole panel). Bands are created
	//! BEFORE their cells to stay behind; only opened containers are descended into,
	//! and the depth guard covers a looping ParentIndex chain.
	float LayoutInventoryLevel( int parentIndex, float originX, float originY, float width, int depth )
	{
		if ( depth > INV_MAX_DEPTH )
			return 0;

		int columns = InventoryColumnsFor( width );

		float y   = originY;
		int   col = 0;

		array<int> order = InventoryLevelOrder( parentIndex );

		// Two passes: a band right under its container would split the grid (at the
		// root, the equipment slot layout) as soon as one is opened. So all cells go
		// first and open containers spill out under the full grid, in grid order.
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

	//! What one level draws, in order. Below the root: listing order (the server
	//! walked PREORDER). The ROOT is by SLOT in vanilla order, since an EMPTY slot
	//! ("no vest") is information: worn item per slot, else its ghost icon. Items
	//! the slot table misses (hands, mod slots) follow rather than being dropped.
	array<int> InventoryLevelOrder( int parentIndex )
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

	//! Index of the root-level item worn in this slot, or -1. Matched on slot ID, not
	//! strings (listing names come from InventorySlots, the table's from CfgSlots),
	//! so case differences cannot break it.
	int InventoryItemInSlot( string slotName )
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

	//! Read the survivor's equipment slots out of config, once. playerSlots matches
	//! the vanilla inventory's order, and a mod-added slot appears for free.
	void EnsureInventorySlotTable()
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

	//! An empty equipment slot: its ghost glyph on a sunken tile. Inserted with item
	//! index -1, which InventoryCellIndexOf handlers treat as "not a cell", so it
	//! cannot be selected, opened, hovered or acted on, with no extra guard.
	Widget BuildInventoryGhostCell( int slotIndex )
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

	void HideInventoryCellPart( Widget cell, string name )
	{
		Widget part = cell.FindAnyWidget( name );
		if ( part )
			part.Show( false );
	}

	//! The ground under an open container: everything that item is holding, on
	//! its own tinted panel, directly below the cell it came out of. Answers the
	//! height it used, the gap after it included.
	float LayoutInventoryBand( int index, float x, float y, float width, int depth )
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

	//! The three whole-band operations in the band header; they act on what the band
	//! DRAWS (the container's direct contents). Real UIActionImageButtons for hover,
	//! border and tooltip. Created per rebuild (the canvas destroys them each
	//! refresh). Without permission they are hidden, not greyed: a dead glyph reads as broken.
	void BuildInventoryBandActions( Widget band, int index )
	{
		//! Packed right, centred. A right-aligned run lays out inwards, so the FIRST
		//! button is rightmost: delete is first to sit on the outside edge, as every
		//! destructive control in COT does.
		WrapSpacerWidget host;
		if ( !Class.CastTo( host, band.FindAnyWidget( "group_actions" ) ) )
			return;

		host.SetContentAlignmentH( WidgetAlignment.WA_RIGHT );
		host.SetContentAlignmentV( WidgetAlignment.WA_CENTER );

		if ( JMPermissions.Has( JMConstants.PERM_PLAYER_INVENTORY_DELETE ) )
		{
			UIActionImageButton wipe = UIActionManager.CreateDeleteButton( host, this, "Click_BandDeleteAll", "#STR_COT_PLAYER_MODULE_TT_INV_GROUP_DELETE" );

			//! CreateDeleteButton uses the bright DANGER accent (for text/rings); like
			//! every delete button in the mod, override to the deeper DANGER_FILL.
			wipe.SetColor( JMTheme.DANGER_FILL );

			PrepareInventoryBandAction( wipe, index, "" );
		}

		if ( JMPermissions.Has( JMConstants.PERM_PLAYER_INVENTORY_REPAIR ) )
		{
			UIActionImageButton repair = UIActionManager.CreateIconButton( host, JMConstants.Lucide( "wrench" ), this, "Click_BandRepairAll" );
			PrepareInventoryBandAction( repair, index, "#STR_COT_PLAYER_MODULE_TT_INV_GROUP_REPAIR" );
		}

		if ( JMPermissions.Has( JMConstants.PERM_PLAYER_INVENTORY_TAKE ) )
		{
			UIActionImageButton take = UIActionManager.CreateIconButton( host, JMConstants.Lucide( "hand" ), this, "Click_BandTakeAll" );
			PrepareInventoryBandAction( take, index, "#STR_COT_PLAYER_MODULE_TT_INV_GROUP_TAKE" );
		}
	}

	//! Size the button to the header strip and pin the container it belongs to
	//! onto it. The INDEX is what travels, not the item: the callback re-reads
	//! m_InventoryItems, which is the array the listing rebuilt.
	void PrepareInventoryBandAction( UIActionImageButton button, int index, string tooltip )
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
			glyph.SetSize( JMUILayout.Px( INV_BAND_ICON_PX ), JMUILayout.Px( INV_BAND_ICON_PX ) );
			glyph.Update();
		}

		button.SetData( new JMStringData( index.ToString() ) );

		if ( tooltip != "" )
			button.SetTooltip( tooltip );
	}

	//! Which container a band action was created for, or -1.
	int InventoryBandActionIndex( UIActionBase action )
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

	void RunInventoryGroupAction( int index, int op )
	{
		if ( !m_Form.m_SelectedInstance )
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

			m_Form.ConfirmAction( "#STR_COT_PLAYER_MODULE_INV_GROUP_DELETE_TITLE", COT_String.TranslateEx( "#STR_COT_PLAYER_MODULE_INV_GROUP_DELETE_BODY" , InventoryChildCount( index ).ToString(), m_InvGroupPendingName ), "InventoryGroupDeleteConfirm" );
			return;
		}

		m_Form.m_Module.InventoryGroupOp( m_Form.m_SelectedInstance.GetGUID(), container.NetIdLow, container.NetIdHigh, op );
	}

	void InventoryGroupDeleteConfirm( JMConfirmation confirmation = NULL )
	{
		if ( !m_Form.m_SelectedInstance )
			return;

		m_Form.m_Module.InventoryGroupOp( m_Form.m_SelectedInstance.GetGUID(), m_InvGroupPendingLow, m_InvGroupPendingHigh, JMInventoryGroupOp.DELETE_ALL );
	}

	//! "Field Backpack (6)" - whose contents these are, and how many.
	string InventoryGroupLabel( int index )
	{
		int count = InventoryChildCount( index );

		return m_InventoryItems[index].GetDisplayName() + " (" + count.ToString() + ")";
	}

	int InventoryChildCount( int index )
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
	bool InventoryGroupIsAttachments( int index )
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

	void ToggleInventoryExpanded( int index )
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
	Widget BuildInventoryCell( int index )
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

		// Exactly vanilla's inventory icon call sequence, same order. The engine frames
		// the model from the config bounding box and invView; do NOT override with
		// SetModelPosition, which made previews a fraction of vanilla's size.
		preview.SetForceFlipEnable( true );
		preview.SetForceFlip( false );
		preview.SetItem( entity );
		preview.SetModelOrientation( vector.Zero );
		preview.SetView( entity.GetViewIndex() );

		preview.Show( true );

		return cell;
	}

	//! What a cell's preview entity was built from. Same-type items holding different
	//! things, or the SAME item in different states, must not share a cached model:
	//! health, quantity, temperature and food stage are baked in at SpawnLocalPreview
	//! time, so a key ignoring them showed the first-spawn state forever.
	string InventoryPreviewKey( int index )
	{
		JMPlayerInventoryItem item = m_InventoryItems[index];

		return item.Type + "|" + InventoryChildCount( index ).ToString() + "|" + item.Health.ToString() + "|" + item.Quantity.ToString() + "|" + item.Temperature.ToString() + "|" + item.Stage.ToString();
	}

	//! Quantity is drawn per the item's config (vanilla QuantityConversions rule):
	//! magazines count rounds, quantityBar or max of one gets a bar, anything else
	//! with a max prints the number ("4/4" bandages; pear and chemlight are bars).
	int InventoryQuantityMode( JMPlayerInventoryItem item )
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

	void PaintInventoryCellQuantity( Widget cell, JMPlayerInventoryItem item )
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
	void PaintInventoryCellHealth( Widget cell, JMPlayerInventoryItem item )
	{
		Widget dot = cell.FindAnyWidget( "cell_health_dot" );
		if ( !dot )
			return;

		dot.SetColor( InventoryHealthColour( item ) );
	}

	//! The condition dot uses VANILLA's colours (ItemManager.GetItemHealthColor, no
	//! alpha, hence the OR). The level comes from the server's healthLevels config;
	//! the threshold fallback is only for listings from older builds.
	int InventoryHealthColour( JMPlayerInventoryItem item )
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
	int InventoryCellIndexOf( Widget w )
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
	int InventoryCellSlotOf( int itemIndex )
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

	void SelectInventoryCell( int index )
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
	void PaintInventoryCellRing( Widget cell, int index, bool selected )
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

	//! The second, inset ring, one pixel inside the first. The 8-slice border is one
	//! pixel wide with no heavier variant, so an open container draws the ring TWICE
	//! to read as a thick edge.
	void PaintInventoryCellInnerRing( Widget inner, bool show, int color )
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

		m_Form.SpinRefreshIcon( action );
		RequestInventory();
	}

	//! Always the player whose inventory is open (m_SelectedInstance), never the
	//! list's checkbox selection: JM_GetSelected() could wipe a different player.
	void Click_ClearCargoOnly( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_Form.m_SelectedInstance )
			return;

		m_Form.ConfirmAction( "#STR_COT_PLAYER_MODULE_INV_CLEAR_CARGO", "#STR_COT_PLAYER_MODULE_INV_CLEAR_CARGO_BODY", "ClearCargoOnlyConfirm" );
	}

	void ClearCargoOnlyConfirm( JMConfirmation confirmation = NULL )
	{
		if ( !m_Form.m_SelectedInstance )
			return;

		m_Form.m_Module.ClearCargo( { m_Form.m_SelectedInstance.GetGUID() } );
		RequestInventory();
	}

	//! Cell pointer events arrive here (plain widgets), false if not a cell. Down =
	//! select and right-click menu. Up = container toggle, as it REBUILDS THE GRID:
	//! on down, the vanished press target recentres the cursor mid-click. Not
	//! OnClick: a PanelWidget does not reliably raise one.
	bool OnInventoryCellPressed( Widget w, int button )
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
	bool OnInventoryCellReleased( Widget w, int button )
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
	bool OnInventoryCellSelect( Widget w )
	{
		int index = InventoryCellIndexOf( w );
		if ( index < 0 )
			return false;

		SelectInventoryCell( index );

		return true;
	}

	bool OnInventoryCellHover( Widget w )
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

	//! DayZ only draws real entities, and these items sit in ANOTHER player's server
	//! inventory (not replicated here). So the preview is a client-local copy from the
	//! classname plus the listing's health/quantity. Never the real item; only the
	//! network ID is used for operations, server-side.
	EntityAI SpawnLocalPreview( JMPlayerInventoryItem item )
	{
		float health = -1;
		if ( item.Health > 0 )
			health = item.Health;

		float quantity = -1;
		if ( item.QuantityMax > 0 )
			quantity = item.Quantity;

		int stage = -1;
		if ( item.HasFoodStage() )
			stage = item.Stage;

		EntityAI ent = JMLocalPreview.Spawn( item.Type, health, quantity, stage );
		if ( !ent )
			return NULL;

		//! SetTemperatureEx dereferences m_TAC, which EntityAI only builds
		//! server-side (InitItemVariables). This preview is client-local, so
		//! the call must be skipped there or it NULLs on entities like Apple.
		ItemBase asItem;
		if ( g_Game.IsServer() && Class.CastTo( asItem, ent ) )
			asItem.SetTemperatureEx( new TemperatureData( item.Temperature ) );

		return ent;
	}

	//! Freeze a preview tree once it is fully built - see JMLocalPreview.Freeze for why LAST.
	void FinishLocalPreview( EntityAI ent )
	{
		JMLocalPreview.Freeze( ent );
	}

	void DestroyLocalPreview( EntityAI ent )
	{
		JMLocalPreview.Destroy( ent );
	}

	EntityAI AttachLocalPreviewChild( EntityAI parent, JMPlayerInventoryItem child, int slotId )
	{
		return JMLocalPreview.AttachLocal( parent, child.Type, slotId );
	}

	//! Rebuild an item's attachments and cargo onto its local copy. The listing is
	//! a PREORDER flat array (parents before children, subtrees contiguous), so one
	//! forward pass suffices. Without it a rifle inspects as a bare receiver.
	void BuildLocalPreviewChildren( int rootIndex, EntityAI rootEntity, bool attachmentsOnly = false )
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

		// No early exit on depth: bailing at the first entry back at root depth
		// dropped the rest of the subtree on odd ordering (optic but no magazine).
		// `spawned` membership already limits this to descendants; the listing is
		// capped at 256, so walk it all.
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

			// Slot id first (right slot for multi-slot items), then type (a magazine's
			// slot name may not resolve to an id the weapon accepts), then the LOCAL
			// inventory operation (vanilla's client path, seats magazines
			// CreateAttachment refuses). Cargo last.
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

			FinishLocalPreview( childEntity );

			spawned.Insert( i, childEntity );
		}

		// Loud on purpose: a missing attachment is invisible in the picture and
		// impossible to tell apart from an item that simply has none.
		if ( failed > 0 )
			Print( "[COT-PREVIEW] children of " + m_InventoryItems[rootIndex].Type + ": built=" + built + " failed=" + failed + " ->" + failures );
	}

	void ShowInventoryTooltip()
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
	void PlaceInventoryTooltip()
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

		float cursorGap = JMUILayout.Px( 18 );
		float edgeGap   = JMUILayout.Px( 10 );

		float x = mx + cursorGap;
		float y = my + cursorGap;

		if ( x + tw > screenW - edgeGap )
			x = screenW - edgeGap - tw;

		if ( y + th > screenH - edgeGap )
			y = screenH - edgeGap - th;

		// The layout root is declared with exact pos flags, but it is parented
		// to the window root here rather than to the inventory screen, so set
		// them explicitly rather than trusting an inherited default.
		m_InvTooltip.SetFlags( WidgetFlags.HEXACTPOS | WidgetFlags.VEXACTPOS, true );
		m_InvTooltip.SetPos( x, y );
	}

	void HideInventoryTooltip()
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
	void DestroyInventoryCellEntities()
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
	void ShowInventoryContextMenu()
	{
		if ( !m_InventoryMenu )
		{
			Error("[JMPlayerFormTabInventory] ShowInventoryContextMenu failed: m_InventoryMenu is null!");
			return;
		}

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
		bool canModify = JMPermissions.Has( JMConstants.PERM_PLAYER_INVENTORY_MODIFY );

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
		m_InventoryMenu.SetItemEnabled( INV_MENU_TAKE,   JMPermissions.Has( JMConstants.PERM_PLAYER_INVENTORY_TAKE ) );
		m_InventoryMenu.SetItemEnabled( INV_MENU_DELETE, JMPermissions.Has( JMConstants.PERM_PLAYER_INVENTORY_DELETE ) );

		m_InventoryMenu.SetItemEnabled( INV_MENU_HEALTH,      canModify );
		m_InventoryMenu.SetItemEnabled( INV_MENU_UNJAM,       canModify );
		m_InventoryMenu.SetItemEnabled( INV_MENU_QUANTITY,    canModify );
		m_InventoryMenu.SetItemEnabled( INV_MENU_TEMPERATURE, canModify );
		m_InventoryMenu.SetItemEnabled( INV_MENU_STATE,       canModify );
		m_InventoryMenu.SetItemEnabled( INV_MENU_LIQUID,      canModify );

		int menuX, menuY;
		GetMousePos( menuX, menuY );
		m_InventoryMenu.OpenAt( menuX, menuY );
	}

	void OnClick_InventoryMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_InventoryMenu )
		{
			Error("[JMPlayerFormTabInventory] OnClick_InventoryMenu failed: m_InventoryMenu is null!");
			return;
		}

		if ( !m_Form.m_SelectedInstance )
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
			m_Form.m_Module.InventoryTake( m_Form.m_SelectedInstance.GetGUID(), item.NetIdLow, item.NetIdHigh );
			return;
		}

		if ( id == INV_MENU_DELETE )
		{
			// The confirmation is asynchronous, so remember which row it was
			// about - the table can be rebuilt before the answer comes back.
			m_InventoryPendingRow = row;
			m_Form.ConfirmAction( "#STR_COT_PLAYER_MODULE_INV_DELETE_CONFIRM_TITLE", COT_String.TranslateEx( "#STR_COT_PLAYER_MODULE_INV_DELETE_CONFIRM_BODY" , item.GetDisplayName() ), "InventoryDeleteConfirm" );
			return;
		}

		// Unjam is the one item edit with nothing to ask: there is exactly one
		// thing the admin can mean by it, so it goes straight out.
		if ( id == INV_MENU_UNJAM )
		{
			m_Form.m_Module.InventoryModify( m_Form.m_SelectedInstance.GetGUID(), item.NetIdLow, item.NetIdHigh, JMInventoryModifyOp.UNJAM, 0 );
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
	void ShowInventoryQuantityPrompt( JMPlayerInventoryItem item )
	{
		string format = "%1";
		if ( item.QuantityMax > 0 )
			format = "%1 / " + Math.Round( item.QuantityMax ).ToString();

		m_InventoryPrompt.OpenSlider( INV_MENU_QUANTITY, "#STR_COT_PLAYER_MODULE_INV_SET_QUANTITY", item.GetDisplayName(), item.QuantityMin, item.QuantityMax, item.Quantity, 1, format );
	}

	//! Same range and the same degree format the object spawner's temperature
	//! slider uses, so the two read alike.
	void ShowInventoryTemperaturePrompt( JMPlayerInventoryItem item )
	{
		m_InventoryPrompt.OpenSlider( INV_MENU_TEMPERATURE, "#STR_COT_PLAYER_MODULE_INV_SET_TEMPERATURE", item.GetDisplayName(), GameConstants.STATE_COLD_LVL_FOUR, GameConstants.STATE_HOT_LVL_FOUR, item.Temperature, 1, "#STR_COT_FORMAT_DEGREE", UIActionValuePrompt.SLIDER_COLOR_TEMPERATURE );
	}

	//! Set any health (coloured like the condition dot and object spawner slider).
	//! Range is the ITEM's own MaxHealth, not a flat 0-100: not all items top out at
	//! 100 and a lower ceiling would silently cap below full.
	void ShowInventoryHealthPrompt( JMPlayerInventoryItem item )
	{
		float max = item.MaxHealth;
		if ( max <= 0 )
			max = 100;

		m_InventoryPrompt.OpenSlider( INV_MENU_HEALTH, "#STR_COT_PLAYER_MODULE_INV_SET_HEALTH", item.GetDisplayName(), 0, max, item.Health, 1, "%1", UIActionValuePrompt.SLIDER_COLOR_HEALTH );
	}

	//! Only the stages this item's config defines: the full enum would offer a steak
	//! a "dried" with no data, and nothing below ChangeFoodStage checks it exists.
	void ShowInventoryStatePrompt( JMPlayerInventoryItem item )
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

		m_InventoryPrompt.OpenOptions( INV_MENU_STATE, "#STR_COT_PLAYER_MODULE_INV_SET_STATE", item.GetDisplayName(), labels, values, item.Stage );
	}

	//! Only the liquids this container's config accepts: built from the game's liquid
	//! definitions (mods add for free), filtered by the container mask as the server
	//! rejects anything outside it.
	void ShowInventoryLiquidPrompt( JMPlayerInventoryItem item )
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

		m_InventoryPrompt.OpenOptions( INV_MENU_LIQUID, "#STR_COT_PLAYER_MODULE_INV_SET_LIQUID", item.GetDisplayName(), labels, values, item.LiquidType );
	}

	string FoodStageLabel( int stage )
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

		if ( !m_Form.m_SelectedInstance || !m_InventoryPrompt )
			return;

		string id = m_InventoryPrompt.GetPromptId();

		if ( id == INV_MENU_QUANTITY )
		{
			m_Form.m_Module.InventoryModify( m_Form.m_SelectedInstance.GetGUID(), m_InvPromptNetLow, m_InvPromptNetHigh, JMInventoryModifyOp.QUANTITY, m_InventoryPrompt.GetSliderValue() );
			return;
		}

		if ( id == INV_MENU_HEALTH )
		{
			m_Form.m_Module.InventoryModify( m_Form.m_SelectedInstance.GetGUID(), m_InvPromptNetLow, m_InvPromptNetHigh, JMInventoryModifyOp.HEALTH, m_InventoryPrompt.GetSliderValue() );
			return;
		}

		if ( id == INV_MENU_TEMPERATURE )
		{
			m_Form.m_Module.InventoryModify( m_Form.m_SelectedInstance.GetGUID(), m_InvPromptNetLow, m_InvPromptNetHigh, JMInventoryModifyOp.TEMPERATURE, m_InventoryPrompt.GetSliderValue() );
			return;
		}

		if ( id == INV_MENU_STATE )
		{
			m_Form.m_Module.InventoryModify( m_Form.m_SelectedInstance.GetGUID(), m_InvPromptNetLow, m_InvPromptNetHigh, JMInventoryModifyOp.FOOD_STAGE, m_InventoryPrompt.GetSelectedValue() );
			return;
		}

		if ( id == INV_MENU_LIQUID )
		{
			m_Form.m_Module.InventoryModify( m_Form.m_SelectedInstance.GetGUID(), m_InvPromptNetLow, m_InvPromptNetHigh, JMInventoryModifyOp.LIQUID_TYPE, m_InventoryPrompt.GetSelectedValue() );
		}
	}

	//! Open DayZ's inspect screen on a local copy of the item. MENU_INSPECT is a
	//! UIScriptedMenu and COT is not a menu (draws onto the workspace), so it stacks
	//! over the COT window and its close button returns here.
	void InspectInventoryItem( int row, JMPlayerInventoryItem item )
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
	void WatchInventoryInspect()
	{
		if ( !m_InvInspectOpen )
			return;

		if ( g_Game.GetUIManager().FindMenu( MENU_INSPECT ) )
			return;

		CloseInventoryInspect();

		// UIScriptedMenu.UnlockControls hides the cursor when there is no PARENT MENU,
		// and COT is not one, so closing inspect would leave the window up with no
		// pointer until something re-shows it.
		RestoreCursorAfterMenu();
	}

	//! Put the pointer back after a vanilla menu closed over the top of COT.
	//! Only the cursor: the menu balanced its own ChangeGameFocus on the way
	//! out, so COT's focus count is still whatever opening the window set.
	void RestoreCursorAfterMenu()
	{
		if ( !m_Form.m_IsShown )
			return;

		g_Game.GetUIManager().ShowUICursor( true );
	}

	void CloseInventoryInspect()
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

	//! Everything known about one item, as lines. No longer used by inspect (vanilla
	//! screen now) but kept as the text form, the only place the classname is spelt
	//! out (types.xml entry, spawn command).
	string BuildInspectText( JMPlayerInventoryItem item )
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
		if ( !m_Form.m_SelectedInstance )
			return;

		if ( m_InventoryPendingRow < 0 || m_InventoryPendingRow >= m_InventoryItems.Count() )
			return;

		JMPlayerInventoryItem item = m_InventoryItems[m_InventoryPendingRow];
		m_InventoryPendingRow = -1;

		m_Form.m_Module.InventoryDelete( m_Form.m_SelectedInstance.GetGUID(), item.NetIdLow, item.NetIdHigh );
	}
}
