modded class JMModuleBase
{
	private JMWebhookModule m_Webhook;

	override void OnMissionStart()
	{
		super.OnMissionStart();
		
		Class.CastTo( m_Webhook, GetModuleManager().GetModule( JMWebhookModule ) );
	}

	void SendWebhook( JMPlayerInstance player, string message )
	{
		if ( !m_Webhook || !player )
			return;

        auto msg = m_Webhook.CreateDiscordMessage( player, "Admin Account: " );

        msg.GetEmbed().AddField( GetWebhookTitle(), message, false );

        m_Webhook.Post( GetModuleName(), msg );
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