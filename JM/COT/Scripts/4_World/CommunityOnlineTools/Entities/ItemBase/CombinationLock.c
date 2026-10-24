modded class CombinationLock
{
	int m_COT_CombinationLocked_Default;
	int m_COT_Combination_Default;
	int m_COT_CombinationInside_Default;

	void CombinationLock()
	{
		m_COT_CombinationLocked_Default = m_CombinationLocked;
		m_COT_Combination_Default = m_Combination;
		m_COT_CombinationInside_Default = m_CombinationInside;
	}

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
		int combination = code.ToInt();

		if ( combination < 0 || code.Length() > m_LockDigits )
			return false;

		SetCombinationLocked( combination );
		m_Combination = combination;
		m_CombinationInside = combination;

		if ( IsLocked() )
		{
			ResetActionVar();

			ShuffleLock();
			CheckLockedStateServer();
			Synchronize();

			return true;
		}

		return COT_SetLockState( true );
	}

	override bool COT_HasCode()
	{
		if ( m_CombinationLocked == m_COT_CombinationLocked_Default && m_Combination == m_COT_Combination_Default && m_CombinationInside == m_COT_CombinationInside_Default )
			return false;

		return true;
	}

	override string COT_GetCode()
	{
		string combinationText = m_CombinationLocked.ToString();

		//! insert zeros to dials with 0 value
		int lengthDiff = m_LockDigits - combinationText.Length();
		for (int i = 0; i < lengthDiff; ++i)
		{
			combinationText = "0" + combinationText;
		}

		return combinationText;
	}

	override int COT_GetMinCodeLength()
	{
		return m_LockDigits;
	}

	override int COT_GetMaxCodeLength()
	{
		return m_LockDigits;
	}

	override bool COT_SetLockState( bool locked )
	{
		if ( !super.COT_SetLockState( locked ) )
			return false;

		EntityAI parent = GetHierarchyParent();

		if ( !parent )
			return false;

		if ( locked )
		{
			LockServer( parent );
		}
		else
		{
			//! Locking runs the vanilla server path (slot lock and shuffle). Unlocking does NOT:
			//! UnlockServer drops the lock on the ground, and an admin wants the gate openable, so
			//! the dial is just turned back to the locked combination (what the lock calls unlocked).
			m_Combination = m_CombinationLocked;
			m_CombinationInside = m_CombinationLocked;

			InventoryLocation inventoryLocation = new InventoryLocation;
			GetInventory().GetCurrentInventoryLocation(inventoryLocation);
			parent.GetInventory().SetSlotLock(inventoryLocation.GetSlot(), false);

			m_LockActionPerformed = LockAction.UNLOCKED;
			SetTakeable(true);
			CheckLockedStateServer();

			UpdateLockDigits();

			Synchronize();
		}

		return true;
	}

	override bool COT_IsLocked()
	{
		return IsLocked();
	}

	override bool COT_CanSetCode()
	{
		return true;
	}

	override bool COT_ResetCode()
	{
		COT_SetLockState( false );

		SetCombinationLocked( m_COT_CombinationLocked_Default );
		m_Combination = m_COT_Combination_Default;
		m_CombinationInside = m_COT_CombinationInside_Default;

		ResetActionVar();

		UpdateLockDigits();

		Synchronize();

		return true;
	}
}
