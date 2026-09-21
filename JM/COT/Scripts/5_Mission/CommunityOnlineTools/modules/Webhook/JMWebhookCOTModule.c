class JMWebhookCOTModule: JMRenderableModuleBase
{
	protected JMWebhookSerialize m_Settings;

	void ~JMWebhookCOTModule()
	{
	}

	array< ref JMWebhookConnectionGroup > GetConnections()
	{
		if ( !m_Settings )
			return new array< ref JMWebhookConnectionGroup >();
		return m_Settings.Connections;
	}

	override void DescribeModule( JMModuleInfo info )
	{
		super.DescribeModule( info );

		info.Title = "#STR_COT_WEBHOOK_MODULE_NAME";
		info.WebhookTitle = "Manage Webhooks";
		info.Icon = "webhook";
		info.Layout = "JM/COT/GUI/layouts/webhook_form.layout";
		info.Category = JMSideBarConfig.CATEGORY_SERVER;
		info.ViewPermission = JMConstants.PERM_WEBHOOK_VIEW;
		info.InputToggle = "UACOTToggleWebhook";
		info.SetRPCRange( JMWebhookCOTModuleRPC.INVALID, JMWebhookCOTModuleRPC.COUNT );

		//! Called on both client and server as the module registers, before the mission loads.
		info.AddPermission( JMConstants.PERM_WEBHOOK_MANAGE );
		info.AddPermission( JMConstants.PERM_WEBHOOK_MANAGE_URL );
		info.AddPermission( JMConstants.PERM_WEBHOOK_MANAGE_URL_ADD );
		info.AddPermission( JMConstants.PERM_WEBHOOK_MANAGE_URL_REMOVE );
		info.AddPermission( JMConstants.PERM_WEBHOOK_MANAGE_URL_EDIT );
		info.AddPermission( JMConstants.PERM_WEBHOOK_MANAGE_TYPE );
		info.AddPermission( JMConstants.PERM_WEBHOOK_MANAGE_TYPE_ADD );
		info.AddPermission( JMConstants.PERM_WEBHOOK_MANAGE_TYPE_REMOVE );
		info.AddPermission( JMConstants.PERM_WEBHOOK_MANAGE_TYPE_STATE );

		info.AddWebhookType( "URL" );
		info.AddWebhookType( "URLSensitive" );
		info.AddWebhookType( "Type" );
		info.AddWebhookType( "TypeSensitive" );
	}

	override void EnableUpdate()
	{
	}

	override void OnMissionStart()
	{
		super.OnMissionStart();

		m_Settings = GetCOTWebhookSettings();

		if ( IsMissionHost() )
		{
			OnSettingsUpdated();
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMWebhookCOTModuleRPC.Load, true, NULL );
		}
	}

	//! The OnMissionStart request can be denied because the server has not
	//! registered this player yet - see Exec_Load. Permissions arriving is
	//! exactly the point at which that request would now succeed, so ask again.
	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();

		if ( IsMissionHost() )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_WEBHOOK_VIEW ) )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMWebhookCOTModuleRPC.Load, true, NULL );
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();

		m_Settings = NULL;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMWebhookCOTModuleRPC.Load:
			RPC_Load( ctx, sender, target );
			break;
		case JMWebhookCOTModuleRPC.AddConnectionGroup:
			RPC_AddConnectionGroup( ctx, sender, target );
			break;
		case JMWebhookCOTModuleRPC.RemoveConnectionGroup:
			RPC_RemoveConnectionGroup( ctx, sender, target );
			break;
		case JMWebhookCOTModuleRPC.EditConnectionGroup:
			RPC_EditConnectionGroup( ctx, sender, target );
			break;
		case JMWebhookCOTModuleRPC.AddType:
			RPC_AddType( ctx, sender, target );
			break;
		case JMWebhookCOTModuleRPC.RemoveType:
			RPC_RemoveType( ctx, sender, target );
			break;
		case JMWebhookCOTModuleRPC.TypeState:
			RPC_TypeState( ctx, sender, target );
			break;
		}
	}

	// -------------------------------------------------------------------------
	//  Load
	// -------------------------------------------------------------------------

	//! These settings carry the Discord webhook URLs, which are secrets - anyone
	//! holding one can post to the server's Discord. So this is permission
	//! gated like every other module's Exec_Load (Weather, Compensations,
	//! Loadout, Teleport all do the same).
	//!
	//! This used to skip the check because the client requests Load from
	//! OnMissionStart, which can land before the player is registered
	//! server-side - and HasPermission denies when it cannot find the instance.
	//! Denying is the right answer there; the fix for the race is to ask again
	//! rather than to hand the URLs to everyone. See OnClientPermissionsUpdated
	//! below, which re-requests once permissions actually arrive.
	protected void Exec_Load( notnull PlayerIdentity ident )
	{
		if ( !GetPermissionsManager().GetPlayer( ident.GetId() ) )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_WEBHOOK_VIEW, ident ) )
			return;

		ScriptRPC rpc = new ScriptRPC();
		m_Settings.OnSend( rpc );
		rpc.Send( NULL, JMWebhookCOTModuleRPC.Load, true, ident );
	}

	protected void RPC_Load( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( g_Game.IsDedicatedServer() )
		{
			if ( !senderRPC )
				return;
			Exec_Load( senderRPC );
		}
		else
		{
			if ( m_Settings.OnRecieve( ctx ) )
			{
				OnSettingsUpdated();
			}
		}
	}

	// -------------------------------------------------------------------------
	//  AddConnectionGroup
	// -------------------------------------------------------------------------

	void AddConnectionGroup( string name, string url )
	{
		if ( !IsMissionClient() )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( name );
		rpc.Write( url );
		rpc.Send( NULL, JMWebhookCOTModuleRPC.AddConnectionGroup, true, NULL );
	}

	protected void Exec_AddConnectionGroup( string name, string url )
	{
		JMWebhookConnectionGroup connection = m_Settings.Get( name );
		connection.ContextURL = "https://discordapp.com/api/webhooks/";
		connection.Address = url;
		m_Settings.Save();
	}

	protected void RPC_AddConnectionGroup( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		string name;
		if ( !ctx.Read( name ) )
			return;

		string url;
		if ( !ctx.Read( url ) )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_WEBHOOK_MANAGE_URL_ADD, senderRPC, instance ) )
			return;

		Exec_AddConnectionGroup( name, url );

		Exec_Load( senderRPC );
	}

	// -------------------------------------------------------------------------
	//  RemoveConnectionGroup
	// -------------------------------------------------------------------------

	void RemoveConnectionGroup( string name )
	{
		if ( !IsMissionClient() )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( name );
		rpc.Send( NULL, JMWebhookCOTModuleRPC.RemoveConnectionGroup, true, NULL );
	}

	protected void Exec_RemoveConnectionGroup( string name )
	{
		m_Settings.Remove( name );
		m_Settings.Save();
	}

	protected void RPC_RemoveConnectionGroup( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		string name;
		if ( !ctx.Read( name ) )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_WEBHOOK_MANAGE_URL_REMOVE, senderRPC, instance ) )
			return;

		Exec_RemoveConnectionGroup( name );

		Exec_Load( senderRPC );
	}

	// -------------------------------------------------------------------------
	//  EditConnectionGroup - update name, URL, and optional player/role filter
	// -------------------------------------------------------------------------

	void EditConnectionGroup( string oldName, string newName, string newUrl, string filterGUID, string filterRole )
	{
		if ( !IsMissionClient() )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( oldName );
		rpc.Write( newName );
		rpc.Write( newUrl );
		rpc.Write( filterGUID );
		rpc.Write( filterRole );
		rpc.Send( NULL, JMWebhookCOTModuleRPC.EditConnectionGroup, true, NULL );
	}

	protected void Exec_EditConnectionGroup( string oldName, string newName, string newUrl, string filterGUID, string filterRole )
	{
		JMWebhookConnectionGroup group = m_Settings.Get( oldName );
		if ( !group )
			return;

		group.Name       = newName;
		group.Address    = newUrl;
		group.ContextURL = "https://discordapp.com/api/webhooks/";
		group.FilterGUID = filterGUID;
		group.FilterRole = filterRole;

		m_Settings.Save();
	}

	protected void RPC_EditConnectionGroup( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		string oldName;
		if ( !ctx.Read( oldName ) )
			return;

		string newName;
		if ( !ctx.Read( newName ) )
			return;

		string newUrl;
		if ( !ctx.Read( newUrl ) )
			return;

		string filterGUID;
		if ( !ctx.Read( filterGUID ) )
			return;

		string filterRole;
		if ( !ctx.Read( filterRole ) )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_WEBHOOK_MANAGE_URL_EDIT, senderRPC, instance ) )
			return;

		Exec_EditConnectionGroup( oldName, newName, newUrl, filterGUID, filterRole );

		Exec_Load( senderRPC );
	}

	// -------------------------------------------------------------------------
	//  AddType
	// -------------------------------------------------------------------------

	void AddType( string name, string group, bool enabled )
	{
		if ( !IsMissionClient() )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( name );
		rpc.Write( group );
		rpc.Write( enabled );
		rpc.Send( NULL, JMWebhookCOTModuleRPC.AddType, true, NULL );
	}

	protected void Exec_AddType( string name, string group, bool enabled )
	{
		JMWebhookConnectionGroup conn = m_Settings.Get( group );
		if ( conn )
			conn.Set( name, enabled );

		m_Settings.Save();
	}

	protected void RPC_AddType( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		string name;
		if ( !ctx.Read( name ) )
			return;

		string group;
		if ( !ctx.Read( group ) )
			return;

		bool enabled;
		if ( !ctx.Read( enabled ) )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_WEBHOOK_MANAGE_TYPE_ADD, senderRPC, instance ) )
			return;

		Exec_AddType( name, group, enabled );

		Exec_Load( senderRPC );
	}

	// -------------------------------------------------------------------------
	//  RemoveType
	// -------------------------------------------------------------------------

	void RemoveType( string name, string group )
	{
		if ( !IsMissionClient() )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( name );
		rpc.Write( group );
		rpc.Send( NULL, JMWebhookCOTModuleRPC.RemoveType, true, NULL );
	}

	protected void Exec_RemoveType( string name, string group )
	{
		JMWebhookConnectionGroup conn = m_Settings.Get( group );
		if ( conn )
			conn.Remove( name );

		m_Settings.Save();
	}

	protected void RPC_RemoveType( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		string name;
		if ( !ctx.Read( name ) )
			return;

		string group;
		if ( !ctx.Read( group ) )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_WEBHOOK_MANAGE_TYPE_REMOVE, senderRPC, instance ) )
			return;

		Exec_RemoveType( name, group );

		Exec_Load( senderRPC );
	}

	// -------------------------------------------------------------------------
	//  TypeState
	// -------------------------------------------------------------------------

	void TypeState( string name, string group, bool enabled )
	{
		if ( !IsMissionClient() )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( name );
		rpc.Write( group );
		rpc.Write( enabled );
		rpc.Send( NULL, JMWebhookCOTModuleRPC.TypeState, true, NULL );
	}

	protected void Exec_TypeState( string name, string group, bool enabled )
	{
		JMWebhookConnectionGroup conn = m_Settings.Get( group );
		if ( conn )
			conn.Set( name, enabled );

		m_Settings.Save();
	}

	protected void RPC_TypeState( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		string name;
		if ( !ctx.Read( name ) )
			return;

		string group;
		if ( !ctx.Read( group ) )
			return;

		bool enabled;
		if ( !ctx.Read( enabled ) )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC ) return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_WEBHOOK_MANAGE_TYPE_STATE, senderRPC, instance ) )
			return;

		Exec_TypeState( name, group, enabled );

		Exec_Load( senderRPC );
	}
}
