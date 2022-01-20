modded class JMModuleManager
{
	void JMModuleManager()
	{
		JMScriptInvokers.COT_ON_OPEN.Insert( SetPreventModuleBindings );
	}

	void ~JMModuleManager()
	{
		JMScriptInvokers.COT_ON_OPEN.Remove( SetPreventModuleBindings );
	}
};
