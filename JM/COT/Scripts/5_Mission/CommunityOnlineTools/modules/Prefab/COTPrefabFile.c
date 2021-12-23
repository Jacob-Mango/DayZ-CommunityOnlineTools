class COTPrefabFile : COTPrefabObject
{
	[NonSerialized()]
	static ref array<COTPrefabObject> s_Begin = new array<COTPrefabObject>();

	[NonSerialized()]
	string m_FileName;

	string DisplayName;

	static COTPrefabFile Load(string fileName)
	{
		COTPrefabFile prefab = new COTPrefabFile();
		prefab.m_FileName = fileName;
		JsonFileLoader<COTPrefabFile>.JsonLoadFile(JMConstants.DIR_PREFABS + prefab.m_FileName + JMConstants.EXT_PREFAB, prefab);
		return prefab;
	}

	override void Save()
	{
		JsonFileLoader<COTPrefabFile>.JsonSaveFile(JMConstants.DIR_PREFABS + m_FileName + JMConstants.EXT_PREFAB, this);
	}

	void Spawn(vector position, vector orientation)
	{
		Object object = GetGame().CreateObjectEx(ClassName, position, ECE_PLACE_ON_SURFACE);
		object.SetOrientation(orientation);
		Spawn(object);
	}

	static COTPrefabFile Create(string className, string displayName)
	{
		COTPrefabFile prefab = new COTPrefabFile();
		prefab.ClassName = className;
		prefab.DisplayName = displayName;
		prefab.m_FileName = displayName;

		prefab.m_FileName.Replace( "\\", "" );
		prefab.m_FileName.Replace( "/", "" );
		prefab.m_FileName.Replace( "=", "" );
		prefab.m_FileName.Replace( "+", "" );

		s_Begin.Clear();

		GetModuleManager().GetModule(COTPrefabModule).PrefabFiles.Insert(prefab);

		return prefab;
	}


};
