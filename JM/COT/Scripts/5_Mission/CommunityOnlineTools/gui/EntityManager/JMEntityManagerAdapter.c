// =============================================================================
//  JMEntityManagerAdapter
//
//  Abstract adapter that a concrete manager (vehicles, garage, territories,
//  events) implements to supply data and handlers to the generic
//  JMEntityManagerForm / JMEntityManagerModule pair.
//
//  Lifecycle: the Module owns one adapter. Server-side it collects entities
//  and serializes them. Client-side it deserializes incoming data and renders.
// =============================================================================

//! How to refresh the client after an action. Set inside HandleServerAction
//! via ReportUpsert / ReportRemove. Default (NONE) means a full resend.
enum JMEntityRefreshScope
{
	NONE           = 0,
	SINGLE_UPSERT  = 1,
	SINGLE_REMOVE  = 2,
}

class JMEntityManagerAdapter
{
	protected int m_LastRefreshScope;

	// Metadata cache - populated by CollectEntities, invalidated on action or TTL.
	protected ref array<ref JMEntityMetaData> m_CachedEntities;
	protected float m_CacheTimestamp;
	protected bool  m_CacheDirty;

	// Set by JMEntityManagerModule so adapters can log and fire webhooks.
	protected JMModuleBase m_OwnerModule;

	// How long (seconds) before the cache is considered stale and rebuilt automatically.
	// Set to 0 to disable TTL (only invalidate on explicit action/Refresh).
	static const float CACHE_TTL = 60.0;

	void JMEntityManagerAdapter()
	{
		m_CachedEntities = new array<ref JMEntityMetaData>;
		m_CacheTimestamp = 0;
		m_CacheDirty     = true;
	}

	// Call this instead of CollectEntities directly. Returns cached data when
	// still fresh; rebuilds from CollectEntities otherwise.
	void GetCachedEntities( out array<ref JMEntityMetaData> outEntities )
	{
		float now = GetGame().GetTime() * 0.001;
		bool expired = ( CACHE_TTL > 0 && ( now - m_CacheTimestamp ) >= CACHE_TTL );

		if ( m_CacheDirty || expired )
		{
			m_CachedEntities.Clear();
			CollectEntities( m_CachedEntities );
			m_CacheTimestamp = now;
			m_CacheDirty     = false;
		}

		foreach ( JMEntityMetaData e: m_CachedEntities )
			outEntities.Insert( e );
	}

	void InvalidateCache()
	{
		m_CacheDirty = true;
	}

	// ---------------------------------------------------------------------------
	//  Identity / registration
	// ---------------------------------------------------------------------------
	string GetPermissionPrefix()   { return "EntityManager"; }     // e.g. "Vehicles", "Garage"
	string GetTitle()              { return "Entity Manager"; }
	string GetIcon()               { return ""; }
	string GetCategory()           { return "Items"; }
	string GetWebhookTitle()       { return GetTitle(); }

	void GetWebhookTypes( out array<string> types )
	{
	}

	// ---------------------------------------------------------------------------
	//  Data collection - SERVER SIDE
	//  Called when a client requests a refresh. Implementations should populate
	//  `out` with JMEntityMetaData instances describing every visible entity.
	// ---------------------------------------------------------------------------
	void CollectEntities( out array<ref JMEntityMetaData> outEntities )
	{
	}

	// ---------------------------------------------------------------------------
	//  Filters / categorization
	// ---------------------------------------------------------------------------
	// Optional filter tabs shown above the list. Return empty array for no filters.
	void GetFilterLabels( out array<string> labels )
	{
	}

	// Called when UI filters - return true to keep this entity for category idx.
	// idx 0 = "All", idx n = filter labels[n-1].
	bool MatchesFilter( JMEntityMetaData entity, int filterIndex )
	{
		return true;
	}

	// ---------------------------------------------------------------------------
	//  Action catalog
	//
	//  Per-entity actions appear in the right-hand options panel when an
	//  entity is selected. Bulk actions (m_IsBulk=true) appear in the
	//  left-hand list header.
	// ---------------------------------------------------------------------------
	void GetActions( out array<ref JMEntityAction> outActions )
	{
	}

	// ---------------------------------------------------------------------------
	//  Action dispatch - called on CLIENT, should fire an RPC to server.
	//  `entity` is null for bulk actions.
	// ---------------------------------------------------------------------------
	void OnAction( string actionId, JMEntityMetaData entity )
	{
	}

	// ---------------------------------------------------------------------------
	//  Server-side action handler - called by the module RPC after permission
	//  check. Default is no-op. entity may be null for bulk actions.
	// ---------------------------------------------------------------------------
	void HandleServerAction( string actionId, JMEntityMetaData entity, PlayerIdentity sender )
	{
	}

	// ---------------------------------------------------------------------------
	//  Info panel - extra rows beyond the built-in ones (name, class, status,
	//  position, type). Default implementation renders m_Extra as-is.
	// ---------------------------------------------------------------------------
	void GetInfoRows( JMEntityMetaData entity, out array<string> keys, out map<string, string> values )
	{
		keys = entity.m_ExtraKeys;
		values = entity.m_Extra;
	}

	// ---------------------------------------------------------------------------
	//  Info panel - optional custom widgets appended after extra rows.
	//  Adapter can build a DataTable, scroller, etc. inside `infoContent`,
	//  and should call `form.TrackDynamicWidget(w)` for any widget it creates
	//  so the form can tear them down on entity switch.
	// ---------------------------------------------------------------------------
	void OnBuildInfoExtras( Widget infoContent, JMEntityMetaData entity, JMEntityManagerForm form )
	{
	}

	// ---------------------------------------------------------------------------
	//  Delta-refresh helpers - call from inside HandleServerAction to tell the
	//  module it only needs to upsert or remove one entity (skipping the
	//  full CollectEntities + resend path).
	// ---------------------------------------------------------------------------
	void ReportUpsert()
	{
		m_LastRefreshScope = JMEntityRefreshScope.SINGLE_UPSERT;
	}

	void ReportRemove()
	{
		m_LastRefreshScope = JMEntityRefreshScope.SINGLE_REMOVE;
	}

	int GetLastRefreshScope()
	{
		return m_LastRefreshScope;
	}

	void ResetLastRefreshScope()
	{
		m_LastRefreshScope = JMEntityRefreshScope.NONE;
	}

	// ---------------------------------------------------------------------------
	//  Build fresh metadata for ONE entity id. Used by the delta upsert path
	//  after an action mutates that entity. Default: walk CollectEntities and
	//  pick the matching one (cheap for small adapters). Subclasses should
	//  override with a direct lookup when CollectEntities is expensive.
	// ---------------------------------------------------------------------------
	JMEntityMetaData BuildMetaForId( string id )
	{
		array<ref JMEntityMetaData> all = new array<ref JMEntityMetaData>;
		CollectEntities( all );
		foreach ( JMEntityMetaData m : all )
		{
			if ( m && m.m_Id == id )
				return m;
		}
		return null;
	}

	// ---------------------------------------------------------------------------
	//  Webhook / log forwarding - called from HandleServerAction implementations.
	//  Requires the owning module to call SetOwnerModule(this) after construction.
	// ---------------------------------------------------------------------------
	void SetOwnerModule( JMModuleBase mod )
	{
		m_OwnerModule = mod;
	}

	protected void SendAdapterWebhookColored( string type, JMPlayerInstance inst, string msg, int color )
	{
		if ( m_OwnerModule )
			m_OwnerModule.SendWebhookColored( type, inst, msg, color );
	}

	protected void LogAction( PlayerIdentity sender, string msg )
	{
		GetCommunityOnlineToolsBase().Log( sender, msg );
	}
}
