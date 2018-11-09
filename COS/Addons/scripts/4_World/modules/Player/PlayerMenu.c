class PlayerMenu extends PopupMenu
{
    TextListboxWidget       m_PlayerScriptList;
    ButtonWidget            m_ReloadScriptButton;
    MultilineEditBoxWidget  m_PermissionsText;
    ButtonWidget            m_SetPermissionsButton;

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
        m_PlayerScriptList      = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("player_list"));
        m_ReloadScriptButton    = ButtonWidget.Cast(layoutRoot.FindAnyWidget("refresh_list"));

        m_PermissionsText       = MultilineEditBoxWidget.Cast(layoutRoot.FindAnyWidget("permissions_list"));
        m_SetPermissionsButton  = ButtonWidget.Cast(layoutRoot.FindAnyWidget("permissions_set_button"));
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
                
                OnPlayerSelected();
            }
        }
        
        if ( w == m_ReloadScriptButton )
        {
            ReloadPlayers();
        }

        if ( w == m_SetPermissionsButton )
        {
            SetPermissions();
        }

        return false;
    }

    void OnPlayerSelected()
    {
        layoutRoot.FindAnyWidget("PlayerPermsContainer").Enable( false );

        if ( SELECTED_PLAYER != NULL )
        {
            GetRPCManager().SendRPC( "COS_Player", "LoadPermissions", new Param, true, NULL, SELECTED_PLAYER );
        }
    }

    void ReloadPlayers()
    {
        GetRPCManager().SendRPC( "COS_Player", "ReloadList", new Param, true );
            
		GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( UpdateList, 500 );
    }

    void LoadPermissions( ref array< string > text )
    {
        for ( int i = 0; i < m_PermissionsText.GetLinesCount(); i++ )
        {
            if ( i < text.Count() )
            {
                m_PermissionsText.SetLine( i, text[i] );
            } else 
            {
                m_PermissionsText.SetLine( i, "" );
            }

            layoutRoot.FindAnyWidget("PlayerPermsContainer").Enable( true );
        }
    }

    void SetPermissions()
    {
        string permissionText;
        m_PermissionsText.GetText( permissionText );
        
        ref array< string > permsArr = new ref array< string >;

        permissionText.Split( "\N", permsArr );

        GetRPCManager().SendRPC( "COS_Player", "SetPermissions", new Param1< ref array< string > >( permsArr ), true, NULL, SELECTED_PLAYER );
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