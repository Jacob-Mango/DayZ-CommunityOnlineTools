modded class MissionGameplay
{
	protected ref JMDebugMonitor m_CDebugMonitor;

	protected JMPlayerInstance m_OfflineInstance;

	// ------------------------------------------------------------
	// Constructor
	// ------------------------------------------------------------
	void MissionGameplay()
	{
		if ( !g_cotBase )
		{
			g_cotBase = new CommunityOnlineTools;
		}

		if ( IsMissionOffline() )
		{
			if ( g_Game.GetGameState() != DayZGameState.MAIN_MENU )
			{
				if ( GetPermissionsManager().OnClientConnected( NULL, m_OfflineInstance ) )
				{
					GetCommunityOnlineToolsBase().SetClient( m_OfflineInstance );
				}
			}
		}
	}

	// ------------------------------------------------------------
	// Deconstructor
	// ------------------------------------------------------------
	void ~MissionGameplay()
	{
		delete g_cotBase;
		
		GetPermissionsManager().OnClientDisconnected( JMConstants.OFFLINE_GUID, m_OfflineInstance );
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
		vector position = GetSpawnPoints().GetRandomElement();
		// position = "13931.9 0 13231.4";
		PlayerBase player = PlayerBase.Cast( GetGame().CreatePlayer( NULL, GetGame().CreateRandomPlayer(), position, 0, "NONE" ) );
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
		if ( IsMissionOffline() )
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
		if ( GetCommunityOnlineToolsBase().IsOpen() )
			return;

		super.ShowInventory();
	}

	// ------------------------------------------------------------
	// Override ShowChat
	// ------------------------------------------------------------
	override void ShowChat()
	{
		super.ShowChat();
	}
}
