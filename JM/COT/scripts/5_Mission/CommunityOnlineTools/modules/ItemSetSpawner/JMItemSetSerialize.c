const string ITEM_SETS_FOLDER = ROOT_COT_DIR + "ItemSets\\";

class JMItemSetSerialize
{
	[NonSerialized()]
	string m_FileName;

	string Name;
	string ContainerClassName;

	ref array< ref JMItemSetItemInfo > Items = new ref array< ref JMItemSetItemInfo >;

	static ref JMItemSetSerialize Load( string file )
	{
		ref JMItemSetSerialize settings = new ref JMItemSetSerialize;
		settings.m_FileName = file;

		JsonFileLoader< JMItemSetSerialize >.JsonLoadFile( ITEM_SETS_FOLDER + settings.m_FileName + ".json", settings );

		return settings;
	}

	void Save()
	{
		JsonFileLoader< JMItemSetSerialize >.JsonSaveFile( ITEM_SETS_FOLDER + m_FileName + ".json", this );
	}

	void Defaults()
	{
	}

}