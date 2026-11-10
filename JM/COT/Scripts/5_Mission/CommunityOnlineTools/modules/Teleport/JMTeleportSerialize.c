class JMTeleportSerialize : Managed
{
	static const string LOCATION_TYPE_AIRFIELD = "Airfield";
	static const string LOCATION_TYPE_MILITARY_BASE = "Military Base";

	protected static ref JMTeleportSerialize s_WorldDefaults;

	ref TStringArray Types;
	ref array< ref JMTeleportLocation > Locations;
	[NonSerialized()]
	ref map<string, ref JMTeleportLocation> m_LocationLookup = new map<string, ref JMTeleportLocation>;
	[NonSerialized()]
	string m_WorldName;
	[NonSerialized()]
	string m_FileName;

	protected void JMTeleportSerialize()
	{
	 	Types = new TStringArray;
		Locations = new array< ref JMTeleportLocation >;
		m_WorldName = g_Game.GetWorldName();
		m_FileName = JMConstants.FILE_TELEPORT + m_WorldName + JMConstants.EXT_TELEPORT;
	}

	static JMTeleportSerialize Load()
	{
		JMTeleportSerialize settings = new JMTeleportSerialize();

		bool changed;

		if ( JMJsonFile<JMTeleportSerialize>.Load( settings.m_FileName, settings ) && settings.Locations.Count() > 0 )
		{
			int idx;

			idx =  settings.Types.Find("ALL");
			if (idx > -1)
			{
				settings.Types.Remove(idx);
				changed = true;
			}

			//! Remove duplicate entries (based on name) and fix empty type

			TStringArray locationNames = {};

			foreach (JMTeleportLocation location: settings.Locations)
			{
				if (!location.Type)
				{
					idx = location.Name.IndexOf(" | ");
					if (idx > -1)
					{
						location.Type = location.Name.Substring(0, idx);

						if (location.Type == "Local")
						{
							idx = location.Name.IndexOf(" | Military base ");
							if (idx > -1)
								location.Type = LOCATION_TYPE_MILITARY_BASE;
						}

						location.Name = location.Name.Substring(idx + 3, location.Name.Length() - idx - 3);
						changed = true;
					}
				}

				//! This error was in both COT and Expansion Navigation CfGworlds ChernarusPlus Names overrides,
				//! always fix a mismatch in loaded teleport locations
				if (location.Name == "Tikhiy zaliv" && settings.m_WorldName == "chernarusplus")
				{
					if (Math.Round(location.Position[0]) == 5735 && Math.Round(location.Position[2]) == 1919)
					{
						location.Name = "Mutny zaliv";
						changed = true;
					}
				}

				if (settings.Types.Find(location.Type) == -1)
				{
					settings.Types.Insert(location.Type);
					changed = true;
				}

				if (!settings.m_LocationLookup[location.Name])
				{
					locationNames.Insert(location.Name);
					settings.m_LocationLookup[location.Name] = location;
				}
			}

			settings.Types.Sort();  //! Don't consider just sorting a change

			if (changed)
			{
				settings.Locations.Clear();

				foreach (string name: locationNames)
				{
					settings.Locations.Insert(settings.m_LocationLookup[name]);
				}
			}
		}
		else
		{
			settings.CopyFromWorldDefaults();
			changed = true;
		}

		if (changed)
			settings.Save();

		return settings;
	}

	static JMTeleportSerialize Create()
	{
		return new JMTeleportSerialize();
	}

	void Save()
	{
		JMJsonFile<JMTeleportSerialize>.Save( m_FileName, this );
	}

	void AddLocation( string type, string name, vector position, float radius = 4.0 )
	{
		if ( Types.Find(type) == -1 )
			Types.Insert(type);

		JMTeleportLocation location = new JMTeleportLocation( type, name, position, radius );
		Locations.Insert( location );
		m_LocationLookup[name] = location;
	}

	void RemoveLocation(int index)
	{
		JMTeleportLocation location = Locations[index];
		if (location)
		{
			Locations.RemoveOrdered(index);
			m_LocationLookup.Remove(location.Name);

			RemoveTypeIfUnused(location.Type);
		}
	}

	void RemoveTypeIfUnused(string type)
	{
		foreach (JMTeleportLocation location: Locations)
		{
			if (location.Type == type)
				return;
		}

		Types.RemoveItem(location.Type);
	}

	void RemoveLocation(string name)
	{
		JMTeleportLocation location = m_LocationLookup[name];
		if (location)
		{
			int index = Locations.Find(location);
			if (index > -1)
				RemoveLocation(index);
		}
	}

	static JMTeleportSerialize CreateWorldDefaults()
	{
		JMTeleportSerialize settings = new JMTeleportSerialize();
		settings.Defaults();
		return settings;
	}

	static JMTeleportSerialize GetWorldDefaults()
	{
		if (!s_WorldDefaults)
			s_WorldDefaults = CreateWorldDefaults();

		return s_WorldDefaults;
	}

	void Defaults()
	{
		Types.Clear();
		Locations.Clear();
		m_LocationLookup.Clear();

		string location_config_path = "CfgWorlds " + m_WorldName + " Names";
		int classNamesCount = g_Game.ConfigGetChildrenCount( location_config_path );
		TStringArray locationNames = {};
		
		for ( int i = 0; i < classNamesCount; ++i ) 
		{
			string location_class_name;
			g_Game.ConfigGetChildName( location_config_path, i, location_class_name );

			string location_class_name_path = location_config_path + " " + location_class_name;

			string location_name_path = location_class_name_path + " name";
			string location_type_path = location_class_name_path + " type";
			string location_position_path = location_class_name_path + " position";

			string location_name;
			string location_type;
			g_Game.ConfigGetText( location_name_path, location_name );
			g_Game.ConfigGetText( location_type_path, location_type );

			TFloatArray location_position = new TFloatArray;
			g_Game.ConfigGetFloatArray( location_position_path, location_position );
			
			if (location_position == null || location_position.Count() != 2)
				continue;

			if ( location_name == "" )
				continue;

			if ( location_type == "" )
				continue;

			if ( location_type.IndexOf("Name") == 0 && location_type.Length() > 4 )
				location_type = location_type.Substring(4, location_type.Length() - 4);

			//! This error was in both COT and Expansion Navigation CfGworlds ChernarusPlus Names overrides,
			//! since it probably was copied to countless other mods as well, always fix a mismatch
			if ( location_class_name == "Marine_Mutnyizaliv" && location_name == "Tikhiy zaliv" )
				location_name = "Mutny zaliv";
				
			if ( location_class_name.IndexOf("AF") == 0 )
				location_type = LOCATION_TYPE_AIRFIELD;
				
			if ( location_class_name.IndexOf("MB") == 0 )
				location_type = LOCATION_TYPE_MILITARY_BASE;

			TStringArray output = new TStringArray;
			location_class_name.Split("_", output);
			foreach(string slice: output)
			{
				if ( slice == "AF" )
					location_type = LOCATION_TYPE_AIRFIELD;
				else if ( slice == "MB" )
					location_type = LOCATION_TYPE_MILITARY_BASE;
			}

			locationNames.Insert(location_name);
			m_LocationLookup[location_name] = new JMTeleportLocation(location_type, location_name, Vector(location_position[0], 0, location_position[1]), 4.0);
		}

		Types.Sort();
	
		JMStatics.SortStringArrayIgnoreCase(locationNames);

		foreach (string name: locationNames)
		{
			Locations.Insert(m_LocationLookup[name]);
		}
	}

	void CopyFromWorldDefaults()
	{
		JMTeleportSerialize worldDefaults = GetWorldDefaults();

		Types.Copy(worldDefaults.Types);

		Locations.Clear();
		m_LocationLookup.Clear();
		foreach (JMTeleportLocation location: worldDefaults.Locations)
		{
			AddLocation(location.Type, location.Name, location.Position, location.Radius);
		}
	}

	JMTeleportLocation FindLocation(string name)
	{
		return m_LocationLookup[name];
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
		m_LocationLookup.Clear();
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

			AddLocation( type, name, position, radius );
		}

		return true;
	}
}
