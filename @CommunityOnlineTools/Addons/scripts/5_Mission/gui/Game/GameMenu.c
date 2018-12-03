class GameMenu extends Form
{
    protected Widget m_ActionsWrapper;

    protected ref array< ref UIActionButton > m_VehicleButtons;

    void GameMenu()
    {
        m_VehicleButtons = new ref array< ref UIActionButton >;
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
        GameModule module = GameModule.Cast( module );

        if ( module == NULL ) return;

        Print( "gamemenu: count " + module.GetVehicles().Count() );

        for ( int i = 0; i < module.GetVehicles().Count(); i++ )
        {
            string name = module.GetVehicles().GetKey( i );

            Print( "Name: " + name );

            GM_VehicleData data = new GM_VehicleData;
            data.ClassName = name;

            ref UIActionButton button = UIActionManager.CreateButton( m_ActionsWrapper, "Spawn " + name, this, "SpawnVehicle" );
            button.SetData( data );

            m_VehicleButtons.Insert( button );
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
        GM_VehicleData data = GM_VehicleData.Cast( action.GetData() );
        if ( !data ) return;

        GetRPCManager().SendRPC( "COT_Game", "SpawnVehicle", new Param2< string, vector >( data.ClassName, GetCursorPos() ), true, NULL, GetGame().GetPlayer() );
    }
}