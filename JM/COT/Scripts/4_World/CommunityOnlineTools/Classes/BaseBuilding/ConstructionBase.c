#ifdef DAYZ_1_29
typedef Construction ConstructionBase
modded class Construction
{
	protected EntityAI 	m_EntityParent;

	override protected void SetParent( BaseBuildingBase parent )
	{
		super.SetParent(parent);
		
		m_EntityParent = parent;
	}
#else
//! 1.30+
//! The half of COT's construction access that both kinds of construction share:
//! Construction (base building objects) and Rebuilding (rebuildable map buildings
//! such as the irrigation tunnel entrance) are siblings under ConstructionBase.
modded class ConstructionBase
{
#endif
	bool COT_CanBuildPart(string partName, bool checkMaterials = true)
	{
		if ( IsPartConstructed( partName ) )
			return false;

		if ( !HasRequiredPart( partName ) )
			return false;

		if ( !PartHasAnySupportFulfilled( partName ) )
			return false;

		if ( COT_HasConflictPart( partName ) )
			return false;

		if ( checkMaterials && !HasMaterials( partName ) )
			return false;

		if ( checkMaterials && MaterialIsRuined( partName ) )
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
		foreach ( string part_name, ConstructionPart part: m_ConstructionParts )
		{
			if ( part.IsUtilityPart() )
				continue;

			JMConstructionPartData data;
			if ( !parts.Find( part_name, data ) )
			{
				data = new JMConstructionPartData;
				parts.Insert( part_name, data );
			}

			data.m_Name = part_name;
			data.m_MainPartName = part.GetMainPartName();

			data.m_DisplayName = part.GetName();

			if ( !data.m_DisplayName )
				data.m_DisplayName = part_name;

			if ( part.IsBuilt() )
			{
				data.m_State = JMConstructionPartState.BUILT;
			} else if ( COT_HasConflictPart( part_name ) )
			{
				data.m_State = JMConstructionPartState.CONFLICTING_PART;
			} else if ( !HasRequiredPart( part_name ) || !PartHasAnySupportFulfilled( part_name ) )
			{
				data.m_State = JMConstructionPartState.REQUIRED_PART_NOT_BUILT;
			} else if ( checkMaterials && ( !HasMaterials( part_name ) || MaterialIsRuined( part_name ) ) )
			{
				data.m_State = JMConstructionPartState.NOT_ENOUGH_MATERIALS;
			} else
			{
				data.m_State = JMConstructionPartState.CAN_BUILD;
			}
		}
	}

	bool COT_HasConflictPart( string partName )
	{
		if ( HasConflictPart( partName ) )
			return true;

		bool hasConflictPart;

		TConstructionPartVariantArray variantsGroups = GetConstructionPart( partName ).GetRequiredPartsVariants();
		if ( variantsGroups )
		{
			foreach ( TStringArray variantsGroup: variantsGroups )
			{
				foreach ( string variant: variantsGroup )
				{
					if ( IsPartConstructed( variant ) )
						return false;  //! Obviously cannot have a conflict if one of the required variants is built

					if ( !hasConflictPart && HasConflictPart( variant ) )
						hasConflictPart = true;
				}
			}
		}

		return hasConflictPart;
	}

	bool COT_PickRequiredPartVariant( string partName, out string requiredVariant )
	{
		TConstructionPartVariantArray variantsGroups = GetConstructionPart( partName ).GetRequiredPartsVariants();
		if ( variantsGroups )
		{
			//! Reverse order because construction parts are usually ordered with highest tier last
			for ( int i = variantsGroups.Count() - 1; i >= 0; --i )
			{
				TStringArray variantsGroup = variantsGroups[i];

				for ( int j = variantsGroup.Count() - 1; j >= 0; --j )
				{
					string variant = variantsGroup[j];
					string partNameLower = variant;
					partNameLower.ToLower();

					if ( partNameLower.Contains( "door" ) && partNameLower.Contains( "barricade" ) )
						continue;  //! Don't build door barricades, prefer actual doors

					if ( IsPartConstructed( variant ) || !HasConflictPart( variant ) )
					{
						requiredVariant = variant;
						return true;
					}
				}
			}
		}

		return false;
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

	array<ConstructionPart> COT_GetBuiltParts()
	{
		return m_BuiltConstructionPartArray;
	}

	//! A model-space box around one part, for drawing it on the client.
	//! Tries the part's own collision_data box first (per part, where the view geometry
	//! is usually shared by a whole main part), then view geometry of the selection named
	//! after the part, then after its main part. False when none exist.
	bool COT_GetPartBounds( string partName, out vector min, out vector max )
	{
		ConstructionPart part = GetConstructionPart( partName );
		if ( !part )
			return false;

	#ifndef DAYZ_1_29
		//! 1.30+
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
	#endif

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

	BaseBuildingBase COT_GetParent()
	{
	#ifdef DAYZ_1_29
		return GetParent();
	#else
		return null;
	#endif
	}

	void COT_BuildParts(TStringArray parts_name, PlayerBase player, bool checkMaterials = true)
	{
		foreach(string part_name: parts_name)
			COT_BuildRequiredParts(part_name, player, checkMaterials);

		UpdateVisuals();
	}
	
	void COT_BuildRequiredParts(string part_name, PlayerBase player, bool checkMaterials = true)
	{
		ConstructionPart part = GetConstructionPart( part_name );
		array<string> requiredParts = part.GetRequiredParts();
		
		string requiredVariant;
		if ( COT_PickRequiredPartVariant( part_name, requiredVariant ) )
			requiredParts.Insert( requiredVariant );
		
		foreach ( string requiredPartName: requiredParts )
		{
			if ( !IsPartConstructed( requiredPartName ) )
				COT_BuildRequiredParts(requiredPartName, player, checkMaterials);
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
		array<string> requiredParts = GetValidDepenentPartsArray(part_name);

		if ( requiredParts )
		{
			foreach ( string requiredPartName: requiredParts )
			{
				if ( HasDependentPart( requiredPartName ) )
					COT_DismantleRequiredParts(requiredPartName, player);
				else
					COT_DismantlePart(requiredPartName, player);
			}
		}

		if ( !HasDependentPart( part_name ) )
			COT_DismantlePart(part_name, player);
	}

	void COT_BuildPart(string part_name, PlayerBase player, bool checkMaterials = true )
	{
		if ( !COT_CanBuildPart( part_name, checkMaterials ) )
			return;

		string damage_zone;
		if ( DamageSystem.GetDamageZoneFromComponentName( m_EntityParent, part_name, damage_zone ) )
		{
			m_EntityParent.SetAllowDamage(true);
			m_EntityParent.SetHealthMax( damage_zone, "Health" );
			m_EntityParent.ProcessInvulnerabilityCheck(m_EntityParent.GetInvulnerabilityTypeString());
		}

	#ifdef DAYZ_1_29
		if ( m_ConstructionBoxTrigger )
			DestroyCollisionTrigger();
	#endif

		if ( COT_GetParent() )
			COT_GetParent().OnPartBuiltServer( player, part_name, AT_BUILD_PART );
	}

	void COT_DismantlePart( string part_name, PlayerBase player )
	{
		if ( !COT_CanDismantlePart( part_name ) )
			return;

		if ( COT_GetParent() )
			COT_GetParent().OnPartDismantledServer( player, part_name, AT_DISMANTLE_PART );

		string damage_zone;
		if ( DamageSystem.GetDamageZoneFromComponentName( m_EntityParent, part_name, damage_zone ) )
		{
			if ( m_EntityParent.GetHealth( damage_zone, "Health" ) > 0 )
			{
				COT_Base.SetHealth( m_EntityParent, damage_zone, "Health", 0 );
			}
		}
	}

	void COT_RepairPart( string part_name )
	{
		string damage_zone;
		if ( DamageSystem.GetDamageZoneFromComponentName( m_EntityParent, part_name, damage_zone ) )
		{
			COT_Base.SetHealthMax( m_EntityParent, damage_zone, "Health" );
		}
	}

	void COT_RepairParts( TStringArray parts_name )
	{
		foreach ( string part_name : parts_name )
			COT_RepairPart( part_name );

		UpdateVisuals();
	}
}
