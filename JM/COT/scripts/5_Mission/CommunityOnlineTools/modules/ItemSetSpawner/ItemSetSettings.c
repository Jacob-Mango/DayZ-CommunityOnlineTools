class ItemSetSettings
{
	autoptr map< string, ref ItemSetFile > ItemSets = new map< string, ref ItemSetFile >;
	
	static ref ItemSetSettings Load()
	{
		ref ItemSetSettings settings = new ItemSetSettings();

		if ( GetGame().IsClient() || ( !GetGame().IsMultiplayer() && GetGame().IsServer() ) )
		{
			settings.Defaults();
			return settings;
		}
		
		MakeDirectory( ITEM_SETS_FOLDER );

		array< string > files = FindFilesInLocation( ITEM_SETS_FOLDER );

		if ( FileExist( ITEM_SETS_FOLDER + COT_FILE_EXIST ) ) 
		{
			GetLogger().Log( "Found existence ( " + ITEM_SETS_FOLDER + " )", "JM_COT_ItemSetSpawner" );
			for ( int i = 0; i < files.Count(); i++ )
			{
				if ( files[i] == COT_FILE_EXIST )
					continue;

				string name = files[i];
				int pos = files[i].IndexOf(".");
				
				if ( pos > -1 )
				{
					name = files[i].Substring( 0, pos );
				}

				settings.ItemSets.Insert( name, ItemSetFile.Load( name ) );
				GetLogger().Log( "	Loading item set file ( " + name + " )", "JM_COT_ItemSetSpawner" );
			}
		} else 
		{
			GetLogger().Log( "Didn't find existence ( " + ITEM_SETS_FOLDER + " )", "JM_COT_ItemSetSpawner" );
			DeleteFiles( ITEM_SETS_FOLDER, files );

			settings.Defaults();
			settings.Save();
		}

		return settings;
	}

	void Save()
	{
		for ( int i = 0; i < ItemSets.Count(); i++ )
		{
			ItemSets.GetElement( i ).Save();
		}

		CreateFilesExist( ITEM_SETS_FOLDER );
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
	}

	void DefaultFenceMetal()
	{
		ref array< ref ItemSetItemInformation > attArr = new ref array< ref ItemSetItemInformation >;

		attArr.Insert( new ItemSetItemInformation( "FenceKit", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Woodenlog", 1, 2 ) );
		attArr.Insert( new ItemSetItemInformation( "WoodenPlank", 1, 8 ) );
		attArr.Insert( new ItemSetItemInformation( "Nail", 1, 56 ) );
		attArr.Insert( new ItemSetItemInformation( "MetalPlate", 1, 6 ) );
		attArr.Insert( new ItemSetItemInformation( "Hammer", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Shovel", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "BarbedWire", 2, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "CamoNet", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "DuctTape", 5, 100 ) );

		ref ItemSetFile file = new ItemSetFile;

		file.Name = "Fence Metal";
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultFenceWood()
	{
		ref array< ref ItemSetItemInformation > attArr = new ref array< ref ItemSetItemInformation >;

		attArr.Insert( new ItemSetItemInformation( "FenceKit", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Woodenlog", 1, 2 ) );
		attArr.Insert( new ItemSetItemInformation( "WoodenPlank", 1, 20 ) );
		attArr.Insert( new ItemSetItemInformation( "Nail", 1, 56 ) );
		attArr.Insert( new ItemSetItemInformation( "Hammer", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Shovel", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "BarbedWire", 2, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "CamoNet", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "DuctTape", 5, 100 ) );

		ref ItemSetFile file = new ItemSetFile;

		file.Name = "Fence Wood";
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultGateMetal()
	{
		ref array< ref ItemSetItemInformation > attArr = new ref array< ref ItemSetItemInformation >;

		attArr.Insert( new ItemSetItemInformation( "FenceKit", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Woodenlog", 1, 2 ) );
		attArr.Insert( new ItemSetItemInformation( "WoodenPlank", 1, 8 ) );
		attArr.Insert( new ItemSetItemInformation( "Nail", 1, 56 ) );
		attArr.Insert( new ItemSetItemInformation( "MetalPlate", 1, 6 ) );
		attArr.Insert( new ItemSetItemInformation( "Hammer", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Shovel", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Pliers", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "BarbedWire", 2, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "CamoNet", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "DuctTape", 5, 100 ) );

		ref ItemSetFile file = new ItemSetFile;

		file.Name = "Gate Metal";
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultGateWood()
	{
		ref array< ref ItemSetItemInformation > attArr = new ref array< ref ItemSetItemInformation >;

		attArr.Insert( new ItemSetItemInformation( "FenceKit", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Woodenlog", 1, 2 ) );
		attArr.Insert( new ItemSetItemInformation( "WoodenPlank", 1, 20 ) );
		attArr.Insert( new ItemSetItemInformation( "Nail", 1, 56 ) );
		attArr.Insert( new ItemSetItemInformation( "Hammer", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Shovel", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Pliers", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "BarbedWire", 2, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "CamoNet", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "DuctTape", 5, 100 ) );

		ref ItemSetFile file = new ItemSetFile;

		file.Name = "Gate Wood";
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultPlatform()
	{
		ref array< ref ItemSetItemInformation > attArr = new ref array< ref ItemSetItemInformation >;

		attArr.Insert( new ItemSetItemInformation( "WoodenPlank", 1, 18 ) );
		attArr.Insert( new ItemSetItemInformation( "Nail", 1, 30 ) );
		attArr.Insert( new ItemSetItemInformation( "Hammer", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "DuctTape", 5, 100 ) );

		ref ItemSetFile file = new ItemSetFile;

		file.Name = "Platform";
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultWatchtowerMetal()
	{
		ref array< ref ItemSetItemInformation > attArr = new ref array< ref ItemSetItemInformation >;

		attArr.Insert( new ItemSetItemInformation( "WatchtowerKit", 5, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Woodenlog", 1, 2 ) );
		attArr.Insert( new ItemSetItemInformation( "WoodenPlank", 1, 8 ) );
		attArr.Insert( new ItemSetItemInformation( "Nail", 3, 100 ) );
		attArr.Insert( new ItemSetItemInformation( "MetalPlate", 3, 12 ) );
		attArr.Insert( new ItemSetItemInformation( "Hammer", 4, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Shovel", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Pliers", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "BarbedWire", 6, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "CamoNet", 9, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Crowbar", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "DuctTape", 5, 100 ) );

		ref ItemSetFile file = new ItemSetFile;

		file.Name = "Watchtower Metal";
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultWatchtowerWood()
	{
		ref array< ref ItemSetItemInformation > attArr = new ref array< ref ItemSetItemInformation >;

		attArr.Insert( new ItemSetItemInformation( "WatchtowerKit", 5, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Woodenlog", 3, 4 ) );
		attArr.Insert( new ItemSetItemInformation( "WoodenPlank", 3, 20 ) );
		attArr.Insert( new ItemSetItemInformation( "Nail", 3, 100 ) );
		attArr.Insert( new ItemSetItemInformation( "Hammer", 4, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Shovel", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Pliers", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "BarbedWire", 6, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "CamoNet", 9, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "Crowbar", 1, 1 ) );
		attArr.Insert( new ItemSetItemInformation( "DuctTape", 5, 100 ) );

		ref ItemSetFile file = new ItemSetFile;

		file.Name = "Watchtower Wood";
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}
}