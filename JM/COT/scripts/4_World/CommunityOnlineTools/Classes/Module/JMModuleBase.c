modded class JMModuleBase
{
	protected JMWebhookModule m_Webhook;

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
		Print( "+JMModuleBase::SendWebhook() - Admin" );
		if ( !m_Webhook || !player || IsMissionOffline() )
			return;

        auto msg = m_Webhook.CreateDiscordMessage( player, "Admin Account: " );

        msg.GetEmbed().AddField( GetWebhookTitle(), message, false );

		Print( "name=" + GetModuleName() + type );
		Print( "message=" + message );

        m_Webhook.Post( GetModuleName() + type, msg );
		Print( "-JMModuleBase::SendWebhook() - Admin" );
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