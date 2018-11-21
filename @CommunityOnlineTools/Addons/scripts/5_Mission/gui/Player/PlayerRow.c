class PlayerRow extends ScriptedWidgetEventHandler 
{
    protected ref Widget layoutRoot;

    TextWidget      Name;

    ButtonWidget    Button;
    CheckBoxWidget  Checkbox;

    ref AuthPlayer  Player;

    ref PlayerMenu  Menu;

    void OnWidgetScriptInit( Widget w )
    {
        layoutRoot = w;
        layoutRoot.SetHandler( this );

        Init();
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

    ref Widget GetLayoutRoot() 
    {
        return layoutRoot;
    }

    void SetPlayer( ref AuthPlayer player )
    {
        Player = player;

        Name.SetText( Player.GetName() );

        if ( Player.GetGUID() == GetGame().GetPlayer().GetIdentity().GetId() )
        {
            ClientAuthPlayer = player;
            
            Name.SetColor( COLOR_GREEN );
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
            Menu.OnPlayer_Checked( this );
        }

        if ( w == Button )
        {
            Menu.OnPlayer_Button( this );
        }

        return true;
    }
}