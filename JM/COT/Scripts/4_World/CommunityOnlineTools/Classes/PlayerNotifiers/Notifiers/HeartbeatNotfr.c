modded class HeartbeatNotfr
{
	override void DisplayBadge()
	{
		COT_SubstituteSpectatedPlayer();

		super.DisplayBadge();

		COT_RestoreOwner();
	}
}
