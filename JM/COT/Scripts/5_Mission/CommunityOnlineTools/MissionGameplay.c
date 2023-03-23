modded class MissionGameplay
{
	protected ref JMDebugMonitor m_CDebugMonitor;

	protected JMPlayerInstance m_OfflineInstance;

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

	void ~MissionGameplay()
	{
		delete g_cotBase;
	}

	void OfflineMissionStart()
	{
		vector position = GetSpawnPoints().GetRandomElement();
		// position = "13931.9 0 13231.4";
		PlayerBase player = PlayerBase.Cast( GetGame().CreatePlayer( NULL, GetGame().CreateRandomPlayer(), position, 0, "NONE" ) );
		GetGame().SelectPlayer( NULL, player );

		if ( player )
		{
			EntityAI item = NULL;
			
			//item = player.GetInventory().CreateInInventory( "AirborneMask" );
			item = player.GetInventory().CreateInInventory( "Mich2001Helmet" );
			item.GetInventory().CreateInInventory( "NVGoggles" );
			item.GetInventory().CreateInInventory( "UniversalLight" );
			item.GetInventory().CreateInInventory( "Battery9V" );
			item.GetInventory().CreateInInventory( "Battery9V" );
			item = player.GetInventory().CreateInInventory( "TacticalGloves_Black" );
			item = player.GetInventory().CreateInInventory( "TTsKOJacket_Camo" );
			item = player.GetInventory().CreateInInventory( "TTSKOPants" );
			item = player.GetInventory().CreateInInventory( "TTSKOBoots" );
			item = player.GetInventory().CreateInInventory( "AliceBag_Camo" );
			item = player.GetInventory().CreateInInventory( "MilitaryBelt" );
			item.GetInventory().CreateInInventory( "NylonKnifeSheath" );
			item.GetInventory().CreateInInventory( "CombatKnife" );
			item.GetInventory().CreateInInventory( "PlateCarrierHolster" );
			item.GetInventory().CreateInInventory( "Magnum" );
			item.GetInventory().CreateInInventory( "Canteen" );
			item = player.GetInventory().CreateInInventory( "Shovel" );
			item = player.GetInventory().CreateInInventory( "Hatchet" );
		}
	}

	override void OnInit()
	{
		super.OnInit();

		JMESPModule espModule;
		if (CF_Modules<JMESPModule>.Get(espModule))
			espModule.CreateCanvas();
	}

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

	override void OnMissionFinish()
	{
		GetCommunityOnlineTools().OnFinish();

		super.OnMissionFinish();

		if ( m_CDebugMonitor )
			m_CDebugMonitor.Hide();
		
		if ( IsMissionOffline() )
		{
			GetPermissionsManager().OnClientDisconnected( JMConstants.OFFLINE_GUID, m_OfflineInstance );
		}
	}

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
		if ( GetCommunityOnlineToolsBase().IsOpen() || GetCOTWindowManager().Count() > 0 )
			return;

		super.ShowInventory();
	}

	override void Pause()
	{
		if ( GetCommunityOnlineToolsBase().IsOpen() || GetCOTWindowManager().Count() > 0 )
			return;

		super.Pause();
	}

	override protected void HandleMapToggleByKeyboardShortcut(Man player)
	{
		if ( GetCommunityOnlineToolsBase().IsOpen() || GetCOTWindowManager().Count() > 0 )
			return;

		super.HandleMapToggleByKeyboardShortcut(player);
	}
};

