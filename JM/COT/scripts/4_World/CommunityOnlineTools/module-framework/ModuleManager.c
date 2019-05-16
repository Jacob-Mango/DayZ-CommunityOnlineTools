class ModuleManager
{
	protected autoptr array< ref Module > m_Modules;
	protected autoptr array< ref EditorModule > m_EditorModules;

	void ModuleManager()
	{
		GetDebugging().Log( "ModuleManager::ModuleManager()", "JM_COT_ModuleFramework" );
	}

	void ~ModuleManager()
	{
		GetDebugging().Log( "ModuleManager::~ModuleManager()", "JM_COT_ModuleFramework" );
	}

	void RegisterModule( Module module )
	{
		m_Modules.Insert( module );

		string message = "";
		
		if ( module.IsInherited( EditorModule ) )
		{
			m_EditorModules.Insert( EditorModule.Cast( module ) );
			message = " as a EditorModule.";
		} else
		{
			message = " as a Module.";
		}

		GetDebugging().Log( "Registered module " + module + message, "JM_COT_ModuleFramework" );
	}

	void RegisterModules( string version )
	{
		GetDebugging().Log( "ModuleManager::RegisterModules()", "JM_COT_ModuleFramework" );

		m_Modules = new ref array< ref Module >;
		m_EditorModules = new ref array< ref EditorModule >;

		RegisterModule( new COTModule( version ) );
	}

	void ReloadSettings()
	{
		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			m_Modules.Get(i).ReloadSettings();
		}
	}

	ref array< ref Module > GetModules()
	{
		return m_Modules;
	}

	ref array< ref EditorModule > GetEditorModules()
	{
		return m_EditorModules;
	}

	void OnInit()
	{
		GetDebugging().Log( "ModuleManager::OnInit()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			m_Modules.Get(i).Init();
		}
	}

	void OnMissionStart()
	{
		GetDebugging().Log( "ModuleManager::OnMissionStart()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			m_Modules.Get(i).OnMissionStart();
		}
	}

	void OnMissionFinish()
	{
		GetDebugging().Log( "ModuleManager::OnMissionFinish()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			m_Modules.Get(i).OnMissionFinish();
		}
	}

	void OnMissionLoaded()
	{
		GetDebugging().Log( "ModuleManager::OnMissionLoaded()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			m_Modules.Get(i).OnMissionLoaded();
		}
	}

	void OnUpdate( float timeslice )
	{
		if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;

		bool inputIsFocused = false;

		ref Widget focusedWidget = GetFocus();
		if ( focusedWidget && focusedWidget.ClassName().Contains("EditBoxWidget") )
		{
			inputIsFocused = true;
		}

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			Module module = m_Modules.Get(i);

			if ( !module.IsPreventingInput() )
			{
				for ( int kb = 0; kb < module.GetBindings().Count(); ++kb )
				{
					KeyMouseBinding k_m_Binding = module.GetBindings().Get(kb);

					if ( COTMenuOpen || GetGame().GetUIManager().GetMenu() )
					{
						if ( !k_m_Binding.CanBeUsedInMenu() )
						{
							continue;
						}
					}

					if ( DISABLE_ALL_INPUT || inputIsFocused )
					{
						continue;
					}

					UAInput input = GetUApi().GetInputByName( k_m_Binding.GetUAInputName() );

					int action = k_m_Binding.GetActionType();

					if ( action & KeyMouseActionType.PRESS && input.LocalPress() )
					{
						GetGame().GameScript.CallFunctionParams( GetModule( k_m_Binding.GetObject() ), k_m_Binding.GetCallBackFunction(), NULL, new Param1< UAInput >( input ) );
					}

					if ( action & KeyMouseActionType.RELEASE && input.LocalRelease() )
					{
						GetGame().GameScript.CallFunctionParams( GetModule( k_m_Binding.GetObject() ), k_m_Binding.GetCallBackFunction(), NULL, new Param1< UAInput >( input ) );
					}

					if ( action & KeyMouseActionType.HOLD && input.LocalHold() )
					{
						GetGame().GameScript.CallFunctionParams( GetModule( k_m_Binding.GetObject() ), k_m_Binding.GetCallBackFunction(), NULL, new Param1< UAInput >( input ) );
					}

					if ( action & KeyMouseActionType.DOUBLECLICK && input.LocalDoubleClick() )
					{
						GetGame().GameScript.CallFunctionParams( GetModule( k_m_Binding.GetObject() ), k_m_Binding.GetCallBackFunction(), NULL, new Param1< UAInput >( input ) );
					}

					if ( action & KeyMouseActionType.CLICK && input.LocalClick() )
					{
						GetGame().GameScript.CallFunctionParams( GetModule( k_m_Binding.GetObject() ), k_m_Binding.GetCallBackFunction(), NULL, new Param1< UAInput >( input ) );
					}
				}
			}
			module.OnUpdate( timeslice );
		}
	}

	ref Module GetModule( typename module_Type )
	{
		for ( int i = 0; i < m_Modules.Count(); ++i )
		{
			ref Module module = m_Modules.Get(i);

			if ( module.GetModuleType() == module_Type)
			{
				return module;
			}
		}

		return NULL;
	}

	ref Module GetModuleByName( string module_name )
	{
		for ( int i = 0; i < m_Modules.Count(); ++i )
		{
			ref Module module = m_Modules.Get( i );

			if (module.GetModuleName() == module_name)
			{
				return module;
			}
		}

		return NULL;
	}

	void OnClientPermissionsUpdated()
	{
		for ( int i = 0; i < m_Modules.Count(); ++i )
		{
			m_Modules[i].OnClientPermissionsUpdated();
		}
	}
}

ref ModuleManager g_cot_ModuleManager;

ref ModuleManager GetModuleManager()
{
	if( !g_cot_ModuleManager )
	{
		g_cot_ModuleManager = new ref ModuleManager();
	}

	return g_cot_ModuleManager;
}

ref ModuleManager NewModuleManager()
{
	if ( g_cot_ModuleManager )
	{
		delete g_cot_ModuleManager;
	}

	g_cot_ModuleManager = new ref ModuleManager();

	return g_cot_ModuleManager;
}