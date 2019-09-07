const string ITEM_SETS_FOLDER = JMConstants.DIR_COT + "ItemSets\\";

class JMItemSetSerialize
{
	[NonSerialized()]
	string m_FileName;

	string Name;
	string ContainerClassName;

	ref array< ref JMItemSetItemInfo > Items = new array< ref JMItemSetItemInfo >;

	static ref JMItemSetSerialize Load( string file )
	{
		ref JMItemSetSerialize settings = new JMItemSetSerialize;
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