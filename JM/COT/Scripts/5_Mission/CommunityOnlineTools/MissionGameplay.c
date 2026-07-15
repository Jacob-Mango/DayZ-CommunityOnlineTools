modded class MissionGameplay
{
	protected ref JMDebugMonitor m_CDebugMonitor;  //! Legacy, not used, only kept for compatibility with 3rd party mods

	protected JMPlayerInstance m_OfflineInstance;

	protected bool m_COT_DebugMonitorWasVisible;

	void MissionGameplay()
	{
		if ( !g_cotBase )
			g_cotBase = new CommunityOnlineTools;

		JMScriptInvokers.COT_ON_OPEN.Insert(COT_OnOpen);
	}

	void ~MissionGameplay()
	{
		g_cotBase = null;

		if (g_Game)
			JMScriptInvokers.COT_ON_OPEN.Remove(COT_OnOpen);
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
		vector begPos = position + "0 1000 0";
		vector endPos = position - "0 1000 0";
		PhxInteractionLayers layers;
		layers |= PhxInteractionLayers.TERRAIN;
		layers |= PhxInteractionLayers.ROADWAY;
		Object hitObj;
		vector hitPos;
		vector hitNorm;
		float hitFrac;
		if (DayZPhysics.RayCastBullet(begPos, endPos, layers, player, hitObj, hitPos, hitNorm, hitFrac))
			player.SetPosition(hitPos);

		if ( player )
		{
			EntityAI item;
			EntityAI knife;
			Weapon_Base weapon;
			EntityAI shovel;
			EntityAI hatchet;
			
		#ifdef EXPANSIONMODCORE
			string loadout = "AdminLoadout";

			if (FileExist(EXPANSION_LOADOUT_FOLDER + loadout + ".json"))
			{
				ExpansionHumanLoadout.Apply(player, loadout);

				item = player.FindAttachmentBySlotName("Hips");
				if (!item)
					item = player;

				//! Find knife/weapon/shovel/hatchet (if any)
				array<EntityAI> children = {};
				player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, children);
				foreach (EntityAI child: children)
				{
					if (!weapon && Class.CastTo(weapon, child))
						continue;

					if (!shovel && child.IsKindOf("Hatchet"))
					{
						hatchet = child;
						continue;
					}

					if (!shovel && child.IsKindOf("Shovel") || child.IsKindOf("FieldShovel"))
					{
						shovel = child;
						continue;
					}

					if (!knife)
					{
						TStringArray inventorySlot = {};
						child.ConfigGetTextArray("inventorySlot", inventorySlot);
						foreach (string slotName: inventorySlot)
						{
							slotName.ToLower();
							if (slotName == "knife")
							{
								knife = child;
								continue;
							}
						}
					}

					if (knife && weapon && shovel && hatchet)
						break;
				}
			}
		#endif

			if (!item)
			{
				//! No loadout applied, spawn default loadout
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
				knife = item.GetInventory().CreateInInventory( "CombatKnife" );
				item.GetInventory().CreateInInventory( "PlateCarrierHolster" );
				item.GetInventory().CreateInInventory( "Canteen" );
				Class.CastTo(weapon, item.GetInventory().CreateInInventory( "Magnum" ));
				shovel = player.GetInventory().CreateInInventory( "Shovel" );
				hatchet = player.GetInventory().CreateInInventory( "Hatchet" );
			}

			if (knife)
				player.SetQuickBarEntityShortcut(knife, 0);

			if (weapon)
			{
				weapon.SpawnAmmo();
				player.SetQuickBarEntityShortcut(weapon, 1);
			}

			if (shovel)
				player.SetQuickBarEntityShortcut(shovel, 2);

			if (hatchet)
				player.SetQuickBarEntityShortcut(hatchet, 3);
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

			PlayerBase player;
			if (Class.CastTo(player, g_Game.GetPlayer()) && player.COTIsInvisible(JMInvisibilityType.DisableSimulation))
			{
				//! Since PlayerBase::EOnFrame will no longer be called by the engine if simulation is disabled,
				//! call stand-in from here so HUD gets updated
				player.COT_SimulationDisabled_OnFrame(timeslice);
			}
		}
	}

	void COT_OnOpen(bool isOpen)
	{
		if (m_DebugMonitor)
		{
			if (isOpen)
			{
				m_COT_DebugMonitorWasVisible = m_DebugMonitor.IsVisible();
				m_DebugMonitor.Hide();
			}
			else if (m_COT_DebugMonitorWasVisible)
			{
				m_DebugMonitor.Show();
			}
		}
	}

	override void HideDebugMonitor()
	{
		super.HideDebugMonitor();

		if (!m_DebugMonitor || !m_DebugMonitor.IsVisible())
			m_COT_DebugMonitorWasVisible = false;
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
