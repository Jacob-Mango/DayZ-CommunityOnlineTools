enum JMInvisibilityType
{
	None,
	DisableSimulation,
	Interactive
}

modded class PlayerBase
{
#ifndef CF_MODULE_PERMISSIONS
	protected JMPlayerInstance m_AuthenticatedPlayer;
#endif

	protected bool m_COT_GodMode;
	protected bool m_COT_GodMode_Preference;
	protected int m_JMIsInvisible;
	protected int m_JMIsInvisibleRemoteSynch;
	protected int m_COT_Invisibility_Preference;
	protected ref Timer m_COT_InvisibilityUpdateTimer;
	protected bool m_COT_WasSpeaking;
	protected bool m_JMIsFrozen;
	protected bool m_JMIsFrozenRemoteSynch;
	protected bool m_JMIsRagdoll;
	protected bool m_JMIsRagdollRemoteSynch;
	protected vector m_JMLastPosition;
	protected bool m_JMHasLastPosition;
	protected bool m_JMHasUnlimitedAmmo;
	protected bool m_JMHasUnlimitedStamina;
	protected bool m_JMHasCustomScale;
	protected float m_JMScaleValue;
	protected bool m_JMHasAdminNVG;
	protected bool m_JMHasAdminNVGRemoteSynch;
	protected ref map<int, bool> m_COT_PlayerVars;
	protected int m_COT_PlayerVarsBitmask;
	Object m_JM_SpectatedObject;
	vector m_JM_CameraPosition;
	protected bool m_COT_SpectateStatsSaved;
	protected float m_COT_SavedHealth;
	protected float m_COT_SavedBlood;
	protected float m_COT_SavedShock;
	protected float m_COT_SavedEnergy;
	protected float m_COT_SavedWater;
	protected float m_COT_SavedStamina;
	protected float m_COT_SavedHeatComfort;
	protected bool m_COT_EdgeTick;
	protected bool m_COT_ReceiveDamageDealt;
	protected bool m_COT_CannotBeTargetedByAI;
	protected bool m_COT_CannotBeTargetedByAI_Preference;
	protected bool m_COT_RemoveCollision;
	protected bool m_COT_RemoveCollision_Preference;
	protected bool m_COT_IsBeingKicked;
	protected bool m_COT_IsLeavingFreeCam;
	protected bool m_COT_TempDisableOnSelectPlayer;

	// ========================================================================
	//  COT kill-tracking hooks
	//
	//  Two events fire on the entity by the engine:
	//    EEHitBy  -- every damage hit from a real source; we record body-part
	//                and approximate hit/miss by checking if the source was
	//                a real player vs the world
	//    EEKilled  -- final death; killer is the entity that landed the
	//                killing blow.  Body-part + distance captured here.
	//
	//  The static methods on JMAntiCheatKillHook (defined in this same file)
	//  forward the events to the anti-cheat module.  A static forwarder
	//  avoids a cross-file class reference that the Mission module's compile
	//  scope has to keep clean.
	// ========================================================================

	//! State of the last hit this player took, captured while it is still
	//! accurate. EEKilled can fire minutes later (bleed-out, shock) by which
	//! time the weapon is holstered, the explosive deleted, and both players
	//! moved -- so a distance computed there is meaningless.
	protected ref JMAntiCheatKillEvent m_COT_LastHitSnapshot;
	protected int m_COT_LastHitTime;

	//private Transport m_COT_TransportCache;
	//private int m_COT_TransportCache_CrewIndex;
	//private int m_COT_TransportCache_Seat;

	void PlayerBase()
	{
		if (g_Game && g_Game.IsClient())
			SetEventMask(EntityEvent.POSTFRAME|EntityEvent.INIT);

		m_COT_PlayerVars = new map<int, bool>;
	}

	void ~PlayerBase()
	{
		if (g_Game && g_Game.IsClient())
		{
			ClearEventMask(EntityEvent.POSTFRAME|EntityEvent.INIT);
		}
	}

	vector GetLastPosition()
	{
		return m_JMLastPosition;
	}

	bool HasLastPosition()
	{
		return m_JMHasLastPosition;
	}

	void SetLastPosition(bool force = false)
	{
		if ( g_Game.IsServer() && (force || (!m_JM_SpectatedObject && m_JM_CameraPosition == vector.Zero)))
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
					COT_Base.PlaceOnSurfaceAtPosition(transport, position);
					
					return;
				}
			}

			SetPosition(parent.WorldToModel(position));
		}
		else
		{
			SetPosition(position);
		}

		if (m_JM_CameraPosition != vector.Zero || m_JM_SpectatedObject)
			SetLastPosition(true);

		//! Every path into here is somebody being moved rather than moving:
		//! an admin teleport, a spectate exit, a vehicle unstuck. Tell the
		//! anti-cheat so the next position sample is discarded instead of
		//! being read as a several-kilometre jump.
		COT_ReportEngineMove();
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
		g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( Safe_SetAuthenticatedPlayer, 2000, false );
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
		playerVars[JMPlayerVariables.UNCONSCIOUS] = IsUnconscious();
		playerVars[JMPlayerVariables.RAGDOLL] = COTIsRagdoll();

		bool isSick = HasDisease();

	#ifndef DAYZ_1_29
		//! 1.30+
		//! Deal with modifiers that do not call IncreaseDiseaseCount (might be a DayZ bug)
		if ( !isSick && GetModifiersManager() )
		{
			isSick |= GetModifiersManager().IsModifierActive( eModifiers.MDF_PARTICLES_BREATH ) || GetModifiersManager().IsModifierActive( eModifiers.MDF_PARTICLES_EYES );
			isSick |= GetModifiersManager().IsModifierActive( eModifiers.MDF_SANDSTORM_EXPOSURE_STATIC ) || GetModifiersManager().IsModifierActive( eModifiers.MDF_SANDSTORM_EXPOSURE_DYNAMIC );
		}
	#endif

		playerVars[JMPlayerVariables.SICK] = isSick;
		playerVars[JMPlayerVariables.BLEEDING] = IsBleeding();
		playerVars[JMPlayerVariables.DEAD] = !IsAlive();
	}

	void COT_SynchPlayerVars()
	{
	#ifdef SERVER
		COT_UpdatePlayerVars(m_COT_PlayerVars);

		m_COT_PlayerVarsBitmask = 0;

		foreach (int value, bool enabled: m_COT_PlayerVars)
		{
			if (enabled)
				m_COT_PlayerVarsBitmask |= value;
		}
	#endif

		SetSynchDirty();
	}

	override void CommandHandler( float pDt, int pCurrentCommandID, bool pCurrentCommandFinished )	
	{
		bool skip;

		if (g_Game.IsServer())
		{
			if ((m_JM_SpectatedObject || m_JM_CameraPosition != vector.Zero) && m_JMIsInvisible)
				skip = true;
		}
		else if (CurrentActiveCamera && m_JMIsInvisible)
		{
			skip = true;
		}

		if (!skip)
		{
			super.CommandHandler( pDt, pCurrentCommandID, pCurrentCommandFinished );
		}
		else
		{
			//! Only handle inventory

			GetDayZPlayerInventory().HandleInventory(pDt);
			GetHumanInventory().Update(pDt);

			UpdateDelete();
		}
	}

	void COT_TempDisableOnSelectPlayer(bool disable = true)
	{
		m_COT_TempDisableOnSelectPlayer = disable;

	#ifndef SERVER
		auto mission = MissionBaseWorld.Cast(g_Game.GetMission());
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
		if (g_Game.GetPlayer() == this && (GetCommunityOnlineToolsBase().IsOpen() || GetCOTWindowManager().HasAnyActive()))
			g_Game.GetUIManager().ShowUICursor(true);
#endif
	}

	override void OnConnect()
	{
		super.OnConnect();

		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Call(COTOnConnect);
	}

	void COTOnConnect()
	{
		m_COT_GodMode = !GetAllowDamage();
		COT_SynchPlayerVars();
	}

	void COT_DecodePlayerVars()
	{
		bool enabled;
		for (int i = 0; i < EnumTools.GetEnumSize(JMPlayerVariables); i++)
		{
			int value = EnumTools.GetEnumValue(JMPlayerVariables, i);
			enabled = (m_COT_PlayerVarsBitmask & value) == value;
			switch (value)
			{
				case JMPlayerVariables.GODMODE:
				#ifdef DIAG
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
				#ifdef DIAG
					if (m_JMHasUnlimitedAmmo != enabled)
						PrintFormat("%1 COT Unlimited Ammo %2", this, enabled);
				#endif
					m_JMHasUnlimitedAmmo = enabled;
					break;

				case JMPlayerVariables.UNLIMITED_STAMINA:
				#ifdef DIAG
					if (m_JMHasUnlimitedStamina != enabled)
						PrintFormat("%1 COT Unlimited Stamina %2", this, enabled);
				#endif
					m_JMHasUnlimitedStamina = enabled;
					break;

				case JMPlayerVariables.ADMIN_NVG:
					m_JMHasAdminNVGRemoteSynch = enabled;
					break;

				case JMPlayerVariables.HAS_CUSTOM_SCALE:
				#ifdef DIAG
					if (m_JMHasCustomScale != enabled)
						PrintFormat("%1 COT Custom Scale %2", this, enabled);
				#endif
					m_JMHasCustomScale = enabled;
					break;

				case JMPlayerVariables.RAGDOLL:
					m_JMIsRagdollRemoteSynch = enabled;
					break;
			}
		}
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		if (g_Game.IsMultiplayer())
			COT_DecodePlayerVars();

		if ( m_JMIsInvisibleRemoteSynch != m_JMIsInvisible )
		{
		#ifdef DIAG
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
		#ifdef DIAG
			PrintFormat("%1 COT Frozen %2", this, m_JMIsFrozenRemoteSynch);
		#endif

			m_JMIsFrozen = m_JMIsFrozenRemoteSynch;

			HumanInputController hic = GetInputController();
			if ( hic )
			{
				hic.SetDisabled( m_JMIsFrozen );
				DisableSimulation(m_JMIsFrozen);
			}
		}

		if ( m_JMHasAdminNVGRemoteSynch != m_JMHasAdminNVG )
		{
		#ifdef DIAG
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
				g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(RemoveActiveNV, 1000, false, JMNVTypes.NV_COT_OFF);
			}
		}

	#ifndef DAYZ_1_29
		//! COTSetRagdoll() only calls PhysicsSetRagdoll() where it runs - the
		//! server. That correctly ragdolls the entity everywhere else (other
		//! clients see it via normal object/physics replication), but the
		//! OWNING client simulates its own player locally and never got told,
		//! same as Frozen/AdminNVG above needing their own client-side apply
		//! here rather than relying on physics replication alone.
		if ( m_JMIsRagdollRemoteSynch != m_JMIsRagdoll )
		{
			m_JMIsRagdoll = m_JMIsRagdollRemoteSynch;

			PhysicsSetRagdoll( m_JMIsRagdoll );
		}
	#endif
	}

	//! @note only way to prevent drawing occluding effects when admin NV is enabled and wearing NVGoggles that are NOT active at the same time
	//! is to not include the latter in the list of optics to draw. It's far simpler to remove them from the list after they've been added than
	//! trying to deal with overriding CachedEquipmentStorage in 1.29
	protected override array<InventoryItem> OnDrawOptics2D()
	{
		array<InventoryItem> optics = super.OnDrawOptics2D();

		if (optics && (m_ActiveNVTypes.Find(JMNVTypes.NV_COT_ON) > -1 || m_ActiveNVTypes.Find(JMNVTypes.NV_COT_OFF) > -1))
		{
			foreach (int i, InventoryItem optic: optics)
			{
				if (optic.IsInherited(NVGoggles))
				{
					optics.Remove(i);  //! Prevent drawing NVG occluding effects
					break;
				}
			}
		}

		return optics;
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

	override bool IsRestrained()
	{
		if (super.IsRestrained())
			return true;

		if (m_JMIsFrozen)
		{
			//! Cannot check whether COT is active or accessing player has permission on MP server, since that info is not available
			//! inside IsRestrained in that case (accessing player is not necessarily controlling this character, so we would need a GUID).
			//! Necessary checks for admin functionality are done on client only.
			//! Just always return true on MP server if frozen, will pretend the character is restrained and enable correct functionality
			//! together with the client checks (client is not able to fake it even if client checks were tampered with or removed).
			if (g_Game.IsDedicatedServer())
				return true;

			if (IsControlledPlayer())
			{
				//! If this instance of a character *is* controlled by the client player (ie. *not* a remote player),
				//! pretend restrained if COT is *not* active _or_ the client player *doesn't* have AccessInventory permission
				//! (thus preventing access for this character to their own inventory)
				if (!GetCommunityOnlineToolsBase().IsActive() || !JMPermissions.Has(JMConstants.PERM_PLAYER_ACCESSINVENTORY))
					return true;
			}
			else
			{
				//! If this instance of a character is *not* controlled by the client player (ie. a remote player),
				//! pretend restrained if COT *is* active _and_ the client player *does* have AccessInventory permission
				//! (thus allowing access to this character's inventory for an admin with appropriate permissions)
				if (GetCommunityOnlineToolsBase().IsActive() && JMPermissions.Has(JMConstants.PERM_PLAYER_ACCESSINVENTORY))
					return true;
			}
		}

		return false;
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

	//! Excuse the next movement samples for this player. Safe to call from any
	//! side and at any time - the anti-cheat only acts on it server-side.
	//!
	//! Because this sits on PlayerBase.SetWorldPosition, ANY mod that teleports
	//! a player through that method is covered without knowing COT exists. Mods
	//! that move a player some other way - a raw SetPosition, a sync juncture -
	//! should call COT_SanctionAction themselves.
	void COT_ReportEngineMove()
	{
		COT_SanctionAction(JMAntiCheatSanction.MOVEMENT, JMAntiCheatSanction.MoveGraceSeconds);
	}

	//! Declare that the server is about to do something to this player that an
	//! anti-cheat rule would otherwise read as a cheat. Call it BEFORE acting.
	//!
	//! This is how a scripted action is told apart from a cheat: not by who the
	//! player is, but by whether our own code announced the action. See
	//! JMAntiCheatSanction for the buckets.
	void COT_SanctionAction(string bucket, float seconds)
	{
		if (!g_Game || !g_Game.IsServer())
			return;

		PlayerIdentity identity = GetIdentity();
		if (!identity)
			return;

		JMAntiCheatSanction.Grant(identity.GetId(), bucket, seconds);
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

	protected void Safe_SetAuthenticatedPlayer()
	{
		if ( m_AuthenticatedPlayer || !GetIdentity() )
			return;

		if ( IsMissionOffline() )
			m_AuthenticatedPlayer = GetPermissionsManager().GetClientPlayer();
		else
			m_AuthenticatedPlayer = GetPermissionsManager().GetPlayer( GetIdentity().GetId() );

		if ( m_AuthenticatedPlayer )		
			m_AuthenticatedPlayer.PlayerObject = this;
		else
			g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( Safe_SetAuthenticatedPlayer, 2000, false );
	}

	override string FormatSteamWebhook()
	{
		//! Only ALIVE actual players will be authenticated
		JMPlayerInstance instance = GetAuthenticatedPlayer();
		if (instance)
			return instance.FormatSteamWebhook();

		//! Could be dead player or AI

		string name = GetCachedName();

		//! Only actual players will have a cached ID so we can early return here
		string id = GetCachedID();
		if (id)
			return string.Format("%1 (id=%2)", name, id);

		switch (GetInstanceType())
		{
			case DayZPlayerInstanceType.INSTANCETYPE_AI_SERVER:
			case DayZPlayerInstanceType.INSTANCETYPE_AI_REMOTE:
			case DayZPlayerInstanceType.INSTANCETYPE_AI_SINGLEPLAYER:
			#ifdef EXPANSIONMODAI
				eAIGroup group = GetGroup();
				if (group)
				{
					string groupName = group.GetName();
					if (groupName)
						name += string.Format(" (%1)", groupName);
				}
			#endif
				return "AI " + name;
		}

		//! Something wrong with vanilla ID cache? Shouldn't happen

		if (!name)
			name = super.FormatSteamWebhook();

		return name + " (WARNING)";
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
		if ( g_Game.IsServer() )
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
		if ( g_Game.IsServer() )
		{
			m_JMIsFrozen = mode;
			m_JMIsFrozenRemoteSynch = mode;

			#ifdef SERVER
			COT_SynchPlayerVars();
			#endif

			HumanInputController hic = GetInputController();
			if ( hic )
			{
				hic.SetDisabled(m_JMIsFrozen);
				DisableSimulation(m_JMIsFrozen);
			}
		}
	}

	bool COTIsRagdoll()
	{
		return m_JMIsRagdoll;
	}

	//! Cached rather than read live off PhysicsIsRagdoll(): this runs from
	//! COT_UpdatePlayerVars, which COTOnConnect fires for every player the
	//! moment they connect - querying the physics engine that early crashed
	//! the server outright, before the entity's physics was up. GodMode and
	//! Freeze already follow this cached pattern for the same reason.
	void COTSetRagdoll( bool mode )
	{
#ifndef DAYZ_1_29
		if ( g_Game.IsServer() )
		{
			PhysicsSetRagdoll( mode );
			m_JMIsRagdoll = mode;

			#ifdef SERVER
			COT_SynchPlayerVars();
			#endif
		}
#endif
	}

	void COTSetInvisibility( int mode, bool preference = true )
	{
		if ( g_Game.IsServer() )
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
		#ifdef SERVER
			m_JMIsInvisible = mode;
		#endif
			m_JMIsInvisibleRemoteSynch = mode;

			COT_SynchPlayerVars();
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
		if ( g_Game.IsServer() )
		{
			m_JMHasUnlimitedAmmo = mode;

			#ifdef SERVER
			COT_SynchPlayerVars();
			#endif
		}
	}

	void COTSetUnlimitedStamina( bool mode )
	{
		if ( g_Game.IsServer() )
		{
			m_JMHasUnlimitedStamina = mode;

			#ifdef SERVER
			COT_SynchPlayerVars();
			#endif
		}
	}

	void COTSetAdminNVG( bool mode )
	{
		if ( g_Game.IsServer() )
		{
		#ifdef SERVER
			m_JMHasAdminNVG = mode;
		#endif
			m_JMHasAdminNVGRemoteSynch = mode;

			COT_SynchPlayerVars();
		}
	}

	void COTSetScale( float value )
	{
		if ( g_Game.IsServer() )
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
		if ( g_Game.IsServer() )
		{
			GetModifiersManager().DeactivateAllModifiers();

			if ( m_AgentPool )
				m_AgentPool.RemoveAllAgents();

			if ( IsUnconscious() )
				DayZPlayerSyncJunctures.SendPlayerUnconsciousness(this, false);
		}
	}

	void COTAddDisease( int agent, float count )
	{
		if ( g_Game.IsServer() )
		{
			if ( m_AgentPool )
				m_AgentPool.AddAgent( agent, count );
		}
	}

	void COTRemoveDisease( int agent )
	{
		if ( g_Game.IsServer() )
		{
			if ( m_AgentPool )
				m_AgentPool.RemoveAgent( agent );
		}
	}

	void COTActivateModifier( int modifier_id )
	{
		if ( g_Game.IsServer() && GetModifiersManager() )
			GetModifiersManager().ActivateModifier( modifier_id );
	}

	void COTDeactivateModifier( int modifier_id )
	{
		if ( g_Game.IsServer() && GetModifiersManager() )
			GetModifiersManager().DeactivateModifier( modifier_id );
	}

	int COTGetAgentCount( int agent )
	{
		if ( m_AgentPool )
			return m_AgentPool.GetSingleAgentCount( agent );

		return 0;
	}

	// Activate a bleeding source on the named body-part selection
	// (e.g. "Head", "LeftArm"). Vanilla only exposes AddBleedingSource(int bit)
	// and only DebugActivateBleedingSource(int idx) maps to a part - but those
	// index maps are internal. We round-trip via the COT mod of the base class
	// which exposes zone enumeration, then call AddBleedingSource(bit).
	bool COTAddBleedingByName( string selectionName )
	{
		if ( !g_Game.IsServer() )
			return false;
		if ( selectionName == "" )
			return false;

		BleedingSourcesManagerServer bms = GetBleedingManagerServer();
		if ( !bms )
			return false;

		int count = bms.COT_GetZoneCount();
		for ( int i = 0; i < count; i++ )
		{
			if ( bms.COT_GetZoneSelectionName( i ) == selectionName )
			{
				int bit = bms.COT_GetZoneBit( i );
				if ( bit != 0 )
					bms.COT_ActivateBleedingSource( bit );
				return true;
			}
		}
		return false;
	}

	void COTResetItemWetness()
	{
		ItemBase item;
		array< EntityAI > items = new array< EntityAI >;
		GetInventory().EnumerateInventory( InventoryTraversalType.PREORDER, items );

		foreach ( EntityAI entity: items )
		{
			if ( Class.CastTo( item, entity ) )
				item.SetWet( 0 );
		}
	}

	void COT_RemoveAllItems()
	{
		ItemBase item;
		for (int i = GetInventory().AttachmentCount() - 1; i >= 0; --i)
		{
			if (Class.CastTo(item, GetInventory().GetAttachmentFromIndex(i)))
				item.DeleteSafe();
		}
	}

	void COT_ClearCargo()
	{
		array<EntityAI> entities = {};
		GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, entities);

		foreach (EntityAI entity: entities)
		{
			if (entity && entity.GetInventory().IsInCargo())
				entity.DeleteSafe();
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
		else if (m_JM_SpectatedObject)
		{
			position = m_JM_SpectatedObject.GetPosition();
			spectate = true;
		}
		else
		{
			return;
		}

		if (spectate)
			COTMirrorSpectatedVitals();

		if (freeCam && vector.DistanceSq(m_JMLastPosition, position) <= 22500)
		{
			//! If we get close (within 150 m) of original position, place player at original position

			if (COTIsInvisible() && !m_COT_Invisibility_Preference)
				COTSetInvisibility( JMInvisibilityType.None, false );

			if (GetPosition()[1] < g_Game.SurfaceY(position[0], position[2]))
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
				position[1] = g_Game.SurfaceY(position[0], position[2]);

				if (m_COT_EdgeTick)
					m_COT_EdgeTick--;
				else
					m_COT_EdgeTick = 3;
			}
			else
			{
				position[1] = g_Game.SurfaceRoadY3D(position[0], position[1], position[2], RoadSurfaceDetection.CLOSEST) - 3.0;
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
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(COTSetGodMode, 34, false, false, false);

		COTRestoreOwnVitals();
	}

	//! While spectating, the admin's own HUD (health/blood/water/energy/
	//! stamina/heat) is made to read the spectated survivor's vitals instead
	//! of the admin's own - the vanilla HUD only ever displays the locally
	//! controlled entity's stats, so there is no other way to make it show
	//! what the target is going through. First tick snapshots the admin's
	//! real values so COTRestoreOwnVitals() can put them back on spectate end.
	protected void COTMirrorSpectatedVitals()
	{
		PlayerBase target;
		if (!Class.CastTo(target, m_JM_SpectatedObject))
			return;

		if (!m_COT_SpectateStatsSaved)
		{
			m_COT_SavedHealth = GetHealth( "GlobalHealth", "Health" );
			m_COT_SavedBlood = GetHealth( "GlobalHealth", "Blood" );
			m_COT_SavedShock = GetHealth( "GlobalHealth", "Shock" );
			m_COT_SavedEnergy = GetStatEnergy().Get();
			m_COT_SavedWater = GetStatWater().Get();
			m_COT_SavedStamina = GetStatStamina().Get();
			m_COT_SavedHeatComfort = GetStatHeatComfort().Get();
			m_COT_SpectateStatsSaved = true;

			//! The 1s position-sync tick that also calls this is too coarse for
			//! something like bleeding blood to read as "live" - run this on
			//! its own faster timer for as long as the spectate lasts.
			g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( COTMirrorSpectatedVitals, 200, true );
		}

		SetHealth( "GlobalHealth", "Health", target.GetHealth( "GlobalHealth", "Health" ) );
		SetHealth( "GlobalHealth", "Blood", target.GetHealth( "GlobalHealth", "Blood" ) );
		SetHealth( "GlobalHealth", "Shock", target.GetHealth( "GlobalHealth", "Shock" ) );
		GetStatEnergy().Set( target.GetStatEnergy().Get() );
		GetStatWater().Set( target.GetStatWater().Get() );
		GetStatStamina().Set( target.GetStatStamina().Get() );
		GetStatHeatComfort().Set( target.GetStatHeatComfort().Get() );
	}

	protected void COTRestoreOwnVitals()
	{
		if (!m_COT_SpectateStatsSaved)
			return;

		g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( COTMirrorSpectatedVitals );

		SetHealth( "GlobalHealth", "Health", m_COT_SavedHealth );
		SetHealth( "GlobalHealth", "Blood", m_COT_SavedBlood );
		SetHealth( "GlobalHealth", "Shock", m_COT_SavedShock );
		GetStatEnergy().Set( m_COT_SavedEnergy );
		GetStatWater().Set( m_COT_SavedWater );
		GetStatStamina().Set( m_COT_SavedStamina );
		GetStatHeatComfort().Set( m_COT_SavedHeatComfort );

		m_COT_SpectateStatsSaved = false;
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

	override void OnCommandSwimStart()
	{
		//! super will lock inventory again, but since locks stack, make sure we're not currently locked
		GameInventory inventory = GetInventory();
		if (inventory && inventory.IsInventoryLocked())
			inventory.UnlockInventory(LOCK_FROM_SCRIPT);

		super.OnCommandSwimStart();
	}

	override void OnCommandLadderStart()
	{
		//! super will lock inventory again, but since locks stack, make sure we're not currently locked
		GameInventory inventory = GetInventory();
		if (inventory && inventory.IsInventoryLocked())
			inventory.UnlockInventory(LOCK_FROM_SCRIPT);

		super.OnCommandLadderStart();
	}

	override void OnCommandFallStart()
	{
		//! super will lock inventory again, but since locks stack, make sure we're not currently locked
		GameInventory inventory = GetInventory();
		if (inventory && inventory.IsInventoryLocked())
			inventory.UnlockInventory(LOCK_FROM_SCRIPT);

		super.OnCommandFallStart();
	}

	override void OnCommandClimbStart()
	{
		//! super will lock inventory again, but since locks stack, make sure we're not currently locked
		GameInventory inventory = GetInventory();
		if (inventory && inventory.IsInventoryLocked())
			inventory.UnlockInventory(LOCK_FROM_SCRIPT);

		super.OnCommandClimbStart();
	}

	override void OnCommandVehicleStart()
	{
		//! super will lock inventory again, but since locks stack, make sure we're not currently locked
		GameInventory inventory = GetInventory();
		if (inventory && inventory.IsInventoryLocked())
			inventory.UnlockInventory(LOCK_FROM_SCRIPT);

		super.OnCommandVehicleStart();
	}

	override void OnCommandVehicleFinish()
	{
		super.OnCommandVehicleFinish();

		if (CurrentActiveCamera && m_COT_IsLeavingFreeCam)
		{
			auto mission = MissionBaseWorld.Cast(g_Game.GetMission());
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
#ifdef DAYZ_1_29
				if (g_Game.IsServer())
				{
					car.ForceUpdateLightsStart(); //! obsolete 1.30, no replacement
					car.ForceUpdateLightsEnd(); //! obsolete 1.30, no replacement
				}
#endif
			}
		}
		//else if (m_COT_TransportCache)
		//{
			//StartCommand_Vehicle(m_COT_TransportCache, m_COT_TransportCache_CrewIndex, m_COT_TransportCache_Seat, IsUnconscious());
			//m_COT_TransportCache = null;
		//}
	}

	void COT_SetLastHitSnapshot(JMAntiCheatKillEvent evt, int timeMs)
	{
		m_COT_LastHitSnapshot = evt;
		m_COT_LastHitTime = timeMs;
	}

	JMAntiCheatKillEvent COT_GetLastHitSnapshot()
	{
		return m_COT_LastHitSnapshot;
	}

	int COT_GetLastHitTime()
	{
		return m_COT_LastHitTime;
	}

	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		JMAntiCheatKillHook.OnPlayerKilled(this, killer);
		JMPlayerStatsHook.OnPlayerKilled(this, killer);

		//! The position history is keyed by guid, not by character, so without
		//! this the first sample after respawning reads as a teleport from
		//! wherever this character died.
		COT_ReportEngineMove();
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		JMAntiCheatKillHook.OnPlayerHit(this, source, dmgZone, damageResult, ammo);
	}
}
