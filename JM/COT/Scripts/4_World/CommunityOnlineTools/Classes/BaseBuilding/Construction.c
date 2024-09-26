modded class Construction
{
	bool COT_CanBuildPart(string partName, bool checkMaterials = true)
	{
		if ( !HasRequiredPart( partName ) )
			return false;

		if ( HasConflictPart( partName ) )
			return false;

		if ( checkMaterials && !HasMaterials( partName ) )
			return false;

		return true;
	}

	bool COT_CanDismantlePart(string partName)
	{
		if ( HasDependentPart( partName ) )
			return false;

		if ( !IsPartConstructed( partName ) )
			return false;

		return true;
	}

	void COT_GetParts( out map< string, ref JMConstructionPartData > parts, bool checkMaterials = true )
	{
		for ( int i = 0; i < m_ConstructionParts.Count(); ++i )
		{
			string part_name = m_ConstructionParts.GetKey( i );
			ConstructionPart part = m_ConstructionParts.Get( part_name );

			JMConstructionPartData data = parts.Get( part_name );
			if ( data == NULL )
			{
				data = new JMConstructionPartData;
				parts.Insert( part_name, data );
			}

			data.m_Name = part_name;
			data.m_DisplayName = part.m_Name;

			if ( part.IsBuilt() )
			{
				data.m_State = JMConstructionPartState.BUILT;
			} else if ( !HasRequiredPart( part_name ) )
			{
				data.m_State = JMConstructionPartState.REQUIRED_PART_NOT_BUILT;
			} else if ( HasConflictPart( part_name ) )
			{
				data.m_State = JMConstructionPartState.CONFLICTING_PART;
			} else if ( checkMaterials && !HasMaterials( part_name ) )
			{
				data.m_State = JMConstructionPartState.NOT_ENOUGH_MATERIALS;
			} else
			{
				data.m_State = JMConstructionPartState.CAN_BUILD;
			}
		}
	}

	void COT_BuildParts(TStringArray parts_name, PlayerBase player, bool checkMaterials = true)
	{
		foreach(string part_name: parts_name)
			COT_BuildRequiredParts(part_name, player, checkMaterials);

		UpdateVisuals();
	}
	
	void COT_BuildRequiredParts(string part_name, PlayerBase player, bool checkMaterials = true)
	{
		string main_part_name = GetConstructionPart( part_name ).GetMainPartName();
		string cfg_path = "cfgVehicles" + " " + GetParent().GetType() + " "+ "Construction" + " " + main_part_name + " " + part_name + " " + "required_parts";
		
		array<string> required_parts = new array<string>;
		GetGame().ConfigGetTextArray( cfg_path, required_parts );
		
		for ( int i = 0; i < required_parts.Count(); ++i )
		{
			if ( !IsPartConstructed( required_parts.Get( i ) ) )
				COT_BuildRequiredParts(required_parts.Get( i ), player, checkMaterials);
		}

		if ( !IsPartConstructed( part_name ) )
			COT_BuildPart(part_name, player, checkMaterials);
	}

	void COT_DismantleParts(TStringArray parts_name, PlayerBase player)
	{
		foreach(string part_name: parts_name)
			COT_DismantleRequiredParts(part_name, player);

		UpdateVisuals();
	}
	
	void COT_DismantleRequiredParts(string part_name, PlayerBase player)
	{
		array<string> required_parts = new array<string>;
		required_parts = GetValidDepenentPartsArray(part_name);
		
		for ( int i = 0; i < required_parts.Count(); ++i )
		{
			if ( HasDependentPart( required_parts.Get( i ) ) )
				COT_DismantleRequiredParts(required_parts.Get( i ), player);
			else
				COT_DismantlePart(required_parts.Get( i ), player);
		}

		if ( !HasDependentPart( part_name ) )
			COT_DismantlePart(part_name, player);
	}

	void COT_BuildPart(string part_name, PlayerBase player, bool checkMaterials = true )
	{
		if ( !COT_CanBuildPart( part_name, checkMaterials ) )
			return;

		string damage_zone;
		if ( DamageSystem.GetDamageZoneFromComponentName( GetParent(), part_name, damage_zone ) )
		{
			GetParent().SetHealthMax( damage_zone, "Health" );
		}

		if ( m_ConstructionBoxTrigger )
			DestroyCollisionTrigger();

		GetParent().OnPartBuiltServer( player, part_name, AT_BUILD_PART );
	}

	void COT_DismantlePart( string part_name, PlayerBase player )
	{
		if ( !COT_CanDismantlePart( part_name ) )
			return;

		GetParent().OnPartDismantledServer( player, part_name, AT_DISMANTLE_PART );

		string damage_zone;
		if ( DamageSystem.GetDamageZoneFromComponentName( GetParent(), part_name, damage_zone ) )
		{
			if ( GetParent().GetHealth( damage_zone, "Health" ) > 0 )
			{
				GetParent().SetHealth( damage_zone, "Health", 0 );
			}
		}
	}

	void COT_RepairPart( string part_name )
	{
		string damage_zone;
		if ( DamageSystem.GetDamageZoneFromComponentName( GetParent(), part_name, damage_zone ) )
		{
			GetParent().SetHealthMax( damage_zone, "Health" );
		}
	}
};