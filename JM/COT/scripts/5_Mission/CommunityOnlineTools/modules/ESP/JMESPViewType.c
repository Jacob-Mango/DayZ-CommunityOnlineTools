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

class JMESPViewTypeBaseBuilding: JMESPViewType
{
	void JMESPViewTypeBaseBuilding()
	{
		Permission = "BaseBuilding";
		Localisation = "Base Building";

		Colour = ARGB( 255, 144, 182, 240 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPViewTypeBaseBuilding::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
		#endif
		
		bool isValid = obj.IsContainer() || obj.CanUseConstruction() || obj.IsFireplace() || obj.IsInherited( GardenBase );
		
		if ( !isValid )
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

class JMESPViewTypeItem: JMESPViewType
{
	void JMESPViewTypeItem()
	{
		Permission = "Item";
		Localisation = "Items";

		Colour = ARGB( 255, 20, 112, 255 );
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPViewTypeItem::IsValid( obj = " + Object.GetDebugName( obj ) + ", out ) bool;" );
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

class JMESPViewTypeItemTool: JMESPViewTypeItem
{
	void JMESPViewTypeItemTool()
	{
		Permission = "Item.Tool";
		Localisation = "Tools";

		Colour = ARGB( 255, 40, 112, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeItemTool::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif
		
		string lootCategory;
		GetGame().ConfigGetText( "cfgVehicles " + obj.GetType() + " lootCategory", lootCategory );
		if ( lootCategory != "Tools" )
			return false;

		return true;
	}
};

class JMESPViewTypeItemCrafted: JMESPViewTypeItem
{
	void JMESPViewTypeItemCrafted()
	{
		Permission = "Item.Crafted";
		Localisation = "Crafted";

		Colour = ARGB( 255, 60, 112, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeItemCrafted::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif
		
		string lootCategory;
		GetGame().ConfigGetText( "cfgVehicles " + obj.GetType() + " lootCategory", lootCategory );
		if ( lootCategory != "Crafted" )
			return false;

		return true;
	}
};

class JMESPViewTypeItemTent: JMESPViewTypeItem
{
	void JMESPViewTypeItemTent()
	{
		Permission = "Item.Tent";
		Localisation = "Tents";

		Colour = ARGB( 255, 80, 112, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeItemTent::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif
		
		string lootCategory;
		GetGame().ConfigGetText( "cfgVehicles " + obj.GetType() + " lootCategory", lootCategory );
		if ( lootCategory != "Tents" )
			return false;

		return true;
	}
};

class JMESPViewTypeItemMaterial: JMESPViewTypeItem
{
	void JMESPViewTypeItemMaterial()
	{
		Permission = "Item.Material";
		Localisation = "Materials";

		Colour = ARGB( 255, 100, 112, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeItemMaterial::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif
		
		string lootCategory;
		GetGame().ConfigGetText( "cfgVehicles " + obj.GetType() + " lootCategory", lootCategory );
		if ( lootCategory != "Materials" )
			return false;

		return true;
	}
};

class JMESPViewTypeItemAttachment: JMESPViewTypeItem
{
	void JMESPViewTypeItemAttachment()
	{
		Permission = "Item.Attachment";
		Localisation = "Attachments";

		Colour = ARGB( 255, 120, 112, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeItemAttachment::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif
		
		string lootCategory;
		GetGame().ConfigGetText( "cfgVehicles " + obj.GetType() + " lootCategory", lootCategory );
		if ( lootCategory != "Attachments" )
			return false;

		return true;
	}
};

class JMESPViewTypeItemFood: JMESPViewTypeItem
{
	void JMESPViewTypeItemFood()
	{
		Permission = "Item.Food";
		Localisation = "Food";

		Colour = ARGB( 255, 140, 112, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeItemFood::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif
		
		string lootCategory;
		GetGame().ConfigGetText( "cfgVehicles " + obj.GetType() + " lootCategory", lootCategory );
		if ( lootCategory != "Food" )
			return false;

		return true;
	}
};

class JMESPViewTypeItemExplosive: JMESPViewTypeItem
{
	void JMESPViewTypeItemExplosive()
	{
		Permission = "Item.Explosive";
		Localisation = "Explosives";

		Colour = ARGB( 255, 160, 112, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeItemExplosive::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif
		
		string lootCategory;
		GetGame().ConfigGetText( "cfgVehicles " + obj.GetType() + " lootCategory", lootCategory );
		if ( lootCategory != "Explosives" )
			return false;

		return true;
	}
};

class JMESPViewTypeItemBook: JMESPViewTypeItem
{
	void JMESPViewTypeItemBook()
	{
		Permission = "Item.Book";
		Localisation = "Books";

		Colour = ARGB( 255, 180, 112, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeItemBook::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif
		
		string lootCategory;
		GetGame().ConfigGetText( "cfgVehicles " + obj.GetType() + " lootCategory", lootCategory );
		if ( lootCategory != "Books" )
			return false;

		return true;
	}
};

class JMESPViewTypeItemContainer: JMESPViewTypeItem
{
	void JMESPViewTypeItemContainer()
	{
		Permission = "Item.Container";
		Localisation = "Containers";

		Colour = ARGB( 255, 200, 112, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeItemContainer::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif
		
		string lootCategory;
		GetGame().ConfigGetText( "cfgVehicles " + obj.GetType() + " lootCategory", lootCategory );
		if ( lootCategory != "Containers" )
			return false;

		return true;
	}
};

class JMESPViewTypeItemEyewear: JMESPViewTypeItem
{
	void JMESPViewTypeItemEyewear()
	{
		Permission = "Item.Eyewear";
		Localisation = "Eyewear";

		Colour = ARGB( 255, 220, 112, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeItemEyewear::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif
		
		string lootCategory;
		GetGame().ConfigGetText( "cfgVehicles " + obj.GetType() + " lootCategory", lootCategory );
		if ( lootCategory != "Eyewear" )
			return false;

		return true;
	}
};

class JMESPViewTypeItemAmmo: JMESPViewTypeItem
{
	void JMESPViewTypeItemAmmo()
	{
		Permission = "Item.Ammo";
		Localisation = "Ammo";

		Colour = ARGB( 255, 240, 112, 255 );
	}

	override bool CheckLootCategory( Object obj )
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "JMESPViewTypeItemAmmo::CheckLootCategory( obj = " + Object.GetDebugName( obj ) + " ) bool;" );
		#endif
		
		string lootCategory;
		GetGame().ConfigGetText( "cfgVehicles " + obj.GetType() + " lootCategory", lootCategory );
		if ( lootCategory != "Ammo" )
			return false;

		return true;
	}
};