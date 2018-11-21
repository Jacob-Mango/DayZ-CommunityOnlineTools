class PlayerMenu extends Form
{
    ref array< ref PlayerRow >      m_PlayerList;

    ref GridSpacerWidget            m_PlayerScriptList;

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
    ref UIActionEditableText m_Energy;
    ref UIActionEditableText m_Water;

    ref UIActionButton m_ModifyPermissions;
    ref UIActionButton m_BanPlayer;
    ref UIActionButton m_KickPlayer;

    ref UIActionCheckbox m_GodMode;

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

    override void OnInit( bool fromMenu )
    {
        m_PlayerScriptList = GridSpacerWidget.Cast(layoutRoot.FindAnyWidget("player_list"));

        m_ActionsForm = layoutRoot.FindAnyWidget("actions_form");
        m_ActionsWrapper = layoutRoot.FindAnyWidget("actions_wrapper");
        
        m_GUID = UIActionManager.CreateText( m_ActionsWrapper, "GUID: ", "" );
        m_Name = UIActionManager.CreateText( m_ActionsWrapper, "Name: ", "" );
        m_Steam64ID = UIActionManager.CreateText( m_ActionsWrapper, "Steam64: ", "" );

        ref Widget pings = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 1, 3 );
        m_PingMin = UIActionManager.CreateText( pings, "Ping Min: ", "" );
        m_PingMax = UIActionManager.CreateText( pings, "Ping Max: ", "" );
        m_PingAvg = UIActionManager.CreateText( pings, "Ping Avg: ", "" );

        ref Widget lifeStats = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 2, 2 );
        m_Health = UIActionManager.CreateEditableText( lifeStats, "Health: ", this, "Click_SetHealth", "", "Set" );
        m_Blood = UIActionManager.CreateEditableText( lifeStats, "Blood: ", this, "Click_SetBlood", "", "Set" );
        m_Energy = UIActionManager.CreateEditableText( lifeStats, "Energy: ", this, "Click_SetEnergy", "", "Set" );
        m_Water = UIActionManager.CreateEditableText( lifeStats, "Water: ", this, "Click_SetWater", "", "Set" );

        ref Widget playerActions = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 1, 3 );
        m_ModifyPermissions = UIActionManager.CreateButton( playerActions, "Modify Permissions", this, "Click_ModifyPermissions" );
        m_BanPlayer = UIActionManager.CreateButton( playerActions, "Ban Player", this, "Click_BanPlayer" );
        m_KickPlayer = UIActionManager.CreateButton( playerActions, "Kick Player", this, "Click_KickPlayer" );

        m_GodMode = UIActionManager.CreateCheckbox( m_ActionsWrapper, "Godmode", this, "Click_GodMode", false );

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

    void Click_BanPlayer( UIEvent eid, ref UIActionButton action )
    {
        GetRPCManager().SendRPC( "COT_Admin", "BanPlayer", new Param1< ref array< string > >( SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_KickPlayer( UIEvent eid, ref UIActionButton action )
    {
        GetRPCManager().SendRPC( "COT_Admin", "KickPlayer", new Param1< ref array< string > >( SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    float ToFloat( string text )
    {
        float f = text.ToFloat();
        int i = text.ToInt();

        if ( f >= i ) return f;

        return i * 1.0;
    }

    void Click_SetHealth( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;

        GetRPCManager().SendRPC( "COT_Admin", "Player_SetHealth", new Param2< float, ref array< string > >( ToFloat( action.GetText() ), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_SetBlood( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;

        GetRPCManager().SendRPC( "COT_Admin", "Player_SetBlood", new Param2< float, ref array< string > >( ToFloat( action.GetText() ), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_SetEnergy( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;

        GetRPCManager().SendRPC( "COT_Admin", "Player_SetEnergy", new Param2< float, ref array< string > >( ToFloat( action.GetText() ), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_SetWater( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;

        GetRPCManager().SendRPC( "COT_Admin", "Player_SetWater", new Param2< float, ref array< string > >( ToFloat( action.GetText() ), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_GodMode( UIEvent eid, ref UIActionCheckbox action )
    {
        GetRPCManager().SendRPC( "COT_Admin", "GodMode", new Param2< bool, ref array< string > >( action.IsChecked(), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void UpdateActionsFields( ref PlayerData data )
    {
        if ( data )
        {
            m_GUID.SetText( data.SGUID );
            m_Name.SetText( data.SName );
            m_Steam64ID.SetText( data.SSteam64ID );

            m_Health.SetText( data.FHealth.ToString() );
            m_Blood.SetText( data.FBlood.ToString() );
            m_Energy.SetText( data.FEnergy.ToString() );
            m_Water.SetText( data.FWater.ToString() );

            m_PingMin.SetText( data.IPingMin.ToString() );
            m_PingMax.SetText( data.IPingMax.ToString() );
            m_PingAvg.SetText( data.IPingAvg.ToString() );

            m_ActionsForm.FindAnyWidget("disabled").Show( false );
        } else 
        {
            m_GUID.SetText( "" );
            m_Name.SetText( "" );
            m_Steam64ID.SetText( "" );

            m_Health.SetText( "" );
            m_Blood.SetText( "" );
            m_Energy.SetText( "" );
            m_Water.SetText( "" );

            m_PingMin.SetText( "" );
            m_PingMax.SetText( "" );
            m_PingAvg.SetText( "" );

            m_ActionsForm.Show( true );
            m_PermissionsWrapper.Show( false );

            m_ActionsForm.FindAnyWidget("disabled").Show( true );
        }
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

        if ( GetSelectedPlayers().Count() != 0 )
        {
            UpdateActionsFields( GetSelectedPlayers()[0].GetData() );
        } else
        {
            UpdateActionsFields( NULL );
        }

        GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).CallLater( UpdateList, 1000, true );
    }

    override void OnHide() 
    {
        GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).Remove( UpdateList );
    }

    void UpdateList()
    {
        ReloadPlayers();

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

    void OnPlayer_Checked( ref PlayerRow row )
    {
        OnPlayerSelected( row, row.Checkbox.IsChecked() );
    }

    void OnPlayer_Button( ref PlayerRow row )
    {
        OnPlayerSelected( NULL );

        if ( OnPlayerSelected( row ) )
        {
            row.Checkbox.SetChecked( true );
        }
    }

    bool OnPlayerSelected( ref PlayerRow row, bool select = true )
    {
        if ( row == NULL )
        {
            UpdateActionsFields( NULL );

            GetSelectedPlayers().Clear();

            for ( int i = 0; i < m_PlayerList.Count(); i++ )
            {
                m_PlayerList[i].Checkbox.SetChecked( false );
            }

            LoadPermissions( NULL );

            return false;
        } else 
        {
            int position = -1;

            if ( select )
            {
                position = AddSelectedPlayer( row.GetPlayer() );

                if ( GetSelectedPlayers().Count() == 1 )
                {
                    UpdateActionsFields( row.GetPlayer().GetData() );

                    LoadPermissions( GetSelectedPlayers()[0].RootPermission );
                }

                row.Checkbox.SetChecked( true );

                return true;
            } else
            {
                position = RemoveSelectedPlayer( row.GetPlayer() );

                if ( position == 0 )
                {
                    if (GetSelectedPlayers().Count() > 0 )
                    {
                        UpdateActionsFields( GetSelectedPlayers()[0].GetData() );
                        LoadPermissions( GetSelectedPlayers()[0].RootPermission );
                    } else 
                    {
                        UpdateActionsFields( NULL );
                        LoadPermissions( NULL );
                    }
                }

                row.Checkbox.SetChecked( false );

                return true;
            }
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

            ref PermissionRow rowScript;
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

        if ( permission == NULL )
        {
            LoadPermission( GetPermissionsManager().GetRootPermission(), m_PermissionUI, false );
        } else 
        {
            LoadPermission( permission, m_PermissionUI, true );
        }
    }

    protected void LoadPermission( ref Permission perm, ref PermissionRow row, bool enabled  )
    {
        if ( enabled )
        {
            row.SetPermission( perm );
        } else 
        {
            row.SetPermission( NULL );
        }

        for ( int i = 0; i < row.Children.Count(); i++ )
        {
            LoadPermission( perm.Children[i], row.Children[i], enabled );
        }
    }

    void SetPermissions()
    {
        Print("PlayerMenu::SetPermissions");

        GetRPCManager().SendRPC( "COT_Admin", "SetPermissions", new Param2< ref array< string >, ref array< string > >( SerializePermissionUI(), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void UpdatePlayerList()
    {
        Print("PlayerMenu::UpdatePlayerList");
       
        array< ref AuthPlayer > players = GetPermissionsManager().GetPlayers();

        for ( int i = 0; i < players.Count(); i++ )
        {
            bool exists = false;

            ref PlayerRow rowScript;

            for ( int j = 0; j < m_PlayerList.Count(); j++ )
            {
                if ( m_PlayerList[j].Player.GetGUID() == players[i].GetGUID() )
                {
                    rowScript = m_PlayerList[j];
                    exists = true;
                    break;
                }
            }

            if ( exists )
            {
                rowScript.SetPlayer( players[i] );
            } else
            {
                Widget permRow = GetGame().GetWorkspace().CreateWidgets( "COT/gui/layouts/player/PlayerRow.layout", m_PlayerScriptList );

                permRow.GetScript( rowScript );
                
                rowScript.SetPlayer( players[i] );

                rowScript.Menu = this;

                m_PlayerList.Insert( rowScript );

                if ( PlayerAlreadySelected( players[i] ) )
                {
                    OnPlayerSelected( rowScript, true );
                }
            }
        }

        int max = m_PlayerList.Count();
        
        for ( int k = 0; k < max; k++ )
        {
            bool found = false;

            for ( int l = 0; l < players.Count(); l++ )
            {
                if ( m_PlayerList[k].Player.GetGUID() == players[l].GetGUID() )
                {
                    found = true;
                    break;
                }
            }

            if ( !found )
            {
                m_PlayerScriptList.RemoveChild( m_PlayerList[k].GetLayoutRoot() );
                m_PlayerList[k].GetLayoutRoot().Unlink();

                m_PlayerList.Remove( k );
                k--;
                max = m_PlayerList.Count();
            }
        }

        
        if (GetSelectedPlayers().Count() > 0 )
        {
            UpdateActionsFields( GetSelectedPlayers()[0].GetData() );
        }
    }
}