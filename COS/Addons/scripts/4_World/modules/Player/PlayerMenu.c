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
        m_PlayerScriptList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("player_list"));
        m_ReloadScriptButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("reload_list"));

        ReloadPlayers();
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
            int selectedRow = m_PlayerScriptList.GetSelectedRow();
            if ( selectedRow >= 0 && selectedRow < m_PlayerScriptList.GetNumItems() )
            {
                m_PlayerScriptList.GetItemData( selectedRow, 0, SELECTED_PLAYER );
            }
        }
        
        if ( w == m_ReloadScriptButton )
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
        Print("PlayerMenu::UpdateList");

        if ( m_PlayerScriptList == NULL ) return;

        Print("Base Module " + baseModule );

        m_PlayerScriptList.ClearItems();

        ref PlayerModule playerModule = PlayerModule.Cast( baseModule ); 
        if ( playerModule )
        {
            for ( int i = 0; i < playerModule.m_Players.Count(); i++ )
            {
                Man player = playerModule.m_Players.Get( i );
                PlayerIdentity identity = player.GetIdentity();
                m_PlayerScriptList.AddItem( identity.GetName() + " (" + identity.GetId() + ")", player, 0, i );

                Print( "Player: " + identity.GetName() );

                if ( identity.GetId() == GetGame().GetPlayer().GetIdentity().GetId() )
                {
                    m_PlayerScriptList.SetItemColor( i, 0, COLOR_GREEN );
                }
            }
        }
    }
}