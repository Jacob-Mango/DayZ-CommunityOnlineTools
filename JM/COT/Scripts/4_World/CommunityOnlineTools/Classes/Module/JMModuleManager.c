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
		if ( !g_Game ) return;

		JMScriptInvokers.COT_ON_OPEN.Remove( SetPreventModuleBindings );
	}

	array< JMModuleBase > GetAllModules()
	{
		return m_ModuleList;
	}

	array< JMRenderableModuleBase > GetCOTModules()
	{
		return m_COTModules;
	}

	protected override void InitModule( JMModuleBase module )
	{
		super.InitModule( module );

		module.DeclarePermissions();

		if ( module.IsInherited( JMRenderableModuleBase ) )
		{
			m_COTModules.Insert( JMRenderableModuleBase.Cast( module ) );
		}
	}

#ifndef DAYZ_1_26
	//! 1.27+
	void RemoveCOTModule(JMRenderableModuleBase module)
	{
		m_COTModules.RemoveItem(module);
	}
#endif

#ifndef CF_MODULE_PERMISSIONS
	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();
		
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnClientPermissionsUpdated");
		#endif

		for ( int i = 0; i < m_ModuleList.Count(); i++ )
		{
			m_ModuleList[i].OnClientPermissionsUpdated();
		}
	}
#endif
}