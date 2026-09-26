modded class ThirstNotfr
{
	override void DisplayTendency(float delta)
	{
		COT_SubstituteSpectatedPlayer();

		super.DisplayTendency(delta);

		COT_RestoreOwner();
	}

	override protected float GetObservedValue()
	{
		COT_SubstituteSpectatedPlayer();

		float value = super.GetObservedValue();

		COT_RestoreOwner();

		return value;
	}
}
