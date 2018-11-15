class EditorMenu extends UIScriptedMenu 
{
    protected ref Widget m_ButtonsContainer;
    protected ref Widget m_Windows;

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
        layoutRoot = GetGame().GetWorkspace().CreateWidgets( "COT\\gui\\layouts\\editor\\EditorMenu.layout" );

        m_ButtonsContainer = layoutRoot.FindAnyWidget( "Buttons" );
        m_Windows = layoutRoot.FindAnyWidget( "Windows" );

        for ( int i = 0; i < m_Modules.Count(); i++ )
        {
            ref EditorModule module = m_Modules.Get( i );

            ref Widget button_bkg = NULL;
            ref Widget button = NULL;

            if ( module.HasAccess() )
            {
                button_bkg = GetGame().GetWorkspace().CreateWidgets( "COT\\gui\\layouts\\editor\\Button.layout", m_ButtonsContainer );
                button = button_bkg.FindAnyWidget( "btn" );
            }

            ref Widget base_window = GetGame().GetWorkspace().CreateWidgets( "COT\\gui\\layouts\\editor\\WindowHandle.layout", m_Windows );

            ref Widget menu = GetGame().GetWorkspace().CreateWidgets( module.GetLayoutRoot(), base_window.FindAnyWidget( "content" ) );

            float width = -1;
            float height = -1;
            menu.GetSize( width, height );

            Form form;
            WindowHandle window;

            menu.GetScript( form );
            base_window.GetScript( window );

            if ( form && window )
            {
                form.window = window;
                form.module = module;
                window.form = form;

                base_window.SetSize( width, height + 25 );

                if ( button_bkg && button )
                {
                    TextWidget title_text = base_window.FindAnyWidget( "title_text" );
                    title_text.SetText( form.GetTitle() );
        
                    TextWidget ttl = button_bkg.FindAnyWidget( "ttl" );
                    ttl.SetText( form.GetTitle() );

                    ImageWidget btn_img = button_bkg.FindAnyWidget( "btn_img" );
                    TextWidget btn_txt = button_bkg.FindAnyWidget( "btn_txt" );

                    if ( form.ImageIsIcon() )
                    {
                        btn_txt.Show( false );
                        btn_img.Show( true );

                        btn_img.LoadImageFile( 0, "set:" + form.GetImageSet() + " image:" + form.GetIconName() );
                    } else
                    {
                        btn_txt.Show( true );
                        btn_img.Show( false );

                        btn_txt.SetText( form.GetIconName() );
                    }
                }
    
                module.menuButton = button;
                module.form = form;
            }
        }

        return layoutRoot;
    }

    override bool UseMouse() 
    {
        return true;
    }

    override bool UseKeyboard() 
    {
        return true;
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
    }

    override bool OnDoubleClick( Widget w, int x, int y, int button ) 
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return false;

        return false;
    }

    override bool OnClick( Widget w, int x, int y, int button )
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return false;
        
        Form form;

        for ( int i = 0; i < m_Modules.Count(); i++ )
        {
            EditorModule module = m_Modules.Get( i );

            if ( w == module.menuButton )
            {
                form = module.form;
            }
        }

        if ( form ) 
        {
            if ( form.GetLayoutRoot().IsVisible() ) 
            {
                form.Hide();
            }
            else 
            {
                form.Show();
            }

            SetButtonFocus( w );
        }

        return false;
    }

    void SetButtonFocus( Widget focus ) 
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;

        // reimplement
    }

    override void Update( float timeslice ) 
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;

		GetGame().GetInput().DisableKey(KeyCode.KC_RETURN);

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

