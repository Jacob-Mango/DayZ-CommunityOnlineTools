// Hooks COT's JMModuleConstructor from our mod only — do not edit @Community-Online-Tools.
#ifdef JM_COT
modded class JMModuleConstructor
{
	override void RegisterModules(out TTypenameArray modules)
	{
		super.RegisterModules(modules);
		modules.Insert(ExpansionConfigReloadCOTModule);
	}
}
#endif
