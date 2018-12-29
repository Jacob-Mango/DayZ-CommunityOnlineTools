class ObjectMenu extends Form
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

    override void OnInit( bool fromMenu )
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

        ref Widget spawnactionswrapper = layoutRoot.FindAnyWidget( "object_spawn_actions_wrapper" );

        m_SpawnerActionsWrapper = UIActionManager.CreateGridSpacer( spawnactionswrapper, 4, 1 );

        ref Widget spawnInfo = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 1, 2 );

        m_SearchBox = UIActionManager.CreateEditableText( spawnInfo, "Search: ", this, "SearchInput_OnChange" );
        m_QuantityItem = UIActionManager.CreateEditableText( spawnInfo, "Quantity: " );
        m_QuantityItem.SetText( "MAX" );

        ref Widget spawnButtons = NULL;

        if ( fromMenu )
        {
            spawnButtons = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 1, 4 );

            UIActionManager.CreateText( spawnButtons, "Spawn on: " )

            UIActionManager.CreateButton( spawnButtons, "Cursor", this, "SpawnCursor" );
            UIActionManager.CreateButton( spawnButtons, "Self", this, "SpawnPosition" );
            UIActionManager.CreateButton( spawnButtons, "Selected Player(s)", this, "SpawnInventory" );
        } else 
        {
            spawnButtons = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 1, 1 );

            UIActionManager.CreateButton( spawnButtons, "Spawn On Cursor", this, "SpawnCursor" );
        }

        UIActionManager.CreateButton( m_SpawnerActionsWrapper, "Delete On Cursor", this, "DeleteOnCursor" );
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

            if ( m_PreviewItem ) 
            {
                GetGame().ObjectDelete( m_PreviewItem );
            }

            if ( GetGame().IsKindOf( strSelection, "DZ_LightAI" ) ) 
            {
                return true;
            }

            m_Orientation = vector.Zero;

            m_PreviewItem = GetGame().CreateObject( strSelection, vector.Zero, true, false, false );

            m_ItemPreview.SetItem( m_PreviewItem );
            m_ItemPreview.SetModelPosition( Vector(0, 0, 0.5) );
            m_ItemPreview.SetModelOrientation( m_Orientation );
        }

        return true;
    }

    override bool OnMouseButtonDown( Widget w, int x, int y, int button )
    {
        super.OnMouseButtonDown( w, x, y, button );

        if ( w == m_PreviewItem )
        {
            GetGame().GetDragQueue().Call(this, "UpdateRotation");
            GetGame().GetMousePos( m_MouseX, m_MouseY );

            return false;
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

    void SetListType( UIEvent eid, ref UIActionButton action )
    {
        if ( eid != UIEvent.CLICK ) return;

        m_CurrentType = m_ObjectTypes.Get( action.GetButton() );

        UpdateList();
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

        GetRPCManager().SendRPC( "COT_Object", "SpawnObjectInventory", new Param3< string, string, ref array< string > >( GetCurrentSelection(), m_QuantityItem.GetText(), SerializePlayersID( GetSelectedPlayers() ) ), true );
    }

    void DeleteOnCursor( UIEvent eid, ref UIActionButton action )
    {
        if ( eid != UIEvent.CLICK ) return;

        GetRPCManager().SendRPC( "COT_Object", "DeleteObject", new Param, true, NULL, GetCursorObject( 2.0, GetGame().GetPlayer(), 0.01 ) );
    }

    void SearchInput_OnChange( UIEvent eid, ref UIActionEditableText action )
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

                if ( m_CurrentType == "All" || GetGame().IsKindOf( strNameLower, m_CurrentType ) ) // Fix for weapon_base not being child of "All"
                {
                    if ( (strSearch != "" && (!strNameLower.Contains( strSearch ))) ) 
                    {
                        continue;
                    }

                    bool shouldCont = false;

                    switch ( strName )
                    {
                        case "Mag_Scout_5Rnd":
                        case "ItemOptics":
                            shouldCont = true;
                            break;
                        default:
                            shouldCont = false;
                            break;
                    }

                    if ( shouldCont ) 
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