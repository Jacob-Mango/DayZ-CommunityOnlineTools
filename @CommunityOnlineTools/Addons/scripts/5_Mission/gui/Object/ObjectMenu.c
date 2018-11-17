class ObjectMenu extends Form
{
    protected Widget m_ActionsWrapper;
    protected Widget m_SpawnerActionsWrapper;

    protected UIActionEditableText m_QuantityItem;
    protected UIActionEditableText m_SearchBox;

    protected TextListboxWidget m_classList;

    private ItemPreviewWidget m_item_widget;
    protected EntityAI previewItem;
    private int m_characterRotationX;
    private int m_characterRotationY; // Borrowed from inspectmenu
    private int m_characterScaleDelta;
    private vector m_characterOrientation;

    void ObjectMenu()
    {
    }

    void ~ObjectMenu()
    {
        if ( previewItem ) 
        {
            GetGame().ObjectDelete( previewItem );
        }
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

    void Init()
    {
        m_classList = TextListboxWidget.Cast( layoutRoot.FindAnyWidget( "object_spawn_list" ) );

        m_SpawnerActionsWrapper = layoutRoot.FindAnyWidget( "object_spawn_actions_wrapper" );

        ref Widget spawnInfo = UIActionManager.CreateGridSpacer( m_SpawnerActionsWrapper, 3, 1 );
        m_QuantityItem = UIActionManager.CreateEditableText( spawnInfo, "Quantity: " );
        m_QuantityItem.SetText( "MAX" );

        m_SearchBox = UIActionManager.CreateEditableText( spawnInfo, "Search: ", this, "SearchInput_OnChange" );

        ref Widget spawnButtons = UIActionManager.CreateGridSpacer( spawnInfo, 1, 4 );
        UIActionManager.CreateText( spawnButtons, "Spawn on: " )
        UIActionManager.CreateButton( spawnButtons, "Cursor", this, "SpawnCursor" );
        UIActionManager.CreateButton( spawnButtons, "Self", this, "SpawnPosition" );
        UIActionManager.CreateButton( spawnButtons, "Selected Players", this, "SpawnInventory" );

        m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );
    }

    override void OnShow()
    {
        super.OnShow();

        UpdateList( "All" );
    }

    override void OnHide() 
    {
        super.OnHide();

        if ( previewItem ) 
        {
            GetGame().ObjectDelete( previewItem );
        }
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

/*
    override bool OnItemSelected( Widget w, int x, int y, int row, int column, int oldRow, int oldColumn )
    {
        if ( w == m_classList ) 
        {
            string strSelection = GetCurrentSelection();
            m_characterOrientation = vector.Zero;

            if ( !m_item_widget )
            {
                Widget preview_panel = layoutRoot.FindAnyWidget("preview_pnl");

                if ( preview_panel ) 
                {
                    float width;
                    float height;
                    preview_panel.GetSize(width, height);

                    m_item_widget = ItemPreviewWidget.Cast( GetGame().GetWorkspace().CreateWidget(ItemPreviewWidgetTypeID, 0, 0, 1, 1, WidgetFlags.VISIBLE, ARGB(255, 255, 255, 255), 10, preview_panel) );
                }
            }

            if ( previewItem ) 
            {
                GetGame().ObjectDelete( previewItem );
            }

            previewItem = GetGame().CreateObject( strSelection, vector.Zero, true );

            m_item_widget.SetItem( previewItem );
            m_item_widget.SetModelPosition( Vector(0,0,0.5) );
            m_item_widget.SetModelOrientation( m_characterOrientation );

            float itemx, itemy;        
            m_item_widget.GetPos(itemx, itemy);

            m_item_widget.SetSize( 1.5, 1.5 );

            // align to center 
            m_item_widget.SetPos( -0.225, -0.225 );
        }

        return true;
    }
*/

    void UpdateList( string classFilter ) // All default
    {
        m_classList.ClearItems();

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

                g_Game.ConfigGetChildName( strConfigPath, nClass, strName );

                int scope = g_Game.ConfigGetInt( strConfigPath + " " + strName + " scope" );

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

                    m_classList.AddItem( strName, NULL, 0 );
                }
            }
        }
    }

    string GetCurrentSelection()
    {
        if ( m_classList.GetSelectedRow() != -1 )
        {
            string result;
            m_classList.GetItemText( m_classList.GetSelectedRow(), 0, result );
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