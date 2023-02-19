class JMSelectedObject : Managed
{
	int networkLow;
	int networkHigh;

	Object obj;

	void JMSelectedObject( notnull Object object )
	{
		obj = object;
		obj.GetNetworkID( networkLow, networkHigh );
	}

	bool Equals( int netLow, int netHigh )
	{
		if ( networkLow != netLow )
			return false;
		if ( networkHigh != netHigh )
			return false;

		return true;
	}

	bool Equals( notnull Object object )
	{
		if ( obj == NULL )
		{
			int netLow;
			int netHigh;
			object.GetNetworkID( netLow, netHigh );

			if ( networkLow != netLow )
				return false;
			if ( networkHigh != netHigh )
				return false;

			obj = object;
			return true;
		}

		return obj == object;
	}
}

class JMSelectedObjects
{
	private ref set< ref JMSelectedObject > m_Objects;
	private ref array< string > m_Players;

	void JMSelectedObjects()
	{
		m_Objects = new set< ref JMSelectedObject >;
		m_Players = new array< string >;

		JMScriptInvokers.ADD_OBJECT.Insert( AddObject );
		JMScriptInvokers.REMOVE_OBJECT.Insert( _RemoveObject );
	}

	void ~JMSelectedObjects()
	{
		delete m_Objects;
		delete m_Players;

		JMScriptInvokers.ADD_OBJECT.Remove( AddObject );
		JMScriptInvokers.REMOVE_OBJECT.Remove( _RemoveObject );
	}

	bool IsObjectSelected( notnull Object obj )
	{
		for ( int i = 0; i < m_Objects.Count(); ++i )
		{
			if ( m_Objects[i].Equals( obj ) )
			{
				return true;
			}
		}

		return false;
	}

	void AddObject( Object obj )
	{
		m_Objects.Insert( new JMSelectedObject( obj ) );
	}

	private void _RemoveObject( Object obj )
	{
		RemoveObject( obj );
	}

	void ClearObjects()
	{
		m_Objects.Clear();
	}

	bool RemoveObject( notnull Object obj )
	{
		for ( int i = 0; i < m_Objects.Count(); ++i )
		{
			if ( m_Objects[i].Equals( obj ) )
			{
				m_Objects.Remove( i );
				return true;
			}
		}
		return false;
	}

	void SerializeObjects( ParamsWriteContext ctx )
	{
		int count = m_Objects.Count();
		ctx.Write( count );

		for ( int i = 0; i < count; ++i )
		{
			ctx.Write( m_Objects[i].networkLow );
			ctx.Write( m_Objects[i].networkHigh );
		}
	}

	bool DeserializeObjects( ParamsReadContext ctx, out set< Object > objects )
	{
		int count;
		if ( !ctx.Read( count ) )
			return false;

		for ( int i = 0; i < count; ++i )
		{
			/*
			Object obj;
			if ( !ctx.Read( obj ) )
				return false;

			objects.Insert( obj );
			*/

			int netLow;
			int netHigh;
			if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
				return false;

			Object obj = GetGame().GetObjectByNetworkId( netLow, netHigh );
			if ( obj )
				objects.Insert( obj );
		}

		return true;
	}

	bool IsSelected( string guid )
	{
		return m_Players.Find( guid ) > -1;
	}

	int GetPlayer( string guid )
	{
		return m_Players.Find( guid );
	}

	int AddPlayer( string guid )
	{
		int idx = m_Players.Find( guid );
		if ( idx > -1 )
			return idx;

		return m_Players.Insert( guid );
	}

	int RemovePlayer( string guid )
	{
		int idx = m_Players.Find( guid );

		if ( idx > -1 )
			m_Players.Remove( idx );

		return idx;
	}

	int NumPlayers()
	{
		return m_Players.Count();
	}

	array< string > GetPlayers()
	{
		return m_Players;
	}

	void ClearPlayers()
	{
		m_Players.Clear();
	}
};

static ref JMSelectedObjects g_cot_selected;

static JMSelectedObjects JM_GetSelected()
{
	//if ( !IsMissionClient() )
	//	return NULL;

	if ( g_cot_selected == NULL )
		g_cot_selected = new JMSelectedObjects;
		
	return g_cot_selected;
}