modded class DigitalCodeLock
{
	override bool COT_IsLock()
	{
		return true;
	}

	override bool COT_CanInteract()
	{
		return HasEnergy();
	}

	override bool COT_HasCode()
	{
		return HasPIN();
	}

	override string COT_GetCode()
	{
		return GetLockPIN();
	}

	override bool COT_SetLockState( bool locked )
	{
		if ( !super.COT_SetLockState( locked ) )
			return false;

		SetLockState( locked );

		return true;
	}

	override bool COT_CanBeLocked()
	{
		if ( CanBeLocked() && HasPIN() )
			return true;

		return false;
	}

	override bool COT_IsLocked()
	{
		return IsLocked();
	}

	override bool COT_CanSetCode()
	{
		return CanSetCode();
	}

	override bool COT_CanResetCode()
	{
		if ( HasPIN() && CanSetCode() )
			return true;

		return false;
	}

	override bool COT_ResetCode()
	{
		if ( !CanSetCode() )
			return false;

		//! Unlocked first: a lock that is still locked when its code is cleared would
		//! keep its door shut with nothing left to open it by.
		SetLockState( false );
		ResetPIN();

		return true;
	}
}
