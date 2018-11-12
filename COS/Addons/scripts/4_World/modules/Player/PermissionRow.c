class PermissionRow extends ScriptedWidgetEventHandler 
{
    private string indent = "";

    ref PermissionRow Parent;
    ref array< ref PermissionRow > Children;

    ref Permission Perm;

    protected ref Widget layoutRoot;

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

        perm_state.AddItem("INHERIT");
        perm_state.AddItem("DISALLOW");
        perm_state.AddItem("ALLOW");
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
            perm_name.SetText( indent + Perm.Name );
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

    void Indent( int depth )
    {
        for ( int i = 0; i < depth; i++ )
        {
            indent = "   " + indent;
        }
    }

    void Set( ref Permission perm, int depth )
    {
        Indent( depth );

        perm_name.SetText( indent + perm.Name );
        perm_state.SetCurrentItem( 0 );

        Perm = NULL;
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