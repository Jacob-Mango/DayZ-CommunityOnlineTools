class ModuleManager
{
    protected const int CLICK_TIME            = 200; //ms
    protected const int HOLD_CLICK_TIME_MIN    = 200; //ms
    protected const int DOUBLE_CLICK_TIME    = 300; //ms

    protected ref array< string > m_ModuleFolders;

    protected ref array< ref Module > m_Modules;
    protected ref array< ref EditorModule > m_EditorModules;
    protected ref array< ref MouseButtonInfo > m_MouseButtons;

    protected ref array< ScriptModule > m_ScriptModules;
    protected ScriptModule m_ParentScriptModule;

    void ModuleManager()
    {
        Print( "ModuleManager::ModuleManager()" );

        m_ModuleFolders = new ref array< string >;

        // m_ModuleFolders.Insert("COT/Scripts/Modules");

        m_MouseButtons = new ref array< ref MouseButtonInfo >;
        m_MouseButtons.Insert( new MouseButtonInfo( MouseState.LEFT ) );
        m_MouseButtons.Insert( new MouseButtonInfo( MouseState.RIGHT ) );
        m_MouseButtons.Insert( new MouseButtonInfo( MouseState.MIDDLE ) );

        // GetUApi().RegisterGroup( COT_INPUT_GROUP, "Community Online Tools" );
    }

    void ~ModuleManager()
    {
        Print( "ModuleManager::~ModuleManager()" );

        ReleaseModules();

        delete m_ModuleFolders;
        delete m_MouseButtons;
    }

    void ReleaseModules()
    {
        /*
        for ( int j = 0; j < m_ScriptModules.Count(); j++ )
        {
            m_ScriptModules.Get( j ).Release();
        }

        delete m_Modules;
        delete m_EditorModules;
        delete m_ScriptModules;
        */
    }

    private bool IsValidModule( string path, string name, FileAttr attributes )
    {
        Print( "Found: " + path + name + " as a " + FileAttributeToString( attributes ) );

        if ( ! (attributes & FileAttr.DIRECTORY ) ) return false;

        if ( name == "" ) return false;

        return true;
    }

    private void LoadModule( string path, string name )
    {
        ScriptModule script = ScriptModule.LoadScript( m_ParentScriptModule, path + name + "/module.c", true );
        if ( script )
        {
            Param p = new Param;
            script.CallFunctionParams( NULL, "RegisterModules", p, new Param1< ModuleManager >( this ) );

            m_ScriptModules.Insert( script );
        }
    }

    void RegisterModule( Module module )
    {
        m_Modules.Insert( module );

        string message = "";
        if ( module.IsInherited( EditorModule ) )
        {
            m_EditorModules.Insert( module );
            message = " as a EditorModule.";
        } else
        {
            message = " as a Module.";
        }

        Print( "Registered module " + module + message );
    }

    private void RegisterModulesByPath( string path )
    {
        Print( "ModuleManager::RegisterModulesByPath()" );
        
        int index = 0;
        string module = "";
        FileAttr oFileAttr = FileAttr.INVALID;
        FindFileHandle oFileHandle = FindFile( path + "*", module, oFileAttr, FindFileFlags.DIRECTORIES );

        if ( module != "" )
        {
            if ( IsValidModule( path, module, oFileAttr ) )
            {
                LoadModule( path, module );
                index++;
            }

            while (FindNextFile(oFileHandle, module, oFileAttr))
            {
                if ( IsValidModule( path, module, oFileAttr ) )
                {
                    LoadModule( path, module );
                    index++;
                }
            }
        }
    }

    void RegisterModules()
    {
        Print( "ModuleManager::RegisterModules()" );

        m_Modules = new ref array< ref Module >;
        m_EditorModules = new ref array< ref EditorModule >;
        m_ScriptModules = new ref array< ScriptModule >;

        m_ParentScriptModule = GetGame().GetMission().MissionScript;

        RegisterModule( new COTModule );

        for ( int i = 0; i < m_ModuleFolders.Count(); i++ )
        {
            // RegisterModulesByPath(m_ModuleFolders.Get(i));
        }
    }

    void ReloadModules()
    {
        ReleaseModules();
        RegisterModules();
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
        Print( "ModuleManager::OnInit()" );

        for ( int i = 0; i < m_Modules.Count(); ++i)
        {
            m_Modules.Get(i).Init();
        }

        //GetUApi().PresetSelect( 0 );

        //GetUApi().Export();
    }

    void OnMissionStart()
    {
        Print( "ModuleManager::OnMissionStart()" );

        for ( int i = 0; i < m_Modules.Count(); ++i)
        {
            m_Modules.Get(i).OnMissionStart();
        }
    }

    void OnMissionFinish()
    {
        Print( "ModuleManager::OnMissionFinish()" );

        for ( int i = 0; i < m_Modules.Count(); ++i)
        {
            m_Modules.Get(i).OnMissionFinish();
        }
    }

    void OnMissionLoaded()
    {
        Print( "ModuleManager::OnMissionLoaded()" );

        for ( int i = 0; i < m_Modules.Count(); ++i)
        {
            m_Modules.Get(i).OnMissionLoaded();
        }
    }

    void OnUpdate( float timeslice )
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return; 

        for ( int i = 0; i < m_Modules.Count(); ++i)
        {
            Module module = m_Modules.Get(i);

            if ( !module.IsPreventingInput() )
            {
                for ( int kb = 0; kb < module.GetBindings().Count(); ++kb )
                {
                    KeyMouseBinding k_m_Binding = module.GetBindings().Get(kb);

                    if ( COTMenuOpen )
                    {
                        if ( !k_m_Binding.CanBeUsedInMenu() || DISABLE_ALL_INPUT)
                        {
                            continue;
                        }
                    }

                    UAInput input = GetUApi().GetInputByName( k_m_Binding.GetUAInputName() );

                    int action = k_m_Binding.GetActionType();

                    if ( action & KeyMouseActionType.PRESS && input.LocalPress() )
                    {
                        GetGame().GameScript.CallFunction( GetModule( k_m_Binding.GetObject() ), k_m_Binding.GetCallBackFunction(), NULL, 0 );
                    }

                    if ( action & KeyMouseActionType.RELEASE && input.LocalRelease() )
                    {
                        GetGame().GameScript.CallFunction( GetModule( k_m_Binding.GetObject() ), k_m_Binding.GetCallBackFunction(), NULL, 0 );
                    }

                    if ( action & KeyMouseActionType.HOLD && input.LocalHold() )
                    {
                        GetGame().GameScript.CallFunction( GetModule( k_m_Binding.GetObject() ), k_m_Binding.GetCallBackFunction(), NULL, 0 );
                    }

                    if ( action & KeyMouseActionType.DOUBLECLICK && input.LocalDoubleClick() )
                    {
                        GetGame().GameScript.CallFunction( GetModule( k_m_Binding.GetObject() ), k_m_Binding.GetCallBackFunction(), NULL, 0 );
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

    MouseButtonInfo GetMouseButtonInfo( int button )
    {
        for ( int i = 0; i < m_MouseButtons.Count(); ++i )
        {
            MouseButtonInfo info = m_MouseButtons.Get(i);

            if ( info.GetButtonID() == button )
            {
                return info;
            }
        }
        return NULL;
    }
}

ref ModuleManager g_com_ModuleManager;

ref ModuleManager GetModuleManager()
{
    if( !g_com_ModuleManager )
    {
        g_com_ModuleManager = new ref ModuleManager();
    }

    return g_com_ModuleManager;
}

ref ModuleManager NewModuleManager()
{
    if ( g_com_ModuleManager )
    {
        delete g_com_ModuleManager;
    }

    g_com_ModuleManager = new ref ModuleManager();

    return g_com_ModuleManager;
}