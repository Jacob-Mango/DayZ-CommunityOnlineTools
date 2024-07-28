class JMWebhookConnectionGroup : Managed
{
	string Name;
	string ContextURL;
	string Address;

	ref array< ref JMWebhookConnection > Types;

	void JMWebhookConnectionGroup()
	{
		Name = "Main";
		Types = new array< ref JMWebhookConnection >;
	}

	void Init()
	{
		for ( int i = 0; i < Types.Count(); ++i )
		{
			Types[i].m_Group = this;
		}
	}

	JMWebhookConnection Add( string name )
	{
		for ( int i = 0; i < Types.Count(); ++i )
		{
			if ( Types[i].Name != name )
				continue;

			return Types[i];
		}

		#ifdef JM_COT_WEBHOOK_DEBUG
		return Insert( name, true );
		#else
		return Insert( name, false );
		#endif
	}

	private JMWebhookConnection Insert( string name, bool enabled )
	{
		JMWebhookConnection type = new JMWebhookConnection();

		type.Name = name;
		type.Enabled = enabled;
		type.m_Group = this;

		Types.Insert( type );

		return type;
	}

	JMWebhookConnection Set( string name, bool enabled )
	{
		for ( int i = 0; i < Types.Count(); ++i )
		{
			if ( Types[i].Name != name )
				continue;

			Types[i].Enabled = enabled;
			return Types[i];
		}

		return Insert( name, enabled );
	}

	void Remove( string name )
	{
		for ( int i = 0; i < Types.Count(); ++i )
		{
			if ( Types[i].Name != name )
				continue;

			Types.Remove( i );
			return;
		}
	}

	bool Contains( string name )
	{
		for ( int i = 0; i < Types.Count(); ++i )
		{
			if ( Types[i].Name != name )
				continue;

			Types.Remove( i );
			return true;
		}

		return false;
	}

	JMWebhookConnection Get( int index )
	{
		return Types[index];
	}

	int Count()
	{
		return Types.Count();
	}
};