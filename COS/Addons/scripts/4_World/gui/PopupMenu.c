class PopupMenu extends ScriptedWidgetEventHandler 
{
    protected ref Widget layoutRoot;
    
    ref Widget baseWindow;

    void PopupMenu() 
    {

    }

    void ~PopupMenu() 
    {
    }

    void OnWidgetScriptInit( Widget w )
    {
        layoutRoot = w;
        layoutRoot.SetHandler( this );

        Init();
    }

    void Init() 
    {

    }

    void OnShow()
    {
        baseWindow.Show( true );
        layoutRoot.Show( true );
    }

    void OnHide() 
    {
        layoutRoot.Show( false );
        baseWindow.Show( false );
    }

    void Update() 
    {
        
    }

    ref Widget GetLayoutRoot() 
    {
        return layoutRoot;
    }

    string GetTitle()
    {
        return "";
    }

}