class Form extends ScriptedWidgetEventHandler 
{
    protected ref Widget layoutRoot;
    
    ref WindowHandle window;

    ref EditorModule module;

    bool HasBeenInitialized;

    void Form() 
    {
        HasBeenInitialized = false;
    }

    void ~Form() 
    {
    }

    void OnWidgetScriptInit( Widget w )
    {
        layoutRoot = w;
        layoutRoot.SetHandler( this );
    }

    void Init() 
    {
        HasBeenInitialized = true;
    }

    void Show()
    {
        window.Show();
        layoutRoot.Show( true );
        OnShow();
    }

    void Hide()
    {
        OnHide();
        layoutRoot.Show( false );
        window.Hide();
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