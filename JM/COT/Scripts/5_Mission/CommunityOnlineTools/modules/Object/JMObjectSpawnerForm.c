class JMObjectSpawnerForm: JMFormBase
{
	private autoptr map< string, string > m_ObjectTypes;
	private string m_CurrentType;

	private Widget m_TypesActionsWrapper;
	private Widget m_SpawnerActionsWrapper;

	private UIActionSlider m_QuantityItem;
	private UIActionSlider m_HealthItem;
	private UIActionDropdownList m_ItemDataList;
	
	private UIActionEditableTextPreview m_SearchBox;
	private UIActionSelectBox m_SpawnMode;

	private TextListboxWidget m_ClassList;

	private ItemPreviewWidget m_ItemPreview;
	private EntityAI m_PreviewItem;
	private vector m_Orientation;
	private float m_Distance;

	private int m_MouseX;
	private int m_MouseY;

	private JMObjectSpawnerModule m_Module;

	private Object m_DeletingObject;

	private static int s_ObjSpawnMode = COT_ObjectSpawnerMode.CURSOR;
	private ref array< string > m_ObjSpawnModeText =
	{
		"#STR_COT_OBJECT_MODULE_EXPORT_RAW",
		"#STR_COT_OBJECT_MODULE_EXPORT_TYPES",
		"#STR_COT_OBJECT_MODULE_EXPORT_MARKET",

		"#STR_COT_OBJECT_MODULE_CURSOR",
		"#STR_COT_OBJECT_MODULE_SELF"
	};

	private ref array< string > m_ObjItemStateFoodText =
	{
		// Food
		"UNKNOWN",
		"RAW",
		"BAKED",
		"BOILED",
		"DRIED",
		"BURNED",
		"ROTTEN"
	};

	private ref array< string > m_ObjItemStateBloodText =
	{
		// Blood bags
		"UNKNOWN",
		"0+",
		"0-",
		"A+",
		"A-",
		"B+",
		"B-",
		"AB+",
		"AB-"
	};

	private ref array< string > m_ObjItemStateLiquidText =
	{
		// Liquid containers
		"UNKNOWN",
		"WATER",
		"RIVERWATER",
		"VODKA",
		"BEER",
		"GASOLINE",
		"DIESEL",
		"DISINFECTANT",
		"SOLUTION"
	};

	private int m_ItemStateType = -1;
	private int m_LiquidType;

	private static ref array< string > m_ItemsThatCrash =
	{
		"quickiebow",
		"recurvebow",
		"gp25base",
		"gp25",
		"gp25_standalone",
		"m203base",
		"m203",
		"m203_standalone",
		"red9",
		"pvcbow",
		"crossbow",
		"m249",
		"undersluggrenadem4",
		"groza",
		"pm73rak",
		"trumpet",
		"lawbase",
		"law",
		"rpg7base",
		"rpg7",
		"dartgun",
		"shockpistol",
		"shockpistol_black",
		"fnx45_arrow",
		"makarovpb",
		"mp133shotgun_pistolgrip",

		"largetentbackpack",
		"splint_applied",
		"leatherbelt_natural",
		"leatherbelt_beige",
		"leatherbelt_brown",
		"leatherbelt_black",
		"leatherknifeshealth",

		"itemoptics",
		"itemoptics_base",
		"fx"
	};

	private static ref array< string > m_RestrictiveBlacklistedClassnames =
	{
		"placing",
		"debug",
		"bldr_",
		"land_",
		"staticobj_",
		"proxy"
	};

	protected static bool m_IknowWhatIamDoing;

	void JMObjectSpawnerForm()
	{
		m_ObjectTypes = new map< string, string >;
	}

	void ~JMObjectSpawnerForm()
	{
		delete m_ObjectTypes;
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_ClassList = TextListboxWidget.Cast( layoutRoot.FindAnyWidget( "object_spawn_list" ) );

		m_TypesActionsWrapper = layoutRoot.FindAnyWidget( "object_types_actions_wrapper" );

		m_ItemPreview = ItemPreviewWidget.Cast( layoutRoot.FindAnyWidget( "object_preview" ) );

		Widget typesButtons = UIActionManager.CreateGridSpacer( m_TypesActionsWrapper, 8, 1 );
		AddObjectType( typesButtons, "#STR_COT_OBJECT_MODULE_FILTER_TYPE_ALL", "" );
		AddObjectType( typesButtons, "#STR_COT_OBJECT_MODULE_FILTER_TYPE_Food", "edible_base" );
		AddObjectType( typesButtons, "#STR_COT_OBJECT_MODULE_FILTER_TYPE_Vehicles", "transport" );
		AddObjectType( typesButtons, "#STR_COT_OBJECT_MODULE_FILTER_TYPE_Firearms", "weapon_base" );
		AddObjectType( typesButtons, "#STR_COT_OBJECT_MODULE_FILTER_TYPE_Clothing", "clothing_base" );
		AddObjectType( typesButtons, "#STR_COT_OBJECT_MODULE_FILTER_TYPE_Items", "inventory_base" );
		AddObjectType( typesButtons, "#STR_COT_OBJECT_MODULE_FILTER_TYPE_Buildings", "house" );
		AddObjectType( typesButtons, "#STR_COT_OBJECT_MODULE_FILTER_TYPE_AI", "dz_lightai" );

		Widget spawnactionswrapper = layoutRoot.FindAnyWidget( "object_spawn_actions_wrapper" );

		m_SpawnerActionsWrapper = UIActionManager.CreateGridSpacer( spawnactionswrapper, 4, 1 );

		Widget actions = UIActionManager.CreatePanel( m_SpawnerActionsWrapper, 0x00000000, 35 );

		m_SearchBox = UIActionManager.CreateEditableTextPreview( actions, "#STR_COT_OBJECT_MODULE_SEARCH", this, "SearchInput_OnChange" );
		m_SearchBox.SetWidth( 0.65 );

		UIActionButton button = UIActionManager.CreateButton( actions, "X", this, "SearchInput_OnClickReset" );
		button.SetWidth( 0.05 );
		button.SetPosition( 0.65 );

		m_ItemDataList = UIActionManager.CreateDropdownBox( actions, spawnactionswrapper, "State:", m_ObjItemStateLiquidText, this, "Click_ItemData" );
		m_ItemDataList.SetPosition( 0.70 );
		m_ItemDataList.SetWidth( 0.3 );

		Widget itemData = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 1, 2 );

		m_QuantityItem = UIActionManager.CreateSlider( itemData, "#STR_COT_OBJECT_MODULE_QUANTITY", 0, 100);
		m_QuantityItem.SetCurrent( 100 );

		m_HealthItem = UIActionManager.CreateSlider( itemData, "#STR_COT_OBJECT_MODULE_HEALTH", 0, 100, this, "Click_SetHealth");
		m_HealthItem.SetStepValue( 1 );
		m_HealthItem.SetCurrent( 100 );

		Widget spawnButtons = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 1, 3 );

		UIActionManager.CreateButton( spawnButtons, "#STR_COT_OBJECT_MODULE_SPAWN_ON", this, "Click_SpawnObject" );
		
		if ( GetGame().IsServer() )
			m_ObjSpawnModeText.Insert("#STR_COT_OBJECT_MODULE_INVENTORY");
		else
			m_ObjSpawnModeText.Insert("#STR_COT_OBJECT_MODULE_SELECTED_PLAYERS");

		m_SpawnMode = UIActionManager.CreateSelectionBox( spawnButtons, "", m_ObjSpawnModeText, this, "ChangeSpawnMode" );
		m_SpawnMode.SetSelectorWidth(1.0);
		m_SpawnMode.SetSelection(s_ObjSpawnMode - 1, false);
		UIActionManager.CreateButton( spawnButtons, "#STR_COT_OBJECT_MODULE_DELETE", this, "DeleteCursor" );

		Widget spawnOptions = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 1, 2 );

		UIActionManager.CreateCheckbox( spawnOptions, "#STR_COT_OBJECT_MODULE_ONDEBUGSPAWN", this, "Click_OnDebugSpawn", m_Module.m_OnDebugSpawn );
		UIActionManager.CreateCheckbox( spawnOptions, "#STR_COT_OBJECT_MODULE_SHOWUNSAFE", this, "Click_OnSafetyToogle", m_IknowWhatIamDoing );
		UIActionManager.CreatePanel( spawnOptions );

		if ( m_Module )
		{
			m_CurrentType = m_Module.m_CurrentType;
			m_SearchBox.SetText(m_Module.m_SearchText);
			UpdateList();
		}

		UpdateItemPreview();
	}

	void Click_ItemData( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
		
		UpdateQuantityItemColor();
		UpdatePreviewItemState();
	}

	void Click_SetHealth( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateHealthItemColor();
	}

	void UpdateItemStateType(int mode, int liquidType = 0)
	{
		m_ItemStateType = mode;
		m_LiquidType = liquidType;

		int idx = -1;

		switch (mode)
		{
			case 0: // Liquids
				m_ItemDataList.SetItems(m_ObjItemStateLiquidText);
				idx = FindEnumValue(COT_LiquidTypes, liquidType);
				break;
			case 1: // Blood
				m_ItemDataList.SetItems(m_ObjItemStateBloodText);
				idx = FindEnumValue(COT_BloodTypes, liquidType);
				break;
			case 2: // Food
				m_ItemDataList.SetItems(m_ObjItemStateFoodText);
				break:
		}

		// Reset to default since 0 is used for UNKNOWN
		if (idx == -1)
			idx = 1;

		m_ItemDataList.SetSelection(idx, false);
		
		UpdateQuantityItemColor();
		UpdatePreviewItemState();
	}

	int FindEnumValue(typename e, int enumValue)
	{
		int cnt = e.GetVariableCount();
		int val;

		for (int i = 0; i < cnt; i++)
		{
			if (e.GetVariableType(i) == int && e.GetVariableValue(null, i, val) && val == enumValue)
			{
				return i;
			}
		}

		return -1;
	}

	void UpdatePreviewItemState()
	{
		Edible_Base edible;
		if (Class.CastTo(edible, m_PreviewItem) && edible.HasFoodStage())
			edible.GetFoodStage().ChangeFoodStage(m_ItemDataList.GetSelection());
	}

	void UpdateQuantityItemColor()
	{
		switch (m_LiquidType)
		{
			case COT_LiquidTypes.WATER:
				m_QuantityItem.SetColor( ARGB( 255, 173, 216, 230 ) );
				break;
			case COT_LiquidTypes.RIVERWATER:
				m_QuantityItem.SetColor( ARGB( 255, 0, 128, 128 ) );
				break;
			case COT_LiquidTypes.BEER:
				m_QuantityItem.SetColor( ARGB( 255, 255, 215, 0 ) );
				break;
			case COT_LiquidTypes.GASOLINE:
				m_QuantityItem.SetColor( ARGB( 255, 165, 123, 63 ) );
				break;
			case COT_LiquidTypes.DIESEL:
				m_QuantityItem.SetColor( ARGB( 255, 0, 100, 0 ) );
				break;
			case COT_LiquidTypes.DISINFECTANT:
			case COT_LiquidTypes.SOLUTION:
				m_QuantityItem.SetColor( ARGB( 255, 173, 216, 230 ) );
				break;
			case COT_BloodTypes.UNKNOWN:
				m_QuantityItem.SetColor( ARGB( 255, 139, 0, 0 ) );
				break;
			case COT_LiquidTypes.UNKNOWN:
			case COT_LiquidTypes.VODKA:
			default:
				m_QuantityItem.SetColor( ARGB( 255, 255, 255, 255 ) );
				break;
		}

		m_QuantityItem.SetAlpha( 1.0 );
	}

	void UpdateHealthItemColor()
	{
		float percent = m_HealthItem.GetCurrent();
		
		if ( percent != 0)
			percent = percent / m_HealthItem.GetMax();

		if ( percent >= 0.69999999 )
		{
			m_HealthItem.SetColor( Colors.COLOR_PRISTINE );
		}
		else if ( percent >= 0.5 )
		{
			m_HealthItem.SetColor( Colors.COLOR_WORN );
		}
		else if ( percent >= 0.30000001 )
		{
			m_HealthItem.SetColor( Colors.COLOR_DAMAGED );
		}
		else if ( percent > 0.001 )
		{
			m_HealthItem.SetColor( Colors.COLOR_BADLY_DAMAGED );
		}
		else if ( percent == -1 )
		{
			m_HealthItem.SetColor( ARGB( 255, 220, 220, 220 ) );			
		}
		else
		{
			m_HealthItem.SetColor( Colors.COLOR_RUINED );
		}

		m_HealthItem.SetAlpha( 1.0 );
	}

	void Click_OnSafetyToogle( UIEvent eid, UIActionBase action )	
	{
		if ( eid != UIEvent.CLICK ) return;

		m_IknowWhatIamDoing = action.IsChecked();
		UpdateList();
	}

	void Click_OnDebugSpawn( UIEvent eid, UIActionBase action )	
	{
		if ( eid != UIEvent.CLICK ) return;

		m_Module.m_OnDebugSpawn = action.IsChecked();	
	}

	void Click_SpawnObject( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;

		SpawnObject(s_ObjSpawnMode);
	}

	void AddObjectType( Widget parent, string name, string config )
	{
		name = Widget.TranslateString( name );
		UIActionManager.CreateButton( parent, name, this, "SetListType" );
		m_ObjectTypes.Insert( name, config );
	}

	void UpdateRotation( int mouse_x, int mouse_y, bool is_dragging )
	{
		m_Orientation[0] = m_Orientation[0] + ( ( m_MouseY - mouse_y ) * 0.01 );
		m_Orientation[1] = m_Orientation[1] - ( ( m_MouseX - mouse_x ) * 0.01 );
			
		m_ItemPreview.SetModelOrientation( m_Orientation );
	}

	void UpdateDistance( float drag )
	{
		m_Distance = m_Distance + (drag * 0.1);
		
		m_ItemPreview.SetModelPosition( Vector( m_Distance, 0, 0.5 + m_Distance ) );
	}

	void UpdateItemPreview()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::UpdateItemPreview" );
		#endif
		
		string strSelection = GetCurrentSelection();

		if ( m_PreviewItem ) 
		{
			GetGame().ObjectDelete( m_PreviewItem );
		}

		if ( GetGame().IsKindOf( strSelection, "DZ_LightAI" ) ) 
		{
			m_ItemPreview.Show( false );

			#ifdef COT_DEBUGLOGS
			Print( "-" + this + "::UpdateItemPreview AI" );
			#endif
			return;
		}

		m_Orientation = vector.Zero;

		m_PreviewItem = EntityAI.Cast( GetGame().CreateObject( strSelection, vector.Zero, true, false ) );

		m_QuantityItem.Disable();
		m_HealthItem.Disable();
		if ( m_PreviewItem )
		{
			m_ItemPreview.SetItem( m_PreviewItem );
			m_ItemPreview.SetModelPosition( Vector( m_Distance, 0, 0.5 + m_Distance ) );
			m_ItemPreview.SetModelOrientation( vector.Zero );
			m_ItemPreview.SetView( m_ItemPreview.GetItem().GetViewIndex() );
			m_ItemPreview.Show( true );

			if (!m_PreviewItem.IsInherited(Building) && !m_PreviewItem.IsInherited(AdvancedCommunication))
			{
				m_HealthItem.Enable();
				m_HealthItem.SetMax(m_PreviewItem.GetMaxHealth());
				if ( m_HealthItem.GetCurrent() == -1 )
					m_HealthItem.SetCurrent(m_PreviewItem.GetMaxHealth());

				m_HealthItem.SetMin(0);
			}

			UpdateHealthItemColor();			

			if (m_PreviewItem.IsInherited(ItemBase)) 
			{
				ItemBase item = ItemBase.Cast(m_PreviewItem);

				m_ItemDataList.Enable();
				if ( item.IsLiquidContainer() )
				{
					int itemStateType;
					if ( item.IsBloodContainer() )
						itemStateType = 1;
					int liquidType = item.GetLiquidTypeInit();
					if ( m_ItemStateType != itemStateType || m_LiquidType != liquidType )
						UpdateItemStateType(itemStateType, liquidType);
				}
				else
				{
					if ( item.HasFoodStage() && item.CanBeCooked() )
					{
						if ( m_ItemStateType != 2 )
							UpdateItemStateType(2);
					}
					else
					{
						m_ItemStateType = -1;
						m_LiquidType = 0;
						m_ItemDataList.SetItems({""});
						UpdateQuantityItemColor();
						m_ItemDataList.Disable();
					}
				}

				if ( item.HasQuantity() )
				{
					m_QuantityItem.SetMin(item.GetQuantityMin());
					m_QuantityItem.SetMax(item.GetQuantityMax());

					if ( item.IsLiquidContainer() )
					{
						m_QuantityItem.SetStepValue(0.1);
					}
					else
					{
						Magazine mag;
						if (Class.CastTo(mag, item))
						{
							if ( mag.IsAmmoPile() )
								m_QuantityItem.SetMin(1);
							else
								m_QuantityItem.SetMin(0);
							m_QuantityItem.SetMax(mag.GetAmmoMax());
						}

						m_QuantityItem.SetStepValue(1);
						int newCurrent = m_QuantityItem.GetCurrent();
						m_QuantityItem.SetCurrent(newCurrent);
					}
					m_QuantityItem.Enable();
				}
			}
		}
		else
		{
			m_HealthItem.SetMin(-1);
			m_HealthItem.SetCurrent(-1);
			m_HealthItem.SetMax(-1);
			m_ItemPreview.Show( false );
		}

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::UpdateItemPreview" );
		#endif
	}

	override bool OnItemSelected( Widget w, int x, int y, int row, int column, int oldRow, int oldColumn )
	{
		if ( w == m_ClassList ) 
		{
			UpdateItemPreview();

			return true;
		}

		return false;
	}

	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		super.OnMouseButtonDown( w, x, y, button );

		if ( w == m_ItemPreview && button == MouseState.LEFT )
		{
			GetGame().GetDragQueue().Call( this, "UpdateRotation" );
			GetGame().GetMousePos( m_MouseX, m_MouseY );

			return true;
		}

		return false;
	}

	override bool OnMouseWheel(Widget  w, int  x, int  y, int wheel)
	{
		if ( w == m_ItemPreview )
		{
			UpdateDistance(wheel);

			return true;
		}

		return super.OnMouseWheel( w, x, y, wheel );
	}

	override bool OnDoubleClick( Widget w, int x, int y, int button )
	{
		super.OnDoubleClick( w, x, y, button );

		if ( w == m_ClassList && button == MouseState.LEFT )
		{
			SpawnObject(s_ObjSpawnMode);
			
			return true;
		}

		return false;
	}

	override void OnShow()
	{
		super.OnShow();

		UpdateList();
		UpdateItemPreview();
	}

	override void OnHide() 
	{
		m_Module.m_CurrentType = m_CurrentType;
		if (m_SearchBox)
			m_Module.m_SearchText = m_SearchBox.GetText();

		super.OnHide();
	}

	void SetListType( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;

		m_CurrentType = m_ObjectTypes.Get( action.GetButton() );

		UpdateList();
	}

	void ChangeSpawnMode( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		s_ObjSpawnMode = action.GetSelection() + 1;
	}

	void SpawnObject(int mode = COT_ObjectSpawnerMode.NONE)
	{
		if (mode == COT_ObjectSpawnerMode.NONE || mode == COT_ObjectSpawnerMode.UNKNOWN)
			return;

		string clipboardOutput = "";
		string result;

		int itemState = 0; // 0 mean don't do anything
		if ( m_ItemStateType != -1 )
			itemState = m_ItemDataList.GetSelection();

		float health = m_HealthItem.GetCurrent();
		float quantity = m_QuantityItem.GetCurrent();

		switch (mode)
		{
			default:
			case COT_ObjectSpawnerMode.POSITION:
				m_Module.SpawnEntity_Position(GetCurrentSelection(), GetGame().GetPlayer().GetPosition(), quantity, health, itemState);
				break;

			case COT_ObjectSpawnerMode.CURSOR:
				m_Module.SpawnEntity_Position(GetCurrentSelection(), GetCursorPos(), quantity, health, itemState);
				break;

			case COT_ObjectSpawnerMode.INVENTORY:
				m_Module.SpawnEntity_Inventory(GetCurrentSelection(), JM_GetSelected().GetPlayers(), quantity, health, itemState);
				break;

			case COT_ObjectSpawnerMode.COPYLISTRAW:
				for (int i = 0; i < m_ClassList.GetNumItems(); i++)
				{
					m_ClassList.GetItemText(i, 0, result);
					clipboardOutput += result + "\n";
				}
				GetGame().CopyToClipboard(clipboardOutput);
				break;

			case COT_ObjectSpawnerMode.COPYLISTTYPES:
				clipboardOutput = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
				clipboardOutput += "<types>\n";
				for (int j = 0; j < m_ClassList.GetNumItems(); j++)
				{
					m_ClassList.GetItemText(j, 0, result);
					clipboardOutput += "	<type name=\"" + result + "\">\n";
					clipboardOutput += "		<nominal>0</nominal>\n";
					clipboardOutput += "		<lifetime>3888000</lifetime>\n";
					clipboardOutput += "		<restock>0</restock>\n";
					clipboardOutput += "		<min>0</min>\n";
					clipboardOutput += "		<quantmin>-1</quantmin>\n";
					clipboardOutput += "		<quantmax>-1</quantmax>\n";
					clipboardOutput += "		<cost>100</cost>\n";
					clipboardOutput += "		<flags count_in_cargo=\"0\" count_in_hoarder=\"0\" count_in_map=\"1\" count_in_player=\"0\" crafted=\"0\" deloot=\"0\"/>\n";
					clipboardOutput += "	</type>\n";
				}
				clipboardOutput += "</types>";
				GetGame().CopyToClipboard(clipboardOutput);
				break;

			case COT_ObjectSpawnerMode.COPYLISTEXPMARKET:
				clipboardOutput = "{\n";
				clipboardOutput += "    \"m_Version\": 12,\n";
				clipboardOutput += "    \"DisplayName\": \"" + m_SearchBox.GetText() + "\",\n";
				clipboardOutput += "    \"Icon\": \"Deliver\",\n";
				clipboardOutput += "    \"Color\": \"FBFCFEFF\",\n";
				clipboardOutput += "    \"IsExchange\": 0,\n";
				clipboardOutput += "    \"InitStockPercent\": 75.0,\n";
				clipboardOutput += "    \"Items\": [\n";
				for (int k = 0; k < m_ClassList.GetNumItems(); k++)
				{
					m_ClassList.GetItemText(k, 0, result);
					clipboardOutput += "        {\n";
					clipboardOutput += "            \"ClassName\": \"" + result + "\",\n";
					clipboardOutput += "            \"MaxPriceThreshold\": 100,\n";
					clipboardOutput += "            \"MinPriceThreshold\": 100,\n";
					clipboardOutput += "            \"SellPricePercent\": -1.0,\n";
					clipboardOutput += "            \"MaxStockThreshold\": 1,\n";
					clipboardOutput += "            \"MinStockThreshold\": 1,\n";
					clipboardOutput += "            \"QuantityPercent\": -1,\n";
					clipboardOutput += "            \"SpawnAttachments\": [],\n";
					clipboardOutput += "            \"Variants\": []\n";

					if (k + 1 < m_ClassList.GetNumItems())
						clipboardOutput += "        },\n";
					else
						clipboardOutput += "        }\n";
				}
				clipboardOutput += "    ]\n";
				clipboardOutput += "}";
				GetGame().CopyToClipboard(clipboardOutput);
				break;
		}
	}

	void DeleteCursor( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_DeletingObject = NULL;

		float distance = 2.0;
		vector rayStart = GetGame().GetCurrentCameraPosition();
		vector rayEnd = rayStart + ( GetGame().GetCurrentCameraDirection() * distance );

		RaycastRVParams rayInput = new RaycastRVParams( rayStart, rayEnd, GetGame().GetPlayer() );
		rayInput.flags = CollisionFlags.NEARESTCONTACT;
		rayInput.radius = 1.0;
		array< ref RaycastRVResult > results = new array< ref RaycastRVResult >;

		Object obj;
		if ( DayZPhysics.RaycastRVProxy( rayInput, results ) )
		{
			for ( int i = 0; i < results.Count(); ++i )
			{
				if ( results[i].obj == NULL || PlayerBase.Cast( results[i].obj ) )
				{
					continue;
				}

				if ( results[i].obj.GetType() == "" )
				{
					continue;
				}

				if ( results[i].obj.GetType() == "#particlesourceenf" )
				{
					continue;
				}

				obj = results[i].obj;
				break;
			}
		}

		if ( obj == NULL )
		{
			return;
		}

		m_DeletingObject = obj;
		
		CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_GENERIC_CONFIRM", string.Format( Widget.TranslateString( "#STR_COT_OBJECT_MODULE_DELETE_CONFIRMATION_BODY" ), Object.GetDebugName( obj ) ), "#STR_COT_GENERIC_NO", "DeleteEntity_No", "#STR_COT_GENERIC_YES", "DeleteEntity_Yes" );
	}

	private void DeleteEntity_Yes( JMConfirmation confirmation )
	{
		m_Module.DeleteEntity( m_DeletingObject );

		m_DeletingObject = NULL;
	}

	private void DeleteEntity_No( JMConfirmation confirmation )
	{
		
	}

	void SearchInput_OnClickReset( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_SearchBox.SetText("");
		UpdateList();
	}

	void SearchInput_OnChange( UIEvent eid, UIActionBase action )
	{		
		UpdateList();
	}

	void UpdateList()
	{
		m_ClassList.ClearItems();
		TStringArray classnamelist = new TStringArray;

		string strSearch = m_SearchBox.GetText();
		
		if ( strSearch != "" )
		{
			TStringArray configs = new TStringArray;
			configs.Insert( CFG_VEHICLESPATH );
			configs.Insert( CFG_WEAPONSPATH );
			configs.Insert( CFG_MAGAZINESPATH );
			configs.Insert( CFG_NONAI_VEHICLES );

			strSearch.ToLower();

			for ( int nConfig = 0; nConfig < configs.Count(); nConfig++ )
			{
				string strConfigPath = configs.Get( nConfig );

				int nClasses = g_Game.ConfigGetChildrenCount( strConfigPath );

				int nClassStart = 0;
				if (nConfig == 0) nClassStart = 20;

				for ( int nClass = nClassStart; nClass < nClasses; nClass++ )
				{
					string strName;

					GetGame().ConfigGetChildName( strConfigPath, nClass, strName );

					int scope = GetGame().ConfigGetInt( strConfigPath + " " + strName + " scope" );

					if ( scope == 0 )
						continue;

					if ( scope == 1 && !m_IknowWhatIamDoing )
						continue;

					if ( !GetGame().ConfigIsExisting( strConfigPath + " " + strName + " model" ) )
						continue;

					string strNameLower = strName;

					strNameLower.ToLower();

					if ( m_CurrentType == "" || GetGame().IsKindOf( strNameLower, m_CurrentType ) )
					{
						if ( CheckItemCrash( strNameLower ) ) 
							continue; 

						if ( !strNameLower.Contains( strSearch ) )
							continue;

						m_ClassList.AddItem( strName, NULL, 0 );
						classnamelist.Insert(strNameLower);
					}
				}
			}
		}

		if (classnamelist.Count())
		{
			m_SearchBox.SetTextPreview(FindClosestWord(classnamelist, strSearch));
		}
		else
		{
			m_SearchBox.SetTextPreview("");
		}
	}

    static string FindClosestWord(TStringArray words, string inputWord)
    {
        TStringArray suggestions = new TStringArray;

        foreach (string word : words)
        {
            if ( word == inputWord )
                return word;

            if ( word.IndexOf(inputWord) == 0 )
                suggestions.Insert(word);
        }

		suggestions.Sort();

        return suggestions[0];
    }

	private bool CheckItemCrash( string name )
	{
		if ( m_ItemsThatCrash.Find( name ) > -1 )
		{
			return true;
		}

		if ( !m_IknowWhatIamDoing )
		{
			for ( int i = 0; i < m_RestrictiveBlacklistedClassnames.Count(); ++i )
			{
				if ( name.Contains( m_RestrictiveBlacklistedClassnames[i] ) )
				{
					return true;
				}
			}
		}

		return false;
	}

	string GetCurrentSelection()
	{
		if ( m_ClassList.GetSelectedRow() != -1 )
		{
			string result;
			m_ClassList.GetItemText( m_ClassList.GetSelectedRow(), 0, result );
			return result;
		}

		return "";
	}
};
