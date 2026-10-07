modded class Construction
{
	override void COT_BuildParts(TStringArray parts_name, PlayerBase player, bool checkMaterials = true)
	{
		foreach(string part_name: parts_name)
			COT_BuildRequiredParts(part_name, player, checkMaterials);

		UpdateVisuals();
	}
	
	override void COT_BuildRequiredParts(string part_name, PlayerBase player, bool checkMaterials = true)
	{
		string main_part_name = GetConstructionPart( part_name ).GetMainPartName();
		string cfg_path = "cfgVehicles" + " " + GetParent().GetType() + " "+ "Construction" + " " + main_part_name + " " + part_name + " " + "required_parts";
		
		array<string> required_parts = new array<string>;
		g_Game.ConfigGetTextArray( cfg_path, required_parts );
		
		for ( int i = 0; i < required_parts.Count(); ++i )
		{
			if ( !IsPartConstructed( required_parts.Get( i ) ) )
				COT_BuildRequiredParts(required_parts.Get( i ), player, checkMaterials);
		}

		if ( !IsPartConstructed( part_name ) )
			COT_BuildPart(part_name, player, checkMaterials);
	}

	override void COT_DismantleParts(TStringArray parts_name, PlayerBase player)
	{
		foreach(string part_name: parts_name)
			COT_DismantleRequiredParts(part_name, player);

		UpdateVisuals();
	}
	
	override void COT_DismantleRequiredParts(string part_name, PlayerBase player)
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
			GetParent().SetAllowDamage(true);
			GetParent().SetHealthMax( damage_zone, "Health" );
			GetParent().ProcessInvulnerabilityCheck(GetParent().GetInvulnerabilityTypeString());
		}

	#ifdef DAYZ_1_29
		if ( m_ConstructionBoxTrigger )
			DestroyCollisionTrigger();
	#endif

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
				COT_Base.SetHealth( GetParent(), damage_zone, "Health", 0 );
			}
		}
	}

	override void COT_RepairPart( string part_name )
	{
		string damage_zone;
		if ( DamageSystem.GetDamageZoneFromComponentName( GetParent(), part_name, damage_zone ) )
		{
			COT_Base.SetHealthMax( GetParent(), damage_zone, "Health" );
		}
	}

	override void COT_RepairParts( TStringArray parts_name )
	{
		foreach ( string part_name : parts_name )
			COT_RepairPart( part_name );

		UpdateVisuals();
	}
}