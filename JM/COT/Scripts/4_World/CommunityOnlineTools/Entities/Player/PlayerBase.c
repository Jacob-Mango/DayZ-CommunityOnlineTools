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

	private bool m_COT_GodMode_Preference;

	private int m_JMIsInvisible;
	private int m_JMIsInvisibleRemoteSynch;
	private int m_COT_Invisibility_Preference;
	private bool m_COT_WasSpeaking;

	private bool m_JMIsFrozen;
	private bool m_JMIsFrozenRemoteSynch;

	private vector m_JMLastPosition;
	private bool m_JMHasLastPosition;

	private bool m_JMHasUnlimitedAmmo;

	private bool m_JMHasUnlimitedStamina;
	private bool m_JMHasUnlimitedStaminaRemoteSynch;

	PlayerBase m_JM_SpectatedPlayer;
	vector m_JM_CameraPosition;

	private bool m_COT_ReceiveDamageDealt;
	private bool m_COT_CannotBeTargetedByAI;
	private bool m_COT_CannotBeTargetedByAI_Preference;
	private bool m_COT_RemoveCollision;
	private bool m_COT_RemoveCollision_Preference;

	private bool m_COT_IsBeingKicked;

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

		RegisterNetSyncVariableInt( "m_JMIsInvisibleRemoteSynch" );
		RegisterNetSyncVariableBool( "m_JMIsFrozenRemoteSynch" );
		RegisterNetSyncVariableBool( "m_JMHasUnlimitedStaminaRemoteSynch" );

#ifndef CF_MODULE_PERMISSIONS
		GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( Safe_SetAuthenticatedPlayer, 2000, false );
#endif

		m_JMHasLastPosition = false;
		m_JMLastPosition = "0 0 0";
	}

	override void CommandHandler( float pDt, int pCurrentCommandID, bool pCurrentCommandFinished )	
	{
		bool skip;

		if (GetGame().IsServer())
			skip = (m_JM_SpectatedPlayer || m_JM_CameraPosition != vector.Zero) && m_JMIsInvisible;
		else
			skip = CurrentActiveCamera && m_JMIsInvisible;

		if (!skip)
			super.CommandHandler( pDt, pCurrentCommandID, pCurrentCommandFinished );
	}

	override void OnSelectPlayer()
	{
		super.OnSelectPlayer();

#ifndef SERVER
		if (GetGame().GetPlayer() == this && (GetCommunityOnlineToolsBase().IsOpen() || GetCOTWindowManager().Count() > 0))
			GetGame().GetUIManager().ShowUICursor(true);
#endif
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		if ( m_JMIsInvisibleRemoteSynch != m_JMIsInvisible )
		{
			m_JMIsInvisible = m_JMIsInvisibleRemoteSynch;

			if (!IsControlledPlayer())  //! Other clients
			{
				if (m_JMIsInvisible == JMInvisibilityType.DisableSimulation)
				{
					//! Set physics non-solid so there is no blocking "ghost"
					//! Needed on client because of disabling simulation (below),
					//! setting on server is not enough in this specific case
					PhysicsSetSolid(false);

					//! Disable simulation to disable position update on client, footstep sounds, etc.
					DisableSimulation(true);
				}
				else
				{
					DisableSimulation(false);
					Update();
				}
			}

			SetInvisible( m_JMIsInvisible );
		}

		if ( m_JMIsFrozenRemoteSynch != m_JMIsFrozen )
		{
			m_JMIsFrozen = m_JMIsFrozenRemoteSynch;

			HumanInputController hic = GetInputController();
			if ( hic )
				hic.SetDisabled( m_JMIsFrozen );
		}

		if ( m_JMHasUnlimitedStaminaRemoteSynch != m_JMHasUnlimitedStamina )
		{
			m_JMHasUnlimitedStaminaRemoteSynch = m_JMHasUnlimitedStamina;
		}
	}

#ifndef SERVER
	override void EOnPostFrame( IEntity other, int extra )
	{
		if ( !m_JMIsInvisible )
			return;

		SetInvisible( true );
	}
#endif

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
					CarScript car;
					if (Class.CastTo(car, transport))
						car.COT_PlaceOnSurfaceAtPosition(position);

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
		if ( Assert_Null( GetAuthenticatedPlayer(), "Player has no identity or improperly programmed AI mod" ) )
		{
			return super.FormatSteamWebhook() + " (WARNING)";
		}

		return GetAuthenticatedPlayer().FormatSteamWebhook();
	}
#endif

	bool COTHasGodMode()
	{
		return !GetAllowDamage();
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

	void COTSetGodMode( bool mode, bool preference = true )
	{
		if ( GetGame().IsServer() )
		{
			if (preference)
				m_COT_GodMode_Preference = mode;
			else if (mode)
				m_COT_GodMode_Preference = !GetAllowDamage();

			SetAllowDamage( !mode );
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

			SetSynchDirty();

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

			m_JMIsInvisible = mode;
			m_JMIsInvisibleRemoteSynch = mode;

			if (mode || preference || !m_COT_RemoveCollision_Preference)
				COTSetRemoveCollision(mode, preference);

			if (mode || preference || !m_COT_CannotBeTargetedByAI_Preference)
				COTSetCannotBeTargetedByAI(mode, preference);

			SetSynchDirty();
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
		}
	}

	void COTSetUnlimitedStamina( bool mode )
	{
		if ( GetGame().IsServer() )
		{
			m_JMHasUnlimitedStamina = mode;
			m_JMHasUnlimitedStaminaRemoteSynch = mode;
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

			if (dir.LengthSq() >= 1000000)
			{
				//! Move to edge of network bubble of target (this is where an ESPer could "see" the admin position,
				//! frozen in time) before moving directly under target.
				//! Randomize the distance a bit to not make it too obvious.
				position = position - dir.Normalized() * Math.RandomFloat(990, 999);
				position[1] = GetGame().SurfaceY(position[0], position[2]);
			}
			else
			{
				position[1] = GetGame().SurfaceY(position[0], position[2]) - 10;
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
};

