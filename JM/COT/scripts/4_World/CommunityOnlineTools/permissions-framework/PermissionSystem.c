static ref set< string > JM_SELECTED_PLAYERS = new set< string >;

set< string > GetSelectedPlayers()
{
	return JM_SELECTED_PLAYERS;
}

bool PlayerAlreadySelected( string guid )
{
	return JM_SELECTED_PLAYERS.Find( guid ) > -1;
}

int RemoveSelectedPlayer( string guid )
{
	int position = JM_SELECTED_PLAYERS.Find( guid );

	if ( position > -1 )
	{
		JM_SELECTED_PLAYERS.Remove( position );
	}

	return position;
}

int AddSelectedPlayer( string guid )
{
	int position = JM_SELECTED_PLAYERS.Find( guid );
	
	if ( position > -1 )
	{
		return position;
	}

	return JM_SELECTED_PLAYERS.Insert( guid );
}

array< string > GetSelectedAsGUIDs()
{
	ref array< string > output = new array< string >;

	for ( int i = 0; i < JM_SELECTED_PLAYERS.Count(); i++)
	{
		output.Insert( JM_SELECTED_PLAYERS[i] );
	}

	return output;
}