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
		Print(( "+JMESPViewType::CreateMeta( out = " + meta + " ) void;" ));
		#endif
		#endif

		if ( meta == NULL )
			Class.CastTo( meta, MetaType.Spawn() );
			
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print(( "-JMESPViewType::CreateMeta( out = " + meta + " ) void;" ));
		#endif
		#endif
	}

	bool IsValid( Object obj, out JMESPMeta meta )
	{
		Error( "Not implemented!" );
		return false;
	}
};

class JMESPViewTypePlayer: JMESPViewType
{
	void JMESPViewTypePlayer()
	{
		Permission = "Player";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_PLAYER";

		Colour = ARGB( 255, 80, 255, 240 );

		MetaType = JMESPMetaPlayer;
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print(( "+JMESPViewTypePlayer::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" ));
		#endif
		#endif

		PlayerBase man;
		if ( !Class.CastTo( man, obj ) )
			return false;
		
		CreateMeta( meta );

		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		if ( man.GetIdentity() )
		{
			meta.player = GetPermissionsManager().GetPlayer( man.GetIdentity().GetId() );
		} 

		if ( meta.player )
		{
			meta.name = meta.player.GetName();
		} else
		{
			meta.name = obj.GetDisplayName();
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

		Colour = ARGB( 255, 80, 255, 240 );

		MetaType = JMESPMeta;
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print(( "+JMESPViewTypePlayerAI::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" ));
		#endif
		#endif

		Man man;
		PlayerBase player;
		// has to cast to man but can't cast to player
		if ( !Class.CastTo( man, obj ) || !Class.CastTo( player, obj ) )
			return false;
		
		#ifdef EXPANSIONMODAI
		if ( !player.IsAI() ) 
			return false;
		#endif

		CreateMeta( meta );

		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		if ( man.GetIdentity() )
		{
			meta.player = GetPermissionsManager().GetPlayer( man.GetIdentity().GetId() );
		} 

		if ( meta.player )
		{
			meta.name = meta.player.GetName();
		} else
		{
			meta.name = obj.GetDisplayName();
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
		Print(( "+JMESPViewTypeInfected::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" ));
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
		
		meta.name = obj.GetDisplayName();
		if ( meta.name == "" )
		{
			meta.name = obj.GetType();
		}

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
		Print(( "+JMESPViewTypeAnimal::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" ));
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
		
		meta.name = obj.GetDisplayName();
		if ( meta.name == "" )
		{
			meta.name = obj.GetType();
		}

		return true;
	}
};

class JMESPViewTypeCar: JMESPViewType // override this in expansion
{
	void JMESPViewTypeCar()
	{
		Permission = "Car";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Car";

		Colour = ARGB( 255, 255, 109, 237 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print(( "+JMESPViewTypeCar::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" ));
		#endif
		#endif
		
		CarScript car;
		if ( !Class.CastTo( car, obj ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = obj.GetDisplayName();
		if ( meta.name == "" )
		{
			meta.name = obj.GetType();
		}

		return true;
	}
};

class JMESPViewTypeWeapon: JMESPViewType
{
	void JMESPViewTypeWeapon()
	{
		Permission = "Weapon";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Weapons";

		Colour = ARGB( 255, 75, 255, 218 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print(( "+JMESPViewTypeWeapon::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" ));
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
		
		meta.name = obj.GetDisplayName();
		if ( meta.name == "" )
		{
			meta.name = obj.GetType();
		}

		return true;
	}
}

class JMESPViewTypeBoltRifle: JMESPViewTypeWeapon
{
	void JMESPViewTypeBoltRifle()
	{
		Permission = "Weapon.BoltRifle";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Bolt_Rifles";

		Colour = ARGB( 255, 100, 255, 218 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print(( "+JMESPViewTypeBoltActionRifle::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" ));
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
		
		meta.name = obj.GetDisplayName();
		if ( meta.name == "" )
		{
			meta.name = obj.GetType();
		}

		return true;
	}
};

class JMESPViewTypeBoltActionRifle: JMESPViewTypeWeapon
{
	void JMESPViewTypeBoltActionRifle()
	{
		Permission = "Weapon.BoltActionRifle";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Bolt_Action_Rifles";

		Colour = ARGB( 255, 100, 255, 218 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print(( "+JMESPViewTypeBoltActionRifle::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" ));
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
		
		meta.name = obj.GetDisplayName();
		if ( meta.name == "" )
		{
			meta.name = obj.GetType();
		}

		return true;
	}
};

class JMESPViewTypeRifle: JMESPViewTypeWeapon
{
	void JMESPViewTypeRifle()
	{
		Permission = "Weapon.Rifle";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Rifles";

		Colour = ARGB( 255, 125, 255, 218 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print(( "+JMESPViewTypeRifle::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" ));
		#endif
		#endif
		
		Rifle_Base wpn;
		if ( !Class.CastTo( wpn, obj ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = obj.GetDisplayName();
		if ( meta.name == "" )
		{
			meta.name = obj.GetType();
		}

		return true;
	}
};

class JMESPViewTypePistol: JMESPViewTypeWeapon
{
	void JMESPViewTypePistol()
	{
		Permission = "Weapon.Pistol";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Pistols";

		Colour = ARGB( 255, 150, 255, 218 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print(( "+JMESPViewTypePistol::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" ));
		#endif
		#endif
		
		Pistol_Base wpn;
		if ( !Class.CastTo( wpn, obj ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = obj.GetDisplayName();
		if ( meta.name == "" )
		{
			meta.name = obj.GetType();
		}

		return true;
	}
};

class JMESPViewTypeItemBase: JMESPViewType
{
	void JMESPViewTypeItemBase()
	{
		Permission = "Item";
		Localisation = "#STR_COT_ESP_MODULE_VIEW_TYPE_Items";

		Colour = ARGB( 255, 20, 112, 255 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print(( "+JMESPViewTypeItemBase::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" ));
		#endif
		#endif
		
		bool isValid = obj.IsItemBase() || obj.IsInventoryItem();
		
		if ( !isValid )
			return false;

		if ( !CheckLootCategory( obj ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = obj.GetDisplayName();
		if ( meta.name == "" )
		{
			meta.name = obj.GetType();
		}

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

		Colour = ARGB( 255, 40, 112, 255 );
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

		Colour = ARGB( 255, 80, 112, 255 );
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

		Colour = ARGB( 255, 120, 112, 255 );

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

		Colour = ARGB( 255, 140, 112, 255 );
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

		Colour = ARGB( 255, 160, 112, 255 );
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

		Colour = ARGB( 255, 180, 112, 255 );
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

		Colour = ARGB( 255, 200, 112, 255 );
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

		Colour = ARGB( 255, 200, 112, 255 );
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

		Colour = ARGB( 255, 220, 112, 255 );
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

		Colour = ARGB( 255, 240, 112, 255 );
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

		Colour = ARGB( 255, 240, 112, 255 );
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

		Colour = ARGB( 255, 250, 105, 218 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print(( "+JMESPViewTypeImmovable::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" ));
		#endif
		#endif

		ItemBase itm;
		if ( Class.CastTo( itm, obj ) )
			return false;

		DayZCreature ctr;
		if ( Class.CastTo( ctr, obj ) )
			return false;

		DayZPlayer plr;
		if ( Class.CastTo( plr, obj ) )
			return false;
				
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );
		
		meta.name = obj.GetDisplayName();
		if ( meta.name == "" )
		{
			meta.name = obj.GetType();
		}

		return true;
	}
};