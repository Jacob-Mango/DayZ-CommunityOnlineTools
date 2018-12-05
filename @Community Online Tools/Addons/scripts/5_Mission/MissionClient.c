modded class MissionGameplay
{
    protected ref CommunityOnlineTools m_Tool;

    protected ref CustomDebugMonitor m_CDebugMonitor;

    void MissionGameplay()
    {
        Print( "MissionGameplay::MissionGameplay()" );
        
        m_Tool = new CommunityOnlineTools();
    }

    void ~MissionGameplay()
    {
        Print( "MissionGameplay::~MissionGameplay()" );
    }

    override void OnInit()
    {
        super.OnInit();
    }

    override void OnMissionStart()
    {
        super.OnMissionStart();
    
        m_Tool.OnStart();
    }

    override void OnMissionFinish()
    {
        m_Tool.OnFinish();

        super.OnMissionFinish();

		if (m_CDebugMonitor)
			m_CDebugMonitor.Hide();
    }

    override void OnUpdate( float timeslice )
    {
        super.OnUpdate( timeslice );

        UIScriptedMenu menu = m_UIManager.GetMenu();
        Input input = GetGame().GetInput();

        m_Tool.OnUpdate( timeslice );

        if ( DISABLE_ALL_INPUT )
        {
            if( input.GetActionDown(UAUIQuickbarRadialOpen, false) )
            {
                if ( GetUIManager().IsMenuOpen( MENU_RADIAL_QUICKBAR ) )
                {
                    RadialQuickbarMenu.CloseMenu();
                }
            }
        }

        if ( m_CDebugMonitor )
        {
            if ( COTMenuOpen )
            {
                m_CDebugMonitor.Hide();
            } else 
            {
                m_CDebugMonitor.Show();
            }
        }
    }


	override void CreateDebugMonitor()
	{
        super.CreateDebugMonitor();

        if ( m_DebugMonitor )
        {
            m_DebugMonitor.Hide();
            delete m_DebugMonitor;
        }

		if (!m_CDebugMonitor)
		{
			m_CDebugMonitor = new CustomDebugMonitor();
			m_CDebugMonitor.Init();
            m_CDebugMonitor.Show();
		}
	}

    override void UpdateDebugMonitor()
    {
		if (!m_CDebugMonitor) return;
		
		PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );
		if (player)
		{
			DebugMonitorValues values = player.GetDebugMonitorValues();
			if (values)
			{
				m_CDebugMonitor.SetHealth(values.GetHealth());
				m_CDebugMonitor.SetBlood(values.GetBlood());
				m_CDebugMonitor.SetLastDamage(values.GetLastDamage());
				m_CDebugMonitor.SetPosition(player.GetPosition());
			}
		}
    }

    override void ShowInventory()
    {
        if ( DISABLE_ALL_INPUT ) return;

        super.ShowInventory();
    }

    override void ShowChat()
    {
        if ( DISABLE_ALL_INPUT ) return;

        super.ShowChat();
    }
}
