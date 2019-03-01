class GameMenu extends Form
{
    protected Widget m_ActionsWrapper;

    protected ref array< ref UIActionButton > m_VehicleButtons;
    protected ref array< ref UIActionButton > m_BaseBuildingButtons;

    void GameMenu()
    {
        m_VehicleButtons = new ref array< ref UIActionButton >;
        m_BaseBuildingButtons = new ref array< ref UIActionButton >;
    }

    void ~GameMenu()
    {
    }

    override string GetTitle()
    {
        return "Gameplay and World";
    }
    
    override string GetIconName()
    {
        return "G";
    }

    override bool ImageIsIcon()
    {
        return false;
    }

    override void OnInit( bool fromMenu )
    {
        m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );

        UIActionManager.CreateCheckbox( m_ActionsWrapper, "Enable 0.62 Aiming", this, "ToggleOldAiming", false );
        // UIActionManager.CreateButton( m_ActionsWrapper, "Throw Apple", this, "ThrowApple" );

    }

    override void OnShow()
    {
        GameModule gm = GameModule.Cast( module );

        if ( gm == NULL ) return;

        string name;
        ref UIActionButton button;

        for ( int i = 0; i < gm.GetVehicles().Count(); i++ )
        {
            name = gm.GetVehicles()[i];

            button = UIActionManager.CreateButton( m_ActionsWrapper, "Spawn " + name + " at Cursor", this, "SpawnVehicle" );
            button.SetData( new GM_Button_Data( name ) );

            m_VehicleButtons.Insert( button );
        }

        for ( int j = 0; j < gm.GetBaseBuilding().Count(); j++ )
        {
            name = gm.GetBaseBuilding()[j];

            button = UIActionManager.CreateButton( m_ActionsWrapper, "Spawn " + name + " on Selected Player(s)", this, "SpawnBaseBuilding" );
            button.SetData( new GM_Button_Data( name ) );

            m_BaseBuildingButtons.Insert( button );
        }        
    }

    override void OnHide() 
    {
        for ( int j = 0; j < m_VehicleButtons.Count(); j++ )
        {
            m_ActionsWrapper.RemoveChild( m_VehicleButtons[j].GetLayoutRoot() );
            m_VehicleButtons[j].GetLayoutRoot().Unlink()
        }

        m_VehicleButtons.Clear();

        for ( int k = 0; k < m_BaseBuildingButtons.Count(); k++ )
        {
            m_ActionsWrapper.RemoveChild( m_BaseBuildingButtons[k].GetLayoutRoot() );
            m_BaseBuildingButtons[k].GetLayoutRoot().Unlink()
        }

        m_BaseBuildingButtons.Clear();
    }

    void ThrowApple( UIEvent eid, ref UIActionButton action )
    {
        GetRPCManager().SendRPC( "COT_Game", "ThrowApple", new Param, true, NULL, GetPlayer() );
    }

    void ToggleOldAiming( UIEvent eid, ref UIActionCheckbox action )
    {
        m_OldAiming = action.IsChecked();

        GetRPCManager().SendRPC( "COT_Game", "SetOldAiming", new Param1< bool >( m_OldAiming ), true );
    }

    void SpawnVehicle( UIEvent eid, ref UIActionButton action ) 
    {
        ref GM_Button_Data data = GM_Button_Data.Cast( action.GetData() );
        if ( !data ) return;

        GetRPCManager().SendRPC( "COT_Game", "SpawnVehicle", new Param2< string, vector >( data.ClassName, GetCursorPos() ), true, NULL, GetGame().GetPlayer() );
    }

    void SpawnBaseBuilding( UIEvent eid, ref UIActionButton action ) 
    {
        ref GM_Button_Data data = GM_Button_Data.Cast( action.GetData() );
        if ( !data ) return;

        GetRPCManager().SendRPC( "COT_Game", "SpawnBaseBuilding", new Param2< string, ref array< string > >( data.ClassName, SerializePlayersID( GetSelectedPlayers() ) ), true );
    }
}