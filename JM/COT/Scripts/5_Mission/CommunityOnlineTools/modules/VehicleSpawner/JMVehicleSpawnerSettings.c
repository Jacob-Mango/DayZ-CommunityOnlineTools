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
				
				JMVehicleSpawnerSerialize serialize = JMVehicleSpawnerSerialize.Load(fileName);
				
				if (!GetGame().ConfigIsExisting("cfgVehicles " + serialize.VehicleName))
				{
					Print("[Community Online Tools] Vehicle Spawner Module - Couldnt find any classnames called <<"+ serialize.VehicleName +">> from the file "+ files[i]);
					continue;
				}
				
				settings.Vehicles.Insert( fileName, JMVehicleSpawnerSerialize.Load( fileName ) );
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
		Default_OffroadHatchback();
		Default_CivilianSedan();
		Default_Hatchback_02();
		Default_Sedan_02();
		Default_Offroad_02();
		Default_Truck_01_Covered();
	}

	void Default_CivilianSedan()
	{
		array<string> colorArr = {"","_Wine","_Black"};

		foreach (string color : colorArr)
		{
			array<string> attArr = new array<string>;

			attArr.Insert("CivSedanHood"+color);
			attArr.Insert("CivSedanTrunk"+color);
			attArr.Insert("CivSedanDoors_Driver"+color);
			attArr.Insert("CivSedanDoors_BackRight"+color);
			attArr.Insert("CivSedanDoors_BackLeft"+color);
			attArr.Insert("CivSedanDoors_CoDriver"+color);
			attArr.Insert("CivSedanWheel");
			attArr.Insert("CivSedanWheel");
			attArr.Insert("CivSedanWheel");
			attArr.Insert("CivSedanWheel");
			attArr.Insert("CivSedanWheel");
			attArr.Insert("CarBattery");
			attArr.Insert("CarRadiator");
			attArr.Insert("SparkPlug");
			attArr.Insert("HeadlightH7");
			attArr.Insert("HeadlightH7");

			JMVehicleSpawnerSerialize file = JMVehicleSpawnerSerialize.Create();

			file.VehicleName = "CivilianSedan"+color;
			file.m_FileName = file.VehicleName;
			file.Parts.Copy( attArr );

			Vehicles.Insert( file.VehicleName, file );
		}
	}

	void Default_OffroadHatchback()
	{
		array<string> colorArr = {"","_Blue","_White"};

		foreach (string color : colorArr)
		{
			array< string> attArr = new array< string>;

			attArr.Insert("HatchbackHood"+color);
			attArr.Insert("HatchbackTrunk"+color);
			attArr.Insert("HatchbackDoors_Driver"+color);
			attArr.Insert("HatchbackDoors_CoDriver"+color);
			attArr.Insert("HatchbackWheel");
			attArr.Insert("HatchbackWheel");
			attArr.Insert("HatchbackWheel");
			attArr.Insert("HatchbackWheel");
			attArr.Insert("HatchbackWheel");
			attArr.Insert("CarBattery");
			attArr.Insert("CarRadiator");
			attArr.Insert("SparkPlug");
			attArr.Insert("HeadlightH7");
			attArr.Insert("HeadlightH7");

			JMVehicleSpawnerSerialize file = JMVehicleSpawnerSerialize.Create();

			file.VehicleName = "OffroadHatchback"+color;
			file.m_FileName = file.VehicleName;
			file.Parts.Copy( attArr );

			Vehicles.Insert( file.VehicleName, file );
		}
	}

	void Default_Hatchback_02()
	{
		array<string> colorArr = {"","_Black","_Blue"};

		foreach (string color : colorArr)
		{
			array< string> attArr = new array< string>;

			attArr.Insert("Hatchback_02_Hood"+color);
			attArr.Insert("Hatchback_02_Trunk"+color);
			attArr.Insert("Hatchback_02_Door_1_1"+color);
			attArr.Insert("Hatchback_02_Door_1_2"+color);
			attArr.Insert("Hatchback_02_Door_2_1"+color);
			attArr.Insert("Hatchback_02_Door_2_2"+color);
			attArr.Insert("Hatchback_02_Wheel");
			attArr.Insert("Hatchback_02_Wheel");
			attArr.Insert("Hatchback_02_Wheel");
			attArr.Insert("Hatchback_02_Wheel");
			attArr.Insert("Hatchback_02_Wheel");
			attArr.Insert("CarBattery");
			attArr.Insert("CarRadiator");
			attArr.Insert("SparkPlug");
			attArr.Insert("HeadlightH7");
			attArr.Insert("HeadlightH7");

			JMVehicleSpawnerSerialize file = JMVehicleSpawnerSerialize.Create();

			file.VehicleName = "Hatchback_02"+color;
			file.m_FileName = file.VehicleName;
			file.Parts.Copy( attArr );

			Vehicles.Insert( file.VehicleName, file );
		}
	}

	void Default_Sedan_02()
	{
		array<string> colorArr = {"","_Red","_Grey"};

		foreach (string color : colorArr)
		{
			array< string> attArr = new array< string>;

			attArr.Insert("Sedan_02_Hood"+color);
			attArr.Insert("Sedan_02_Trunk"+color);
			attArr.Insert("Sedan_02_Door_1_1"+color);
			attArr.Insert("Sedan_02_Door_1_2"+color);
			attArr.Insert("Sedan_02_Door_2_1"+color);
			attArr.Insert("Sedan_02_Door_2_2"+color);
			attArr.Insert("Sedan_02_Wheel");
			attArr.Insert("Sedan_02_Wheel");
			attArr.Insert("Sedan_02_Wheel");
			attArr.Insert("Sedan_02_Wheel");
			attArr.Insert("Sedan_02_Wheel");
			attArr.Insert("CarBattery");
			attArr.Insert("CarRadiator");
			attArr.Insert("SparkPlug");
			attArr.Insert("HeadlightH7");
			attArr.Insert("HeadlightH7");

			JMVehicleSpawnerSerialize file = JMVehicleSpawnerSerialize.Create();

			file.VehicleName = "Sedan_02"+color;
			file.m_FileName = file.VehicleName;
			file.Parts.Copy( attArr );

			Vehicles.Insert( file.VehicleName, file );
		}
	}

	void Default_Offroad_02()
	{
		array< string> attArr = new array< string>;

		attArr.Insert("Offroad_02_Hood");
		attArr.Insert("Offroad_02_Trunk");
		attArr.Insert("Offroad_02_Door_1_1");
		attArr.Insert("Offroad_02_Door_1_2");
		attArr.Insert("Offroad_02_Door_2_1");
		attArr.Insert("Offroad_02_Door_2_2");
		attArr.Insert("Offroad_02_Wheel");
		attArr.Insert("Offroad_02_Wheel");
		attArr.Insert("Offroad_02_Wheel");
		attArr.Insert("Offroad_02_Wheel");
		attArr.Insert("CarBattery");
		attArr.Insert("GlowPlug");
		attArr.Insert("HeadlightH7");
		attArr.Insert("HeadlightH7");

		JMVehicleSpawnerSerialize file = JMVehicleSpawnerSerialize.Create();

		file.VehicleName = "Offroad_02";
		file.m_FileName = file.VehicleName;
		file.Parts.Copy( attArr );

		Vehicles.Insert( file.VehicleName, file );
	}

	void Default_Truck_01_Covered()
	{
		array<string> colorArr = {"","_Orange","_Blue"};

		foreach (string color : colorArr)
		{
			array< string> attArr = new array< string>;

			attArr.Insert("Truck_01_Hood"+color);
			attArr.Insert("Truck_01_Door_1_1"+color);
			attArr.Insert("Truck_01_Door_2_1"+color);
			attArr.Insert("Truck_01_WheelDouble");
			attArr.Insert("Truck_01_WheelDouble");
			attArr.Insert("Truck_01_WheelDouble");
			attArr.Insert("Truck_01_WheelDouble");
			attArr.Insert("Truck_01_Wheel");
			attArr.Insert("Truck_01_Wheel");
			attArr.Insert("Truck_01_Wheel");
			attArr.Insert("Truck_01_Wheel");
			attArr.Insert("TruckBattery");
			attArr.Insert("HeadlightH7");
			attArr.Insert("HeadlightH7");

			JMVehicleSpawnerSerialize file = JMVehicleSpawnerSerialize.Create();

			file.VehicleName = "Truck_01_Covered"+color;
			file.m_FileName = file.VehicleName;
			file.Parts.Copy( attArr );

			Vehicles.Insert( file.VehicleName, file );
		}
	}
};