static ref AuthPlayer ClientAuthPlayer;

static ref array< ref AuthPlayer > SELECTED_PLAYERS;

static bool PERRMISSIONS_FRAMEWORK_DEBUG_MODE_ENABLED = false;

ref array< ref AuthPlayer > GetSelectedPlayers()
{
    if ( SELECTED_PLAYERS == NULL )
    {
        SELECTED_PLAYERS = new ref array< ref AuthPlayer >;
    }
    return SELECTED_PLAYERS;
}

bool PlayerAlreadySelected( ref AuthPlayer player )
{
    int position = GetSelectedPlayers().Find( player );

    return position > -1;
}

int RemoveSelectedPlayer( ref AuthPlayer player )
{
	int position = GetSelectedPlayers().Find( player );

	if ( position > -1 )
	{
		GetSelectedPlayers().Remove( position );
	}

    return position;
}

int AddSelectedPlayer( ref AuthPlayer player )
{
    int position = GetSelectedPlayers().Find( player );
    
    if ( position > -1 )
        return position;

    return GetSelectedPlayers().Insert( player );
}

ref PlayerData SerializePlayer( ref AuthPlayer player )
{
    player.Serialize();

    return player.Data;
}

ref AuthPlayer DeserializePlayer( ref PlayerData data )
{
    return GetPermissionsManager().GetPlayer( data );
}

array< ref PlayerData > SerializePlayers( ref array< ref AuthPlayer > players )
{
    array< ref PlayerData > output = new array< ref PlayerData >;

    for ( int i = 0; i < players.Count(); i++)
    {
        output.Insert( SerializePlayer( players[i] ) );
    }

    return output;
}

array< ref AuthPlayer > DeserializePlayers( ref array< ref PlayerData > players )
{
    array< ref AuthPlayer > output = new array< ref AuthPlayer >;

    for ( int i = 0; i < players.Count(); i++)
    {
        output.Insert( DeserializePlayer( players[i] ) );
    }

    return output;
}

ref array< string > SerializePlayersID( array< ref AuthPlayer > players )
{
    ref array< string > output = new ref array< string >;

    for ( int i = 0; i < players.Count(); i++)
    {
        output.Insert( players[i].GetSteam64ID() );
    }

    return output;
}

array< ref AuthPlayer > DeserializePlayersID( ref array< string > steam64Ids )
{
    return GetPermissionsManager().GetPlayers( steam64Ids );
}