class PlayerMenu extends Form
{
    ref array< ref PlayerRow >      m_PlayerList;

    ref GridSpacerWidget            m_PlayerScriptList;
    ref ButtonWidget                m_ReloadScriptButton;

    ref Widget                      m_ActionsWrapper;
    ref Widget                      m_ActionsForm;

    ref Widget                      m_PermissionsWrapper;
    ref Widget                      m_PermsContainer;
    ref ButtonWidget                m_SetPermissionsButton;
    ref ButtonWidget                m_PermissionsBackButton;

    bool                            m_ShouldUpdateList;
    bool                            m_CanUpdateList;

    ref Permission                  m_LoadedPermission;
    ref PermissionRow               m_PermissionUI;

    private bool                    m_PermissionsLoaded;

    ref UIActionText m_GUID;
    ref UIActionText m_Name;
    ref UIActionText m_Steam64ID;

    ref UIActionText m_PingMin;
    ref UIActionText m_PingMax;
    ref UIActionText m_PingAvg;

    ref UIActionEditableText m_Health;
    ref UIActionEditableText m_Blood;

    ref UIActionButton m_ModifyPermissions;
    ref UIActionButton m_BanPlayer;
    ref UIActionButton m_KickPlayer;

    void PlayerMenu()
    {
        m_CanUpdateList = true;
        m_ShouldUpdateList = false;

        m_PermissionsLoaded = false;

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

        m_ActionsForm = layoutRoot.FindAnyWidget("actions_form");
        m_ActionsWrapper = layoutRoot.FindAnyWidget("actions_wrapper");
        
        m_GUID = UIActionManager.CreateText( m_ActionsWrapper, "GUID: ", "" );
        m_Name = UIActionManager.CreateText( m_ActionsWrapper, "Name: ", "" );
        m_Steam64ID = UIActionManager.CreateText( m_ActionsWrapper, "Steam64: ", "" );

        m_PingMin = UIActionManager.CreateText( m_ActionsWrapper, "Ping Min: ", "" );
        m_PingMax = UIActionManager.CreateText( m_ActionsWrapper, "Ping Man: ", "" );
        m_PingAvg = UIActionManager.CreateText( m_ActionsWrapper, "Ping Avg: ", "" );

        m_Health = UIActionManager.CreateEditableText( m_ActionsWrapper, "Health: ", "", "Set", this, "Click_SetHealth" );
        m_Blood = UIActionManager.CreateEditableText( m_ActionsWrapper, "Blood: ", "", "Set", this, "Click_SetBlood" );

        m_ModifyPermissions = UIActionManager.CreateButton( m_ActionsWrapper, "Modify Permissions", this, "Click_ModifyPermissions" );
        m_BanPlayer = UIActionManager.CreateButton( m_ActionsWrapper, "Ban Player", this, "Click_BanPlayer" );
        m_KickPlayer = UIActionManager.CreateButton( m_ActionsWrapper, "Kick Player", this, "Click_KickPlayer" );

        m_PermissionsWrapper = layoutRoot.FindAnyWidget("permissions_wrapper");
        m_PermsContainer = layoutRoot.FindAnyWidget("permissions_container");
        m_SetPermissionsButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("permissions_set_button"));
        m_PermissionsBackButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("permissions_back_button"));

    }

    void Click_ModifyPermissions()
    {
        m_ActionsForm.Show( false );
        m_PermissionsWrapper.Show( true );
    }

    void Click_BanPlayer()
    {
        GetRPCManager().SendRPC( "COT_Admin", "BanPlayer", new Param1< ref array< ref PlayerData > >( SerializePlayers( GetSelectedPlayers() ) ), true );
    }

    void Click_KickPlayer()
    {
        GetRPCManager().SendRPC( "COT_Admin", "KickPlayer", new Param1< ref array< ref PlayerData > >( SerializePlayers( GetSelectedPlayers() ) ), true );
    }

    void Click_SetHealth( ref UIActionEditableText action )
    {
        string text = action.GetText();
    }

    void Click_SetBlood( ref UIActionEditableText action )
    {
        string text = action.GetText();
    }

    void UpdateActionsFields( ref PlayerData data )
    {
        m_GUID.SetText( data.SGUID );
        m_Name.SetText( data.SName );
        m_Steam64ID.SetText( data.SSteam64ID );

        m_PingMin.SetText( data.IPingMin.ToString() );
        m_PingMax.SetText( data.IPingMax.ToString() );
        m_PingAvg.SetText( data.IPingAvg.ToString() );
    }

    override void OnShow()
    {
        ReloadPlayers();

        if ( m_PermissionsLoaded == false )
        {
            SetupPermissionsUI();

            m_PermissionsLoaded = true;
        }

        m_PermissionsWrapper.Show( false );
        m_ActionsWrapper.Show( true );
        
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater( UpdateList, 1000, true );
    }

    override void OnHide() 
    {
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove( UpdateList );
    }

    void UpdateList()
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

        if ( w == m_PermissionsBackButton )
        {
            m_PermissionsWrapper.Show( false );
            m_ActionsForm.Show( true );
        }

        return false;
    }

    void OnPlayerSelected( PlayerRow row )
    {
        Print("PlayerMenu::OnPlayerSelected");
        
        int position = AddSelectedPlayer( row.GetPlayer() );

        if ( GetSelectedPlayers().Count() == 1 )
        {
            UpdateActionsFields( row.GetPlayer().GetData() );
        }

        if ( GetSelectedPlayers().Count() != 0 ) 
        {
            LoadPermissions( GetSelectedPlayers()[0].RootPermission );
        } else 
        {
            LoadPermissions( NULL );
        }
    }

    void ReloadPlayers()
    {
        GetRPCManager().SendRPC( "COT_Admin", "ReloadList", new Param, true );
    }

    void SetupPermissionsUI()
    {
        Print("PlayerMenu::SetupPermissionsUI");

        ref Permission rootPerm = GetPermissionsManager().GetRootPermission() 

        Widget permRow = GetGame().GetWorkspace().CreateWidgets( "COT/gui/layouts/player/permissions/PermissionRow.layout", m_PermsContainer );

        permRow.GetScript( m_PermissionUI );

        permRow.Show( false );
        permRow.SetSize( 0, 0 );

        if ( m_PermissionUI )
        {
            m_PermissionUI.Set( rootPerm, 0 );

            InitPermissionUIRow( rootPerm, 0, m_PermissionUI );
        }
    }

    void ResetPermissionUI()
    {
        m_PermissionUI.Disable();
    }

    ref array< string > SerializePermissionUI()
    {
        ref array< string > output = new ref array< string >;
        m_PermissionUI.Serialize( output );
        return output;
    }

    private void InitPermissionUIRow( ref Permission perm, int depth, ref PermissionRow parentRow )
    {
        for ( int i = 0; i < perm.Children.Count(); i++ )
        {
            ref Permission cPerm = perm.Children[i];

            Print( "Adding permission " + cPerm.Name );

            Widget permRow = GetGame().GetWorkspace().CreateWidgets( "COT/gui/layouts/player/permissions/PermissionRow.layout", m_PermsContainer );

            PermissionRow rowScript;
            permRow.GetScript( rowScript );

            if ( rowScript )
            {
                rowScript.Set( cPerm, depth );

                parentRow.Children.Insert( rowScript );
                rowScript.Parent = parentRow;

                InitPermissionUIRow( cPerm, depth + 1, rowScript );
            }
        }
    }

    void LoadPermissions( ref Permission permission )
    {
        Print("PlayerMenu::LoadPermissions");

        m_LoadedPermission = permission;

        if ( m_LoadedPermission == NULL )
        {
            LoadPermission( GetPermissionsManager().GetRootPermission(), m_PermissionUI );
        } else 
        {
            LoadPermission( m_LoadedPermission, m_PermissionUI );
        }
    }

    protected void LoadPermission( ref Permission perm, ref PermissionRow row  )
    {
        row.SetPermission( perm );

        for ( int i = 0; i < row.Children.Count(); i++ )
        {
            LoadPermission( perm.Children[i], row.Children[i] );
        }
    }

    void SetPermissions()
    {
        Print("PlayerMenu::SetPermissions");

        ref array< string > guids = new ref array< string >;

        ref array< ref AuthPlayer > players = GetSelectedPlayers();

        for ( int i = 0; i < players.Count(); i++ )
        {
            guids.Insert( players[i].GetGUID() );
        }

        GetRPCManager().SendRPC( "COT_Admin", "SetPermissions", new Param2< ref array< string >, ref array< string > >( SerializePermissionUI(), guids ), true );
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
            Widget permRow = GetGame().GetWorkspace().CreateWidgets( "COT/gui/layouts/player/PlayerRow.layout", m_PlayerScriptList );

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