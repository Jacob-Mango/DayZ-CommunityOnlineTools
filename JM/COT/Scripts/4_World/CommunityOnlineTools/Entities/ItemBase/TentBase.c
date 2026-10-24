modded class TentBase
{
	override bool COT_IsOpenable()
	{
		return GetState() == PITCHED;
	}

	int COT_GetOpenCount()
	{
		int open;

		foreach (ToggleAnimations toggle, bool toggleState: m_ToggleAnimations)
		{
			bool is_open = (m_OpeningMask & toggle.GetOpeningBit()) == 0;

			if (is_open)
				++open;
		}

		return open;
	}

	bool COT_IsOpen()
	{
		if (COT_GetOpenCount() > 0)
			return true;

		return false;
	}

	int COT_ToggleAnimations(bool state)
	{
		int openEntrances;
		int toggled;
		int toggledEntrances;
		int toggledEntrancesOpened;

		foreach (ToggleAnimations toggle, bool toggleState: m_ToggleAnimations)
		{
			string toggle_off = toggle.GetToggleOff();
			toggle_off.ToLower();

			bool is_entrance = false;
			if (toggle_off.Contains("entrance") || toggle_off.Contains("door"))
				is_entrance = true;

			bool is_closed = (m_OpeningMask & toggle.GetOpeningBit()) != 0;

			if (is_entrance && !is_closed)
				++openEntrances;

			//! `state` true = want to open
			if (is_closed == state)
			{
				if (CanToggleAnimations(toggle_off))
				{
					ToggleAnimation(toggle_off);
					++toggled;

					if (is_entrance)
					{
						++toggledEntrances;

						if (is_closed)
							++toggledEntrancesOpened;
					}
				}
			}
		}

		if (toggledEntrances > 0)
		{
			ItemBase lockItem = JMLockItem.Get( this );

			if (lockItem)
			{
				if (toggledEntrancesOpened > 0)
					lockItem.COT_SetLockState( false );
				else if (openEntrances == 0)
					lockItem.COT_SetLockState( true );
			}
		
			SetAffectPathgraph(true, false);
			
			if (CanAffectPathgraph())
				RegenerateNavmesh();
		}

		return toggled;
	}
}
