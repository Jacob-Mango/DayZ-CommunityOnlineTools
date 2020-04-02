static ref array< string > JM_SELECTED_PLAYERS = new array< string >;

array< string > GetSelectedPlayers()
{
	return JM_SELECTED_PLAYERS;
}

bool PlayerAlreadySelected( string guid )
{
	return JM_SELECTED_PLAYERS.Find( guid ) > -1;
}

int PlayerSelectedIndex( string guid )
{
	return JM_SELECTED_PLAYERS.Find( guid );
}

void ClearSelectedPlayers()
{
	JM_SELECTED_PLAYERS.Clear();
}

int CountPlayersSelected()
{
	return JM_SELECTED_PLAYERS.Count();
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
		// Print( guid + " was already selected..." );
		return position;
	}

	return JM_SELECTED_PLAYERS.Insert( guid );
}