enum JMNVTypes
{
	NV_COT_OFF = -1000,
	NV_COT_ON = 1000
}

modded class DayZPlayerCameraBase
{
	bool m_JMHasAdminNVG;

	override void SetNVPostprocess(int NVtype)
	{
        super.SetNVPostprocess(NVtype);

		switch (NVtype)
		{
			case JMNVTypes.NV_COT_OFF:
				m_JMHasAdminNVG = false;
				PPERequesterBank.GetRequester(COTPPERequester_CameraNV).Stop();
			break;
			case JMNVTypes.NV_COT_ON:
				m_JMHasAdminNVG = true;
				PPERequesterBank.GetRequester(PPERequesterBank.COTREQ_CAMERANV).Start( new Param1<int>(COTPPERequester_CameraNV.COT_ON) );
			break;
			default:
			if (m_JMHasAdminNVG)
			{
				if (NVtype > 0)
					SetNVPostprocess(JMNVTypes.NV_COT_OFF);
				else
					SetNVPostprocess(JMNVTypes.NV_COT_ON);
			}
			break;
		}
		
		if (PlayerBaseClient.Cast(m_pPlayer))
			PlayerBaseClient.Cast(m_pPlayer).SwitchPersonalLight(NVtype < 1);
	}
};
