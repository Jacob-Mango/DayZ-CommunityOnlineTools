class PermissionRow extends ScriptedWidgetEventHandler 
{
    ref PermissionRow Parent;
    ref array< ref PermissionRow > Children;

    ref Permission Perm;

    protected ref Widget layoutRoot;
    protected ref Widget parent;

    EditBoxWidget edit_name;
    CheckBoxWidget perm_state;
    ButtonWidget del_perm;
    
    void OnWidgetScriptInit( Widget w )
    {
        layoutRoot = w;
        layoutRoot.SetHandler( this );

        Init();
    }

    void Init() 
    {
        Children = new ref array< ref PermissionRow >;

        edit_name = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("permission_name"));
        perm_state = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("permission_setting"));
        del_perm = ButtonWidget.Cast(layoutRoot.FindAnyWidget("button_remove"));
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

    void Update() 
    {
        
    }

    ref Widget GetLayoutRoot() 
    {
        return layoutRoot;
    }

    ref Widget GetParent()
    {
        return parent;
    }

    string Indent( int depth )
    {
        string s "";
        for ( int i = 0; i < depth; i++ )
        {
            s = s + "  ";
        }
        return s;
    }

    void Set( ref Permission permission, int depth, ref Widget p )
    {
        Perm = permission;

        edit_name.SetText( Indent( depth ) + Perm.Name );
        perm_state.SetChecked( false );

        parent = p;
    }

    string GetPermission()
    {
        return edit_name.GetText():
    }

    bool GetState()
    {
        return perm_state.IsChecked();
    }
}