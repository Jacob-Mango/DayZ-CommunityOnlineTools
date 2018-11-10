class PlayerRow extends ScriptedWidgetEventHandler 
{
    protected ref Widget layoutRoot;

    TextWidget      m_Name;
    TextWidget      m_GUID;

    ButtonWidget    m_Button;

    Man             m_Player;
    PlayerIdentity  m_Identity;

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
        m_GUID = TextWidget.Cast(layoutRoot.FindAnyWidget("text_guid"));

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

    void SetPlayer( ref PlayerIdentity identity, ref Man player )
    {
        m_Identity = identity;
        m_Player = player;

        m_Name.SetText( m_Identity.GetName() );
        m_GUID.SetText( m_Identity.GetId() );

        if ( m_Identity == GetGame().GetPlayer().GetIdentity() )
        {
            m_Name.SetColor( COLOR_GREEN );
            m_GUID.SetColor( COLOR_GREEN );
        }
    }

    string GetName()
    {
        return m_Identity.GetName();
    }

    string GetGUID()
    {
        return m_Identity.GetId();
    }

	override bool OnClick(Widget w, int x, int y, int button)
	{
        playerMenu.OnPlayerSelected( this );
        return true;
    }
}