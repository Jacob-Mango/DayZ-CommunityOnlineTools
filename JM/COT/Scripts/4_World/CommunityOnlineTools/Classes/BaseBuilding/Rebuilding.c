#ifndef DAYZ_1_29
//! 1.30+
//! Build, dismantle and repair for rebuildable map buildings (irrigation tunnel, well,
//! rebuildable houses): plain Buildings with a Rebuilding construction. Uses
//! BuildPartServerEx / DismantlePartServerEx, as the vanilla debug spawn does.
modded class Rebuilding
{
	override void COT_BuildPart(string part_name, PlayerBase player, bool checkMaterials = true )
	{
		if ( !IsPartConstructed( part_name ) && COT_CanBuildPart( part_name, checkMaterials ) )
			BuildPartServerEx( player, part_name, AT_BUILD_PART );
	}

	override void COT_DismantlePart( string part_name, PlayerBase player )
	{
		if ( !COT_CanDismantlePart( part_name ) )
			return;

		DismantlePartServerEx( NULL, part_name, AT_DISMANTLE_PART );
	}
}
#endif
