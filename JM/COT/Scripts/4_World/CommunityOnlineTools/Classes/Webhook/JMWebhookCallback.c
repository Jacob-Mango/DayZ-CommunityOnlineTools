class JMWebhookCallback: RestCallback
{
	void JMWebhookCallback()
	{
	}
	
	override void OnSuccess( string data, int dataSize )
	{
		#ifdef JM_COT_WEBHOOK_DEBUG
		Print( "OnSuccess - " + data + " - " + dataSize );
		#endif
	}
	
	override void OnError( int errorCode )
	{
		if ( errorCode < ERestResultState.EREST_ERROR )
			return;

		Print( "OnError - " + errorCode );
	}
	
	override void OnTimeout()
	{
		#ifdef JM_COT_WEBHOOK_DEBUG
		Print( "OnTimeout" );
		#endif
	}
};