//! The half of COT's construction access that both kinds of construction share:
//! Construction (base building objects) and Rebuilding (rebuildable map buildings
//! such as the irrigation tunnel entrance) are siblings under ConstructionBase.
modded class ConstructionBase
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
			data.m_MainPartName = part.GetMainPartName();

			// 1.30: the part's own m_Name field is stale and no longer
			// populated - the localized label now only comes through
			// GetName() -> m_PartTypeData.GetNameLocalized().
			data.m_DisplayName = part.GetName();

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

	//! The verbs below differ per construction type - Construction goes through
	//! its BaseBuildingBase parent, Rebuilding through its own server methods -
	//! so they are overridden, never reached through the base.
	void COT_BuildParts(TStringArray parts_name, PlayerBase player, bool checkMaterials = true)
	{
	}

	void COT_BuildRequiredParts(string part_name, PlayerBase player, bool checkMaterials = true)
	{
	}

	void COT_DismantleParts(TStringArray parts_name, PlayerBase player)
	{
	}

	void COT_DismantleRequiredParts(string part_name, PlayerBase player)
	{
	}

	void COT_RepairPart( string part_name )
	{
	}

	void COT_RepairParts( TStringArray parts_name )
	{
	}
}
