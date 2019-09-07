class JMVehicleSpawnerSettings
{
	autoptr map< string, ref JMVehicleSpawnerSerialize > Vehicles = new map< string, ref JMVehicleSpawnerSerialize >;
	
	static ref JMVehicleSpawnerSettings Load()
	{
		ref JMVehicleSpawnerSettings settings = new JMVehicleSpawnerSettings();

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

				settings.Vehicles.Insert( name, JMVehicleSpawnerSerialize.Load( name ) );
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
		Default_CivilianSedan();
		Default_OffroadHatchback();
		Default_Hatchback_02();
		Default_Sedan_02();
	}

	void Default_CivilianSedan()
	{
		ref array< string> attArr = new array< string>;

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
		attArr.Insert("SparkPlug");

		ref JMVehicleSpawnerSerialize file = new JMVehicleSpawnerSerialize;

		file.VehicleName = "CivilianSedan";
		file.DisplayName = file.VehicleName;
		file.m_FileName = file.VehicleName;
		file.Parts = attArr;

		Vehicles.Insert( file.VehicleName, file );
	}

	void Default_OffroadHatchback()
	{
		ref array< string> attArr = new array< string>;

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
		attArr.Insert("SparkPlug");

		ref JMVehicleSpawnerSerialize file = new JMVehicleSpawnerSerialize;

		file.VehicleName = "OffroadHatchback";
		file.DisplayName = file.VehicleName;
		file.m_FileName = file.VehicleName;
		file.Parts = attArr;

		Vehicles.Insert( file.VehicleName, file );
	}

	void Default_Hatchback_02()
	{
		ref array< string> attArr = new array< string>;

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
		attArr.Insert("SparkPlug");

		ref JMVehicleSpawnerSerialize file = new JMVehicleSpawnerSerialize;

		file.VehicleName = "Hatchback_02";
		file.DisplayName = file.VehicleName;
		file.m_FileName = file.VehicleName;
		file.Parts = attArr;

		Vehicles.Insert( file.VehicleName, file );
	}

	void Default_Sedan_02()
	{
		ref array< string> attArr = new array< string>;

		attArr.Insert("Sedan_02_Hood");
		attArr.Insert("Sedan_02_Trunk");
		attArr.Insert("Sedan_02_Doors_Driver");
		attArr.Insert("Sedan_02_Doors_CoDriver");
		attArr.Insert("Sedan_02_Wheel");
		attArr.Insert("Sedan_02_Wheel");
		attArr.Insert("Sedan_02_Wheel");
		attArr.Insert("Sedan_02_Wheel");
		attArr.Insert("CarBattery");
		attArr.Insert("CarRadiator");
		attArr.Insert("SparkPlug");

		ref JMVehicleSpawnerSerialize file = new JMVehicleSpawnerSerialize;

		file.VehicleName = "Sedan_02";
		file.DisplayName = file.VehicleName;
		file.m_FileName = file.VehicleName;
		file.Parts = attArr;

		Vehicles.Insert( file.VehicleName, file );
	}
}