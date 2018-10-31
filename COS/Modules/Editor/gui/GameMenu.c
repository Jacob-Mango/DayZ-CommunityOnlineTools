class GameMenu extends PopupMenu
{
	TextListboxWidget m_gameScriptList;
	Widget 			  m_checkboxPanel;
	ButtonWidget 	  m_gameScriptButton;
	ButtonWidget 	  m_reloadScriptButton;

	protected ref map< string, string > checkBoxMap = new map< string, string >; // store widget name

	string checkboxLayout = "COS\\Modules\\Editor\\gui\\layouts\\CheckboxTemplate.layout";

	void GameMenu()
	{
	}

	void ~GameMenu()
	{
	}

	override void Init()
	{
		m_checkboxPanel = layoutRoot.FindAnyWidget("game_checkbox_panel");
		m_gameScriptList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("game_list_box"));
		m_gameScriptButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("game_script_button"));
		m_reloadScriptButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("reload_scripts_button"));

		m_gameScriptList.AddItem("Spawn Hatchback", new Param1<string>("SpawnHatchback"), 0);
//		m_gameScriptList.AddItem("Spawn Sedan", 	new Param1<string>("SpawnSedan"), 	  0);
//		m_gameScriptList.AddItem("Spawn V3S_Cargo", new Param1<string>("SpawnV3SCargo"),  0);
		m_gameScriptList.AddItem("Spawn V3S",		new Param1<string>("SpawnV3S"), 	  0);
//		m_gameScriptList.AddItem("Spawn Bus", 		new Param1<string>("SpawnBus"), 	  0);
//		m_gameScriptList.AddItem("Spawn Van",	    new Param1<string>("SpawnVan"), 	  0);

		CheckBoxWidget checkBoxGodmode = CheckBoxWidget.Cast(GetGame().GetWorkspace().CreateWidgets( checkboxLayout, m_checkboxPanel ));
		checkBoxGodmode.SetName("Godmode");
		checkBoxGodmode.SetText("Godmode");

		CheckBoxWidget checkBoxAiming = CheckBoxWidget.Cast(GetGame().GetWorkspace().CreateWidgets( checkboxLayout, m_checkboxPanel ));
		checkBoxAiming.SetName("OldAiming");
		checkBoxAiming.SetText("Old Aiming");

		CheckBoxWidget checkBoxLaser = CheckBoxWidget.Cast(GetGame().GetWorkspace().CreateWidgets( checkboxLayout, m_checkboxPanel ));
		checkBoxLaser.SetName("LaserPointer");
		checkBoxLaser.SetText("Laser Pointer");

		CheckBoxWidget checkBoxKP = CheckBoxWidget.Cast(GetGame().GetWorkspace().CreateWidgets( checkboxLayout, m_checkboxPanel ));
		checkBoxKP.SetName("KillPlayer");
		checkBoxKP.SetText("Kill Player");

		checkBoxMap.Insert( checkBoxGodmode.GetName(), "ToggleGodMode" );
		checkBoxMap.Insert( checkBoxAiming.GetName(), "ToggleOldAiming" );
		checkBoxMap.Insert( checkBoxLaser.GetName(), "ToggleLaser" );
		checkBoxMap.Insert( checkBoxKP.GetName(), "KillPlayer" );
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

	bool ToggleOldAiming( CheckBoxWidget widget )
	{
		if ( widget ) // Temp work around. Danny is lazy xd
		{
			GetRPCManager().SendRPC( "COS", "SetOldAiming", new Param1< bool >( !m_OldAiming ), true );
		}
		return m_OldAiming;
	}

	bool ToggleGodMode( CheckBoxWidget widget ) 
	{
		if ( widget ) 
		{
			m_GodMode = !m_GodMode;

			GetRPCManager().SendRPC( "COS", "SetGodMode", new Param1< PlayerBase >( GetGame().GetPlayer() ), true );
		}
		return m_GodMode;
	}

	bool ToggleLaser( CheckBoxWidget widget ) 
	{
		if ( widget ) 
		{
			bc_Visible = !bc_Visible;
		}
		return bc_Visible;
	}

	bool KillPlayer( CheckBoxWidget widget ) 
	{
		if ( widget ) 
		{
			GetRPCManager().SendRPC( "COS", "KillEntity", new Param1< EntityAI >( GetGame().GetPlayer() ), true );
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
				GetGame().GameScript.CallFunction( this , param , NULL, CheckBoxWidget.Cast( w ) );
			}
		}

		if ( w == m_reloadScriptButton )
		{
			GetModuleManager().ReloadModules();
		}

		return false;
	}

	void SpawnHatchback() 
	{
		TStringArray attArr = {
		"HatchbackWheel","HatchbackWheel","HatchbackWheel","HatchbackWheel",
		"CarBattery","CarRadiator","EngineBelt","SparkPlug","HatchbackHood",
		"HatchbackTrunk","HatchbackDoors_Driver","HatchbackDoors_CoDriver",
		};

		SpawnVehicle( "OffroadHatchback", attArr );
	}

	void SpawnSedan() 
	{
		TStringArray attArr = {
		"CivSedanWheel","CivSedanWheel","CivSedanWheel","CivSedanWheel",
		"CarBattery", "CarRadiator","EngineBelt", "SparkPlug","CivSedanHood",
		"CivSedanTrunk", "CivSedanDoors_Driver","CivSedanDoors_CoDriver",
		"CivSedanDoors_BackLeft", "CivSedanDoors_BackRight",
		};

		SpawnVehicle( "CivilianSedan", attArr );
	}

	void SpawnV3SCargo() 
	{
		TStringArray attArr = {
		"V3SWheel","V3SWheel", "V3SWheel","V3SWheel", "V3SWheelDouble","V3SWheelDouble", "V3SWheelDouble","V3SWheelDouble",
		"TruckBattery","TruckRadiator","EngineBelt","GlowPlug","V3SHood",
		"V3SDoors_Driver_Orange","V3SDoors_CoDriver_Orange",
		};

		SpawnVehicle( "V3S_Cargo_Blue", attArr );
	}

	void SpawnV3S() 
	{
		TStringArray attArr = {
		"V3SWheel","V3SWheel", "V3SWheel","V3SWheel", "V3SWheelDouble","V3SWheelDouble", "V3SWheelDouble","V3SWheelDouble",
		"TruckBattery","TruckRadiator","EngineBelt","GlowPlug","V3SHood",
		"V3SDoors_Driver_Orange","V3SDoors_CoDriver_Orange",
		};

		SpawnVehicle( "V3S_Chassis_Blue", attArr );
	}

	void SpawnBus() 
	{
		TStringArray attArr = {
		"TransitBusWheel","TransitBusWheel", "TransitBusWheelDouble","TransitBusWheelDouble",
		"TruckBattery","TruckRadiator","EngineBelt","GlowPlug","BusHood",
		"BusDoors_Left","BusDoors_Right", "BusDoors_Left","BusDoors_Right", "BusDoors_Left","BusDoors_Right",
		};

		SpawnVehicle( "TransitBus", attArr );
	}

	void SpawnVan() 
	{
		TStringArray attArr = {
			"CivVanWheel","CivVanWheel","CivVanWheel","CivVanWheel",
			"CarBattery","CarRadiator","EngineBelt","SparkPlug","CivVanTrunk",
			"CivVanDoors_Driver","CivVanDoors_CoDriver","CivVanDoors_BackRight",
			"CivVanDoors_TrumpDown", "CivVanDoors_TrumpUp",
		};

		SpawnVehicle( "CivilianVan", attArr );
	}

	void SpawnVehicle( string vehicle, TStringArray attachments) 
	{
		GetRPCManager().SendRPC( "COS", "SpawnVehicle", new Param3< string, vector, TStringArray >( vehicle, GetCursorPos(), attachments ), true, NULL, GetGame().GetPlayer() );
	}
}