class JMTeleportSerialize : Managed
{
	ref TStringArray Types;
	ref array< ref JMTeleportLocation > Locations;

	[NonSerialized()]
	string m_WorldName;

	static string m_FileName;

	private void JMTeleportSerialize()
	{
	 	Types = new TStringArray;
		Locations = new array< ref JMTeleportLocation >;
		m_WorldName = GetGame().GetWorldName();
		m_FileName = JMConstants.FILE_TELEPORT + m_WorldName + JMConstants.EXT_TELEPORT;
	}

	static JMTeleportSerialize Load()
	{
		JMTeleportSerialize settings = new JMTeleportSerialize();

		if ( FileExist( m_FileName ) )
		{
			JsonFileLoader<JMTeleportSerialize>.JsonLoadFile( m_FileName, settings );
			
			//! we dont have versionning so this is a fail safe
			if ( settings.Types.Count() == 0 )
			{
				settings.Defaults();
				settings.Save();
			}
			else if (settings.Types[0] == "")
			{
				settings.Types[0] = "ALL";
				settings.Save();
			}
		} else {
			settings.Defaults();
			settings.Save();
		}

		return settings;
	}

	static JMTeleportSerialize Create()
	{
		return new JMTeleportSerialize();
	}

	void Save()
	{
		JsonFileLoader<JMTeleportSerialize>.JsonSaveFile( m_FileName, this );
	}

	void AddLocation( string type, string name, vector position, float radius = 4.0 )
	{
		Locations.Insert( new JMTeleportLocation( type, name, position, radius ) );
	}

	void Defaults()
	{
		string location_config_path = "CfgWorlds " + m_WorldName + " Names";
		int classNamesCount = GetGame().ConfigGetChildrenCount( location_config_path );
		
		for ( int i = 0; i < classNamesCount; ++i ) 
		{
			string location_class_name;
			GetGame().ConfigGetChildName( location_config_path, i, location_class_name );

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

			if ( location_name == "" )
				continue;

			if ( location_type == "" )
				continue;

			if ( location_type.Substring(0, 4) == "Name" )
				location_type = location_type.Substring(4, location_name.Length());
				
			if ( location_class_name.Substring(0, 2) == "AF" )
				location_type = "Airfield";
				
			if ( location_class_name.Substring(0, 2) == "MB" )
				location_type = "Military Base";

			TStringArray output = new TStringArray;
			location_class_name.Split("_", output);
			foreach(string slice: output)
			{
				if ( slice == "AF" )
				{
					location_type = "Airfield";
				}
				else if ( slice == "MB" )
				{
					location_type = "Military Base";
				}
			}

			if ( Types.Find(location_type) == -1 )
				Types.Insert(location_type);

			AddLocation( location_type, location_name, Vector(location_position[0], 0, location_position[1]) );
		}

		Types.Sort();
		Types.InsertAt("ALL", 0);
	
		ref array< ref string > AfterSorting = new array< ref string >;
		ref array< ref string > BeforeSorting = new array< ref string >;

		int loc_count = Locations.Count();
		
		for ( int l = 0; l < loc_count; ++l ) 
		{
			BeforeSorting.Insert(Locations[l].Name);
			AfterSorting.Insert(Locations[l].Name);
		}

		AfterSorting.Sort();

		ref array< ref JMTeleportLocation > tmp_locations = new array< ref JMTeleportLocation >;
		
		for ( int j = 0; j < loc_count; ++j ) 
		{
			int key = BeforeSorting.Find(AfterSorting[j]);
			tmp_locations.Insert(Locations[key]);	
		}

		Locations = new array< ref JMTeleportLocation >;
		Locations = tmp_locations;
	}

	void Write( ParamsWriteContext ctx )
	{
		ctx.Write( Types );

		ctx.Write( Locations.Count() );
		foreach ( auto location: Locations )
		{
			location.Write( ctx );
		}
	}

	bool Read( ParamsReadContext ctx )
	{
		TStringArray locTypes
		if ( !ctx.Read( locTypes ) )
			return false;

		Types = locTypes;

		int count;
		if ( !ctx.Read( count ) )
			return false;

		Locations.Clear();
		for ( int i = 0; i < count; i++ )
		{
			string type;
			if ( !ctx.Read( type ) )
				return false;

			string name;
			if ( !ctx.Read( name ) )
				return false;

			vector position;
			if ( !ctx.Read( position ) )
				return false;

			float radius;
			if ( !ctx.Read( radius ) )
				return false;

			Locations.Insert( new JMTeleportLocation( type, name, position, radius ) );
		}

		return true;
	}
};
