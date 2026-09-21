class JMItemStatsForm: JMFormBase
{
	protected ref array<ref JMItemStatsBaseData> m_Items;
	protected UIActionScroller m_Scroller;
	protected UIActionButton m_Button;
	protected UIActionNavigateButton 	m_ItemModeButton;
	protected ref array< string > m_ItemModeButtonWordList =
	{
		"Clothing",
		"Weapon",
		"Edible",
		"Vehicles",
		"Magazine",
		"Optic",
		"All Items"
	};
	protected ref array< string > m_ItemTypeList =
	{
		"clothing_base",
		"weapon_base",
		"edible_base",
		"transport",
		"magazine_base",
		"itemoptics",
		"inventory_base"
	};
	protected JMItemStatsEnum m_ItemMode;

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMItemStatsModule m_Module;

	//! GetExportData() is virtual on the data classes, so no cast per mode is needed.
	string GetExportedItemData(JMItemStatsBaseData itemData)
	{
		return itemData.GetExportData();
	}

	//! The CSV header row for the current mode. GetItemData() is the one place that maps a mode to its
	//! data class, so this asks it instead of repeating the mapping.
	string GetHeaderItemData()
	{
		JMItemStatsBaseData itemData = GetItemData("", "");

		if ( !itemData )
			return "";

		return itemData.GetExportHeaderData();
	}

	JMItemStatsBaseData GetItemData(string path, string name)
	{
		switch(m_ItemMode)
		{
			default:
			case JMItemStatsEnum.CLOTHING:
				return new JMItemStatsClothingData(path, name);
			break;
			case JMItemStatsEnum.FIREARM:
				return new JMItemStatsFirearmData(path, name);
			break;
			case JMItemStatsEnum.MAGAZINE:
				return new JMItemStatsMagazineData(path, name);
			break;
			case JMItemStatsEnum.VEHICLE:
				return new JMItemStatsVehicleData(path, name);
			break;
			case JMItemStatsEnum.EDIBLE:
				return new JMItemStatsEdibleData(path, name);
			break;
			case JMItemStatsEnum.OPTIC:
				return new JMItemStatsOpticData(path, name);
			break;
			case JMItemStatsEnum.ITEMS:
				return new JMItemStatsBaseData(path, name);
			break;
		}
		return NULL;
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnCreate()
	{
		UIActionCard card = UIActionManager.CreateScrollCard( layoutRoot.FindAnyWidget( "panel" ), "#STR_COT_ITEMSTATS_SECTION_EXPORT", m_Scroller );
		Widget body = card.GetContent();

		m_ItemModeButton = UIActionManager.CreateNavButton( body, m_ItemModeButtonWordList[0], JM_COT_ICON_ARROW_LEFT, JM_COT_ICON_ARROW_RIGHT, this, "OnClick_NavButton" );
		m_ItemModeButton.SetTooltip( "#STR_COT_ITEMSTATS_TT_CATEGORY" );

		m_Button = UIActionManager.CreateButton( body, "#STR_COT_ITEMSTATS_GENERATE_DATA", this, "OnClick_Button" );
		m_Button.SetTooltip( "#STR_COT_ITEMSTATS_TT_GENERATE" );

		m_Scroller.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_Scroller )
			m_Scroller.UpdateScroller();
	}

	void OnClick_NavButton( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK || eid == UIEvent.CLICK_RIGHTSIDE )
			m_ItemMode++;
		else if ( eid == UIEvent.CLICK_LEFTSIDE )
			m_ItemMode--;
		else
			return;

		if ( m_ItemMode < 0 )
			m_ItemMode = 0;

		if ( m_ItemMode > m_ItemModeButtonWordList.Count() - 1 )
			m_ItemMode = m_ItemModeButtonWordList.Count() - 1;

		m_ItemModeButton.SetButton(m_ItemModeButtonWordList[m_ItemMode]);
	}

	void OnClick_Button( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		GenerateData();

		if (!m_Items)
		{
			action.AnimateError();
			return;
		}

		CreateFile();

		action.AnimateFeedback();

		COTCreateLocalAdminNotification( new StringLocaliser( "Created ItemStatsExported.csv" ) );
	}

	void GenerateData()
	{
		m_Items = new array<ref JMItemStatsBaseData>;
		
		TStringArray configs = new TStringArray;
		configs.Insert( CFG_VEHICLESPATH );
		configs.Insert( CFG_WEAPONSPATH );
		configs.Insert( CFG_MAGAZINESPATH );
		int configCount = configs.Count();
		
		for ( int nConfig = 0; nConfig < configCount; nConfig++ )
		{
			string strConfigPath = configs.Get( nConfig );

			int nClasses = g_Game.ConfigGetChildrenCount( strConfigPath );

			int nClassStart = 0;
			if (nConfig == 0)
				nClassStart = 20;

			for ( int nClass = nClassStart; nClass < nClasses; nClass++ )
			{
				string strName;

				g_Game.ConfigGetChildName( strConfigPath, nClass, strName );

				int scope = g_Game.ConfigGetInt( strConfigPath + " " + strName + " scope" );

				if ( scope != 2 )
					continue;

				string model;
				if (!g_Game.ConfigGetText(strConfigPath + " " + strName + " model", model) || model == string.Empty || model == "bmp")
					continue;

				string strNameLower = strName;

				strNameLower.ToLower();

				if (m_ItemTypeList[m_ItemMode] == "" || g_Game.IsKindOf( strNameLower, m_ItemTypeList[m_ItemMode] ))
				{
					if ( m_Module.IsExcludedClassName( strNameLower ) ) 
						continue;

					JMItemStatsBaseData itemData = GetItemData(strConfigPath + " " + strName, strName);
					if (!itemData)
						continue;

					m_Items.Insert(itemData);
				}
			}
		}
	}

	void CreateFile()
	{
		if (!FileExist(JMConstants.DIR_EXPORTS))
			MakeDirectory(JMConstants.DIR_EXPORTS);

		FileHandle CSVFile;
        string filepath = JMConstants.DIR_EXPORTS + "ItemStatsExported" + JMConstants.EXT_CSV;
		
		CSVFile = OpenFile(filepath, FileMode.WRITE);

		FPrintln( CSVFile, GetHeaderItemData());
		foreach(JMItemStatsBaseData itemData: m_Items)
		{
			FPrintln( CSVFile, GetExportedItemData(itemData));
		}

		CloseFile( CSVFile );
	}
}

enum JMItemStatsEnum
{
	CLOTHING = 0,
	FIREARM,
	EDIBLE,
	VEHICLE,
	MAGAZINE,
	OPTIC,
	ITEMS
}