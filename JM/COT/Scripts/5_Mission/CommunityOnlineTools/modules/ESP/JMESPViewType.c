class JMESPViewType
{
	typename MetaType = JMESPMeta;

	string Localisation;
	string Permission;

	int Colour;

	bool HasPermission;
	bool View;

	void JMESPViewType()
	{
		Permission = "INVALID";
		Localisation = "INVALID";

		Colour = ARGB( 255, 255, 255, 255 );

		View = false;
	}

	void CreateMeta( out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewType::CreateMeta( out = " + meta + " ) void;" );
		#endif
		#endif

		if ( meta == NULL )
			Class.CastTo( meta, MetaType.Spawn() );
			
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "-JMESPViewType::CreateMeta( out = " + meta + " ) void;" );
		#endif
		#endif
	}

	bool IsValid( Object obj, out JMESPMeta meta )
	{
		Error( "Not implemented!" );
		return false;
	}

	static bool IsPlayer( Object obj, out DayZPlayer player = null )
	{
		if ( !Class.CastTo( player, obj ) )
			return false;

		DayZPlayerInstanceType type = player.GetInstanceType();

		return type == DayZPlayerInstanceType.INSTANCETYPE_CLIENT || type == DayZPlayerInstanceType.INSTANCETYPE_REMOTE;
	}
};

class JMESPViewTypePlayer: JMESPViewType
{
	void JMESPViewTypePlayer()
	{
		Permission = "Player";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_PLAYER";

		Colour = ARGB( 255, 58, 220, 206 );

		MetaType = JMESPMetaPlayer;
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewTypePlayer::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		#endif

		DayZPlayer player;
		if ( !IsPlayer( obj, player ) )
			return false;
		
		CreateMeta( meta );

		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		if ( player.GetIdentity() )
		{
			meta.player = GetPermissionsManager().GetPlayer( player.GetIdentity().GetId() );
		} 

		if ( meta.player )
		{
			meta.name = meta.player.GetName();
		} else
		{
			meta.name = meta.GetName();
		}

		return true;
	}
};

class JMESPViewTypePlayerAI: JMESPViewType
{
	void JMESPViewTypePlayerAI()
	{
		Permission = "Player AI";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_PLAYER_AI";

		Colour = ARGB( 255, 58, 220, 206 );

		MetaType = JMESPMeta;
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewTypePlayerAI::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		#endif

		if ( !obj.IsMan() || IsPlayer( obj ) )
			return false;

		CreateMeta( meta );

		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );

		if ( meta.player )
		{
			meta.name = meta.player.GetName();
		} else
		{
			meta.name = meta.GetName();
		}

		return true;
	}
};

class JMESPViewTypeInfected: JMESPViewType
{
	void JMESPViewTypeInfected()
	{
		Permission = "Infected";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Infected";

		Colour = ARGB( 255, 215, 219, 0 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewTypeInfected::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		#endif

		EntityAI entity;
		if ( !Class.CastTo( entity, obj ) || !( entity.IsZombie() || entity.IsZombieMilitary() ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypeAnimal: JMESPViewType
{
	void JMESPViewTypeAnimal()
	{
		Permission = "Animal";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Animal";

		Colour = ARGB( 255, 46, 219, 0 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewTypeAnimal::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		#endif
		
		EntityAI entity;
		if ( !Class.CastTo( entity, obj ) || !entity.IsAnimal() )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypeCar: JMESPViewType
{
	void JMESPViewTypeCar()
	{
		Permission = "Car";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Car";

		MetaType = JMESPMetaCar;

		Colour = ARGB( 255, 255, 113, 237 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewTypeCar::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		#endif
		
		if ( !obj.IsInherited(CarScript) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypeBoat: JMESPViewType
{
	void JMESPViewTypeBoat()
	{
		Permission = "Boat";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Boat";

		MetaType = JMESPMetaBoat;

		Colour = ARGB( 255, 255, 113, 237 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewTypeBoat::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		#endif
		
		if ( !obj.IsInherited(BoatScript) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypeTrain: JMESPViewType
{
	void JMESPViewTypeTrain()
	{
		Permission = "Train";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Train";

		MetaType = JMESPMetaTrain;

		Colour = ARGB( 255, 255, 113, 237 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		if ( !obj.IsBuilding() || !CommunityOnlineToolsBase.IsHypeTrain(obj) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypeWeapon: JMESPViewType
{
	void JMESPViewTypeWeapon()
	{
		Permission = "Weapon";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Weapons";

		Colour = ARGB( 255, 55, 223, 189 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewTypeWeapon::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		#endif
		
		Weapon_Base wpn;
		if ( !Class.CastTo( wpn, obj ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};


class JMESPViewTypeArchery: JMESPViewTypeWeapon
{
	void JMESPViewTypeArchery()
	{
		Permission = "Weapon.Archery";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Archery";

		Colour = ARGB( 255, 79, 221, 188 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewTypeArchery::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		#endif
		
		Archery_Base wpn;
		if ( !Class.CastTo( wpn, obj ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypeBoltRifle: JMESPViewTypeWeapon
{
	void JMESPViewTypeBoltRifle()
	{
		Permission = "Weapon.BoltRifle";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Bolt_Rifles";

		Colour = ARGB( 255, 79, 221, 188 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewTypeBoltActionRifle::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		#endif
		
		BoltRifle_Base wpn;
		if ( !Class.CastTo( wpn, obj ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypeBoltActionRifle: JMESPViewTypeWeapon
{
	void JMESPViewTypeBoltActionRifle()
	{
		Permission = "Weapon.BoltActionRifle";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Bolt_Action_Rifles";

		Colour = ARGB( 255, 79, 221, 188 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewTypeBoltActionRifle::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		#endif
		
		BoltActionRifle_Base wpn;
		if ( !Class.CastTo( wpn, obj ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypeRifle: JMESPViewTypeWeapon
{
	void JMESPViewTypeRifle()
	{
		Permission = "Weapon.Rifle";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Rifles";

		Colour = ARGB( 255, 102, 219, 186 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewTypeRifle::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		#endif
		
		Rifle_Base wpn;
		if ( !Class.CastTo( wpn, obj ) || obj.IsKindOf("Pistol_Base") )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypePistol: JMESPViewTypeWeapon
{
	void JMESPViewTypePistol()
	{
		Permission = "Weapon.Pistol";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Pistols";

		Colour = ARGB( 255, 123, 216, 183 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewTypePistol::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		#endif
		
		if ( !obj.IsKindOf("Pistol_Base") )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypeLauncher: JMESPViewTypeWeapon
{
	void JMESPViewTypeLauncher()
	{
		Permission = "Weapon.Launcher";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Launchers";

		Colour = ARGB( 255, 123, 216, 183 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewTypeLauncher::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		#endif
		
		if ( !obj.ShootsExplosiveAmmo() )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypeItemBase: JMESPViewType
{
	void JMESPViewTypeItemBase()
	{
		Permission = "Item";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Items";

		Colour = ARGB( 255, 70, 180, 255 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewTypeItemBase::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		#endif
		
		if ( !obj.IsItemBase() && !obj.IsInventoryItem() )
			return false;

		if ( !CheckLootCategory( obj ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}

	bool CheckLootCategory( Object obj )
	{
		return true;
	}
};

class JMESPViewTypeUnknown: JMESPViewTypeItemBase
{
	void JMESPViewTypeUnknown()
	{
		Permission = "Item.Unknown";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Unknown_Items";

		Colour = ARGB( 255, 87, 178, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeUnknown::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif

		ItemBase item;
		if ( !Class.CastTo( item, obj ) )
			return false;

		if ( Edible_Base.Cast( obj ) != NULL )
			return false;

		if ( item.IsFood() )
			return false;

		if ( item.IsItemTent() )
			return false;

		if ( BaseBuildingBase.Cast( obj ) != NULL )
			return false;

		if ( item.IsExplosive() )
			return false;

		if ( ItemBook.Cast( obj ) != NULL )
			return false;

		if ( item.IsContainer() )
			return false;

		if ( item.IsTransmitter() )
			return false;

		if ( item.IsClothing() )
			return false;

		if ( item.IsMagazine() )
			return false;

		if ( item.IsAmmoPile() )
			return false;

		if ( item.IsWeapon() )
			return false;

		return true;
	}
};

class JMESPViewTypeTent: JMESPViewTypeItemBase
{
	void JMESPViewTypeTent()
	{
		Permission = "Item.Tent";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Tents";

		Colour = ARGB( 255, 130, 174, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeTent::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif

		return obj.IsItemTent();
	}
};

class JMESPViewTypeBaseBuilding: JMESPViewTypeItemBase
{
	void JMESPViewTypeBaseBuilding()
	{
		Permission = "Item.BaseBuilding";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Base_Building";

		Colour = ARGB( 255, 174, 166, 255 );

		MetaType = JMESPMetaBaseBuilding;
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeBaseBuilding::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif

		return BaseBuildingBase.Cast( obj ) != NULL;
	}
};

class JMESPViewTypeFood: JMESPViewTypeItemBase
{
	void JMESPViewTypeFood()
	{
		Permission = "Item.Food";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Food";

		Colour = ARGB( 255, 194, 161, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeFood::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif

		return obj.IsFood() || Edible_Base.Cast( obj ) != NULL;
	}
};

class JMESPViewTypeExplosive: JMESPViewTypeItemBase
{
	void JMESPViewTypeExplosive()
	{
		Permission = "Item.Explosive";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Explosives";

		Colour = ARGB( 255, 213, 156, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeExplosive::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif
		
		ItemBase item;
		if ( Class.CastTo( item, obj ) )
		{
			return item.IsExplosive();
		}

		return false;
	}
};

class JMESPViewTypeBook: JMESPViewTypeItemBase
{
	void JMESPViewTypeBook()
	{
		Permission = "Item.Book";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Books";

		Colour = ARGB( 255, 231, 150, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeBook::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif

		return ItemBook.Cast( obj ) != NULL;
	}
};

class JMESPViewTypeContainer: JMESPViewTypeItemBase
{
	void JMESPViewTypeContainer()
	{
		Permission = "Item.Container";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Containers";

		Colour = ARGB( 255, 246, 144, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeContainer::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif

		return obj.IsContainer() && !obj.IsItemTent();
	}
};

class JMESPViewTypeTransmitter: JMESPViewTypeItemBase
{
	void JMESPViewTypeTransmitter()
	{
		Permission = "Item.Transmitter";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Transmitters";

		Colour = ARGB( 255, 246, 144, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeTransmitter::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif
		
		return obj.IsTransmitter();
	}
};

class JMESPViewTypeClothing: JMESPViewTypeItemBase
{
	void JMESPViewTypeClothing()
	{
		Permission = "Item.Clothing";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Clothing";

		Colour = ARGB( 255, 255, 138, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeClothing::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif
		
		return obj.IsClothing();
	}
};

class JMESPViewTypeMagazine: JMESPViewTypeItemBase
{
	void JMESPViewTypeMagazine()
	{
		Permission = "Item.Magazine";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Magazines";

		Colour = ARGB( 255, 255, 132, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeMagazine::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif

		return obj.IsMagazine() && !obj.IsAmmoPile();
	}
};

class JMESPViewTypeAmmo: JMESPViewTypeItemBase
{
	void JMESPViewTypeAmmo()
	{
		Permission = "Item.Ammo";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Ammo";

		Colour = ARGB( 255, 255, 132, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeAmmo::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif

		return obj.IsAmmoPile();
	}
};

class JMESPViewTypeImmovable: JMESPViewType
{
	void JMESPViewTypeImmovable()
	{
		Permission = "Immovable";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Immovable";

		Colour = ARGB( 255, 255, 124, 255 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPViewTypeImmovable::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		#endif

		if ( obj.IsItemBase() )
			return false;

		if ( obj.IsDayZCreature() )
			return false;

		if ( obj.IsMan() )
			return false;

		if ( obj.IsTransport() )
			return false;

		if ( obj.IsPlainObject() )
			return false;

		if ( obj.IsRock() )
			return false;

		if ( obj.IsBush() )
			return false;

		if ( obj.IsTree() )
			return false;

		if ( obj.IsBuilding() )
			return false;
				
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypePlainObject: JMESPViewTypeImmovable
{
	void JMESPViewTypePlainObject()
	{
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Plain_Objects";
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		if ( !obj.IsPlainObject() )
			return false;
				
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypeRock: JMESPViewTypeImmovable
{
	void JMESPViewTypeRock()
	{
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Rocks";
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		if ( !obj.IsRock() )
			return false;
				
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypeWoodSource: JMESPViewTypeImmovable
{
	void JMESPViewTypeWoodSource()
	{
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Wood_Sources";
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		if ( !obj.IsWoodBase() )
			return false;
				
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypeBush: JMESPViewTypeWoodSource
{
	void JMESPViewTypeBush()
	{
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Bushes";
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		if ( !obj.IsBush() )
			return false;
				
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypeTree: JMESPViewTypeWoodSource
{
	void JMESPViewTypeTree()
	{
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Trees";
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		if ( !obj.IsTree() )
			return false;
				
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};

class JMESPViewTypeBuilding: JMESPViewTypeImmovable
{
	void JMESPViewTypeBuilding()
	{
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Buildings";
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		if ( !obj.IsBuilding() || CommunityOnlineToolsBase.IsHypeTrain(obj) )
			return false;
				
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = meta.GetName();

		return true;
	}
};
