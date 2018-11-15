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

    override void Init()
    {
        m_GameScriptList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("game_list_box"));
        m_GameScriptButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("game_script_button"));

        m_GameScriptList.AddItem("Spawn Hatchback", new Param1<string>("SpawnHatchback"), 0);
        m_GameScriptList.AddItem("Spawn V3S",        new Param1<string>("SpawnV3S"),       0);

        m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );

        if ( GetPermissionsManager().HasPermission( "Game.ChangeAimingMode" ) )
        {
            UIActionManager.CreateCheckbox( m_ActionsWrapper, "Enable 0.62 Aiming", false, this, "ToggleOldAiming" );
        }
    }

    override void OnShow()
    {
    }

    override void OnHide() 
    {
        
    }

    void ToggleOldAiming( ref UIActionCheckbox action )
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