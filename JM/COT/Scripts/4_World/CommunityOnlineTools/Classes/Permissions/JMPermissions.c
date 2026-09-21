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
//!
//! @note if GetPermissionsManager() is NULL in the Has/HasRPC methods,
//! or ident is NULL in the HasRPC method it's a programming error on the caller's side,
//! not something that should be caught and silently handled here.
//! It will fail loudly (exception log) and is intended because that's on the author
//! of the respective mod code (ident aka sender identity in an RPC received on the server is NEVER null)
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
		return GetPermissionsManager().HasPermissionRPC( permission, ident );
	}

	static bool HasRPC( string permission, PlayerIdentity ident, out JMPlayerInstance instance )
	{
		return GetPermissionsManager().HasPermissionRPC( permission, ident, instance );
	}

	static bool Register( string permission )
	{
		GetPermissionsManager().RegisterPermission( permission );
		return true;
	}

	static bool Has( string permission, PlayerIdentity ident = null )
	{
		return GetPermissionsManager().HasPermission( permission, ident );
	}

	//! Also hands back the checked player's instance, so a handler that needs the
	//! admin for a webhook or a log line does not look them up a second time.
	static bool Has( string permission, PlayerIdentity ident, out JMPlayerInstance instance )
	{
		return GetPermissionsManager().HasPermission( permission, ident, instance );
	}

	//! Client-side form that only wants the local player's instance back.
	static bool Has( string permission, out JMPlayerInstance instance )
	{
		return GetPermissionsManager().HasPermission( permission, instance );
	}
}
