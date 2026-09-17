class JMFilterRegistry
{
	private static ref array< ref JMFilterEntry > s_PlayerFilters = new array< ref JMFilterEntry >;
	private static ref array< ref JMFilterEntry > s_ItemFilters = new array< ref JMFilterEntry >;
	private static ref array< ref JMFilterEntry > s_ObjectFilters = new array< ref JMFilterEntry >;
	private static ref array< ref JMFilterEntry > s_TeleportFilters = new array< ref JMFilterEntry >;
	private static ref array< ref JMFilterEntry > s_ESPFilters = new array< ref JMFilterEntry >;

	static void RegisterPlayerFilter( string id, string label, string icon = "", int color = 0, Class target = null, string callback = "" )
	{
		s_PlayerFilters.Insert( new JMFilterEntry( id, label, icon, color, target, callback ) );
	}

	static void RegisterItemFilter( string id, string label, string icon = "", int color = 0, Class target = null, string callback = "" )
	{
		s_ItemFilters.Insert( new JMFilterEntry( id, label, icon, color, target, callback ) );
	}

	static void RegisterObjectFilter( string id, string label, string icon = "", int color = 0, Class target = null, string callback = "" )
	{
		s_ObjectFilters.Insert( new JMFilterEntry( id, label, icon, color, target, callback ) );
	}

	static void RegisterTeleportFilter( string id, string label, string icon = "", int color = 0, Class target = null, string callback = "" )
	{
		s_TeleportFilters.Insert( new JMFilterEntry( id, label, icon, color, target, callback ) );
	}

	static void RegisterESPFilter( string id, string label, string icon = "", int color = 0, Class target = null, string callback = "" )
	{
		s_ESPFilters.Insert( new JMFilterEntry( id, label, icon, color, target, callback ) );
	}

	static array< ref JMFilterEntry > GetPlayerFilters()
	{
		return s_PlayerFilters;
	}

	static array< ref JMFilterEntry > GetItemFilters()
	{
		return s_ItemFilters;
	}

	static array< ref JMFilterEntry > GetObjectFilters()
	{
		return s_ObjectFilters;
	}

	static array< ref JMFilterEntry > GetTeleportFilters()
	{
		return s_TeleportFilters;
	}

	static array< ref JMFilterEntry > GetESPFilters()
	{
		return s_ESPFilters;
	}
}
