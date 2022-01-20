modded class CF_ModuleWorld
{
	protected JMWebhookModule m_Webhook;

	override void OnInit()
	{
		super.OnInit();
		
		m_Webhook = CF_Modules<JMWebhookModule>.Get();
	}

	void SendWebhook( string type, string message )
	{
		if ( !m_Webhook || IsMissionOffline() )
			return;

		auto msg = m_Webhook.CreateDiscordMessage();

		msg.GetEmbed().AddField( GetWebhookTitle(), message, false );

		m_Webhook.Post( ClassName() + type, msg );
	}

	void SendWebhook( string type, JMPlayerInstance player, string message )
	{
		if ( !m_Webhook || !player || IsMissionOffline() )
			return;

		auto msg = m_Webhook.CreateDiscordMessage( player, "Admin Account: " );

		msg.GetEmbed().AddField( GetWebhookTitle(), message, false );

		m_Webhook.Post( ClassName() + type, msg );
	}

	string GetWebhookTitle()
	{
		return "Unknown";
	}

	void GetWebhookTypes( out array< string > types )
	{
		
	}

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
};
