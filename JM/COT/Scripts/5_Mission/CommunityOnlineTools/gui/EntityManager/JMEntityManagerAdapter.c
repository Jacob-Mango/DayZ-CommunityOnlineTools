//  JMEntityManagerAdapter - abstract adapter a concrete manager implements to
//  feed the generic JMEntityManagerForm / Module pair. The Module owns one:
//  server-side collects and serializes entities, client-side renders them.

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

	// ---------------------------------------------------------------------------
	//  Action catalog: per-entity actions go in the right options panel, bulk
	//  actions (m_IsBulk=true) in the left list header.
	// ---------------------------------------------------------------------------
	void GetActions( out array<ref JMEntityAction> outActions )
	{
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

	string GetCategory()           { return JMSideBarConfig.CATEGORY_ITEMS; }

	// ---------------------------------------------------------------------------
	//  Filters / categorization
	// ---------------------------------------------------------------------------
	// Optional filter tabs shown above the list. Return empty array for no filters.
	void GetFilterLabels( out array<string> labels )
	{
	}

	string GetIcon()               { return ""; }

	// ---------------------------------------------------------------------------
	//  Info panel - extra rows beyond the built-in ones (name, class, status,
	//  position, type). Default implementation renders m_Extra as-is.
	// ---------------------------------------------------------------------------
	void GetInfoRows( JMEntityMetaData entity, out array<string> keys, out map<string, string> values )
	{
		keys = entity.m_ExtraKeys;
		values = entity.m_Extra;
	}

	int GetLastRefreshScope()
	{
		return m_LastRefreshScope;
	}

	// ---------------------------------------------------------------------------
	//  Identity / registration
	// ---------------------------------------------------------------------------
	string GetPermissionPrefix()   { return "EntityManager"; }     // e.g. "Vehicles", "Garage"

	string GetTitle()              { return "Entity Manager"; }

	string GetWebhookTitle()       { return GetTitle(); }

	void GetWebhookTypes( out array<string> types )
	{
	}

	// ---------------------------------------------------------------------------
	//  Webhook / log forwarding - called from HandleServerAction implementations.
	//  Requires the owning module to call SetOwnerModule(this) after construction.
	// ---------------------------------------------------------------------------
	void SetOwnerModule( JMModuleBase mod )
	{
		m_OwnerModule = mod;
	}

	void InvalidateCache()
	{
		m_CacheDirty = true;
	}

	// ---------------------------------------------------------------------------
	//  Data collection - SERVER SIDE, on client refresh request. Populate `out`
	//  with JMEntityMetaData for every visible entity.
	// ---------------------------------------------------------------------------
	void CollectEntities( out array<ref JMEntityMetaData> outEntities )
	{
	}

	// Called when UI filters - return true to keep this entity for category idx.
	// idx 0 = "All", idx n = filter labels[n-1].
	bool MatchesFilter( JMEntityMetaData entity, int filterIndex )
	{
		return true;
	}

	// ---------------------------------------------------------------------------
	//  Action dispatch - called on CLIENT, should fire an RPC to server.
	//  `entity` is null for bulk actions.
	// ---------------------------------------------------------------------------
	void OnAction( string actionId, JMEntityMetaData entity )
	{
	}

	// ---------------------------------------------------------------------------
	//  Server-side action handler, called by the module RPC after the permission
	//  check. Default no-op; entity may be null for bulk actions.
	void HandleServerAction( string actionId, JMEntityMetaData entity, PlayerIdentity sender )
	{
	}

	// ---------------------------------------------------------------------------
	//  Info panel - custom widgets after the extra rows, built in `infoContent`;
	//  call `form.TrackDynamicWidget(w)` for each so entity switches tear them down.
	// ---------------------------------------------------------------------------
	void OnBuildInfoExtras( Widget infoContent, JMEntityMetaData entity, JMEntityManagerForm form )
	{
	}

	// ---------------------------------------------------------------------------
	//  Delta-refresh helpers - call inside HandleServerAction to upsert/remove
	//  one entity instead of the full CollectEntities + resend.
	// ---------------------------------------------------------------------------
	void ReportUpsert()
	{
		m_LastRefreshScope = JMEntityRefreshScope.SINGLE_UPSERT;
	}

	void ReportRemove()
	{
		m_LastRefreshScope = JMEntityRefreshScope.SINGLE_REMOVE;
	}

	void ResetLastRefreshScope()
	{
		m_LastRefreshScope = JMEntityRefreshScope.NONE;
	}

	// ---------------------------------------------------------------------------
	//  Fresh metadata for ONE entity id (delta upsert path). Default walks
	//  CollectEntities; override with a direct lookup when that is expensive.
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
