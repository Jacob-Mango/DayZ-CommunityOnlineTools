#ifdef DZ_Expansion_BaseBuilding
modded class ExpansionCodeLock
{
	override bool COT_IsLock()
	{
		return true;
	}

	override bool COT_CanInteract()
	{
		return true;
	}

	override bool COT_SetCode( string code, PlayerBase player = null )
	{
		if ( code )
			COT_CloseHierarchyRoot();

		SetCode( code, player );

		return true;
	}

	override bool COT_HasCode()
	{
		return HasCode();
	}

	override string COT_GetCode()
	{
		return GetCode();
	}

	override int COT_GetMinCodeLength()
	{
		return GetExpansionSettings().GetBaseBuilding().CodeLockLength;
	}

	override int COT_GetMaxCodeLength()
	{
		return GetExpansionSettings().GetBaseBuilding().CodeLockLength;
	}

	override bool COT_SetLockState( bool locked )
	{
		if ( !super.COT_SetLockState( locked ) )
			return false;

		if ( locked )
			ExpansionLock();
		else
			ExpansionUnlock();

		return true;
	}

	override bool COT_IsLocked()
	{
		return ExpansionIsLocked();
	}

	override bool COT_CanSetCode()
	{
		return true;
	}

	override bool COT_ResetCode()
	{
		SetCode( "" );

		return true;
	}
}
#endif