class JMObjectSpawnerForm: JMFormBase
{
	private autoptr map< string, string > m_ObjectTypes;
	private string m_CurrentType;

	private Widget m_TypesActionsWrapper;
	private Widget m_SpawnerActionsWrapper;

	private UIActionSlider m_QuantityItem;
	private UIActionEditableText m_SearchBox;
	private UIActionNavigateButton m_SpawnMode;

	private TextListboxWidget m_ClassList;

	private ItemPreviewWidget m_ItemPreview;
	private EntityAI m_PreviewItem;
	private vector m_Orientation;

	private int m_MouseX;
	private int m_MouseY;

	private string m_ItemsList;

	private JMObjectSpawnerModule m_Module;

	private Object m_DeletingObject;

	private int m_ObjSpawnMode = SpawnSelectMode.CURSOR;
	private ref array< string > m_ObjSpawnModeText =
	{
		"None",
		"#STR_COT_OBJECT_MODULE_CURSOR",
		"#STR_COT_OBJECT_MODULE_SELF"
	};

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
		"staticobj_",
		"proxy"
	};

	protected bool m_IknowWhatIamDoing;

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

		m_SpawnerActionsWrapper = UIActionManager.CreateGridSpacer( spawnactionswrapper, 3, 1 );

		Widget actions = UIActionManager.CreatePanel( m_SpawnerActionsWrapper, 0x00000000, 35 );

		m_SearchBox = UIActionManager.CreateEditableText( actions, "#STR_COT_OBJECT_MODULE_SEARCH", this, "SearchInput_OnChange" );
		m_SearchBox.SetWidth( 0.5 );

		UIActionButton button = UIActionManager.CreateButton( actions, "X", this, "SearchInput_OnClickReset" );
		button.SetWidth( 0.05 );
		button.SetPosition( 0.5 );

		m_QuantityItem = UIActionManager.CreateSlider( actions, "#STR_COT_OBJECT_MODULE_QUANTITY", 0, 100);
		m_QuantityItem.SetWidth( 0.35 );
		m_QuantityItem.SetPosition( 0.55 );
		
		Widget spawnButtons = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 1, 3 );

		UIActionManager.CreateButton( spawnButtons, "#STR_COT_OBJECT_MODULE_SPAWN_ON", this, "Click_SpawnObject" );
		m_SpawnMode = UIActionManager.CreateNavButton( spawnButtons, " #STR_COT_OBJECT_MODULE_CURSOR", JM_COT_ICON_ARROW_LEFT, JM_COT_ICON_ARROW_RIGHT, this, "ChangeSpawnMode" );
		UIActionManager.CreateButton( spawnButtons, "#STR_COT_OBJECT_MODULE_DELETE", this, "DeleteCursor" );
		
		if ( GetGame().IsServer() )
		{
			m_ObjSpawnModeText.Insert("#STR_COT_OBJECT_MODULE_INVENTORY");
		} else {
			m_ObjSpawnModeText.Insert("#STR_COT_OBJECT_MODULE_SELECTED_PLAYERS");
		}

		Widget spawnOptions = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 1, 2 );

		UIActionManager.CreateCheckbox( spawnOptions, "#STR_COT_OBJECT_MODULE_ONDEBUGSPAWN", this, "Click_OnDebugSpawn", m_Module.m_OnDebugSpawn );
		UIActionManager.CreateCheckbox( spawnOptions, "#STR_COT_OBJECT_MODULE_SHOWUNSAFE", this, "Click_OnSafetyToogle", m_IknowWhatIamDoing );
		UIActionManager.CreatePanel( spawnOptions );

		UpdateItemPreview();
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

		SpawnObject(m_ObjSpawnMode);
	}

	void AddObjectType( Widget parent, string name, string config )
	{
		name = Widget.TranslateString( name );
		UIActionManager.CreateButton( parent, name, this, "SetListType" );
		m_ObjectTypes.Insert( name, config );
	}

	void UpdateRotation( int mouse_x, int mouse_y, bool is_dragging )
	{		
		m_Orientation[0] = m_Orientation[0] + ( m_MouseY - mouse_y );
		m_Orientation[1] = m_Orientation[1] - ( m_MouseX - mouse_x );
			
		m_ItemPreview.SetModelOrientation( m_Orientation );
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
		if ( m_PreviewItem )
		{
			m_ItemPreview.SetItem( m_PreviewItem );
			m_ItemPreview.SetModelPosition( Vector( 0, 0, 0.5 ) );
			m_ItemPreview.SetModelOrientation( vector.Zero );
			m_ItemPreview.SetView( m_ItemPreview.GetItem().GetViewIndex() );
			m_ItemPreview.Show( true );

			if (m_PreviewItem.IsInherited(ItemBase)) 
			{
				ItemBase item = ItemBase.Cast(m_PreviewItem);
				if ( item.HasQuantity() )
				{
					m_QuantityItem.Enable();
					m_QuantityItem.SetMin(item.GetQuantityMin());
					m_QuantityItem.SetMax(item.GetQuantityMax());
				}
			}
		}
		else
		{
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

		if ( w == m_PreviewItem && button == MouseState.LEFT )
		{
			GetGame().GetDragQueue().Call( this, "UpdateRotation" );
			GetGame().GetMousePos( m_MouseX, m_MouseY );

			return true;
		} 

		return false;
	}

	override bool OnDoubleClick( Widget w, int x, int y, int button )
	{
		super.OnDoubleClick( w, x, y, button );

		if ( w == m_ClassList && button == MouseState.LEFT )
		{
			SpawnObject(m_ObjSpawnMode);
			
			return true;
		}

		return false;
	}

	override void OnShow()
	{
		super.OnShow();

		m_CurrentType = "";
		
		UpdateList();
	}

	override void OnHide() 
	{
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
		if ( eid == UIEvent.CLICK || eid == UIEvent.CLICK_RIGHTSIDE )
		{
			m_ObjSpawnMode++;
		}
		else if ( eid == UIEvent.CLICK_LEFTSIDE )
		{
			m_ObjSpawnMode--;
		}
		else
		{
			return;
		}

		if ( m_ObjSpawnMode == SpawnSelectMode.UNKNOWN )
			m_ObjSpawnMode = SpawnSelectMode.NONE + 1;

		if ( m_ObjSpawnMode == SpawnSelectMode.NONE )
			m_ObjSpawnMode = SpawnSelectMode.UNKNOWN - 1;

		m_SpawnMode.SetButton(m_ObjSpawnModeText[m_ObjSpawnMode]);
	}

	void SpawnObject(int mode = SpawnSelectMode.NONE)
	{
		if ( mode == SpawnSelectMode.NONE || mode == SpawnSelectMode.UNKNOWN )
			return;

		switch(mode)
		{
			default:
			case SpawnSelectMode.POSITION:
				m_Module.SpawnEntity_Position( GetCurrentSelection(), GetGame().GetPlayer().GetPosition(), m_QuantityItem.GetCurrent(), -1 );
			break;
			case SpawnSelectMode.CURSOR:
				m_Module.SpawnEntity_Position( GetCurrentSelection(), GetCursorPos(), m_QuantityItem.GetCurrent(), -1 );
			break;
			case SpawnSelectMode.INVENTORY:
				m_Module.SpawnEntity_Inventory( GetCurrentSelection(), JM_GetSelected().GetPlayers(), m_QuantityItem.GetCurrent(), -1 );
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

		TStringArray configs = new TStringArray;
		configs.Insert( CFG_VEHICLESPATH );
		configs.Insert( CFG_WEAPONSPATH );
		configs.Insert( CFG_MAGAZINESPATH );
		configs.Insert( "CfgNonAIVehicles" );

		string strSearch = m_SearchBox.GetText();

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
				{
					continue;
				}

				if ( !GetGame().ConfigIsExisting( strConfigPath + " " + strName + " model" ) )
				{
					continue;
				}

				string strNameLower = strName;

				strNameLower.ToLower();

				if ( m_CurrentType == "" || GetGame().IsKindOf( strNameLower, m_CurrentType ) )
				{
					if ( CheckItemCrash( strNameLower ) ) 
					{
						continue; 
					}

					if ( ( strSearch != "" && ( !strNameLower.Contains( strSearch ) ) ) ) 
					{
						continue;
					}

					m_ClassList.AddItem( strName, NULL, 0 );
				}
			}
		}
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
