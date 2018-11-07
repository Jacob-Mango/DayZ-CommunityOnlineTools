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
        layoutRoot = GetGame().GetWorkspace().CreateWidgets( "COS\\gui\\layouts\\editor\\EditorMenu.layout" );

        m_ButtonsContainer = layoutRoot.FindAnyWidget( "Buttons" );

        for ( int i = 0; i < m_Modules.Count(); i++ )
        {
            ref EditorModule module = m_Modules.Get( i );
            Widget button_bkg = GetGame().GetWorkspace().CreateWidgets( "COS\\gui\\layouts\\editor\\Button.layout", m_ButtonsContainer );

            ref Widget button = button_bkg.FindAnyWidget( "btn" );

            ref Widget base_window = GetGame().GetWorkspace().CreateWidgets( "COS\\gui\\layouts\\editor\\BaseWindow.layout", layoutRoot );

            ref Widget menu = GetGame().GetWorkspace().CreateWidgets( module.GetLayoutRoot(), base_window.FindAnyWidget( "content" ) );

            float width = -1;
            float height = -1;
            menu.GetSize( width, height );

            PopupMenu popMenu;
            BaseWindow baseWindow;

            menu.GetScript( popMenu );
            base_window.GetScript( baseWindow );

            if ( popMenu && baseWindow )
            {
                popMenu.baseWindow = baseWindow;
                popMenu.baseModule = module;
                baseWindow.popupMenu = popMenu;

                base_window.SetSize( width, height );

                TextWidget title_text = base_window.FindAnyWidget( "title_text" );
                title_text.SetText( popMenu.GetTitle() );
    
                TextWidget ttl = button_bkg.FindAnyWidget( "ttl" );
                ttl.SetText( popMenu.GetTitle() );

                ImageWidget btn_img = button_bkg.FindAnyWidget( "btn_img" );
                TextWidget btn_txt = button_bkg.FindAnyWidget( "btn_txt" );

                if ( popMenu.ImageIsIcon() )
                {
                    btn_txt.Show( false );
                    btn_img.Show( true );

		            btn_img.LoadImageFile( 0, "set:" + popMenu.GetImageSet() + " image:" + popMenu.GetIconName() );
                } else
                {
                    btn_txt.Show( true );
                    btn_img.Show( false );

                    btn_txt.SetText( popMenu.GetIconName() );
                }
    
                module.m_Button = button;
                module.m_Menu = menu;
            }
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

        GetGame().GetInput().ChangeGameFocus( 1 );
        GetGame().GetUIManager().ShowUICursor( true );
    }

    override void OnHide()
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;
        
        super.OnHide();

        GetGame().GetInput().ResetGameFocus();

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
                popMenu.Hide();
            }
            else 
            {
                popMenu.Show();
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

