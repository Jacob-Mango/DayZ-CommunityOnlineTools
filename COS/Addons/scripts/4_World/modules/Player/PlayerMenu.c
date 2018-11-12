class PlayerMenu extends PopupMenu
{
    ref array< ref PlayerRow >      m_PlayerList;

    ref GridSpacerWidget            m_PlayerScriptList;
    ref ButtonWidget                m_ReloadScriptButton;

    ref Widget                      m_ActionsWrapper;
    ref ButtonWidget                m_ActionModifyPermissions;

    ref Widget                      m_PermissionsWrapper;
    ref Widget                      m_PermsContainer;
    ref ButtonWidget                m_SetPermissionsButton;
    ref ButtonWidget                m_PermissionsBackButton;

    bool                            m_ShouldUpdateList;
    bool                            m_CanUpdateList;

    ref Permission                  m_LoadedPermission;
    ref PermissionRow               m_PermissionUI;

    void PlayerMenu()
    {
        m_CanUpdateList = true;
        m_ShouldUpdateList = false;

        m_PlayerList = new ref array< ref PlayerRow >;
    }

    void ~PlayerMenu()
    {
    }

    override string GetTitle()
    {
        return "Player Management";
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
        m_PlayerScriptList = GridSpacerWidget.Cast(layoutRoot.FindAnyWidget("player_list"));
        m_ReloadScriptButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("refresh_list"));

        m_ActionsWrapper = layoutRoot.FindAnyWidget("actions_wrapper");
        m_ActionModifyPermissions = ButtonWidget.Cast(layoutRoot.FindAnyWidget("actions_modify_permissions"));

        m_PermissionsWrapper = layoutRoot.FindAnyWidget("permissions_wrapper");
        m_PermsContainer = layoutRoot.FindAnyWidget("permissions_container");
        m_SetPermissionsButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("permissions_set_button"));
        m_PermissionsBackButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("permissions_back_button"));
    }

    override void OnShow()
    {
        ReloadPlayers();

        SetPermissionOptions( GetPermissionsManager().GetRootPermission() );

        m_PermissionsWrapper.Show( false );
        m_ActionsWrapper.Show( true );
        
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater( UpdateLists, 1000, true );
    }

    override void OnHide() 
    {
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove( UpdateLists );
    }

    void UpdateLists()
    {
        if ( m_ShouldUpdateList && m_CanUpdateList )
        {
            m_CanUpdateList = false;
            m_ShouldUpdateList = false;
            UpdatePlayerList();
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

        if ( w == m_ActionModifyPermissions )
        {
            m_ActionsWrapper.Show( false );
            m_PermissionsWrapper.Show( true );
        }

        if ( w == m_PermissionsBackButton )
        {
            m_PermissionsWrapper.Show( false );
            m_ActionsWrapper.Show( true );
        }

        return false;
    }

    void OnPlayerSelected( PlayerRow row )
    {
        Print("PlayerMenu::OnPlayerSelected");
        
        int position = AddSelectedPlayer( row.GetPlayer() );

        layoutRoot.FindAnyWidget("PlayerPermsContainer").Enable( false );

        if ( GetSelectedPlayers().Count() != 1 ) return;

        GetRPCManager().SendRPC( "COS_Admin", "LoadPermissions", new Param1<string>( row.GetPlayer().GetGUID() ), true );
    }

    void ReloadPlayers()
    {
        GetRPCManager().SendRPC( "COS_Admin", "ReloadList", new Param, true );
    }

    void SetPermissionOptions( ref Permission permission )
    {
        Print("PlayerMenu::SetPermissionOptions");

        AddPermissionRow( permission, 0, m_PermsContainer );
    }

    void AddPermissionRow( ref Permission perm, int depth, ref Widget parent )
    {
        for ( int i = 0; i < perm.Children.Count(); i++ )
        {
            ref Permission cPerm = perm.Children[i];

            Print( "Adding permission " + cPerm.Name );

            Widget permRow = GetGame().GetWorkspace().CreateWidgets( "COS/gui/layouts/player/permissions/PermissionRow.layout", parent );

            PermissionRow rowScript;
            permRow.GetScript( rowScript );

            if ( rowScript )
            {
                rowScript.Set( cPerm, depth );

                AddPermissionRow( cPerm, depth + 1, parent );
            }
        }
    }

    void LoadPermissions( ref Permission permission )
    {
        Print("PlayerMenu::LoadPermissions");

        m_LoadedPermission = permission;

        LoadPermission( m_LoadedPermission, m_PermissionUI );
    }

    protected void LoadPermission( ref Permission perm, ref PermissionRow row  )
    {
        if ( row.Children.Count() != perm.Children.Count() )
        {
            Print( "ERROR: Permissions aren't the same. This indicates a mod mismatch." );
            return
        }

        for ( int i = 0; i < row.Children.Count(); i++ )
        {
            row.Children[i].SetPermission( perm.Children[i] );

            LoadPermission( perm.Children[i], row.Children[i] );
        }
    }

    void SetPermissions()
    {
        Print("PlayerMenu::SetPermissions");

        ref array< string > output = new ref array< string >;

        m_LoadedPermission.Serialize( output );

        if ( GetSelectedPlayers().Count() == 0 ) return;

        GetRPCManager().SendRPC( "COS_Admin", "SetPermissions", new Param2< ref array< string >, ref array< ref PlayerData > >( output, SerializePlayers( GetSelectedPlayers() ) ), true );
    }

    void UpdatePlayerList()
    {
        Print("PlayerMenu::UpdatePlayerList");
       
        for ( int j = 0; j < m_PlayerList.Count(); j++ )
        {
            m_PlayerScriptList.RemoveChild( m_PlayerList[j].GetLayoutRoot() );
            m_PlayerList[j].GetLayoutRoot().Unlink();
        }

        m_PlayerList.Clear();

        ref array< ref AuthPlayer > players = GetPermissionsManager().GetPlayers();

        for ( int i = 0; i < players.Count(); i++ )
        {
            Widget permRow = GetGame().GetWorkspace().CreateWidgets( "COS/gui/layouts/player/PlayerRow.layout", m_PlayerScriptList );

            if ( permRow == NULL ) 
            {
                Print("Skipped player.");
                continue;
            }

            PlayerRow rowScript;
            permRow.GetScript( rowScript );
            
            if ( rowScript )
            {
                rowScript.SetPlayer( players[i] );

                rowScript.playerMenu = this;

                m_PlayerList.Insert( rowScript );
            }
        }
    }
}