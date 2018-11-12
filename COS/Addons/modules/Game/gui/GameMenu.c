class GameMenu extends PopupMenu
{
    TextListboxWidget m_gameScriptList;
    Widget               m_checkboxPanel;
    ButtonWidget       m_gameScriptButton;

    protected ref map< string, string > checkBoxMap = new map< string, string >; // store widget name
    protected ref map< string, string > buttonMap = new map< string, string >; // store widget name

    string checkboxLayout = "COS/Modules/Game/gui/layouts/Checkbox.layout";
    string buttonLayout = "COS/Modules/Game/gui/layouts/Button.layout";

    void GameMenu()
    {
    }

    void ~GameMenu()
    {
    }

    override string GetTitle()
    {
        return "Gameplay";
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
        m_checkboxPanel = layoutRoot.FindAnyWidget("game_checkbox_panel");
        m_gameScriptList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("game_list_box"));
        m_gameScriptButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("game_script_button"));

        m_gameScriptList.AddItem("Spawn Hatchback", new Param1<string>("SpawnHatchback"), 0);
//        m_gameScriptList.AddItem("Spawn Sedan",     new Param1<string>("SpawnSedan"),       0);
//        m_gameScriptList.AddItem("Spawn V3S_Cargo", new Param1<string>("SpawnV3SCargo"),  0);
        m_gameScriptList.AddItem("Spawn V3S",        new Param1<string>("SpawnV3S"),       0);
//        m_gameScriptList.AddItem("Spawn Bus",         new Param1<string>("SpawnBus"),       0);
//        m_gameScriptList.AddItem("Spawn Van",        new Param1<string>("SpawnVan"),       0);

        CheckBoxWidget checkBoxGodmode = CheckBoxWidget.Cast(GetGame().GetWorkspace().CreateWidgets( checkboxLayout, m_checkboxPanel ));
        checkBoxGodmode.SetName("Godmode");
        checkBoxGodmode.SetText("Godmode");

        CheckBoxWidget checkBoxAiming = CheckBoxWidget.Cast(GetGame().GetWorkspace().CreateWidgets( checkboxLayout, m_checkboxPanel ));
        checkBoxAiming.SetName("OldAiming");
        checkBoxAiming.SetText("Old Aiming");

        CheckBoxWidget checkBoxKP = CheckBoxWidget.Cast(GetGame().GetWorkspace().CreateWidgets( checkboxLayout, m_checkboxPanel ));
        checkBoxKP.SetName("KillPlayer");
        checkBoxKP.SetText("Kill Player");

        checkBoxMap.Insert( checkBoxGodmode.GetName(), "ToggleGodMode" );
        checkBoxMap.Insert( checkBoxAiming.GetName(), "ToggleOldAiming" );
        checkBoxMap.Insert( checkBoxKP.GetName(), "KillPlayer" );

        //ButtonWidget buttonKP = ButtonWidget.Cast(GetGame().GetWorkspace().CreateWidgets( buttonLayout, m_checkboxPanel ));
        //buttonKP.SetName("KillPlayer");
        //buttonKP.SetText("Kill Player");

        //buttonMap.Insert( buttonKP.GetName(), "KillPlayer" );
    }

    override void OnShow()
    {
        // Update checkbox checked
        UpdateCheckBoxes();
    }

    override void OnHide() 
    {
        
    }

    void UpdateCheckBoxes() 
    {
        foreach( string widgetName, string function : checkBoxMap ) 
        {
            Widget widget = m_checkboxPanel.FindWidget( widgetName );

            if ( widget.IsInherited( CheckBoxWidget )) 
            {
                CheckBoxWidget checkbox = CheckBoxWidget.Cast( widget );
                bool checked;
                GetGame().GameScript.CallFunction( this , function , checked, NULL );
                checkbox.SetChecked( checked );
            }
        }
    }

    void Update() 
    {
    }

    bool ToggleOldAiming( Widget widget )
    {
        if ( widget ) // Temp work around. Danny is lazy xd
        {
            m_OldAiming = !m_OldAiming;

            GetRPCManager().SendRPC( "COS_Game", "SetOldAiming", new Param1< bool >( m_OldAiming ), true );
        }
        return m_OldAiming;
    }

    bool ToggleGodMode( Widget widget ) 
    {
        if ( widget ) 
        {
            if ( GetSelectedPlayers().Count() )
            {
                GetRPCManager().SendRPC( "COS_Game", "SetGodMode", new Param1< ref array< ref AuthPlayer > >( GetSelectedPlayers() ), true );
            }
        }
        return m_GodMode;
    }

    bool KillPlayer( Widget widget ) 
    {
        if ( widget ) 
        {
            if ( GetSelectedPlayers().Count() )
            {
                GetRPCManager().SendRPC( "COS_Game", "KillEntity", new Param1< ref array< ref AuthPlayer > >( GetSelectedPlayers() ), true );
            }
        }
        return true;
    }

    override bool OnClick( Widget w, int x, int y, int button )
    {
        string param;
        Param1<string> param1;

        if ( w == m_gameScriptButton ) 
        {
            int selectRow = m_gameScriptList.GetSelectedRow();

            if ( selectRow == -1 ) return false;

            m_gameScriptList.GetItemData( selectRow, 0, param1 );

            if ( param1 ) 
            {
                GetGame().GameScript.CallFunction( this , param1.param1 , NULL, 0 );
            }
        }

        if ( w.IsInherited( CheckBoxWidget ) ) 
        {
            param = checkBoxMap.Get( w.GetName() );

            if ( param ) 
            {
                GetGame().GameScript.CallFunction( this , param , NULL, w );
            }
        }

        if ( w.IsInherited( ButtonWidget ) ) 
        {
            param = buttonMap.Get( w.GetName() );

            if ( param ) 
            {
                GetGame().GameScript.CallFunction( this , param , NULL, w );
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
        
        /*{
            "HatchbackWheel","HatchbackWheel","HatchbackWheel","HatchbackWheel",
            "CarBattery","CarRadiator","EngineBelt","SparkPlug","HatchbackHood",
            "HatchbackTrunk","HatchbackDoors_Driver","HatchbackDoors_CoDriver",
        };*/

        SpawnVehicle( "OffroadHatchback", attArr );
    }

    void SpawnSedan() 
    {
        ref array< string> attArr = new ref array< string>;
        
        /*{
            "CivSedanWheel","CivSedanWheel","CivSedanWheel","CivSedanWheel",
            "CarBattery", "CarRadiator","EngineBelt", "SparkPlug","CivSedanHood",
            "CivSedanTrunk", "CivSedanDoors_Driver","CivSedanDoors_CoDriver",
            "CivSedanDoors_BackLeft", "CivSedanDoors_BackRight",
        };*/

        SpawnVehicle( "CivilianSedan", attArr );
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

    void SpawnBus() 
    {
        ref array< string> attArr = new ref array< string>;
        
        /*{
            "TransitBusWheel","TransitBusWheel", "TransitBusWheelDouble","TransitBusWheelDouble",
            "TruckBattery","TruckRadiator","EngineBelt","GlowPlug","BusHood",
            "BusDoors_Left","BusDoors_Right", "BusDoors_Left","BusDoors_Right", "BusDoors_Left","BusDoors_Right",
        };*/

        SpawnVehicle( "TransitBus", attArr );
    }

    void SpawnVan() 
    {
        ref array< string> attArr = new ref array< string>;
        
        /*{
            "CivVanWheel","CivVanWheel","CivVanWheel","CivVanWheel",
            "CarBattery","CarRadiator","EngineBelt","SparkPlug","CivVanTrunk",
            "CivVanDoors_Driver","CivVanDoors_CoDriver","CivVanDoors_BackRight",
            "CivVanDoors_TrumpDown", "CivVanDoors_TrumpUp",
        };*/

        SpawnVehicle( "CivilianVan", attArr );
    }

    void SpawnVehicle( string vehicle, ref array< string> attachments) 
    {
        GetRPCManager().SendRPC( "COS_Game", "SpawnVehicle", new Param3< string, vector, ref array< string> >( vehicle, GetCursorPos(), attachments ), true, NULL, GetGame().GetPlayer() );
    }
}