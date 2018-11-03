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

    void AddPermission( string inp )
    {
        array<string> tokens = new array<string>;
        inp.Split( ".", tokens );
        
        if ( tokens.Count() == 0 )
        {
            VerifyAddPermission( inp );
        } else
        {
            int depth = tokens.Find( Name );

            if ( depth > -1 )
            {
                AddPermissionInternal( tokens, depth + 1 );
            } else 
            {
                AddPermissionInternal( tokens, 0 );
            }
        }
    }

    private void AddPermissionInternal( array<string> tokens, int depth )
    {
        if ( tokens.Count() == depth )
        {
            VerifyAddPermission( tokens[depth - 1] );
        }

        bool permissionExisted = false;

        for ( int i = 0; i < Children.Count(); i++ )
        {
            if ( tokens[depth] == Children[i].Name )
            {
                AddPermissionInternal( tokens, depth + 1 );
                permissionExisted = true;
                break;
            }
        }

        if ( permissionExisted == false )
        {
            VerifyAddPermission( tokens[depth] );
            AddPermissionInternal( tokens, depth + 1 );
        }
    }

    private void VerifyAddPermission( string token )
    {
        bool permissionExisted = false;

        for ( int i = 0; i < Children.Count(); i++ )
        {
            if ( token == Children[i].Name )
            {
                permissionExisted = true;
                break;
            }
        }
        
        if ( permissionExisted )
        {
            Children.Insert( new Permission( token, this ) );
        }
    }

    // This (RemovePermission) most likely doesn't work at all...
    void RemovePermission( string inp )
    {
        int i = 0;

        array<string> tokens = new array<string>;
        inp.Split( ".", tokens );

        if ( tokens.Count() == 0 )
        {
            for ( i = 0; i < Children.Count(); i++ )
            {
                if ( inp == Children[i].Name )
                {
                    Children.Remove( i );
                    break;
                }
            }
        } else 
        {
            for ( i = 0; i < Children.Count(); i++ )
            {
                if ( tokens[tokens.Count() - 1] == Children[i].Name )
                {
                    Children.Remove( i );
                    break;
                }
            }
        }
    }

    bool CheckPermission( string inp )
    {
        array<string> tokens = new array<string>;
        inp.Split( ".", tokens );

        if ( tokens.Count() == 0 ) return false;
        
        int depth = tokens.Find(Name);

        if ( depth > -1 )
        {
            return Check( tokens, depth + 1 );
        }

        return false;
    }

    bool Check( array<string> tokens, int depth )
    {
        if ( tokens.Count() == depth )
        {
            return true;
        }

        for ( int i = 0; i < Children.Count(); i++ )
        {
            if ( tokens[depth] == Children[i].Name )
            {
                return Check( tokens, depth + 1 );
            }
        }
        return false;
    }

    void Serialize( ref array< string > output, string prepend = "" )
    {
        if ( prepend == "" )
        {
            prepend = Name;
        }

        for ( int i = 0; i < Children.Count(); i++ )
        {
            string serialize = prepend + "." + Children[i].Name;

            output.Insert( serialize );

            Serialize( output, serialize );
        }
    }
}