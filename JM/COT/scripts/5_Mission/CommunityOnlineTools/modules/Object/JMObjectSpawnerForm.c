class JMObjectSpawnerForm extends JMFormBase
{
	protected ref map< string, string > m_ObjectTypes;
	protected string m_CurrentType;

	protected Widget m_TypesActionsWrapper;
	protected Widget m_SpawnerActionsWrapper;

	protected UIActionEditableText m_QuantityItem;
	protected UIActionEditableText m_SearchBox;

	protected TextListboxWidget m_ClassList;

	protected ItemPreviewWidget m_ItemPreview;
	protected EntityAI m_PreviewItem;
	protected vector m_Orientation;

	protected int m_MouseX;
	protected int m_MouseY;

	protected string m_ItemsList;

	void JMObjectSpawnerForm()
	{
		m_ObjectTypes = new map< string, string >;
	}

	void ~JMObjectSpawnerForm()
	{
	}

	void AddObjectType( ref Widget parent, string name, string config )
	{
		UIActionManager.CreateButton( parent, name, this, "SetListType" );
		m_ObjectTypes.Insert( name, config );
	}

	override void OnInit()
	{
		m_ClassList = TextListboxWidget.Cast( layoutRoot.FindAnyWidget( "object_spawn_list" ) );

		m_TypesActionsWrapper = layoutRoot.FindAnyWidget( "object_types_actions_wrapper" );

		m_ItemPreview = ItemPreviewWidget.Cast( layoutRoot.FindAnyWidget( "object_preview" ) );

		Widget typesButtons = UIActionManager.CreateGridSpacer( m_TypesActionsWrapper, 8, 1 );
		AddObjectType( typesButtons, "ALL", "" );
		AddObjectType( typesButtons, "Food", "edible_base" );
		AddObjectType( typesButtons, "Vehicles", "transport" );
		AddObjectType( typesButtons, "Firearms", "weapon_base" );
		AddObjectType( typesButtons, "Clothing", "clothing_base" );
		AddObjectType( typesButtons, "Items", "inventory_base" );
		AddObjectType( typesButtons, "Buildings", "house" );
		AddObjectType( typesButtons, "AI", "dz_lightai" );

		Widget spawnactionswrapper = layoutRoot.FindAnyWidget( "object_spawn_actions_wrapper" );

		m_SpawnerActionsWrapper = UIActionManager.CreateGridSpacer( spawnactionswrapper, 4, 1 );

		Widget spawnInfo = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 1, 2 );

		m_SearchBox = UIActionManager.CreateEditableText( spawnInfo, "Search: ", this, "SearchInput_OnChange" );
		m_QuantityItem = UIActionManager.CreateEditableText( spawnInfo, "Quantity: " );
		m_QuantityItem.SetText( "MAX" );

		Widget spawnButtons = NULL;

		spawnButtons = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 1, 4 );

		UIActionManager.CreateText( spawnButtons, "Spawn on: " );

		UIActionManager.CreateButton( spawnButtons, "Cursor", this, "SpawnCursor" );
		UIActionManager.CreateButton( spawnButtons, "Self", this, "SpawnPosition" );
		
		if ( GetGame().IsServer() )
		{
			UIActionManager.CreateButton( spawnButtons, "Inventory", this, "SpawnInventory" );
		} else
		{
			UIActionManager.CreateButton( spawnButtons, "Selected Player(s)", this, "SpawnInventory" );
		}

		UpdateItemPreview();
	}

	void UpdateRotation( int mouse_x, int mouse_y, bool is_dragging )
	{		
		m_Orientation[0] = m_Orientation[0] + ( m_MouseY - mouse_y );
		m_Orientation[1] = m_Orientation[1] - ( m_MouseX - mouse_x );
			
		m_ItemPreview.SetModelOrientation( m_Orientation );
	}

	void UpdateItemPreview()
	{
		string strSelection = GetCurrentSelection();

		if ( m_PreviewItem ) 
		{
			GetGame().ObjectDelete( m_PreviewItem );
		}

		if ( GetGame().IsKindOf( strSelection, "DZ_LightAI" ) ) 
		{
			m_ItemPreview.Show( false );
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

		m_CurrentType = "All";
		
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

		JMObjectSpawnerModule mod;
		if ( !Class.CastTo( mod, module ) )
			return;

		int quantity = -1;

		string quantText = m_QuantityItem.GetText();

		quantText.ToUpper();

		if ( quantText != "MAX")
		{
			quantity = quantText.ToInt();
		}

		mod.SpawnEntity_Position( GetCurrentSelection(), GetCursorPos(), quantity, -1 );
	}

	void SpawnPosition( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMObjectSpawnerModule mod;
		if ( !Class.CastTo( mod, module ) )
			return;

		int quantity = -1;

		string quantText = m_QuantityItem.GetText();

		quantText.ToUpper();

		if ( quantText != "MAX")
		{
			quantity = quantText.ToInt();
		}

		mod.SpawnEntity_Position( GetCurrentSelection(), GetGame().GetPlayer().GetPosition(), quantity, -1 );
	}

	void SpawnInventory( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMObjectSpawnerModule mod;
		if ( !Class.CastTo( mod, module ) )
			return;

		// TODO: FIX TO BE PLAYERS INSTEAD!
		array< EntityAI > entities = new array< EntityAI >;

		int quantity = -1;

		string quantText = m_QuantityItem.GetText();

		quantText.ToUpper();

		if ( quantText != "MAX")
		{
			quantity = quantText.ToInt();
		}

		mod.SpawnEntity_Inventory( GetCurrentSelection(), entities, quantity, -1 );
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
					if ( CheckItemCrash( strName ) ) 
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
		switch ( name )
		{
			case "ItemOptics":
				return true;
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