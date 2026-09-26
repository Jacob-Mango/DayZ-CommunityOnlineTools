modded class StuffedNotfr
{
	override void DisplayBadge()
	{
		COT_SubstituteSpectatedPlayer();

		super.DisplayBadge(delta);

		COT_RestoreOwner();
	}
}
