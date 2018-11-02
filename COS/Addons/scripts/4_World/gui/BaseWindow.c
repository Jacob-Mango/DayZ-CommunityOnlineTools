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
        super.OnClick( w, x, y, button );

        Print( "w: " + w );

        if ( w == m_CloseButton )
        {
            popupMenu.Hide();
        }

        return false;
    }

	override bool OnDrop( Widget w, int x, int y, Widget reciever )
	{
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