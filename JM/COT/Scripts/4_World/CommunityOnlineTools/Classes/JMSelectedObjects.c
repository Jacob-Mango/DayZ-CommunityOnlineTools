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
	private ref set< Object > m_Objs;
	private ref array< string > m_Players;

	void JMSelectedObjects()
	{
		m_Objects = new set< ref JMSelectedObject >;
		m_Objs = new set< Object >;
		m_Players = new array< string >;

		JMScriptInvokers.ADD_OBJECT.Insert( AddObject );
		JMScriptInvokers.REMOVE_OBJECT.Insert( _RemoveObject );
	}

	void ~JMSelectedObjects()
	{
		JMScriptInvokers.ADD_OBJECT.Remove( AddObject );
		JMScriptInvokers.REMOVE_OBJECT.Remove( _RemoveObject );
	}

	bool IsObjectSelected( notnull Object obj )
	{
		if (m_Objs.Find( obj ) > -1)
			return true;

		return false;
	}

	void AddObject( Object obj )
	{
		if (m_Objs.Find( obj ) == -1)
		{
			m_Objects.Insert( new JMSelectedObject( obj ) );
			m_Objs.Insert( obj );
		}
	}

	private void _RemoveObject( Object obj, int netLow, int netHigh )
	{
		if (obj)
			RemoveObject( obj );
		else
			RemoveObjectEx( netLow, netHigh );
	}

	void ClearObjects()
	{
		m_Objects.Clear();
		m_Objs.Clear();
	}

	bool RemoveObject( notnull Object obj )
	{
		int index = m_Objs.Find( obj );
		
		if (index > -1)
		{
			m_Objects.Remove( index );
			m_Objs.Remove( index );
			return true;
		}

		return false;
	}

	bool RemoveObjectEx( int netLow, int netHigh )
	{
		foreach ( int i, JMSelectedObject selectedObj: m_Objects )
		{
			if ( selectedObj.Equals( netLow, netHigh ) )
			{
				m_Objects.Remove( i );
				m_Objs.Remove( i );
				return true;
			}
		}
		return false;
	}

	void SerializeObjects( ParamsWriteContext ctx )
	{
		set<ref JMSelectedObject> objects = new set<ref JMSelectedObject>;

		PlayerBase player;
		foreach (JMSelectedObject selectedObj: m_Objects)
		{
			if (!Class.CastTo(player, selectedObj.obj) || !player.GetIdentity())
				objects.Insert(selectedObj);
		}
		
		int count = objects.Count();
		ctx.Write(count);

		foreach (JMSelectedObject obj: objects)
		{
			ctx.Write(obj.networkLow);
			ctx.Write(obj.networkHigh);
		}
	}

	bool DeserializeObjects( ParamsReadContext ctx, out set< Object > objects )
	{
		int count;
		if ( !ctx.Read( count ) )
			return false;

		for ( int i = 0; i < count; ++i )
		{
			int netLow;
			int netHigh;
			if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
				return false;

			Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
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

	int NumPlayers(bool autoSelect = true)
	{
		return GetPlayers(autoSelect).Count();
	}

	array< string > GetPlayers(bool autoSelect = true)
	{
		if (autoSelect && !m_Players.Count())
		{
			if (IsMissionOffline())
				m_Players.Insert(JMConstants.OFFLINE_GUID);
			else if (GetPermissionsManager().GetClientGUID())
				m_Players.Insert(GetPermissionsManager().GetClientGUID());
		}

		return m_Players;
	}

	array< string > GetPlayersOrSelf()
	{
		return GetPlayers(true);
	}

	set< ref JMSelectedObject > GetObjects()
	{
		return m_Objects;
	}

	void ClearPlayers()
	{
		m_Players.Clear();
	}
}

static ref JMSelectedObjects g_cot_selected;

static JMSelectedObjects JM_GetSelected()
{
	if ( g_cot_selected == NULL )
		g_cot_selected = new JMSelectedObjects;
		
	return g_cot_selected;
}