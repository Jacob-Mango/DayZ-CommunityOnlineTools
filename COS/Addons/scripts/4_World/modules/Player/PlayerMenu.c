class PlayerMenu extends PopupMenu
{
    ref array< ref PermissionRow >  m_Permissions;
    ref array< ref PlayerRow >      m_PlayerList;

    GridSpacerWidget                m_PlayerScriptList;
    ButtonWidget                    m_ReloadScriptButton;
    GridSpacerWidget                m_PermsContainer;
    ButtonWidget                    m_SetPermissionsButton;

    bool                            m_ShouldUpdateList;
    bool                            m_CanUpdateList;

    void PlayerMenu()
    {
        m_CanUpdateList = true;
        m_ShouldUpdateList = false;

        m_Permissions = new ref array< ref PermissionRow >;
        m_PlayerList = new ref array< ref PlayerRow >;
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
        m_PlayerScriptList      = GridSpacerWidget.Cast(layoutRoot.FindAnyWidget("player_list"));
        m_ReloadScriptButton    = ButtonWidget.Cast(layoutRoot.FindAnyWidget("refresh_list"));

        m_PermsContainer        = GridSpacerWidget.Cast(layoutRoot.FindAnyWidget("PermsListCont"));
        m_SetPermissionsButton  = ButtonWidget.Cast(layoutRoot.FindAnyWidget("permissions_set_button"));
    }

    override void OnShow()
    {
        ReloadPlayers();
        
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater( IncUpdateList, 1000, true );
    }

    override void OnHide() 
    {
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove( IncUpdateList );
    }

    void IncUpdateList()
    {
        if ( m_ShouldUpdateList && m_CanUpdateList )
        {
            m_CanUpdateList = false;
            m_ShouldUpdateList = false;
            UpdateList();
            m_CanUpdateList = true;
        }
    }

    override bool OnClick( Widget w, int x, int y, int button )
    {        
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

    void OnPlayerSelected( PlayerRow row )
    {
        SELECTED_PLAYER = row.m_Player;

        layoutRoot.FindAnyWidget("PlayerPermsContainer").Enable( false );

        GetRPCManager().SendRPC( "COS_Player", "LoadPermissions", new Param1<PlayerIdentity>( row.m_Identity ), true );
    }

    void ReloadPlayers()
    {
        // GetRPCManager().SendRPC( "COS_Player", "ReloadList", new Param, true );
        
        m_ShouldUpdateList = true;
    }

    void LoadPermissions( ref array< string > perms )
    {
        for ( int j = 0; j < m_Permissions.Count(); j++ )
        {
            m_Permissions[j].GetLayoutRoot().Unlink();
        }

        m_Permissions.Clear();

        for ( int i = 0; i < perms.Count(); i++ )
        {
            Widget permRow = GetGame().GetWorkspace().CreateWidgets( "COS/gui/layouts/player/PermissionRow.layout", m_PermsContainer );

            PermissionRow rowScript;
            permRow.GetScript( rowScript );

            if ( rowScript )
            {
                array<string> spaces = new array<string>;
                perms[i].Split( " ", spaces );

                if ( spaces.Count() != 2 ) continue;

                bool value = false;

                if ( spaces[1].ToInt() == 1 )
                {
                    value = true;
                }

                rowScript.SetPermission( spaces[0], value );

                m_Permissions.Insert( rowScript );
            }
        }
    }

    void SetPermissions()
    {
        // GetRPCManager().SendRPC( "COS_Player", "SetPermissions", new Param1< ref array< string > >( m_Permissions ), true, NULL, GetSelectedPlayer() );

        layoutRoot.FindAnyWidget("PlayerPermsContainer").Enable( true );
    }

    void UpdateList()
    {
        Print("PlayerMenu::UpdateList");
       
        for ( int j = 0; j < m_PlayerList.Count(); j++ )
        {
            m_PlayerScriptList.RemoveChild( m_PlayerList[j].GetLayoutRoot() );
            m_PlayerList[j].GetLayoutRoot().Unlink();
        }
        m_PlayerList.Clear();
        
        ref array<PlayerIdentity> identities = new ref array<PlayerIdentity>;
        GetGame().GetPlayerIndentities( identities );
        
        ref array<Man> players = new ref array<Man>;
        GetGame().GetPlayers( players );

        for ( int i = 0; i < identities.Count(); i++ )
        {
            Widget permRow = GetGame().GetWorkspace().CreateWidgets( "COS/gui/layouts/player/PlayerRow.layout", m_PlayerScriptList );

            if ( permRow == NULL ) 
            {
                Print("Skipped player.");
                continue;
            }

            PlayerIdentity identity = identities[i]; // players.GetKey( i );
            Man player = NULL; //players.GetElement( i );

            for ( int k = 0; k < players.Count(); k++ )
            {
                if ( players[k].GetIdentity().GetId() == identity.GetId() )
                {
                    player = players[k];
                }
            }

            PlayerRow rowScript;
            permRow.GetScript( rowScript );

            if ( rowScript )
            {
                rowScript.SetPlayer( identity, player );

                rowScript.playerMenu = this;

                m_PlayerList.Insert( rowScript );
            }
        }
    }
}