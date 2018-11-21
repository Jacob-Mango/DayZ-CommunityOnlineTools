class UIActionBase extends ScriptedWidgetEventHandler 
{
    protected ref Widget layoutRoot;

    protected Class m_Instance;
    protected string m_FuncName;

    protected bool m_HasCallback;

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

    void SetCallback( Class instance, string funcname )
    {
        if ( instance == NULL || funcname == "" ) return;

        m_Instance = instance;
        m_FuncName = funcname;

        m_HasCallback = true;
    }

    bool CallEvent( UIEvent eid )
    {
        return false;
    }
}