// =============================================================================
//  JMTeleportHistory
//
//  Where things were before the admin moved them, so a teleport can be taken
//  back.
//
//  Client-side and per-session on purpose. This is a record of what THIS admin
//  moved, which is exactly what an undo is, and keeping it on the client costs
//  no netcode, no serialisation and no per-tick bandwidth in the player list.
//  It is NOT a server audit trail: teleports another admin issued, or ones from
//  before this client connected, are not in here and are not undoable.
//
//  Every target is keyed by a string - a player GUID, or an object's network id
//  - so players, vehicles and anything ESP tracks share one store and one cap.
//
//  Usage:
//      // before moving something:
//      JMTeleportHistory.PushPlayers( guids );
//      JMTeleportHistory.PushObject( vehicleObject );
//
//      // undoing, most recent first - and putting it back:
//      JMTeleportHistory.UndoTarget( object, 0 );
//      JMTeleportHistory.RedoTarget( object, 0 );
// =============================================================================
class JMTeleportHistory
{
	//! How far back an undo can walk. Five is the ask, and it is also about as
	//! many positions as anyone can still tell apart by their coordinates.
	static const int MAX_ENTRIES = 5;

	//! Two teleports landing this close together are the same move. Squared
	//! metres, so 4 is two metres.
	//!
	//! Holding the cursor-teleport key fires once per frame, and without this
	//! the whole history fills with one spot and buries the position that was
	//! actually worth going back to.
	static const float SAME_PLACE_SQ = 4.0;
	protected static ref map<string, ref array<ref JMTeleportHistoryEntry>> s_Targets;

	//! Where an undone step was going, so it can be put back.
	//!
	//! Same shape and same keys as s_Targets, and the two behave as one control:
	//! undoing moves a step from the history onto here, redoing moves it back,
	//! and a NEW teleport throws this away - the future the admin had undone is
	//! not reachable once they have gone somewhere else instead, which is how
	//! every other undo/redo pair in software behaves.
	protected static ref map<string, ref array<ref JMTeleportHistoryEntry>> s_Redo;

	//! Set while an undo is being applied.
	//!
	//! An undo goes out through the same module call every other teleport uses,
	//! and that call is where the history is written - without this it would
	//! record the position it is undoing FROM and the stack could never get
	//! shorter.
	protected static bool s_Applying;

	static bool HasRedo( string key )
	{
		return RedoCount( key ) > 0;
	}

	protected static map<string, ref array<ref JMTeleportHistoryEntry>> Targets()
	{
		if ( !s_Targets )
			s_Targets = new map<string, ref array<ref JMTeleportHistoryEntry>>;

		return s_Targets;
	}

	protected static map<string, ref array<ref JMTeleportHistoryEntry>> Redos()
	{
		if ( !s_Redo )
			s_Redo = new map<string, ref array<ref JMTeleportHistoryEntry>>;

		return s_Redo;
	}

	//! The list `key` keeps in `store`, created on first use.
	protected static array<ref JMTeleportHistoryEntry> ListFor( map<string, ref array<ref JMTeleportHistoryEntry>> store, string key )
	{
		array<ref JMTeleportHistoryEntry> entries = store.Get( key );

		if ( !entries )
		{
			entries = new array<ref JMTeleportHistoryEntry>;
			store.Insert( key, entries );
		}

		return entries;
	}

	//! Move one position onto the front of a stack, trimming the far end.
	protected static void PushOnto( map<string, ref array<ref JMTeleportHistoryEntry>> store, string key, vector position )
	{
		if ( key == "" || position == vector.Zero )
			return;

		array<ref JMTeleportHistoryEntry> entries = ListFor( store, key );

		entries.InsertAt( new JMTeleportHistoryEntry( position ), 0 );

		while ( entries.Count() > MAX_ENTRIES )
			entries.Remove( entries.Count() - 1 );
	}

	// =========================================================================
	//  Keys
	// =========================================================================

	static string PlayerKey( string guid )
	{
		if ( guid == "" )
			return "";

		return "p:" + guid;
	}

	static string ObjectKey( int networkLow, int networkHigh )
	{
		return "o:" + networkLow + ":" + networkHigh;
	}

	//! An object's key from the object itself, for call sites that hold one
	//! rather than a pair of network ids.
	static string ObjectKeyOf( Object object )
	{
		if ( !object )
			return "";

		int networkLow;
		int networkHigh;
		object.GetNetworkID( networkLow, networkHigh );

		return ObjectKey( networkLow, networkHigh );
	}

	//! The player `object` IS, as far as this client can name one.
	//!
	//! This is what makes the history SHARED rather than one stack per window.
	//! JMESPMeta fills its own `player` from the target's identity, and a client
	//! only has identities for players it can see - so a tracked player often
	//! arrives with it null, the ESP menu would key that player by network id,
	//! and a teleport issued from the player manager would be filed under a GUID
	//! the ESP menu never looks up.
	//!
	//! The roster the permissions manager keeps is populated for every player on
	//! the server, so matching the object against it names the same player from
	//! either window.
	static JMPlayerInstance PlayerOf( Object object )
	{
		if ( !object || !object.IsMan() )
			return NULL;

		JMPermissionManager permissions = GetPermissionsManager();

		if ( !permissions || !permissions.Players )
			return NULL;

		//! Exact where it is available: a client holds identities for the
		//! players it can see, which is most of the ones it can act on.
		Man man;

		if ( Class.CastTo( man, object ) && man.GetIdentity() )
		{
			JMPlayerInstance byIdentity = permissions.GetPlayer( man.GetIdentity().GetId() );

			if ( byIdentity )
				return byIdentity;
		}

		//! Otherwise match the object against the roster. The roster carries
		//! every player on the server and fills PlayerObject for the ones this
		//! client has loaded, so a tracked player without a local identity is
		//! still named the same here as in the player manager.
		for ( int i = 0; i < permissions.Players.Count(); i++ )
		{
			JMPlayerInstance instance = permissions.Players.GetElement( i );

			if ( !instance )
				continue;

			Object tracked = instance.PlayerObject;

			if ( tracked == object )
				return instance;
		}

		return NULL;
	}

	static string GUIDOf( Object object )
	{
		JMPlayerInstance instance = PlayerOf( object );

		if ( !instance )
			return "";

		return instance.GetGUID();
	}

	//! The one key every caller resolves an object through: a player is filed
	//! under its GUID no matter which window is asking, everything else under
	//! its network id.
	static string KeyFor( Object object )
	{
		string guid = GUIDOf( object );

		if ( guid != "" )
			return PlayerKey( guid );

		return ObjectKeyOf( object );
	}

	//! The GUID of whoever is playing on this client, or "" off a client.
	static string SelfGUID()
	{
		JMPlayerInstance self = GetPermissionsManager().GetClientPlayer();

		if ( !self )
			return "";

		return self.GetGUID();
	}

	// =========================================================================
	//  Recording
	// =========================================================================

	//! Record where `key` is being moved FROM.
	//!
	//! Call BEFORE the teleport, never after: the whole value of the entry is
	//! the position that is about to stop being true.
	static void Push( string key, vector position )
	{
		//! A dedicated server has no admin UI to undo from, and every call site
		//! below is in code that also runs there.
		if ( g_Game.IsDedicatedServer() )
			return;

		if ( s_Applying )
			return;

		if ( key == "" || position == vector.Zero )
			return;

		array<ref JMTeleportHistoryEntry> entries = ListFor( Targets(), key );

		if ( entries.Count() > 0 && vector.DistanceSq( entries[0].Position, position ) < SAME_PLACE_SQ )
			return;

		//! A move that is not an undo ends the branch the redo stack was holding
		//! open. Going somewhere new is a decision; the steps that were undone
		//! to get here are not somewhere to go forward to any more.
		Redos().Remove( key );

		entries.InsertAt( new JMTeleportHistoryEntry( position ), 0 );

		while ( entries.Count() > MAX_ENTRIES )
			entries.Remove( entries.Count() - 1 );
	}

	//! Where a player is right now, as far as this client knows.
	static vector PlayerPosition( string guid )
	{
		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( guid );

		if ( !instance )
			return vector.Zero;

		//! The live object beats the synced field: an instance's position only
		//! refreshes on the player list's own tick, so two teleports in quick
		//! succession would otherwise both record the position from before the
		//! first one.
		if ( instance.PlayerObject )
			return instance.PlayerObject.GetPosition();

		return instance.GetPosition();
	}

	static void PushPlayer( string guid )
	{
		Push( PlayerKey( guid ), PlayerPosition( guid ) );
	}

	static void PushPlayers( array<string> guids )
	{
		if ( !guids )
			return;

		for ( int i = 0; i < guids.Count(); i++ )
			PushPlayer( guids[i] );
	}

	//! The admin themselves - for the teleports that move the sender rather
	//! than a target.
	static void PushSelf()
	{
		PushPlayer( SelfGUID() );
	}

	//! Record an object under whichever key it belongs to - see KeyFor.
	static void PushObject( Object object )
	{
		if ( !object )
			return;

		Push( KeyFor( object ), object.GetPosition() );
	}

	// =========================================================================
	//  Reading
	// =========================================================================

	//! Most recent first. NULL where nothing has been recorded for `key`.
	static array<ref JMTeleportHistoryEntry> Entries( string key )
	{
		if ( key == "" )
			return NULL;

		return Targets().Get( key );
	}

	static int Count( string key )
	{
		array<ref JMTeleportHistoryEntry> entries = Entries( key );

		if ( !entries )
			return 0;

		return entries.Count();
	}

	static bool Has( string key )
	{
		return Count( key ) > 0;
	}

	//! Read one entry without consuming it. NULL when there is nothing that far
	//! back.
	static JMTeleportHistoryEntry EntryAt( string key, int index )
	{
		array<ref JMTeleportHistoryEntry> entries = Entries( key );

		if ( !entries || index < 0 || index >= entries.Count() )
			return NULL;

		return entries[index];
	}

	// =========================================================================
	//  Undoing
	// =========================================================================

	//! Take everything down to and including `index` off the stack and answer
	//! where it says to go. vector.Zero when there is nothing that far back.
	//!
	//! The skipped entries are DISCARDED, not kept: they are places the target
	//! only passed through, and leaving them in would make the next undo walk
	//! forwards again.
	static vector Pop( string key, int index, vector current = vector.Zero )
	{
		return Transfer( Targets(), Redos(), key, index, current );
	}

	//! The mirror of Pop: take `index` off the redo stack, hand the steps it
	//! passed back to the history, and answer where to go.
	static vector PopRedo( string key, int index, vector current = vector.Zero )
	{
		return Transfer( Redos(), Targets(), key, index, current );
	}

	//! Walk `from` back `index` + 1 steps, parking everything the move skipped
	//! on `to` so the opposite button can retrace it exactly.
	//!
	//! `current` is where the target is right NOW - the one position neither
	//! stack holds, because a stack only ever records places something has
	//! LEFT. It becomes the far end of the other stack, so the first press of
	//! the opposite button comes straight back here.
	//!
	//! The order matters: pushing current first and then the skipped steps in
	//! the order they were passed leaves the nearest one on top, so undoing
	//! three steps and pressing redo three times retraces them one at a time
	//! rather than jumping the whole way in one go.
	protected static vector Transfer( map<string, ref array<ref JMTeleportHistoryEntry>> from, map<string, ref array<ref JMTeleportHistoryEntry>> to, string key, int index, vector current )
	{
		array<ref JMTeleportHistoryEntry> entries = from.Get( key );

		if ( !entries || index < 0 || index >= entries.Count() )
			return vector.Zero;

		vector position = entries[index].Position;

		PushOnto( to, key, current );

		for ( int i = 0; i < index; i++ )
			PushOnto( to, key, entries[i].Position );

		for ( int j = index; j >= 0; j-- )
			entries.Remove( j );

		return position;
	}

	//! The steps an undo has parked, next one forward first. NULL when there is
	//! nothing to redo for `key`.
	static array<ref JMTeleportHistoryEntry> RedoEntries( string key )
	{
		if ( key == "" )
			return NULL;

		return Redos().Get( key );
	}

	static int RedoCount( string key )
	{
		array<ref JMTeleportHistoryEntry> entries = RedoEntries( key );

		if ( !entries )
			return 0;

		return entries.Count();
	}

	static JMTeleportHistoryEntry RedoEntryAt( string key, int index )
	{
		array<ref JMTeleportHistoryEntry> entries = RedoEntries( key );

		if ( !entries || index < 0 || index >= entries.Count() )
			return NULL;

		return entries[index];
	}

	static void Clear( string key )
	{
		if ( key == "" )
			return;

		Targets().Remove( key );
		Redos().Remove( key );
	}

	//! Send a player back to where they were `index` + 1 teleports ago.
	//!
	//! Goes out through JMPlayerModule like every other teleport of a player,
	//! so an undo is logged, webhooked and permissioned exactly as the move it
	//! is taking back was.
	static bool UndoPlayer( string guid, int index )
	{
		if ( guid == "" )
			return false;

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION ) )
			return false;

		JMPlayerModule playerModule;
		if ( !Class.CastTo( playerModule, GetModuleManager().GetModule( JMPlayerModule ) ) )
			return false;

		vector position = Pop( PlayerKey( guid ), index, PlayerPosition( guid ) );

		if ( position == vector.Zero )
			return false;

		s_Applying = true;
		playerModule.TeleportTo( position, { guid } );
		s_Applying = false;

		return true;
	}

	//! Put a player back where an undo took them from.
	//!
	//! Not simply "teleport forwards": redo only ever reaches a position an undo
	//! parked, so it cannot invent a move the admin never made.
	static bool RedoPlayer( string guid, int index )
	{
		if ( guid == "" )
			return false;

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION ) )
			return false;

		JMPlayerModule playerModule;
		if ( !Class.CastTo( playerModule, GetModuleManager().GetModule( JMPlayerModule ) ) )
			return false;

		vector position = PopRedo( PlayerKey( guid ), index, PlayerPosition( guid ) );

		if ( position == vector.Zero )
			return false;

		s_Applying = true;
		playerModule.TeleportTo( position, { guid } );
		s_Applying = false;

		return true;
	}

	//! Undo one step for whatever `object` is, routed by WHAT it is rather than
	//! by which window asked.
	//!
	//! Every surface that can undo an object calls this, so an undo means the
	//! same thing and spends the same step in all of them.
	static bool UndoTarget( Object object, int index )
	{
		string guid = GUIDOf( object );

		if ( guid != "" )
			return UndoPlayer( guid, index );

		return UndoObject( object, index );
	}

	//! Redo one step for whatever `object` is, routed the same way UndoTarget
	//! routes - so the pair always act on the same stack from every window.
	static bool RedoTarget( Object object, int index )
	{
		string guid = GUIDOf( object );

		if ( guid != "" )
			return RedoPlayer( guid, index );

		return RedoObject( object, index );
	}

	//! A non-player target goes back the way it came: an ESP position set,
	//! permissioned exactly as the move that put it where it is.
	static bool UndoObject( Object object, int index )
	{
		if ( !object )
			return false;

		if ( !JMPermissions.Has( JMConstants.PERM_ESP_OBJECT_SETPOSITION ) )
			return false;

		JMESPModule espModule;
		if ( !Class.CastTo( espModule, GetModuleManager().GetModule( JMESPModule ) ) )
			return false;

		vector position = Pop( ObjectKeyOf( object ), index, object.GetPosition() );

		if ( position == vector.Zero )
			return false;

		s_Applying = true;
		espModule.SetPosition( position, object );
		s_Applying = false;

		return true;
	}

	static bool RedoObject( Object object, int index )
	{
		if ( !object )
			return false;

		if ( !JMPermissions.Has( JMConstants.PERM_ESP_OBJECT_SETPOSITION ) )
			return false;

		JMESPModule espModule;
		if ( !Class.CastTo( espModule, GetModuleManager().GetModule( JMESPModule ) ) )
			return false;

		vector position = PopRedo( ObjectKeyOf( object ), index, object.GetPosition() );

		if ( position == vector.Zero )
			return false;

		s_Applying = true;
		espModule.SetPosition( position, object );
		s_Applying = false;

		return true;
	}

	//! Wrap a move that is itself an undo, for the targets that do not go
	//! through JMPlayerModule or JMESPModule - a vehicle named only by its
	//! network id, which the vehicle manager can reach when this client has no
	//! object for it at all.
	//!
	//! Their forward move records the history at its own call site rather than
	//! inside a shared module call, so without this the undo would push the
	//! position it is leaving and the stack would never shrink.
	static void BeginApply()
	{
		s_Applying = true;
	}

	static void EndApply()
	{
		s_Applying = false;
	}
}
