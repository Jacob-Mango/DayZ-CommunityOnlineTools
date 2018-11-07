class PlayerMenu extends PopupMenu
{
    TextListboxWidget   m_PlayerScriptList;
    ButtonWidget        m_ReloadScriptButton;

    void PlayerMenu()
    {
    }

    void ~PlayerMenu()
    {
    }

    override string GetTitle()
    {
        return "Players";
    }
    
    override string GetIconName()
    {
        return "P";
    }

    override bool ImageIsIcon()
    {
        return false;
    }

    override void Init()
    {
        m_PlayerScriptList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("game_list_box"));
        m_ReloadScriptButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("reload_scripts_button"));
    }

    override void OnShow()
    {
        ReloadPlayers();
    }

    override void OnHide() 
    {
    }

    override bool OnClick( Widget w, int x, int y, int button )
    {  
        if ( w == m_PlayerScriptList )
        {
            ReloadPlayers();
        }

        return false;
    }

    void ReloadPlayers()
    {
        GetRPCManager().SendRPC( "COS_Player", "ReloadList", new Param, true );
            
		GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( UpdateList, 500 );
    }

    void UpdateList()
    {
        m_PlayerScriptList.ClearItems();

        ref PlayerModule playerModule = PlayerModule.Cast( baseModule ); 
        if ( playerModule )
        {
            for ( int i = 0; i < playerModule.m_Players.Count(); i++ )
            {
                Man player = playerModule.m_Players.Get( i );
                PlayerIdentity identity = player.GetIdentity();
                m_PlayerScriptList.AddItem( identity.GetName() + " (" + identity.GetId() + ")", player, 0 );
            }
        }
    }
}