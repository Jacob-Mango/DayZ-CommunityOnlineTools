class JMVehicleSpawnerSettings
{
	autoptr map< string, ref JMVehicleSpawnerSerialize > Vehicles = new map< string, ref JMVehicleSpawnerSerialize >;
	
	static JMVehicleSpawnerSettings Load()
	{
		JMVehicleSpawnerSettings settings = new JMVehicleSpawnerSettings();

		if ( GetGame().IsClient() )
		{
			settings.Defaults();
			return settings;
		}

		if ( !FileExist( JMConstants.DIR_VEHICLES ) )
		{
			MakeDirectory( JMConstants.DIR_VEHICLES );

			settings.Defaults();
			settings.Save();

			return settings;
		}

		array< string > files = FindFilesInLocation( JMConstants.DIR_VEHICLES );

		for ( int i = 0; i < files.Count(); i++ )
		{
			string fileName;
			string fileType;
			int pos = files[i].IndexOf(".");
			
			if ( pos > -1 )
			{
				fileName = files[i].Substring( 0, pos );
				// fileType = files[i].Substring( pos, files[i].Length() - 1 );

				// if ( fileType == JMConstants.EXT_VEHICLE )
				// {
					settings.Vehicles.Insert( fileName, JMVehicleSpawnerSerialize.Load( fileName ) );
				// }
			}
		}
		
		return settings;
	}

	void Save()
	{
		for ( int i = 0; i < Vehicles.Count(); i++ )
		{
			Vehicles.GetElement( i ).Save();
		}
	}

	void Defaults()
	{
		Default_CivilianSedan();
		Default_OffroadHatchback();
		Default_Hatchback_02();
		Default_Sedan_02();
		Default_Truck_01_Covered();
		Default_Truck_01_Chassis();
		Default_Truck_01_Cargo();
		Default_Truck_01_Command();
	}

	void Default_CivilianSedan()
	{
		array< string> attArr = new array< string>;

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

		JMVehicleSpawnerSerialize file = JMVehicleSpawnerSerialize.Create();

		file.VehicleName = "CivilianSedan";
		file.m_FileName = file.VehicleName;
		file.Parts.Copy( attArr );

		Vehicles.Insert( file.VehicleName, file );
	}

	void Default_OffroadHatchback()
	{
		array< string> attArr = new array< string>;

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

		JMVehicleSpawnerSerialize file = JMVehicleSpawnerSerialize.Create();

		file.VehicleName = "OffroadHatchback";
		file.m_FileName = file.VehicleName;
		file.Parts.Copy( attArr );

		Vehicles.Insert( file.VehicleName, file );
	}

	void Default_Hatchback_02()
	{
		array< string> attArr = new array< string>;

		attArr.Insert("Hatchback_02_Hood");
		attArr.Insert("Hatchback_02_Trunk");
		attArr.Insert("Hatchback_02_Door_1_1");
		attArr.Insert("Hatchback_02_Door_1_2");
		attArr.Insert("Hatchback_02_Door_2_1");
		attArr.Insert("Hatchback_02_Door_2_2");
		attArr.Insert("Hatchback_02_Wheel");
		attArr.Insert("Hatchback_02_Wheel");
		attArr.Insert("Hatchback_02_Wheel");
		attArr.Insert("Hatchback_02_Wheel");
		attArr.Insert("CarBattery");
		attArr.Insert("CarRadiator");
		attArr.Insert("SparkPlug");

		JMVehicleSpawnerSerialize file = JMVehicleSpawnerSerialize.Create();

		file.VehicleName = "Hatchback_02";
		file.m_FileName = file.VehicleName;
		file.Parts.Copy( attArr );

		Vehicles.Insert( file.VehicleName, file );
	}

	void Default_Sedan_02()
	{
		array< string> attArr = new array< string>;

		attArr.Insert("Sedan_02_Hood");
		attArr.Insert("Sedan_02_Trunk");
		attArr.Insert("Sedan_02_Door_1_1");
		attArr.Insert("Sedan_02_Door_1_2");
		attArr.Insert("Sedan_02_Door_2_1");
		attArr.Insert("Sedan_02_Door_2_2");
		attArr.Insert("Sedan_02_Wheel");
		attArr.Insert("Sedan_02_Wheel");
		attArr.Insert("Sedan_02_Wheel");
		attArr.Insert("Sedan_02_Wheel");
		attArr.Insert("CarBattery");
		attArr.Insert("CarRadiator");
		attArr.Insert("SparkPlug");

		JMVehicleSpawnerSerialize file = JMVehicleSpawnerSerialize.Create();

		file.VehicleName = "Sedan_02";
		file.m_FileName = file.VehicleName;
		file.Parts.Copy( attArr );

		Vehicles.Insert( file.VehicleName, file );
	}

	void Default_Truck_01_Covered()
	{
		array< string> attArr = new array< string>;

		attArr.Insert("Truck_01_Hood");
		attArr.Insert("Truck_01_Door_1_1_Blue");
		attArr.Insert("Truck_01_Door_2_1_Blue");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("TruckBattery");
		attArr.Insert("SparkPlug");

		JMVehicleSpawnerSerialize file = JMVehicleSpawnerSerialize.Create();

		file.VehicleName = "Truck_01_Covered_Blue";
		file.m_FileName = file.VehicleName;
		file.Parts.Copy( attArr );

		Vehicles.Insert( file.VehicleName, file );
	}

	void Default_Truck_01_Chassis()
	{
		array< string> attArr = new array< string>;

		attArr.Insert("Truck_01_Hood");
		attArr.Insert("Truck_01_Door_1_1");
		attArr.Insert("Truck_01_Door_2_1");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("TruckBattery");
		attArr.Insert("SparkPlug");

		JMVehicleSpawnerSerialize file = JMVehicleSpawnerSerialize.Create();

		file.VehicleName = "Truck_01_Chassis_Blue";
		file.m_FileName = file.VehicleName;
		file.Parts.Copy( attArr );

		Vehicles.Insert( file.VehicleName, file );
	}

	void Default_Truck_01_Cargo()
	{
		array< string> attArr = new array< string>;

		attArr.Insert("Truck_01_Hood");
		attArr.Insert("Truck_01_Door_1_1");
		attArr.Insert("Truck_01_Door_2_1");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("TruckBattery");
		attArr.Insert("SparkPlug");

		JMVehicleSpawnerSerialize file = JMVehicleSpawnerSerialize.Create();

		file.VehicleName = "Truck_01_Cargo_Blue";
		file.m_FileName = file.VehicleName;
		file.Parts.Copy( attArr );

		Vehicles.Insert( file.VehicleName, file );
	}

	void Default_Truck_01_Command()
	{
		array< string> attArr = new array< string>;

		attArr.Insert("Truck_01_Hood");
		attArr.Insert("Truck_01_Door_1_1");
		attArr.Insert("Truck_01_Door_2_1");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_WheelDouble");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("Truck_01_Wheel");
		attArr.Insert("TruckBattery");
		attArr.Insert("SparkPlug");

		JMVehicleSpawnerSerialize file = JMVehicleSpawnerSerialize.Create();

		file.VehicleName = "Truck_01_Command";
		file.m_FileName = file.VehicleName;
		file.Parts.Copy( attArr );

		Vehicles.Insert( file.VehicleName, file );
	}
}