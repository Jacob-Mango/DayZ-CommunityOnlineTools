class PlayerRow extends ScriptedWidgetEventHandler 
{
    protected ref Widget layoutRoot;

    TextWidget      m_Name;

    ButtonWidget    m_Button;

    ref PlayerData  Player;

    ref PlayerMenu  playerMenu;

    void OnWidgetScriptInit( Widget w )
    {
        layoutRoot = w;
        layoutRoot.SetHandler( this );

        Init();
    }

    void Init() 
    {
        m_Name = TextWidget.Cast(layoutRoot.FindAnyWidget("text_name"));

        m_Button = ButtonWidget.Cast(layoutRoot.FindAnyWidget("PlayerRow"));
        
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

    void SetPlayer( ref PlayerData player )
    {
        Player = player;

        m_Name.SetText( Player.Name );

        if ( Player.GUID == GetGame().GetPlayer().GetIdentity().GetId() )
        {
            m_Name.SetColor( COLOR_GREEN );
        }
    }

    string GetName()
    {
        return Player.Name;
    }

    string GetGUID()
    {
        return Player.GUID;
    }

    ref Man GetPlayer()
    {
        return Player.player;
    }

	override bool OnClick(Widget w, int x, int y, int button)
	{
        Print( "PlayerRow::OnClick" );
        playerMenu.OnPlayerSelected( this );
        return true;
    }
}