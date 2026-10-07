//! Build, dismantle and repair for the rebuildable map buildings - the irrigation
//! tunnel entrance, the well, the rebuildable houses - which are plain Buildings
//! with a Rebuilding construction rather than BaseBuildingBase with a Construction.
//!
//! These go through BuildPartServerEx / DismantlePartServerEx, the same calls the
//! vanilla debug spawn of a pre-built variant uses, which already run the part's
//! built/dismantled processing and take no materials that are not there.
modded class Rebuilding
{
	override void COT_BuildParts(TStringArray parts_name, PlayerBase player, bool checkMaterials = true)
	{
		foreach(string part_name: parts_name)
			COT_BuildRequiredParts(part_name, player, checkMaterials);

		UpdateVisuals();
	}

	override void COT_BuildRequiredParts(string part_name, PlayerBase player, bool checkMaterials = true)
	{
		ConstructionPart part = GetConstructionPart( part_name );
		if ( !part )
			return;

		array<string> required_parts = part.GetRequiredParts();
		if ( required_parts )
		{
			for ( int i = 0; i < required_parts.Count(); ++i )
			{
				if ( !IsPartConstructed( required_parts.Get( i ) ) )
					COT_BuildRequiredParts( required_parts.Get( i ), player, checkMaterials );
			}
		}

		if ( !IsPartConstructed( part_name ) && COT_CanBuildPart( part_name, checkMaterials ) )
			BuildPartServerEx( player, part_name, AT_BUILD_PART );
	}

	override void COT_DismantleParts(TStringArray parts_name, PlayerBase player)
	{
		foreach(string part_name: parts_name)
			COT_DismantleRequiredParts(part_name, player);

		UpdateVisuals();
	}

	override void COT_DismantleRequiredParts(string part_name, PlayerBase player)
	{
		array<string> dependent_parts = GetValidDepenentPartsArray( part_name );

		if ( dependent_parts )
		{
			for ( int i = 0; i < dependent_parts.Count(); ++i )
			{
				if ( HasDependentPart( dependent_parts.Get( i ) ) )
					COT_DismantleRequiredParts( dependent_parts.Get( i ), player );
				else
					COT_DismantlePart( dependent_parts.Get( i ) );
			}
		}

		if ( !HasDependentPart( part_name ) )
			COT_DismantlePart( part_name );
	}

	//! No player is handed on: the materials of a part an admin takes apart
	//! are not an admin's to collect, and a null player is a supported caller.
	protected void COT_DismantlePart( string part_name )
	{
		if ( !COT_CanDismantlePart( part_name ) )
			return;

		DismantlePartServerEx( NULL, part_name, AT_DISMANTLE_PART );
	}

	override void COT_RepairPart( string part_name )
	{
		string damage_zone;
		if ( DamageSystem.GetDamageZoneFromComponentName( m_EntityParent, part_name, damage_zone ) )
			COT_Base.SetHealthMax( m_EntityParent, damage_zone, "Health" );
	}

	override void COT_RepairParts( TStringArray parts_name )
	{
		foreach ( string part_name : parts_name )
			COT_RepairPart( part_name );

		UpdateVisuals();
	}
}
