class PlayerMenu extends PopupMenu
{
    ref array< ref PermissionRow >  m_Permissions;
    ref array< ref PlayerRow >      m_PlayerList;

    GridSpacerWidget                m_PlayerScriptList;
    ButtonWidget                    m_ReloadScriptButton;

    Widget                          m_ActionsWrapper;
    ButtonWidget                    m_ActionModifyPermissions;

    Widget                          m_PermissionsWrapper;
    GridSpacerWidget                m_PermsContainer;
    ButtonWidget                    m_SetPermissionsButton;
    ButtonWidget                    m_PermissionsBackButton;

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
        m_PermsContainer = GridSpacerWidget.Cast(layoutRoot.FindAnyWidget("PermsListCont"));
        m_SetPermissionsButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("permissions_set_button"));
        m_PermissionsBackButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("permissions_back_button"));
    }

    override void OnShow()
    {
        ReloadPlayers();

        SetPermissionOptions( GetPermissionsManager().GetRootPermission() );

        m_PermissionsWrapper.Show( false );
        m_ActionsWrapper.Show( true );
        
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

        SELECTED_PLAYER = row.Player;

        layoutRoot.FindAnyWidget("PlayerPermsContainer").Enable( false );

        GetRPCManager().SendRPC( "COS_Admin", "LoadPermissions", new Param1<string>( SELECTED_PLAYER.GetGUID() ), true );
    }

    void ReloadPlayers()
    {
        GetRPCManager().SendRPC( "COS_Admin", "ReloadList", new Param, true );
    }

    void SetPermissionOptions( ref Permission permission )
    {
        Print("PlayerMenu::SetPermissionOptions");

        for ( int j = 0; j < m_Permissions.Count(); j++ )
        {
            m_Permissions[j].GetLayoutRoot().Unlink();
        }

        m_Permissions.Clear();

        AddPermissionRow( permission, 0 );
    }

    void AddPermissionRow( ref Permission perm, int depth )
    {
        for ( int i = 0; i < perm.Children.Count(); i++ )
        {
            ref Permission cPerm = perm.Children[i];

            Print( "Adding permission " + cPerm.Name );

            Widget permRow = GetGame().GetWorkspace().CreateWidgets( "COS/gui/layouts/player/PermissionRow.layout", m_PermsContainer );

            PermissionRow rowScript;
            permRow.GetScript( rowScript );

            if ( rowScript )
            {
                rowScript.Set( cPerm, depth );

                m_Permissions.Insert( rowScript );

                AddPermissionRow( cPerm, depth + 1 );
            }
        }
    }

    void LoadPermissions( ref array< string > perms )
    {
        Print("PlayerMenu::LoadPermissions");
        // child_perms

        /*

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

                if ( spaces[1] == "1" )
                {
                    value = true;
                }

                Print( spaces[0] + " is " + value );

                rowScript.SetPermission( spaces[0], value );

                m_Permissions.Insert( rowScript );
            }
        }

        */
    }

    void SetPermissions()
    {
        Print("PlayerMenu::SetPermissions");
        // GetRPCManager().SendRPC( "COS_Admin", "SetPermissions", new Param1< ref array< string > >( m_Permissions ), true, NULL, GetSelectedPlayer() );

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

        ref PlayerModule pm = PlayerModule.Cast( baseModule );

        for ( int i = 0; i < pm.Players.Count(); i++ )
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
                rowScript.SetPlayer( pm.Players[i] );

                rowScript.playerMenu = this;

                m_PlayerList.Insert( rowScript );
            }
        }
    }
}