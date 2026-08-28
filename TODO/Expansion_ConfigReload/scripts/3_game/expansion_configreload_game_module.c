[CF_RegisterModule(ExpansionConfigReloadGameModule)]
class ExpansionConfigReloadGameModule: CF_ModuleGame
{
	protected static ExpansionConfigReloadGameModule s_Instance;

	void ExpansionConfigReloadGameModule()
	{
		s_Instance = this;
	}

	static ExpansionConfigReloadGameModule GetInstance()
	{
		return s_Instance;
	}

	override void OnInit()
	{
		super.OnInit();

		#ifndef JM_COT
		if (GetGame() && GetGame().IsServer())
		{
			ExpansionConfigReloadAdmin.EnsureInitialized();
		}
		#endif
	}
}
