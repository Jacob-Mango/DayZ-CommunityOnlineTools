class PlayerBox extends ScriptedWidgetEventHandler 
{
    protected ref Widget layoutRoot;

    ref TextWidget      Name;
    ref ButtonWidget    Button;
    ref CheckBoxWidget  Checkbox;

    ref AuthPlayer  Player;

    ref PlayerMenu  Menu;

    bool ShowOnScreen;

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
        GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Insert( this.Update );
    }

    void OnHide() 
    {
        GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove( this.Update );
    }

    void Update() 
    {
        if ( ShowOnScreen && Player )
        {
            vector pos = GetGame().GetScreenPos( Player.Data.VPosition );
            layoutRoot.SetPos( -pos[0], -pos[1], true );
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
        } else 
        {
            Show();

            Name.SetText( Player.GetName() );

            if ( GetGame().GetPlayer() == NULL ) return;

            if ( !GetGame().IsMultiplayer() )
            {
                Name.SetColor( 0xFF4B77BE );
                return;
            }

            if ( Player.GetGUID() == GetGame().GetPlayer().GetIdentity().GetId() )
            {
                ShowOnScreen = false;
                Name.SetColor( 0xFF2ECC71 );
            } else
            {
                ShowOnScreen = true;
                Name.SetColor( 0xFFFFFFFF );
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