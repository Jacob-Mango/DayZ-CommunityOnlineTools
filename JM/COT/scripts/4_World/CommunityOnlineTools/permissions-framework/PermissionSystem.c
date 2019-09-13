static ref array< ref JMPlayerInstance > JM_SELECTED_PLAYERS;

ref array< ref JMPlayerInstance > GetSelectedPlayers()
{
	if ( JM_SELECTED_PLAYERS == NULL )
	{
		JM_SELECTED_PLAYERS = new array< ref JMPlayerInstance >;
	}
	return JM_SELECTED_PLAYERS;
}

bool PlayerAlreadySelected( ref JMPlayerInstance player )
{
	int position = GetSelectedPlayers().Find( player );

	return position > -1;
}

int RemoveSelectedPlayer( ref JMPlayerInstance player )
{
	if ( !player )
	{
		return -1;
	}

	int position = GetSelectedPlayers().Find( player );

	if ( position > -1 )
	{
		GetSelectedPlayers().Remove( position );
	}

	return position;
}

int AddSelectedPlayer( ref JMPlayerInstance player )
{
	int position = GetSelectedPlayers().Find( player );
	
	if ( position > -1 )
		return position;

	return GetSelectedPlayers().Insert( player );
}

array< string > GetSelectedAsGUIDs()
{
	ref array< string > output = new array< string >;

	for ( int i = 0; i < JM_SELECTED_PLAYERS.Count(); i++)
	{
		output.Insert( JM_SELECTED_PLAYERS[i].GetGUID() );
	}

	return output;
}