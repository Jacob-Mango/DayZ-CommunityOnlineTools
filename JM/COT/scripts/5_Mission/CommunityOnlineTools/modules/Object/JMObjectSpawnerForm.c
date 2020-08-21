class JMObjectSpawnerForm extends JMFormBase
{
	private autoptr map< string, string > m_ObjectTypes;
	private string m_CurrentType;

	private Widget m_TypesActionsWrapper;
	private Widget m_SpawnerActionsWrapper;

	private UIActionEditableText m_QuantityItem;
	private UIActionEditableText m_SearchBox;

	private TextListboxWidget m_ClassList;

	private ItemPreviewWidget m_ItemPreview;
	private EntityAI m_PreviewItem;
	private vector m_Orientation;

	private int m_MouseX;
	private int m_MouseY;

	private string m_ItemsList;

	private JMObjectSpawnerModule m_Module;

	private Object m_DeletingObject;

	void JMObjectSpawnerForm()
	{
		m_ObjectTypes = new map< string, string >;
	}

	void ~JMObjectSpawnerForm()
	{
		delete m_ObjectTypes;
	}

	protected override bool SetModule( ref JMRenderableModuleBase mdl )
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

		Widget spawnInfo = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 1, 2 );

		m_SearchBox = UIActionManager.CreateEditableText( spawnInfo, "#STR_COT_OBJECT_MODULE_SEARCH", this, "SearchInput_OnChange" );
		m_QuantityItem = UIActionManager.CreateEditableText( spawnInfo, "#STR_COT_OBJECT_MODULE_QUANTITY" );
		m_QuantityItem.SetText( "MAX" );

		Widget spawnButtons = NULL;

		spawnButtons = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 1, 5 );

		UIActionManager.CreateText( spawnButtons, "#STR_COT_OBJECT_MODULE_SPAWN_ON" );

		UIActionManager.CreateButton( spawnButtons, "#STR_COT_OBJECT_MODULE_CURSOR", this, "SpawnCursor" );
		UIActionManager.CreateButton( spawnButtons, "#STR_COT_OBJECT_MODULE_SELF", this, "SpawnPosition" );
		
		if ( GetGame().IsServer() )
		{
			UIActionManager.CreateButton( spawnButtons, "#STR_COT_OBJECT_MODULE_INVENTORY", this, "SpawnInventory" );
		} else
		{
			UIActionManager.CreateButton( spawnButtons, "#STR_COT_OBJECT_MODULE_SELECTED_PLAYERS", this, "SpawnInventory" );
		}

		UIActionManager.CreateButton( spawnButtons, "#STR_COT_OBJECT_MODULE_DELETE", this, "DeleteCursor" );

		UpdateItemPreview();
	}

	void AddObjectType( ref Widget parent, string name, string config )
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
		//Print( "+" + this + "::UpdateItemPreview" );
		
		string strSelection = GetCurrentSelection();

		if ( m_PreviewItem ) 
		{
			GetGame().ObjectDelete( m_PreviewItem );
		}

		if ( GetGame().IsKindOf( strSelection, "DZ_LightAI" ) ) 
		{
			m_ItemPreview.Show( false );

			//Print( "-" + this + "::UpdateItemPreview AI" );
			return;
		}

		m_Orientation = vector.Zero;

		m_PreviewItem = EntityAI.Cast( GetGame().CreateObject( strSelection, vector.Zero, true, false ) );

		if ( m_PreviewItem )
		{
			m_ItemPreview.SetItem( m_PreviewItem );
			m_ItemPreview.SetModelPosition( Vector( 0, 0, 0.5 ) );
			m_ItemPreview.SetModelOrientation( vector.Zero );
			m_ItemPreview.SetView( m_ItemPreview.GetItem().GetViewIndex() );
			m_ItemPreview.Show( true );
		} else
		{
			m_ItemPreview.Show( false );
		}

		//Print( "-" + this + "::UpdateItemPreview" );
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

	void SetListType( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;

		m_CurrentType = m_ObjectTypes.Get( action.GetButton() );

		UpdateList();
	}

	void SpawnCursor( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		int quantity = -1;

		string quantText = m_QuantityItem.GetText();

		quantText.ToUpper();

		if ( quantText != "MAX")
		{
			quantity = quantText.ToInt();
		}

		m_Module.SpawnEntity_Position( GetCurrentSelection(), GetCursorPos(), quantity, -1 );
	}

	void SpawnPosition( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		int quantity = -1;

		string quantText = m_QuantityItem.GetText();

		quantText.ToUpper();

		if ( quantText != "MAX")
		{
			quantity = quantText.ToInt();
		}

		m_Module.SpawnEntity_Position( GetCurrentSelection(), GetGame().GetPlayer().GetPosition(), quantity, -1 );
	}

	void SpawnInventory( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		int quantity = -1;

		string quantText = m_QuantityItem.GetText();

		quantText.ToUpper();

		if ( quantText != "MAX")
		{
			quantity = quantText.ToInt();
		}

		m_Module.SpawnEntity_Inventory( GetCurrentSelection(), JM_GetSelected(), quantity, -1 );
	}

	void DeleteCursor( UIEvent eid, ref UIActionBase action )
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
		
		CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_OBJECT_MODULE_DELETE_CONFIRMATION_HEADER", string.Format( Widget.TranslateString( "#STR_COT_OBJECT_MODULE_DELETE_CONFIRMATION_BODY" ), Object.GetDebugName( obj ) ), "#STR_COT_GENERIC_NO", "DeleteEntity_No", "#STR_COT_GENERIC_YES", "DeleteEntity_Yes" );
	}

	private void DeleteEntity_Yes( JMConfirmation confirmation )
	{
		m_Module.DeleteEntity( m_DeletingObject );

		m_DeletingObject = NULL;
	}

	private void DeleteEntity_No( JMConfirmation confirmation )
	{
		
	}

	void SearchInput_OnChange( UIEvent eid, ref UIActionBase action )
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

		for ( int nConfig = 0; nConfig < configs.Count(); ++nConfig )
		{
			string strConfigPath = configs.Get( nConfig );

			int nClasses = g_Game.ConfigGetChildrenCount( strConfigPath );

			for ( int nClass = 0; nClass < nClasses; ++nClass )
			{
				string strName;

				GetGame().ConfigGetChildName( strConfigPath, nClass, strName );

				int scope = GetGame().ConfigGetInt( strConfigPath + " " + strName + " scope" );

				if ( scope == 0 )
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
		static ref array< string > items =
		{
			"itemoptics",
			"quickiebow",
			"m203",
			#ifndef DAYZ_1_09
			"magnum",
			#endif
			"gp25",
			"gp25_standalone",
			"gp25_base",
			"m203_base",
			"m203_standalone"
		};

		for ( int i = 0; i < items.Count(); ++i )
		{
			if ( items[i] == name )
			{
				return true;
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
}