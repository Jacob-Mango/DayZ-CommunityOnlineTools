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
        GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove( this.Update );
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
    }

    float ATan( float a )
    {
        return Math.Sin( a ) / Math.Cos( a );
    }

    void Update() 
    {
        vector normalize = ( Player.Data.VPosition - GetGame().GetCurrentCameraPosition() );
        float dot = vector.Dot( normalize.Normalized(), GetGame().GetCurrentCameraDirection().Normalized() );
        
        Print( "dot " + dot + " fov " + FOV );

        if ( dot < 0 )
        {
            Hide();
            ShowOnScreen = false;
        }

        if ( dot < Math.Cos( Math.PI - ( FOV / 2 ) ) )
        {
            Hide();
            ShowOnScreen = false;
        }
            
        ScreenPos = GetGame().GetScreenPos( Player.Data.VPosition );

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
                ShowOnScreen = false;
                Name.SetColor( 0xFF2ECC71 );
                Hide();
            } else
            {
                Name.SetColor( 0xFFFFFFFF );
            
                ScreenPos = GetGame().GetScreenPos( Player.Data.VPosition );

                if ( ScreenPos[2] > 1000 )
                {
                    Hide();
                    ShowOnScreen = false;
                } else 
                {
                    ShowOnScreen = true;

                    GetScreenSize( Width, Height );

                    FOV = 2.0 * ATan( Math.Tan( Camera.GetCurrentFOV() * 0.5 ) * ( Width / Height ) );

                    GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Insert( this.Update );

                    Show();
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