class ModuleManager
{
	protected autoptr array< ref Module > m_Modules;
	protected autoptr array< ref EditorModule > m_EditorModules;

	void ModuleManager()
	{
		GetLogger().Log( "ModuleManager::ModuleManager()", "JM_COT_ModuleFramework" );
	}

	void ~ModuleManager()
	{
		GetLogger().Log( "ModuleManager::~ModuleManager()", "JM_COT_ModuleFramework" );
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

		GetLogger().Log( "Registered module " + module + message, "JM_COT_ModuleFramework" );
	}

	void RegisterModules()
	{
		GetLogger().Log( "ModuleManager::RegisterModules()", "JM_COT_ModuleFramework" );

		m_Modules = new ref array< ref Module >;
		m_EditorModules = new ref array< ref EditorModule >;

		RegisterModule( new COTModule );
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
		GetLogger().Log( "ModuleManager::OnInit()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			m_Modules.Get(i).Init();
		}
	}

	void OnMissionStart()
	{
		GetLogger().Log( "ModuleManager::OnMissionStart()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			m_Modules.Get(i).OnMissionStart();
		}
	}

	void OnMissionFinish()
	{
		GetLogger().Log( "ModuleManager::OnMissionFinish()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			m_Modules.Get(i).OnMissionFinish();
		}
	}

	void OnMissionLoaded()
	{
		GetLogger().Log( "ModuleManager::OnMissionLoaded()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			m_Modules.Get(i).OnMissionLoaded();
		}
	}

	void OnUpdate( float timeslice )
	{
		bool inputIsFocused = false;

		if ( GetGame().IsClient() || !GetGame().IsMultiplayer() )
		{
			ref Widget focusedWidget = GetFocus();
			if ( focusedWidget && focusedWidget.ClassName().Contains("EditBoxWidget") )
			{
				inputIsFocused = true;
			}
		}

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			Module module = m_Modules.Get(i);

			if ( !module.IsPreventingInput() && ( GetGame().IsClient() || !GetGame().IsMultiplayer() ) )
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

					bool localPress = input.LocalPress();
					bool localRelease = input.LocalRelease();
					bool localHold = input.LocalHold();
					bool localClick = input.LocalClick();
					bool localDoubleClick = input.LocalDoubleClick();

					bool canLocalPress = input.IsPressLimit();
					bool canLocalRelease = input.IsReleaseLimit();
					bool canLocalHold = input.IsHoldLimit();
					bool canLocalClick = input.IsClickLimit();
					bool canLocalDoubleClick = input.IsDoubleClickLimit();

					bool isLimited = canLocalPress || canLocalRelease || canLocalHold || canLocalClick || canLocalDoubleClick;

					if ( isLimited )
					{
						if ( canLocalPress && localPress )
						{
							GetGame().GameScript.CallFunctionParams( module, k_m_Binding.GetCallBackFunction(), NULL, new Param1< UAInput >( input ) );
						}
						
						if ( canLocalRelease && localRelease )
						{
							GetGame().GameScript.CallFunctionParams( module, k_m_Binding.GetCallBackFunction(), NULL, new Param1< UAInput >( input ) );
						}

						if ( canLocalHold && localHold )
						{
							GetGame().GameScript.CallFunctionParams( module, k_m_Binding.GetCallBackFunction(), NULL, new Param1< UAInput >( input ) );
						}

						if ( canLocalClick && localClick )
						{
							GetGame().GameScript.CallFunctionParams( module, k_m_Binding.GetCallBackFunction(), NULL, new Param1< UAInput >( input ) );
						}

						if ( canLocalDoubleClick && localDoubleClick )
						{
							GetGame().GameScript.CallFunctionParams( module, k_m_Binding.GetCallBackFunction(), NULL, new Param1< UAInput >( input ) );
						}
					} else
					{
						GetGame().GameScript.CallFunctionParams( module, k_m_Binding.GetCallBackFunction(), NULL, new Param1< UAInput >( input ) );
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