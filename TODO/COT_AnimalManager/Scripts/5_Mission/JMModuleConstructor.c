// Ajoute le module "Gestion Animaux" a la liste des modules COT.
// En 5_Mission, COT enregistre ses modules via JMModuleConstructor (et non ...Base,
// qui est la classe 4_World).
modded class JMModuleConstructor
{
	override void RegisterModules(out TTypenameArray modules)
	{
		super.RegisterModules(modules);

		modules.Insert(JMAnimalsModule);
	}
}
