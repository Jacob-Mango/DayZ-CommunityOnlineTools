class JMItemSetSettings
{
	autoptr map< string, ref JMItemSetSerialize > ItemSets = new map< string, ref JMItemSetSerialize >;
	
	static ref JMItemSetSettings Load()
	{
		ref JMItemSetSettings settings = new JMItemSetSettings();

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
				//fileType = files[i].Substring( pos, files[i].Length() - 1 );

				//if ( fileType == JMConstants.EXT_ITEM )
				//{
					settings.ItemSets.Insert( fileName, JMItemSetSerialize.Load( fileName ) );
				//}
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