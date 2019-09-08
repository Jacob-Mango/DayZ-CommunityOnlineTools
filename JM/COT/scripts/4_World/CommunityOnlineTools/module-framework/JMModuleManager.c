class JMModuleManager
{
	protected autoptr array< ref JMModuleBase > m_Modules;
	protected autoptr array< ref JMRenderableModuleBase > m_EditorModules;

	void JMModuleManager()
	{
		GetLogger().Log( "JMModuleManager::JMModuleManager()", "JM_COT_ModuleFramework" );
	}

	void ~JMModuleManager()
	{
		GetLogger().Log( "JMModuleManager::~JMModuleManager()", "JM_COT_ModuleFramework" );
	}

	void RegisterModule( JMModuleBase module )
	{
		m_Modules.Insert( module );

		string message = "";
		
		if ( module.IsInherited( JMRenderableModuleBase ) )
		{
			m_EditorModules.Insert( JMRenderableModuleBase.Cast( module ) );
			message = " (type=JMRenderableModuleBase)";
		} else
		{
			message = " (type=JMModuleBase)";
		}

		GetLogger().Log( "Registered module " + module + message, "JM_COT_ModuleFramework" );
	}

	void RegisterModules()
	{
		GetLogger().Log( "JMModuleManager::RegisterModules()", "JM_COT_ModuleFramework" );

		m_Modules = new array< ref JMModuleBase >;
		m_EditorModules = new array< ref JMRenderableModuleBase >;

		RegisterModule( new COTModule );
	}

	void ReloadSettings()
	{
		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			m_Modules.Get(i).ReloadSettings();
		}
	}

	ref array< ref JMModuleBase > GetModules()
	{
		return m_Modules;
	}

	ref array< ref JMRenderableModuleBase > GetEditorModules()
	{
		return m_EditorModules;
	}

	void OnInit()
	{
		GetLogger().Log( "JMModuleManager::OnInit()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			m_Modules.Get(i).Init();
		}
	}

	void OnMissionStart()
	{
		GetLogger().Log( "JMModuleManager::OnMissionStart()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			m_Modules.Get(i).OnMissionStart();
		}
	}

	void OnMissionFinish()
	{
		GetLogger().Log( "JMModuleManager::OnMissionFinish()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			m_Modules.Get(i).OnMissionFinish();
		}
	}

	void OnMissionLoaded()
	{
		GetLogger().Log( "JMModuleManager::OnMissionLoaded()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			m_Modules.Get(i).OnMissionLoaded();
		}
	}

	
	void OnClientLogoutCancelled( PlayerBase player, PlayerIdentity identity )
	{
		GetLogger().Log( "JMModuleManager::OnClientLogoutCancelled()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			if ( m_Modules.Get(i).OnClientLogoutCancelled( player, identity ) )
			{
				break;
			}
		}
	}

	bool OnClientNew( out PlayerBase player, PlayerIdentity identity, vector pos, ParamsReadContext ctx )
	{
		GetLogger().Log( "JMModuleManager::OnClientNew()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			if ( m_Modules.Get(i).OnClientNew( player, identity, pos, ctx ) )
			{
				return true;
			}
		}

		return false;
	}

	void OnClientReady( PlayerBase player, PlayerIdentity identity )
	{
		GetLogger().Log( "JMModuleManager::OnClientReady()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			if ( m_Modules.Get(i).OnClientReady( player, identity ) )
			{
				break;
			}
		}
	}

	void OnClientPrepare( PlayerIdentity identity, out bool useDB, out vector pos, out float yaw, out int preloadTimeout )
	{
		GetLogger().Log( "JMModuleManager::OnClientPrepare()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			if ( m_Modules.Get(i).OnClientPrepare( identity, useDB, pos, yaw, preloadTimeout ) )
			{
				break;
			}
		}
	}

	void OnClientReconnect( PlayerBase player, PlayerIdentity identity )
	{
		GetLogger().Log( "JMModuleManager::OnClientReconnect()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			if ( m_Modules.Get(i).OnClientReconnect( player, identity ) )
			{
				break;
			}
		}
	}

	void OnClientRespawn( PlayerBase player, PlayerIdentity identity )
	{
		GetLogger().Log( "JMModuleManager::OnClientRespawn()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			if ( m_Modules.Get(i).OnClientRespawn( player, identity ) )
			{
				break;
			}
		}
	}

	void OnClientDisconnected( PlayerBase player, PlayerIdentity identity, int logoutTime, bool authFailed )
	{
		GetLogger().Log( "JMModuleManager::OnClientDisconnected()", "JM_COT_ModuleFramework" );

		for ( int i = 0; i < m_Modules.Count(); ++i)
		{
			if ( m_Modules.Get(i).OnClientDisconnected( player, identity, logoutTime, authFailed ) )
			{
				break;
			}
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
			JMModuleBase module = m_Modules.Get(i);

			if ( !module.IsPreventingInput() && ( GetGame().IsClient() || !GetGame().IsMultiplayer() ) )
			{
				for ( int kb = 0; kb < module.GetBindings().Count(); ++kb )
				{
					JMModuleBinding k_m_Binding = module.GetBindings().Get(kb);

					if ( GetCommunityOnlineToolsBase().IsOpen() || GetGame().GetUIManager().GetMenu() )
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

	ref JMModuleBase GetModule( typename module_Type )
	{
		for ( int i = 0; i < m_Modules.Count(); ++i )
		{
			ref JMModuleBase module = m_Modules.Get(i);

			if ( module.GetModuleType() == module_Type)
			{
				return module;
			}
		}

		return NULL;
	}

	ref JMModuleBase GetModuleByName( string module_name )
	{
		for ( int i = 0; i < m_Modules.Count(); ++i )
		{
			ref JMModuleBase module = m_Modules.Get( i );

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

ref JMModuleManager g_cot_ModuleManager;

static ref JMModuleManager GetModuleManager()
{
	return g_cot_ModuleManager;
}

static void CreateModuleManager()
{
	g_cot_ModuleManager = new JMModuleManager;
}

static void DestroyModuleManager()
{
	delete g_cot_ModuleManager;
}