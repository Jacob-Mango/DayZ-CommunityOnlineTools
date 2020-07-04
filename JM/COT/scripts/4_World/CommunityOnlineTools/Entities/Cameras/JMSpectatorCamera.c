class JMSpectatorCamera extends JMCameraBase
{
	void OnTargetSelected( Object target )
	{
		DayZPlayerImplement impl;
		if ( !Class.CastTo( impl, target ) )
			return;

		impl.OnSpectateStart( this );
	}

	void OnTargetDeselected( Object target )
	{
		DayZPlayerImplement impl;
		if ( !Class.CastTo( impl, target ) )
			return;

		impl.OnSpectateEnd();
	}

	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );
	}
}