class JMTeleportSerialize : Managed
{
	ref array< ref JMTeleportLocation > Locations;

	private void JMTeleportSerialize()
	{
		Locations = new array< ref JMTeleportLocation >;
	}

	void ~JMTeleportSerialize()
	{
		delete Locations;
	}

	static JMTeleportSerialize Load()
	{
		JMTeleportSerialize settings = new JMTeleportSerialize();

		if ( FileExist( JMConstants.FILE_TELEPORT ) )
		{
			JsonFileLoader<JMTeleportSerialize>.JsonLoadFile( JMConstants.FILE_TELEPORT, settings );
		} else 
		{
			settings.Defaults();
			settings.Save();
		}

		return settings;
	}

	void Save()
	{
		JsonFileLoader<JMTeleportSerialize>.JsonSaveFile( JMConstants.FILE_TELEPORT, this );
	}

	void AddLocation( string name, vector position, float radius = 4.0 )
	{
		string permission = name + "";
		permission.Replace( " ", "" );
		permission.Replace( "'", "" );
		permission.Replace( "(", "" );
		permission.Replace( ")", "" );
		permission.Replace( "[", "" );
		permission.Replace( "]", "" );
		permission.Replace( "{", "" );
		permission.Replace( "}", "" );
		permission.Replace( "\\", "" );
		permission.Replace( "/", "" );

		Locations.Insert( new JMTeleportLocation( name, permission, position, radius ) );
	}

	void Defaults()
	{
		string location_config_path = "CfgWorlds " + GetGame().GetWorldName() + " Names";
		int classNamesCount = GetGame().ConfigGetChildrenCount( location_config_path );

		TStringArray locations = { "AF", "MB", "Camp", "Ruin", "Settlement" };
		
		for ( int l = 0; l < classNamesCount; ++l ) 
		{
			string location_class_name;
			GetGame().ConfigGetChildName( location_config_path, l, location_class_name );

			string location_class_name_path = location_config_path + " " + location_class_name;

			string location_name_path = location_class_name_path + " name";
			string location_type_path = location_class_name_path + " type";
			string location_position_path = location_class_name_path + " position";

			string location_name;
			string location_type;
			GetGame().ConfigGetText( location_name_path, location_name );
			GetGame().ConfigGetText( location_type_path, location_type );

			TFloatArray location_position = new TFloatArray;
			GetGame().ConfigGetFloatArray( location_position_path, location_position );
			
			if (location_position == null || location_position.Count() != 2)
				continue;

			foreach(string location: locations)
			{
				if ( location_class_name.Contains( location ) )
				{
					int loc_length = location.Length() + 1;
					if ( loc_length == 3 )
						loc_length = 9;

					location_name = location_class_name.Substring(loc_length, location_class_name.Length());

					switch (location)
					{
						case "AF":
							location_name = "Airfield | " + location_name;
						break;
						case "MB":
							location_name = "Millitary Base | " + location_name;
						break;
						case "Camp":
						case "Ruin":
							location_name = location + " | " + location_name;
						break;
						case "Settlement":
							if ( location_type == "Local")
								location_type = "Unknown";

							location_name = location_type + " | " + location_name;
						break;
					}

					AddLocation( location_name, Vector(location_position[0], 0, location_position[1]) );
				}
			}
		}

		//Sort(Locations,Locations.Count());
	}
}