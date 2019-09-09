modded class JMModuleConstructorBase
{
    override void RegisterModules( TTypenameArray modules )
	{
        super.RegisterModules( modules );

        modules.Insert( COTModule );
    }
}