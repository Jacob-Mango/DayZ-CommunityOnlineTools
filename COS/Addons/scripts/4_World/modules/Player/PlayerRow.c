class PlayerRow extends ScriptedWidgetEventHandler 
{
    protected ref Widget layoutRoot;

    TextWidget      m_Name;
    TextWidget      m_GUID;

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

    void SetPlayer( PlayerIdentity identity, Man player )
    {
        m_Identity = identity;
        m_Player = player;

        m_Name.SetText( identity.GetName() );
        m_GUID.SetText( identity.GetId() );
    }

    string GetName()
    {
        return ""; //m_Name.GetText():
    }

    string m_GUID()
    {
        return ""; //m_GUID.GetText();
    }

	override bool OnClick(Widget w, int x, int y, int button)
	{
        playerMenu.OnPlayerSelected( this );
        return true;
    }
}