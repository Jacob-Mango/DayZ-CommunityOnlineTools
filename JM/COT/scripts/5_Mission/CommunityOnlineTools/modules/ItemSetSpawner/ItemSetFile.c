const string ITEM_SETS_FOLDER = ROOT_COT_DIR + "ItemSets/";

class ItemSetFile
{
	[NonSerialized()]
	string m_FileName;

	string Name;
	string ContainerClassName;

	ref array< ref ItemSetItemInformation > Items = new ref array< ref ItemSetItemInformation >;

	static ref ItemSetFile Load( string file )
	{
		ref ItemSetFile settings = new ref ItemSetFile;
		settings.m_FileName = file;

		JsonFileLoader< ItemSetFile >.JsonLoadFile( ITEM_SETS_FOLDER + settings.m_FileName + ".json", settings );

		return settings;
	}

	void Save()
	{
		JsonFileLoader< ItemSetFile >.JsonSaveFile( ITEM_SETS_FOLDER + m_FileName + ".json", this );
	}

	void Defaults()
	{
	}

}