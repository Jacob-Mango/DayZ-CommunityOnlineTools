modded class JMModuleConstructorBase
{
	override void RegisterModules( out TTypenameArray modules )
	{
		super.RegisterModules( modules );

		modules.Insert( COTSideBarModule );
		modules.Insert( JMWebhookModule );
	}
}