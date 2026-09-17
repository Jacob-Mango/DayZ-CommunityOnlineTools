modded class PPERequester_SandstormEffect
{
	//! An admin inspecting players/events from the free spectator camera should
	//! not have to squint through the sandstorm glow/godrays PPE meant for a
	//! player physically standing in it - force the effect to zero while
	//! spectating instead of letting it fight the requester's own fade.
	override void SetTargetIntensity(float intensity)
	{
		//! JMSpectatorCamera lives in 4_World, one layer above this file, so it
		//! cannot be named here directly - class name string is the only handle
		//! this layer has on it.
		if ( CurrentActiveCamera && CurrentActiveCamera.ClassName() == "JMSpectatorCamera" )
		{
			super.SetTargetIntensity( 0 );
			return;
		}

		super.SetTargetIntensity( intensity );
	}
}
