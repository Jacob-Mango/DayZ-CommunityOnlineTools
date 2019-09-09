modded class JMModuleManager
{
    protected autoptr array< ref JMRenderableModuleBase > m_COTModules;

    void JMModuleManager()
    {
		m_COTModules = new array< ref JMRenderableModuleBase >;

        CommunityOnlineToolsBase.SI_OPEN.Insert( SetPreventModuleBindings );
    }

    void ~JMModuleManager()
    {
        CommunityOnlineToolsBase.SI_OPEN.Remove( SetPreventModuleBindings );
    }
    
	protected override void RegisterModule( ref JMModuleBase module )
	{
        if ( module.IsInherited( JMRenderableModuleBase ) )
        {
            m_COTModules.Insert( JMRenderableModuleBase.Cast( module ) );
        }
    }

	array< ref JMRenderableModuleBase > GetCOTModules()
	{
		return m_COTModules;
	}

	void OnClientPermissionsUpdated()
	{
		for ( int i = 0; i < m_Modules.Count(); i++ )
		{
			m_Modules.GetElement(i).OnClientPermissionsUpdated();
		}
	}
}