class JMWebhookCallback: RestCallback
{
	void JMWebhookCallback()
	{
	}
	
	override void OnSuccess( string data, int dataSize )
	{
		Print( "OnSuccess - " + data + " - " + dataSize );
	}
	
	override void OnError( int errorCode )
	{
		Print( "OnError - " + errorCode );
	}
	
	override void OnTimeout()
	{
		Print( "OnTimeout" );
	}
};