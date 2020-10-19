modded class JMModuleBase
{
	protected JMWebhookModule m_Webhook;

	private array< ref JMCommand > m_Commands;
	private ref JMCommand m_LastCommand;

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

	string GetWebhookTitle()
	{
		return "Unknown";
	}

	void GetWebhookTypes( out array< string > types )
	{
		
	}

#ifndef CF_COT_MOVE
	void OnClientPermissionsUpdated()
	{
	}
#endif

	void GetCommands( inout array< ref JMCommand > commands )
	{
		m_Commands = commands;
		SetCommands();
	}

	void SetCommands()
	{

	}

	void AddCommand( string command, string function )
	{
		m_LastCommand = new JMCommand( command, function );
		m_Commands.Insert( m_LastCommand );
	}

	void AddParameter( JMCommandParameterType type, string name )
	{
		m_LastCommand.AddParameter( type, name );
	}

	private void FinishCommand()
	{
		
	}
}