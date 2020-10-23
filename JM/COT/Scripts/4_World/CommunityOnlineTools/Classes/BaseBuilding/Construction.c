modded class Construction
{
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

	void COT_BuildPart( string part_name, PlayerBase player, bool checkMaterials = true )
	{
		if ( !HasRequiredPart( part_name ) )
			return;

		if ( HasConflictPart( part_name ) )
			return;

		if ( checkMaterials && !HasMaterials( part_name ) )
			return;

		string damage_zone;
		if ( DamageSystem.GetDamageZoneFromComponentName( GetParent(), part_name, damage_zone ) )
		{
			GetParent().SetHealthMax( damage_zone, "Health" );
		}

		if ( m_ConstructionBoxTrigger )
			DestroyCollisionTrigger();

		#ifdef DAYZ_1_10
		GetParent().OnPartBuiltServer( player, part_name, AT_BUILD_PART );
		#else
		GetParent().OnPartBuiltServer( part_name, AT_BUILD_PART );
		#endif
	}

	void COT_DismantlePart( string part_name, PlayerBase player )
	{
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