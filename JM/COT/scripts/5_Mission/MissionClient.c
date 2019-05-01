modded class MissionGameplay
{
	protected ref CommunityOnlineTools m_COT;
	protected ref PermissionsFramework m_PF;

	protected ref CustomDebugMonitor m_CDebugMonitor;

	// ------------------------------------------------------------
	// Constructor
	// ------------------------------------------------------------
	void MissionGameplay()
	{
		Print( "MissionGameplay::MissionGameplay()" );
		
		m_COT = new ref CommunityOnlineTools;
		m_PF = new ref PermissionsFramework;
	}

	// ------------------------------------------------------------
	// Deconstructor
	// ------------------------------------------------------------
	void ~MissionGameplay()
	{
		Print( "MissionGameplay::~MissionGameplay()" );

		delete m_PF;
		delete m_COT;
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
				
			AuthPlayer auPlayer = GetPermissionsManager().PlayerJoined( NULL );
			auPlayer.PlayerObject = player;
		}
	}

	// ------------------------------------------------------------
	// Override OnMissionStart
	// ------------------------------------------------------------
	override void OnMissionStart()
	{
		super.OnMissionStart();
		
		m_PF.OnStart();
		m_COT.OnStart();
		
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
		m_PF.OnFinish();

		m_COT.OnFinish();

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

		UIScriptedMenu menu = m_UIManager.GetMenu();
		Input input = GetGame().GetInput();

		m_COT.OnUpdate( timeslice );

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
			if ( COTMenuOpen )
			{
				m_CDebugMonitor.Hide();
			} else 
			{
				m_CDebugMonitor.Show();
			}
		}

		m_PF.Update( timeslice );
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
			m_CDebugMonitor = new CustomDebugMonitor();
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
