class JMWebhookSerialize : Managed
{
	ref array< ref JMWebhookConnectionGroup > Connections;

	void JMWebhookSerialize()
	{
		Connections = new array< ref JMWebhookConnectionGroup >;
	}

	void Load()
	{
		JsonFileLoader< JMWebhookSerialize >.JsonLoadFile( JMConstants.FILE_WEBHOOK, this );
		
		for ( int i = 0; i < Connections.Count(); ++i )
		{
			Connections[i].Init();
		}
	}

	void Save()
	{
		JsonFileLoader< JMWebhookSerialize >.JsonSaveFile( JMConstants.FILE_WEBHOOK, this );
	}

	void OnSend( ParamsWriteContext ctx )
	{
		ctx.Write( Connections );
	}

	bool OnRecieve( ParamsReadContext ctx )
	{
		ctx.Read( Connections );

		return true;
	}

	bool Remove( string name )
	{
		for ( int i = 0; i < Connections.Count(); ++i )
		{
			if ( Connections[i].Name == name )
			{
				Connections.Remove( i );
				return true;
			}
		}

		return false;
	}

	JMWebhookConnectionGroup Get( string name )
	{
		for ( int i = 0; i < Connections.Count(); ++i )
		{
			if ( Connections[i].Name == name )
			{
				return Connections[i];
			}
		}

		JMWebhookConnectionGroup group = new JMWebhookConnectionGroup();
		group.Name = name;

		Connections.Insert( group );

		return group;
	}
};

static ref JMWebhookSerialize g_cot_Webhook;

static JMWebhookSerialize GetCOTWebhookSettings()
{
	if ( !g_cot_Webhook )
		g_cot_Webhook = new JMWebhookSerialize();

	return g_cot_Webhook;
}