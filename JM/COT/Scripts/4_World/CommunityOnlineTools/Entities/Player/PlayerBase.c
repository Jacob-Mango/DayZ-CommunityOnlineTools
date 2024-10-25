enum JMInvisibilityType
{
	None,
	DisableSimulation,
	Interactive
}

modded class PlayerBase
{
#ifndef CF_MODULE_PERMISSIONS
	private JMPlayerInstance m_AuthenticatedPlayer;
#endif

	private bool m_COT_GodMode;
	private bool m_COT_GodMode_Preference;

	private int m_JMIsInvisible;
	private int m_JMIsInvisibleRemoteSynch;
	private int m_COT_Invisibility_Preference;
	protected ref Timer m_COT_InvisibilityUpdateTimer;
	private bool m_COT_WasSpeaking;

	private bool m_JMIsFrozen;
	private bool m_JMIsFrozenRemoteSynch;

	private vector m_JMLastPosition;
	private bool m_JMHasLastPosition;

	private bool m_JMHasUnlimitedAmmo;
	private bool m_JMHasUnlimitedStamina;
	
	private bool m_JMHasCustomScale;
	private float m_JMScaleValue;

	private bool m_JMHasAdminNVG;
	private bool m_JMHasAdminNVGRemoteSynch;

	private ref map<int, bool> m_COT_PlayerVars;
	private int m_COT_PlayerVarsBitmask;

	PlayerBase m_JM_SpectatedPlayer;
	vector m_JM_CameraPosition;
	private bool m_COT_EdgeTick;

	private bool m_COT_ReceiveDamageDealt;
	private bool m_COT_CannotBeTargetedByAI;
	private bool m_COT_CannotBeTargetedByAI_Preference;
	private bool m_COT_RemoveCollision;
	private bool m_COT_RemoveCollision_Preference;

	private bool m_COT_IsBeingKicked;
	private bool m_COT_IsLeavingFreeCam;

	//private Transport m_COT_TransportCache;
	//private int m_COT_TransportCache_CrewIndex;
	//private int m_COT_TransportCache_Seat;

	void PlayerBase()
	{
		if ( IsMissionOffline() )
		{
			GetStatEnergy().Set( GetStatEnergy().GetMax() );
			GetStatWater().Set( GetStatWater().GetMax() );
			COTSetGodMode(true);
			COTSetUnlimitedAmmo(true);
			COTSetUnlimitedStamina(true);
		}

		if (GetGame() && GetGame().IsClient())
		{
			SetEventMask(EntityEvent.POSTFRAME|EntityEvent.INIT);
		}

		m_COT_PlayerVars = new map<int, bool>;
	}

	
	void ~PlayerBase()
	{
		if (GetGame() && GetGame().IsClient())
		{
			ClearEventMask(EntityEvent.POSTFRAME|EntityEvent.INIT);
		}
	}

	override void Init()
	{
		/*
		Print( "TOUCH: " + EntityEvent.TOUCH );
		Print( "VISIBLE: " + EntityEvent.VISIBLE );
		Print( "NOTVISIBLE: " + EntityEvent.NOTVISIBLE );
		Print( "FRAME: " + EntityEvent.FRAME );
		Print( "POSTFRAME: " + EntityEvent.POSTFRAME );
		Print( "INIT: " + EntityEvent.INIT );
		Print( "JOINTBREAK: " + EntityEvent.JOINTBREAK );
		Print( "SIMULATE: " + EntityEvent.SIMULATE );
		Print( "POSTSIMULATE: " + EntityEvent.POSTSIMULATE );
		Print( "PHYSICSMOVE: " + EntityEvent.PHYSICSMOVE );
		Print( "CONTACT: " + EntityEvent.CONTACT );
		Print( "EXTRA: " + EntityEvent.EXTRA );
		Print( "ANIMEVENT: " + EntityEvent.ANIMEVENT );
		Print( "SOUNDEVENT: " + EntityEvent.SOUNDEVENT );
		Print( "PHYSICSSTEADY: " + EntityEvent.PHYSICSSTEADY );
		Print( "USER: " + EntityEvent.USER );
		Print( "ALL: " + EntityEvent.ALL );
		*/
		super.Init();

		int playerVarsBitmaskMaxValue = Math.Pow(2, EnumTools.GetEnumSize(JMPlayerVariables));
		RegisterNetSyncVariableInt("m_COT_PlayerVarsBitmask", 0, playerVarsBitmaskMaxValue);

		RegisterNetSyncVariableFloat("m_JMScaleValue", 0.1, 10.0);

#ifndef CF_MODULE_PERMISSIONS
		GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( Safe_SetAuthenticatedPlayer, 2000, false );
#endif

		m_JMHasLastPosition = false;
		m_JMLastPosition = "0 0 0";
	}

	void COT_UpdatePlayerVars(map<int, bool> playerVars)
	{
		playerVars[JMPlayerVariables.BLOODY_HANDS] = HasBloodyHands();
		playerVars[JMPlayerVariables.GODMODE] = COTHasGodMode();
		playerVars[JMPlayerVariables.FROZEN] = COTIsFrozen();
		playerVars[JMPlayerVariables.INVISIBILITY] = COTIsInvisible();
		playerVars[JMPlayerVariables.UNLIMITED_AMMO] = COTHasUnlimitedAmmo();
		playerVars[JMPlayerVariables.UNLIMITED_STAMINA] = COTHasUnlimitedStamina();
		playerVars[JMPlayerVariables.BROKEN_LEGS] = m_BrokenLegState != eBrokenLegs.NO_BROKEN_LEGS;
		playerVars[JMPlayerVariables.RECEIVE_DMG_DEALT] = COTGetReceiveDamageDealt();
		playerVars[JMPlayerVariables.CANNOT_BE_TARGETED_BY_AI] = COTGetCannotBeTargetedByAI();
		playerVars[JMPlayerVariables.REMOVE_COLLISION] = COTGetRemoveCollision();
		playerVars[JMPlayerVariables.ADMIN_NVG] = COTHasAdminNVG();
		playerVars[JMPlayerVariables.HAS_CUSTOM_SCALE] = COTHasCustomScale();
		playerVars[JMPlayerVariables.INVISIBILITY_INTERACTIVE] = COTIsInvisible(JMInvisibilityType.Interactive);
	}

	void COT_SynchPlayerVars()
	{
		COT_UpdatePlayerVars(m_COT_PlayerVars);

		m_COT_PlayerVarsBitmask = 0;

		foreach (int value, bool enabled: m_COT_PlayerVars)
		{
			if (enabled)
				m_COT_PlayerVarsBitmask |= value;
		}

		SetSynchDirty();
	}

	override void CommandHandler( float pDt, int pCurrentCommandID, bool pCurrentCommandFinished )	
	{
		bool skip;

		if (GetGame().IsServer())
		{
			if ((m_JM_SpectatedPlayer || m_JM_CameraPosition != vector.Zero) && m_JMIsInvisible)
				skip = true;
		}
		else if (CurrentActiveCamera && m_JMIsInvisible)
		{
			skip = true;
		}

		if (!skip)
			super.CommandHandler( pDt, pCurrentCommandID, pCurrentCommandFinished );
	}

	protected bool m_COT_TempDisableOnSelectPlayer;

	void COT_TempDisableOnSelectPlayer(bool disable = true)
	{
		m_COT_TempDisableOnSelectPlayer = disable;

	#ifndef SERVER
		auto mission = MissionBaseWorld.Cast(GetGame().GetMission());
		if (mission)
			mission.COT_TempDisableOnSelectPlayer(disable);
	#endif
	}

	override void OnSelectPlayer()
	{
	#ifdef DIAG
		auto trace = CF_Trace_0(this);
		PrintFormat("Is player selected? %1", m_PlayerSelected.ToString());
	#endif

	#ifdef DIAG
		PrintFormat("Is OnSelectPlayer temporarily disabled? %1", m_COT_TempDisableOnSelectPlayer.ToString());
	#endif

		if (m_COT_TempDisableOnSelectPlayer)
			COT_TempDisableOnSelectPlayer(false);
		else
			super.OnSelectPlayer();

		//COT_ResumeVehicleCommand();

#ifndef SERVER
		if (GetGame().GetPlayer() == this && (GetCommunityOnlineToolsBase().IsOpen() || GetCOTWindowManager().HasAnyActive()))
			GetGame().GetUIManager().ShowUICursor(true);
#endif
	}

	override void OnConnect()
	{
		super.OnConnect();

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(COTOnConnect);
	}

	void COTOnConnect()
	{
		m_COT_GodMode = !GetAllowDamage();
		COT_SynchPlayerVars();
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		bool enabled;
		for (int i = 0; i < EnumTools.GetEnumSize(JMPlayerVariables); i++)
		{
			int value = EnumTools.GetEnumValue(JMPlayerVariables, i);
			enabled = (m_COT_PlayerVarsBitmask & value) == value;
			switch (value)
			{
				case JMPlayerVariables.GODMODE:
				#ifdef DIAG_DEVELOPER
					if (m_COT_GodMode != enabled)
						PrintFormat("%1 COT GodMode %2", this, enabled);
				#endif
					m_COT_GodMode = enabled;
					break;

				case JMPlayerVariables.FROZEN:
					m_JMIsFrozenRemoteSynch = enabled;
					break;

				case JMPlayerVariables.INVISIBILITY:
					if (enabled)
						m_JMIsInvisibleRemoteSynch = JMInvisibilityType.DisableSimulation;
					else
						m_JMIsInvisibleRemoteSynch = JMInvisibilityType.None;
					break;

				case JMPlayerVariables.INVISIBILITY_INTERACTIVE:
					if (enabled)
						m_JMIsInvisibleRemoteSynch = JMInvisibilityType.Interactive;
					break;

				case JMPlayerVariables.UNLIMITED_AMMO:
				#ifdef DIAG_DEVELOPER
					if (m_JMHasUnlimitedAmmo != enabled)
						PrintFormat("%1 COT Unlimited Ammo %2", this, enabled);
				#endif
					m_JMHasUnlimitedAmmo = enabled;
					break;

				case JMPlayerVariables.UNLIMITED_STAMINA:
				#ifdef DIAG_DEVELOPER
					if (m_JMHasUnlimitedStamina != enabled)
						PrintFormat("%1 COT Unlimited Stamina %2", this, enabled);
				#endif
					m_JMHasUnlimitedStamina = enabled;
					break;

				case JMPlayerVariables.ADMIN_NVG:
					m_JMHasAdminNVGRemoteSynch = enabled;
					break;

				case JMPlayerVariables.HAS_CUSTOM_SCALE:
				#ifdef DIAG_DEVELOPER
					if (m_JMHasCustomScale != enabled)
						PrintFormat("%1 COT Custom Scale %2", this, enabled);
				#endif
					m_JMHasCustomScale = enabled;
					break;
			}
		}

		if ( m_JMIsInvisibleRemoteSynch != m_JMIsInvisible )
		{
		#ifdef DIAG_DEVELOPER
			PrintFormat("%1 COT Invisibility %2", this, m_JMIsInvisibleRemoteSynch);
		#endif

			m_JMIsInvisible = m_JMIsInvisibleRemoteSynch;

			COTUpdateInvisibility();

			SetInvisible( m_JMIsInvisible );

			if (!m_COT_InvisibilityUpdateTimer)
				m_COT_InvisibilityUpdateTimer = new Timer(CALL_CATEGORY_GUI);

			if (m_JMIsInvisible && !m_COT_InvisibilityUpdateTimer.IsRunning())
				m_COT_InvisibilityUpdateTimer.Run(0.1, this, "COTUpdateInvisibility", NULL, true);
			else if (!m_JMIsInvisible && m_COT_InvisibilityUpdateTimer.IsRunning())
				m_COT_InvisibilityUpdateTimer.Stop();
		}

		if ( m_JMIsFrozenRemoteSynch != m_JMIsFrozen )
		{
		#ifdef DIAG_DEVELOPER
			PrintFormat("%1 COT Frozen %2", this, m_JMIsInvisibleRemoteSynch);
		#endif

			m_JMIsFrozen = m_JMIsFrozenRemoteSynch;

			HumanInputController hic = GetInputController();
			if ( hic )
				hic.SetDisabled( m_JMIsFrozen );
		}

		if ( m_JMHasAdminNVGRemoteSynch != m_JMHasAdminNVG )
		{
		#ifdef DIAG_DEVELOPER
			PrintFormat("%1 COT Admin NVG %2", this, m_JMHasAdminNVGRemoteSynch);
		#endif

			m_JMHasAdminNVG = m_JMHasAdminNVGRemoteSynch;

			// idk why or how it works.
			// This makes no fucking sense.
			// it just works. Kill me. -LT
			if (m_JMHasAdminNVG)
			{
				AddActiveNV(JMNVTypes.NV_COT_ON);
				RemoveActiveNV(JMNVTypes.NV_COT_OFF);
			}
			else
			{
				AddActiveNV(JMNVTypes.NV_COT_OFF);
				RemoveActiveNV(JMNVTypes.NV_COT_ON);
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(RemoveActiveNV, 1000, false, JMNVTypes.NV_COT_OFF);
			}
		}
	}

	protected void COTUpdateInvisibility()
	{
		//! @note Not a controlled player = this player object on other clients
		//! @note WARNING: Do NOT use Camera.GetCurrentCamera() after leaving spectator cam, crashes game
		JMSpectatorCamera spectatorCam;
		if (m_JMIsInvisible == JMInvisibilityType.DisableSimulation && ((!Class.CastTo(spectatorCam, CurrentActiveCamera) && !IsControlledPlayer()) || (spectatorCam && spectatorCam.SelectedTarget != this)))
		{
			if (PhysicsIsSolid())
			{
				//! Set physics non-solid so there is no blocking "ghost"
				//! Needed on client because of disabling simulation (below),
				//! setting on server is not enough in this specific case
				PhysicsSetSolid(false);
			}

			PlayerBase spectatedPlayer;  //! The identity check is for determining if we're spectating AI, in which case we won't disable sim
			if (!GetIsSimulationDisabled() && (!spectatorCam || !Class.CastTo(spectatedPlayer, spectatorCam.SelectedTarget) || spectatedPlayer.GetIdentity()))
			{
				//! Disable simulation to disable position update on client, footstep sounds, etc.
				DisableSimulation(true);
			}
		}
		else if (GetIsSimulationDisabled() && IsAlive())
		{
			DisableSimulation(false);
			Update();
		}
	}

	//! @note after disabling simulation, this no longer gets called, just keep that in mind
#ifndef SERVER
	override void EOnPostFrame( IEntity other, int extra )
	{
		if ( !m_JMIsInvisible )
		{
			if (m_JMHasCustomScale && m_JMScaleValue > 0.0)
				SetScale(m_JMScaleValue);
		}
		else
		{
			SetInvisible( true );
		}
	}
#endif

	void COT_SimulationDisabled_OnFrame(float timeSlice)
	{
		//! Verbatim copy of the client part of vanilla PlayerBase::EOnFrame,
		//! this is a stand-in to update HUD etc in case of true invis mode which disables simulation
		if (GetInstanceType() == DayZPlayerInstanceType.INSTANCETYPE_CLIENT)
		{
			#ifndef NO_GUI
			m_Hud.Update(timeSlice);
			
			if (IsControlledPlayer() && m_EffectWidgets && m_EffectWidgets.IsAnyEffectRunning())
			{
				m_EffectWidgets.Update(timeSlice);
			}
			#endif

			if (m_UndergroundHandler)
				m_UndergroundHandler.Tick(timeSlice);
			
			//! DayZ 1.26+
			if (m_UndergroundBunkerHandler)
				m_UndergroundBunkerHandler.Tick(timeSlice);
		}
	}

	override bool CanBeTargetedByAI( EntityAI ai )
	{
		if (!super.CanBeTargetedByAI( ai ))
			return false;

		if (m_COT_CannotBeTargetedByAI)
			return false;

		return true;
	}

	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;

		PlayerBase sourcePlayer;
		if (source && Class.CastTo(sourcePlayer, source.GetHierarchyRootPlayer()) && sourcePlayer != this && sourcePlayer.COTGetReceiveDamageDealt())
		{
			sourcePlayer.ProcessDirectDamage(damageType, source, dmgZone, ammo, modelPos, speedCoef);
			return false;
		}

		return true;
	}

	bool HasLastPosition()
	{
		return m_JMHasLastPosition;
	}

	vector GetLastPosition()
	{
		return m_JMLastPosition;
	}

	void SetLastPosition(bool force = false)
	{
		if ( GetGame().IsServer() && (force || (!m_JM_SpectatedPlayer && m_JM_CameraPosition == vector.Zero)))
		{
			vector trans[4];
			GetTransform( trans );
			m_JMLastPosition = trans[3];

			m_JMHasLastPosition = true;
		}
	}

	void SetWorldPosition(vector position)
	{
		Object parent;
		if (Class.CastTo(parent, GetParent()))
		{
			Transport transport;
			if (Class.CastTo(transport, parent))
			{
				if (transport.CrewMemberIndex(this) != -1)
				{
					CommunityOnlineToolsBase.PlaceOnSurfaceAtPosition(transport, position);
					
					return;
				}
			}

			SetPosition(parent.WorldToModel(position));
		}
		else
		{
			SetPosition(position);
		}

		if (m_JM_CameraPosition != vector.Zero || m_JM_SpectatedPlayer)
			SetLastPosition(true);
	}

#ifndef CF_MODULE_PERMISSIONS
	JMPlayerInstance GetAuthenticatedPlayer()
	{
		if ( m_AuthenticatedPlayer )
			return m_AuthenticatedPlayer;

		if ( !GetIdentity() ) // Could be AI
			return NULL;

		if ( !m_AuthenticatedPlayer )
		{
			if ( IsMissionOffline() )
			{
				m_AuthenticatedPlayer = GetPermissionsManager().GetClientPlayer();
			} else
			{
				m_AuthenticatedPlayer = GetPermissionsManager().GetPlayer( GetIdentity().GetId() );
			}
		}

		if ( Assert_Null( m_AuthenticatedPlayer ) )
			return NULL;
		
		m_AuthenticatedPlayer.PlayerObject = this;
		return m_AuthenticatedPlayer;
	}

	private void Safe_SetAuthenticatedPlayer()
	{
		if ( m_AuthenticatedPlayer )
			return;

		if ( !GetIdentity() ) // Could be AI
			return;

		if ( !m_AuthenticatedPlayer )
		{
			if ( IsMissionOffline() )
			{
				m_AuthenticatedPlayer = GetPermissionsManager().GetClientPlayer();
			} else
			{
				m_AuthenticatedPlayer = GetPermissionsManager().GetPlayer( GetIdentity().GetId() );
			}
		}

		if ( m_AuthenticatedPlayer )		
			m_AuthenticatedPlayer.PlayerObject = this;
		else
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( Safe_SetAuthenticatedPlayer, 2000, false );
	}

	override string FormatSteamWebhook()
	{
		if ( !GetAuthenticatedPlayer() )
		{
			return super.FormatSteamWebhook() + " (WARNING)";
		}

		return GetAuthenticatedPlayer().FormatSteamWebhook();
	}
#endif

	bool COTHasGodMode()
	{
		return m_COT_GodMode;
	}

	bool COTIsFrozen()
	{
		return m_JMIsFrozen;
	}

	bool COTIsInvisible(int type = 0)
	{
		if (!type)
			return m_JMIsInvisible;

		return m_JMIsInvisible == type;
	}

	bool COTHasUnlimitedAmmo()
	{
		return m_JMHasUnlimitedAmmo;
	}

	bool COTHasUnlimitedStamina()
	{
		return m_JMHasUnlimitedStamina;
	}

	bool COTHasAdminNVG()
	{
		return m_JMHasAdminNVG;
	}

	bool COTHasCustomScale()
	{
		return m_JMHasCustomScale;
	}

	void COTSetGodMode( bool mode, bool preference = true )
	{
		if ( GetGame().IsServer() )
		{
			if (preference)
				m_COT_GodMode_Preference = mode;
			else if (mode)
				m_COT_GodMode_Preference = !GetAllowDamage();

			SetAllowDamage( !mode );

			m_COT_GodMode = mode;
			#ifdef SERVER
			COT_SynchPlayerVars();
			#endif
		}
		else
		{
			Error("COTSetGodMode cannot be called on client");
		}
	}

	override void ProcessHandDamage(float delta_time, HumanMovementState pState)
	{
        if ( !COTHasGodMode() )
			super.ProcessHandDamage(delta_time, pState);
	}
	
	override void ProcessFeetDamageServer(int pUserInt)
	{
        if ( !COTHasGodMode() )
			super.ProcessFeetDamageServer(pUserInt);
	}

	void COTSetFreeze( bool mode )
	{
		if ( GetGame().IsServer() )
		{
			m_JMIsFrozen = mode;
			m_JMIsFrozenRemoteSynch = mode;

			#ifdef SERVER
			COT_SynchPlayerVars();
			#endif

			HumanInputController hic = GetInputController();
			if ( hic )
				hic.SetDisabled( m_JMIsFrozen );
		}
	}

	void COTSetInvisibility( int mode, bool preference = true )
	{
		if ( GetGame().IsServer() )
		{
			if (preference)
				m_COT_Invisibility_Preference = mode;

			COTSetInvisibilityOnly(mode);

			if (mode || preference || !m_COT_RemoveCollision_Preference)
				COTSetRemoveCollision(mode, preference);

			if (mode || preference || !m_COT_CannotBeTargetedByAI_Preference)
				COTSetCannotBeTargetedByAI(mode, preference);
		}
	}

	void COTSetInvisibilityOnly(int mode)
	{
		if (m_JMIsInvisible != mode)
		{
			m_JMIsInvisible = mode;
			m_JMIsInvisibleRemoteSynch = mode;

			#ifdef SERVER
			COT_SynchPlayerVars();
			#endif
		}
	}

	void COTSetCannotBeTargetedByAI( bool mode, bool preference = true )
	{
		if (preference)
			m_COT_CannotBeTargetedByAI_Preference = mode;

		m_COT_CannotBeTargetedByAI = mode;
	}

	bool COTGetCannotBeTargetedByAI()
	{
		return m_COT_CannotBeTargetedByAI;
	}

	void COTSetRemoveCollision( bool mode, bool preference = true )
	{
		if (preference)
			m_COT_RemoveCollision_Preference = mode;

		m_COT_RemoveCollision = mode;
		PhysicsSetSolid(!mode);
	}

	bool COTGetRemoveCollision()
	{
		return m_COT_RemoveCollision;
	}

	void COTSetReceiveDamageDealt(bool state)
	{
		m_COT_ReceiveDamageDealt = state;
	}

	bool COTGetReceiveDamageDealt()
	{
		return m_COT_ReceiveDamageDealt;
	}

	void COTSetUnlimitedAmmo( bool mode )
	{
		if ( GetGame().IsServer() )
		{
			m_JMHasUnlimitedAmmo = mode;

			#ifdef SERVER
			COT_SynchPlayerVars();
			#endif
		}
	}

	void COTSetUnlimitedStamina( bool mode )
	{
		if ( GetGame().IsServer() )
		{
			m_JMHasUnlimitedStamina = mode;

			#ifdef SERVER
			COT_SynchPlayerVars();
			#endif
		}
	}

	void COTSetAdminNVG( bool mode )
	{
		if ( GetGame().IsServer() )
		{
			m_JMHasAdminNVG = mode;

			#ifdef SERVER
			COT_SynchPlayerVars();
			#endif
		}
	}

	void COTSetScale( float value )
	{
		if ( GetGame().IsServer() )
		{
			m_JMHasCustomScale = value != 1;
			m_JMScaleValue = value;
			SetScale(value);

			#ifdef SERVER
			COT_SynchPlayerVars();
			#endif
		}
	}

	void COTRemoveAllDiseases()
	{
		if ( GetGame().IsServer() )
		{
			GetModifiersManager().DeactivateAllModifiers();
			
			if ( m_AgentPool )
				m_AgentPool.RemoveAllAgents();

			if ( IsUnconscious() )
				DayZPlayerSyncJunctures.SendPlayerUnconsciousness(this, false);
		}
	}

	void COTResetItemWetness()
	{
		array< EntityAI > items = new array< EntityAI >;
		GetInventory().EnumerateInventory( InventoryTraversalType.PREORDER, items );
		ItemBase item;
		foreach ( EntityAI entity: items )
		{
			if ( Class.CastTo( item, entity ) )
				item.SetWet( 0 );
		}
	}

	void COT_RemoveAllItems()
	{
		array<EntityAI> entities = {};
		GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, entities);

		ItemBase item;
		foreach (EntityAI entity: entities)
		{
			if (entity != this && Class.CastTo(item, entity))
			{
				item.DeleteSafe();
			}
		}
	}

	void COTUpdateSpectatorPosition()
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "COTUpdateSpectatorPosition");
#endif

		vector position;
		bool freeCam;
		bool spectate;

		if (m_JM_CameraPosition != vector.Zero)
		{
			position = m_JM_CameraPosition;
			freeCam = true;
		}
		else if (m_JM_SpectatedPlayer)
		{
			position = m_JM_SpectatedPlayer.GetPosition();
			spectate = true;
		}
		else
		{
			return;
		}

		if (freeCam && vector.DistanceSq(m_JMLastPosition, position) <= 22500)
		{
			//! If we get close (within 150 m) of original position, place player at original position

			if (COTIsInvisible() && !m_COT_Invisibility_Preference)
				COTSetInvisibility( JMInvisibilityType.None, false );

			if (GetPosition()[1] < GetGame().SurfaceY(position[0], position[2]))
			{
				PhysicsEnableGravity( true );
				SetPosition(m_JMLastPosition);
			}
		}
		else
		{
			vector dir = vector.Direction(GetPosition(), position);

			if (dir.LengthSq() >= 1000000 || m_COT_EdgeTick)
			{
				//! Move to edge of network bubble of target (this is where an ESPer could "see" the admin position,
				//! frozen in time) before moving directly under target.
				//! Randomize the distance a bit to not make it too obvious.
				position = position - dir.Normalized() * Math.RandomFloat(981, 990);
				position[1] = GetGame().SurfaceY(position[0], position[2]);

				if (m_COT_EdgeTick)
					m_COT_EdgeTick--;
				else
					m_COT_EdgeTick = 3;
			}
			else
			{
				position[1] = GetGame().SurfaceRoadY3D(position[0], position[1], position[2], RoadSurfaceDetection.CLOSEST) - 3.0;
			}

			if (!COTHasGodMode())
				COTSetGodMode( true, false );

			if (!COTIsInvisible(JMInvisibilityType.DisableSimulation))
				COTSetInvisibility( JMInvisibilityType.DisableSimulation, false );

			PhysicsEnableGravity( false );
			SetPosition( position );
		}
	}

	void COTResetSpectator()
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "COTResetSpectator");
#endif

		if (!m_COT_Invisibility_Preference)
			COTSetInvisibility( JMInvisibilityType.None, false );
		else
			COTSetInvisibility( m_COT_Invisibility_Preference, false );

		PhysicsEnableGravity( true );

		if ( HasLastPosition() )
			SetPosition( GetLastPosition() );

		if (!m_COT_GodMode_Preference)
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(COTSetGodMode, 34, false, false, false);
	}

	void COTSetIsBeingKicked(bool state)
	{
		m_COT_IsBeingKicked = state;
	}

	bool COTIsBeingKicked()
	{
		return m_COT_IsBeingKicked;
	}

	bool COT_IsLeavingFreeCam()
	{
		return m_COT_IsLeavingFreeCam;
	}
	
	override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);

		//AddAction(COT_QuickActionBuild, InputActionMap);
		//AddAction(COT_QuickActionDismantle, InputActionMap);
	}

	ActionBase COT_StartAction(typename actionType, ActionTarget target, ItemBase mainItem = null)
	{
		ActionManagerClient mngr_client;
		if (!Class.CastTo(mngr_client, GetActionManager()))
			return null;

		ActionBase action = mngr_client.GetAction(actionType);

		if (!action.Can(this, target, mainItem))
			return null;

		if (!mainItem)
			mainItem = GetItemInHands();

		mngr_client.PerformActionStart(action, target, mainItem);

		return action;
	}

	ActionBase COT_StartActionObject(typename actionType, Object target, ItemBase mainItem = null)
	{
		return COT_StartAction(actionType, new ActionTarget(target, null, -1, vector.Zero, -1.0), mainItem);
	}

	void COT_RememberVehicle()
	{
	#ifdef DIAG
		auto trace = CF_Trace_0(this);
		PrintFormat("GetCommand_Move? %1", GetCommand_Move().ToString());
		PrintFormat("GetCommand_Vehicle? %1", GetCommand_Vehicle().ToString());
		//PrintFormat("m_COT_TransportCache? %1", m_COT_TransportCache.ToString());
	#endif

		//HumanCommandVehicle hcv = GetCommand_Vehicle();

		//if (hcv && !hcv.IsGettingOut())
		//{
			//m_COT_TransportCache = hcv.GetTransport();
			//m_COT_TransportCache_CrewIndex = m_COT_TransportCache.CrewMemberIndex(this);
			//m_COT_TransportCache_Seat = m_COT_TransportCache.GetSeatAnimationType(m_COT_TransportCache_CrewIndex);
		//}
	}

	bool COT_GetOutVehicle()
	{
		auto hcv = GetCommand_Vehicle();
		if (hcv)
		{
			if (hcv.GetTransport())
			{
				if (!hcv.IsGettingOut())
				{
					if (!COT_StartActionObject(ActionGetOutTransport, null))
					{
						if (!COT_StartActionObject(ActionOpenCarDoors, null))
						{
							COTCreateLocalAdminNotification(new StringLocaliser("Couldn't get out of vehicle because a door is blocked. Please exit the vehicle first before leaving freecam."));

							CurrentActiveCamera.SetPosition(GetPosition() + "0 1.5 0");
							return true;
						}
					}

					if (!m_COT_IsLeavingFreeCam)
					{
						m_COT_IsLeavingFreeCam = true;
						COTCreateLocalAdminNotification(new StringLocaliser("Leaving freecam..."));
					}
				}

				return true;
			}
		}

		return false;
	}

	override void OnCommandVehicleFinish()
	{
		super.OnCommandVehicleFinish();

		if (CurrentActiveCamera && m_COT_IsLeavingFreeCam)
		{
			auto mission = MissionBaseWorld.Cast(GetGame().GetMission());
			if (mission)
				mission.COT_LeaveFreeCam();
			m_COT_IsLeavingFreeCam = false;
		}
	}

	void COT_ResumeVehicleCommand()
	{
	#ifdef DIAG
		auto trace = CF_Trace_0(this);
		PrintFormat("GetCommand_Vehicle? %1", GetCommand_Vehicle().ToString());
		//PrintFormat("m_COT_TransportCache? %1", m_COT_TransportCache.ToString());
	#endif

		auto hcv = GetCommand_Vehicle();
		if (hcv)
		{
			hcv.GetOutVehicle();  //! Needed to end vehicle cmd
			Transport trans = hcv.GetTransport();
			trans.CrewGetOut(trans.CrewMemberIndex(this));  //! Needed to unlink from seat
			UnlinkFromLocalSpace();  //! DOESN'T WORK, still in vehicle space after getting out
			OnCommandVehicleFinish();

			CarScript car;
			if (Class.CastTo(car, trans))
			{
				GetDayZGame().GetBacklit().OnLeaveCar();
				if (GetGame().IsServer())
				{
					car.ForceUpdateLightsStart();
					car.ForceUpdateLightsEnd();
				}
			}
		}
		//else if (m_COT_TransportCache)
		//{
			//StartCommand_Vehicle(m_COT_TransportCache, m_COT_TransportCache_CrewIndex, m_COT_TransportCache_Seat, IsUnconscious());
			//m_COT_TransportCache = null;
		//}
	}
};
