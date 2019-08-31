class JMModuleBase
{
	protected bool m_Enabled;
	protected bool m_PreventInput;
	protected ref set< ref JMModuleBinding > m_KeyBindings;
	
	void JMModuleBase()
	{
		m_Enabled = true;
		m_PreventInput = false;
		m_KeyBindings = new set< ref JMModuleBinding >;
	}
	
	void ~JMModuleBase()
	{
	}
	
	void Init()
	{
		RegisterKeyMouseBindings();
	}

	void Toggle()
	{
		m_Enabled = !m_Enabled;
	}

	void PreventInput(bool prevent)
	{
		m_PreventInput = prevent;
	}

	string GetModuleName()
	{
		return ClassName();
	}

	typename GetModuleType()
	{
		return GetModuleName().ToType();
	}

	bool IsEnabled()
	{
		return m_Enabled;
	}

	bool IsPreventingInput()
	{
		return m_PreventInput;
	}

	void RegisterKeyMouseBindings() 
	{
	}
	
	void RegisterKeyMouseBinding( JMModuleBinding binding ) 
	{
		m_KeyBindings.Insert( binding );
	}
	
	set< ref JMModuleBinding > GetBindings()
	{
		return m_KeyBindings;
	}

	// Override functions 
	
	void OnUpdate( float timeslice ) 
	{
	}

	void ReloadSettings()
	{
	}

	void OnClientPermissionsUpdated()
	{
	}
	
	/* Mission Functions */
	
	void OnMissionStart()
	{
	}
	
	void OnMissionFinish()
	{
	}
	
	void OnMissionLoaded()
	{
	}
}
