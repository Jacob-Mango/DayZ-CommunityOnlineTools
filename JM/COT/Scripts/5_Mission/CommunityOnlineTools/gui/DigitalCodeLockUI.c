modded class DigitalCodeLockUI
{
	protected int m_COT_MinCodeLength = MIN_LOCK_PIN_LENGTH;
	protected int m_COT_MaxCodeLength = MAX_LOCK_PIN_LENGTH;

	protected bool m_COT_ForceSetCode;

	override protected void ClearInput()
	{
		super.ClearInput();

		for (int i = 0; i < MAX_LOCK_PIN_LENGTH; ++i)
		{
			TextWidget digitTextW = TextWidget.Cast(layoutRoot.FindAnyWidget("PasswordDigitNumber_Text" + i));
			if (digitTextW)
			{
				bool show = false;
				if (i < m_COT_MaxCodeLength)
					show = true;
				digitTextW.GetParent().Show(show);
			}
		}
	}

	override void COT_SetTargetLock( ItemBase targetLock )
	{
		if ( m_TargetLock || !targetLock )
			return;

		m_TargetLock = targetLock;
		m_TargetPosition = targetLock.GetPosition();
		m_HasPIN = targetLock.COT_HasCode();

		m_COT_MinCodeLength = targetLock.COT_GetMinCodeLength();
		m_COT_MaxCodeLength = targetLock.COT_GetMaxCodeLength();

		m_COT_ForceSetCode = true;

		ClearInput();
	}

	override protected void Select()
	{
		super.Select();

		int pinLength = m_EnteredPIN.Length();
		if (pinLength > m_COT_MaxCodeLength)
		{
			m_EnteredPIN = m_EnteredPIN.Substring(0, m_COT_MaxCodeLength);

			for (int i = m_COT_MaxCodeLength; i < pinLength; ++i)
			{
				TextWidget digitTextW = TextWidget.Cast(layoutRoot.FindAnyWidget("PasswordDigitNumber_Text" + i));
				if (digitTextW)
					digitTextW.SetText("");
			}
		}
	}

	override protected void ConfirmCode()
	{
		if (!m_COT_ForceSetCode)
		{
			super.ConfirmCode();
			return;
		}

		if (m_EnteredPIN == "" || !m_TargetLock)
			return;
		
		int pinLength = m_EnteredPIN.Length();
		
		if (pinLength < m_COT_MinCodeLength || pinLength > m_COT_MaxCodeLength)
			return;

		//! Need to reset code first before we can force set new code
		if (m_HasPIN)
			CF_Modules<JMESPModule>.Get().ObjectAction(JMESPObjectAction.ResetCode, 0, 0, m_TargetLock);

		Param1<string> pin = new Param1<string>(m_EnteredPIN);
		g_Game.RPCSingleParam(m_TargetLock, ERPCs.RPC_CODE_LOCK_REQUEST, pin, true);

		ClearInput();
	}

	override void Update(float timeslice)
	{
		EntityAI targetLock;

		//! Disable distance check while COT sidebar is open
		if ( m_TargetLock && g_cotBase.IsOpen() )
		{
			targetLock = m_TargetLock;
			m_TargetLock = null;
		}

		super.Update(timeslice);

		if (targetLock)
			m_TargetLock = targetLock;
	}

	override void OnHide()
	{
		super.OnHide();

		m_COT_MinCodeLength = MIN_LOCK_PIN_LENGTH;
		m_COT_MaxCodeLength = MAX_LOCK_PIN_LENGTH;

		m_COT_ForceSetCode = false;
	}
}
