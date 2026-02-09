modded class UndergroundHandlerClient
{
	protected COTPPERequester_CameraNV m_COT_NVRequester;
	
	void UndergroundHandlerClient(PlayerBase player)
	{
		m_COT_NVRequester = COTPPERequester_CameraNV.Cast(PPERequesterBank.GetRequester( PPERequesterBank.COTREQ_CAMERANV));
	}

	override protected void OnUndergroundPresenceUpdate(EUndergroundPresence newPresence, EUndergroundPresence oldPresence)
	{
		super.OnUndergroundPresenceUpdate(newPresence, oldPresence);

		m_COT_NVRequester.m_COT_UndergroundPresenceFactorTarget = (float)newPresence / (float)EUndergroundPresence.FULL;
	}
	
	override protected void UpdateNVGRequester(float value)
	{
		super.UpdateNVGRequester(value);

		m_COT_NVRequester.SetUndergroundExposureCoef(value);
	}
}
