//! The one entry point for permission work. Module code declares, checks and
//! binds through this class instead of reaching for GetPermissionsManager(),
//! so there is a single spelling to learn, a single place a mod can wrap, and
//! no NULL-manager case for callers to think about.
//!
//!   Register( node )                  declare a node (DeclarePermissions() only)
//!   Has( node )                       client: may I?  server: never true, pass the sender
//!   Has( node, sender )               may this player?
//!   HasRPC( node, sender )            server-side gate for an incoming RPC. A refusal is
//!                                     reported to anti-cheat, because a legitimate client
//!                                     never sends what its own UI does not offer it.
//!   HasAny / HasAll( nodes, sender )  several nodes at once
//!
//! Pass the node as a JMConstants.PERM_* constant, never as a literal: a typo in a
//! literal is not an error, it silently resolves to the nearest registered parent.
class JMPermissions
{
	//! An empty list is not "all of nothing": it answers false, so a caller that
	//! built its list from data cannot be granted access by that data being empty.
	static bool HasAll( array< string > permissions, PlayerIdentity ident = null )
	{
		if ( !permissions || permissions.Count() == 0 )
			return false;

		for ( int i = 0; i < permissions.Count(); i++ )
		{
			if ( !Has( permissions[i], ident ) )
				return false;
		}

		return true;
	}

	static bool HasAny( array< string > permissions, PlayerIdentity ident = null )
	{
		if ( !permissions )
			return false;

		for ( int i = 0; i < permissions.Count(); i++ )
		{
			if ( Has( permissions[i], ident ) )
				return true;
		}

		return false;
	}

	static bool HasRPC( string permission, PlayerIdentity ident = null )
	{
		if ( !GetPermissionsManager() )
			return false;

		//! A dedicated server has no local player: with no sender there is nobody to check.
		if ( !ident && g_Game.IsDedicatedServer() )
			return false;

		return GetPermissionsManager().HasPermissionRPC( permission, ident );
	}

	static bool HasRPC( string permission, PlayerIdentity ident, out JMPlayerInstance instance )
	{
		if ( !GetPermissionsManager() )
			return false;

		//! A dedicated server has no local player: with no sender there is nobody to check.
		if ( !ident && g_Game.IsDedicatedServer() )
			return false;

		return GetPermissionsManager().HasPermissionRPC( permission, ident, instance );
	}

	static bool Register( string permission )
	{
		if ( !GetPermissionsManager() )
			return false;

		GetPermissionsManager().RegisterPermission( permission );
		return true;
	}

	static bool Has( string permission, PlayerIdentity ident = null )
	{
		if ( !GetPermissionsManager() )
			return false;

		//! A dedicated server has no local player: with no sender there is nobody to check.
		if ( !ident && g_Game.IsDedicatedServer() )
			return false;

		return GetPermissionsManager().HasPermission( permission, ident );
	}

	//! Also hands back the checked player's instance, so a handler that needs the
	//! admin for a webhook or a log line does not look them up a second time.
	static bool Has( string permission, PlayerIdentity ident, out JMPlayerInstance instance )
	{
		if ( !GetPermissionsManager() )
			return false;

		//! A dedicated server has no local player: with no sender there is nobody to check.
		if ( !ident && g_Game.IsDedicatedServer() )
			return false;

		return GetPermissionsManager().HasPermission( permission, ident, instance );
	}

	//! Client-side form that only wants the local player's instance back.
	static bool Has( string permission, out JMPlayerInstance instance )
	{
		if ( !GetPermissionsManager() )
			return false;

		return GetPermissionsManager().HasPermission( permission, instance );
	}
}
