//! Single entry point for permission work (Register in DeclarePermissions() only; Has,
//! HasAny/HasAll; HasRPC = server RPC gate whose refusals go to anti-cheat). Has(node)
//! on server is never true. Use JMConstants.PERM_*, not literals (a typo silently
//! resolves to the nearest parent). NULL manager/sender is a caller bug, fails loudly.
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
