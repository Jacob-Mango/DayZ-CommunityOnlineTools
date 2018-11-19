class GameMenu extends Form
{
    TextListboxWidget  m_GameScriptList;
    Widget             m_ActionsWrapper;
    ButtonWidget       m_GameScriptButton;

    void GameMenu()
    {
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
        m_GameScriptList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("game_spawn_box"));
        m_GameScriptButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("game_spawn_button"));

        m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );

        if ( GetPermissionsManager().HasPermission( "Game.SpawnVehicle" ) )
        {
            m_GameScriptList.AddItem("Spawn Hatchback", new Param1<string>("SpawnHatchback"), 0);
            m_GameScriptList.AddItem("Spawn V3S", new Param1<string>("SpawnV3S"), 0);
            m_GameScriptList.AddItem("Spawn V3S Cargo", new Param1<string>("SpawnV3SCargo"), 0);
            m_GameScriptList.AddItem("Spawn Sedan", new Param1<string>("SpawnSedan"), 0);
            // m_GameScriptList.AddItem("Spawn Van", new Param1<string>("SpawnVan"), 0);
        }

        if ( GetPermissionsManager().HasPermission( "Game.ChangeAimingMode" ) )
        {
            UIActionManager.CreateCheckbox( m_ActionsWrapper, "Enable 0.62 Aiming", this, "ToggleOldAiming", false );
        }
    }

    override void OnShow()
    {
    }

    override void OnHide() 
    {
        
    }

    void ToggleOldAiming( UIEvent eid, ref UIActionCheckbox action )
    {
        m_OldAiming = action.IsChecked();

        GetRPCManager().SendRPC( "COT_Game", "SetOldAiming", new Param1< bool >( m_OldAiming ), true );
    }

    override bool OnClick( Widget w, int x, int y, int button )
    {
        string param;
        Param1<string> param1;

        if ( w == m_GameScriptButton ) 
        {
            int selectRow = m_GameScriptList.GetSelectedRow();

            if ( selectRow == -1 ) return false;

            m_GameScriptList.GetItemData( selectRow, 0, param1 );

            if ( param1 )
            {
                GetGame().GameScript.CallFunction( this , param1.param1 , NULL, 0 );
            }
        }

        return false;
    }

    void SpawnVan() 
    {
        ref array< string> attArr = new ref array< string>;

        attArr.Insert("CivVanDoors_TrumpUp");
        attArr.Insert("CivVanDoors_BackRight");
        attArr.Insert("CivVanDoors_TrumpDown");
        attArr.Insert("CivVanDoors_CoDriver");
        attArr.Insert("CivVanTrunk");
        attArr.Insert("CivVanWheel");
        attArr.Insert("CivVanWheel");
        attArr.Insert("CivVanWheel");
        attArr.Insert("CivVanWheel");
        attArr.Insert("CarBattery");
        attArr.Insert("CarRadiator");
        attArr.Insert("EngineBelt");
        attArr.Insert("SparkPlug");

        SpawnVehicle( "CivilianVan", attArr );
    }

    void SpawnSedan() 
    {
        ref array< string> attArr = new ref array< string>;
        
        attArr.Insert("CivSedanHood");
        attArr.Insert("CivSedanTrunk");
        attArr.Insert("CivSedanDoors_Driver");
        attArr.Insert("CivSedanDoors_BackRight");
        attArr.Insert("CivSedanDoors_BackLeft");
        attArr.Insert("CivSedanDoors_CoDriver");
        attArr.Insert("CivSedanWheel");
        attArr.Insert("CivSedanWheel");
        attArr.Insert("CivSedanWheel");
        attArr.Insert("CivSedanWheel");
        attArr.Insert("CarBattery");
        attArr.Insert("CarRadiator");
        attArr.Insert("EngineBelt");
        attArr.Insert("SparkPlug");

        SpawnVehicle( "CivilianSedan", attArr );
    }

    void SpawnHatchback() 
    {
        ref array< string> attArr = new ref array< string>;

        attArr.Insert("HatchbackHood");
        attArr.Insert("HatchbackTrunk");
        attArr.Insert("HatchbackDoors_Driver");
        attArr.Insert("HatchbackDoors_CoDriver");
        attArr.Insert("HatchbackWheel");
        attArr.Insert("HatchbackWheel");
        attArr.Insert("HatchbackWheel");
        attArr.Insert("HatchbackWheel");
        attArr.Insert("CarBattery");
        attArr.Insert("CarRadiator");
        attArr.Insert("EngineBelt");
        attArr.Insert("SparkPlug");

        SpawnVehicle( "OffroadHatchback", attArr );
    }

    void SpawnV3SCargo() 
    {
        ref array< string> attArr = new ref array< string>;
        
        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("TruckBattery");
        attArr.Insert("TruckRadiator");
        attArr.Insert("EngineBelt");
        attArr.Insert("GlowPlug");
        attArr.Insert("V3SHood");
        attArr.Insert("V3SDoors_Driver_Blue");
        attArr.Insert("V3SDoors_CoDriver_Blue");

        SpawnVehicle( "V3S_Cargo_Blue", attArr );
    }

    void SpawnV3S() 
    {
        ref array< string> attArr = new ref array< string>;
        
        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("TruckBattery");
        attArr.Insert("TruckRadiator");
        attArr.Insert("EngineBelt");
        attArr.Insert("GlowPlug");
        attArr.Insert("V3SHood");
        attArr.Insert("V3SDoors_Driver_Blue");
        attArr.Insert("V3SDoors_CoDriver_Blue");

        SpawnVehicle( "V3S_Chassis_Blue", attArr );
    }

    void SpawnVehicle( string vehicle, ref array< string> attachments) 
    {
        GetRPCManager().SendRPC( "COT_Game", "SpawnVehicle", new Param3< string, vector, ref array< string> >( vehicle, GetCursorPos(), attachments ), true, NULL, GetGame().GetPlayer() );
    }
}