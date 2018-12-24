class PermissionRow extends ScriptedWidgetEventHandler 
{
    private string indent = "";

    ref PermissionRow Parent;
    ref array< ref PermissionRow > Children;

    string Name;
    int Type;

    protected ref Widget layoutRoot;
    protected ref TStringArray stateOptions;

    ref TextWidget perm_name;
    ref OptionSelectorMultistate perm_state;

    void OnWidgetScriptInit( Widget w )
    {
        layoutRoot = w;
        layoutRoot.SetHandler( this );

        Init();
    }

    void Init() 
    {
        Children = new ref array< ref PermissionRow >;

        perm_name = TextWidget.Cast(layoutRoot.FindAnyWidget("permission_name"));

        stateOptions = new ref TStringArray;
        stateOptions.Insert("INHERIT");
        stateOptions.Insert("DISALLOW");
        stateOptions.Insert("ALLOW");

        perm_state = new ref OptionSelectorMultistate( layoutRoot.FindAnyWidget( "permission_setting" ), 0, NULL, true, stateOptions );
        perm_state.m_OptionChanged.Insert( OnPermissionStateChanged );
    }

    void Show()
    {
        layoutRoot.Show( true );
        OnShow();
    }

    void Hide()
    {
        OnHide();
        layoutRoot.Show( false );
    }

    void OnShow()
    {
    }

    void OnHide() 
    {
    }

    ref Widget GetLayoutRoot() 
    {
        return layoutRoot;
    }

    void Indent( int depth )
    {
        for ( int i = 0; i < depth; i++ )
        {
            indent = "   " + indent;
        }
    }

    void OnPermissionStateChanged()
    {
        Type = perm_state.GetValue();
    }

    void Set( ref Permission permission, int depth )
    {
        Indent( depth );

        Name = permission.Name;

        perm_name.SetText( indent + Name );
        perm_state.SetValue( 0, true );
    }

    void SetPermission( ref Permission permission )
    {
        if ( permission )
        {
            Enable();

            perm_name.SetText( indent + permission.Name );
            perm_state.SetValue( permission.Type, true );
        } else 
        {
            Disable();
        }
    }

    void Enable()
    {
        for ( int i = 0; i < Children.Count(); i++ )
        {
            Children[i].Enable();
        }

        OnEnable();
    }

    void Disable()
    {
        for ( int i = 0; i < Children.Count(); i++ )
        {
            Children[i].Disable();
        }

        OnDisable();
    }

    void OnEnable()
    {
        perm_state.Enable();
    }

    void OnDisable()
    {
        perm_state.SetValue( 0, true );

        perm_state.Disable();
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
    
    void SetPermission( string inp )
    {
        array<string> tokens = new array<string>;

        array<string> spaces = new array<string>;
        inp.Split( " ", spaces );

        PermissionType type;

        if ( spaces.Count() == 2 )
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
            type = PermissionType.INHERIT;

            inp.Split( ".", tokens );
        } else {
            Print( "Warning, permission line improperly formatted! Read as \"" + inp + "\" but meant to be in format \"Perm.Perm {n}\"." );
            return;
        }
        
        int depth = tokens.Find( Name );

        if ( depth > -1 )
        {
            Set( tokens, depth + 1, type );
        } else 
        {
            Set( tokens, 0, type );
        }
    }

    private ref PermissionRow Set( array<string> tokens, int depth, int type )
    {
        if ( depth < tokens.Count() )
        {
            for ( int i = 0; i < Children.Count(); i++ )
            {
                if ( Children[i].Name == tokens[depth] )
                {
                    if ( depth < tokens.Count() - 1 )
                    {
                        return Children[i].Set( tokens, depth + 1, type );
                    } else
                    {
                        Children[i].Enable();
                        Children[i].Type = type;
                        return Children[i];
                    }
                }
            }
        }
        Print("Should not be reached!");
        return NULL;
    }
}