modded class CF_InputBindings
{
	CF_ModuleGame m_COT_ModuleGame;

	void CF_InputBindings(Class instance)
	{
		Class.CastTo(m_COT_ModuleGame, instance);
	}

	override void Update( float dt )
	{
		if (m_COT_ModuleGame && !m_COT_ModuleGame.COT_CanProcessInputs())
			return;

		#ifndef CF_INPUTBINDINGS_FOCUS_FIX
		Widget focus = GetFocus();
		if ( focus && ( focus.IsInherited( EditBoxWidget ) || focus.IsInherited( MultilineEditBoxWidget ) ) && focus.IsVisible() )
		{
			if ( !GetUApi().GetInputByID( UAUIBack ).LocalPress() )
				return;
		}
		#endif

		super.Update( dt );
	}
}