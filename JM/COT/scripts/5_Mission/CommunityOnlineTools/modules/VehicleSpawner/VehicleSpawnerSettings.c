class VehicleSpawnerSettings
{
	autoptr map< string, ref VehicleSpawnerFile > Vehicles = new map< string, ref VehicleSpawnerFile >;
	
	static ref VehicleSpawnerSettings Load()
	{
		ref VehicleSpawnerSettings settings = new VehicleSpawnerSettings();

		if ( GetGame().IsClient() || ( !GetGame().IsMultiplayer() && GetGame().IsServer() ) )
		{
			settings.Defaults();
			return settings;
		}
		
		MakeDirectory( VEHICLE_SPAWNER_FOLDER );

		array< string > files = FindFilesInLocation( VEHICLE_SPAWNER_FOLDER );

		if ( FileExist( VEHICLE_SPAWNER_FOLDER + COT_FILE_EXIST ) ) 
		{
			GetLogger().Log( "Found existence ( " + VEHICLE_SPAWNER_FOLDER + " )", "JM_COT_VehicleSpawner" );
			for ( int i = 0; i < files.Count(); i++ )
			{
				if ( files[i].Contains( COT_FILE_EXIST ) )
					continue;

				string name = files[i];
				int pos = files[i].IndexOf(".");
				
				if ( pos > -1 )
					name = files[i].Substring( 0, pos );

				settings.Vehicles.Insert( name, VehicleSpawnerFile.Load( name ) );
				GetLogger().Log( "	Loading vehicle file ( " + name + " )", "JM_COT_VehicleSpawner" );
			}
		} else 
		{
			GetLogger().Log( "Didn't find existence ( " + VEHICLE_SPAWNER_FOLDER + " )", "JM_COT_VehicleSpawner" );
			DeleteFiles( VEHICLE_SPAWNER_FOLDER, files );

			settings.Defaults();
			settings.Save();
		}

		return settings;
	}

	void Save()
	{
		for ( int i = 0; i < Vehicles.Count(); i++ )
		{
			Vehicles.GetElement( i ).Save();
		}

		CreateFilesExist( VEHICLE_SPAWNER_FOLDER );
	}

	void Defaults()
	{
		DefaultUtilityVehicle();
		DefaultS120();
		DefaultVan();
		DefaultSedan();
		DefaultHatchback();
		DefaultV3SCargo();
		DefaultV3S();
		DefaultHatchback02();
	}

	void DefaultUtilityVehicle()
	{
		ref array< string> attArr = new ref array< string>;

		attArr.Insert("UtilityVehicleWheelDoors_Driver");
		attArr.Insert("UtilityVehicleWheelDoors_CoDriver");
		attArr.Insert("UtilityVehicleWheel");
		attArr.Insert("UtilityVehicleWheel");
		attArr.Insert("UtilityVehicleWheel");
		attArr.Insert("UtilityVehicleWheel");
		attArr.Insert("CarBattery");
		attArr.Insert("CarRadiator");
		attArr.Insert("EngineBelt");
		attArr.Insert("SparkPlug");

		ref VehicleSpawnerFile file = new VehicleSpawnerFile;

		file.VehicleName = "UtilityVehicle";
		file.DisplayName = file.VehicleName;
		file.m_FileName = file.VehicleName;
		file.Parts = attArr;

		Vehicles.Insert( file.VehicleName, file );
	}

	void DefaultS120()
	{
		ref array< string> attArr = new ref array< string>;

		attArr.Insert("S120Trunk");
		attArr.Insert("S120Hood");
		attArr.Insert("S120Doors_Driver");
		attArr.Insert("S120Doors_CoDriver");
		attArr.Insert("S120Doors_BackLeft");
		attArr.Insert("S120Doors_BackRight");
		attArr.Insert("S120Wheel");
		attArr.Insert("S120Wheel");
		attArr.Insert("S120Wheel");
		attArr.Insert("S120Wheel");
		attArr.Insert("CarBattery");
		attArr.Insert("CarRadiator");
		attArr.Insert("EngineBelt");
		attArr.Insert("SparkPlug");

		ref VehicleSpawnerFile file = new VehicleSpawnerFile;

		file.VehicleName = "S120";
		file.DisplayName = file.VehicleName;
		file.m_FileName = file.VehicleName;
		file.Parts = attArr;

		Vehicles.Insert( file.VehicleName, file );
	}

	void DefaultVan()
	{
		ref array< string> attArr = new ref array< string>;

		attArr.Insert("CivVanDoors_TrunkDown");
		attArr.Insert("CivVanDoors_TrunkUp");
		attArr.Insert("CivVanDoors_BackRight");
		attArr.Insert("CivVanDoors_CoDriver");
		attArr.Insert("CivVanWheel");
		attArr.Insert("CivVanWheel");
		attArr.Insert("CivVanWheel");
		attArr.Insert("CivVanWheel");
		attArr.Insert("CarBattery");
		attArr.Insert("CarRadiator");
		attArr.Insert("EngineBelt");
		attArr.Insert("SparkPlug");

		ref VehicleSpawnerFile file = new VehicleSpawnerFile;

		file.VehicleName = "CivilianVan";
		file.DisplayName = file.VehicleName;
		file.m_FileName = file.VehicleName;
		file.Parts = attArr;

		Vehicles.Insert( file.VehicleName, file );
	}

	void DefaultSedan()
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

		ref VehicleSpawnerFile file = new VehicleSpawnerFile;

		file.VehicleName = "CivilianSedan";
		file.DisplayName = file.VehicleName;
		file.m_FileName = file.VehicleName;
		file.Parts = attArr;

		Vehicles.Insert( file.VehicleName, file );
	}

	void DefaultHatchback()
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

		ref VehicleSpawnerFile file = new VehicleSpawnerFile;

		file.VehicleName = "OffroadHatchback";
		file.DisplayName = file.VehicleName;
		file.m_FileName = file.VehicleName;
		file.Parts = attArr;

		Vehicles.Insert( file.VehicleName, file );
	}

	void DefaultHatchback02()
	{
		ref array< string> attArr = new ref array< string>;

		attArr.Insert("Hatchback_02_Hood");
		attArr.Insert("Hatchback_02_Trunk");
		attArr.Insert("Hatchback_02_Doors_Driver");
		attArr.Insert("Hatchback_02_Doors_CoDriver");
		attArr.Insert("Hatchback_02_Wheel");
		attArr.Insert("Hatchback_02_Wheel");
		attArr.Insert("Hatchback_02_Wheel");
		attArr.Insert("Hatchback_02_Wheel");
		attArr.Insert("CarBattery");
		attArr.Insert("CarRadiator");
		attArr.Insert("EngineBelt");
		attArr.Insert("SparkPlug");

		ref VehicleSpawnerFile file = new VehicleSpawnerFile;

		file.VehicleName = "Hatchback_02";
		file.DisplayName = file.VehicleName;
		file.m_FileName = file.VehicleName;
		file.Parts = attArr;

		Vehicles.Insert( file.VehicleName, file );
	}

	void DefaultV3SCargo()
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

		ref VehicleSpawnerFile file = new VehicleSpawnerFile;

		file.VehicleName = "V3S_Cargo_Blue";
		file.DisplayName = file.VehicleName;
		file.m_FileName = file.VehicleName;
		file.Parts = attArr;

		Vehicles.Insert( file.VehicleName, file );
	}

	void DefaultV3S()
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

		ref VehicleSpawnerFile file = new VehicleSpawnerFile;

		file.VehicleName = "V3S_Chassis_Blue";
		file.DisplayName = file.VehicleName;
		file.m_FileName = file.VehicleName;
		file.Parts = attArr;

		Vehicles.Insert( file.VehicleName, file );
	}
}