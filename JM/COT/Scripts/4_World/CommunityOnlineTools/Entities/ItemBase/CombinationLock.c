modded class CombinationLock
{
	int m_COT_CombinationLocked_Default;
	int m_COT_Combination_Default;
	int m_COT_CombinationInside_Default;

	void CombinationLock()
	{
		Print("CombinationLock()");

		m_COT_CombinationLocked_Default = m_CombinationLocked;
		m_COT_Combination_Default = m_Combination;
		m_COT_CombinationInside_Default = m_CombinationInside;
		Print(m_CombinationLocked);
		Print(m_Combination);
		Print(m_CombinationInside);
		Print(m_COT_CombinationLocked_Default);
		Print(m_COT_Combination_Default);
		Print(m_COT_CombinationInside_Default);
	}

	override bool COT_IsLock()
	{
		return true;
	}

	override bool COT_CanInteract()
	{
		return true;
	}

	override bool COT_SetCode( int code )
	{
		if ( code < 0 || code.ToString().Length() > m_LockDigits )
			return false;

		SetCombinationLocked( code );
		m_Combination = code;
		m_CombinationInside = code;

		if ( IsLocked() )
		{
			ShuffleLock();
			CheckLockedStateServer();
			Synchronize();

			return true;
		}

		return COT_SetLockState( true );
	}

	override bool COT_HasCode()
	{
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

	override bool COT_SetLockState( bool locked )
	{
		if ( !super.COT_SetLockState( locked ) )
			return false;

		EntityAI parent = GetHierarchyParent();

		if ( !parent )
			return false;

		if ( locked )
		{
			if ( !IsLocked() )
				LockServer( parent );
		}
		else if ( IsLocked() )
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

	override bool COT_CanBeLocked()
	{
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

	override bool COT_CanResetCode()
	{
		if ( m_CombinationLocked == m_COT_CombinationLocked_Default && m_Combination == m_COT_Combination_Default && m_CombinationInside == m_COT_CombinationInside_Default )
			return false;

		return true;
	}

	override bool COT_ResetCode()
	{
		COT_SetLockState( false );

		Print(m_CombinationLocked);
		Print(m_Combination);
		Print(m_CombinationInside);
		Print(m_COT_CombinationLocked_Default);
		Print(m_COT_Combination_Default);
		Print(m_COT_CombinationInside_Default);
		SetCombinationLocked( m_COT_CombinationLocked_Default );
		m_Combination = m_COT_Combination_Default;
		m_CombinationInside = m_COT_CombinationInside_Default;
		Print(m_CombinationLocked);
		Print(m_Combination);
		Print(m_CombinationInside);

		UpdateLockDigits();

		Synchronize();

		return true;
	}
}
