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
	private ref set< ref JMSelectedObject > objects;
	private ref array< string > players;

	void JMSelectedObjects()
	{
		objects = new set< ref JMSelectedObject >;
		players = new array< string >;

		JMScriptInvokers.ADD_OBJECT.Insert( AddObject );
		JMScriptInvokers.REMOVE_OBJECT.Insert( _RemoveObject );
	}

	void ~JMSelectedObjects()
	{
		delete objects;
		delete players;

		JMScriptInvokers.ADD_OBJECT.Remove( AddObject );
		JMScriptInvokers.REMOVE_OBJECT.Remove( _RemoveObject );
	}

	bool IsObjectSelected( notnull Object obj )
	{
		for ( int i = 0; i < objects.Count(); ++i )
		{
			if ( objects[i].Equals( obj ) )
			{
				return true;
			}
		}

		return false;
	}

	void AddObject( Object obj )
	{
		objects.Insert( new JMSelectedObject( obj ) );
	}

	private void _RemoveObject( Object obj )
	{
		RemoveObject( obj );
	}

	void ClearObjects()
	{
		objects.Clear();
	}

	bool RemoveObject( notnull Object obj )
	{
		for ( int i = 0; i < objects.Count(); ++i )
		{
			if ( objects[i].Equals( obj ) )
			{
				objects.Remove( i );
				return true;
			}
		}
		return false;
	}

	void SerializeObjects( ParamsWriteContext ctx )
	{
		int count = objects.Count();
		ctx.Write( count );

		for ( int i = 0; i < count; ++i )
		{
			ctx.Write( objects[i].networkLow );
			ctx.Write( objects[i].networkHigh );
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
		return players.Find( guid ) > -1;
	}

	int GetPlayer( string guid )
	{
		return players.Find( guid );
	}

	int AddPlayer( string guid )
	{
		int idx = players.Find( guid );
		if ( idx > -1 )
			return idx;

		return players.Insert( guid );
	}

	int RemovePlayer( string guid )
	{
		int idx = players.Find( guid );

		if ( idx > -1 )
			players.Remove( idx );

		return idx;
	}

	int NumPlayers()
	{
		return players.Count();
	}

	array< string > GetPlayers()
	{
		return players;
	}

	void ClearPlayers()
	{
		players.Clear();
	}
};

static ref JMSelectedObjects g_cot_selected;

static ref JMSelectedObjects JM_GetSelected()
{
	//if ( !IsMissionClient() )
	//	return NULL;

	if ( g_cot_selected == NULL )
		g_cot_selected = new JMSelectedObjects;
		
	return g_cot_selected;
}