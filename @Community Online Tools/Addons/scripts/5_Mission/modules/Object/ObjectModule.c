class ObjectModule: EditorModule
{
    void ObjectModule()
    {   
        GetRPCManager().AddRPC( "COT_Object", "SpawnObjectPosition", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Object", "SpawnObjectInventory", this, SingeplayerExecutionType.Server );

        GetRPCManager().AddRPC( "COT_Object", "DeleteObject", this, SingeplayerExecutionType.Server );

        GetPermissionsManager().RegisterPermission( "Object.Spawn.Position" );
        GetPermissionsManager().RegisterPermission( "Object.Spawn.Inventory" );
        GetPermissionsManager().RegisterPermission( "Object.View" );

        GetPermissionsManager().RegisterPermission( "Object.Delete" );
    }

    override bool HasAccess()
    {
        return GetPermissionsManager().HasPermission( "Object.View" );
    }

    override string GetLayoutRoot()
    {
        return "JM/COT/gui/layouts/Object/ObjectMenu.layout";
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

            if ( entity == NULL ) return;

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

            COTLog( sender, "Spawned object " + data.param1 + " at " + data.param2.ToString() + " with amount " + data.param3 );
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
            array< ref AuthPlayer > players = DeserializePlayersID( data.param3 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                EntityAI entity = players[i].PlayerObject.GetInventory().CreateInInventory( data.param1 );

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

                COTLog( sender, "Spawned object " + data.param1 + " on " + players[i].GetGUID() + " with amount " + data.param3 );
            }
        }
    }

    void DeleteObject( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Object.Delete", sender ) )
            return;
        
        if( type == CallType.Server )
        {
            GetGame().ObjectDelete( target );
        }
    }
}