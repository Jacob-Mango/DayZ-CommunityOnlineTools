// =============================================================================
//  JMMapEditorForm.c
//
//  In-game map editor. UI shell with:
//    - Top toolbar: mode (Select/Move/Rotate/Scale/Spawn) + Save/Load/Undo/Redo + Refresh
//    - Snap toggles: Terrain / Grid / Surface
//    - Left sidebar: search + asset browser (built from JMMapEditorCatalog)
//    - Right panel: list of placed objects (server-driven)
//
//  This file is intentionally minimal and written from scratch. Earlier
//  versions had accumulated structural damage from incremental edits. The
//  goal here is a known-good starting point; 3D gizmos / freecam / drag
//  hooks are added in follow-up patches after this base compiles.
// =============================================================================

// Per-render state. Class-scope static const is the pattern that worked in
// JMVehiclesModule (file-scope const + auto-import into class).
const int MAPEDITOR_MODE_SELECT = 0;
const int MAPEDITOR_MODE_MOVE   = 1;
const int MAPEDITOR_MODE_ROTATE = 2;
const int MAPEDITOR_MODE_SPAWN  = 3;
const int MAPEDITOR_MODE_SCALE  = 4;

const int SNAP_TERRAIN = 1;
const int SNAP_GRID    = 2;
const int SNAP_SURFACE = 4;

class JMMapEditorForm : JMFormBase
{
	// --- Mode toolbar ---
	protected UIActionButton m_ModeSelect;
	protected UIActionButton m_ModeMove;
	protected UIActionButton m_ModeRotate;
	protected UIActionButton m_ModeScale;
	protected UIActionButton m_ModeSpawn;
	protected UIActionButton m_Refresh;
	protected UIActionButton m_Save;
	protected UIActionButton m_Load;
	protected UIActionButton m_Undo;
	protected UIActionButton m_Redo;

	// --- Snap toggles ---
	protected UIActionCheckbox m_SnapTerrain;
	protected UIActionCheckbox m_SnapGrid;
	protected UIActionCheckbox m_SnapSurface;

	// --- Mode + snaps runtime state ---
	protected int m_Mode;
	protected int m_Snaps;

	// --- Asset browser (left sidebar) ---
	protected UIActionSearchBox m_AssetSearch;
	protected UIActionScroller m_AssetScroller;
	protected Widget m_AssetContent;

	// --- Selection ---
	protected UIActionText m_SelId;
	protected UIActionText m_SelClass;
	protected UIActionEditableText m_PosX;
	protected UIActionEditableText m_PosY;
	protected UIActionEditableText m_PosZ;
	protected UIActionEditableText m_OriYaw;
	protected UIActionSlider m_Scale;
	protected UIActionButton m_ApplyTransform;
	protected UIActionButton m_TeleportTo;
	protected UIActionButton m_Delete;
	protected UIActionButton m_ClearAll;

	// ---- Bulk + clipboard + undo buttons ----
	protected UIActionButton m_BulkMove;
	protected UIActionButton m_BulkRotate;
	protected UIActionButton m_BulkScale;
	protected UIActionButton m_CopyBtn;
	protected UIActionButton m_CutBtn;
	protected UIActionButton m_PasteBtn;
	protected UIActionButton m_UndoBtn;
	protected UIActionButton m_RedoBtn;

	// --- Local mirror of server list + spawn state ---
	protected ref array< ref JMMapEditorEntry > m_AllEntries;
	protected int m_SelectedId;
	protected string m_PendingSpawnClass;

	// --- Multi-select (shift-click to add / remove from the set) ---
	protected ref set< int > m_SelectedIds;

	// --- 3D interaction state ---
	protected ref JMEditorGizmos m_Gizmos;
	protected vector m_DragStartOrigin;     // object pos when drag began
	protected vector m_DragStartOrient;     // object orient when drag began
	protected float   m_DragStartScale;      // object scale when drag began
	protected int     m_DragAxis;            // -1 if not dragging
	protected vector m_DragStartMouseWorld;  // cursor world pos at drag begin
	protected int     m_PreviousMouseX;       // cursor px at last Tick (for delta)
	protected int     m_PreviousMouseY;
	protected bool    m_HasPreviousMouse;
	protected bool    m_LmbDown;             // LMB held this frame
	protected bool    m_RmbDown;             // RMB held this frame (use for look-only)
	protected bool    m_LmbWasDown;          // LMB state from previous tick
	protected bool    m_LmbClickPending;     // LMB just released - fire click handler next tick

	// --- Keyboard shortcuts (edge-detected in Tick) ---
	protected bool    m_DeleteKeyWasDown;
	protected bool    m_UndoKeyWasDown;
	protected bool    m_RedoKeyWasDown;

	//! Set whenever this form opens ITS OWN confirmation dialog (Delete /
	//! Clear All). The dialog widget is one shared, reused instance per
	//! window rather than a fresh one per call, so a hotkey firing while it
	//! is still up would race whatever the user is mid-click on - hence the
	//! guard in HandleKeyboardShortcuts.
	protected JMConfirmation m_ActiveConfirmation;
	protected const int  TICK_HANDLE = 0;
	protected const int  TICK_PERIOD_MS = 33;  // ~30 fps UI tick

	// Static handle for the per-frame callback. The CallQueue can only
	// resolve a method by name when it's a static class member, so the
	// static TickStatic dispatches to the single active form.
	protected static ref JMMapEditorForm s_Instance;
	protected JMMapEditorModule m_Module;

	void JMMapEditorForm()
	{
		m_AllEntries        = new array< ref JMMapEditorEntry >;
		m_Mode              = MAPEDITOR_MODE_SELECT;
		m_Snaps             = 0;
		m_SelectedId        = -1;
		m_PendingSpawnClass = "";
		m_Gizmos            = new JMEditorGizmos();
		m_DragAxis          = -1;
		m_PreviousMouseX    = 0;
		m_PreviousMouseY    = 0;
		m_HasPreviousMouse  = false;
		m_LmbDown           = false;
		m_RmbDown           = false;
		m_SelectedIds       = new set< int >;
	}

	//! Whether this form currently owns Ctrl+Z/Ctrl+Y. While shown, its own
	//! Tick()-polled shortcut handles them against its OWN undo stack; the
	//! global hotkey (MissionGameplay.OnUpdate) steps aside so the same
	//! keypress cannot fire both.
	static bool IsShown()
	{
		return s_Instance != NULL;
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	// ========================================================================
	//  OnCreate - build all UI
	// ========================================================================

	override void OnCreate()
	{
		// ---- Top toolbar (2 rows: mode buttons + file ops, then snap toggles) ----
		// WrapSpacer (flex-wrap) so the mode buttons don't overflow when the
		// panel is narrower than the button row would require.
		Widget top = layoutRoot.FindAnyWidget( "panel_top" );

		Widget topStack = UIActionManager.CreateGridSpacer( top, 2, 1 );

		Widget modeRow = UIActionManager.CreateWrapSpacer( topStack );
		m_ModeSelect = MakeModeButton( modeRow, "Select", MAPEDITOR_MODE_SELECT, "Click on a placed object to select it" );
		m_ModeMove   = MakeModeButton( modeRow, "Move",   MAPEDITOR_MODE_MOVE,   "Move the selected object (drag axis arrows in the 3D viewport)" );
		m_ModeRotate = MakeModeButton( modeRow, "Rotate", MAPEDITOR_MODE_ROTATE, "Rotate the selected object (drag the ring)" );
		m_ModeScale  = MakeModeButton( modeRow, "Scale",  MAPEDITOR_MODE_SCALE,  "Scale the selected object (drag the cube)" );
		m_ModeSpawn  = MakeModeButton( modeRow, "Spawn",  MAPEDITOR_MODE_SPAWN,  "Pick an asset in the sidebar, then click in the 3D viewport" );

		m_Save = UIActionManager.CreateButton( modeRow, "#STR_COT_MAPEDITOR_SAVE", this, "" );
		if ( m_Save ) m_Save.SetOnClick( this, "OnClick_Save" );
		m_Save.SetWidth( 0.07 );
		m_Save.SetTooltip( "#STR_COT_MAPEDITOR_SAVE_THE_CURRENT_LAYOUT_TO_A" );

		m_Load = UIActionManager.CreateButton( modeRow, "#STR_COT_MAPEDITOR_LOAD", this, "" );
		if ( m_Load ) m_Load.SetOnClick( this, "OnClick_Load" );
		m_Load.SetWidth( 0.07 );
		m_Load.SetTooltip( "#STR_COT_MAPEDITOR_LOAD_A_SAVED_LAYOUT_PRESET_FROM" );

		m_Undo = UIActionManager.CreateButton( modeRow, "#STR_COT_MAPEDITOR_UNDO", this, "OnClick_Undo" );
		m_Undo.SetWidth( 0.07 );
		m_Undo.SetTooltip( "#STR_COT_MAPEDITOR_UNDO_THE_LAST_PENDING_MUTATION_CLIENT" );

		m_Redo = UIActionManager.CreateButton( modeRow, "#STR_COT_MAPEDITOR_REDO", this, "OnClick_Redo" );
		m_Redo.SetWidth( 0.07 );
		m_Redo.SetTooltip( "#STR_COT_MAPEDITOR_REDO_A_PREVIOUSLY_UNDONE_MUTATION" );

		m_Refresh = UIActionManager.CreateButton( modeRow, "#STR_COT_MAPEDITOR_REFRESH", this, "OnClick_Refresh" );
		m_Refresh.SetWidth( 0.07 );
		m_Refresh.SetTooltip( "#STR_COT_MAPEDITOR_RELOAD_THE_PLACED_OBJECT_LIST_FROM" );
		// No SetIcon - toolbar buttons are text-only so the icon doesn't
		// sit in the center. The vanilla button class doesn't expose an
		// image-align API, so the simpler path is to keep all toolbar
		// actions as plain text and put any icon hints in the tooltip.

		Widget snapRow = UIActionManager.CreateWrapSpacer( topStack );
		m_SnapTerrain = UIActionManager.CreateCheckbox( snapRow, "#STR_COT_MAPEDITOR_SNAP_TERRAIN", this, "Click_SnapTerrain", false );
		m_SnapTerrain.SetWidth( 0.33 );
		m_SnapTerrain.SetTooltip( "#STR_COT_MAPEDITOR_SPAWNED_MOVED_OBJECTS_SNAP_TO_THE" );
		m_SnapGrid    = UIActionManager.CreateCheckbox( snapRow, "#STR_COT_MAPEDITOR_SNAP_GRID", this, "Click_SnapGrid", false );
		m_SnapGrid.SetWidth( 0.33 );
		m_SnapGrid.SetTooltip( "#STR_COT_MAPEDITOR_ROUND_COORDINATES_TO_1_M_INCREMENTS" );
		m_SnapSurface = UIActionManager.CreateCheckbox( snapRow, "#STR_COT_MAPEDITOR_SNAP_SURFACE", this, "Click_SnapSurface", false );
		m_SnapSurface.SetWidth( 0.33 );
		m_SnapSurface.SetTooltip( "#STR_COT_MAPEDITOR_ALIGN_YAW_TO_THE_SURFACE_NORMAL" );

		// ---- Left sidebar: asset browser ----
		Widget left = layoutRoot.FindAnyWidget( "panel_left" );
		Widget leftStack = UIActionManager.CreateGridSpacer( left, 2, 1 );

		m_AssetSearch = UIActionManager.CreateSearchBox( leftStack, this, "OnChange_AssetFilter", "Search assets..." );
		m_AssetSearch.SetTooltip( "#STR_COT_MAPEDITOR_FILTER_THE_ASSET_BROWSER_BY_CLASSNAME" );

		m_AssetScroller = UIActionManager.CreateScroller( leftStack );
		m_AssetContent  = m_AssetScroller.GetContentWidget();
		m_AssetScroller.UpdateScroller();

		// ---- Right panel: selection / properties ----
		Widget right = layoutRoot.FindAnyWidget( "panel_right" );
		Widget rightStack = UIActionManager.CreateGridSpacer( right, 7, 1 );

		UIActionManager.CreateText( rightStack, "#STR_COT_MAPEDITOR_PROPERTIES", "" );

		m_SelId = UIActionManager.CreateText( rightStack, "", "#STR_COT_MAPEDITOR_NO_SELECTION" );
		m_SelId.SetWidth( 1.0 );

		m_SelClass = UIActionManager.CreateText( rightStack, "", "" );
		m_SelClass.SetWidth( 1.0 );

		Widget posRow = UIActionManager.CreateGridSpacer( rightStack, 1, 3 );
		m_PosX = UIActionManager.CreateEditableText( posRow, "X:", this, "" );
		m_PosX.SetWidth( 0.32 );
		m_PosX.SetTooltip( "#STR_COT_MAPEDITOR_WORLD_X_METERS" );
		m_PosY = UIActionManager.CreateEditableText( posRow, "Y:", this, "" );
		m_PosY.SetWidth( 0.32 );
		m_PosY.SetTooltip( "#STR_COT_MAPEDITOR_WORLD_Y_METERS" );
		m_PosZ = UIActionManager.CreateEditableText( posRow, "Z:", this, "" );
		m_PosZ.SetWidth( 0.32 );
		m_PosZ.SetTooltip( "#STR_COT_MAPEDITOR_WORLD_Z_METERS" );

		Widget oriRow = UIActionManager.CreateGridSpacer( rightStack, 1, 1 );
		m_OriYaw = UIActionManager.CreateEditableText( oriRow, "#STR_COT_MAPEDITOR_YAW_DEG", this, "" );
		m_OriYaw.SetWidth( 1.0 );
		m_OriYaw.SetTooltip( "#STR_COT_MAPEDITOR_YAW_ROTATION_IN_DEGREES_0_359" );

		m_Scale = UIActionManager.CreateSlider( rightStack, "#STR_COT_MAPEDITOR_SCALE", 0.1, 5.0, this, "" );
		m_Scale.SetCurrent( 1.0 );
		m_Scale.SetStepValue( 0.1 );
		m_Scale.SetWidth( 0.95 );
		m_Scale.SetTooltip( "#STR_COT_MAPEDITOR_UNIFORM_SCALE_MULTIPLIER" );

		Widget actionRow = UIActionManager.CreateGridSpacer( rightStack, 1, 3 );
		m_ApplyTransform = UIActionManager.CreateButton( actionRow, "#STR_COT_MAPEDITOR_APPLY", this, "" );
		if ( m_ApplyTransform ) m_ApplyTransform.SetOnClick( this, "OnClick_ApplyTransform" );
		m_ApplyTransform.SetWidth( 0.32 );
		m_ApplyTransform.SetTooltip( "#STR_COT_MAPEDITOR_APPLY_POSITION_ROTATION_SCALE_TO_THE" );
		m_TeleportTo = UIActionManager.CreateButton( actionRow, "#STR_COT_MAPEDITOR_TELEPORT", this, "" );
		if ( m_TeleportTo ) m_TeleportTo.SetOnClick( this, "OnClick_TeleportTo" );
		m_TeleportTo.SetWidth( 0.32 );
		m_TeleportTo.SetTooltip( "#STR_COT_MAPEDITOR_TELEPORT_YOUR_PLAYER_TO_THE_SELECTED" );
		m_Delete = UIActionManager.CreateButton( actionRow, "#STR_COT_MAPEDITOR_DELETE", this, "OnClick_Delete" );
		m_Delete.SetColor( JMTheme.DANGER_FILL );
		m_Delete.SetWidth( 0.32 );
		m_Delete.SetTooltip( "#STR_COT_MAPEDITOR_DELETE_THE_SELECTED_OBJECT_2" );

		Widget clearRow = UIActionManager.CreateGridSpacer( rightStack, 1, 1 );
		m_ClearAll = UIActionManager.CreateButton( clearRow, "#STR_COT_MAPEDITOR_CLEAR_ALL", this, "" );
		if ( m_ClearAll ) m_ClearAll.SetOnClick( this, "OnClick_ClearAll" );
		m_ClearAll.SetColor( JMTheme.DANGER_FILL );
		m_ClearAll.SetWidth( 1.0 );
		m_ClearAll.SetTooltip( "#STR_COT_MAPEDITOR_DELETE_EVERY_PLACED_OBJECT_IRREVERSIBLE" );

		// ---- Bulk + clipboard + undo/undo rows ----
		Widget bulkRow = UIActionManager.CreateGridSpacer( rightStack, 1, 3 );
		m_BulkMove   = UIActionManager.CreateButton( bulkRow, "#STR_COT_MAPEDITOR_BULK_MOVE", this, "" );
		if ( m_BulkMove ) m_BulkMove.SetOnClick( this, "OnClick_BulkMove" );
		m_BulkMove.SetWidth( 0.33 );
		m_BulkMove.SetTooltip( "#STR_COT_MAPEDITOR_APPLY_X_Y_Z_DELTA_TO" );
		m_BulkRotate = UIActionManager.CreateButton( bulkRow, "#STR_COT_MAPEDITOR_BULK_ROTATE", this, "" );
		if ( m_BulkRotate ) m_BulkRotate.SetOnClick( this, "OnClick_BulkRotate" );
		m_BulkRotate.SetWidth( 0.33 );
		m_BulkRotate.SetTooltip( "#STR_COT_MAPEDITOR_ADD_THE_YAW_VALUE_DEG_ABOVE" );
		m_BulkScale  = UIActionManager.CreateButton( bulkRow, "#STR_COT_MAPEDITOR_BULK_SCALE", this, "" );
		if ( m_BulkScale ) m_BulkScale.SetOnClick( this, "OnClick_BulkScale" );
		m_BulkScale.SetWidth( 0.33 );
		m_BulkScale.SetTooltip( "#STR_COT_MAPEDITOR_MULTIPLY_THE_CURRENT_SCALE_OF_EVERY" );

		Widget clipRow = UIActionManager.CreateGridSpacer( rightStack, 1, 3 );
		m_CopyBtn   = UIActionManager.CreateButton( clipRow, "#STR_COT_MAPEDITOR_COPY", this, "" );
		if ( m_CopyBtn ) m_CopyBtn.SetOnClick( this, "OnClick_Copy" );
		m_CopyBtn.SetWidth( 0.33 );
		m_CopyBtn.SetTooltip( "#STR_COT_MAPEDITOR_SNAPSHOT_THE_CURRENTLY_SELECTED_OBJECT_I" );
		m_CutBtn    = UIActionManager.CreateButton( clipRow, "#STR_COT_MAPEDITOR_CUT", this, "" );
		if ( m_CutBtn ) m_CutBtn.SetOnClick( this, "OnClick_Cut" );
		m_CutBtn.SetWidth( 0.33 );
		m_CutBtn.SetTooltip( "#STR_COT_MAPEDITOR_SNAPSHOT_DELETE_THE_CURRENTLY_SELECTED_O" );
		m_PasteBtn  = UIActionManager.CreateButton( clipRow, "#STR_COT_MAPEDITOR_PASTE", this, "" );
		if ( m_PasteBtn ) m_PasteBtn.SetOnClick( this, "OnClick_Paste" );
		m_PasteBtn.SetWidth( 0.33 );
		m_PasteBtn.SetTooltip( "#STR_COT_MAPEDITOR_SPAWN_THE_CLIPBOARD_AT_THE_CURSOR" );

		Widget undoRow = UIActionManager.CreateGridSpacer( rightStack, 1, 2 );
		m_UndoBtn = UIActionManager.CreateButton( undoRow, "#STR_COT_MAPEDITOR_UNDO", this, "OnClick_Undo" );
		m_UndoBtn.SetWidth( 0.49 );
		m_UndoBtn.SetTooltip( "#STR_COT_MAPEDITOR_REVERT_THE_LAST_SERVER_SIDE_MUTATION" );
		m_RedoBtn = UIActionManager.CreateButton( undoRow, "#STR_COT_MAPEDITOR_REDO", this, "OnClick_Redo" );
		m_RedoBtn.SetWidth( 0.49 );
		m_RedoBtn.SetTooltip( "#STR_COT_MAPEDITOR_RE_APPLY_A_REVERTED_MUTATION" );

		// ---- How to use ----
		Widget helpRow = UIActionManager.CreateGridSpacer( rightStack, 1, 1 );
		Widget help = UIActionManager.CreateRow( helpRow, 140 );
		UIActionManager.CreateText( help, "#STR_COT_MAPEDITOR_HOW_TO_USE", "#STR_COT_MAPEDITOR_1_OPEN_THE_FREECAM_CAMERA_MODULE" );

		// ---- Permissions ----
		//! Mirrors exactly what JMMapEditorModule's own RPC handlers enforce, so
		//! a control is only offered when the matching server call would be
		//! accepted. The module is already behind Admin.MapEditor.View via
		//! HasAccess(), so read-only controls (Refresh, Copy, the snap toggles)
		//! need nothing further.
		BindPermission( m_ApplyTransform, JMConstants.PERM_MAPEDITOR_TRANSFORM );
		BindPermission( m_BulkMove,       JMConstants.PERM_MAPEDITOR_TRANSFORM );
		BindPermission( m_BulkRotate,     JMConstants.PERM_MAPEDITOR_TRANSFORM );
		BindPermission( m_BulkScale,      JMConstants.PERM_MAPEDITOR_TRANSFORM );
		BindPermission( m_Undo,           JMConstants.PERM_MAPEDITOR_TRANSFORM );
		BindPermission( m_Redo,           JMConstants.PERM_MAPEDITOR_TRANSFORM );
		BindPermission( m_UndoBtn,        JMConstants.PERM_MAPEDITOR_TRANSFORM );
		BindPermission( m_RedoBtn,        JMConstants.PERM_MAPEDITOR_TRANSFORM );

		BindPermission( m_Delete,         JMConstants.PERM_MAPEDITOR_DELETE );
		BindPermission( m_ClearAll,       JMConstants.PERM_MAPEDITOR_DELETE );
		BindPermission( m_CutBtn,         JMConstants.PERM_MAPEDITOR_DELETE );

		BindPermission( m_PasteBtn,       JMConstants.PERM_MAPEDITOR_SPAWN );
		BindPermission( m_Load,           JMConstants.PERM_MAPEDITOR_SPAWN );

		// ---- Initial paint ----
		RefreshModeButtons();
		BuildAssetBrowser();
	}

	override void OnShow()
	{
		super.OnShow();

		BindPermission( m_Refresh, JMConstants.PERM_MAPEDITOR_VIEW );
		BindPermission( m_Save, JMConstants.PERM_MAPEDITOR_SPAWN );
		BindPermission( m_Load, JMConstants.PERM_MAPEDITOR_SPAWN );
		BindPermission( m_ApplyTransform, JMConstants.PERM_MAPEDITOR_TRANSFORM );
		BindPermission( m_TeleportTo, JMConstants.PERM_MAPEDITOR_TRANSFORM );
		BindPermission( m_Delete, JMConstants.PERM_MAPEDITOR_DELETE );
		BindPermission( m_ClearAll, JMConstants.PERM_MAPEDITOR_DELETE );

		// Register per-frame tick. Drives 3D gizmo draw, axis hover, drag,
		// and 3D-pick click handling. 30 fps is fine for a UI panel.
		// Use the static TickStatic function reference (the string-name
		// overload doesn't bind on class methods).
		s_Instance = this;
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( TickStatic, TICK_PERIOD_MS, true );

		if ( m_Module )
			m_Module.RequestList();
	}

	override void OnHide()
	{
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( TickStatic );
		s_Instance = NULL;
		super.OnHide();
	}

	// ========================================================================
	//  Per-frame tick (called via CallLater from OnShow)
	//
	//  This is the 3D-only interaction surface: the user's cursor in 3D
	//  world space drives everything.  Per-frame we:
	//    1. Compute the cursor's world position (Pick3D)
	//    2. Drive gizmo drag (when active) and update server-side transform
	//    3. Render the gizmo + selection wireframe
	//    4. Auto-pick: if a placed object is under the cursor and the
	//       user is in SELECT mode, select it on LMB up
	//    5. Click-to-place: in SPAWN mode, drop a pending asset at the
	//       cursor's world position on LMB up
	// ========================================================================

	// Static dispatcher: the per-class CallLater form only resolves
	// method names that are static class members, so the call queue
	// calls this and we forward to the active instance.
	static void TickStatic()
	{
		if ( s_Instance )
			s_Instance.Tick();
	}

	void Tick()
	{
		if ( !m_Module )
			return;

		HandleKeyboardShortcuts();

		int mx, my;
		GetMousePos( mx, my );
		int dx = 0, dy = 0;
		if ( m_HasPreviousMouse )
		{
			dx = mx - m_PreviousMouseX;
			dy = my - m_PreviousMouseY;
		}
		m_PreviousMouseX = mx;
		m_PreviousMouseY = my;
		m_HasPreviousMouse = true;

		// Mouse button state via the engine helper
		m_LmbDown = ( GetMouseState( MouseState.LEFT  ) & MB_PRESSED_MASK ) != 0;
		m_RmbDown = ( GetMouseState( MouseState.RIGHT ) & MB_PRESSED_MASK ) != 0;

		// Sync gizmo state with the selected entry
		JMMapEditorEntry sel = FindEntry( m_SelectedId );
		if ( sel )
		{
			m_Gizmos.m_Position    = sel.Position;
			m_Gizmos.m_Orientation = sel.Orientation;
			m_Gizmos.m_Scale       = sel.Scale;
			m_Gizmos.m_Mode        = m_Mode;
		}
		else
		{
			m_Gizmos.m_Mode = -1;
		}

		// ---- 3D pick from cursor ----
		vector cursorWorld;
		Object cursorObj;
		JMEditorScene.CastFromScreen( mx, my, cursorWorld, cursorObj );

		// ---- Hover detection (only when RMB is NOT held - RMB is reserved
		// for the future freecam look binding) ----
		if ( !m_RmbDown && sel )
		{
			vector mouseScreen = Vector( mx, my, 0 );
			m_Gizmos.HitTest( mouseScreen );
		}
		else
		{
			m_Gizmos.m_HoveredAxis = JMEditorGizmos.AXIS_NONE;
		}

		// ---- Drag advance ----
		if ( m_DragAxis != -1 && sel )
		{
			vector delta = ComputeWorldDelta( dx, dy, m_DragAxis );
			vector newPos = m_DragStartOrigin;
			vector newOri = m_DragStartOrient;
			float   newScale = m_DragStartScale;

			if ( m_DragAxis >= 0 && m_DragAxis <= 2 ) // X / Y / Z move
			{
				newPos = newPos + delta;
				if ( m_Snaps & SNAP_GRID )
				{
					newPos[0] = Math.Round( newPos[0] );
					newPos[1] = Math.Round( newPos[1] );
					newPos[2] = Math.Round( newPos[2] );
				}
			}
			else if ( m_DragAxis == 3 ) // rotate
			{
				newOri[1] = newOri[1] + delta[1];
			}
			else if ( m_DragAxis == 4 ) // scale
			{
				newScale = Math.Clamp( newScale + delta[0] * 0.01, 0.1, 10.0 );
			}

			if ( m_Snaps & SNAP_TERRAIN )
				newPos[1] = Math.Max( newPos[1], JMEditorScene.GetSurfaceY( newPos[0], newPos[2] ) );

			m_Gizmos.m_Position    = newPos;
			m_Gizmos.m_Orientation = newOri;
			m_Gizmos.m_Scale       = newScale;

			// Commit to server each tick so admins see instant feedback.
			// This is one TransformObject RPC per tick; acceptable for an
			// admin-only tool.
			m_Module.TransformObject( m_SelectedId, newPos, newOri, newScale );
		}

		// ---- Render gizmo + selection wireframe ----
		m_Gizmos.Draw();
		DrawSelectionWireframe( sel );

		// ---- LMB edge detection: fire click handler on LMB-up if not mid-drag ----
		if ( m_LmbDown && !m_LmbWasDown && m_DragAxis == -1 )
		{
			// LMB just pressed this frame. Start drag if hovering a gizmo axis;
			// otherwise a click will be pending for the LMB-up frame.
			if ( m_Gizmos.m_HoveredAxis != JMEditorGizmos.AXIS_NONE && sel )
			{
				m_DragAxis = m_Gizmos.m_HoveredAxis;
				m_DragStartOrigin  = m_Gizmos.m_Position;
				m_DragStartOrient  = m_Gizmos.m_Orientation;
				m_DragStartScale   = m_Gizmos.m_Scale;
				m_DragStartMouseWorld = cursorWorld;
			}
			else
			{
				m_LmbClickPending = true;
			}
		}

		if ( m_LmbClickPending && !m_LmbDown )
		{
			// LMB released this tick after a click (no drag). Run the
			// mode-specific click handler.
			HandleLMBClick( cursorWorld, cursorObj );
			m_LmbClickPending = false;
		}

		m_LmbWasDown = m_LmbDown;

		// ---- Auto-start drag on LMB down over a gizmo axis ----
		if ( m_LmbDown && m_DragAxis == -1 && m_Gizmos.m_HoveredAxis != JMEditorGizmos.AXIS_NONE )
		{
			m_DragAxis = m_Gizmos.m_HoveredAxis;
			m_DragStartOrigin  = m_Gizmos.m_Position;
			m_DragStartOrient  = m_Gizmos.m_Orientation;
			m_DragStartScale   = m_Gizmos.m_Scale;
			m_DragStartMouseWorld = cursorWorld;
		}

		// ---- End drag on LMB up ----
		if ( !m_LmbDown && m_DragAxis != -1 )
			m_DragAxis = -1;
	}

	// Delete / Undo / Redo hotkeys. Skipped while the user is typing in one of
	// this form's own text fields (position / yaw / asset search), the same
	// EditBoxWidget check CF_InputBindings uses to gate its own global input,
	// and skipped while this form's own confirmation dialog is up so a
	// hotkey can never race whatever the user is mid-click on.
	// Edge-detected against last tick so a held key fires once, not 30x/sec.
	void HandleKeyboardShortcuts()
	{
		Widget focus = GetFocus();
		bool typing = focus && ( focus.IsInherited( EditBoxWidget ) || focus.IsInherited( MultilineEditBoxWidget ) );
		bool confirmOpen = m_ActiveConfirmation && m_ActiveConfirmation.IsVisible();

		bool ctrl = CTRL();

		// Shift+Delete, not plain Delete: matches the OS convention (Shift+Delete
		// bypasses the recycle bin) for "delete now, skip the prompt" and avoids
		// plain Delete colliding with anything else bound to that raw key.
		bool deleteDown = SHIFT() && ( KeyState( KeyCode.KC_DELETE ) > 0 );
		if ( deleteDown && !m_DeleteKeyWasDown && !typing && !confirmOpen && m_SelectedId != -1 && m_Module )
		{
			// Keyboard shortcut skips the confirmation prompt - same delete
			// the confirmed button runs, called directly.
			OnConfirmation_Delete( NULL );
		}
		m_DeleteKeyWasDown = deleteDown;

		bool undoDown = ctrl && ( KeyState( KeyCode.KC_Z ) > 0 );
		if ( undoDown && !m_UndoKeyWasDown && !typing && !confirmOpen && m_Module )
			m_Module.Undo();
		m_UndoKeyWasDown = undoDown;

		bool redoDown = ctrl && ( KeyState( KeyCode.KC_Y ) > 0 );
		if ( redoDown && !m_RedoKeyWasDown && !typing && !confirmOpen && m_Module )
			m_Module.Redo();
		m_RedoKeyWasDown = redoDown;
	}

	// LMB just-up dispatch: select / place / no-op by mode
	void HandleLMBClick( vector cursorWorld, Object cursorObj )
	{
		// Mode-specific click handling
		if ( m_DragAxis != -1 )
			return; // mid-drag - the gizmo is handling input

		if ( m_Mode == MAPEDITOR_MODE_SELECT )
		{
			// Pick a placed object under the cursor. The cursorObj from
			// CastFromScreen is the closest hit along the camera ray.
			// For placed objects, the pick is already a child of the world;
			// we just need to identify which of our m_AllEntries owns it.
			int pickedId = -1;
			foreach ( JMMapEditorEntry e : m_AllEntries )
			{
				// The actual placed object isn't in m_AllEntries (just
				// the metadata) - match by approximate position since
				// we know the server-spawned position.
				if ( vector.Distance( e.Position, cursorWorld ) < 1.0 )
				{
					pickedId = e.Id;
					break;
				}
			}

			// If we didn't hit a known placed object, just clear the
			// selection. (picking from the world like terrain is handled
			// by the terrain-snap feature, not selection.)
			m_SelectedId = pickedId;
			if ( m_SelectedId == -1 )
				m_SelectedIds.Clear();
			else
				m_SelectedIds = new set< int >;
			m_SelectedIds.Insert( m_SelectedId );
		}
		else if ( m_Mode == MAPEDITOR_MODE_SPAWN )
		{
			// Drop the pending asset at the cursor world position
			if ( m_PendingSpawnClass == "" )
				return;

			vector pos = cursorWorld;
			if ( pos == vector.Zero )
				return; // empty pick - skip

			// Snap-to-terrain by default on spawn unless the user disabled it
			if ( m_Snaps & SNAP_TERRAIN )
				pos[1] = JMEditorScene.GetSurfaceY( pos[0], pos[2] );

			m_Module.SpawnObject( m_PendingSpawnClass, pos, "0 0 0", 1.0 );
			m_PendingSpawnClass = "";
			m_Mode = MAPEDITOR_MODE_SELECT;
			RefreshModeButtons();
		}
	}

	// Draw a yellow wireframe box around the selected object
	void DrawSelectionWireframe( JMMapEditorEntry sel )
	{
		if ( !sel )
			return;
		vector c  = sel.Position;
		float s   = Math.Max( sel.Scale, 0.2 );
		int   col = ARGB( 220, 230, 200, 60 );

		vector a1 = c + Vector( -s, -s, -s );
		vector a2 = c + Vector(  s, -s, -s );
		vector a3 = c + Vector(  s, -s,  s );
		vector a4 = c + Vector( -s, -s,  s );
		vector b1 = c + Vector( -s,  s, -s );
		vector b2 = c + Vector(  s,  s, -s );
		vector b3 = c + Vector(  s,  s,  s );
		vector b4 = c + Vector( -s,  s,  s );

		// Bottom rectangle
		Debug.DrawLine( a1, a2, col, ShapeFlags.ONCE );
		Debug.DrawLine( a2, a3, col, ShapeFlags.ONCE );
		Debug.DrawLine( a3, a4, col, ShapeFlags.ONCE );
		Debug.DrawLine( a4, a1, col, ShapeFlags.ONCE );
		// Top rectangle
		Debug.DrawLine( b1, b2, col, ShapeFlags.ONCE );
		Debug.DrawLine( b2, b3, col, ShapeFlags.ONCE );
		Debug.DrawLine( b3, b4, col, ShapeFlags.ONCE );
		Debug.DrawLine( b4, b1, col, ShapeFlags.ONCE );
		// Vertical edges
		Debug.DrawLine( a1, b1, col, ShapeFlags.ONCE );
		Debug.DrawLine( a2, b2, col, ShapeFlags.ONCE );
		Debug.DrawLine( a3, b3, col, ShapeFlags.ONCE );
		Debug.DrawLine( a4, b4, col, ShapeFlags.ONCE );
	}

	// World-space delta from screen-space mouse delta. Camera-aligned
	// so dragging X moves the object along the camera-right axis, etc.
	vector ComputeWorldDelta( int dx, int dy, int axis )
	{
		if ( axis == 0 ) return Vector( dx * 0.05, 0, 0 );          // X move
		if ( axis == 1 ) return Vector( 0, -dy * 0.05, 0 );        // Y move (invert)
		if ( axis == 2 ) return Vector( 0, 0, dx * 0.05 );        // Z move
		if ( axis == 3 ) return Vector( 0, dx * 0.5, 0 );          // yaw rotate
		if ( axis == 4 ) return Vector( dx * 0.01, 0, 0 );        // scale
		return vector.Zero;
	}

	override void OnResize( float w, float h )
	{
		super.OnResize( w, h );
		if ( m_AssetScroller )
			m_AssetScroller.UpdateScroller();
	}

	// ========================================================================
	//  Mode buttons
	// ========================================================================

	protected UIActionButton MakeModeButton( Widget parent, string label, int mode, string tooltip )
	{
		UIActionButton b = UIActionManager.CreateButton( parent, label, this, "" );
		if ( b ) b.SetOnClick( this, "OnClick_Mode" );
		b.SetWidth( 0.08 );
		b.SetData( new JMMapEditorModeData( mode ) );
		b.SetTooltip( tooltip );
		return b;
	}

	void OnClick_Mode( UIActionBase action )
	{
		JMMapEditorModeData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_Mode = data.Mode;
		m_PendingSpawnClass = "";
		RefreshModeButtons();
	}

	protected void RefreshModeButtons()
	{
		int ACTIVE   = ARGB(255, 80, 200, 80);
		int INACTIVE = ARGB(255, 220, 220, 220);

		if ( m_Mode == MAPEDITOR_MODE_SELECT )
			m_ModeSelect.SetColor( ACTIVE );
		else
			m_ModeSelect.SetColor( INACTIVE );

		if ( m_Mode == MAPEDITOR_MODE_MOVE )
			m_ModeMove.SetColor( ACTIVE );
		else
			m_ModeMove.SetColor( INACTIVE );

		if ( m_Mode == MAPEDITOR_MODE_ROTATE )
			m_ModeRotate.SetColor( ACTIVE );
		else
			m_ModeRotate.SetColor( INACTIVE );

		if ( m_Mode == MAPEDITOR_MODE_SCALE )
			m_ModeScale.SetColor( ACTIVE );
		else
			m_ModeScale.SetColor( INACTIVE );

		if ( m_Mode == MAPEDITOR_MODE_SPAWN )
			m_ModeSpawn.SetColor( ACTIVE );
		else
			m_ModeSpawn.SetColor( INACTIVE );
	}

	// ========================================================================
	//  Snap toggles
	// ========================================================================

	void Click_SnapTerrain( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		if ( m_Snaps & SNAP_TERRAIN )
			m_Snaps = m_Snaps ^ SNAP_TERRAIN;
		else
			m_Snaps = m_Snaps | SNAP_TERRAIN;
	}

	void Click_SnapGrid( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		if ( m_Snaps & SNAP_GRID )
			m_Snaps = m_Snaps ^ SNAP_GRID;
		else
			m_Snaps = m_Snaps | SNAP_GRID;
	}

	void Click_SnapSurface( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		if ( m_Snaps & SNAP_SURFACE )
			m_Snaps = m_Snaps ^ SNAP_SURFACE;
		else
			m_Snaps = m_Snaps | SNAP_SURFACE;
	}

	// ========================================================================
	//  Save / Load / Refresh
	// ========================================================================

	void OnClick_Save( UIActionBase action )
	{
		if ( m_Module )
			m_Module.SaveToDisk();
	}

	void OnClick_Load( UIActionBase action )
	{
		if ( m_Module )
		{
			m_Module.LoadFromDisk();
			m_Module.RequestList();
		}
	}

	void OnClick_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		UIActionImageButton btn;
		if ( Class.CastTo( btn, action ) )
			btn.TriggerSpin( 2 );
		if ( m_Module )
			m_Module.RequestList();
	}

	void OnClick_Undo( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		if ( m_Module )
			m_Module.Undo();
	}

	void OnClick_Redo( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		if ( m_Module )
			m_Module.Redo();
	}

	// ========================================================================
	//  Asset browser
	// ========================================================================

	protected void BuildAssetBrowser()
	{
		if ( !m_AssetContent )
			return;

		UIActionManager.ClearChildren( m_AssetContent );

		array< ref JMMapEditorAsset > items = JMMapEditorCatalog.Build();
		array< string > categories = JMMapEditorCatalog.GetCategories();

		for ( int c = 0; c < categories.Count(); c++ )
		{
			string cat = categories.Get( c );
			UIActionManager.CreateText( m_AssetContent, cat );

			for ( int i = 0; i < items.Count(); i++ )
			{
				JMMapEditorAsset a = items.Get( i );
				if ( a.Category != cat )
					continue;

				UIActionButton row = UIActionManager.CreateButton( m_AssetContent, a.DisplayName, this, "" );
				if ( row ) row.SetOnClick( this, "OnClick_Asset" );
				row.SetWidth( 1.0 );
				row.SetData( new JMMapEditorAssetRowData( a.ClassName, a.DisplayName ) );
				row.SetTooltip( "Class: " + a.ClassName );
			}
		}

		m_AssetScroller.UpdateScroller();
	}

	void OnChange_AssetFilter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
		// Filter re-render: lazy - for now, the asset list is rebuilt wholesale
		// (no client-side asset DB). Future: pre-build + per-name search.
		BuildAssetBrowser();
	}

	void OnClick_Asset( UIActionBase action )
	{
		JMMapEditorAssetRowData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_PendingSpawnClass = data.ClassName;
		m_Mode = MAPEDITOR_MODE_SPAWN;
		RefreshModeButtons();
	}

	// ========================================================================
	//  Bulk + clipboard + undo click handlers
	// ========================================================================

	void OnClick_BulkMove( UIActionBase action )
	{
		if ( !m_Module || m_SelectedIds.Count() == 0 ) return;
		if ( m_SelectedId == -1 && m_PosX.GetText() == "" ) return;

		// Use the pos-text fields as the per-object delta. If empty,
		// default to a 1m forward nudge so the button is still useful.
		vector delta = vector.Zero;
		if ( m_PosX.GetText() != "" ) delta[0] = m_PosX.GetText().ToFloat();
		if ( m_PosY.GetText() != "" ) delta[1] = m_PosY.GetText().ToFloat();
		if ( m_PosZ.GetText() != "" ) delta[2] = m_PosZ.GetText().ToFloat();
		if ( delta == vector.Zero ) delta = Vector( 1, 0, 0 );

		int count = m_SelectedIds.Count();
		array< int >   ids  = new array< int >;
		array< vector > dp   = new array< vector >;
		array< vector > dor  = new array< vector >;
		array< float >  ds   = new array< float >;
		for ( int i = 0; i < count; i++ )
		{
			ids.Insert( m_SelectedIds.Get( i ) );
			dp.Insert( delta );
			dor.Insert( vector.Zero );
			ds.Insert( 1.0 );
		}
		m_Module.BulkTransform( ids, dp, dor, ds );
	}

	void OnClick_BulkRotate( UIActionBase action )
	{
		if ( !m_Module || m_SelectedIds.Count() == 0 ) return;

		float yawDelta = 0;
		if ( m_OriYaw.GetText() != "" )
			yawDelta = m_OriYaw.GetText().ToFloat();

		int count = m_SelectedIds.Count();
		array< int >   ids  = new array< int >;
		array< vector > dp   = new array< vector >;
		array< vector > dor  = new array< vector >;
		array< float >  ds   = new array< float >;
		for ( int i = 0; i < count; i++ )
		{
			ids.Insert( m_SelectedIds.Get( i ) );
			dp.Insert( vector.Zero );
			vector yawVec = Vector( 0, yawDelta, 0 );
			dor.Insert( yawVec );
			ds.Insert( 1.0 );
		}
		m_Module.BulkTransform( ids, dp, dor, ds );
	}

	void OnClick_BulkScale( UIActionBase action )
	{
		if ( !m_Module || m_SelectedIds.Count() == 0 ) return;

		float scaleFactor = m_Scale.GetCurrent();
		if ( scaleFactor <= 0 ) scaleFactor = 1.0;

		int count = m_SelectedIds.Count();
		array< int >   ids  = new array< int >;
		array< vector > dp   = new array< vector >;
		array< vector > dor  = new array< vector >;
		array< float >  ds   = new array< float >;
		for ( int i = 0; i < count; i++ )
		{
			ids.Insert( m_SelectedIds.Get( i ) );
			dp.Insert( vector.Zero );
			dor.Insert( vector.Zero );
			ds.Insert( scaleFactor );
		}
		m_Module.BulkTransform( ids, dp, dor, ds );
	}

	void OnClick_Copy( UIActionBase action )
	{
		if ( m_SelectedId == -1 || !m_Module ) return;
		m_Module.CopyObject( m_SelectedId );
	}

	void OnClick_Cut( UIActionBase action )
	{
		if ( m_SelectedId == -1 || !m_Module ) return;
		m_Module.CutObject( m_SelectedId );
		// After cut the selection no longer points to a live object
		m_SelectedId = -1;
		m_SelectedIds.Clear();
		RebuildList();
	}

	void OnClick_Paste( UIActionBase action )
	{
		if ( !m_Module )
		{
			Error("[JMMapEditorForm] OnClick_Paste failed: m_Module is null!");
			return;
		}

		// Spawn at the cursor's world position. Falls back to the selected
		// object's position when the cursor pick returned zero.
		vector pos = vector.Zero;
		int mx, my;
		GetMousePos( mx, my );
		vector cursorWorld; Object cursorObj;
		JMEditorScene.CastFromScreen( mx, my, cursorWorld, cursorObj );

		if ( cursorWorld != vector.Zero )
			pos = cursorWorld;
		else if ( m_SelectedId != -1 )
		{
			JMMapEditorEntry sel = FindEntry( m_SelectedId );
			if ( sel ) pos = sel.Position;
		}

		// Default orient / scale
		vector ori = vector.Zero;
		float   scale = 1.0;
		if ( m_SelectedId != -1 )
		{
			JMMapEditorEntry sourceEntry = FindEntry( m_SelectedId );
			if ( sourceEntry )
			{
				ori = sourceEntry.Orientation;
				scale = sourceEntry.Scale;
			}
		}

		m_Module.PasteCloned( pos, ori, scale );
	}

	// Re-render the placed-object list. Stub for now - the existing form
	// doesn't expose a single "rebuild all" method, so callers should
	// just call m_Module.RequestList() which fires the OnServerListUpdated
	// hook to repopulate the asset list.
	void RebuildList()
	{
		if ( m_Module )
			m_Module.RequestList();
	}

	// ========================================================================
	//  Server list hook
	// ========================================================================

	void OnServerListUpdated( array< ref JMMapEditorObject > serverList )
	{
		m_AllEntries.Clear();
		foreach ( JMMapEditorObject src : serverList )
		{
			JMMapEditorEntry e = new JMMapEditorEntry();
			e.ClassName   = src.ClassName;
			e.Id          = src.Id;
			e.Position    = src.Position;
			e.Orientation = src.Orientation;
			e.Scale       = src.Scale;
			m_AllEntries.Insert( e );
		}

		JMMapEditorEntry sel = FindEntry( m_SelectedId );
		if ( sel )
		{
			m_SelId.SetText( "#STR_COT_MAPEDITOR_ID" + ": " + m_SelectedId.ToString() );
			m_SelClass.SetText( "#STR_COT_MAPEDITOR_CLASS" + ": " + sel.ClassName );
			m_PosX.SetText( sel.Position[0].ToString() );
			m_PosY.SetText( sel.Position[1].ToString() );
			m_PosZ.SetText( sel.Position[2].ToString() );
			m_OriYaw.SetText( sel.Orientation[1].ToString() );
			m_Scale.SetCurrent( sel.Scale );
		}
	}

	// ========================================================================
	//  Selection / transform apply / delete
	// ========================================================================

	protected JMMapEditorEntry FindEntry( int id )
	{
		foreach ( JMMapEditorEntry e : m_AllEntries )
			if ( e.Id == id )
				return e;
		return NULL;
	}

	void OnClick_ApplyTransform( UIActionBase action )
	{
		if ( m_SelectedId == -1 || !m_Module )
			return;

		vector pos;
		pos[0] = m_PosX.GetText().ToFloat();
		pos[1] = m_PosY.GetText().ToFloat();
		pos[2] = m_PosZ.GetText().ToFloat();
		float yaw = m_OriYaw.GetText().ToFloat();
		vector ori = Vector( 0, yaw, 0 );
		float scale = m_Scale.GetCurrent();
		m_Module.TransformObject( m_SelectedId, pos, ori, scale );
	}

	void OnClick_TeleportTo( UIActionBase action )
	{
		if ( m_SelectedId == -1 )
			return;
		JMMapEditorEntry e = FindEntry( m_SelectedId );
		if ( !e )
			return;
		PlayerBase player = PlayerBase.Cast( g_Game.GetPlayer() );
		if ( player )
		{
			vector safe = e.Position;
			safe[1] = safe[1] + 1.5;
			player.SetPosition( safe );
		}
	}

	void OnClick_Delete( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		if ( m_SelectedId == -1 || !m_Module )
			return;
		m_ActiveConfirmation = ConfirmAction( "#STR_COT_MAPEDITOR_DELETE_OBJECT", "#STR_COT_MAPEDITOR_DELETE_THE_SELECTED_OBJECT", "OnConfirmation_Delete" );
	}

	void OnConfirmation_Delete( JMConfirmation confirmation )
	{
		if ( m_SelectedId == -1 || !m_Module )
			return;
		m_Module.DeleteObject( m_SelectedId );
		m_SelectedId = -1;
		m_SelId.SetText( "#STR_COT_MAPEDITOR_NO_SELECTION" );
		m_SelClass.SetText( "" );
	}

	void OnClick_ClearAll( UIActionBase action )
	{
		m_ActiveConfirmation = ConfirmAction( "#STR_COT_MAPEDITOR_CLEAR_ALL_2", "#STR_COT_MAPEDITOR_DELETE_EVERY_OBJECT_PLACED_BY_THE", "OnConfirmation_ClearAll" );
	}

	void OnConfirmation_ClearAll( JMConfirmation confirmation )
	{
		if ( m_Module )
			m_Module.ClearAll();
		m_SelectedId = -1;
	}
}