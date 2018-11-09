class PlayerMenu extends PopupMenu
{
    ref array< ref PermissionRow >  m_Permissions;

    TextListboxWidget               m_PlayerScriptList;
    ButtonWidget                    m_ReloadScriptButton;
    GridSpacerWidget                m_PermsContainer;
    ButtonWidget                    m_SetPermissionsButton;

    void PlayerMenu()
    {
        m_Permissions = new ref array< ref PermissionRow >;
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

        m_PermsContainer        = GridSpacerWidget.Cast(layoutRoot.FindAnyWidget("perms_cont"));
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
    }

    void LoadPermissions( ref array< string > perms )
    {
        /*
        for ( int j = 0; j < m_Permissions.Count(); j++ )
        {
            m_Permissions[j].GetLayoutRoot().Close();
        }
        */

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

                bool value = false;

                if ( spaces[1].ToInt() == 1 )
                {
                    value = true;
                }

                rowScript.SetPermission( spaces[0], value );

                m_Permissions.Insert( rowScript );

                /*if ( i > 0 )
                {
                    m_PermsContainer.AddChildAfter( permRow, m_Permissions[i - 1].GetLayoutRoot() );
                } else
                {
                    m_PermsContainer.AddChildAfter( permRow, NULL );
                }*/
            }
        }
    }

    void SetPermissions()
    {
        // GetRPCManager().SendRPC( "COS_Player", "SetPermissions", new Param1< ref array< string > >( m_Permissions ), true, NULL, SELECTED_PLAYER );

        layoutRoot.FindAnyWidget("PlayerPermsContainer").Enable( true );
    }

    void UpdateList( ref array< Man > players )
    {
        if ( m_PlayerScriptList == NULL ) return;

        m_PlayerScriptList.ClearItems();

        for ( int i = 0; i < players.Count(); i++ )
        {
            Man player = players.Get( i );

            PlayerIdentity identity = player.GetIdentity();

            m_PlayerScriptList.AddItem( identity.GetName() + " (" + identity.GetId() + ")", player, 0, i );

            if ( identity.GetId() == GetGame().GetPlayer().GetIdentity().GetId() )
            {
                m_PlayerScriptList.SetItemColor( i, 0, COLOR_GREEN );
            }
        }
    }
}