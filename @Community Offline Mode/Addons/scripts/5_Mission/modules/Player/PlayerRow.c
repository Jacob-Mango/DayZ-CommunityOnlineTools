class PlayerRow extends ScriptedWidgetEventHandler 
{
    protected ref Widget layoutRoot;

    TextWidget      m_Name;

    ButtonWidget    m_Button;
    CheckBoxWidget  m_Checkbox;

    ref AuthPlayer  Player;

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

        m_Button = ButtonWidget.Cast(layoutRoot.FindAnyWidget("button"));
        m_Checkbox = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("button"));
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

        m_Name.SetText( Player.GetName() );

        if ( Player.GetGUID() == GetGame().GetPlayer().GetIdentity().GetId() )
        {
            m_Name.SetColor( COLOR_GREEN );
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
        Print( "PlayerRow::OnClick" );
        
        if ( w == m_Checkbox )
        {
            playerMenu.OnPlayerSelected( this );
        }

        if ( w == m_Button )
        {
            RemoveAllSelectedPlayers();

            playerMenu.OnPlayerSelected( this );
        }
        return true;
    }
}