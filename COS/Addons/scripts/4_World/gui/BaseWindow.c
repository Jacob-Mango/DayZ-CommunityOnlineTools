class BaseWindow extends ScriptedWidgetEventHandler 
{
    protected ref Widget layoutRoot;

    protected ref ButtonWidget m_CloseButton;
    protected ref Widget m_TitleWrapper;

    ref PopupMenu popupMenu;

    float offsetX;
    float offsetY;

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
        m_TitleWrapper = Widget.Cast( layoutRoot.FindAnyWidget( "title_wrapper" ) );

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

    override bool OnUpdate(Widget w)
    {
        if ( w == m_TitleWrapper )
        {
            m_TitleWrapper.SetPos( 0, 0, true );
        }
        return true;
    }

    override bool OnClick( Widget w, int x, int y, int button )
    {
        if ( w == m_CloseButton )
        {
            popupMenu.Hide();
        }

        return false;
    }

	override bool OnDrag( Widget w, int x, int y )
	{
        if ( w == m_TitleWrapper )
        {
		    layoutRoot.GetPos( offsetX, offsetY );

            offsetX = x - offsetX;
            offsetY = y - offsetY;

            m_TitleWrapper.SetPos( 0, 0, true );
        }

		return true;
	}

	override bool OnDragging( Widget w, int x, int y, Widget reciever )
	{
        if ( w == m_TitleWrapper )
        {
		    layoutRoot.SetPos( x - offsetX, y - offsetY, true );
            m_TitleWrapper.SetPos( 0, 0, true );
        }

		return true;
	}

	override bool OnDrop( Widget w, int x, int y, Widget reciever )
	{
        if ( w == m_TitleWrapper )
        {
		    layoutRoot.SetPos( x - offsetX, y - offsetY, true );
            m_TitleWrapper.SetPos( 0, 0, true );
        }

        // TODO: Save the position temporarily so it remembers on close. Maybe also permanently when game opens and closes.

		return true;
	}

    ref Widget GetLayoutRoot() 
    {
        return layoutRoot;
    }
}