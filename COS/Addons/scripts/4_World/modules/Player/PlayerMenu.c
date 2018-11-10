class PlayerMenu extends PopupMenu
{
    ref array< ref PermissionRow >  m_Permissions;
    ref array< ref PlayerRow >      m_PlayerList;

    GridSpacerWidget                m_PlayerScriptList;
    ButtonWidget                    m_ReloadScriptButton;
    GridSpacerWidget                m_PermsContainer;
    ButtonWidget                    m_SetPermissionsButton;

    void PlayerMenu()
    {
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
    }

    override void OnHide() 
    {
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
        for ( int j = 0; j < m_PlayerList.Count(); j++ )
        {
            m_PlayerList[j].GetLayoutRoot().SetColor(0xFF000000);

            if ( m_PlayerList[j].m_Player.GetIdentity().GetId() == GetGame().GetPlayer().GetIdentity().GetId() )
            {
                row.GetLayoutRoot().SetColor(0x9900AA00);
            }
        }
        row.GetLayoutRoot().SetColor(0x99AAAAAA);

        SELECTED_PLAYER = row.m_Player;

        layoutRoot.FindAnyWidget("PlayerPermsContainer").Enable( false );

        GetRPCManager().SendRPC( "COS_Player", "LoadPermissions", new Param1<PlayerIdentity>( row.m_Identity ), true );
    }

    void ReloadPlayers()
    {
        GetRPCManager().SendRPC( "COS_Player", "ReloadList", new Param, true );
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

    void UpdateList( ref map< PlayerIdentity, Man > players )
    {
        for ( int j = 0; j < m_PlayerList.Count(); j++ )
        {
            m_PlayerList[j].GetLayoutRoot().Unlink();
        }

        m_PlayerList.Clear();

        MapIterator start = players.Begin();
        MapIterator end = players.End();
        MapIterator it = players.Begin();
        while ( it >= start && it < end )
        {
            PlayerIdentity identity = players.GetIteratorKey( it );
            Man player = players.GetIteratorElement( it );

            Widget permRow = GetGame().GetWorkspace().CreateWidgets( "COS/gui/layouts/player/PlayerRow.layout", m_PlayerScriptList );

            if ( permRow == NULL ) 
            {
                continue;
            }

            PlayerRow rowScript;
            permRow.GetScript( rowScript );

            if ( rowScript )
            {
                rowScript.SetPlayer( identity, player );

                rowScript.playerMenu = this;

                m_PlayerList.Insert( rowScript );
            }

            it = players.Next( it );
        }

        if ( m_PlayerList.Count() > 0 )
        {
            OnPlayerSelected( m_PlayerList[0] );
        }
    }
}