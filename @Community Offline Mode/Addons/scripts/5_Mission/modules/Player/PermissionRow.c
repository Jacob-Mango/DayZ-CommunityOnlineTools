class PermissionRow extends ScriptedWidgetEventHandler 
{
    private string indent = "";

    ref PermissionRow Parent;
    ref array< ref PermissionRow > Children;

    ref Permission Perm;

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
		Perm.Type = perm_state.GetValue();
	}

    void Set( ref Permission perm, int depth )
    {
        Indent( depth );

        perm_name.SetText( indent + perm.Name );
        perm_state.SetValue( 0, false );

        Perm = NULL;
    }

    void SetPermission( ref Permission permission )
    {
        Perm = permission;

        if ( Perm )
        {            
            Enable();

            perm_name.SetText( indent + Perm.Name );
            perm_state.SetValue( Perm.Type, false );
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
        perm_state.Disable();
    }
}