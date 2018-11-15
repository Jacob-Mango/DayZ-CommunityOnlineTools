static ref AuthPlayer ClientAuthPlayer;

static ref array< ref AuthPlayer > SELECTED_PLAYERS;

ref array< ref AuthPlayer > GetSelectedPlayers()
{
    if ( SELECTED_PLAYERS == NULL )
    {
        SELECTED_PLAYERS = new ref array< ref AuthPlayer >;
    }
    return SELECTED_PLAYERS;
}

int AddSelectedPlayer( ref AuthPlayer player )
{
    int position = GetSelectedPlayers().Find( player );
    
    if ( position > -1 )
        return position;

    return GetSelectedPlayers().Insert( player );
}

ref array< ref PlayerData > SerializePlayers( ref array< ref AuthPlayer > players )
{
    ref array< ref PlayerData > output = new ref array< ref PlayerData >;
    for ( int i = 0; i < players.Count(); i++)
    {
        players[i].Serialize();

        output.Insert( players[i].GetData() );
    }
    return output;
}

ref array< ref AuthPlayer > DeserializePlayers( ref array< ref PlayerData > players )
{
    ref array< ref AuthPlayer > output = new ref array< ref AuthPlayer >;

    for ( int i = 0; i < players.Count(); i++)
    {
        ref AuthPlayer player = GetPermissionsManager().GetPlayer( players[i] );
        player.Deserialize();
        output.Insert( player );
    }

    return output;
}