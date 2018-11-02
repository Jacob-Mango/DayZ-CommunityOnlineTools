class BaseWindow extends ScriptedWidgetEventHandler 
{
    protected ref Widget layoutRoot;

    protected ref ButtonWidget m_CloseButton;

    ref PopupMenu popupMenu;

    void BaseWindow() 
    {

    }

    void ~BaseWindow() 
    {
    }

    void OnWidgetScriptInit( Widget w )
    {
        layoutRoot = w;
        layoutRoot.SetHandler( this );

        m_CloseButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "close_button" ) );

        Print( "m_CloseButton: " + m_CloseButton );

        Init();
    }

    void Init() 
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

    void Update() 
    {
        
    }

    override bool OnClick( Widget w, int x, int y, int button )
    {
        Print( "w: " + w );

        if ( w == m_CloseButton )
        {
            popupMenu.Hide();
        }

        return false;
    }

    ref Widget GetLayoutRoot() 
    {
        return layoutRoot;
    }
}