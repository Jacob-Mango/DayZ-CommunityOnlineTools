// =============================================================================
//  JMEntityManagerModule
//
//  Generic base module for any "list + map + act on entities" admin UI.
//  Subclass to provide an adapter (GetAdapter) and an RPC range
//  (GetRPCRequest / GetRPCSend / GetRPCAction). Everything else - permission
//  registration, refresh dispatch, action RPC - is done here.
//
//  Subclasses override:
//    - CreateAdapter()                     create the adapter (one-time)
//    - GetRPCRequest/Send/Action()         ints from the module's own RPC enum
//    - GetLayoutRoot/Title/IconName/etc.   standard JMRenderableModuleBase
//
//  The shared layout and form come from:
//    "JM/COT/GUI/layouts/entitymanager/EntityManager_Menu.layout"
// =============================================================================
class JMEntityManagerModule: JMRenderableModuleBase
{
	protected ref JMEntityManagerAdapter m_Adapter;
	protected ref array<ref JMEntityMetaData> m_Entities;
	protected ref array<string> m_Permissions;

	void JMEntityManagerModule()
	{
		m_Entities    = new array<ref JMEntityMetaData>;
		m_Permissions = new array<string>;

		m_Adapter = CreateAdapter();
		if ( m_Adapter )
		{
			m_Adapter.SetOwnerModule( this );
			RegisterPermissions();
		}
	}

	// -- Subclass hooks -----------------------------------------------------------

	JMEntityManagerAdapter CreateAdapter()
	{
		return null;
	}

	int GetRPCRequest()
	{
		return -1;
	}

	int GetRPCSend()
	{
		return -1;
	}

	int GetRPCAction()
	{
		return -1;
	}

	// Delta slots. Default to -1 when a concrete module doesn't expose them;
	// in that case the module falls back to full refreshes (old behaviour).
	int GetRPCSendUpsert()
	{
		return -1;
	}

	int GetRPCSendRemove()
	{
		return -1;
	}

	override int GetRPCMin()
	{
		return GetRPCRequest() - 1;  // INVALID slot sits just before Request
	}

	override int GetRPCMax()
	{
		// If the subclass exposed delta slots they sit just after Action; the
		// range has to cover them.  +3 leaves room for Action -> SendUpsert
		// -> SendRemove -> COUNT.
		if ( GetRPCSendRemove() > 0 )
			return GetRPCSendRemove() + 1;
		return GetRPCAction() + 1;
	}

	// Default view permission is {Prefix}.View. Add more via RegisterExtraPermissions.
	protected void RegisterPermissions()
	{
		string prefix = m_Adapter.GetPermissionPrefix();

		GetPermissionsManager().RegisterPermission( prefix + ".View" );
		m_Permissions.Insert( prefix + ".View" );

		array<ref JMEntityAction> actions = new array<ref JMEntityAction>;
		m_Adapter.GetActions( actions );
		foreach ( JMEntityAction a: actions )
		{
			if ( a.m_Permission == "" )
				continue;
			if ( m_Permissions.Find( a.m_Permission ) != -1 )
				continue;
			GetPermissionsManager().RegisterPermission( a.m_Permission );
			m_Permissions.Insert( a.m_Permission );
		}
	}

	JMEntityManagerAdapter GetAdapter()
	{
		return m_Adapter;
	}

	array<ref JMEntityMetaData> GetEntities()
	{
		return m_Entities;
	}

	// -- JMRenderableModuleBase ---------------------------------------------------

	override bool HasAccess()
	{
		if ( !m_Adapter )
			return false;
		return GetPermissionsManager().HasPermission( m_Adapter.GetPermissionPrefix() + ".View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/entitymanager/EntityManager_Menu.layout";
	}

	override string GetTitle()
	{
		if ( m_Adapter )
			return m_Adapter.GetTitle();
		return "Entities";
	}

	override string GetIconName()
	{
		if ( m_Adapter )
			return m_Adapter.GetIcon();
		return "";
	}

	override string GetCategory()
	{
		if ( m_Adapter )
			return m_Adapter.GetCategory();
		return "Items";
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
		if ( m_Adapter )
			return m_Adapter.GetWebhookTitle();
		return "Entity Manager";
	}

	// -- Client API ---------------------------------------------------------------

	void RequestServerEntities()
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, GetRPCRequest(), true );
	}

	void RequestAction( string actionId, JMEntityMetaData entity )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( actionId );

		bool hasEntity = ( entity != null );
		rpc.Write( hasEntity );
		if ( hasEntity )
			entity.Write( rpc );

		rpc.Send( NULL, GetRPCAction(), true );
	}

	// -- RPC dispatch -------------------------------------------------------------

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		super.OnRPC( sender, target, rpc_type, ctx );

		if ( rpc_type == GetRPCRequest() )         RPC_Request(    ctx, sender, target );
		else if ( rpc_type == GetRPCSend() )       RPC_Send(       ctx, sender, target );
		else if ( rpc_type == GetRPCAction() )     RPC_Action(     ctx, sender, target );
		else if ( rpc_type == GetRPCSendUpsert() ) RPC_SendUpsert( ctx, sender, target );
		else if ( rpc_type == GetRPCSendRemove() ) RPC_SendRemove( ctx, sender, target );
	}

	protected void UpdateEntitiesFromAdapter()
	{
		if ( m_Entities.Count() > 0 )
			m_Entities.Clear();

		if ( m_Adapter )
			m_Adapter.GetCachedEntities( m_Entities );
	}

	protected void RPC_Request( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionHost() )
			return;

		if ( !m_Adapter )
			return;

		if ( !GetPermissionsManager().HasPermission( m_Adapter.GetPermissionPrefix() + ".View", sender ) )
			return;

		UpdateEntitiesFromAdapter();

		// SP / listen server - apply directly to client form
		if ( IsMissionHost() && !g_Game.IsDedicatedServer() )
		{
			JMEntityManagerForm form;
			if ( Class.CastTo( form, GetForm() ) )
				form.LoadEntities();
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( m_Entities.Count() );
		foreach ( auto e: m_Entities )
			e.Write( rpc );
		rpc.Send( NULL, GetRPCSend(), true, sender );
	}

	protected void RPC_Send( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionClient() )
			return;

		int count;
		if ( !ctx.Read( count ) )
			return;

		m_Entities.Clear();

		while ( count )
		{
			JMEntityMetaData e = new JMEntityMetaData();
			if ( !e.Read( ctx ) )
				return;
			m_Entities.Insert( e );
			count--;
		}

		JMEntityManagerForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.LoadEntities();
	}

	protected void RPC_Action( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionHost() )
			return;

		if ( !m_Adapter )
			return;

		string actionId;
		if ( !ctx.Read( actionId ) )
			return;

		bool hasEntity;
		if ( !ctx.Read( hasEntity ) )
			return;

		JMEntityMetaData entity;
		if ( hasEntity )
		{
			entity = new JMEntityMetaData();
			if ( !entity.Read( ctx ) )
				return;
		}

		// Per-action permission check
		array<ref JMEntityAction> actions = new array<ref JMEntityAction>;
		m_Adapter.GetActions( actions );
		foreach ( JMEntityAction a: actions )
		{
			if ( a.m_Id != actionId )
				continue;
			if ( a.m_Permission != "" && !GetPermissionsManager().HasPermission( a.m_Permission, sender ) )
				return;
			break;
		}

		m_Adapter.HandleServerAction( actionId, entity, sender );

		// Any action mutates server state - always mark cache dirty so the next
		// UpdateEntitiesFromAdapter (delta or full) rebuilds from scratch.
		m_Adapter.InvalidateCache();

		// Decide how to refresh the requester's view. Adapters may set one of
		// these in HandleServerAction to indicate the delta scope; the default
		// is a full resend (preserves behaviour of any adapter that hasn't
		// been updated yet).
		int scope = m_Adapter.GetLastRefreshScope();
		m_Adapter.ResetLastRefreshScope();

		if ( scope == JMEntityRefreshScope.SINGLE_UPSERT && GetRPCSendUpsert() > 0 && entity )
		{
			JMEntityMetaData fresh = m_Adapter.BuildMetaForId( entity.m_Id );
			if ( fresh )
				SendUpsert( fresh, sender );
			else
				SendRemove( entity.m_Id, entity.m_NetworkIDLow, entity.m_NetworkIDHigh, sender );
		}
		else if ( scope == JMEntityRefreshScope.SINGLE_REMOVE && GetRPCSendRemove() > 0 && entity )
		{
			SendRemove( entity.m_Id, entity.m_NetworkIDLow, entity.m_NetworkIDHigh, sender );
		}
		else
		{
			// Fallback: full resend - bulk actions, unknown scope, or subclass
			// hasn't opted into deltas yet.
			UpdateEntitiesFromAdapter();
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( m_Entities.Count() );
			foreach ( auto e2: m_Entities )
				e2.Write( rpc );
			rpc.Send( NULL, GetRPCSend(), true, sender );
		}
	}

	// --- Delta send helpers (server) -----------------------------------------

	protected void SendUpsert( JMEntityMetaData meta, PlayerIdentity sender )
	{
		if ( !meta )
			return;
		ScriptRPC rpc = new ScriptRPC();
		meta.Write( rpc );
		rpc.Send( NULL, GetRPCSendUpsert(), true, sender );
	}

	protected void SendRemove( string id, int netLow, int netHigh, PlayerIdentity sender )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( id );
		rpc.Write( netLow );
		rpc.Write( netHigh );
		rpc.Send( NULL, GetRPCSendRemove(), true, sender );
	}

	// --- Delta receive handlers (client) -------------------------------------

	protected void RPC_SendUpsert( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionClient() )
			return;

		JMEntityMetaData fresh = new JMEntityMetaData();
		if ( !fresh.Read( ctx ) )
			return;

		// Replace in-place if we already know this id, otherwise append.
		bool replaced = false;
		for ( int i = 0; i < m_Entities.Count(); i++ )
		{
			JMEntityMetaData e = m_Entities[i];
			if ( e && MatchesEntity( e, fresh.m_Id, fresh.m_NetworkIDLow, fresh.m_NetworkIDHigh ) )
			{
				m_Entities.Set( i, fresh );
				replaced = true;
				break;
			}
		}
		if ( !replaced )
			m_Entities.Insert( fresh );

		JMEntityManagerForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.OnDeltaUpsert( fresh );
	}

	protected void RPC_SendRemove( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionClient() )
			return;

		string id;
		int netLow, netHigh;
		if ( !ctx.Read( id ) )     return;
		if ( !ctx.Read( netLow ) ) return;
		if ( !ctx.Read( netHigh ) )return;

		for ( int i = m_Entities.Count() - 1; i >= 0; i-- )
		{
			JMEntityMetaData e = m_Entities[i];
			if ( e && MatchesEntity( e, id, netLow, netHigh ) )
				m_Entities.Remove( i );
		}

		JMEntityManagerForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.OnDeltaRemove( id, netLow, netHigh );
	}

	protected bool MatchesEntity( JMEntityMetaData e, string id, int netLow, int netHigh )
	{
		if ( id != "" && e.m_Id == id )
			return true;
		if ( ( netLow != 0 || netHigh != 0 ) && e.m_NetworkIDLow == netLow && e.m_NetworkIDHigh == netHigh )
			return true;
		return false;
	}
}
