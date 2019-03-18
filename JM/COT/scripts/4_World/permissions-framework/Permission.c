class Permission
{
    ref Permission Parent;

    ref array< ref Permission > Children;

    string Name;

    PermissionType Type;

    void Permission( string name, ref Permission parent = NULL )
    {
        Name = name;
        Parent = parent;

        Type = PermissionType.INHERIT;

        if ( Parent == NULL )
        {
            Type = PermissionType.DISALLOW;
        }

        Children = new ref array< ref Permission >;
    }

    void ~Permission()
    {
        delete Children;
    }

    string GetFullName()
    {
        string permission = Name;
        ref Permission parent = Parent;

        while ( parent != NULL )
        {
            if ( parent.Parent == NULL ) 
            {
                return permission;
            }
            
            permission = parent.Name + "." +  permission;
            parent = parent.Parent;
        }

        return permission;
    }

    void AddPermission( string inp, PermissionType permType = PermissionType.INHERIT )
    {
        array<string> tokens = new array<string>;

        array<string> spaces = new array<string>;
        inp.Split( " ", spaces );

        PermissionType type;

        if ( permType == PermissionType.INHERIT && spaces.Count() == 2 )
        {
            if ( spaces[1].Contains( "2" ) )
            {
                type = PermissionType.ALLOW;
            } else if ( spaces[1].Contains( "1" ) )
            {
                type = PermissionType.DISALLOW;
            } else 
            {
                type = PermissionType.INHERIT;
            }

            spaces[0].Split( ".", tokens );
        } else if ( spaces.Count() < 2 )
        {
            type = permType;

            inp.Split( ".", tokens );
        } else {
            Print( "Warning, permission line improperly formatted! Read as \"" + inp + "\" but meant to be in format \"Perm.Perm {n}\"." );
            return;
        }
        
        int depth = tokens.Find( Name );

        if ( depth > -1 )
        {
            AddPermissionInternal( tokens, depth + 1, type );
        } else 
        {
            AddPermissionInternal( tokens, 0, type );
        }
    }

    private void AddPermissionInternal( array<string> tokens, int depth, PermissionType value )
    {
        if ( depth < tokens.Count() )
        {
            string name = tokens[depth];

            ref Permission nChild = VerifyAddPermission( name );

            nChild.AddPermissionInternal( tokens, depth + 1, value );
        } else {
            Type = value;
        }
    }

    private ref Permission VerifyAddPermission( string name )
    {
        ref Permission nChild = NULL;

        for ( int i = 0; i < Children.Count(); i++ )
        {
            if ( name == Children[i].Name )
            {
                nChild = Children[i];
                break;
            }
        }
            
        if ( nChild == NULL )
        {
            nChild = new Permission( name, this );
            nChild.Type = PermissionType.INHERIT;

            Children.Insert( nChild );
        }

        return nChild;
    }

    ref Permission GetPermission( string inp )
    {
        array<string> tokens = new array<string>;
        inp.Split( ".", tokens );
        
        int depth = tokens.Find(Name);

        if ( depth > -1 )
        {
            return Get( tokens, depth + 1 );
        } else 
        {
            return Get( tokens, 0 );
        }
    }

    private ref Permission Get( array<string> tokens, int depth )
    {
        if ( depth < tokens.Count() )
        {
            ref Permission nChild = NULL;

            for ( int i = 0; i < Children.Count(); i++ )
            {
                if ( Children[i].Name == tokens[depth] )
                {
                    nChild = Children[i]; 
                }
            }

            if ( nChild )
            {
                return nChild.Get( tokens, depth + 1 );
            }
        }

        return this;
    }

    bool HasPermission( string inp, out PermissionType permType )
    {
        array<string> tokens = new array<string>;
        inp.Split( ".", tokens );

        if ( tokens.Count() == 0 ) return false;
        
        int depth = tokens.Find(Name);

        bool parentDisallowed = false;

        if ( Type == PermissionType.DISALLOW )
        {
            parentDisallowed = true;
        } else if ( Type == PermissionType.INHERIT )
        {
            // magic fuckery to figure this out...
        }

        if ( depth > -1 )
        {
            return Check( tokens, depth + 1, parentDisallowed, permType );
        } else 
        {
            return Check( tokens, 0, parentDisallowed, permType );
        }
    }

    bool Check( array<string> tokens, int depth, bool parentDisallowed, out PermissionType permType )
    {
        bool ifReturnAs = false;

        if ( Type == PermissionType.ALLOW )
        {
            ifReturnAs = true;
        }

        if ( Type == PermissionType.INHERIT && parentDisallowed == false )
        {
            ifReturnAs = true;
        }

        if ( Type == PermissionType.DISALLOW )
        {
            parentDisallowed = true;
        }

        if ( Type == PermissionType.ALLOW )
        {
            parentDisallowed = false;
        }

        if ( depth < tokens.Count() )
        {
            ref Permission nChild = NULL;

            for ( int i = 0; i < Children.Count(); i++ )
            {
                if ( Children[i].Name == tokens[depth] )
                {
                    nChild = Children[i]; 
                }
            }

            if ( nChild )
            {
                return nChild.Check( tokens, depth + 1, parentDisallowed, permType );
            }
        }

        permType = Type;

        return ifReturnAs;
    }

    void Serialize( ref array< string > output, string prepend = "" )
    {
        for ( int i = 0; i < Children.Count(); i++ )
        {
            string serialize = prepend + Children[i].Name;
                
            output.Insert( serialize + " " + Children[i].Type );

            if ( Children[i].Children.Count() > 0 ) 
            {
                Children[i].Serialize( output, serialize + "." );
            }
        }
    }

    void DebugPrint( int depth = 0 )
    {
        string message = this.Name;

        for ( int j = 0; j < depth; j++ )
        {
            message = "  " + message;
        }

        string append = "";

        switch ( Type )
        {
            case PermissionType.ALLOW:
            {
                append = " ALLOW";
                break;
            }
            case PermissionType.DISALLOW:
            {
                append = " DISALLOW";
                break;
            }
            default:
            case PermissionType.INHERIT:
            {
                append = " INHERIT";
                break;
            }
        }

        Print( message + append );

        for ( int i = 0; i < Children.Count(); i++ )
        {
            Children[i].DebugPrint( depth + 1 );
        }
    }
}