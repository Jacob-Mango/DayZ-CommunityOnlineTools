class EditorMenu 
{
    protected Widget layoutRoot;

    protected Widget m_ButtonsContainer;
    protected Widget m_Windows;

    protected array< ref EditorModule > m_Modules;

    void EditorMenu()
    {
    }
    
    void ~EditorMenu()
    {
        Hide();
    }

    bool IsVisible()
    {
        return layoutRoot.IsVisible();
    }
    
    Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets( "COT\\gui\\layouts\\editor\\EditorMenu.layout" );
        layoutRoot.Show( false );

        m_ButtonsContainer = layoutRoot.FindAnyWidget( "Buttons" );
        m_Windows = layoutRoot.FindAnyWidget( "Windows" );

        m_Modules = GetModuleManager().GetEditorModules();

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

            ref Form form;
            ref WindowHandle window;

            menu.GetScript( form );
            base_window.GetScript( window );

            if ( form && window )
            {
                form.window = window;
                form.module = module;

                form.Init();

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


                WidgetHandler.GetInstance().RegisterOnClick( module.menuButton, this, "OnClick" );
            }
        }

        return layoutRoot;
    }

    void Show()
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;

        layoutRoot.Show( true );

        InCommunityOfflineTools = true;

        OnShow();
    }

    void Hide()
    {
        layoutRoot.Show( false );

        InCommunityOfflineTools = false;

        OnHide();
    }

    void OnShow()
    {
        GetGame().GetInput().ChangeGameFocus( 1 );
        GetGame().GetUIManager().ShowUICursor( true );
    }

    void OnHide()
    {
        GetGame().GetInput().ResetGameFocus();
        GetGame().GetUIManager().ShowUICursor( false );
    }

    bool OnClick( Widget w, int x, int y, int button )
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
        }

        return false;
    }

    void Update() 
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;
    }
}

