class COTMenu 
{
    protected Widget layoutRoot;

    protected Widget m_ButtonsContainer;
    protected Widget m_Windows;

    protected array< ref EditorModule > m_Modules;

    protected bool m_GameFocus;

    void COTMenu()
    {
    }
    
    void ~COTMenu()
    {
        Hide();
    }

    bool IsVisible()
    {
        return layoutRoot.IsVisible();
    }
    
    Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets( "JM\\COT\\gui\\layouts\\COT\\COTMenu.layout" );
        layoutRoot.Show( false );

        m_ButtonsContainer = layoutRoot.FindAnyWidget( "Buttons" );
        m_Windows = layoutRoot.FindAnyWidget( "Windows" );

        m_Modules = GetModuleManager().GetEditorModules();

        for ( int i = 0; i < m_Modules.Count(); i++ )
        {
            ref EditorModule module = m_Modules.Get( i );

            ref Widget button_bkg = NULL;
            ref ButtonWidget button = NULL;

            button_bkg = GetGame().GetWorkspace().CreateWidgets( "JM\\COT\\gui\\layouts\\COT\\COTButton.layout", m_ButtonsContainer );
            button = ButtonWidget.Cast( button_bkg.FindAnyWidget( "btn" ) );

            ref Widget base_window = GetGame().GetWorkspace().CreateWidgets( "JM\\COT\\gui\\layouts\\COT\\WindowHandle.layout", m_Windows );

            ref Widget menu = GetGame().GetWorkspace().CreateWidgets( module.GetLayoutRoot(), base_window.FindAnyWidget( "content" ) );

            float width = -1;
            float height = -1;
            menu.GetSize( width, height );

            ref Form form;
            ref WindowHandle window;

            menu.GetScript( form );
            base_window.GetScript( window );

            if ( form && window )
            {
                form.window = window;
                form.module = module;

                form.Init( true );

                window.form = form;

                base_window.SetSize( width, height + 25 );

                if ( button_bkg && button )
                {
                    TextWidget title_text = TextWidget.Cast( base_window.FindAnyWidget( "title_text" ) );
                    title_text.SetText( form.GetTitle() );
        
                    TextWidget ttl = TextWidget.Cast( button_bkg.FindAnyWidget( "ttl" ) );
                    ttl.SetText( form.GetTitle() );

                    ImageWidget btn_img = ImageWidget.Cast( button_bkg.FindAnyWidget( "btn_img" ) );
                    TextWidget btn_txt = TextWidget.Cast( button_bkg.FindAnyWidget( "btn_txt" ) );

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

                WidgetHandler.GetInstance().RegisterOnClick( module.menuButton, this, "OnClick" );
                WidgetHandler.GetInstance().RegisterOnDoubleClick( module.menuButton, this, "OnDoubleClick" );
            }
        }

        return layoutRoot;
    }

    void Show()
    {
        SetFocus( NULL );

        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;

        layoutRoot.Show( true );

        COTMenuOpen = true;

        OnShow();
    }

    void Hide()
    {
        SetFocus( NULL );

        OnHide();

        layoutRoot.Show( false );

        COTMenuOpen = false;
    }

    void OnShow()
    {
        SetInputFocus( false );

        GetGame().GetInput().ChangeGameFocus( 1 );
        GetGame().GetUIManager().ShowUICursor( true );

        GetGame().GetMission().GetHud().Show( false );
    }

    void OnHide()
    {
        SetInputFocus( true );

        GetGame().GetInput().ResetGameFocus();
        GetGame().GetUIManager().ShowUICursor( false );

        GetGame().GetMission().GetHud().Show( true );
    }

    void ToggleInputFocus()
    {
        m_GameFocus = !m_GameFocus;
    }

    void SetInputFocus( bool focus )
    {
        m_GameFocus = focus;
    }

    void OnUpdate( float timeslice )
    {
        if ( !IsVisible() ) return;

        if ( m_GameFocus )
        {
            GetGame().GetInput().ResetGameFocus();
            GetGame().GetUIManager().ShowUICursor( false );
        } else
        {
            GetGame().GetInput().ChangeGameFocus( 1 );
            GetGame().GetUIManager().ShowUICursor( true );
        }
    }

    bool OnClick( Widget w, int x, int y, int button )
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return false;
        
        Form form;
        EditorModule module;

        for ( int i = 0; i < m_Modules.Count(); i++ )
        {
            module = m_Modules.Get( i );

            if ( w == module.menuButton )
            {
                form = module.form;
                break;
            }
        }

        if ( form ) 
        {
            if ( form.GetLayoutRoot().IsVisible() ) 
            {
                form.Hide();
            }
            else if ( module.HasAccess() )
            {
                form.Show();
            }
        }

        return false;
    }

    // TODO: Fix
    bool OnDoubleClick(Widget w, int x, int y, int button)
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

        if ( form && form.window && !form.GetLayoutRoot().IsVisible() ) 
        {
            //form.Show();

            //form.window.SetPosition( 0, 0 );

            return true;
        }

        return false;
    }
}

