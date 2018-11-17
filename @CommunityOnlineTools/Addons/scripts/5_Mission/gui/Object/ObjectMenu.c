class ObjectMenu extends Form
{
    protected ref map< string, string > m_ObjectTypes;

    protected Widget m_ActionsWrapper;
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

    protected bool m_InEditor;
    protected Object m_SelectedObject;

    protected UIActionText m_SelectedObjectType;
    protected UIActionEditableVector m_SelectedObjectPosition;
    protected UIActionEditableVector m_SelectedObjectRotation;

    protected bool m_MouseButtonPressed;

    void ObjectMenu()
    {
        m_ObjectTypes = new ref map< string, string >;
    }

    void ~ObjectMenu()
    {
    }

    override string GetTitle()
    {
        return "Object Spawner";
    }
    
    override string GetIconName()
    {
        return "OB";
    }

    override bool ImageIsIcon()
    {
        return false;
    }

    void AddObjectType( ref Widget parent, string name, string config )
    {
        UIActionManager.CreateButton( parent, name, this, "SetListType" );
        m_ObjectTypes.Insert( name, config );
    }

    void Init()
    {
        m_ClassList = TextListboxWidget.Cast( layoutRoot.FindAnyWidget( "object_spawn_list" ) );

        m_TypesActionsWrapper = layoutRoot.FindAnyWidget( "object_types_actions_wrapper" );

        ref Widget object_preview_wrapper = layoutRoot.FindAnyWidget( "object_preview_wrapper" );

        if ( object_preview_wrapper )
            m_ItemPreview = ItemPreviewWidget.Cast( GetGame().GetWorkspace().CreateWidget(ItemPreviewWidgetTypeID, 0, 0, 1, 1, WidgetFlags.VISIBLE, ARGB(255, 255, 255, 255), 10, object_preview_wrapper ) );

        ref Widget typesButtons = UIActionManager.CreateGridSpacer( m_TypesActionsWrapper, 7, 1 );
        AddObjectType( typesButtons, "ALL", "All" );
        AddObjectType( typesButtons, "Food", "edible_base" );
        AddObjectType( typesButtons, "Vehicles", "transport" );
        AddObjectType( typesButtons, "Firearms", "weapon_base" );
        AddObjectType( typesButtons, "Clothing", "clothing_base" );
        AddObjectType( typesButtons, "Buildings", "house" );
        AddObjectType( typesButtons, "AI", "dz_lightai" );

        m_SpawnerActionsWrapper = layoutRoot.FindAnyWidget( "object_spawn_actions_wrapper" );

        ref Widget spawnactionswrapper = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 3, 1 );

        ref Widget spawnInfo = UIActionManager.CreateGridSpacer( spawnactionswrapper, 1, 2 );

        m_SearchBox = UIActionManager.CreateEditableText( spawnInfo, "Search: ", this, "SearchInput_OnChange" );
        m_QuantityItem = UIActionManager.CreateEditableText( spawnInfo, "Quantity: " );
        m_QuantityItem.SetText( "MAX" );

        ref Widget spawnButtons = UIActionManager.CreateGridSpacer( spawnactionswrapper, 1, 4 );
        UIActionManager.CreateText( spawnButtons, "Spawn on: " )
        UIActionManager.CreateButton( spawnButtons, "Cursor", this, "SpawnCursor" );
        UIActionManager.CreateButton( spawnButtons, "Self", this, "SpawnPosition" );
        UIActionManager.CreateButton( spawnButtons, "Selected Player(s)", this, "SpawnInventory" );

        m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );

        ref Widget editorWrapper = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 9, 1 );

        UIActionManager.CreateButton( editorWrapper, "Toggle Editor", this, "ToggleEditorMode" );

        m_SelectedObjectType = UIActionManager.CreateText( editorWrapper, "Name: " );
        m_SelectedObjectPosition = UIActionManager.CreateEditableVector( editorWrapper, "Position: " );
        m_SelectedObjectRotation = UIActionManager.CreateEditableVector( editorWrapper, "Orientation: " );
    }

    void ToggleEditorMode( UIEvent eid, ref UIActionButton action )
    {
        Print( "ToggleEditorMode" )
        if ( eid != UIEvent.CLICK ) return;

        m_InEditor = !m_InEditor;
    }

	void UpdateRotation(int mouse_x, int mouse_y, bool is_dragging)
	{		
        m_Orientation[0] = m_Orientation[0] + ( m_MouseY - mouse_y );
        m_Orientation[1] = m_Orientation[1] - ( m_MouseX - mouse_x );
            
        m_ItemPreview.SetModelOrientation( m_Orientation );
	}

    override bool OnItemSelected( Widget w, int x, int y, int row, int column, int oldRow, int oldColumn )
	{
        if ( w == m_ClassList ) 
        {
        	string strSelection = GetCurrentSelection();

            m_Orientation = vector.Zero;

			if ( m_PreviewItem ) 
			{
				GetGame().ObjectDelete( m_PreviewItem );
			}

			if ( strSelection == "Mag_Scout_5Rnd") return false; 

			m_PreviewItem = GetGame().CreateObject( strSelection, vector.Zero, true );

			m_ItemPreview.SetItem( m_PreviewItem );
			m_ItemPreview.SetModelPosition( Vector(0, 0, 0.5) );
			m_ItemPreview.SetModelOrientation( m_Orientation );
        }

		return true;
	}

    // HACK FIX UNTIL PROPER INPUT API IS EXPOSED
	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
    {
        super.OnMouseButtonDown( w, x, y, button );

		if ( w == m_PreviewItem )
		{
			GetGame().GetDragQueue().Call(this, "UpdateRotation");
			GetGame().GetMousePos( m_MouseX, m_MouseY );

            return false;
        } 

        // CheckForSelection();

        return false;
    }

    void CheckForSelection()
    {
        m_SelectedObject = GetPointerObject( GetPlayer(), 0.2, m_SelectedObject );
        if ( m_SelectedObject )
        {
            m_SelectedObjectType.SetText( m_SelectedObject.GetType() );
            m_SelectedObjectPosition.SetVector( m_SelectedObject.GetPosition() );
            m_SelectedObjectRotation.SetVector( m_SelectedObject.GetOrientation() );
        } else 
        {
            m_SelectedObjectType.SetText( "" );
            m_SelectedObjectPosition.SetVector( "0 0 0" );
            m_SelectedObjectRotation.SetVector( "0 0 0" );
        }
    }

    void OnUpdate( float timeslice )
    {
        if ( m_InEditor && layoutRoot.IsVisible() )
        {
            if ( GetMouseState(MouseState.LEFT) & MB_PRESSED_MASK )
            {
                if ( m_MouseButtonPressed == false )
                {
                    CheckForSelection();
                }
                
                m_MouseButtonPressed = true;
            } else {
                m_MouseButtonPressed = false;
            }


            if ( m_SelectedObject )
            {
            }
        }
    }

    override void OnShow()
    {
        super.OnShow();

        UpdateList( "All" );
    }

    override void OnHide() 
    {
        super.OnHide();
    }

    void SetListType( UIEvent eid, ref UIActionButton action )
    {
        if ( eid != UIEvent.CLICK ) return;

        UpdateList( m_ObjectTypes.Get( action.GetButton() ) );
    }

    void SpawnCursor( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;

        GetRPCManager().SendRPC( "COT_Object", "SpawnObjectPosition", new Param3< string, vector, string >( GetCurrentSelection(), GetCursorPos(), m_QuantityItem.GetText() ), true );
    }

    void SpawnPosition( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;

        GetRPCManager().SendRPC( "COT_Object", "SpawnObjectPosition", new Param3< string, vector, string >( GetCurrentSelection(), GetGame().GetPlayer().GetPosition(), m_QuantityItem.GetText() ), true );
    }

    void SpawnInventory( UIEvent eid, ref UIActionEditableText action )
    {
        if ( eid != UIEvent.CLICK ) return;

        GetRPCManager().SendRPC( "COT_Object", "SpawnObjectInventory", new Param3< string, string, ref array< string > >( GetCurrentSelection(), m_QuantityItem.GetText(), SerializePlayersGUID( GetSelectedPlayers() ) ), true );
    }

    void SearchInput_OnChange( UIEvent eid, ref UIActionEditableText action )
    {
        UpdateList( "All" );
    }

    void UpdateList( string classFilter ) // All default
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

                if ( classFilter == "All" || GetGame().IsKindOf( strNameLower, classFilter ) ) // Fix for weapon_base not being child of "All"
                {
                    if ( (strSearch != "" && (!strNameLower.Contains( strSearch ))) ) 
                    {
                        continue;
                    }

                    if ( strName == "ItemOptics" ) 
                    {
                        continue; // Fix crash
                    }

                    m_ClassList.AddItem( strName, NULL, 0 );
                }
            }
        }
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

// DZ_LightAI
// House?
// Transport
// Weapon_Base
// Edible_Base
// Clothing_Base