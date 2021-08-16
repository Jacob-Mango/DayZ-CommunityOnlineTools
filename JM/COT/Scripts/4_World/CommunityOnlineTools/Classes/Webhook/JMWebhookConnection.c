class JMWebhookConnection : Managed
{
	string Name;
	bool Enabled;
	
	[NonSerialized()]
	JMWebhookConnectionGroup m_Group;
	
	[NonSerialized()]
	RestContext m_Context;

	string GetContextURL()
	{
		return m_Group.ContextURL;
	}

	string GetAddress()
	{
		return m_Group.Address;
	}

	void Remove()
	{
		GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Call( m_Group.Remove, Name );
	}

	void Post( RestApi core, notnull JsonSerializer serializer, notnull JMWebhookMessage message )
	{
		#ifdef JM_COT_WEBHOOK_DEBUG
		Print( "+JMWebhookConnection::Post" );
		Print( "name=" + Name );
		Print( "this=" + this );
		Print( "group=" + m_Group );
		Print( "context=" + GetContextURL() );
		Print( "address=" + GetAddress() );
		Print( "enabled=" + Enabled );
		#endif
		
		if ( Enabled && !IsMissionClient() )
		{
			if ( !m_Context )
			{
				m_Context = core.GetRestContext( GetContextURL() );
			}

			string data = message.Prepare( serializer );

			#ifdef JM_COT_WEBHOOK_DEBUG
			Print( "data=" + data );
			#endif

			m_Context.SetHeader( "application/json" );
			m_Context.POST( new JMWebhookCallback, GetAddress(), data );
		}

		#ifdef JM_COT_WEBHOOK_DEBUG
		Print( "-JMWebhookConnection::Post" );
		#endif
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
		str += GetContextURL();
		str += ":";
		str += GetAddress();
		str += ":";
		if ( Enabled )
			str += "TRUE";
		else 
			str += "FALSE";

		return str;
	}
};