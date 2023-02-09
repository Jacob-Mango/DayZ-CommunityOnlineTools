modded class CarScript
{
	static CarScript s_JM_Head;
	CarScript m_JM_Next;
	CarScript m_JM_Prev;

	void CarScript()
	{
		m_JM_Next = s_JM_Head;
		s_JM_Head = this;
	}

	void ~CarScript()
	{
		if (s_JM_Head == this)
			s_JM_Head = m_JM_Next;

		if (m_JM_Next)
			m_JM_Next.m_JM_Prev = m_JM_Prev;

		if (m_JM_Prev)
			m_JM_Prev.m_JM_Next = m_JM_Next;
	}
}
