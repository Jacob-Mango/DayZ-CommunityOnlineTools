class CommunityOnlineTools
{
    protected bool m_bLoaded;
    protected string m_Version;
    protected int m_MajorVersion;
    protected int m_MinorVersion;
    protected int m_BuildVersion;

    void CommunityOnlineTools()
    {
        Print("CommunityOnlineTools::CommunityOnlineTools");

        m_bLoaded = false;

        string versionPath;
        GetGame().ConfigGetText( "CfgMods CommunityOnlineTools version", versionPath );
        Print("VersionPath: " + versionPath);
        FileHandle file_handle = OpenFile( versionPath, FileMode.READ );

        string line_content;

		while ( FGets( file_handle, line_content ) > 0 )
		{
            m_Version = line_content;
            break;
		}

        Print( "Community Online Tools is running on version " + m_Version );

        CloseFile( file_handle );

        array<string> values = new array<string>;
        m_Version.Split(".", values);

        if (values.Count() == 3)
        {
            m_MajorVersion = values[0].ToInt();
            m_MinorVersion = values[1].ToInt();
            m_BuildVersion = values[2].ToInt();

            Print( "Major: " + m_MajorVersion + " Minor: " + m_MinorVersion + " Build: " + m_BuildVersion );
        }

        NewModuleManager();

        GetRPCManager().AddRPC( "COT", "CheckVersion", this, SingeplayerExecutionType.Server );
    }

    void CheckVersion( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param3< int, int, int > data;
        if ( !ctx.Read( data ) ) return;

        if( type == CallType.Server )
        {
            Print( "" + sender.GetPlainId() + " is loading in with version " + data.param1 + "." + data.param2 + "." + data.param3 );

            if ( data.param1 != m_MajorVersion )
            {
                Print( "" + sender.GetPlainId() + " is running a different major version of Community Online Tools." );
                return;
            }

            if ( data.param2 != m_MinorVersion )
            {
                Print( "" + sender.GetPlainId() + " is running a different minor version of Community Online Tools." );
                return;
            }

            if ( data.param3 != m_BuildVersion )
            {
                Print( "" + sender.GetPlainId() + " is running a different revision of Community Online Tools." );       
                return;
            }
        }
    }

    void RegisterModules()
    {
        GetModuleManager().RegisterModule( new PlayerModule );
        GetModuleManager().RegisterModule( new ObjectModule );
        // GetModuleManager().RegisterModule( new MapEditorModule );
        GetModuleManager().RegisterModule( new TeleportModule );
        GetModuleManager().RegisterModule( new CameraTool );
        GetModuleManager().RegisterModule( new GameModule );
        GetModuleManager().RegisterModule( new WeatherModule );
		GetModuleManager().RegisterModule( new COTMapModule );
		GetModuleManager().RegisterModule( new ESPModule );
    }

    void ~CommunityOnlineTools()
    {
        Print("CommunityOnlineTools::~CommunityOnlineTools");
    }
    
    void OnStart()
    {
        GetModuleManager().RegisterModules();

        RegisterModules();

        GetModuleManager().OnInit();
        
        GetModuleManager().ReloadSettings();

        GetModuleManager().OnMissionStart();
    }

    void OnFinish()
    {
        GetModuleManager().OnMissionFinish();

        NewModuleManager();
    }

    void OnLoaded()
    {
        GetModuleManager().OnMissionLoaded();

        GetRPCManager().SendRPC( "COT", "CheckVersion", new Param3< int, int, int >( m_MajorVersion, m_MinorVersion, m_BuildVersion ), true );
    }

    void OnUpdate( float timeslice )
    {
        if( !m_bLoaded && !GetDayZGame().IsLoading() )
        {
            m_bLoaded = true;
            OnLoaded();
        } else {
            GetModuleManager().OnUpdate( timeslice );
        }
    }
}