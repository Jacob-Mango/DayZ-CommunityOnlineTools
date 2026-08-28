modded class ItemBase
{
	override void EEInit()
	{
		super.EEInit();
		
		ZenEntityTracker.Register(this);
	}

	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);
		
		ZenEntityTracker.Unregister(this);
	}
}

modded class AnimalBase
{
	override void EEInit()
	{
		super.EEInit();
		
		ZenEntityTracker.Register(this);
	}

	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);
		
		ZenEntityTracker.Unregister(this);
	}
}

modded class ZombieBase
{
	override void EEInit()
	{
		super.EEInit();
		
		ZenEntityTracker.Register(this);
	}

	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);
		
		ZenEntityTracker.Unregister(this);
	}
}

modded class BuildingBase
{
	override void EEInit()
	{
		super.EEInit();
		
		ZenEntityTracker.Register(this);
	}

	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);
		
		ZenEntityTracker.Unregister(this);
	}
}

modded class CarScript
{
	override void EEInit()
	{
		super.EEInit();
		
		ZenEntityTracker.Register(this);
	}

	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);
		
		ZenEntityTracker.Unregister(this);
	}
}

modded class BoatScript
{
	override void EEInit()
	{
		super.EEInit();
		
		ZenEntityTracker.Register(this);
	}

	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);
		
		ZenEntityTracker.Unregister(this);
	}
}

#ifdef DZ_Expansion_AI
modded class eAIBase
{
	override void EEInit()
	{
		super.EEInit();
		
		ZenEntityTracker.Register(this);
	}

	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);
		
		ZenEntityTracker.Unregister(this);
	}
}
#endif