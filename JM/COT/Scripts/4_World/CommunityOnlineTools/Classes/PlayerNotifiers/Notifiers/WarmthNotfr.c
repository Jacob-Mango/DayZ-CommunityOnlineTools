modded class WarmthNotfr
{
	override protected DSLevelsTemp DetermineLevelEx()
	{
		COT_SubstituteSpectatedPlayer();

		DSLevelsTemp level = super.DetermineLevelEx();

		COT_RestoreOwner();

		return level;
	}

	override protected float GetObservedValue()
	{
		COT_SubstituteSpectatedPlayer();

		float value = super.GetObservedValue();

		COT_RestoreOwner();

		return value;
	}
}
