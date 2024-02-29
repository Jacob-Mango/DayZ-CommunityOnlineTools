modded class JMModuleManager
{
	protected autoptr array< JMRenderableModuleBase > m_COTModules;

	void JMModuleManager()
	{
		m_COTModules = new array< JMRenderableModuleBase >;

		JMScriptInvokers.COT_ON_OPEN.Insert( SetPreventModuleBindings );
	}

	void ~JMModuleManager()
	{
		JMScriptInvokers.COT_ON_OPEN.Remove( SetPreventModuleBindings );
	}
	
	protected override void InitModule( JMModuleBase module )
	{
		super.InitModule( module );
		
		if ( module.IsInherited( JMRenderableModuleBase ) )
		{
			m_COTModules.Insert( JMRenderableModuleBase.Cast( module ) );
		}
	}

	array< JMRenderableModuleBase > GetCOTModules()
	{
		return m_COTModules;
	}

	array< JMModuleBase > GetAllModules()
	{
		return m_ModuleList;
	}

#ifndef CF_MODULE_PERMISSIONS
	override void OnClientPermissionsUpdated()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnClientPermissionsUpdated");
		#endif

		for ( int i = 0; i < m_ModuleList.Count(); i++ )
		{
			m_ModuleList[i].OnClientPermissionsUpdated();
		}
	}
#endif
};