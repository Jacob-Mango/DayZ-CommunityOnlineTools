const string ITEM_SETS_FOLDER = JMConstants.DIR_COT + "ItemSets\\";

class JMItemSetSerialize
{
	[NonSerialized()]
	string m_FileName;

	string Name;
	string ContainerClassName;

	ref array< ref JMItemSetItemInfo > Items;
	
	/*private*/ void JMItemSetSerialize()
	{
		Items = new array< ref JMItemSetItemInfo >;
	}

	void ~JMItemSetSerialize()
	{
		delete Items;
	}

	static ref JMItemSetSerialize Create()
	{
		return new JMItemSetSerialize;
	}

	static ref JMItemSetSerialize Load( string file )
	{
		ref JMItemSetSerialize settings = new JMItemSetSerialize;
		settings.m_FileName = file;

		JsonFileLoader< JMItemSetSerialize >.JsonLoadFile( JMConstants.DIR_ITEMS + settings.m_FileName + JMConstants.EXT_ITEM, settings );

		return settings;
	}

	void Save()
	{
		JsonFileLoader< JMItemSetSerialize >.JsonSaveFile( JMConstants.DIR_ITEMS + m_FileName + JMConstants.EXT_ITEM, this );
	}
}