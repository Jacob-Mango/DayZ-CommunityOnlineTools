class JMSelectedObjects
{
	ref array< EntityAI > objects;
	ref array< string > players;

	void JMSelectedObjects()
	{
		objects = new array< EntityAI >;
		players = new array< string >;
	}

	void ~JMSelectedObjects()
	{
		delete objects;
		delete players;
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