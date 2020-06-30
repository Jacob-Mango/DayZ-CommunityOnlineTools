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
		Print( "+JMESPViewType::CreateMeta( out = " + meta + " ) void;" );
		#endif

		if ( meta == NULL )
			Class.CastTo( meta, MetaType.Spawn() );
			
		#ifdef JM_COT_ESP_DEBUG
		Print( "-JMESPViewType::CreateMeta( out = " + meta + " ) void;" );
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
		Localisation = "Players";

		Colour = ARGB( 255, 80, 255, 240 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPViewTypePlayer::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif

		Man man;
		if ( !Class.CastTo( man, obj ) || !man.IsPlayer() )
			return false;
		
		CreateMeta( meta );

		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;
		
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
		Localisation = "Infected";

		Colour = ARGB( 255, 215, 219, 0 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPViewTypeInfected::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif

		EntityAI entity;
		if ( !Class.CastTo( entity, obj ) || !( entity.IsZombie() || entity.IsZombieMilitary() ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;
		
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
		Localisation = "Animal";

		Colour = ARGB( 255, 46, 219, 0 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPViewTypeAnimal::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		
		EntityAI entity;
		if ( !Class.CastTo( entity, obj ) || !entity.IsAnimal() )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;
		
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
		Localisation = "Car";

		Colour = ARGB( 255, 255, 109, 237 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPViewTypeCar::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		
		CarScript car;
		if ( !Class.CastTo( car, obj ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;
		
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
		Localisation = "Weapons";

		Colour = ARGB( 255, 75, 255, 218 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPViewTypeWeapon::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		
		Weapon_Base wpn;
		if ( !Class.CastTo( wpn, obj ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;
		
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
		Localisation = "Bolt Rifles";

		Colour = ARGB( 255, 100, 255, 218 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPViewTypeBoltActionRifle::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		
		BoltRifle_Base wpn;
		if ( !Class.CastTo( wpn, obj ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;
		
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
		Localisation = "Bolt Action Rifles";

		Colour = ARGB( 255, 100, 255, 218 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPViewTypeBoltActionRifle::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		
		BoltActionRifle_Base wpn;
		if ( !Class.CastTo( wpn, obj ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;
		
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
		Localisation = "Rifles";

		Colour = ARGB( 255, 125, 255, 218 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPViewTypeRifle::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		
		Rifle_Base wpn;
		if ( !Class.CastTo( wpn, obj ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;
		
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
		Localisation = "Pistols";

		Colour = ARGB( 255, 150, 255, 218 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPViewTypePistol::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		
		Pistol_Base wpn;
		if ( !Class.CastTo( wpn, obj ) )
			return false;
		
		CreateMeta( meta );
		
		meta.target = obj;
		meta.colour = Colour;
		meta.type = this;
		
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
		Localisation = "Items";

		Colour = ARGB( 255, 20, 112, 255 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPViewTypeItemBase::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
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
		Localisation = "Unknown Items";

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
		Localisation = "Tents";

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
		Localisation = "Base Building";

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
		Localisation = "Food";

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
		Localisation = "Explosives";

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
		Localisation = "Books";

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
		Localisation = "Containers";

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
		Localisation = "Transmitters";

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
		Localisation = "Clothing";

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
		Localisation = "Magazines";

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
		Localisation = "Ammo";

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