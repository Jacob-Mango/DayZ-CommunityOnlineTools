modded class MissionGameplay
{
	protected ref JMDebugMonitor m_CDebugMonitor;  //! Legacy, not used, only kept for compatibility with 3rd party mods

	protected JMPlayerInstance m_OfflineInstance;

	void MissionGameplay()
	{
		if ( !g_cotBase )
			g_cotBase = new CommunityOnlineTools;
	}

	void ~MissionGameplay()
	{
		g_cotBase = null;
	}

	override void ResetGUI()
	{
	#ifdef DIAG
		auto trace = CF_Trace_0(this);
		PrintFormat("Is GUI reset disabled? %1", m_COT_TempDisableOnSelectPlayer.ToString());
	#endif

		if (m_COT_TempDisableOnSelectPlayer)
			m_COT_TempDisableOnSelectPlayer = false;
		else
			super.ResetGUI();  //! This crashes client when called more than once because it's called from PlayerBase::OnSelectPlayer when leaving freecam
	}

	void OfflineMissionStart()
	{
		vector position = GetSpawnPoints().GetRandomElement();
		// position = "13931.9 0 13231.4";
		PlayerBase player = PlayerBase.Cast( g_Game.CreatePlayer( NULL, g_Game.CreateRandomPlayer(), position, 0, "NONE" ) );
		g_Game.SelectPlayer( NULL, player );

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
	
			if ( g_Game.GetGameState() != DayZGameState.MAIN_MENU )
			{
				if ( GetPermissionsManager().OnClientConnected( NULL, m_OfflineInstance ) )
				{
					m_OfflineInstance.PlayerObject = GetPlayer();

					GetCommunityOnlineToolsBase().SetClient( m_OfflineInstance );
				}
			}
		}
	}

	override void OnMissionFinish()
	{
		GetCOTWindowManager().DestroyAllWindows();

		GetCommunityOnlineTools().OnFinish();

		super.OnMissionFinish();
		
		if ( IsMissionOffline() )
			GetPermissionsManager().OnClientDisconnected( JMConstants.OFFLINE_GUID, m_OfflineInstance );
	}

	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );

		if ( m_bLoaded )
		{
			UIScriptedMenu menu = m_UIManager.GetMenu();
			Input input = g_Game.GetInput();

			GetCommunityOnlineTools().OnUpdate( timeslice );

			if ( m_DebugMonitor )
			{
				if ( GetCommunityOnlineToolsBase().IsOpen() )
					m_DebugMonitor.Hide();
				else 
					m_DebugMonitor.Show();
			}

			PlayerBase player;
			if (Class.CastTo(player, g_Game.GetPlayer()) && player.COTIsInvisible(JMInvisibilityType.DisableSimulation))
			{
				//! Since PlayerBase::EOnFrame will no longer be called by the engine if simulation is disabled,
				//! call stand-in from here so HUD gets updated
				player.COT_SimulationDisabled_OnFrame(timeslice);
			}
		}
	}

	override void ShowInventory()
	{
		if ( GetCommunityOnlineToolsBase().IsOpen() || GetCOTWindowManager().HasAnyActive() )
			return;

		super.ShowInventory();
	}

	override void Pause()
	{
		if ( GetCommunityOnlineToolsBase().IsOpen() || GetCOTWindowManager().HasAnyActive() )
			return;

		super.Pause();
	}

	override protected void HandleMapToggleByKeyboardShortcut(Man player)
	{
		if ( GetCommunityOnlineToolsBase().IsOpen() || GetCOTWindowManager().HasAnyActive() )
			return;

		super.HandleMapToggleByKeyboardShortcut(player);
	}

	override void COT_LeaveFreeCam()
	{
		JMCameraModule module;
		CF_Modules<JMCameraModule>.Get(module);
		module.Leave();
	}
}
