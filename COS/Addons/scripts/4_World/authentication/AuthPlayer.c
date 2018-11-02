class AuthPlayer
{
    protected string m_GUID;

    protected ref array< ref Role > m_Roles;
    protected ref array< ref Permission > m_Permissions;

    void AuthPlayer( string guid )
    {
        m_GUID = guid;

        LoadData();
    }

    void LoadData()
    {

    }
}