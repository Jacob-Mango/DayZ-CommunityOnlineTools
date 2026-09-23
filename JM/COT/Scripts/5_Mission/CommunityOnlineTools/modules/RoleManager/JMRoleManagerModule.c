class JMRoleManagerModule : JMRenderableModuleBase
{
	void SetRolePermissions( string roleName, JMPermission perms )
	{
		if ( IsMissionHost() )
		{
			Exec_SetRolePermissions( roleName, perms, NULL, NULL );
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( roleName );
			perms.OnSend( rpc );
			rpc.Send( NULL, JMRoleManagerModuleRPC.SetRolePermissions, true, NULL );
		}
	}

	override void DescribeModule( JMModuleInfo info )
	{
		super.DescribeModule( info );

		info.Title = "#STR_COT_ROLEMANAGER_MODULE_NAME";
		info.WebhookTitle = "Role Manager Module";
		info.Icon = "shield-user";
		info.Layout = "JM/COT/GUI/layouts/role_manager_form.layout";
		info.Category = JMSideBarConfig.CATEGORY_PLAYERS;
		info.ViewPermission = JMConstants.PERM_ROLES_VIEW;
		info.SetRPCRange( JMRoleManagerModuleRPC.INVALID, JMRoleManagerModuleRPC.COUNT );

		//! Called on both client and server as the module registers, before the mission loads.
		info.AddPermission( JMConstants.PERM_ROLES_CREATE );
		info.AddPermission( JMConstants.PERM_ROLES_DELETE );
		info.AddPermission( JMConstants.PERM_ROLES_PERMISSIONS );

		info.AddWebhookType( "CreateRole" );
		info.AddWebhookType( "DeleteRole" );
		info.AddWebhookType( "SetRolePermissions" );
	}

	// -------------------------------------------------------------------------
	//  JMRenderableModuleBase overrides
	// -------------------------------------------------------------------------

	override void EnableUpdate()
	{
	}

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

	override void RequestData()
	{
		//! COTModule::OnInvokeConnect syncs all roles to clients so we already have them
		JMRoleManagerForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.PopulateRoleList();
	}

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

	// -------------------------------------------------------------------------
	//  Server: send role list to a specific client (or NULL = self on host)
	// -------------------------------------------------------------------------

	protected void SendRoleListToClient( PlayerIdentity recipient )
	{
		map<string, ref JMRole> roles = GetPermissionsManager().Roles;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( roles.Count() );

		foreach ( string name, JMRole role : roles )
		{
			rpc.Write( role.Name );
			role.RootPermission.OnSend(rpc);
		}

		rpc.Send( NULL, JMRoleManagerModuleRPC.RoleList, true, recipient );
	}

	// -------------------------------------------------------------------------
	//  Server RPCs
	// -------------------------------------------------------------------------

	protected void RPC_RequestRoleList( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionHost() )
			return;

		if ( !sender ) return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_ROLES_VIEW, sender ) )
			return;

		SendRoleListToClient( sender );
	}

	protected void RPC_CreateRole( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !sender ) return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_ROLES_CREATE, sender, instance ) )
			return;

		string name;
		if ( !ctx.Read( name ) || name == "" )
			return;

		Exec_CreateRole( name, sender, instance );
	}

	protected void RPC_DeleteRole( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !sender ) return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_ROLES_DELETE, sender, instance ) )
			return;

		string name;
		if ( !ctx.Read( name ) || name == "" )
			return;

		Exec_DeleteRole( name, sender, instance );
	}

	protected void RPC_SetRolePermissions( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !sender ) return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_ROLES_PERMISSIONS, sender, instance ) )
			return;

		string roleName;
		if ( !ctx.Read( roleName ) || roleName == "" )
			return;

		JMPermission perms = new JMPermission( JMConstants.PERM_ROOT );
		perms.CopyPermissions(GetPermissionsManager().RootPermission);
		if ( !perms.OnReceive( ctx ) )
			return;

		Exec_SetRolePermissions( roleName, perms, sender, instance );
	}

	// -------------------------------------------------------------------------
	//  Client RPC: receive role list, push to open form
	// -------------------------------------------------------------------------

	protected void RPC_RoleList( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( IsMissionHost() )
			return;

		int count;
		if ( !ctx.Read( count ) )
			return;

		GetPermissionsManager().Roles.Clear();

		for ( int i = 0; i < count; i++ )
		{
			string name;
			if ( !ctx.Read( name ) )
				break;

			JMRole role;
			GetPermissionsManager().LoadRole( name, role );
			if (!role.RootPermission.OnReceive(ctx))
				break;
		}

		JMRoleManagerForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.PopulateRoleList();
	}

	// -------------------------------------------------------------------------
	//  Exec helpers (run server-side, also called directly when IsMissionHost)
	// -------------------------------------------------------------------------

	protected void Exec_CreateRole( string name, PlayerIdentity ident, JMPlayerInstance instance )
	{
		if ( GetPermissionsManager().RoleExists( name ) )
		{
			if ( ident )
				COTCreateNotification( ident, new StringLocaliser( "#STR_COT_ROLEMANAGER_MODULE_ROLE_ALREADY_EXISTS_NOTIFICATION", name ) );
			return;
		}

		GetPermissionsManager().CreateRole( name );

		GetCommunityOnlineToolsBase().Log( ident, "Created role: " + name );
		SendWebhook( "CreateRole", NULL, "Created role: " + name );

		if ( ident )
			SendRoleListToClient( ident );
	}

	protected void Exec_DeleteRole( string name, PlayerIdentity ident, JMPlayerInstance instance )
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

	protected void Exec_SetRolePermissions( string roleName, JMPermission perms, PlayerIdentity ident, JMPlayerInstance instance )
	{
		JMRole role = GetPermissionsManager().GetRole( roleName );
		if ( !role )
		{
			role = GetPermissionsManager().CreateRole( roleName );
		}

		role.RootPermission.CopyPermissions(perms);
		role.Save();

		GetCommunityOnlineToolsBase().Log( ident, "Updated permissions for role: " + roleName );
		SendWebhook( "SetRolePermissions", NULL, "Updated permissions for role: " + roleName );

		if ( ident )
			SendRoleListToClient( ident );
	}
}

