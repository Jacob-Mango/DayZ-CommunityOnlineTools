class PermissionRow extends ScriptedWidgetEventHandler 
{
    ref PermissionRow Parent;
    ref array< ref PermissionRow > Children;

    ref Permission Perm;

    protected ref Widget layoutRoot;
    protected ref Widget parent;

    TextWidget perm_name;
    XComboBoxWidget perm_state;

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
        perm_state = XComboBoxWidget.Cast(layoutRoot.FindAnyWidget("permission_setting"));
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

	override bool OnUpdate( Widget w )
    {
        if ( Perm )
        {
            perm_name.SetText( Perm.Name );
            perm_state.SetCurrentItem( Perm.Type );
        } else 
        {
            perm_state.SetCurrentItem( 0 );
        }

        return true;
    }

    ref Widget GetLayoutRoot() 
    {
        return layoutRoot;
    }

    ref Widget GetParent()
    {
        return parent;
    }

    void Set( string name, ref Widget p )
    {
        perm_name.SetText( name );
        perm_state.SetCurrentItem( 0 );

        parent = p;
    }

    void SetPermission( ref Permission permission )
    {
        Perm = permission;
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

    }

    void OnDisable()
    {

    }
}