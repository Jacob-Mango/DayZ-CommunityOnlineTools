modded class JMModuleBase
{
	protected JMWebhookModule m_Webhook;

	// Per-sender rate limiting: maps sender GUID -> last RPC time (g_Game.GetTime() ms)
	protected ref map< string, int > m_RPCLastTime = new map< string, int >();

	// Returns true if this sender is sending too fast and the RPC should be dropped.
	protected bool IsRateLimited( PlayerIdentity sender )
	{
		if ( !sender )
			return false;

		string guid    = sender.GetId();
		int    now     = g_Game.GetTime();
		int    limitMs = (int)( JMConstants.RPC_RATE_LIMIT_S * 1000.0 );

		int last;
		if ( m_RPCLastTime.Find( guid, last ) && ( now - last ) < limitMs )
			return true;

		m_RPCLastTime.Set( guid, now );
		return false;
	}

	// Gate applied once here so every subclass module is covered automatically.
	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		if ( IsMissionHost() && IsRateLimited( sender ) )
		{
			COTCreateNotification( sender, new StringLocaliser( "STR_COT_NOTIFICATION_RATE_LIMITED" ) );
			return;
		}

		super.OnRPC( sender, target, rpc_type, ctx );
	}

	override void OnMissionStart()
	{
		super.OnMissionStart();

		Class.CastTo( m_Webhook, GetModuleManager().GetModule( JMWebhookModule ) );
	}

	void SendWebhook( string type, string message )
	{
		if ( !m_Webhook || IsMissionOffline() )
			return;

		auto msg = m_Webhook.CreateDiscordMessage();

		msg.GetEmbed().AddField( GetWebhookTitle(), message, false );

		m_Webhook.Post( GetModuleName() + type, msg );
	}

	// SendWebhook with a specific embed color for the event category.
	void SendWebhookColored( string type, string message, int color )
	{
		if ( !m_Webhook || IsMissionOffline() )
			return;

		auto msg = m_Webhook.CreateDiscordMessageColored( color );

		msg.GetEmbed().AddField( GetWebhookTitle(), message, false );

		m_Webhook.Post( GetModuleName() + type, msg );
	}

	void SendWebhook( string type, JMPlayerInstance player, string message )
	{
		#ifdef JM_COT_WEBHOOK_DEBUG
		Print( "+JMModuleBase::SendWebhook() - Admin" );
		#endif
		if ( !m_Webhook || !player || IsMissionOffline() )
			return;

		auto msg = m_Webhook.CreateDiscordMessage( player, "Admin Account: " );

		msg.GetEmbed().AddField( GetWebhookTitle(), message, false );

		#ifdef JM_COT_WEBHOOK_DEBUG
		Print( "name=" + GetModuleName() + type );
		Print( "message=" + message );
		#endif

		m_Webhook.Post( GetModuleName() + type, msg );

		#ifdef JM_COT_WEBHOOK_DEBUG
		Print( "-JMModuleBase::SendWebhook() - Admin" );
		#endif
	}

	// SendWebhook with admin context and a specific embed color.
	void SendWebhookColored( string type, JMPlayerInstance player, string message, int color )
	{
		if ( !m_Webhook || !player || IsMissionOffline() )
			return;

		auto msg = m_Webhook.CreateDiscordMessageAdmin( player, "Admin Account: ", color );

		msg.GetEmbed().AddField( GetWebhookTitle(), message, false );

		m_Webhook.Post( GetModuleName() + type, msg );
	}

	string GetWebhookTitle()
	{
		return "Unknown";
	}

	void GetWebhookTypes( out array< string > types )
	{
		
	}

#ifndef CF_MODULE_PERMISSIONS
	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();
	}
#endif

	void GetSubCommands(inout array<ref JMCommand> commands)
	{
	}

	void AddSubCommand(inout array<ref JMCommand> commands, string command, string function, string permission)
	{
		commands.Insert(new JMSubCommand(this, command, function, permission));
	}

	array<string> GetCommandNames()
	{
		return new array<string>();
	}
}
