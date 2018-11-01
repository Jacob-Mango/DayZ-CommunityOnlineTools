class EditorMenu extends UIScriptedMenu 
{
    protected ref Widget m_ButtonsContainer;

    protected ref array< ref EditorModule > m_Modules;

    void EditorMenu()
    {
        m_Modules = GetModuleManager().GetEditorModules();
    }
    
    void ~EditorMenu()
    {
    }
    
    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets( "COS\\gui\\layouts\\EditorMenu.layout" );

        m_ButtonsContainer = layoutRoot.FindAnyWidget( "Buttons" );

        for ( int i = 0; i < m_Modules.Count(); i++ )
        {
            Widget button_bkg = GetGame().GetWorkspace().CreateWidgets( "COS\\gui\\layouts\\Button.layout", m_ButtonsContainer );

            ref Widget button = button_bkg.FindAnyWidget( "btn" );

            ref Widget menu = GetGame().GetWorkspace().CreateWidgets( m_Modules.Get( i ).GetLayoutRoot(), layoutRoot );

            m_Modules.Get( i ).m_Button = button;
            m_Modules.Get( i ).m_Menu = menu;
        }

        return layoutRoot;
    }

    override bool UseMouse() 
    {
        return false;
    }

    override bool UseKeyboard() 
    {
        return false;
    }

    override void OnShow()
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;
        
        super.OnShow();

        GetGame().GetInput().ChangeGameFocus( 1, INPUT_DEVICE_MOUSE);
        GetGame().GetUIManager().ShowUICursor( true );
    }

    override void OnHide()
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;
        
        super.OnHide();

        GetGame().GetInput().ResetGameFocus( INPUT_DEVICE_MOUSE );
        GetPlayer().GetInputController().OverrideMovementSpeed( false, 0 );
    }

    override bool OnDoubleClick( Widget w, int x, int y, int button ) 
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return false;

        return false;
    }

    override bool OnClick( Widget w, int x, int y, int button )
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return false;
        
        PopupMenu popMenu;

        for ( int i = 0; i < m_Modules.Count(); i++ )
        {
            EditorModule module = m_Modules.Get( i );
            if ( w == module.m_Button )
            {
                module.m_Menu.GetScript( popMenu );
            }
        }

        if ( popMenu ) 
        {
            if ( popMenu.GetLayoutRoot().IsVisible() ) 
            {
                popMenu.GetLayoutRoot().Show( false );
                popMenu.OnHide();
            }
            else 
            {
                popMenu.GetLayoutRoot().Show( true );
                popMenu.OnShow();
            }

            SetButtonFocus( w );
            HideMenus( popMenu.GetLayoutRoot() );
        }

        return false;
    }

    void SetButtonFocus( Widget focus ) 
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;

        // reimplement
    }

    void HideMenus( Widget focus ) 
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;

        // reimplement
    }

    override void Update( float timeslice ) 
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;

        if ( GetMouseState( MouseState.RIGHT ) & MB_PRESSED_MASK ) 
        {
            if ( GetGame().GetUIManager().IsCursorVisible() ) 
            {
                GetGame().GetUIManager().ShowUICursor( false );
                GetGame().GetInput().ResetGameFocus( INPUT_DEVICE_MOUSE );
            }
        }
        else
        {
            if ( !GetGame().GetUIManager().IsCursorVisible() ) 
            {
                GetGame().GetUIManager().ShowUICursor( true );
                GetGame().GetInput().ChangeGameFocus( 1, INPUT_DEVICE_MOUSE );
            }
        }
    }
}

