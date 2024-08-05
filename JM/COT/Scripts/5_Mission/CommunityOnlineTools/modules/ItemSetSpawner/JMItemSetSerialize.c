const string ITEM_SETS_FOLDER = JMConstants.DIR_COT + "ItemSets\\";

class JMItemSetSerialize
{
	[NonSerialized()]
	string m_FileName;

	string Name;
	string ContainerClassName;

	ref array< ref JMItemSetItemInfo > Items;
	
	void JMItemSetSerialize()
	{
		Items = new array< ref JMItemSetItemInfo >;
	}

	static JMItemSetSerialize Create()
	{
		return new JMItemSetSerialize;
	}

	static JMItemSetSerialize Load( string file )
	{
		JMItemSetSerialize settings = new JMItemSetSerialize;
		settings.m_FileName = file;

		string errorMessage;
		if ( !JsonFileLoader< JMItemSetSerialize >.LoadFile( JMConstants.DIR_ITEMS + settings.m_FileName + JMConstants.EXT_ITEM, settings, errorMessage ) )
		{
			Print("[Community Online Tools] ItemSets Module - " + errorMessage);
			return NULL;
		}

		return settings;
	}

	void AddItem( Object obj )
	{
		//string item, int numStacks, int stackSize
	}

	void Save()
	{
		JsonFileLoader< JMItemSetSerialize >.JsonSaveFile( JMConstants.DIR_ITEMS + m_FileName + JMConstants.EXT_ITEM, this );
	}
};
