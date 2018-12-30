int JM_COT_CURRENT_VERSION_MAJOR = 0;
int JM_COT_CURRENT_VERSION_MINOR = 7;
int JM_COT_CURRENT_VERSION_REVISION = 5;

class CommunityOnlineTools
{
    protected bool m_bLoaded;

    void CommunityOnlineTools()
    {
        Print("CommunityOnlineTools::CommunityOnlineTools");

        m_bLoaded = false;

        NewModuleManager();

        GetRPCManager().AddRPC( "COT", "CheckVersion", this, SingeplayerExecutionType.Server );
    }

    void CheckVersion( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param3< int, int, int > data;
        if ( !ctx.Read( data ) ) return;

        if( type == CallType.Server )
        {
            if ( data.param1 != JM_COT_CURRENT_VERSION_MAJOR )
            {
                Print( "" + sender.GetPlainId() + " is running a different major version of Community Online Tools." );
                return;
            }

            if ( data.param2 != JM_COT_CURRENT_VERSION_MINOR )
            {
                Print( "" + sender.GetPlainId() + " is running a different minor version of Community Online Tools." );
                return;
            }

            if ( data.param3 != JM_COT_CURRENT_VERSION_REVISION )
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

        GetRPCManager().SendRPC( "COT", "CheckVersion", new Param3< int, int, int >( JM_COT_CURRENT_VERSION_MAJOR, JM_COT_CURRENT_VERSION_MINOR, JM_COT_CURRENT_VERSION_REVISION ), true );
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