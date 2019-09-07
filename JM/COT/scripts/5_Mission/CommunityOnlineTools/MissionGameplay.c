modded class MissionGameplay
{
	protected ref JMDebugMonitor m_CDebugMonitor;

	// ------------------------------------------------------------
	// Constructor
	// ------------------------------------------------------------
	void MissionGameplay()
	{
		GetLogger().Log( "MissionGameplay::MissionGameplay()", "JGetCommunityOnlineTools()_Mission" );
		
		if ( !g_cotBase )
		{
			g_cotBase = new CommunityOnlineTools;
		}
	}

	// ------------------------------------------------------------
	// Deconstructor
	// ------------------------------------------------------------
	void ~MissionGameplay()
	{
		GetLogger().Log( "MissionGameplay::~MissionGameplay()", "JGetCommunityOnlineTools()_Mission" );
	}

	// ------------------------------------------------------------
	// Override OnInit
	// ------------------------------------------------------------
	override void OnInit()
	{
		super.OnInit();
	}

	// ------------------------------------------------------------
	// COT OfflineMissionStart
	// ------------------------------------------------------------
	void OfflineMissionStart()
	{
		GetLogger().Log( "MissionGameplay::OfflineMissionStart()", "JGetCommunityOnlineTools()_Mission" );

		PlayerBase player = PlayerBase.Cast( GetGame().CreatePlayer( NULL, GetGame().CreateRandomPlayer(), GetSpawnPoints().GetRandomElement(), 0, "NONE" ) );
		GetGame().SelectPlayer( NULL, player );

		if ( player )
		{
			EntityAI item = NULL;
			
			item = player.GetInventory().CreateInInventory( "AviatorGlasses" );
			item = player.GetInventory().CreateInInventory( "MilitaryBeret_UN" );
			item = player.GetInventory().CreateInInventory( "M65Jacket_Black" );
			item = player.GetInventory().CreateInInventory( "TacticalGloves_Black" );
			item = player.GetInventory().CreateInInventory( "HunterPants_Autumn" );
			item = player.GetInventory().CreateInInventory( "MilitaryBoots_Black" );
			item = player.GetInventory().CreateInInventory( "AliceBag_Camo" );
			item = player.GetInventory().CreateInInventory( "Shovel" );
				
			JMPlayerInstance auPlayer = GetPermissionsManager().PlayerJoined( NULL );
			auPlayer.PlayerObject = player;
		}
	}

	// ------------------------------------------------------------
	// Override OnMissionStart
	// ------------------------------------------------------------
	override void OnMissionStart()
	{
		super.OnMissionStart();
		
		GetCommunityOnlineTools().OnStart();
		
		// If game is not multiplayer, add a default offline player
		if ( !GetGame().IsMultiplayer() )
		{
			OfflineMissionStart();
		}
	}

	// ------------------------------------------------------------
	// Override OnMissionStart
	// ------------------------------------------------------------
	override void OnMissionFinish()
	{
		GetCommunityOnlineTools().OnFinish();

		super.OnMissionFinish();

		if (m_CDebugMonitor)
			m_CDebugMonitor.Hide();
	}

	// ------------------------------------------------------------
	// Override OnUpdate
	// ------------------------------------------------------------
	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );

		if ( m_bLoaded )
		{
			UIScriptedMenu menu = m_UIManager.GetMenu();
			Input input = GetGame().GetInput();

			GetCommunityOnlineTools().OnUpdate( timeslice );

			// Disable openning radial menu
			if ( DISABLE_ALL_INPUT )
			{
				if( input.LocalPress("UAUIQuickbarRadialOpen", false) )
				{
					if ( GetUIManager().IsMenuOpen( MENU_RADIAL_QUICKBAR ) )
					{
						RadialQuickbarMenu.CloseMenu();
					}
				}
			}

			// Force the custom debug monitor to show instead
			if ( m_CDebugMonitor )
			{
				if ( GetCommunityOnlineToolsBase().IsOpen() )
				{
					m_CDebugMonitor.Hide();
				} else 
				{
					m_CDebugMonitor.Show();
				}
			}
		}
	}

	// ------------------------------------------------------------
	// Override CreateDebugMonitor
	// ------------------------------------------------------------
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
			m_CDebugMonitor = new JMDebugMonitor();
			m_CDebugMonitor.Init();
			m_CDebugMonitor.Show();
		}
	}

	// ------------------------------------------------------------
	// Override UpdateDebugMonitor
	// ------------------------------------------------------------
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

	// ------------------------------------------------------------
	// Override ShowInventory
	// ------------------------------------------------------------
	override void ShowInventory()
	{
		if ( DISABLE_ALL_INPUT ) return;

		super.ShowInventory();
	}

	// ------------------------------------------------------------
	// Override ShowChat
	// ------------------------------------------------------------
	override void ShowChat()
	{
		if ( DISABLE_ALL_INPUT ) return;

		super.ShowChat();
	}
}
