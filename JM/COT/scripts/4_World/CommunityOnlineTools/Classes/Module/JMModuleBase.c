modded class JMModuleBase
{
	protected JMWebhookModule m_Webhook;

	override void OnMissionStart()
	{
		super.OnMissionStart();
		
		if ( IsMissionOffline() )
		{
			m_Webhook = NULL;
		} else
		{
			Class.CastTo( m_Webhook, GetModuleManager().GetModule( JMWebhookModule ) );
		}
	}

	void SendWebhook( string type, string message )
	{
		if ( !m_Webhook )
			return;

        auto msg = m_Webhook.CreateDiscordMessage();

        msg.GetEmbed().AddField( GetWebhookTitle(), message, false );

        m_Webhook.Post( GetModuleName() + type, msg );
	}

	void SendWebhook( string type, JMPlayerInstance player, string message )
	{
		if ( !m_Webhook || !player )
			return;

        auto msg = m_Webhook.CreateDiscordMessage( player, "Admin Account: " );

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

	void OnClientPermissionsUpdated()
	{
	}
}