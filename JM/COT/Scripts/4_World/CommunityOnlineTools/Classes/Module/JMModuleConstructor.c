modded class JMModuleConstructorBase
{
	override void RegisterModules( out TTypenameArray modules )
	{
		super.RegisterModules( modules );

		modules.Insert( COTModule );
		modules.Insert( JMWebhookModule );
	}
};
