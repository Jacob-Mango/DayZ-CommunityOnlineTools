#ifdef JM_CommunityOnlineTools
// Example: dispatching custom webhook events to Discord via JMWebhookModule.
modded class JMWebhookModule
{
	void SendSubModNotification( string title, string message )
	{
		if ( !g_Game.IsServer() )
			return;

		auto payload = CreateDiscordMessageColored( JMConstants.WEBHOOK_COLOR_DEFAULT );
		payload.GetEmbed().SetTitle( title );
		payload.GetEmbed().SetDescription( message );

		// "ServerStartup" is one of JMWebhookConstructor's built-in connection types (Discord/Settings tab);
		// a real sub-mod would add its own type there and route through that name instead.
		Post( "ServerStartup", payload );
	}
}
#endif
