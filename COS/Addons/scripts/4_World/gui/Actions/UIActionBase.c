class UIActionBase extends ScriptedWidgetEventHandler 
{
    protected ref Widget layoutRoot;

    void OnWidgetScriptInit( Widget w )
    {
        layoutRoot = w;
        layoutRoot.SetHandler( this );

        OnInit();
    }

    void OnInit() 
    {
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
}