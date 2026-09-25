#ifdef JM_CommunityOnlineTools
// Example: dispatching custom webhook events to Discord via JMWebhookModule.
modded class JMWebhookModule
{
	void SendSubModNotification( string title, string message )
	{
		if ( !g_Game.IsServer() )
			return;

		CreateNotification( title, message );
	}
}
#endif
