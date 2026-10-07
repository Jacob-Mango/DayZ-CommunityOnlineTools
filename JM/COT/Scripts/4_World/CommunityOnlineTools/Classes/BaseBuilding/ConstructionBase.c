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

	//! The not yet built parts that building `partName` through COT_BuildRequiredParts
	//! puts up: the part itself and, recursively, every required part still missing.
	//! Read on the client to preview a build; it changes nothing.
	void COT_GetPartsToBuild( string partName, notnull TStringArray result )
	{
		ConstructionPart part = GetConstructionPart( partName );
		if ( !part || part.IsBuilt() || result.Find( partName ) > -1 )
			return;

		result.Insert( partName );

		array<string> requiredParts = part.GetRequiredParts();
		if ( !requiredParts )
			return;

		foreach ( string requiredPart : requiredParts )
			COT_GetPartsToBuild( requiredPart, result );
	}

	//! The built parts that dismantling `partName` through COT_DismantleRequiredParts
	//! takes down: the part itself and, recursively, every built part depending on it.
	void COT_GetPartsToDismantle( string partName, notnull TStringArray result )
	{
		if ( !IsPartConstructed( partName ) || result.Find( partName ) > -1 )
			return;

		result.Insert( partName );

		array<string> dependentParts = GetValidDepenentPartsArray( partName );
		if ( !dependentParts )
			return;

		foreach ( string dependentPart : dependentParts )
			COT_GetPartsToDismantle( dependentPart, result );
	}

	//! A model-space box around one part, for drawing it on the client.
	//!
	//! The part's own collision_data box comes first: it is per part, where the
	//! view geometry selection is usually shared by every part of one main part
	//! (all the walls of a fence answer to "wall"). Then the view geometry of
	//! the selection named after the part, then after its main part. False when
	//! the model has none of the three.
	bool COT_GetPartBounds( string partName, out vector min, out vector max )
	{
		ConstructionPart part = GetConstructionPart( partName );
		if ( !part )
			return false;

		array<string> collisionData = part.GetCollisionData();
		if ( collisionData && collisionData.Count() == 2 )
		{
			string minPoint = collisionData[0];
			string maxPoint = collisionData[1];

			if ( m_EntityParent.MemoryPointExists( minPoint ) && m_EntityParent.MemoryPointExists( maxPoint ) )
			{
				vector a = m_EntityParent.GetMemoryPointPos( minPoint );
				vector b = m_EntityParent.GetMemoryPointPos( maxPoint );

				//! The two points are named min and max, but nothing makes a
				//! model author put them that way round on every axis.
				for ( int i = 0; i < 3; ++i )
				{
					min[i] = Math.Min( a[i], b[i] );
					max[i] = Math.Max( a[i], b[i] );
				}

				return true;
			}
		}

		if ( COT_GetSelectionBounds( partName, min, max ) )
			return true;

		return COT_GetSelectionBounds( part.GetMainPartName(), min, max );
	}

	//! The model-space box around every view geometry component of a selection -
	//! the geometry a right-click on the object hits, so the box is where to click.
	bool COT_GetSelectionBounds( string selection, out vector min, out vector max )
	{
		int level = m_EntityParent.GetViewGeometryLevel();

		TIntArray components = new TIntArray;
		m_EntityParent.GetActionComponentsForSelectionName( level, selection, components );

		if ( components.Count() == 0 )
			return false;

		for ( int c = 0; c < components.Count(); ++c )
		{
			vector componentMin;
			vector componentMax;
			m_EntityParent.GetActionComponentMinMax( level, components[c], componentMin, componentMax );

			if ( c == 0 )
			{
				min = componentMin;
				max = componentMax;
				continue;
			}

			for ( int j = 0; j < 3; ++j )
			{
				min[j] = Math.Min( min[j], componentMin[j] );
				max[j] = Math.Max( max[j], componentMax[j] );
			}
		}

		return true;
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
