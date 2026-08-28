// Ajoute le module "Gestion KillFeed" a la liste des modules COT.
modded class JMModuleConstructor
{
	override void RegisterModules(out TTypenameArray modules)
	{
		super.RegisterModules(modules);

		modules.Insert(JMKillFeedModule);
	}
}
