enum PermType 
{
    UNKNOWN, DISALLOWED, ALLOWED
}

class Permission
{
    ref Permission Parent;

    ref array< ref Permission > Children;

    string Name;

    bool Value;

    void Permission( string name, ref Permission parent = NULL )
    {
        Name = name;
        Parent = parent;

        Children = new ref array< ref Permission >;
    }

    void ~Permission()
    {
        delete Children;
    }

    void AddPermission( string inp, PermType type = PermType.UNKNOWN )
    {
        array<string> tokens = new array<string>;
        inp.Split( ".", tokens );

        array<string> spaces = new array<string>;
        inp.Split( " ", spaces );

        bool value = false;

        if ( type == PermType.UNKNOWN )
        {
            if ( spaces.Count() == 2 )
            {
                Print("Warning, permission line improperly formatted! Read as \"" + inp + "\" but meant to be in format \"Perm.Perm {n}\".");
                return;
            }

            if ( spaces[1].ToInt() == 1 )
            {
                value = true;
            }
        } else
        {
            if ( type == PermType.ALLOWED )
            {
                value = true;
            }
        }
        
        if ( tokens.Count() == 0 )
        {
            VerifyAddPermission( inp, value );
        } else
        {
            int depth = tokens.Find( Name );

            if ( depth > -1 )
            {
                AddPermissionInternal( tokens, depth + 1, value );
            } else 
            {
                AddPermissionInternal( tokens, 0, value );
            }
        }
    }

    private void AddPermissionInternal( array<string> tokens, int depth, bool value )
    {
        if ( depth < tokens.Count() )
        {
            string token = tokens[depth];

            ref Permission nChild = NULL;

            for ( int i = 0; i < Children.Count(); i++ )
            {
                if ( token == Children[i].Name )
                {
                    nChild = Children[i];
                    break;
                }
            }
            
            if ( nChild == NULL )
            {
                nChild = new Permission( token, this );

                Children.Insert( nChild );
            }

            nChild.AddPermissionInternal( tokens, depth + 1 );
        } else {
            Value = value;
        }
    }

    private void VerifyAddPermission( string token, bool value )
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
        
        if ( permissionExisted == false )
        {
            ref Permission newPerm = new Permission( token, this );
            newPerm.Value = value;

            Children.Insert( newPerm );
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

    bool HasPermission( string inp )
    {
        array<string> tokens = new array<string>;
        inp.Split( ".", tokens );

        if ( tokens.Count() == 0 ) return false;
        
        int depth = tokens.Find(Name);

        if ( depth > -1 )
        {
            return Check( tokens, depth + 1 );
        } else 
        {
            return Check( tokens, 0 );
        }

        return false;
    }

    bool Check( array<string> tokens, int depth )
    {
        if ( depth < tokens.Count() )
        {
            if ( Children.Count() == 0 ) 
            {
                return true;
            }

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
                return nChild.Check( tokens, depth + 1 );
            }

            return false;
        }

        return true;
    }

    void Serialize( ref array< string > output, string prepend = "" )
    {
        for ( int i = 0; i < Children.Count(); i++ )
        {
            string serialize = prepend + Children[i].Name;

            if ( Children[i].Children.Count() == 0 )
            {
                output.Insert( serialize );
            } else 
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

        Print( message );

        for ( int i = 0; i < Children.Count(); i++ )
        {
            Children[i].DebugPrint( depth + 1 );
        }
    }
}