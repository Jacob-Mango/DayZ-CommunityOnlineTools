modded class ZombieMaleBase
{
	//! Vanilla ZombieMaleBase (which inherits from ZombieBase) doesn't call super in EEInit, so have to register here
	override void EEInit()
	{
		super.EEInit();

		if (g_Game.IsServer())
			JMEntityTracker.Register(this);
	}
}
