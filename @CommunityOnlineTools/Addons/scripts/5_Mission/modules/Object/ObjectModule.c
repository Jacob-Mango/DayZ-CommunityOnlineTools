class ObjectModule: EditorModule
{
    protected bool m_InEditor;
    protected Object m_SelectedObject;

    protected bool m_MouseButtonPressed;

    void ObjectModule()
    {   
        GetRPCManager().AddRPC( "COT_Object", "SpawnObjectPosition", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Object", "SpawnObjectInventory", this, SingeplayerExecutionType.Server );

        GetPermissionsManager().RegisterPermission( "Object.Spawn.Position" );
        GetPermissionsManager().RegisterPermission( "Object.Spawn.Inventory" );
    }

    override string GetLayoutRoot()
    {
        return "COT/gui/layouts/Object/ObjectMenu.layout";
    }
    
    void CheckForSelection( ref ObjectMenu objForm )
    {
        m_SelectedObject = GetPointerObject( GetPlayer(), 0.2, m_SelectedObject );

        objForm.SetSelectedObject( m_SelectedObject );
    }

    void ToggleEditor( UIEvent eid, ref UIActionButton action )
    {
        Print( "ToggleEditor" );

        if ( eid != UIEvent.CLICK ) return;

        m_InEditor = !m_InEditor;
    }

    override void OnUpdate( float timeslice ) 
    {
        super.OnUpdate( timeslice );

        ref ObjectMenu objForm = ObjectMenu.Cast( form );

        if ( objForm == NULL ) return;
        
        if ( m_InEditor && objForm.GetLayoutRoot().IsVisible() && ( GetWidgetUnderCursor() == NULL || GetWidgetUnderCursor().GetName() == "Windows" ) )
        {
            if ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK )
            {
                if ( m_MouseButtonPressed == false )
                {
                    CheckForSelection( objForm );
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
    
    void SpawnObjectPosition( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Object.Spawn.Position", sender ) )
            return;

        Param3< string, vector, string > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Server )
        {
            bool ai = false;

            if ( GetGame().IsKindOf( data.param1, "DZ_LightAI" ) ) 
            {
                ai = true;
            }

            EntityAI entity = GetGame().CreateObject( data.param1, data.param2, false, ai );

            entity.SetHealth( entity.GetMaxHealth() );

            if ( entity.IsInherited( ItemBase ) )
            {
                ItemBase oItem = ( ItemBase ) entity;
                SetupSpawnedItem( oItem, oItem.GetMaxHealth(), 1 );

                int quantity = 0;

                string text = data.param3;

                text.ToUpper();

                if (text == "MAX")
                {
                    quantity = oItem.GetQuantityMax();
                } else
                {
                    quantity = text.ToInt();
                }

                oItem.SetQuantity(quantity);
            }

            entity.PlaceOnSurface();
        }
    }

    void SpawnObjectInventory( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Object.Spawn.Inventory", sender ) )
            return;

        ref Param3< string, string, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Server )
        {
            ref array< ref AuthPlayer > players = DeserializePlayersGUID( data.param3 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                EntityAI entity = players[i].GetPlayerObject().GetInventory().CreateInInventory( data.param1 );

                entity.SetHealth( entity.GetMaxHealth() );

                if ( entity.IsInherited( ItemBase ) )
                {
                    ItemBase oItem = ( ItemBase ) entity;
                    SetupSpawnedItem( oItem, oItem.GetMaxHealth(), 1 );

                    int quantity = 0;

                    string text = data.param2;

                    text.ToUpper();

                    if (text == "MAX")
                    {
                        quantity = oItem.GetQuantityMax();
                    } else
                    {
                        quantity = text.ToInt();
                    }

                    oItem.SetQuantity(quantity);
                }
            }
        }
    }
}