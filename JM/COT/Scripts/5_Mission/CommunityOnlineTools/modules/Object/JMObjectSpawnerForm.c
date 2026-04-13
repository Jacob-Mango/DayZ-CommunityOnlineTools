class JMObjectSpawnerForm: JMFormBase
{
	private autoptr map< string, string > m_ObjectTypes;

	private Widget m_TypesActionsWrapper;
	private Widget m_SpawnerActionsWrapper;

	private UIActionSlider m_QuantityItem;
	private UIActionSlider m_TemperatureItem;
	private UIActionSlider m_HealthItem;
	private UIActionDropdownList m_ItemDataList;
	
	private UIActionEditableTextPreview m_SearchBox;
	private UIActionSelectBox m_SpawnMode;

	private UIActionSelectBox m_ObjSetupMode;

	private TextListboxWidget m_ClassList;

	private ItemPreviewWidget m_ItemPreview;
	private EntityAI m_PreviewItem;
	private vector m_Orientation;
	private float m_Distance;

	private int m_MouseX;
	private int m_MouseY;

	private JMObjectSpawnerModule m_Module;

	private Object m_DeletingObject;

	private UIActionButton m_SpawnButton;
	private UIActionButton m_AttachmentsButton;
	private static int s_ObjSpawnMode = COT_ObjectSpawnerMode.CURSOR;
	private ref array< string > m_ObjSpawnModeText =
	{
		"#STR_COT_OBJECT_MODULE_EXPORT_RAW",
		"#STR_COT_OBJECT_MODULE_EXPORT_TYPES",
	#ifdef DZ_Expansion_Market
		"#STR_COT_OBJECT_MODULE_EXPORT_MARKET",
	#endif

		"#STR_COT_OBJECT_MODULE_CROSSHAIR",
		"#STR_COT_OBJECT_MODULE_SELF",
		"#STR_COT_OBJECT_MODULE_TARGET"
	};

	private ref array< string > m_ObjItemStateFoodText = {};

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

	private ref array< int > m_ObjItemStateLiquid = {};
	private ref array< string > m_ObjItemStateLiquidText = {};
	private ref map<int, int> m_ObjItemStateLiquidColors = new map<int, int>;

	private int m_ItemStateType = -1;
	private int m_LiquidType;

	void JMObjectSpawnerForm()
	{
		m_ObjectTypes = new map< string, string >;
	}

	void ~JMObjectSpawnerForm()
	{
		if (m_PreviewItem)
			g_Game.ObjectDelete(m_PreviewItem);
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
		AddObjectType( typesButtons, "#STR_COT_OBJECT_MODULE_FILTER_TYPE_Ammo_Magazines", "magazine_base" );
		AddObjectType( typesButtons, "#STR_COT_OBJECT_MODULE_FILTER_TYPE_Clothing", "clothing_base" );
		AddObjectType( typesButtons, "#STR_COT_OBJECT_MODULE_FILTER_TYPE_Containers", "container_base" );
		AddObjectType( typesButtons, "#STR_COT_OBJECT_MODULE_FILTER_TYPE_Items", "inventory_base" );
		AddObjectType( typesButtons, "#STR_COT_OBJECT_MODULE_FILTER_TYPE_Buildings", "house" );
		AddObjectType( typesButtons, "#STR_COT_OBJECT_MODULE_FILTER_TYPE_AI", "dz_lightai" );

		Widget spawnactionswrapper = layoutRoot.FindAnyWidget( "object_spawn_actions_wrapper" );

		m_SpawnerActionsWrapper = UIActionManager.CreateGridSpacer( spawnactionswrapper, 4, 1 );

		Widget actions = UIActionManager.CreatePanel( m_SpawnerActionsWrapper, 0x00000000, 35 );

		Widget searchSpacer = UIActionManager.CreateWrapSpacerCompact( actions, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_SearchBox = UIActionManager.CreateEditableTextPreview( searchSpacer, "#STR_COT_OBJECT_MODULE_SEARCH", this, "SearchInput_OnChange" );
		m_SearchBox.SetWidth( 0.65 );

		UIActionImageButton button = UIActionManager.CreateImageButton( searchSpacer, "set:dayz_gui image:icon_x", this, "SearchInput_OnClickReset" );
		button.SetFixedSize( 28, 28 );

		int foodStageCount = FoodStageType.COUNT;
		for (int foodStage = 0; foodStage < FoodStageType.COUNT; foodStage++)
		{
			m_ObjItemStateFoodText.Insert(typename.EnumToString(FoodStageType, foodStage));
		}

		m_ObjItemStateLiquidColors[0] = COLOR_WHITE;

		string displayName;
		string translated;
		int color;
		foreach (int liquidType, LiquidInfo liquidInfo: Liquid.m_LiquidInfosByType)
		{
			NutritionalProfile nutritionProfile = liquidInfo.m_NutriProfile;

			string liquidClsName = nutritionProfile.GetLiquidClassname();
			string underscored = JMStatics.CamelCaseToWords(liquidClsName, "_");

			//! Liquids (except blood)
			if (nutritionProfile.IsLiquid() && liquidType > 255)
			{
				//!@note most of this joinked from ExpansionWorld::GetLiquidDisplayName

				g_Game.ConfigGetTextRaw("CfgLiquidDefinitions " + liquidClsName +  " displayName", displayName);
				g_Game.FormatRawConfigStringKeys(displayName);

				if (displayName.IndexOf("#") == 0)
					translated = Widget.TranslateString(displayName);
				else
					translated = displayName;

				//! Fix up vanilla liquid display name
				if (displayName.IndexOf("#STR_cfgLiquidDefinitions_") == 0 && translated.IndexOf("$UNT$") == 0)
					translated = liquidClsName;

				int idx = 0;
				foreach (string liquidText: m_ObjItemStateLiquidText)
				{
					if (JMStatics.StrCaseCmp(translated, liquidText) < 0)
						break;
					idx++;
				}

				m_ObjItemStateLiquid.InsertAt(liquidType, idx);
				m_ObjItemStateLiquidText.InsertAt(translated, idx);
			}

			//! Liquids (including blood)
			if (nutritionProfile.IsLiquid())
			{
				string colorPath = "CfgLiquidDefinitions " + liquidClsName +  " color";

				color = g_Game.ConfigGetInt(colorPath);

				if (!color)
				{
					string colorConstantName;
					g_Game.ConfigGetTextRaw(colorPath, colorConstantName);

					if (!colorConstantName)
					{
						//! Fallback to liquid classname, all uppercase
						colorConstantName = liquidClsName;
						colorConstantName.ToUpper();
					}

					bool found = JMStatics.StringToEnumEx(Colors, colorConstantName, color);

					if (!found)
					{
						if (!colorConstantName.Contains("LIQUID"))
						{
							colorConstantName += "LIQUID";  //! e.g. RaG_Liquid_Framework
							found = JMStatics.StringToEnumEx(Colors, colorConstantName, color);
						}

						if (!found)
						{
							//! Fallback to liquid classname, all uppercase, words delimited by underscore
							colorConstantName = underscored;
							colorConstantName.ToUpper();

							if (!JMStatics.StringToEnumEx(Colors, colorConstantName, color))
							{
								switch (liquidType)
								{
									case LIQUID_BEER:
										color = Colors.ORANGE;
										break;
									case LIQUID_DIESEL:
									case LIQUID_GASOLINE:
										color = Colors.YELLOW;
										break;
									case LIQUID_DISINFECTANT:
									case LIQUID_VODKA:
										color = Colors.GRAY;
										break;
									default:
										if (liquidType > 255)
											color = Colors.COLOR_LIQUID;
										else
											//! Blood
											color = Colors.RED;
										break;
								}
							}
						}
					}
				}

				m_ObjItemStateLiquidColors[liquidType] = color;
			}
		}
		
	#ifdef DIAG
		for (int k = 0; k < m_ObjItemStateLiquidText.Count(); k++)
		{
			TIntArray argb = {};

			for (int i = 0; i < 4; i++)
			{
				argb.Insert((m_ObjItemStateLiquidColors[m_ObjItemStateLiquid[k]] >> (24 - i * 8)) & 255);
			}

			m_ObjItemStateLiquidColors[m_ObjItemStateLiquid[k]];
			PrintFormat("LIQUID %1 displayName='%2' type=%3 className='%4' color={%5, %6, %7, %8}", k, m_ObjItemStateLiquidText[k], m_ObjItemStateLiquid[k], Liquid.GetNutritionalProfileByType(m_ObjItemStateLiquid[k]).GetLiquidClassname(), argb[0], argb[1], argb[2], argb[3]);
		}
	#endif

		m_ItemDataList = UIActionManager.CreateDropdownBox( actions, spawnactionswrapper, "State:", {""}, this, "Click_ItemData" );
		m_ItemDataList.SetPosition( 0.70 );
		m_ItemDataList.SetWidth( 0.3 );
		m_ItemDataList.Disable();

		Widget itemData = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 2, 2 );

		m_QuantityItem = UIActionManager.CreateSlider( itemData, "#STR_COT_OBJECT_MODULE_QUANTITY", 0, 100);
		m_QuantityItem.SetCurrent( 100 );

		m_HealthItem = UIActionManager.CreateSlider( itemData, "#STR_COT_OBJECT_MODULE_HEALTH", 0, 100, this, "Click_SetHealth");
		m_HealthItem.SetStepValue( 1 );
		m_HealthItem.SetCurrent( 100 );

		m_TemperatureItem = UIActionManager.CreateSlider( itemData, "#STR_COT_OBJECT_MODULE_TEMPERATURE", GameConstants.STATE_COLD_LVL_FOUR, GameConstants.STATE_HOT_LVL_FOUR, this, "Click_SetTemperature");
		m_TemperatureItem.SetSliderWidth(0.6);
		m_TemperatureItem.SetStepValue( 1 );
		m_TemperatureItem.SetFormat( "#STR_COT_FORMAT_DEGREE" );
		m_TemperatureItem.SetCurrent( GameConstants.STATE_NEUTRAL_TEMP );

		UIActionManager.CreateCheckbox( itemData, "#STR_COT_OBJECT_MODULE_SPAWN_DISPLAYNAME", this, "Click_OnFilterDisplayName", m_Module.m_FilterWithDisplayName );

		Widget spawnButtons = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 1, 3 );

		m_SpawnButton = UIActionManager.CreateButton( spawnButtons, "#STR_COT_OBJECT_MODULE_SPAWN_ON", this, "Click_SpawnObject" );
		
		if ( !g_Game.IsMultiplayer() )
			m_ObjSpawnModeText.Insert("#STR_COT_OBJECT_MODULE_INVENTORY");
		else
			m_ObjSpawnModeText.Insert("#STR_COT_OBJECT_MODULE_SELECTED_PLAYERS");

		m_ObjSpawnModeText.Insert("#STR_COT_OBJECT_MODULE_SELECTED_OBJECTS");

		m_SpawnMode = UIActionManager.CreateSelectionBox( spawnButtons, "", m_ObjSpawnModeText, this, "ChangeSpawnMode" );
		m_SpawnMode.SetSelectorWidth(1.0);
		m_SpawnMode.SetSelection(s_ObjSpawnMode, false);

		UIActionButton delbtn = UIActionManager.CreateButton( spawnButtons, "Delete", this, "DeleteCursor" );
		delbtn.SetColor(COLOR_RED);

		Widget spawnOptions = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 1, 3 );

		m_AttachmentsButton = UIActionManager.CreateButton(spawnOptions, "#STR_COT_OBJECT_MODULE_SPAWN_WITH", this, "Click_SpawnObject");
		m_ObjSetupMode =  UIActionManager.CreateSelectionBox( spawnOptions, "", {"#STR_COT_OBJECT_MODULE_SPAWN_WITH_DEBUG", "#STR_COT_OBJECT_MODULE_SPAWN_WITH_CE", "#STR_COT_GENERIC_NONE"}, this, "Click_ObjSetupMode" );
		m_ObjSetupMode.SetSelectorWidth(1.0);
		m_ObjSetupMode.SetSelection(m_Module.m_ObjSetupMode, false);

		UIActionManager.CreateCheckbox( spawnOptions, "#STR_COT_OBJECT_MODULE_SHOWUNSAFE", this, "Click_OnSafetyToogle", m_Module.m_AllowRestrictedClassNames );

		m_SearchBox.SetText(m_Module.m_SearchText);
		UpdateList();

		UpdateItemPreview();
	}

	void Click_ItemData( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
		
		if (m_ItemStateType == 0)
			m_LiquidType = m_ObjItemStateLiquid[m_ItemDataList.GetSelection()];
		
		UpdateQuantityItemColor();
		UpdatePreviewItemState();
	}

	void Click_SetHealth( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateHealthItemColor();
		UpdateItemPreview();
	}

	void Click_SetTemperature( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateTemperatureItemColor();
		UpdateItemPreview();
	}

	void UpdateItemStateType(int mode, int liquidType = 0)
	{
		m_ItemStateType = mode;

		int idx = -1;

		switch (mode)
		{
			case 0: // Liquids
				m_ItemDataList.SetItems(m_ObjItemStateLiquidText);
				idx = m_ObjItemStateLiquid.Find(m_LiquidType);
				if (idx == -1)
				{
					idx = m_ObjItemStateLiquid.Find(liquidType);  //! Fallback
					m_LiquidType = liquidType;
				}
				break;
			case 1: // Blood
				m_ItemDataList.SetItems(m_ObjItemStateBloodText);
				idx = FindEnumValue(COT_BloodTypes, m_LiquidType);
				if (idx == -1)
				{
					idx = FindEnumValue(COT_BloodTypes, liquidType);  //! Fallback
					m_LiquidType = liquidType;
				}
				break;
			case 2: // Food
				m_ItemDataList.SetItems(m_ObjItemStateFoodText);
				break;
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
				return i;
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
		int color;

		if (!m_ObjItemStateLiquidColors.Find(m_LiquidType, color))
			color = COLOR_WHITE;

		m_QuantityItem.SetColor( color );
		m_QuantityItem.SetAlpha( 1.0 );
	}

	void UpdateHealthControls(string type)
	{
		float maxHealth = MiscGameplayFunctions.GetTypeMaxGlobalHealth(type);
		if (maxHealth > 0)
		{
			m_HealthItem.Enable();
			m_HealthItem.SetMinMax(0, maxHealth);
		}
		else
		{
			m_HealthItem.Disable();
		}
	}

	void UpdateHealthItemColor()
	{
		if (!m_HealthItem.IsEnabled())
		{
			m_HealthItem.SetColor( ARGB( 255, 220, 220, 220 ) );
			return;
		}

		float health = m_HealthItem.GetCurrent();
		float health01;
		float sliderMax = m_HealthItem.GetMax();

		if (sliderMax > 0)
			health01 = health / sliderMax;

		if ( health01 >= 0.7 )			m_HealthItem.SetColor( Colors.COLOR_PRISTINE );
		else if ( health01 >= 0.5 )		m_HealthItem.SetColor( Colors.COLOR_WORN );
		else if ( health01 >= 0.3 )		m_HealthItem.SetColor( Colors.COLOR_DAMAGED );
		else if ( health01 > 0 )		m_HealthItem.SetColor( Colors.COLOR_BADLY_DAMAGED );
		else							m_HealthItem.SetColor( Colors.COLOR_RUINED );

		m_HealthItem.SetAlpha( 1.0 );
	}
	

	void UpdateTemperatureItemColor()
	{
		int value = m_TemperatureItem.GetCurrent();

		m_TemperatureItem.SetColor( ObjectTemperatureState.GetStateData(value).m_Color );
		if (ObjectTemperatureState.GetStateData(value).m_State != GameConstants.STATE_NEUTRAL_TEMP)
			m_TemperatureItem.SetFormat( "#STR_COT_FORMAT_DEGREE " + ObjectTemperatureState.GetStateData(value).m_LocalizedName );
		else
			m_TemperatureItem.SetFormat("#STR_COT_FORMAT_DEGREE");

		m_TemperatureItem.SetAlpha( 1.0 );
	}

	void Click_OnSafetyToogle( UIEvent eid, UIActionBase action )	
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.m_AllowRestrictedClassNames = action.IsChecked();
		UpdateList();
	}

	void Click_OnFilterDisplayName( UIEvent eid, UIActionBase action )	
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.m_FilterWithDisplayName = action.IsChecked();
		UpdateList();
	}

	void Click_ObjSetupMode( UIEvent eid, UIActionBase action )	
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Module.m_ObjSetupMode = action.GetSelection();
	}

	void Click_SpawnObject( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

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

	void UpdateDistance( float wheel )
	{
		vector minMax[2];
		float radius = m_PreviewItem.ClippingInfo(minMax);

		m_Distance = m_Distance - (wheel * radius / 10.0);
		
		m_ItemPreview.SetModelPosition( Vector( m_Distance, 0, 0.5 + m_Distance ) );
	}

	void UpdateItemPreview()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::UpdateItemPreview" );
		#endif
		
		string strSelection = GetCurrentSelection();

		if (m_PreviewItem && m_PreviewItem.GetType() != strSelection) 
		{
			g_Game.ObjectDelete( m_PreviewItem );
			m_PreviewItem = null;
		}

		if (!m_PreviewItem)
		{
			m_PreviewItem = EntityAI.Cast( g_Game.CreateObject( strSelection, vector.Zero, true, false, false ) );

			if (m_PreviewItem)
			{
				dBodyActive(m_PreviewItem, ActiveState.INACTIVE);
				dBodyDynamic(m_PreviewItem, false);
				m_PreviewItem.DisableSimulation(true);
				m_ItemPreview.SetItem( m_PreviewItem );

				m_Distance = 0;
				m_Orientation = vector.Zero;
				m_ItemPreview.SetModelPosition( Vector( m_Distance, 0, 0.5 + m_Distance ) );
				m_ItemPreview.SetModelOrientation( vector.Zero );
				m_ItemPreview.SetView( m_ItemPreview.GetItem().GetViewIndex() );
				m_ItemPreview.Show( true );
			}
		}

		m_QuantityItem.Disable();
		UpdateHealthControls(strSelection);
		UpdateHealthItemColor();
		m_TemperatureItem.Disable();

		int itemStateType = m_ItemStateType;

		if ( m_PreviewItem )
		{
			if (m_HealthItem.IsEnabled() && !m_PreviewItem.IsTransport())
				m_PreviewItem.SetHealth("", "", m_HealthItem.GetCurrent());

			if (m_PreviewItem.IsInherited(ItemBase)) 
			{
				ItemBase item = ItemBase.Cast(m_PreviewItem);

				if ( item.IsLiquidContainer() )
				{
					if ( item.IsBloodContainer() )
					{
						itemStateType = 1;
					}
					else
					{
						m_TemperatureItem.Enable();
						itemStateType = 0;
					}
					int liquidType = item.GetLiquidTypeInit();
					if ( m_ItemStateType != itemStateType || m_LiquidType != liquidType )
						UpdateItemStateType(itemStateType, liquidType);
				}
				else
				{
					if ( item.HasFoodStage() && item.CanBeCooked() )
					{
						m_TemperatureItem.Enable();
						
						if ( m_ItemStateType != 2 )
							UpdateItemStateType(2);
					}
					else
					{
						m_ItemStateType = -1;
						m_LiquidType = 0;
					}
				}

				Magazine mag;
				if (Class.CastTo(mag, item))
				{
					if (mag.GetAmmoMax() > 0)
					{
						float min;

						if ( mag.IsAmmoPile() && mag.GetAmmoMax() > 1 )
							min = 1.0;
						else
							min = 0.0;

						m_QuantityItem.SetMinMax(min, mag.GetAmmoMax());
						m_QuantityItem.SetStepValue(1);
						m_QuantityItem.Enable();
					}
				}
				else if (item.GetQuantityMax() - item.GetQuantityMin() > 0)
				{
					if ( item.IsSplitable() )
						m_QuantityItem.SetStepValue(1);
					else
						m_QuantityItem.SetStepValue(0.1);

					m_QuantityItem.SetMinMax(item.GetQuantityMin(), item.GetQuantityMax());
					m_QuantityItem.Enable();
				}
			}
		}
		else
		{
			m_ItemPreview.Show( false );
		}

		if (m_ItemStateType > -1)
		{
			m_ItemDataList.Enable();
		}		
		else if (m_ItemStateType != itemStateType)
		{
			m_ItemDataList.SetItems({""});
			UpdateQuantityItemColor();
			m_ItemDataList.Disable();
		}

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::UpdateItemPreview" );
		#endif
	}

	override void OnFocus()
	{
		super.OnFocus();

		m_ItemPreview.Show(true);
	}

	override void OnUnfocus()
	{
		super.OnUnfocus();

		m_ItemPreview.Show(false);
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
			g_Game.GetDragQueue().Call( this, "UpdateRotation" );
			GetMousePos( m_MouseX, m_MouseY );

			return true;
		}

		return false;
	}

	override bool OnMouseWheel(Widget  w, int  x, int  y, int wheel)
	{
		if ( w == m_ItemPreview && m_PreviewItem )
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

	void SetListType( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.m_CurrentType = m_ObjectTypes.Get( action.GetButton() );

		UpdateList();
	}

	void ChangeSpawnMode( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		s_ObjSpawnMode = action.GetSelection();
		
		switch (s_ObjSpawnMode)
		{
			default:
			case COT_ObjectSpawnerMode.PLAYER_POSITION:
			case COT_ObjectSpawnerMode.CURSOR:
			case COT_ObjectSpawnerMode.TARGET_INVENTORY:
			case COT_ObjectSpawnerMode.PLAYER_INVENTORY:
			case COT_ObjectSpawnerMode.OBJECT_INVENTORY:
				m_SpawnButton.SetButton("#STR_COT_OBJECT_MODULE_SPAWN_ON");
				m_AttachmentsButton.Enable();
				m_ObjSetupMode.Enable();
			break;
			case COT_ObjectSpawnerMode.COPYLISTRAW:
			case COT_ObjectSpawnerMode.COPYLISTTYPES:
		#ifdef DZ_Expansion_Market
			case COT_ObjectSpawnerMode.COPYLISTEXPMARKET:
		#endif
				m_SpawnButton.SetButton("#STR_COT_TO_CLIPBOARD:");
				m_AttachmentsButton.Disable();
				m_ObjSetupMode.Disable();
			break;
		}
	}

	void SpawnObject(int mode = COT_ObjectSpawnerMode.CURSOR)
	{
		string clipboardOutput = "";
		string result;

		int itemState = 0; // 0 mean don't do anything
		if ( m_ItemStateType != -1 )
		{
			itemState = m_ItemDataList.GetSelection();
			if (m_ItemStateType == 0)
			{
				//! Liquid
				itemState = m_ObjItemStateLiquid[itemState];
			#ifdef DIAG
				PrintFormat("Liquid type %1 %2", itemState, Liquid.GetNutritionalProfileByType(itemState).GetLiquidClassname());
			#endif
			}
		}

		float health = m_HealthItem.GetCurrent();
		float temp = m_TemperatureItem.GetCurrent();
		float quantity = m_QuantityItem.GetCurrent();

		switch (mode)
		{
			default:
			case COT_ObjectSpawnerMode.PLAYER_POSITION:
				m_Module.SpawnEntity_Position(GetCurrentSelection(), g_Game.GetPlayer().GetPosition(), quantity, health, temp, itemState);
				break;

			case COT_ObjectSpawnerMode.CURSOR:
				m_Module.SpawnEntity_Position(GetCurrentSelection(), GetCursorPos(), quantity, health, temp, itemState);
				break;

			case COT_ObjectSpawnerMode.TARGET_INVENTORY:
				m_Module.SpawnEntity_Position(GetCurrentSelection(), GetCursorPos(), quantity, health, temp, itemState, true);
				break;

			case COT_ObjectSpawnerMode.PLAYER_INVENTORY:
				m_Module.SpawnEntity_Inventory(GetCurrentSelection(), JM_GetSelected().GetPlayers(), quantity, health, temp, itemState);
				break;

			case COT_ObjectSpawnerMode.OBJECT_INVENTORY:
				m_Module.SpawnEntity_Inventory(GetCurrentSelection(), JM_GetSelected().GetObjects(), quantity, health, temp, itemState);
				break;

			case COT_ObjectSpawnerMode.COPYLISTRAW:
				for (int i = 0; i < m_ClassList.GetNumItems(); i++)
				{
					m_ClassList.GetItemText(i, 0, result);
					clipboardOutput += result + "\n";
				}
				g_Game.CopyToClipboard(clipboardOutput);
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
				g_Game.CopyToClipboard(clipboardOutput);
				break;

		#ifdef DZ_Expansion_Market
			case COT_ObjectSpawnerMode.COPYLISTEXPMARKET:
				string categoryJSON;

				auto category = new ExpansionMarketCategory();
				category.Defaults();
				category.DisplayName = m_SearchBox.GetText();

				for (int k = 0; k < m_ClassList.GetNumItems(); k++)
				{
					m_ClassList.GetItemText(k, 0, result);
					auto item = new ExpansionMarketItem(-1, result, 100, 100, 1, 1);
					category.Items.Insert(item);
				}

				string errorMsg;
				if (JsonFileLoader<ExpansionMarketCategory>.MakeData(category, categoryJSON, errorMsg))
					g_Game.CopyToClipboard(categoryJSON);
				else
					COTCreateLocalAdminNotification(new StringLocaliser(errorMsg));
				break;
		#endif
		}
	}

	void DeleteCursor( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		Object obj = m_Module.GetObjectAtCursor();

		if ( obj )
			 DeleteCursor(obj);
	}

	void DeleteCursor(Object obj)
	{
		m_DeletingObject = obj;
		
		CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_GENERIC_CONFIRM", string.Format( Widget.TranslateString( "#STR_COT_OBJECT_MODULE_DELETE_CONFIRMATION_BODY" ), Object.GetDebugName( obj ) ), "#STR_COT_GENERIC_NO", "DeleteEntity_No", "#STR_COT_GENERIC_YES", "DeleteEntity_Yes" );
	}

	private void DeleteEntity_Yes( JMConfirmation confirmation )
	{
		m_Module.DeleteEntity( m_DeletingObject );

		m_DeletingObject = NULL;

		if (m_Module.m_AutoShow)
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Call(m_Module.Hide);  //! Hide after a delay so we can still block actions
	}

	private void DeleteEntity_No( JMConfirmation confirmation )
	{
		m_DeletingObject = NULL;

		if (m_Module.m_AutoShow)
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Call(m_Module.Hide);  //! Hide after a delay so we can still block actions
	}

	void SearchInput_OnClickReset( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_SearchBox.SetText("");
		m_Module.m_SearchText = "";
		UpdateList();
	}

	void SearchInput_OnChange( UIEvent eid, UIActionBase action )
	{
		m_Module.m_SearchText = m_SearchBox.GetText();
		UpdateList();
	}

	void UpdateList()
	{
	#ifdef DIAG_DEVELOPER
		int ticks = TickCount(0);
	#endif

		m_ClassList.ClearItems();
		string closestMatch;
		
		TStringArray configs = new TStringArray;
		configs.Insert( CFG_VEHICLESPATH );
		configs.Insert( CFG_WEAPONSPATH );
		configs.Insert( CFG_MAGAZINESPATH );

		COT_String strSearch = m_Module.m_SearchText;
		bool requireAllKeywords;
		TStringArray keywords = strSearch.KeywordSearch_Prepare(requireAllKeywords);

		for ( int nConfig; nConfig < configs.Count(); nConfig++ )
		{
			string strConfigPath = configs.Get( nConfig );

			int nClasses = g_Game.ConfigGetChildrenCount( strConfigPath );

			for ( int nClass = 0; nClass < nClasses; nClass++ )
			{
				string strName;

				g_Game.ConfigGetChildName( strConfigPath, nClass, strName );

				int scope = g_Game.ConfigGetInt( strConfigPath + " " + strName + " scope" );

				if ( scope == 0 || (scope == 1 && !m_Module.m_AllowRestrictedClassNames) )
					continue;

				string model;
				if (!g_Game.ConfigGetText(strConfigPath + " " + strName + " model", model) || model == string.Empty || model == "bmp")
					continue;

				COT_String strNameLower = strName;

				strNameLower.ToLower();

				if (m_Module.m_CurrentType == "" || g_Game.IsKindOf( strNameLower, m_Module.m_CurrentType ) )
				{
					if ( m_Module.IsExcludedClassName( strNameLower ) ) 
						continue;
					
					if (m_Module.m_FilterWithDisplayName)
					{
						if (!g_Game.ConfigGetText(strConfigPath + " " + strName + " displayName", strNameLower))
							continue;

						strNameLower.ToLower();
					}

					if ( strSearch != "" )
					{
						if (!strNameLower.KeywordSearchImplEx(strSearch, keywords, requireAllKeywords, closestMatch))
							continue;
					}

					m_ClassList.AddItem( strName, NULL, 0 );
				}
			}
		}

	#ifdef DIAG_DEVELOPER
		float elapsed = TickCount(ticks) * 0.0001;
		PrintFormat("UpdateList %1 %2 ms", m_Module.m_SearchText, elapsed);
	#endif

		m_SearchBox.SetTextPreview(closestMatch);
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
}
