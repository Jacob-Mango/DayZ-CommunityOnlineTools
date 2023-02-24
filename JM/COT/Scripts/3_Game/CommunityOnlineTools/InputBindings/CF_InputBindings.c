#ifndef CF_INPUTBINDINGS_FOCUS_FIX
modded class CF_InputBindings
{
	override void Update( float dt )
	{
		if (!g_Game)
			return;

		auto focus = GetFocus();
		if ( focus && ( focus.IsInherited( EditBoxWidget ) || focus.IsInherited( MultilineEditBoxWidget ) ) && focus.IsVisible() && !GetUApi().GetInputByID( UAUIMenu ).LocalPress() )
			return;

		super.Update( dt );
	}
};
#endif
