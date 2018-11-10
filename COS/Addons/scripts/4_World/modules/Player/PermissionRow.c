class PermissionRow extends ScriptedWidgetEventHandler 
{
    protected ref Widget layoutRoot;

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

    string Indent( int depth )
    {
        string s "";
        for ( int i = 0; i < depth; i++ )
        {
            s = s + "  ";
        }
        return s;
    }

    void Set( ref Permission permission, int depth )
    {
        edit_name.SetText( Indent( depth ) + permission.Name );
        perm_state.SetChecked( false );
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