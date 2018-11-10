class PopupMenu extends ScriptedWidgetEventHandler 
{
    protected ref Widget layoutRoot;
    
    ref BaseWindow baseWindow;

    ref EditorModule baseModule;

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

    void Show()
    {
        baseWindow.Show();
        layoutRoot.Show( true );
        OnShow();
    }

    void Hide()
    {
        OnHide();
        layoutRoot.Show( false );
        baseWindow.Hide();
    }

    void OnShow()
    {
    }

    void OnHide() 
    {
    }

	override bool OnUpdate( Widget w )
    {
        Update();

        super.OnUpdate( w );

        return true;
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

    string GetImageSet()
    {
        return "";
    }

    string GetIconName()
    {
        return "";
    }

    bool ImageIsIcon()
    {
        return false;
    }
}