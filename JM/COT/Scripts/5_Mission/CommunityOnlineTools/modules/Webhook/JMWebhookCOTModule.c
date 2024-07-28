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

	private void Server_Load( notnull PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Webhook.View", ident, instance ) )
			return;

		ScriptRPC rpc = new ScriptRPC();
		m_Settings.OnSend( rpc );
		rpc.Send( NULL, JMWebhookCOTModuleRPC.Load, true, ident );
	}

	private void RPC_Load( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_Load").Add(senderRPC);
		#endif

		if ( GetGame().IsDedicatedServer() )
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

	void AddConnectionGroup( string name, string url )
	{
		if ( !IsMissionClient() )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( name );
		rpc.Write( url );
		rpc.Send( NULL, JMWebhookCOTModuleRPC.AddConnectionGroup, true, NULL );
	}

	private void Exec_AddConnectionGroup( string name, string url, JMPlayerInstance instance )
	{
		JMWebhookConnectionGroup connection = m_Settings.Get( name );

		connection.ContextURL = "https://discordapp.com/api/webhooks/";
		connection.Address = url;

		m_Settings.Save();
	}

	private void RPC_AddConnectionGroup( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_AddConnectionGroup").Add(senderRPC);
		#endif

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

		Exec_AddConnectionGroup( name, url, instance );
	}

	void RemoveConnectionGroup( string name )
	{
		if ( !IsMissionClient() )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( name );
		rpc.Send( NULL, JMWebhookCOTModuleRPC.RemoveConnectionGroup, true, NULL );
	}

	private void Exec_RemoveConnectionGroup( string name, JMPlayerInstance instance )
	{
		m_Webhook.RemoveGroup( name );

		m_Settings.Save();
	}

	private void RPC_RemoveConnectionGroup( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_RemoveConnectionGroup").Add(senderRPC);
		#endif

		if ( !IsMissionHost() )
			return;
		string name;
		if ( !ctx.Read( name ) )
			return;
		
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Webhook.Manage.URL.Rempve", senderRPC, instance ) )
			return;

		Exec_RemoveConnectionGroup( name, instance );
	}

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

	private void Exec_AddType( string name, string group, bool enabled, JMPlayerInstance instance )
	{
		m_Webhook.AddConnection( name, group );
		m_Webhook.SetConnection( name, group, enabled );

		m_Settings.Save();
	}

	private void RPC_AddType( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_AddType").Add(senderRPC);
		#endif

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

		Exec_AddType( name, group, enabled, instance );
	}

	void RemoveType( string name, string group )
	{
		if ( !IsMissionClient() )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( name );
		rpc.Write( group );
		rpc.Send( NULL, JMWebhookCOTModuleRPC.RemoveType, true, NULL );
	}

	private void Exec_RemoveType( string name, string group, JMPlayerInstance instance )
	{
		m_Webhook.RemoveConnection( name, group );

		m_Settings.Save();
	}

	private void RPC_RemoveType( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_RemoveType").Add(senderRPC);
		#endif

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

		Exec_RemoveType( name, group, instance );
	}

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

	private void Exec_TypeState( string name, string group, bool enabled, JMPlayerInstance instance )
	{
		m_Webhook.SetConnection( name, group, enabled );

		m_Settings.Save();
	}

	private void RPC_TypeState( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_TypeState").Add(senderRPC);
		#endif

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

		Exec_TypeState( name, group, enabled, instance );
	}
};