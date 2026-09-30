modded class StanceIndicator
{
	override void Update()
	{
		PlayerBase spectatedPlayer;
		if ( m_Player && Class.CastTo( spectatedPlayer, m_Player.m_JM_SpectatedObject ) )
		{
			//! Can't use movement state for spectated player (not available)
			vector headPosition;

			MiscGameplayFunctions.GetHeadBonePos( spectatedPlayer, headPosition );
			vector position = spectatedPlayer.GetPosition();

			vector transform[4];
			spectatedPlayer.GetTransform( transform );

			//! To model space (to be able to detect if we're prone on an incline)
			position = position.InvMultiply4( transform );
			headPosition = headPosition.InvMultiply4( transform );

			int stance = 1;  //! Standing

			if ( headPosition[1] - position[1] < 1.3 )
				stance = 2;  //! Crouching

			if ( headPosition[1] - position[1] < 0.6 )
				stance = 3;  //! Prone

			DisplayStance( stance );
		}
		else
		{
			super.Update();
		}
	}
}
