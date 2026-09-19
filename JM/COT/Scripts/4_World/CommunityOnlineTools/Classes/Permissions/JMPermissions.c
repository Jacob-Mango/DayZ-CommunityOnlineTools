class JMPermissions
{
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

		return GetPermissionsManager().HasPermission( permission, ident );
	}

	static bool HasRPC( string permission, PlayerIdentity ident = null )
	{
		if ( !GetPermissionsManager() )
			return false;

		return GetPermissionsManager().HasPermissionRPC( permission, ident );
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
}
