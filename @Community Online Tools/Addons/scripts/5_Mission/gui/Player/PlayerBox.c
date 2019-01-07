class PlayerBox extends ScriptedWidgetEventHandler 
{
    protected ref Widget layoutRoot;

    ref TextWidget      Name;
    ref ButtonWidget    Button;
    ref CheckBoxWidget  Checkbox;

    ref AuthPlayer  Player;

    ref PlayerMenu  Menu;

    bool ShowOnScreen;

    int Width;
    int Height;
    float FOV;
    vector ScreenPos;

    void OnWidgetScriptInit( Widget w )
    {
        layoutRoot = w;
        layoutRoot.SetHandler( this );

        Init();
    }

    void ~PlayerBox()
    {
    }

    void Init() 
    {
        Name = TextWidget.Cast(layoutRoot.FindAnyWidget("text_name"));
        Button = ButtonWidget.Cast(layoutRoot.FindAnyWidget("button"));
        Checkbox = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("checkbox"));

    }

    void Show()
    {
        layoutRoot.Show( true );
        Button.Show( true );
        Checkbox.Show( true );
        Name.Show( true );
        OnShow();
    }

    void Hide()
    {
        OnHide();
        Name.Show( false );
        Button.Show( false );
        Checkbox.Show( false );
        layoutRoot.Show( false );
    }

    void OnShow()
    {
    }

    void OnHide() 
    {
        GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove( this.Update );
    }

    float ATan( float a )
    {
        return Math.Asin( a ) / Math.Acos( a );
    }

    void Update() 
    {
        vector normalize = ( Player.PlayerObject.GetPosition() - GetGame().GetCurrentCameraPosition() );
        float dot = vector.Dot( normalize.Normalized(), GetGame().GetCurrentCameraDirection().Normalized() );
        
        float limit = FOV / 1.5;

        if ( dot < limit )
        {
            Hide();
            ShowOnScreen = false;
        } else
        {
            ShowOnScreen = true;
        }
            
        ScreenPos = GetGame().GetScreenPos( Player.PlayerObject.GetPosition() + "0 2 0");

        if ( ShowOnScreen )
        {
            if ( ScreenPos[2] > 1000 )
            {
                Hide();
                ShowOnScreen = false;
            }
            
            if ( !layoutRoot.IsVisible() )
            {
                Show();
            }
        }

        if ( ShowOnScreen && Player )
        {
            layoutRoot.SetPos( ScreenPos[0], ScreenPos[1] - ( Height / 2 ), true );
        }
    }

    ref Widget GetLayoutRoot() 
    {
        return layoutRoot;
    }

    void SetPlayer( ref AuthPlayer player )
    {
        Player = player;
        
        if ( Player == NULL ) 
        {
            ShowOnScreen = false;
            Hide();
            GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove( this.Update );
        } else 
        {
            ShowOnScreen = true;

            if ( Player.PlayerObject == NULL )
            {
                Hide();
                ShowOnScreen = false;
                return;
            }

            Name.SetText( Player.GetName() );

            if ( GetGame().GetPlayer() == NULL ) return;

            if ( !GetGame().IsMultiplayer() )
            {
                ShowOnScreen = false;
                Name.SetColor( 0xFF4B77BE );
                Hide();
                return;
            }

            if ( Player.GetGUID() == GetGame().GetPlayer().GetIdentity().GetId() )
            {
                if ( CurrentActiveCamera == NULL )
                {
                    ShowOnScreen = false
                    Hide();
                }

                Name.SetColor( 0xFF2ECC71 );
            } 

            if ( ShowOnScreen )
            {
                Name.SetColor( 0xFFFFFFFF );
            
                ScreenPos = GetGame().GetScreenPos( Player.PlayerObject.GetPosition() );

                if ( ScreenPos[2] > 1000 )
                {
                    Hide();
                    ShowOnScreen = false;
                } else 
                {
                    ShowOnScreen = true;

                    GetScreenSize( Width, Height );

                    FOV = Camera.GetCurrentFOV() * ( Height * 1.0 ) / ( Width * 1.0 );

                    GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Insert( this.Update );

                    Update();
                }
            }
        }
    }

    string GetName()
    {
        return Player.GetName();
    }

    ref AuthPlayer GetPlayer()
    {
        return Player;
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {        
        if ( w == Checkbox )
        {
            Menu.OnPlayer_Checked( Player, Checkbox.IsChecked() );
        }

        if ( w == Button )
        {
            Checkbox.SetChecked( Menu.OnPlayer_Button( Player ) );
        }

        return true;
    }
}