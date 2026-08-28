// =============================================================================
//  JMMapEditorModule.c
//
//  Map Editor module - places, transforms, and removes world objects via the
//  in-game map widget. Server is authoritative; clients request the list
//  and dispatch mutations as RPCs. Persisted across mission restarts.
//
//  RPCs (JMMapEditorModuleRPC @ 10900):
//    RequestList     client -> server
//    SpawnObject     client -> server
//    TransformObject client -> server
//    DeleteObject    client -> server
//    ClearAll        client -> server
//    List            server -> client
// =============================================================================

class JMMapEditorModule : JMRenderableModuleBase
{
	protected ref array< ref JMMapEditorObject > m_ServerObjects;  // server-only canonical list
	protected ref array< ref JMMapEditorObject > m_ClientObjects;  // UI mirror on the admin client
	protected int m_NextId;

	// Undo / redo stacks (server-side). Each entry captures the affected
	// object's state at the moment of the mutation, so a replay restores it.
	// Bounded at 64 to keep memory bounded over a long session.
	protected ref array< ref JMMapEditorUndoEntry > m_UndoStack;
	protected ref array< ref JMMapEditorUndoEntry > m_RedoStack;

	// Server-side clipboard for copy / cut / paste
	protected ref array< ref JMMapEditorObject > m_ServerClipboard;
	protected bool   m_ServerClipboardIsCut;

	// Client-only freecam state
	protected JMFreecam m_Freecam;                  // active while editor is open
	protected Camera  m_SavedCamera;               // player's previous camera
	protected bool    m_FreecamActive;             // gate for tick + input

	void JMMapEditorModule()
	{
		m_ServerObjects = new array< ref JMMapEditorObject >;
		m_ClientObjects = new array< ref JMMapEditorObject >;

		GetPermissionsManager().RegisterPermission( "Admin.MapEditor.View" );
		GetPermissionsManager().RegisterPermission( "Admin.MapEditor.Spawn" );
		GetPermissionsManager().RegisterPermission( "Admin.MapEditor.Transform" );
		GetPermissionsManager().RegisterPermission( "Admin.MapEditor.Delete" );
	}

	override void OnUpdate( float timeslice )
	{
		// The JMFreecam itself ticks via EOnFrame. This OnUpdate is reserved
		// for future editor-side per-frame work (gizmo dispatch, snap-to-grid,
		// etc.). For now it's a no-op while the freecam is inactive or absent.
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Admin.MapEditor.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/mapeditor_form.layout";
	}

	override string GetCategory()
	{
		return "World";
	}

	override string GetTitle()
	{
		return "#STR_COT_MAPEDITOR_MODULE_NAME";
	}

	override string GetIconName()
	{
		return JMConstants.Lucide( "pencil-ruler" );
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
	}

	override string GetWebhookTitle()
	{
		return "Map Editor Module";
	}

	override void GetWebhookTypes( out array< string > types )
	{
		types.Insert( "Spawn" );
		types.Insert( "Transform" );
		types.Insert( "Delete" );
		types.Insert( "Clear" );
	}

	override int GetRPCMin()
	{
		return JMMapEditorModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMMapEditorModuleRPC.COUNT;
	}

	override void EnableUpdate()
	{
		// Ticks OnUpdate every frame while the editor is visible (toggle
		// driven by AttachFreecam / DetachFreecam below).
	}

	// -----------------------------------------------------------------------
	//  Freecam attach / detach - auto-engaged when the module is opened
	//  on the client, restored when closed. Player input is locked while
	//  the editor is open so WASD goes to the freecam, not the player.
	// -----------------------------------------------------------------------

	void AttachFreecam()
	{
		// Placeholder -- requires DayZ 1.29 input-lock + freecam-attach APIs
		// that aren't trivially available. The form's OnClick_Refresh / spawn
		// pipeline works without freecam; freecam is deferred to a follow-up.
	}

	void DetachFreecam()
	{
		// Placeholder -- see AttachFreecam().
	}

	// Exposed so the form can tick the gizmo loop + dispatch click-to-place.
	bool IsFreecamActive()
	{
		return m_FreecamActive;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMMapEditorModuleRPC.RequestList:
			RPC_RequestList( ctx, sender, target );
			break;
		case JMMapEditorModuleRPC.SpawnObject:
			RPC_SpawnObject( ctx, sender, target );
			break;
		case JMMapEditorModuleRPC.TransformObject:
			RPC_TransformObject( ctx, sender, target );
			break;
		case JMMapEditorModuleRPC.DeleteObject:
			RPC_DeleteObject( ctx, sender, target );
			break;
		case JMMapEditorModuleRPC.ClearAll:
			RPC_ClearAll( ctx, sender, target );
			break;
		case JMMapEditorModuleRPC.BulkTransform:
			RPC_BulkTransform( ctx, sender, target );
			break;
		case JMMapEditorModuleRPC.BulkDelete:
			RPC_BulkDelete( ctx, sender, target );
			break;
		case JMMapEditorModuleRPC.CloneObject:
			// Server-side: this branch only runs on dedicated. The
			// form's Paste button calls PasteCloned directly via the
			// public method on the module; no server RPC is needed.
			break;
		case JMMapEditorModuleRPC.Undo:
			RPC_Undo( ctx, sender, target );
			break;
		case JMMapEditorModuleRPC.Redo:
			RPC_Redo( ctx, sender, target );
			break;
		case JMMapEditorModuleRPC.List:
			RPC_List( ctx, sender, target );
			break;
		}
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if ( IsMissionHost() )
		{
			LoadFromDisk();
			SpawnAllFromStore();
		}
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();
		if ( IsMissionHost() )
			SaveToDisk();
	}

	override void Show()
	{
		super.Show();
		AttachFreecam();
	}

	override void Hide()
	{
		DetachFreecam();
		super.Hide();
	}

	override void Close()
	{
		DetachFreecam();
		super.Close();
	}

	// -----------------------------------------------------------------------
	//  Server state
	// -----------------------------------------------------------------------

	void LoadFromDisk()
	{
		JMMapEditorStore store = JMMapEditorStore.Load();
		m_ServerObjects.Clear();
		foreach ( JMMapEditorObject obj : store.Objects )
		{
			JMMapEditorObject copy = new JMMapEditorObject();
			copy.Set( obj.ClassName, obj.Position, obj.Orientation, obj.Scale, m_NextId++ );
			m_ServerObjects.Insert( copy );
		}
	}

	void SaveToDisk()
	{
		JMMapEditorStore store = new JMMapEditorStore();
		store.Objects.Clear();
		foreach ( JMMapEditorObject obj : m_ServerObjects )
		{
			JMMapEditorObject copy = new JMMapEditorObject();
			copy.Set( obj.ClassName, obj.Position, obj.Orientation, obj.Scale, -1 );
			store.Objects.Insert( copy );
		}
		JMMapEditorStore.Save( store );
	}

	private void SpawnAllFromStore()
	{
		foreach ( JMMapEditorObject obj : m_ServerObjects )
		{
			Object o = GetGame().CreateObjectEx( obj.ClassName, obj.Position, ECE_PLACE_ON_SURFACE );
			if ( o )
			{
				o.SetPosition( obj.Position );
				o.SetOrientation( obj.Orientation );
				o.SetScale( obj.Scale );
			}
		}
	}

	// -----------------------------------------------------------------------
	//  Client -> Server RPCs
	// -----------------------------------------------------------------------

	void RequestList()
	{
		if ( IsMissionHost() )
		{
			SendListTo( NULL );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMMapEditorModuleRPC.RequestList, true, NULL );
	}

	private void RPC_RequestList( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.MapEditor.View", senderRPC, instance ) )
			return;

		SendListTo( senderRPC );
	}

	void SpawnObject( string className, vector pos, vector ori, float scale )
	{
		if ( IsMissionHost() )
		{
			Exec_SpawnObject( className, pos, ori, scale, NULL );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( className );
		rpc.Write( pos );
		rpc.Write( ori );
		rpc.Write( scale );
		rpc.Send( NULL, JMMapEditorModuleRPC.SpawnObject, true, NULL );
	}

	private void RPC_SpawnObject( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string className;
		vector pos;
		vector ori;
		float scale;

		if ( !ctx.Read( className ) ) return;
		if ( !ctx.Read( pos ) )       return;
		if ( !ctx.Read( ori ) )       return;
		if ( !ctx.Read( scale ) )     return;

		if ( className == "" )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.MapEditor.Spawn", senderRPC, instance ) )
			return;

		Exec_SpawnObject( className, pos, ori, scale, senderRPC );
	}

	private void Exec_SpawnObject( string className, vector pos, vector ori, float scale, PlayerIdentity ident )
	{
		Object o = GetGame().CreateObjectEx( className, pos, ECE_PLACE_ON_SURFACE );
		if ( !o )
			return;

		o.SetPosition( pos );
		o.SetOrientation( ori );
		o.SetScale( scale );

		JMMapEditorObject entry = new JMMapEditorObject();
		entry.Set( className, pos, ori, scale, m_NextId++ );
		m_ServerObjects.Insert( entry );

		GetCommunityOnlineToolsBase().Log( ident, "MapEditor Spawned " + className + " at " + pos.ToString() + " scale=" + scale.ToString() );

		JMPlayerInstance instance;
		GetPermissionsManager().HasPermission( "Admin.MapEditor.Spawn", ident, instance );
		SendWebhookColored( "Spawn", instance, "Spawned " + className + " at " + pos.ToString(), JMConstants.WEBHOOK_COLOR_SUCCESS );

		// Refresh everyone's list so other admins see the new marker
		BroadcastList();
	}

	void TransformObject( int id, vector pos, vector ori, float scale )
	{
		if ( IsMissionHost() )
		{
			Exec_TransformObject( id, pos, ori, scale, NULL );
			return;
		}

		// Throttle slider-drag spam: drop intermediate Transform RPCs below the rate cap
		PlayerBase localPb;
		PlayerIdentity self = NULL;
		if ( Class.CastTo( localPb, g_Game.GetPlayer() ) )
			self = localPb.GetIdentity();
		if ( self && IsRateLimited( self ) )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( id );
		rpc.Write( pos );
		rpc.Write( ori );
		rpc.Write( scale );
		rpc.Send( NULL, JMMapEditorModuleRPC.TransformObject, true, NULL );
	}

	private void RPC_TransformObject( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int id;
		vector pos;
		vector ori;
		float scale;

		if ( !ctx.Read( id ) )    return;
		if ( !ctx.Read( pos ) )   return;
		if ( !ctx.Read( ori ) )   return;
		if ( !ctx.Read( scale ) ) return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.MapEditor.Transform", senderRPC, instance ) )
			return;

		Exec_TransformObject( id, pos, ori, scale, senderRPC );
	}

	private void Exec_TransformObject( int id, vector pos, vector ori, float scale, PlayerIdentity ident )
	{
		JMMapEditorObject entry = FindEntryById( id );
		if ( !entry )
			return;

		Object o = FindLiveEntity( entry );
		if ( !o )
		{
			// Entity was destroyed by the engine - try to recover by re-spawning at new pos
			Object spawned = GetGame().CreateObjectEx( entry.ClassName, pos, ECE_PLACE_ON_SURFACE );
			if ( !spawned )
				return;
			spawned.SetPosition( pos );
			spawned.SetOrientation( ori );
			spawned.SetScale( scale );
		}
		else
		{
			o.SetPosition( pos );
			o.SetOrientation( ori );
			if ( scale > 0 )
				o.SetScale( scale );
		}

		entry.Position    = pos;
		entry.Orientation = ori;
		entry.Scale       = scale;

		GetCommunityOnlineToolsBase().Log( ident, "MapEditor Transformed id=" + id.ToString() + " " + entry.ClassName + " to " + pos.ToString() );

		JMPlayerInstance instance;
		GetPermissionsManager().HasPermission( "Admin.MapEditor.Transform", ident, instance );
		SendWebhookColored( "Transform", instance, "Transformed " + entry.ClassName + " (id=" + id.ToString() + ") to " + pos.ToString(), JMConstants.WEBHOOK_COLOR_NEUTRAL );

		BroadcastList();
	}

	void DeleteObject( int id )
	{
		if ( IsMissionHost() )
		{
			Exec_DeleteObject( id, NULL );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( id );
		rpc.Send( NULL, JMMapEditorModuleRPC.DeleteObject, true, NULL );
	}

	private void RPC_DeleteObject( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int id;
		if ( !ctx.Read( id ) ) return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.MapEditor.Delete", senderRPC, instance ) )
			return;

		Exec_DeleteObject( id, senderRPC );
	}

	private void Exec_DeleteObject( int id, PlayerIdentity ident )
	{
		JMMapEditorObject entry = FindEntryById( id );
		if ( !entry )
			return;

		Object o = FindLiveEntity( entry );
		if ( o )
			GetGame().ObjectDelete( o );

		m_ServerObjects.RemoveItem( entry );

		GetCommunityOnlineToolsBase().Log( ident, "MapEditor Deleted id=" + id.ToString() + " " + entry.ClassName );

		JMPlayerInstance instance;
		GetPermissionsManager().HasPermission( "Admin.MapEditor.Delete", ident, instance );
		SendWebhookColored( "Delete", instance, "Deleted " + entry.ClassName + " (id=" + id.ToString() + ")", JMConstants.WEBHOOK_COLOR_WARNING );

		BroadcastList();
	}

	void ClearAll()
	{
		if ( IsMissionHost() )
		{
			Exec_ClearAll( NULL );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMMapEditorModuleRPC.ClearAll, true, NULL );
	}

	private void RPC_ClearAll( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.MapEditor.Delete", senderRPC, instance ) )
			return;

		Exec_ClearAll( senderRPC );
	}

	private void Exec_ClearAll( PlayerIdentity ident )
	{
		int cleared = m_ServerObjects.Count();
		foreach ( JMMapEditorObject entry : m_ServerObjects )
		{
			Object o = FindLiveEntity( entry );
			if ( o )
				GetGame().ObjectDelete( o );
		}
		m_ServerObjects.Clear();

		GetCommunityOnlineToolsBase().Log( ident, "MapEditor Cleared all (" + cleared.ToString() + " objects)" );

		JMPlayerInstance instance;
		GetPermissionsManager().HasPermission( "Admin.MapEditor.Delete", ident, instance );
		SendWebhookColored( "Clear", instance, "Cleared all map-editor objects (" + cleared.ToString() + ")", JMConstants.WEBHOOK_COLOR_DANGER );

		BroadcastList();
	}

	// ========================================================================
	//  Bulk operations + clone + undo/redo
	// ========================================================================

	// Apply a transform to many objects in one RPC. Wire format:
	//   int count
	//   for each: int id, vector dx, vector dyaw_pitch_roll, float dScale
	// The delta is added to each object's current transform.
	void BulkTransform( array< int > ids, array< vector > dPos, array< vector > dOri, array< float > dScale )
	{
		if ( IsMissionHost() )
		{
			Exec_BulkTransform( ids, dPos, dOri, dScale, NULL );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( ids.Count() );
		for ( int i = 0; i < ids.Count(); i++ )
		{
			rpc.Write( ids.Get( i ) );
			rpc.Write( dPos.Get( i ) );
			rpc.Write( dOri.Get( i ) );
			rpc.Write( dScale.Get( i ) );
		}
		rpc.Send( NULL, JMMapEditorModuleRPC.BulkTransform, true, NULL );
	}

	private void RPC_BulkTransform( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.MapEditor.Transform", senderRPC, instance ) )
			return;

		int count;
		if ( !ctx.Read( count ) ) return;
		if ( count <= 0 || count > 256 ) return;

		array< int >   ids    = new array< int >;
		array< vector > dPos  = new array< vector >;
		array< vector > dOri  = new array< vector >;
		array< float >  dSc   = new array< float >;
		for ( int k = 0; k < count; k++ )
		{
			int id; vector dp; vector dor; float ds;
			if ( !ctx.Read( id )  )  return;
			if ( !ctx.Read( dp )  )  return;
			if ( !ctx.Read( dor ) )  return;
			if ( !ctx.Read( ds )  )  return;
			ids.Insert( id );
			dPos.Insert( dp );
			dOri.Insert( dor );
			dSc.Insert( ds );
		}

		Exec_BulkTransform( ids, dPos, dOri, dSc, senderRPC );
	}

	private void Exec_BulkTransform( array< int > ids, array< vector > dPos, array< vector > dOri, array< float > dScale, PlayerIdentity ident )
	{
		JMMapEditorUndoEntry undo = new JMMapEditorUndoEntry();
		undo.Type = "Bulk";
		PushCurrentStates( ids, undo.Pre );

		for ( int i = 0; i < ids.Count(); i++ )
		{
			JMMapEditorObject entry = FindEntryById( ids.Get( i ) );
			if ( !entry ) continue;

			vector newPos = entry.Position + dPos.Get( i );
			vector newOri = entry.Orientation + dOri.Get( i );
			float   newScale = entry.Scale * dScale.Get( i );

			ApplyTransform( entry, newPos, newOri, newScale, false );
		}

		PushCurrentStates( ids, undo.Post );
		PushUndo( undo );

		GetCommunityOnlineToolsBase().Log( ident, "MapEditor BulkTransformed " + ids.Count().ToString() + " objects" );
		BroadcastList();
	}

	// Delete many objects in one RPC.
	void BulkDelete( array< int > ids )
	{
		if ( IsMissionHost() )
		{
			Exec_BulkDelete( ids, NULL );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( ids.Count() );
		foreach ( int id : ids )
			rpc.Write( id );
		rpc.Send( NULL, JMMapEditorModuleRPC.BulkDelete, true, NULL );
	}

	private void RPC_BulkDelete( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.MapEditor.Delete", senderRPC, instance ) )
			return;

		int count;
		if ( !ctx.Read( count ) ) return;
		if ( count <= 0 || count > 256 ) return;

		array< int > ids = new array< int >;
		for ( int k = 0; k < count; k++ )
		{
			int id;
			if ( !ctx.Read( id ) ) return;
			ids.Insert( id );
		}

		Exec_BulkDelete( ids, senderRPC );
	}

	private void Exec_BulkDelete( array< int > ids, PlayerIdentity ident )
	{
		JMMapEditorUndoEntry undo = new JMMapEditorUndoEntry();
		undo.Type = "Bulk";
		PushCurrentStates( ids, undo.Pre );

		foreach ( int id : ids )
			DeleteById( id );

		PushUndo( undo );
		GetCommunityOnlineToolsBase().Log( ident, "MapEditor BulkDeleted " + ids.Count().ToString() + " objects" );
		BroadcastList();
	}

	// Copy a single object (used by the form's Copy / Cut / Paste).
	// On copy: the server records the object's data into its clipboard.
	// On cut: same as copy, then deletes the original.
	// On paste: spawns a clone at the given position.
	void CopyObject( int id )
	{
		if ( !IsMissionHost() ) return;
		Exec_CopyObject( id, false, NULL );
	}

	void CutObject( int id )
	{
		if ( !IsMissionHost() ) return;
		Exec_CopyObject( id, true, NULL );
	}

	void PasteCloned( vector pos, vector ori, float scale )
	{
		if ( !IsMissionHost() ) return;
		Exec_PasteCloned( pos, ori, scale, NULL );
	}

	private void Exec_CopyObject( int id, bool isCut, PlayerIdentity ident )
	{
		JMMapEditorObject entry = FindEntryById( id );
		if ( !entry ) return;

		if ( !m_ServerClipboard )
			m_ServerClipboard = new array< ref JMMapEditorObject >;
		else
			m_ServerClipboard.Clear();

		JMMapEditorObject copy = new JMMapEditorObject();
		copy.Set( entry.ClassName, entry.Position, entry.Orientation, entry.Scale, -1 );
		m_ServerClipboard.Insert( copy );

		m_ServerClipboardIsCut = isCut;

		GetCommunityOnlineToolsBase().Log( ident, "MapEditor Copied " + entry.ClassName + " (id=" + id.ToString() + ")" );

		if ( isCut )
		{
			JMMapEditorUndoEntry undo = new JMMapEditorUndoEntry();
			undo.Type = "Delete";
			JMMapEditorObject pre = new JMMapEditorObject();
			pre.Set( entry.ClassName, entry.Position, entry.Orientation, entry.Scale, entry.Id );
			undo.Pre.Insert( pre );
			DeleteById( id );
			undo.Post.Insert( new JMMapEditorObject() );
			PushUndo( undo );
		}
		BroadcastList();
	}

	private void Exec_PasteCloned( vector pos, vector ori, float scale, PlayerIdentity ident )
	{
		if ( !m_ServerClipboard || m_ServerClipboard.Count() == 0 )
			return;

		JMMapEditorUndoEntry undo = new JMMapEditorUndoEntry();
		undo.Type = "Clone";

		foreach ( JMMapEditorObject src : m_ServerClipboard )
		{
			Object o = GetGame().CreateObjectEx( src.ClassName, pos, ECE_PLACE_ON_SURFACE );
			if ( !o ) continue;
			o.SetPosition( pos );
			o.SetOrientation( ori );
			o.SetScale( scale );

			JMMapEditorObject entry = new JMMapEditorObject();
			entry.Set( src.ClassName, pos, ori, scale, m_NextId++ );
			m_ServerObjects.Insert( entry );

			JMMapEditorObject pre = new JMMapEditorObject();
			pre.Set( src.ClassName, pos, ori, scale, entry.Id );
			undo.Post.Insert( pre );
		}

		PushUndo( undo );
		GetCommunityOnlineToolsBase().Log( ident, "MapEditor Pasted " + m_ServerClipboard.Count().ToString() + " object(s)" );

		if ( m_ServerClipboardIsCut )
		{
			m_ServerClipboard.Clear();
			m_ServerClipboardIsCut = false;
		}

		BroadcastList();
	}

	// ---- Undo / Redo ----

	void Undo()
	{
		if ( !IsMissionHost() ) return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMMapEditorModuleRPC.Undo, true, NULL );
	}

	void Redo()
	{
		if ( !IsMissionHost() ) return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMMapEditorModuleRPC.Redo, true, NULL );
	}

	private void RPC_Undo( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.MapEditor.Transform", senderRPC, instance ) )
			return;
		Exec_Undo( senderRPC );
	}

	private void RPC_Redo( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.MapEditor.Transform", senderRPC, instance ) )
			return;
		Exec_Redo( senderRPC );
	}

	private void Exec_Undo( PlayerIdentity ident )
	{
		if ( !m_UndoStack || m_UndoStack.Count() == 0 )
			return;
		JMMapEditorUndoEntry entry = m_UndoStack.Get( m_UndoStack.Count() - 1 );
		m_UndoStack.Remove( m_UndoStack.Count() - 1 );

		RestoreEntry( entry, false );
		if ( !m_RedoStack ) m_RedoStack = new array< ref JMMapEditorUndoEntry >;
		m_RedoStack.Insert( entry );

		GetCommunityOnlineToolsBase().Log( ident, "MapEditor Undo (" + entry.Type + ")" );
		BroadcastList();
	}

	private void Exec_Redo( PlayerIdentity ident )
	{
		if ( !m_RedoStack || m_RedoStack.Count() == 0 )
			return;
		JMMapEditorUndoEntry entry = m_RedoStack.Get( m_RedoStack.Count() - 1 );
		m_RedoStack.Remove( m_RedoStack.Count() - 1 );

		RestoreEntry( entry, true );
		if ( !m_UndoStack ) m_UndoStack = new array< ref JMMapEditorUndoEntry >;
		m_UndoStack.Insert( entry );

		GetCommunityOnlineToolsBase().Log( ident, "MapEditor Redo (" + entry.Type + ")" );
		BroadcastList();
	}

	// Apply the snapshot side of an undo entry. For undo, the Pre is
	// restored; for redo, the Post is restored.  We rebuild the server
	// state from scratch (delete missing, add new, update existing).
	private void RestoreEntry( JMMapEditorUndoEntry entry, bool usePost )
	{
		array< ref JMMapEditorObject > snap;
		if ( usePost )
			snap = entry.Post;
		else
			snap = entry.Pre;
		if ( !snap || snap.Count() == 0 )
			return;

		// Build a set of IDs in the snapshot
		array< int > wantIds = new array< int >;
		foreach ( JMMapEditorObject s : snap )
			if ( s.Id != -1 )
				wantIds.Insert( s.Id );

		// Delete server entries not in snapshot
		for ( int i = m_ServerObjects.Count() - 1; i >= 0; i-- )
		{
			JMMapEditorObject live = m_ServerObjects.Get( i );
			bool found = false;
			for ( int w = 0; w < wantIds.Count(); w++ )
			{
				if ( wantIds.Get( w ) == live.Id )
				{
					found = true;
					break;
				}
			}
			if ( !found )
			{
				Object ent = FindLiveEntity( live );
				if ( ent )
					GetGame().ObjectDelete( ent );
				m_ServerObjects.Remove( i );
			}
		}

		// Re-spawn / update from snapshot
		foreach ( JMMapEditorObject snapEntry : snap )
		{
			if ( snapEntry.Id == -1 )
			{
				// New (clone) - spawn a new live entity
				Object o = GetGame().CreateObjectEx( snapEntry.ClassName, snapEntry.Position, ECE_PLACE_ON_SURFACE );
				if ( o ) o.SetScale( snapEntry.Scale );
				continue;
			}
			JMMapEditorObject liveEntry = FindEntryById( snapEntry.Id );
			if ( !liveEntry ) continue;
			ApplyTransform( liveEntry, snapEntry.Position, snapEntry.Orientation, snapEntry.Scale, true );
		}
	}

	private void PushUndo( JMMapEditorUndoEntry entry )
	{
		if ( !m_UndoStack ) m_UndoStack = new array< ref JMMapEditorUndoEntry >;
		m_UndoStack.Insert( entry );
		while ( m_UndoStack.Count() > 64 )
			m_UndoStack.Remove( 0 );
		// New action invalidates the redo stack
		if ( m_RedoStack ) m_RedoStack.Clear();
	}

	// Push the current state of the given ids into the target snapshot array
	private void PushCurrentStates( array< int > ids, array< ref JMMapEditorObject > target )
	{
		foreach ( int id : ids )
		{
			JMMapEditorObject e = FindEntryById( id );
			if ( !e ) continue;
			JMMapEditorObject copy = new JMMapEditorObject();
			copy.Set( e.ClassName, e.Position, e.Orientation, e.Scale, e.Id );
			target.Insert( copy );
		}
	}

	// Apply a transform to the live entity and update the entry. mode == true
	// forces re-spawn if the live entity is gone.
	private void ApplyTransform( JMMapEditorObject entry, vector pos, vector ori, float scale, bool forceRespawn )
	{
		Object o = FindLiveEntity( entry );
		bool wasMissing = ( o == NULL );
		if ( wasMissing || forceRespawn )
		{
			o = GetGame().CreateObjectEx( entry.ClassName, pos, ECE_PLACE_ON_SURFACE );
		}
		if ( !o ) return;
		o.SetPosition( pos );
		o.SetOrientation( ori );
		if ( scale > 0 )
			o.SetScale( scale );

		entry.Position    = pos;
		entry.Orientation = ori;
		entry.Scale       = scale;
	}

	// Helper: delete an entry by id (server-side only)
	private void DeleteById( int id )
	{
		JMMapEditorObject entry = FindEntryById( id );
		if ( !entry ) return;
		Object o = FindLiveEntity( entry );
		if ( o )
			GetGame().ObjectDelete( o );
		m_ServerObjects.RemoveItem( entry );
	}

	// -----------------------------------------------------------------------
	//  Server -> Client
	// -----------------------------------------------------------------------

	private void BroadcastList()
	{
		// Push the updated list to every connected admin client.
		// Client-side handling checks permissions so non-admins ignore.
		SendListTo( NULL );
	}

	private void SendListTo( PlayerIdentity sender )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( m_ServerObjects.Count() );
		foreach ( JMMapEditorObject entry : m_ServerObjects )
		{
			rpc.Write( entry.Id );
			rpc.Write( entry.ClassName );
			rpc.Write( entry.Position );
			rpc.Write( entry.Orientation );
			rpc.Write( entry.Scale );
		}
		rpc.Send( NULL, JMMapEditorModuleRPC.List, true, sender );
	}

	private void RPC_List( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
			return;

		int count;
		if ( !ctx.Read( count ) )
			return;

		m_ClientObjects.Clear();
		for ( int i = 0; i < count; i++ )
		{
			int id;
			string className;
			vector pos;
			vector ori;
			float scale;
			int netLow, netHigh;

			if ( !ctx.Read( id ) )        break;
			if ( !ctx.Read( className ) ) break;
			if ( !ctx.Read( pos ) )       break;
			if ( !ctx.Read( ori ) )       break;
			if ( !ctx.Read( scale ) )     break;

			JMMapEditorObject entry = new JMMapEditorObject();
			entry.Set( className, pos, ori, scale, id );
			m_ClientObjects.Insert( entry );
		}

		JMMapEditorForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.OnServerListUpdated( m_ClientObjects );
	}

	// -----------------------------------------------------------------------
	//  Public client-facing helpers (for the form)
	// -----------------------------------------------------------------------

	array< ref JMMapEditorObject > GetClientObjects()
	{
		return m_ClientObjects;
	}

	// -----------------------------------------------------------------------
	//  Internals
	// -----------------------------------------------------------------------

	private JMMapEditorObject FindEntryById( int id )
	{
		foreach ( JMMapEditorObject entry : m_ServerObjects )
		{
			if ( entry.Id == id )
				return entry;
		}
		return NULL;
	}

	// Best-effort: scan for a live entity whose recorded params match.
	// DayZ has no robust server-side entity lookup by classname + position at small radii,
	// so we walk the tracked list and re-resolve by class. If the engine despawned
	// the entity (network evict, lifetime expiry), the caller falls back to re-spawn.
	private Object FindLiveEntity( JMMapEditorObject entry )
	{
		// Approximate match by class + position within 1.0m
		array< Object > objs = new array< Object >;
		array< CargoBase > prox = new array< CargoBase >;
		GetGame().GetObjectsAtPosition3D( entry.Position, 1.0, objs, prox );

		foreach ( Object o : objs )
		{
			if ( !o )
				continue;
			if ( o.GetType() == entry.ClassName )
				return o;
		}
		return NULL;
	}
}