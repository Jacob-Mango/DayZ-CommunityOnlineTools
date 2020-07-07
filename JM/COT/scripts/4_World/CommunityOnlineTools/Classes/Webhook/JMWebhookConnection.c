class JMWebhookConnection: Managed
{
    private RestContext m_Context;

    string Name;
    string ContextURL;
    string Address;

    bool Enabled;

	void Post( RestApi core, notnull ref JsonSerializer serializer, notnull ref JMWebhookMessage message )
	{
        if ( Enabled )
        {
            if ( !m_Context )
            {
                m_Context = core.GetRestContext( ContextURL );
            }

            string data = message.Prepare( serializer );

            m_Context.SetHeader( "application/json" );
            m_Context.POST( new JMWebhookCallback, Address, data );
        }
	}

    string DebugString()
    {
        string str = "";
        if ( m_Context )
            str += "CTX CREATED";
        else
            str += "CTX NOT CREATED";

        str += ":";
        str += Name;
        str += ":";
        str += ContextURL;
        str += ":";
        str += Address;
        str += ":";
        if ( Enabled )
            str += "TRUE";
        else 
            str += "FALSE";

        return str;
    }
};