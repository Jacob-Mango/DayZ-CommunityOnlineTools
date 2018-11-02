class Role
{
    protected ref array< ref Permission > m_Permissions;

    void Role()
    {

    }

    void ~Role()
    {

    }

    void AddPermission( ref Permission perm )
    {
        ref Permission parent = perm.Parent;

        while ( parent != NULL )
        {
            parent = perm.Parent;
        }
    }
}