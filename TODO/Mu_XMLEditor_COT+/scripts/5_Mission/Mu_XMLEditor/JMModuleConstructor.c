modded class JMModuleConstructor
{
	override void RegisterModules( out TTypenameArray modules )
	{
		super.RegisterModules( modules );

		Print( "[Mu_XMLEditor][DEBUG] JMModuleConstructor::RegisterModules() wywolane, modules.Count() przed wstawieniem = " + modules.Count().ToString() );

		modules.Insert( JMXMLEditorModule );

		Print( "[Mu_XMLEditor][DEBUG] JMXMLEditorModule dodany do listy, modules.Count() po wstawieniu = " + modules.Count().ToString() );
	}
}
