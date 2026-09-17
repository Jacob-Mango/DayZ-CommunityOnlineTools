class JMRoleManagerModule : JMRenderableModuleBase
{
	void JMRoleManagerModule()
	{
		JMPermissions.Register( JMConstants.PERM_ROLES_VIEW        );
		JMPermissions.Register( JMConstants.PERM_ROLES_CREATE      );
		JMPermissions.Register( JMConstants.PERM_ROLES_DELETE      );
		JMPermissions.Register( JMConstants.PERM_ROLES_PERMISSIONS );
	}

	// -------------------------------------------------------------------------
	//  JMRenderableModuleBase overrides
	// -------------------------------------------------------------------------

	override bool HasAccess()
	{
		return JMPermissions.Has( JMConstants.PERM_ROLES_VIEW );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/role_manager_form.layout";
	}

	override string GetCategory()
	{
		return "Players";
	}

	override string GetTitle()
	{
		return "#STR_COT_ROLEMANAGER_MODULE_NAME";
	}

	override string GetIconName()
	{
		return JMConstants.Lucide( "shield-user" );
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
		return "Role Manager Module";
	}

	override void GetWebhookTypes( out array<string> types )
	{
		types.Insert( "CreateRole" );
		types.Insert( "DeleteRole" );
		types.Insert( "SetRolePermissions" );
	}

	override int GetRPCMin()
	{
		return JMRoleManagerModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMRoleManagerModuleRPC.COUNT;
	}

#ifndef SERVER
	override void EnableUpdate()
	{
	}
#endif

	// -------------------------------------------------------------------------
	//  OnRPC
	// -------------------------------------------------------------------------

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMRoleManagerModuleRPC.RequestRoleList:
			RPC_RequestRoleList( ctx, sender, target );
			break;
		case JMRoleManagerModuleRPC.CreateRole:
			RPC_CreateRole( ctx, sender, target );
			break;
		case JMRoleManagerModuleRPC.DeleteRole:
			RPC_DeleteRole( ctx, sender, target );
			break;
		case JMRoleManagerModuleRPC.SetRolePermissions:
			RPC_SetRolePermissions( ctx, sender, target );
			break;
		case JMRoleManagerModuleRPC.RoleList:
			RPC_RoleList( ctx, sender, target );
			break;
		}
	}

	// -------------------------------------------------------------------------
	//  Public API - called by JMRoleManagerForm
	// -------------------------------------------------------------------------

	void RequestRoleList()
	{
		if ( IsMissionHost() )
		{
			SendRoleListToClient( NULL );
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMRoleManagerModuleRPC.RequestRoleList, true, NULL );
		}
	}

	void CreateRole( string name )
	{
		if ( IsMissionHost() )
		{
			Exec_CreateRole( name, NULL, NULL );
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( name );
			rpc.Send( NULL, JMRoleManagerModuleRPC.CreateRole, true, NULL );
		}
	}

	void DeleteRole( string name )
	{
		if ( IsMissionHost() )
		{
			Exec_DeleteRole( name, NULL, NULL );
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( name );
			rpc.Send( NULL, JMRoleManagerModuleRPC.DeleteRole, true, NULL );
		}
	}

	void SetRolePermissions( string roleName, array<string> serializedPerms )
	{
		if ( IsMissionHost() )
		{
			Exec_SetRolePermissions( roleName, serializedPerms, NULL, NULL );
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( roleName );
			rpc.Write( serializedPerms );
			rpc.Send( NULL, JMRoleManagerModuleRPC.SetRolePermissions, true, NULL );
		}
	}

	// -------------------------------------------------------------------------
	//  Server: send role list to a specific client (or NULL = self on host)
	// -------------------------------------------------------------------------

	private void SendRoleListToClient( PlayerIdentity recipient )
	{
		array<JMRole> roles = new array<JMRole>();
		GetPermissionsManager().GetRolesAsList( roles );

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( roles.Count() );

		foreach ( JMRole role : roles )
		{
			rpc.Write( role.Name );
			array<string> perms = role.Serialize();
			rpc.Write( perms );
		}

		rpc.Send( NULL, JMRoleManagerModuleRPC.RoleList, true, recipient );
	}

	// -------------------------------------------------------------------------
	//  Server RPCs
	// -------------------------------------------------------------------------

	private void RPC_RequestRoleList( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionHost() )
			return;

		if ( !sender ) return;
		if ( !JMPermissions.Has( JMConstants.PERM_ROLES_VIEW, sender ) )
			return;

		SendRoleListToClient( sender );
	}

	private void RPC_CreateRole( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !sender ) return;
		if ( !GetPermissionsManager().HasPermission( JMConstants.PERM_ROLES_CREATE, sender, instance ) )
			return;

		string name;
		if ( !ctx.Read( name ) || name == "" )
			return;

		Exec_CreateRole( name, sender, instance );
	}

	private void RPC_DeleteRole( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !sender ) return;
		if ( !GetPermissionsManager().HasPermission( JMConstants.PERM_ROLES_DELETE, sender, instance ) )
			return;

		string name;
		if ( !ctx.Read( name ) || name == "" )
			return;

		Exec_DeleteRole( name, sender, instance );
	}

	private void RPC_SetRolePermissions( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !sender ) return;
		if ( !GetPermissionsManager().HasPermission( JMConstants.PERM_ROLES_PERMISSIONS, sender, instance ) )
			return;

		string roleName;
		if ( !ctx.Read( roleName ) || roleName == "" )
			return;

		array<string> perms = new array<string>();
		if ( !ctx.Read( perms ) )
			return;

		Exec_SetRolePermissions( roleName, perms, sender, instance );
	}

	// -------------------------------------------------------------------------
	//  Client RPC: receive role list, push to open form
	// -------------------------------------------------------------------------

	private void RPC_RoleList( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( IsMissionHost() )
			return;

		int count;
		if ( !ctx.Read( count ) )
			return;

		array<ref JMRoleData> roles = new array<ref JMRoleData>();

		for ( int i = 0; i < count; i++ )
		{
			string name;
			if ( !ctx.Read( name ) )
				break;

			array<string> perms = new array<string>();
			if ( !ctx.Read( perms ) )
				break;

			JMRoleData rd = new JMRoleData( name, perms );
			roles.Insert( rd );
		}

		JMRoleManagerForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.PopulateRoleList( roles );
	}

	// -------------------------------------------------------------------------
	//  Exec helpers (run server-side, also called directly when IsMissionHost)
	// -------------------------------------------------------------------------

	private void Exec_CreateRole( string name, PlayerIdentity ident, JMPlayerInstance instance )
	{
		if ( GetPermissionsManager().RoleExists( name ) )
		{
			if ( ident )
				COTCreateNotification( ident, new StringLocaliser( "#STR_COT_ROLEMANAGER_MODULE_ROLE_ALREADY_EXISTS_NOTIFICATION", name ) );
			return;
		}

		array<string> emptyPerms = new array<string>();
		GetPermissionsManager().CreateRole( name, emptyPerms );

		GetCommunityOnlineToolsBase().Log( ident, "Created role: " + name );
		SendWebhook( "CreateRole", NULL, "Created role: " + name );

		if ( ident )
			SendRoleListToClient( ident );
	}

	private void Exec_DeleteRole( string name, PlayerIdentity ident, JMPlayerInstance instance )
	{
		if ( name == "everyone" )
		{
			if ( ident )
				COTCreateNotification( ident, new StringLocaliser( "#STR_COT_ROLEMANAGER_MODULE_CANNOT_DELETE_EVERYONE_NOTIFICATION" ) );
			return;
		}

		if ( !GetPermissionsManager().RoleExists( name ) )
		{
			if ( ident )
				COTCreateNotification( ident, new StringLocaliser( "#STR_COT_ROLEMANAGER_MODULE_ROLE_NOT_FOUND_NOTIFICATION", name ) );
			return;
		}

		JMRole role = GetPermissionsManager().GetRole( name );
		if ( role )
		{
			// Erase the permission file for this role
			string rolePath = JMConstants.DIR_ROLES + name + JMConstants.EXT_ROLE;
			if ( FileExist( rolePath ) )
				DeleteFile( rolePath );
		}

		GetPermissionsManager().Roles.Remove( name );

		GetCommunityOnlineToolsBase().Log( ident, "Deleted role: " + name );
		SendWebhook( "DeleteRole", NULL, "Deleted role: " + name );

		if ( ident )
			SendRoleListToClient( ident );
	}

	private void Exec_SetRolePermissions( string roleName, array<string> serializedPerms, PlayerIdentity ident, JMPlayerInstance instance )
	{
		JMRole role = GetPermissionsManager().GetRole( roleName );
		if ( !role )
		{
			array<string> emptyPerms = new array<string>();
			role = GetPermissionsManager().CreateRole( roleName, emptyPerms );
		}

		role.SerializedData.Clear();
		role.SerializedData.Copy( serializedPerms );
		role.Deserialize();
		role.Save();

		GetCommunityOnlineToolsBase().Log( ident, "Updated permissions for role: " + roleName );
		SendWebhook( "SetRolePermissions", NULL, "Updated permissions for role: " + roleName );

		if ( ident )
			SendRoleListToClient( ident );
	}
}

// Lightweight DTO used to pass role data from server -> client via RPC
class JMRoleData
{
	string           Name;
	ref array<string> Permissions;

	void JMRoleData( string name, array<string> perms )
	{
		Name        = name;
		Permissions = new array<string>();
		Permissions.Copy( perms );
	}
}
