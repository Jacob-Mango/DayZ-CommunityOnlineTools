class GameModule: EditorModule
{
    protected ref GameSettings settings;
    protected ref GameMeta meta;

    void GameModule()
    {
        GetRPCManager().AddRPC( "COT_Game", "LoadData", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Game", "SpawnBaseBuilding", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Game", "SpawnVehicle", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Game", "SetOldAiming", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Game", "ThrowApple", this, SingeplayerExecutionType.Client );

        GetPermissionsManager().RegisterPermission( "Game.ChangeAimingMode" );
        GetPermissionsManager().RegisterPermission( "Game.ThrowApple" );
        GetPermissionsManager().RegisterPermission( "Game.View" );
    }

    override bool HasAccess()
    {
        return GetPermissionsManager().HasPermission( "Game.View" );
    }

    override void OnMissionLoaded()
    {
        super.OnMissionLoaded();

        if ( GetGame().IsClient() )
            GetRPCManager().SendRPC( "COT_Game", "LoadData", new Param, true );
    }

    override void ReloadSettings()
    {
        super.ReloadSettings();

        settings = GameSettings.Load();

        for ( int i = 0; i < GameSettings_Vehicles.Count(); i++ )
        {
            string vehicle = GameSettings_Vehicles.GetKey( i );
            vehicle.Replace( " ", "." );
            GetPermissionsManager().RegisterPermission( "Game.Spawn.Vehicle." + vehicle );
        }

        for ( int j = 0; j < GameSettings_BaseBuilding.Count(); j++ )
        {
            string base = GameSettings_BaseBuilding.GetKey( j );
            base.Replace( " ", "." );
            GetPermissionsManager().RegisterPermission( "Game.Spawn.BaseBuilding." + base );
        }
    }

    override void OnMissionFinish()
    {
        super.OnMissionFinish();

        if ( GetGame().IsServer() )
            settings.Save();
    }

    override string GetLayoutRoot()
    {
        return "JM/COT/gui/layouts/Game/GameMenu.layout";
    }

    ref array< string > GetVehicles()
    {
        return meta.Vehicles;
    }

    ref array< string > GetBaseBuilding()
    {
        return meta.BaseBuilding;
    }

    override void OnUpdate( float timeslice )
    {
        if ( GetGame().IsClient() )
        {
            if ( m_OldAiming )
            {
                GetPlayer().OverrideShootFromCamera( false );
            }
        }
    }

    private void FillCar( Car car, CarFluid fluid )
    {
        float cap = car.GetFluidCapacity( fluid );
        car.Fill( fluid, cap );
    }
    
    void LoadData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if( type == CallType.Server )
        {
            GetRPCManager().SendRPC( "COT_Game", "LoadData", new Param1< string >( JsonFileLoader< GameMeta >.JsonMakeData( GameMeta.DeriveFromSettings( settings ) ) ), true );
        }

        if( type == CallType.Client )
        {
            Param1< string > data;
            if ( !ctx.Read( data ) ) return;

            JsonFileLoader< GameMeta >.JsonLoadData( data.param1, meta );

            for ( int i = 0; i < meta.Vehicles.Count(); i++ )
            {
                string vehicle = meta.Vehicles[i];
                vehicle.Replace( " ", "." );
                GetPermissionsManager().RegisterPermission( "Game.Spawn.Vehicle." + vehicle );
            }

            for ( int j = 0; j < meta.BaseBuilding.Count(); j++ )
            {
                string base = meta.BaseBuilding[i];
                base.Replace( " ", "." );
                GetPermissionsManager().RegisterPermission( "Game.Spawn.BaseBuilding." + base );
            }
        }
    }

    void ThrowApple( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Game.ThrowApple", sender ) )
            return;
            
        if( type == CallType.Server )
        {
            vector direction = target.GetDirection();
            vector position = target.GetPosition();

            Human human = Human.Cast( target );

            if ( human )
            {
                int index = human.GetBoneIndexByName("Camera3rd_Helper");

                float pQuat[];
                human.GetBoneRotationWS( index, pQuat )
                vector rotation = Math3D.QuatToAngles( pQuat );

                position = human.GetBonePositionWS( index );
                direction = rotation.AnglesToVector();
            }

            position = position + ( direction * 2 ) + "0 1.5 0";
            
            Object physicsObj = GetGame().CreateObject( "Apple", position );

            if ( physicsObj == NULL ) return;

            dBodyDestroy( physicsObj );

            autoptr PhysicsGeomDef geoms[] = { PhysicsGeomDef("", dGeomCreateSphere( 0.1 ), "material/default", 0xffffffff )};
            dBodyCreateDynamicEx( physicsObj, "0 0 0", 1.0, geoms );
        }
    }

    void SpawnVehicle( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        ref Param2< string, vector > data;
        if ( !ctx.Read( data ) ) return;

        ref GameVehicleFile file = GameSettings_Vehicles.Get( data.param1 );

        if ( file == NULL ) return;

        ref array< string > attachments = file.VehicleParts;

        if ( attachments.Count() == 0 )
        {
            return;
        }

        if ( !GetPermissionsManager().HasPermission( "Game.Spawn.Vehicle." + data.param1, sender ) )
        {
            return;
        }
        
        if( type == CallType.Server )
        {
            #ifdef HELICOPTERS
            PlayerBase player = PlayerBase.Cast( target );
			JM_UH1Y heli = GetGame().CreateObject( "JM_UH1Y", data.param2 + "0 2 0" );
		
			HumanCommandVehicle vehCommand = player.StartCommand_Vehicle(heli, 0, 0);
			if( vehCommand )
			{
				vehCommand.SetVehicleType(heli.GetAnimInstance());
				player.GetItemAccessor().HideItemInHands(true);

				heli.EngineStart();
				heli.AutohoverOn();
			}
            #else
            Car oCar = Car.Cast( GetGame().CreateObject( data.param1, data.param2 ) );

            for (int j = 0; j < attachments.Count(); j++)
            {
                oCar.GetInventory().CreateInInventory( attachments[j] );
            }

            FillCar( oCar, CarFluid.FUEL );
            FillCar( oCar, CarFluid.OIL );
            FillCar( oCar, CarFluid.BRAKE );
            FillCar( oCar, CarFluid.COOLANT );
            #endif


            COTLog( sender, "Spawned vehicle " + data.param1 + " at " + data.param2.ToString() );
        }
    }

    void SetOldAiming( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Game.ChangeAimingMode", sender ) )
            return;

        Param1< bool > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Server )
        {
            m_OldAiming = data.param1;

            GetRPCManager().SendRPC( "COT_Game", "SetOldAiming", data, true );

            COTLog( sender, "Set the server aiming mode to " + data.param1 );
        }

        if( type == CallType.Client )
        {
            m_OldAiming = data.param1;
            
            GetPlayer().OverrideShootFromCamera( !m_OldAiming );
        }
    }

    private bool DetermineWillFit( EntityAI fittingCargo, string ClassName )
    {
        TIntArray values = new TIntArray;
        GetGame().ConfigGetIntArray( "CfgVehicles " + ClassName + " itemSize", values );

        if ( values.Count() != 2 ) return false;

        int iwidth = values[0];
        int iheight = values[1];

        CargoBase cargo = fittingCargo.GetInventory().GetCargo();

        int cwidth = cargo.GetWidth();
        int cheight = cargo.GetHeight();

        if ( iwidth > cwidth ) return false;
        if ( iheight > cheight ) return false;

        return true;
    }

    private EntityAI SpawnItem( PlayerBase player, string ClassName )
    {
        vector position = player.GetPosition();

        position[0] = position[0] + ( Math.RandomInt( 1, 50 ) - 25 ) / 10.0;
        position[1] = position[1] + 0.1;
        position[2] = position[2] + ( Math.RandomInt( 1, 50 ) - 25 ) / 10.0;

        return EntityAI.Cast( GetGame().CreateObject( ClassName, position, false, true, true ) );
    }

    private EntityAI SpawnItemInSeaChest( PlayerBase player, EntityAI chest, string ClassName, float numStacks, float stackSize )
    {
        EntityAI item;
        ItemBase itemBs;

        for ( float i = 0; i < numStacks; i++ )
        {
            item = EntityAI.Cast( chest.GetInventory().CreateInInventory( ClassName ) );

            if ( item )
            {
                itemBs = ItemBase.Cast( item );

                itemBs.SetQuantity( stackSize );
            } else 
            {
                if ( DetermineWillFit( chest, ClassName ) )
                {
                    chest = SpawnItem( player, "SeaChest" );

                    item = EntityAI.Cast( chest.GetInventory().CreateInInventory( ClassName ) );

                    itemBs = ItemBase.Cast( item );

                    itemBs.SetQuantity( stackSize );
                } else
                {
                    item = SpawnItem( player, ClassName );

                    itemBs = ItemBase.Cast( item );

                    itemBs.SetQuantity( stackSize );
                }
            }
        }

        return chest;
    }

    void SpawnBaseBuilding( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        ref Param2< string, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

        ref GameBaseBuildingFile file = GameSettings_BaseBuilding.Get( data.param1 );

        if ( file == NULL ) return;

        ref array< ref GameBaseBuildingSpawn > parts = file.BaseBuildingAttachments;

        if ( parts.Count() == 0 )
        {
            return;
        }

        string perm = file.Name;
        perm.Replace( " ", "." );

        if ( !GetPermissionsManager().HasPermission( "Game.Spawn.BaseBuilding." + perm, sender ) )
        {
            return;
        }
        
        if( type == CallType.Server )
        {
            for ( int i = 0; i < players.Count(); i++ )
            {
                EntityAI chest = SpawnItem( players[i].PlayerObject, "SeaChest" );

                for (int j = 0; j < parts.Count(); j++)
                {
                    chest = SpawnItemInSeaChest( players[i].PlayerObject, chest, parts[j].Item, parts[j].NumberOfStacks, parts[j].StackSize );
                }

                COTLog( sender, "Spawned basebuilding " + data.param1 + " on " + players[i].GetGUID() );
            }

        }
    }
}