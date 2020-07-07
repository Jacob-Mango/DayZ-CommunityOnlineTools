class JMWebhookConnection: Managed
{
	[NonSerialized()]
    private RestContext m_Context;

    string Name;
    string ContextURL;
    string Address;

    bool Enabled;

    void Init( string ctxUrl, RestApi core )
    {
        if ( ContextURL != "" ) 
            ctxUrl = ContextURL;

        Print( ctxUrl );
        m_Context = core.GetRestContext( ctxUrl );
    }

	void Post( string adr, notnull ref JsonSerializer serializer, notnull ref JMWebhookMessage message )
	{
        if ( Enabled )
        {
            if ( Address != "" ) 
                adr = Address;

            string data = message.Prepare( serializer );
            Print( data );
            Print( adr );
            m_Context.SetHeader( "application/json" );
            m_Context.POST( new JMWebhookCallback, adr, data );
        }
	}
};