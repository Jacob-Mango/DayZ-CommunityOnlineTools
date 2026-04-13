class JMWebhookCOTModule: JMRenderableModuleBase
{
	private JMWebhookSerialize m_Settings;

	void JMWebhookCOTModule()
	{
		GetPermissionsManager().RegisterPermission( "Webhook.View" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage.URL" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage.URL.Add" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage.URL.Remove" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage.URL.Edit" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage.Type" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage.Type.Add" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage.Type.Remove" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage.Type.State" );
	}

	void ~JMWebhookCOTModule()
	{
	}

	override void EnableUpdate()
	{
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Webhook.View" );
	}

	override string GetInputToggle()
	{
		return "UACOTToggleWebhook";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/webhook_form.layout";
	}

	override string GetCategory()
	{
		return "Other";
	}

	override string GetTitle()
	{
		return "#STR_COT_WEBHOOK_MODULE_NAME";
	}

	override string GetIconName()
	{
		return "WH";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override string GetWebhookTitle()
	{
		return "Manage Webhooks";
	}

	override void GetWebhookTypes( out array< string > types )
	{
		types.Insert( "URL" );
		types.Insert( "URLSensitive" );
		types.Insert( "Type" );
		types.Insert( "TypeSensitive" );
	}

	array< ref JMWebhookConnectionGroup > GetConnections()
	{
		if ( !m_Settings )
			return new array< ref JMWebhookConnectionGroup >();
		return m_Settings.Connections;
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

	override void OnMissionFinish()
	{
		super.OnMissionFinish();

		m_Settings = NULL;
	}

	override int GetRPCMin()
	{
		return JMWebhookCOTModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMWebhookCOTModuleRPC.COUNT;
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

	private void Server_Load( notnull PlayerIdentity ident )
	{
		// Skip permission check for the initial settings sync — the player instance
		// may not exist yet if the Load RPC arrives before the player is fully registered.
		// Mutation RPCs (Add/Remove/Edit) still enforce full permission checks.
		ScriptRPC rpc = new ScriptRPC();
		m_Settings.OnSend( rpc );
		rpc.Send( NULL, JMWebhookCOTModuleRPC.Load, true, ident );
	}

	private void RPC_Load( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( g_Game.IsDedicatedServer() )
		{
			Server_Load( senderRPC );
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

	private void Exec_AddConnectionGroup( string name, string url )
	{
		JMWebhookConnectionGroup connection = m_Settings.Get( name );
		connection.ContextURL = "https://discordapp.com/api/webhooks/";
		connection.Address = url;
		m_Settings.Save();
	}

	private void RPC_AddConnectionGroup( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
		if ( !GetPermissionsManager().HasPermission( "Webhook.Manage.URL.Add", senderRPC, instance ) )
			return;

		Exec_AddConnectionGroup( name, url );

		Server_Load( senderRPC );
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

	private void Exec_RemoveConnectionGroup( string name )
	{
		m_Settings.Remove( name );
		m_Settings.Save();
	}

	private void RPC_RemoveConnectionGroup( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		string name;
		if ( !ctx.Read( name ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Webhook.Manage.URL.Remove", senderRPC, instance ) )
			return;

		Exec_RemoveConnectionGroup( name );

		Server_Load( senderRPC );
	}

	// -------------------------------------------------------------------------
	//  EditConnectionGroup — update name, URL, and optional player/role filter
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

	private void Exec_EditConnectionGroup( string oldName, string newName, string newUrl, string filterGUID, string filterRole )
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

	private void RPC_EditConnectionGroup( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
		if ( !GetPermissionsManager().HasPermission( "Webhook.Manage.URL.Edit", senderRPC, instance ) )
			return;

		Exec_EditConnectionGroup( oldName, newName, newUrl, filterGUID, filterRole );

		Server_Load( senderRPC );
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

	private void Exec_AddType( string name, string group, bool enabled )
	{
		JMWebhookConnectionGroup conn = m_Settings.Get( group );
		if ( conn )
			conn.Set( name, enabled );

		m_Settings.Save();
	}

	private void RPC_AddType( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
		if ( !GetPermissionsManager().HasPermission( "Webhook.Manage.Type.Add", senderRPC, instance ) )
			return;

		Exec_AddType( name, group, enabled );

		Server_Load( senderRPC );
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

	private void Exec_RemoveType( string name, string group )
	{
		JMWebhookConnectionGroup conn = m_Settings.Get( group );
		if ( conn )
			conn.Remove( name );

		m_Settings.Save();
	}

	private void RPC_RemoveType( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
		if ( !GetPermissionsManager().HasPermission( "Webhook.Manage.Type.Remove", senderRPC, instance ) )
			return;

		Exec_RemoveType( name, group );

		Server_Load( senderRPC );
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

	private void Exec_TypeState( string name, string group, bool enabled )
	{
		JMWebhookConnectionGroup conn = m_Settings.Get( group );
		if ( conn )
			conn.Set( name, enabled );

		m_Settings.Save();
	}

	private void RPC_TypeState( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
		if ( !GetPermissionsManager().HasPermission( "Webhook.Manage.Type.State", senderRPC, instance ) )
			return;

		Exec_TypeState( name, group, enabled );

		Server_Load( senderRPC );
	}
}
