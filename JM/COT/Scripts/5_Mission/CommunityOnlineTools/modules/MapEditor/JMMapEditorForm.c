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
	private UIActionButton m_ModeSelect;
	private UIActionButton m_ModeMove;
	private UIActionButton m_ModeRotate;
	private UIActionButton m_ModeScale;
	private UIActionButton m_ModeSpawn;
	private UIActionButton m_Refresh;
	private UIActionButton m_Save;
	private UIActionButton m_Load;
	private UIActionButton m_Undo;
	private UIActionButton m_Redo;

	// --- Snap toggles ---
	private UIActionCheckbox m_SnapTerrain;
	private UIActionCheckbox m_SnapGrid;
	private UIActionCheckbox m_SnapSurface;

	// --- Mode + snaps runtime state ---
	private int m_Mode;
	private int m_Snaps;

	// --- Asset browser (left sidebar) ---
	private UIActionSearchBox m_AssetSearch;
	private UIActionScroller m_AssetScroller;
	private Widget m_AssetContent;

	// --- Selection ---
	private UIActionText m_SelId;
	private UIActionText m_SelClass;
	private UIActionEditableText m_PosX;
	private UIActionEditableText m_PosY;
	private UIActionEditableText m_PosZ;
	private UIActionEditableText m_OriYaw;
	private UIActionSlider m_Scale;
	private UIActionButton m_ApplyTransform;
	private UIActionButton m_TeleportTo;
	private UIActionButton m_Delete;
	private UIActionButton m_ClearAll;

	// ---- Bulk + clipboard + undo buttons ----
	private UIActionButton m_BulkMove;
	private UIActionButton m_BulkRotate;
	private UIActionButton m_BulkScale;
	private UIActionButton m_CopyBtn;
	private UIActionButton m_CutBtn;
	private UIActionButton m_PasteBtn;
	private UIActionButton m_UndoBtn;
	private UIActionButton m_RedoBtn;

	// --- Local mirror of server list + spawn state ---
	private ref array< ref JMMapEditorEntry > m_AllEntries;
	private int m_SelectedId;
	private string m_PendingSpawnClass;

	// --- Multi-select (shift-click to add / remove from the set) ---
	private ref set< int > m_SelectedIds;

	// --- 3D interaction state ---
	private ref JMEditorGizmos m_Gizmos;
	private vector m_DragStartOrigin;     // object pos when drag began
	private vector m_DragStartOrient;     // object orient when drag began
	private float   m_DragStartScale;      // object scale when drag began
	private int     m_DragAxis;            // -1 if not dragging
	private vector m_DragStartMouseWorld;  // cursor world pos at drag begin
	private int     m_PreviousMouseX;       // cursor px at last Tick (for delta)
	private int     m_PreviousMouseY;
	private bool    m_HasPreviousMouse;
	private bool    m_LmbDown;             // LMB held this frame
	private bool    m_RmbDown;             // RMB held this frame (use for look-only)
	private bool    m_LmbWasDown;          // LMB state from previous tick
	private bool    m_LmbClickPending;     // LMB just released - fire click handler next tick

	private const int  TICK_HANDLE = 0;
	private const int  TICK_PERIOD_MS = 33;  // ~30 fps UI tick

	// Static handle for the per-frame callback. The CallQueue can only
	// resolve a method by name when it's a static class member, so the
	// static TickStatic dispatches to the single active form.
	private static ref JMMapEditorForm s_Instance;

	private JMMapEditorModule m_Module;

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

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	// ========================================================================
	//  OnInit - build all UI
	// ========================================================================

	override void OnInit()
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

		m_Save = UIActionManager.CreateButton( modeRow, "Save", this, "OnClick_Save" );
		m_Save.SetWidth( 0.07 );
		m_Save.SetTooltip( "Save the current layout to a preset file on disk (per server)" );

		m_Load = UIActionManager.CreateButton( modeRow, "Load", this, "OnClick_Load" );
		m_Load.SetWidth( 0.07 );
		m_Load.SetTooltip( "Load a saved layout preset from disk" );

		m_Undo = UIActionManager.CreateButton( modeRow, "Undo", this, "OnClick_Undo" );
		m_Undo.SetWidth( 0.07 );
		m_Undo.SetTooltip( "Undo the last pending mutation (client-side stack)" );

		m_Redo = UIActionManager.CreateButton( modeRow, "Redo", this, "OnClick_Redo" );
		m_Redo.SetWidth( 0.07 );
		m_Redo.SetTooltip( "Redo a previously undone mutation" );

		m_Refresh = UIActionManager.CreateButton( modeRow, "Refresh", this, "OnClick_Refresh" );
		m_Refresh.SetWidth( 0.07 );
		m_Refresh.SetTooltip( "Reload the placed-object list from the server" );
		// No SetIcon - toolbar buttons are text-only so the icon doesn't
		// sit in the center. The vanilla button class doesn't expose an
		// image-align API, so the simpler path is to keep all toolbar
		// actions as plain text and put any icon hints in the tooltip.

		Widget snapRow = UIActionManager.CreateWrapSpacer( topStack );
		m_SnapTerrain = UIActionManager.CreateCheckbox( snapRow, "Snap to Terrain", this, "Click_SnapTerrain", false );
		m_SnapTerrain.SetWidth( 0.33 );
		m_SnapTerrain.SetTooltip( "Spawned/moved objects snap to the ground beneath them" );
		m_SnapGrid    = UIActionManager.CreateCheckbox( snapRow, "Snap to Grid (1m)", this, "Click_SnapGrid", false );
		m_SnapGrid.SetWidth( 0.33 );
		m_SnapGrid.SetTooltip( "Round coordinates to 1 m increments" );
		m_SnapSurface = UIActionManager.CreateCheckbox( snapRow, "Snap to Surface", this, "Click_SnapSurface", false );
		m_SnapSurface.SetWidth( 0.33 );
		m_SnapSurface.SetTooltip( "Align yaw to the surface normal" );

		// ---- Left sidebar: asset browser ----
		Widget left = layoutRoot.FindAnyWidget( "panel_left" );
		Widget leftStack = UIActionManager.CreateGridSpacer( left, 2, 1 );

		m_AssetSearch = UIActionManager.CreateSearchBox( leftStack, this, "OnChange_AssetFilter", "Search assets..." );
		m_AssetSearch.SetTooltip( "Filter the asset browser by classname or display name" );

		m_AssetScroller = UIActionManager.CreateScroller( leftStack );
		m_AssetContent  = m_AssetScroller.GetContentWidget();
		m_AssetScroller.UpdateScroller();

		// ---- Right panel: selection / properties ----
		Widget right = layoutRoot.FindAnyWidget( "panel_right" );
		Widget rightStack = UIActionManager.CreateGridSpacer( right, 7, 1 );

		UIActionManager.CreateText( rightStack, "Properties", "" );

		m_SelId = UIActionManager.CreateText( rightStack, "", "No selection" );
		m_SelId.SetWidth( 1.0 );

		m_SelClass = UIActionManager.CreateText( rightStack, "", "" );
		m_SelClass.SetWidth( 1.0 );

		Widget posRow = UIActionManager.CreateGridSpacer( rightStack, 1, 3 );
		m_PosX = UIActionManager.CreateEditableText( posRow, "X:", this, "" );
		m_PosX.SetWidth( 0.32 );
		m_PosX.SetTooltip( "World X (meters)" );
		m_PosY = UIActionManager.CreateEditableText( posRow, "Y:", this, "" );
		m_PosY.SetWidth( 0.32 );
		m_PosY.SetTooltip( "World Y (meters)" );
		m_PosZ = UIActionManager.CreateEditableText( posRow, "Z:", this, "" );
		m_PosZ.SetWidth( 0.32 );
		m_PosZ.SetTooltip( "World Z (meters)" );

		Widget oriRow = UIActionManager.CreateGridSpacer( rightStack, 1, 1 );
		m_OriYaw = UIActionManager.CreateEditableText( oriRow, "Yaw (deg):", this, "" );
		m_OriYaw.SetWidth( 1.0 );
		m_OriYaw.SetTooltip( "Yaw rotation in degrees (0-359)" );

		m_Scale = UIActionManager.CreateSlider( rightStack, "Scale:", 0.1, 5.0, this, "" );
		m_Scale.SetCurrent( 1.0 );
		m_Scale.SetStepValue( 0.1 );
		m_Scale.SetWidth( 0.95 );
		m_Scale.SetTooltip( "Uniform scale multiplier" );

		Widget actionRow = UIActionManager.CreateGridSpacer( rightStack, 1, 3 );
		m_ApplyTransform = UIActionManager.CreateButton( actionRow, "Apply", this, "OnClick_ApplyTransform" );
		m_ApplyTransform.SetWidth( 0.32 );
		m_ApplyTransform.SetTooltip( "Apply position / rotation / scale to the selected object" );
		m_TeleportTo = UIActionManager.CreateButton( actionRow, "Teleport", this, "OnClick_TeleportTo" );
		m_TeleportTo.SetWidth( 0.32 );
		m_TeleportTo.SetTooltip( "Teleport your player to the selected object" );
		m_Delete = UIActionManager.CreateButton( actionRow, "Delete", this, "OnClick_Delete" );
		m_Delete.SetColor( JMTheme.DANGER_FILL );
		m_Delete.SetWidth( 0.32 );
		m_Delete.SetTooltip( "Delete the selected object" );

		Widget clearRow = UIActionManager.CreateGridSpacer( rightStack, 1, 1 );
		m_ClearAll = UIActionManager.CreateButton( clearRow, "Clear All", this, "OnClick_ClearAll" );
		m_ClearAll.SetColor( JMTheme.DANGER_FILL );
		m_ClearAll.SetWidth( 1.0 );
		m_ClearAll.SetTooltip( "Delete every placed object (irreversible)" );

		// ---- Bulk + clipboard + undo/undo rows ----
		Widget bulkRow = UIActionManager.CreateGridSpacer( rightStack, 1, 3 );
		m_BulkMove   = UIActionManager.CreateButton( bulkRow, "Bulk Move", this, "OnClick_BulkMove" );
		m_BulkMove.SetWidth( 0.33 );
		m_BulkMove.SetTooltip( "Apply X / Y / Z delta to the cursor (or use the pos fields above). Affects all selected objects" );
		m_BulkRotate = UIActionManager.CreateButton( bulkRow, "Bulk Rotate", this, "OnClick_BulkRotate" );
		m_BulkRotate.SetWidth( 0.33 );
		m_BulkRotate.SetTooltip( "Add the yaw value (deg) above) to all selected objects" );
		m_BulkScale  = UIActionManager.CreateButton( bulkRow, "Bulk Scale", this, "OnClick_BulkScale" );
		m_BulkScale.SetWidth( 0.33 );
		m_BulkScale.SetTooltip( "Multiply the current scale of every selected object by the scale value above" );

		Widget clipRow = UIActionManager.CreateGridSpacer( rightStack, 1, 3 );
		m_CopyBtn   = UIActionManager.CreateButton( clipRow, "Copy", this, "OnClick_Copy" );
		m_CopyBtn.SetWidth( 0.33 );
		m_CopyBtn.SetTooltip( "Snapshot the currently-selected object into the clipboard" );
		m_CutBtn    = UIActionManager.CreateButton( clipRow, "Cut", this, "OnClick_Cut" );
		m_CutBtn.SetWidth( 0.33 );
		m_CutBtn.SetTooltip( "Snapshot + delete the currently-selected object" );
		m_PasteBtn  = UIActionManager.CreateButton( clipRow, "Paste", this, "OnClick_Paste" );
		m_PasteBtn.SetWidth( 0.33 );
		m_PasteBtn.SetTooltip( "Spawn the clipboard at the cursor world position (snap-to-terrain on)" );

		Widget undoRow = UIActionManager.CreateGridSpacer( rightStack, 1, 2 );
		m_UndoBtn = UIActionManager.CreateButton( undoRow, "Undo", this, "OnClick_Undo" );
		m_UndoBtn.SetWidth( 0.49 );
		m_UndoBtn.SetTooltip( "Revert the last server-side mutation" );
		m_RedoBtn = UIActionManager.CreateButton( undoRow, "Redo", this, "OnClick_Redo" );
		m_RedoBtn.SetWidth( 0.49 );
		m_RedoBtn.SetTooltip( "Re-apply a reverted mutation" );

		// ---- How to use ----
		Widget helpRow = UIActionManager.CreateGridSpacer( rightStack, 1, 1 );
		Widget help = UIActionManager.CreatePanel( helpRow, 0x00000000, 140 );
		UIActionManager.CreateText( help, "How to use:", "1. Open the freecam (Camera module). 2. Click an object in the 3D world to select. 3. Shift-click to multi-select. 4. Use Bulk Move / Rotate / Scale to act on every selected. 5. Use Copy / Cut / Paste to clone. 6. Undo / Redo for history. 7. Snap to Terrain / Grid / Surface as needed." );

		// ---- Initial paint ----
		RefreshModeButtons();
		BuildAssetBrowser();
	}

	override void OnShow()
	{
		super.OnShow();

		m_Refresh.UpdatePermission( "Admin.MapEditor.View" );
		m_Save.UpdatePermission( "Admin.MapEditor.Spawn" );
		m_Load.UpdatePermission( "Admin.MapEditor.Spawn" );
		m_ApplyTransform.UpdatePermission( "Admin.MapEditor.Transform" );
		m_TeleportTo.UpdatePermission( "Admin.MapEditor.Transform" );
		m_Delete.UpdatePermission( "Admin.MapEditor.Delete" );
		m_ClearAll.UpdatePermission( "Admin.MapEditor.Delete" );

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

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();
		m_Refresh.UpdatePermission( "Admin.MapEditor.View" );
		m_Save.UpdatePermission( "Admin.MapEditor.Spawn" );
		m_Load.UpdatePermission( "Admin.MapEditor.Spawn" );
		m_ApplyTransform.UpdatePermission( "Admin.MapEditor.Transform" );
		m_TeleportTo.UpdatePermission( "Admin.MapEditor.Transform" );
		m_Delete.UpdatePermission( "Admin.MapEditor.Delete" );
		m_ClearAll.UpdatePermission( "Admin.MapEditor.Delete" );
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

	private UIActionButton MakeModeButton( Widget parent, string label, int mode, string tooltip )
	{
		UIActionButton b = UIActionManager.CreateButton( parent, label, this, "OnClick_Mode" );
		b.SetWidth( 0.08 );
		b.SetData( new JMMapEditorModeData( mode ) );
		b.SetTooltip( tooltip );
		return b;
	}

	void OnClick_Mode( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMMapEditorModeData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_Mode = data.Mode;
		m_PendingSpawnClass = "";
		RefreshModeButtons();
	}

	private void RefreshModeButtons()
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

	void OnClick_Save( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		if ( m_Module )
			m_Module.SaveToDisk();
	}

	void OnClick_Load( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
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
		// Server-side undo would require inverse RPCs; client-side best-effort
		// is intentionally a no-op for now.
	}

	void OnClick_Redo( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
	}

	// ========================================================================
	//  Asset browser
	// ========================================================================

	private void BuildAssetBrowser()
	{
		if ( !m_AssetContent )
			return;

		Widget child = m_AssetContent.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}

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

				UIActionButton row = UIActionManager.CreateButton( m_AssetContent, a.DisplayName, this, "OnClick_Asset" );
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

	void OnClick_Asset( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

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

	void OnClick_BulkMove( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
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

	void OnClick_BulkRotate( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
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

	void OnClick_BulkScale( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
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

	void OnClick_Copy( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		if ( m_SelectedId == -1 || !m_Module ) return;
		m_Module.CopyObject( m_SelectedId );
	}

	void OnClick_Cut( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		if ( m_SelectedId == -1 || !m_Module ) return;
		m_Module.CutObject( m_SelectedId );
		// After cut the selection no longer points to a live object
		m_SelectedId = -1;
		m_SelectedIds.Clear();
		RebuildList();
	}

	void OnClick_Paste( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		if ( !m_Module ) return;

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
			m_SelId.SetText( "ID: " + m_SelectedId.ToString() );
			m_SelClass.SetText( "Class: " + sel.ClassName );
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

	private JMMapEditorEntry FindEntry( int id )
	{
		foreach ( JMMapEditorEntry e : m_AllEntries )
			if ( e.Id == id )
				return e;
		return NULL;
	}

	void OnClick_ApplyTransform( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
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

	void OnClick_TeleportTo( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
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
		CreateConfirmation_Two( JMConfirmationType.INFO, "Delete object", "Delete the selected object?", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "OnConfirmation_Delete" );
	}

	void OnConfirmation_Delete( JMConfirmation confirmation )
	{
		if ( m_SelectedId == -1 || !m_Module )
			return;
		m_Module.DeleteObject( m_SelectedId );
		m_SelectedId = -1;
		m_SelId.SetText( "No selection" );
		m_SelClass.SetText( "" );
	}

	void OnClick_ClearAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		CreateConfirmation_Two( JMConfirmationType.INFO, "Clear all", "Delete every object placed by the map editor?", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "OnConfirmation_ClearAll" );
	}

	void OnConfirmation_ClearAll( JMConfirmation confirmation )
	{
		if ( m_Module )
			m_Module.ClearAll();
		m_SelectedId = -1;
	}
}