static ref JMPlayerInstance ClientAuthPlayer;

static ref array< ref JMPlayerInstance > SELECTED_PLAYERS;

ref array< ref JMPlayerInstance > GetSelectedPlayers()
{
	if ( SELECTED_PLAYERS == NULL )
	{
		SELECTED_PLAYERS = new array< ref JMPlayerInstance >;
	}
	return SELECTED_PLAYERS;
}

bool PlayerAlreadySelected( ref JMPlayerInstance player )
{
	int position = GetSelectedPlayers().Find( player );

	return position > -1;
}

int RemoveSelectedPlayer( ref JMPlayerInstance player )
{
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

ref JMPlayerInformation SerializePlayer( ref JMPlayerInstance player )
{
	player.Serialize();

	return player.Data;
}

ref JMPlayerInstance DeserializePlayer( ref JMPlayerInformation data )
{
	return GetPermissionsManager().GetPlayer( data );
}

array< ref JMPlayerInformation > SerializePlayers( ref array< ref JMPlayerInstance > players )
{
	array< ref JMPlayerInformation > output = new array< ref JMPlayerInformation >;

	for ( int i = 0; i < players.Count(); i++)
	{
		output.Insert( SerializePlayer( players[i] ) );
	}

	return output;
}

array< ref JMPlayerInstance > DeserializePlayers( ref array< ref JMPlayerInformation > players )
{
	array< ref JMPlayerInstance > output = new array< ref JMPlayerInstance >;

	for ( int i = 0; i < players.Count(); i++)
	{
		output.Insert( DeserializePlayer( players[i] ) );
	}

	return output;
}

ref array< string > SerializePlayersID( array< ref JMPlayerInstance > players )
{
	ref array< string > output = new array< string >;

	for ( int i = 0; i < players.Count(); i++)
	{
		output.Insert( players[i].GetSteam64ID() );
	}

	return output;
}

array< ref JMPlayerInstance > DeserializePlayersID( ref array< string > steam64Ids )
{
	return GetPermissionsManager().GetPlayersFromArray( steam64Ids );
}

ref JMPlayerInstance GetPlayerForID( string steam )
{
	return GetPermissionsManager().GetPlayerBySteam64ID( steam );
}