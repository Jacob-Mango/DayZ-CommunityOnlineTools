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

        Init();
    }

    void Init() 
    {
        m_CloseButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "close_button" ) );

        WidgetHandler.GetInstance().RegisterOnClick( m_CloseButton, this, "OnClick" );
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

    bool OnClick( Widget w, int x, int y, int button )
    {
        Print("BaseWindow::OnClick");
        Print( "w: " + w );

        if ( w == m_CloseButton )
        {
            popupMenu.Hide();
        }

        return false;
    }

	bool OnDrop( Widget w, int x, int y, Widget reciever )
	{
        Print("BaseWindow::OnDrop");
        Print( "w: " + w );

        if ( reciever == NULL )
        {
		    w.SetPos( x, y, true );
        }

		return true;
	}

    ref Widget GetLayoutRoot() 
    {
        return layoutRoot;
    }
}