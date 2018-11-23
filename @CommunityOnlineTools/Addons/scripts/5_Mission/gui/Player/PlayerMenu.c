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

    private bool                    m_DataJustUpdated;

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
    ref UIActionEditableText m_Shock;
    ref UIActionEditableText m_HeatComfort;
    ref UIActionEditableText m_Wet;
    ref UIActionEditableText m_Tremor;
    ref UIActionEditableText m_Stamina;
    ref UIActionEditableText m_LastShaved;
    ref UIActionCheckbox m_BloodyHands;

    ref UIActionButton m_KickTransport;
    ref UIActionButton m_RepairTransport;
    ref UIActionButton m_TeleportToMe;
    ref UIActionButton m_TeleportMeTo;


    ref UIActionButton m_ModifyPermissions;
    ref UIActionButton m_BanPlayer;
    ref UIActionButton m_KickPlayer;
    ref UIActionCheckbox m_Freecam;
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

        ref Widget playerActions = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 8, 2 );
        m_Health = UIActionManager.CreateEditableText( playerActions, "Health: ", this, "Click_SetHealth", "", "Set" );
        m_Shock = UIActionManager.CreateEditableText( playerActions, "Shock: ", this, "Click_SetShock", "", "Set" );
        m_Blood = UIActionManager.CreateEditableText( playerActions, "Blood: ", this, "Click_SetBlood", "", "Set" );
        m_Energy = UIActionManager.CreateEditableText( playerActions, "Energy: ", this, "Click_SetEnergy", "", "Set" );
        m_Water = UIActionManager.CreateEditableText( playerActions, "Water: ", this, "Click_SetWater", "", "Set" );
        m_HeatComfort = UIActionManager.CreateEditableText( playerActions, "Temp Change: ", this, "Click_SetHeatComfort", "", "Set" );
        m_Wet = UIActionManager.CreateEditableText( playerActions, "Wet: ", this, "Click_SetWet", "", "Set" );
        m_Tremor = UIActionManager.CreateEditableText( playerActions, "Tremor: ", this, "Click_SetTremor", "", "Set" );
        m_Stamina = UIActionManager.CreateEditableText( playerActions, "Stamina: ", this, "Click_SetStamina", "", "Set" );
        m_LastShaved = UIActionManager.CreateEditableText( playerActions, "Beard: ", this, "Click_SetLifeSpanState", "", "Set" ); // TODO: make select box action box instead
        m_BloodyHands = UIActionManager.CreateCheckbox( playerActions, "Bloody Hands: ", this, "Click_SetBloodyHands", false );
        m_KickTransport = UIActionManager.CreateButton( playerActions, "Kick Transport", this, "Click_KickTransport" );
        m_RepairTransport = UIActionManager.CreateButton( playerActions, "Repair Transport", this, "Click_RepairTransport" );
        m_TeleportToMe = UIActionManager.CreateButton( playerActions, "Teleport To Me", this, "Click_TeleportToMe" );
        m_TeleportMeTo = UIActionManager.CreateButton( playerActions, "Teleport Me To", this, "Click_TeleportMeTo" );

        ref Widget serverActions = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 2, 2 );
        m_ModifyPermissions = UIActionManager.CreateButton( serverActions, "Modify Permissions", this, "Click_ModifyPermissions" );
        m_Freecam = UIActionManager.CreateCheckbox( serverActions, "Freecam", this, "Click_ToggleFreecam", false );
        m_GodMode = UIActionManager.CreateCheckbox( serverActions, "Godmode", this, "Click_GodMode", false );

        //m_BanPlayer = UIActionManager.CreateButton( serverActions, "Ban Player", this, "Click_BanPlayer" );
        //m_KickPlayer = UIActionManager.CreateButton( serverActions, "Kick Player", this, "Click_KickPlayer" );

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
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "BanPlayer", new Param1< ref array< string > >( SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_KickPlayer( UIEvent eid, ref UIActionButton action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "KickPlayer", new Param1< ref array< string > >( SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_KickTransport( UIEvent eid, ref UIActionButton action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "Player_KickTransport", new Param1< ref array< string > >( SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_RepairTransport( UIEvent eid, ref UIActionButton action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "Player_RepairTransport", new Param1< ref array< string > >( SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }
    
    void Click_TeleportToMe( UIEvent eid, ref UIActionButton action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;

        if ( CurrentActiveCamera && CurrentActiveCamera.IsActive() )
        {
            GetRPCManager().SendRPC( "COT_Admin", "Player_TeleportToMe", new Param2< vector, ref array< string > >( CurrentActiveCamera.GetPosition(), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
        } else 
        {
            GetRPCManager().SendRPC( "COT_Admin", "Player_TeleportToMe", new Param2< vector, ref array< string > >( GetPlayer().GetPosition(), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
        }
    }

    void Click_TeleportMeTo( UIEvent eid, ref UIActionButton action )
    {
        if ( GetSelectedPlayers().Count() != 1 ) return;

        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;

        GetRPCManager().SendRPC( "COT_Admin", "Player_TeleportMeTo", new Param1< ref array< string > >( SerializePlayersGUID( GetSelectedPlayers() ) ), true, NULL, GetPlayer() );
    }

    void Click_SetHealth( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "Player_SetHealth", new Param2< float, ref array< string > >( ToFloat( action.GetText() ), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_SetShock( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "Player_SetShock", new Param2< float, ref array< string > >( ToFloat( action.GetText() ), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_SetBlood( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "Player_SetBlood", new Param2< float, ref array< string > >( ToFloat( action.GetText() ), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_SetEnergy( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "Player_SetEnergy", new Param2< float, ref array< string > >( ToFloat( action.GetText() ), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_SetWater( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "Player_SetWater", new Param2< float, ref array< string > >( ToFloat( action.GetText() ), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_SetHeatComfort( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "Player_SetHeatComfort", new Param2< float, ref array< string > >( ToFloat( action.GetText() ), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_SetWet( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "Player_SetWet", new Param2< float, ref array< string > >( ToFloat( action.GetText() ), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_SetTremor( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "Player_SetTremor", new Param2< float, ref array< string > >( ToFloat( action.GetText() ), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_SetStamina( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "Player_SetStamina", new Param2< float, ref array< string > >( ToFloat( action.GetText() ), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_SetLifeSpanState( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;

        if ( action.GetText().Length() != 1 ) return;

        int state = ToSingleDigit( action.GetText().Get( 0 ) );

        if ( state >= LifeSpanState.BEARD_NONE && state < LifeSpanState.COUNT )
        {
            m_DataJustUpdated = true;
            GetRPCManager().SendRPC( "COT_Admin", "Player_SetLifeSpanState", new Param2< int, ref array< string > >( state, SerializePlayersGUID( GetSelectedPlayers() ) ), true );
        }
    }

    void Click_SetBloodyHands( UIEvent eid, ref UIActionCheckbox action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "Player_SetBloodyHands", new Param2< float, ref array< string > >( action.IsChecked(), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_ToggleFreecam( UIEvent eid, ref UIActionCheckbox action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "ToggleFreecam", new Param2< bool, ref array< string > >( action.IsChecked(), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void Click_GodMode( UIEvent eid, ref UIActionCheckbox action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_DataJustUpdated = true;
        GetRPCManager().SendRPC( "COT_Admin", "GodMode", new Param2< bool, ref array< string > >( action.IsChecked(), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void UpdateActionsFields( ref PlayerData data )
    {
        if ( m_DataJustUpdated )
        {
            m_DataJustUpdated = false;
            return;
        }

        if ( data )
        {
            m_GUID.SetText( data.SGUID );
            m_Name.SetText( data.SName );
            m_Steam64ID.SetText( data.SSteam64ID );

            m_Health.SetText( data.FHealth.ToString() );
            m_Blood.SetText( data.FBlood.ToString() );
            m_Energy.SetText( data.FEnergy.ToString() );
            m_Water.SetText( data.FWater.ToString() );
            m_Shock.SetText( data.FShock.ToString() );
            m_HeatComfort.SetText( data.FHeatComfort.ToString() );
            m_Wet.SetText( data.FWet.ToString() );
            m_Tremor.SetText( data.FTremor.ToString() );
            m_Stamina.SetText( data.FStamina.ToString() );
            m_LastShaved.SetText( data.ILifeSpanState.ToString() );
            m_BloodyHands.SetChecked( data.BBloodyHands );

            m_PingMin.SetText( data.IPingMin.ToString() );
            m_PingMax.SetText( data.IPingMax.ToString() );
            m_PingAvg.SetText( data.IPingAvg.ToString() );

            m_ActionsForm.FindAnyWidget( "disabled" ).Show( false );
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
            UpdateActionsFields( GetSelectedPlayers()[0].Data );
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
                    UpdateActionsFields( row.GetPlayer().Data );

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
                        UpdateActionsFields( GetSelectedPlayers()[0].Data );
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
        GetRPCManager().SendRPC( "COT_Admin", "UpdatePlayers", new Param, true );
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
            UpdateActionsFields( GetSelectedPlayers()[0].Data );
        }
    }
}