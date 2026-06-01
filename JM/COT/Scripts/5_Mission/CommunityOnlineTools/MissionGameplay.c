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

		JsonDataContaminatedAreas effectAreaData = EffectAreaLoader.GetData();
		if (effectAreaData)
		{
			foreach (JsonDataContaminatedArea area: effectAreaData.Areas)
			{
				vector areaPos = Vector(area.Data.Pos[0], 0, area.Data.Pos[2]);
				if (Math.IsPointInCircle(areaPos, area.Data.Radius, position))
				{
					position = MiscGameplayFunctions.GetClosestSafePos(position, effectAreaData.SafePositions);
					break;
				}
			}
		}

		string characterType = g_Game.CreateRandomPlayer();
		string preferredCharacterType = g_Game.GetMenuDefaultCharacterData().GetCharacterType();

		if (g_Game.ListAvailableCharacters().Find(preferredCharacterType) > -1)
			characterType = preferredCharacterType;

		PlayerBase player = PlayerBase.Cast( g_Game.CreatePlayer( NULL, characterType, position, 0, "NONE" ) );
		g_Game.SelectPlayer( NULL, player );

		//! Having to determine surface yet again is needed because returned surface height can be different when
		//! player character is there vs not there
		auto surfParams = new SurfaceDetectionParameters();
		surfParams.type = SurfaceDetectionType.Roadway;
		surfParams.syncMode = UseObjectsMode.Wait;
		surfParams.rsd = RoadSurfaceDetection.UNDER;
		auto surfResult = new SurfaceDetectionResult();
		//! Using a high Y for checking deals with corner cases like Livonia bunker (Dambog) or Namalsk A3
		surfParams.position = Vector(position[0], 1000, position[2]);
		g_Game.GetSurface(surfParams, surfResult);
		position[1] = surfResult.height;
		player.SetPosition(position);

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
			player.SetQuickBarEntityShortcut(item.GetInventory().CreateInInventory( "CombatKnife" ), 0);
			item.GetInventory().CreateInInventory( "PlateCarrierHolster" );
			item.GetInventory().CreateInInventory( "Canteen" );
			item = item.GetInventory().CreateInInventory( "Magnum" );
			Weapon_Base.Cast(item).SpawnAmmo();
			player.SetQuickBarEntityShortcut(item, 1);
			player.SetQuickBarEntityShortcut(player.GetInventory().CreateInInventory( "Shovel" ), 2);
			player.SetQuickBarEntityShortcut(player.GetInventory().CreateInInventory( "Hatchet" ), 3);
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
