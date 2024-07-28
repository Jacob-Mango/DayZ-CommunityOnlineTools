class JMItemSetSettings
{
	ref map< string, ref JMItemSetSerialize > ItemSets;
	
	private void JMItemSetSettings()
	{
		ItemSets = new map< string, ref JMItemSetSerialize >;
	}

	static JMItemSetSettings Load()
	{
		JMItemSetSettings settings = new JMItemSetSettings();

		if ( GetGame().IsClient() )
		{
			settings.Defaults();
			return settings;
		}

		if ( !FileExist( JMConstants.DIR_ITEMS ) )
		{
			MakeDirectory( JMConstants.DIR_ITEMS );

			settings.Defaults();
			settings.Save();

			return settings;
		}

		array< string > files = FindFilesInLocation( JMConstants.DIR_ITEMS );

		for ( int i = 0; i < files.Count(); i++ )
		{
			string fileName;
			string fileType;

			int pos = files[i].IndexOf(".");
			
			if ( pos > -1 )
			{
				fileName = files[i].Substring( 0, pos );
				settings.ItemSets.Insert( fileName, JMItemSetSerialize.Load( fileName ) );
			}
		}

		return settings;
	}

	void Save()
	{
		for ( int i = 0; i < ItemSets.Count(); i++ )
		{
			ItemSets.GetElement( i ).Save();
		}
	}

	void Defaults()
	{
		DefaultFenceMetal();
		DefaultFenceWood();

		DefaultGateMetal();
		DefaultGateWood();

		DefaultPlatform();

		DefaultWatchtowerMetal();
		DefaultWatchtowerWood();

		DefaultMedicalSet();
		DefaultNBCSet();
	}

	void DefaultMedicalSet()
	{
		array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

		attArr.Insert( new JMItemSetItemInfo( "TetracyclineAntibiotics", 5, 100 ) );
		attArr.Insert( new JMItemSetItemInfo( "PurificationTablets", 5, 100 ) );
		attArr.Insert( new JMItemSetItemInfo( "DisinfectantAlcohol", 5, 100 ) );
		attArr.Insert( new JMItemSetItemInfo( "PainkillerTablets", 5, 100 ) );
		attArr.Insert( new JMItemSetItemInfo( "AntiChemInjector", 5, 100 ) );
		attArr.Insert( new JMItemSetItemInfo( "BandageDressing", 5, 100 ) );
		attArr.Insert( new JMItemSetItemInfo( "CharcoalTablets", 5, 100 ) );
		attArr.Insert( new JMItemSetItemInfo( "VitaminBottle", 5, 100 ) );
		attArr.Insert( new JMItemSetItemInfo( "Epinephrine", 5, 100 ) );
		attArr.Insert( new JMItemSetItemInfo( "Thermometer", 2, 100 ) );
		attArr.Insert( new JMItemSetItemInfo( "Morphine", 5, 100 ) );
		attArr.Insert( new JMItemSetItemInfo( "Heatpack", 5, 100 ) );		

		JMItemSetSerialize file = JMItemSetSerialize.Create();

		file.Name = "Medical Set";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultNBCSet()
	{
		array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

		attArr.Insert( new JMItemSetItemInfo( "NBCGlovesGray", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "NBCHoodGray", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "NBCPantsGray", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "NBCBootsGray", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "NBCJacketGray", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "AirborneMask", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "AirborneMask", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "GasMask_Filter", 4, 100 ) );

		JMItemSetSerialize file = JMItemSetSerialize.Create();

		file.Name = "NBC Set";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultFenceMetal()
	{
		array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

		attArr.Insert( new JMItemSetItemInfo( "FenceKit", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Woodenlog", 1, 2 ) );
		attArr.Insert( new JMItemSetItemInfo( "WoodenPlank", 1, 8 ) );
		attArr.Insert( new JMItemSetItemInfo( "Nail", 1, 56 ) );
		attArr.Insert( new JMItemSetItemInfo( "MetalPlate", 1, 6 ) );
		attArr.Insert( new JMItemSetItemInfo( "Hammer", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Shovel", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "BarbedWire", 2, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "CamoNet", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "DuctTape", 5, 100 ) );

		JMItemSetSerialize file = JMItemSetSerialize.Create();

		file.Name = "Fence Metal";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultFenceWood()
	{
		array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

		attArr.Insert( new JMItemSetItemInfo( "FenceKit", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Woodenlog", 1, 2 ) );
		attArr.Insert( new JMItemSetItemInfo( "WoodenPlank", 1, 20 ) );
		attArr.Insert( new JMItemSetItemInfo( "Nail", 1, 56 ) );
		attArr.Insert( new JMItemSetItemInfo( "Hammer", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Shovel", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "BarbedWire", 2, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "CamoNet", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "DuctTape", 5, 100 ) );

		JMItemSetSerialize file = JMItemSetSerialize.Create();

		file.Name = "Fence Wood";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultGateMetal()
	{
		array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

		attArr.Insert( new JMItemSetItemInfo( "FenceKit", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Woodenlog", 1, 2 ) );
		attArr.Insert( new JMItemSetItemInfo( "WoodenPlank", 1, 8 ) );
		attArr.Insert( new JMItemSetItemInfo( "Nail", 1, 56 ) );
		attArr.Insert( new JMItemSetItemInfo( "MetalPlate", 1, 6 ) );
		attArr.Insert( new JMItemSetItemInfo( "Hammer", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Shovel", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Pliers", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "BarbedWire", 2, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "CamoNet", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "DuctTape", 5, 100 ) );

		JMItemSetSerialize file = JMItemSetSerialize.Create();

		file.Name = "Gate Metal";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultGateWood()
	{
		array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

		attArr.Insert( new JMItemSetItemInfo( "FenceKit", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Woodenlog", 1, 2 ) );
		attArr.Insert( new JMItemSetItemInfo( "WoodenPlank", 1, 20 ) );
		attArr.Insert( new JMItemSetItemInfo( "Nail", 1, 56 ) );
		attArr.Insert( new JMItemSetItemInfo( "Hammer", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Shovel", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Pliers", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "BarbedWire", 2, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "CamoNet", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "DuctTape", 5, 100 ) );

		JMItemSetSerialize file = JMItemSetSerialize.Create();

		file.Name = "Gate Wood";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultPlatform()
	{
		array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

		attArr.Insert( new JMItemSetItemInfo( "WoodenPlank", 1, 18 ) );
		attArr.Insert( new JMItemSetItemInfo( "Nail", 1, 30 ) );
		attArr.Insert( new JMItemSetItemInfo( "Hammer", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "DuctTape", 5, 100 ) );

		JMItemSetSerialize file = JMItemSetSerialize.Create();

		file.Name = "Platform";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultWatchtowerMetal()
	{
		array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

		attArr.Insert( new JMItemSetItemInfo( "WatchtowerKit", 5, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Woodenlog", 1, 2 ) );
		attArr.Insert( new JMItemSetItemInfo( "WoodenPlank", 1, 8 ) );
		attArr.Insert( new JMItemSetItemInfo( "Nail", 3, 100 ) );
		attArr.Insert( new JMItemSetItemInfo( "MetalPlate", 3, 12 ) );
		attArr.Insert( new JMItemSetItemInfo( "Hammer", 4, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Shovel", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Pliers", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "BarbedWire", 6, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "CamoNet", 9, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Crowbar", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "DuctTape", 5, 100 ) );

		JMItemSetSerialize file = JMItemSetSerialize.Create();

		file.Name = "Watchtower Metal";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultWatchtowerWood()
	{
		array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

		attArr.Insert( new JMItemSetItemInfo( "WatchtowerKit", 5, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Woodenlog", 3, 4 ) );
		attArr.Insert( new JMItemSetItemInfo( "WoodenPlank", 3, 20 ) );
		attArr.Insert( new JMItemSetItemInfo( "Nail", 3, 100 ) );
		attArr.Insert( new JMItemSetItemInfo( "Hammer", 4, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Shovel", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Pliers", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "BarbedWire", 6, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "CamoNet", 9, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Crowbar", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "DuctTape", 5, 100 ) );

		JMItemSetSerialize file = JMItemSetSerialize.Create();

		file.Name = "Watchtower Wood";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}
};
