class PlayerData
{
    Man player;

    string Name;
    string GUID;

    void PlayerData( Man p, PlayerIdentity i )
    {
        player = p;
        Name = i.GetName();
        GUID = i.GetId();
    }

    string GetGUID()
    {
        return GUID;
    }

    string GetName()
    {
        return Name;
    }
}