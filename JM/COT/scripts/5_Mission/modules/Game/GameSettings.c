const string GAME_SETTINGS_FILE = ROOT_COT_DIR + "GameModule.json";

autoptr map< string, ref GameBaseBuildingFile > GameSettings_BaseBuilding = new map< string, ref GameBaseBuildingFile >;
autoptr map< string, ref GameVehicleFile > GameSettings_Vehicles = new map< string, ref GameVehicleFile >;

class GameSettings
{
    static ref GameSettings Load()
    {
        ref GameSettings settings = new GameSettings();

        MakeDirectory( GAME_BASEBUILDING_FOLDER );
        MakeDirectory( GAME_VEHICLE_FOLDER );

        if ( FileExist( GAME_SETTINGS_FILE ) )
        {
            JsonFileLoader<GameSettings>.JsonLoadFile( GAME_SETTINGS_FILE, settings );
        } else {
            settings.Defaults();
            settings.Save();
        }

        LoadVehicleFiles();
        LoadBaseBuildingFiles();

        return settings;
    }

    static void LoadVehicleFiles()
    {
		string fileName;
		FileAttr fileAttr;

		FindFileHandle file = FindFile( GAME_VEHICLE_FOLDER + "/*", fileName, fileAttr, 0 );
		
        if ( !file ) return;

        GameSettings_Vehicles.Insert( fileName, GameVehicleFile.Load( fileName ) );

		while( FindNextFile( file, fileName, fileAttr ) )
		{
            GameSettings_Vehicles.Insert( fileName, GameVehicleFile.Load( fileName ) );
		}

		CloseFindFile( file );
    }

    static void LoadBaseBuildingFiles()
    {
		string fileName;
		FileAttr fileAttr;

		FindFileHandle file = FindFile( GAME_BASEBUILDING_FOLDER + "/*", fileName, fileAttr, 0 );
		
        if ( !file ) return;

        GameSettings_BaseBuilding.Insert( fileName, GameBaseBuildingFile.Load( fileName ) );

		while( FindNextFile( file, fileName, fileAttr ) )
		{
            GameSettings_BaseBuilding.Insert( fileName, GameBaseBuildingFile.Load( fileName ) );
		}

		CloseFindFile( file );
    }

    void Save()
    {
        JsonFileLoader<GameSettings>.JsonSaveFile( GAME_SETTINGS_FILE, this );
    }

    void Defaults()
    {
        // DefaultVan();
        // DefaultSedan();
        DefaultHatchback();
        // DefaultV3SCargo();
        // DefaultV3S();

        DefaultFenceMetal();
        DefaultFenceWood();

        DefaultGateMetal();
        DefaultGateWood();

        DefaultPlatform();

        DefaultWatchtowerMetal();
        DefaultWatchtowerWood();
    }

    void SaveSetVehicleFile( ref GameVehicleFile file )
    {
        GameSettings_Vehicles.Insert( file.Name, file );

        file.Save();
    }

    void SaveSetBaseBuildingFile( ref GameBaseBuildingFile file )
    {
        GameSettings_BaseBuilding.Insert( file.Name, file );

        file.Save();
    }

    void DefaultFenceMetal()
    {
        ref array< ref GameBaseBuildingSpawn > attArr = new ref array< ref GameBaseBuildingSpawn >;

        attArr.Insert( new GameBaseBuildingSpawn( "FenceKit", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Woodenlog", 1, 2 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "WoodenPlank", 1, 8 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Nail", 1, 56 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "MetalPlate", 1, 6 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Hammer", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Shovel", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "BarbedWire", 2, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "CamoNet", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "DuctTape", 5, 100 ) );

        ref GameBaseBuildingFile file = new GameBaseBuildingFile;

        file.Name = "Fence Metal";
        file.BaseBuildingAttachments = attArr;

        SaveSetBaseBuildingFile( file );
    }

    void DefaultFenceWood()
    {
        ref array< ref GameBaseBuildingSpawn > attArr = new ref array< ref GameBaseBuildingSpawn >;

        attArr.Insert( new GameBaseBuildingSpawn( "FenceKit", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Woodenlog", 1, 2 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "WoodenPlank", 1, 20 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Nail", 1, 56 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Hammer", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Shovel", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "BarbedWire", 2, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "CamoNet", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "DuctTape", 5, 100 ) );

        ref GameBaseBuildingFile file = new GameBaseBuildingFile;

        file.Name = "Fence Wood";
        file.BaseBuildingAttachments = attArr;

        SaveSetBaseBuildingFile( file );
    }

    void DefaultGateMetal()
    {
        ref array< ref GameBaseBuildingSpawn > attArr = new ref array< ref GameBaseBuildingSpawn >;

        attArr.Insert( new GameBaseBuildingSpawn( "FenceKit", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Woodenlog", 1, 2 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "WoodenPlank", 1, 8 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Nail", 1, 56 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "MetalPlate", 1, 6 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Hammer", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Shovel", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Pliers", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "BarbedWire", 2, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "CamoNet", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "DuctTape", 5, 100 ) );

        ref GameBaseBuildingFile file = new GameBaseBuildingFile;

        file.Name = "Gate Metal";
        file.BaseBuildingAttachments = attArr;

        SaveSetBaseBuildingFile( file );
    }

    void DefaultGateWood()
    {
        ref array< ref GameBaseBuildingSpawn > attArr = new ref array< ref GameBaseBuildingSpawn >;

        attArr.Insert( new GameBaseBuildingSpawn( "FenceKit", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Woodenlog", 1, 2 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "WoodenPlank", 1, 20 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Nail", 1, 56 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Hammer", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Shovel", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Pliers", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "BarbedWire", 2, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "CamoNet", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "DuctTape", 5, 100 ) );

        ref GameBaseBuildingFile file = new GameBaseBuildingFile;

        file.Name = "Gate Wood";
        file.BaseBuildingAttachments = attArr;

        SaveSetBaseBuildingFile( file );
    }

    void DefaultPlatform()
    {
        ref array< ref GameBaseBuildingSpawn > attArr = new ref array< ref GameBaseBuildingSpawn >;

        attArr.Insert( new GameBaseBuildingSpawn( "WoodenPlank", 1, 18 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Nail", 1, 30 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Hammer", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "DuctTape", 5, 100 ) );

        ref GameBaseBuildingFile file = new GameBaseBuildingFile;

        file.Name = "Platform";
        file.BaseBuildingAttachments = attArr;

        SaveSetBaseBuildingFile( file );
    }

    void DefaultWatchtowerMetal()
    {
        ref array< ref GameBaseBuildingSpawn > attArr = new ref array< ref GameBaseBuildingSpawn >;

        attArr.Insert( new GameBaseBuildingSpawn( "WatchtowerKit", 5, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Woodenlog", 1, 2 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "WoodenPlank", 1, 8 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Nail", 3, 100 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "MetalPlate", 3, 12 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Hammer", 4, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Shovel", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Pliers", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "BarbedWire", 6, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "CamoNet", 9, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Crowbar", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "DuctTape", 5, 100 ) );

        ref GameBaseBuildingFile file = new GameBaseBuildingFile;

        file.Name = "Watchtower Metal";
        file.BaseBuildingAttachments = attArr;

        SaveSetBaseBuildingFile( file );
    }

    void DefaultWatchtowerWood()
    {
        ref array< ref GameBaseBuildingSpawn > attArr = new ref array< ref GameBaseBuildingSpawn >;

        attArr.Insert( new GameBaseBuildingSpawn( "WatchtowerKit", 5, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Woodenlog", 3, 4 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "WoodenPlank", 3, 20 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Nail", 3, 100 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Hammer", 4, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Shovel", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Pliers", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "BarbedWire", 6, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "CamoNet", 9, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "Crowbar", 1, 1 ) );
        attArr.Insert( new GameBaseBuildingSpawn( "DuctTape", 5, 100 ) );

        ref GameBaseBuildingFile file = new GameBaseBuildingFile;

        file.Name = "Watchtower Wood";
        file.BaseBuildingAttachments = attArr;

        SaveSetBaseBuildingFile( file );
    }

    void DefaultVan()
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

        ref GameVehicleFile file = new GameVehicleFile;

        file.Name = "CivilianVan";
        file.VehicleParts = attArr;

        SaveSetVehicleFile( file );
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

        ref GameVehicleFile file = new GameVehicleFile;

        file.Name = "CivilianSedan";
        file.VehicleParts = attArr;

        SaveSetVehicleFile( file );
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

        ref GameVehicleFile file = new GameVehicleFile;

        file.Name = "OffroadHatchback";
        file.VehicleParts = attArr;

        SaveSetVehicleFile( file );
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

        ref GameVehicleFile file = new GameVehicleFile;

        file.Name = "V3S_Cargo_Blue";
        file.VehicleParts = attArr;

        SaveSetVehicleFile( file );
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

        ref GameVehicleFile file = new GameVehicleFile;

        file.Name = "V3S_Chassis_Blue";
        file.VehicleParts = attArr;

        SaveSetVehicleFile( file );
    }
}