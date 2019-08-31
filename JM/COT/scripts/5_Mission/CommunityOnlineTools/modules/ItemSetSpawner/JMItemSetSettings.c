class JMItemSetSettings
{
	autoptr map< string, ref JMItemSetSerialize > ItemSets = new map< string, ref JMItemSetSerialize >;
	
	static ref JMItemSetSettings Load()
	{
		ref JMItemSetSettings settings = new JMItemSetSettings();

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

				settings.ItemSets.Insert( name, JMItemSetSerialize.Load( name ) );
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
		ref array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

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

		ref JMItemSetSerialize file = new JMItemSetSerialize;

		file.Name = "Fence Metal";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultFenceWood()
	{
		ref array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

		attArr.Insert( new JMItemSetItemInfo( "FenceKit", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Woodenlog", 1, 2 ) );
		attArr.Insert( new JMItemSetItemInfo( "WoodenPlank", 1, 20 ) );
		attArr.Insert( new JMItemSetItemInfo( "Nail", 1, 56 ) );
		attArr.Insert( new JMItemSetItemInfo( "Hammer", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "Shovel", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "BarbedWire", 2, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "CamoNet", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "DuctTape", 5, 100 ) );

		ref JMItemSetSerialize file = new JMItemSetSerialize;

		file.Name = "Fence Wood";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultGateMetal()
	{
		ref array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

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

		ref JMItemSetSerialize file = new JMItemSetSerialize;

		file.Name = "Gate Metal";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultGateWood()
	{
		ref array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

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

		ref JMItemSetSerialize file = new JMItemSetSerialize;

		file.Name = "Gate Wood";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultPlatform()
	{
		ref array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

		attArr.Insert( new JMItemSetItemInfo( "WoodenPlank", 1, 18 ) );
		attArr.Insert( new JMItemSetItemInfo( "Nail", 1, 30 ) );
		attArr.Insert( new JMItemSetItemInfo( "Hammer", 1, 1 ) );
		attArr.Insert( new JMItemSetItemInfo( "DuctTape", 5, 100 ) );

		ref JMItemSetSerialize file = new JMItemSetSerialize;

		file.Name = "Platform";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultWatchtowerMetal()
	{
		ref array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

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

		ref JMItemSetSerialize file = new JMItemSetSerialize;

		file.Name = "Watchtower Metal";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}

	void DefaultWatchtowerWood()
	{
		ref array< ref JMItemSetItemInfo > attArr = new array< ref JMItemSetItemInfo >;

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

		ref JMItemSetSerialize file = new JMItemSetSerialize;

		file.Name = "Watchtower Wood";
		file.m_FileName = file.Name;
		file.ContainerClassName = "SeaChest";
		file.Items = attArr;

		ItemSets.Insert( file.Name, file );
	}
}