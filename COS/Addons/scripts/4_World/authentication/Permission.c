class Permission
{
    ref Permission Parent;
    
    ref array< ref Permission > Children;

    string Name;

    void Permission( string name, ref Permission parent = NULL )
    {
        Name = name;
        Parent = parent;

        Children = new ref array< ref Permission >;
    }

    void ~Permission()
    {
        if ( Parent.Children.Find( this ) > -1 )
        {
            Parent.Children.RemoveItem( this );
        }
    }
}