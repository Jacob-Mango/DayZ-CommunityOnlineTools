const string ITEM_SETS_FOLDER = ROOT_COT_DIR + "ItemSets";

class ItemSetFile
{
	string Name;
	string ContainerClassName;

	ref array< ref ItemSetItemInformation > Items = new ref array< ref ItemSetItemInformation >;

	static ref ItemSetFile Load( string file )
	{
		ref ItemSetFile settings = new ref ItemSetFile;

		JsonFileLoader< ItemSetFile >.JsonLoadFile( ITEM_SETS_FOLDER + "/" + file, settings );

		return settings;
	}

	void Save()
	{
		JsonFileLoader< ItemSetFile >.JsonSaveFile( ITEM_SETS_FOLDER + "/" + Name, this );
	}

	void Defaults()
	{
	}

}