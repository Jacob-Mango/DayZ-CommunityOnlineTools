modded class JMModuleBase
{
	protected JMWebhookModule m_Webhook;

	// Per-sender rate limiting: maps sender GUID -> last RPC time (g_Game.GetTime() ms)
	protected ref map< string, int > m_RPCLastTime = new map< string, int >();

	//! Event types this module has already been reported for, so the warning below is once per type.
	protected ref array< string > m_UndeclaredWebhookTypes;

	//! What the module declared in DescribeModule(); built on first use.
	protected ref JMModuleInfo m_Info;

	//! RPC id -> the JMModuleAction type that owns it. Built on first use from RegisterActions().
	protected ref map< int, typename > m_ActionTypes;

	array<string> GetCommandNames()
	{
		return new array<string>();
	}

	//! The module's declaration. A getter the module overrides itself still wins over it.
	JMModuleInfo GetModuleInfo()
	{
		if ( !m_Info )
		{
			m_Info = new JMModuleInfo();
			DescribeModule( m_Info );
		}

		return m_Info;
	}

	void GetSubCommands(inout array<ref JMCommand> commands)
	{
	}

	string GetWebhookTitle()
	{
		string title = GetModuleInfo().WebhookTitle;
		if ( title != "" )
			return title;

		return "Unknown";
	}

	//! Every webhook type the module declared plus the ones its actions send. A module that
	//! overrides this without calling super lists its own and gets none of the above.
	void GetWebhookTypes( out array< string > types )
	{
		array< string > declared = GetModuleInfo().GetWebhookTypes();
		foreach ( string declaredType : declared )
		{
			if ( types.Find( declaredType ) == -1 )
				types.Insert( declaredType );
		}

		map< int, typename > actionTypes = GetActionTypes();
		foreach ( int rpc, typename actionType : actionTypes )
		{
			JMModuleAction prototype = JMModuleAction.Cast( actionType.Spawn() );
			if ( !prototype )
				continue;

			string actionWebhook = prototype.GetWebhookType();
			if ( actionWebhook != "" && types.Find( actionWebhook ) == -1 )
				types.Insert( actionWebhook );
		}
	}

	override int GetRPCMin()
	{
		JMModuleInfo info = GetModuleInfo();
		if ( info.RPCMax > 0 )
			return info.RPCMin;

		return super.GetRPCMin();
	}

	override int GetRPCMax()
	{
		JMModuleInfo info = GetModuleInfo();
		if ( info.RPCMax > 0 )
			return info.RPCMax;

		return super.GetRPCMax();
	}

	//! Fill in the module's declaration. Override, call super, then set what applies - see JMModuleInfo.
	void DescribeModule( JMModuleInfo info )
	{
	}

	//! Declare the module's client -> server actions. Override, call super, and DefineAction( Type )
	//! for each - see JMModuleAction.
	void RegisterActions()
	{
	}

	//! Register one action type under the RPC id it names.
	protected void DefineAction( typename type )
	{
		JMModuleAction prototype = JMModuleAction.Cast( type.Spawn() );
		if ( !prototype )
			return;

		if ( m_ActionTypes.Contains( prototype.GetRPC() ) )
		{
			Error( "" + GetModuleName() + ": two actions claim rpc " + prototype.GetRPC() );
			return;
		}

		m_ActionTypes.Set( prototype.GetRPC(), type );
	}

	map< int, typename > GetActionTypes()
	{
		if ( !m_ActionTypes )
		{
			m_ActionTypes = new map< int, typename >;
			RegisterActions();
		}

		return m_ActionTypes;
	}

	//! Send the action to the server, or run it here when this process is the host.
	void SubmitAction( JMModuleAction action )
	{
		if ( IsMissionHost() )
		{
			ExecAction( action, NULL );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		action.Write( rpc );
		rpc.Send( NULL, action.GetRPC(), true, NULL );
	}

	//! Call from OnRPC(). True when rpc_type belonged to an action, so the id has been handled
	//! (or dropped) and the caller has nothing left to do with it.
	protected bool RunAction( PlayerIdentity sender, int rpc_type, ParamsReadContext ctx )
	{
		map< int, typename > actionTypes = GetActionTypes();

		typename type;
		if ( !actionTypes.Find( rpc_type, type ) )
			return false;

		//! Actions travel client -> server only.
		if ( !IsMissionHost() )
			return true;

		JMModuleAction action = JMModuleAction.Cast( type.Spawn() );
		if ( !action )
			return true;

		if ( !action.Read( ctx ) )
			return true;

		ExecAction( action, sender );
		return true;
	}

	//! Permission gate, apply, then the log line and the webhook.
	protected void ExecAction( JMModuleAction action, PlayerIdentity sender )
	{
		string permission = action.GetPermission();
		if ( permission == "" )
			return;

		JMPlayerInstance admin;
		if ( !JMPermissions.HasRPC( permission, sender, admin ) )
			return;

		if ( !action.Apply( this, sender, admin ) )
			return;

		string text = action.Describe();
		if ( text == "" )
			return;

		GetCommunityOnlineToolsBase().Log( sender, text );

		string webhookType = action.GetWebhookType();
		if ( webhookType != "" )
			SendWebhookColored( webhookType, admin, action.DescribeWebhook(), action.GetWebhookColor() );
	}

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

	//! Opt-in flood gate: runs only if OnRPC() calls super.OnRPC(). Deliberately not the norm: one
	//! click can send a burst (Player Apply sends up to seven) and a blanket limit would drop all
	//! but the first. For handlers costing disk or HTTP use JMRPCThrottle (per action).
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

	//! A webhook type only reaches a Discord channel if the admin can switch it on, and the list they
	//! switch from is GetWebhookTypes(). A type sent without being declared there is silently
	//! unreachable, so diagnostic builds say so - once per type - instead of letting it drift.
	protected void CheckWebhookType( string type )
	{
	#ifdef DIAG
		array< string > declared = new array< string >;
		GetWebhookTypes( declared );

		if ( declared.Find( type ) != -1 )
			return;

		if ( !m_UndeclaredWebhookTypes )
			m_UndeclaredWebhookTypes = new array< string >;

		if ( m_UndeclaredWebhookTypes.Find( type ) != -1 )
			return;

		m_UndeclaredWebhookTypes.Insert( type );

		CF_Log.Warn( "%1 sent webhook type '%2' that is not in its GetWebhookTypes(); nobody can subscribe to it", GetModuleName(), type );
	#endif
	}

	void SendWebhook( string type, string message )
	{
		if ( !m_Webhook || IsMissionOffline() )
			return;

		CheckWebhookType( type );

		auto msg = m_Webhook.CreateDiscordMessage();

		msg.GetEmbed().AddField( GetWebhookTitle(), message, false );

		m_Webhook.Post( GetModuleName() + type, msg );
	}

	// SendWebhook with a specific embed color for the event category.
	void SendWebhookColored( string type, string message, int color )
	{
		if ( !m_Webhook || IsMissionOffline() )
			return;

		CheckWebhookType( type );

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

		CheckWebhookType( type );

		auto msg = m_Webhook.CreateDiscordMessageAdmin( player, "Player:", JMConstants.WEBHOOK_COLOR_INFO );

		AddWebhookBody( msg, type, message );

		#ifdef JM_COT_WEBHOOK_DEBUG
		Print( "name=" + GetModuleName() + type );
		Print( "message=" + message );
		#endif

		//! The acting admin is the target of the group GUID / role filters.
		m_Webhook.PostAs( GetModuleName() + type, msg, player );

		#ifdef JM_COT_WEBHOOK_DEBUG
		Print( "-JMModuleBase::SendWebhook() - Admin" );
		#endif
	}

	// SendWebhook with admin context and a specific embed color.
	//! `details` is extra markdown lines for the event (before / after values, the target, ...),
	//! shown under the one-line `message`.
	void SendWebhookColored( string type, JMPlayerInstance player, string message, int color, string details = "" )
	{
		if ( !m_Webhook || !player || IsMissionOffline() )
			return;

		CheckWebhookType( type );

		auto msg = m_Webhook.CreateDiscordMessageAdmin( player, "Player:", color );

		AddWebhookBody( msg, type, message, details );

		m_Webhook.PostAs( GetModuleName() + type, msg, player );
	}

	//! What was done: the module's title field with the message, the optional details, and the
	//! module / action as the footer.
	protected void AddWebhookBody( JMWebhookDiscordMessage msg, string type, string message, string details = "" )
	{
		auto embed = msg.GetEmbed();

		embed.AddField( GetWebhookTitle(), message, false );

		if ( details != "" )
			embed.AddField( "Details", details, false );

		embed.SetFooter( GetWebhookTitle() + " - " + type );
	}

	//! Declare this module's permissions. Called on BOTH client and server at registration, before
	//! the mission loads (the permission sync needs that). Override and call JMPermissions.Register().
	//! The base declares the view permission and all those named by JMModuleInfo and its actions;
	//! an override must call super to keep them.
	void DeclarePermissions()
	{
		JMModuleInfo info = GetModuleInfo();

		if ( info.ViewPermission != "" )
			JMPermissions.Register( info.ViewPermission );

		array< string > declared = info.GetPermissions();
		foreach ( string declaredPermission : declared )
			JMPermissions.Register( declaredPermission );

		map< int, typename > actionTypes = GetActionTypes();
		foreach ( int rpc, typename actionType : actionTypes )
		{
			JMModuleAction prototype = JMModuleAction.Cast( actionType.Spawn() );
			if ( prototype && prototype.GetPermission() != "" )
				JMPermissions.Register( prototype.GetPermission() );
		}
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();
	}

	void AddSubCommand(inout array<ref JMCommand> commands, string command, string function, string permission)
	{
		commands.Insert(new JMSubCommand(this, command, function, permission));
	}
}
