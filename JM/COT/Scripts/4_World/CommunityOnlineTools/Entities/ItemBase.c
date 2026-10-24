modded class ItemBase
{
	static bool s_COT_IsServerCheck_AllowClientOverride;

	//! Last player to throw or place this item, for kill attribution.
	//! ItemBase is the common ancestor of ExplosivesBase and TrapBase. Only set for
	//! trackable items (COT_IsKillTrackable()). Needed because the thrower and the
	//! explosive are gone by the time EEKilled fires on the victim.
	protected PlayerBase m_COT_KillOwner;

	//! Only explosives and traps can kill someone after leaving the owner's
	//! hands, so everything else skips tagging entirely.
	bool COT_IsKillTrackable()
	{
		return IsExplosive() || IsInherited( TrapBase );
	}

	void COT_SetKillOwner( PlayerBase owner )
	{
		m_COT_KillOwner = owner;
	}

	PlayerBase COT_GetKillOwner()
	{
		return m_COT_KillOwner;
	}

	bool COT_IsLock();

	bool COT_CanInteract();

	bool COT_SetCode( string code, PlayerBase player = null );

	bool COT_HasCode();

	string COT_GetCode();

	int COT_GetMinCodeLength();

	int COT_GetMaxCodeLength();

	bool COT_SetLockState( bool locked )
	{
		if ( !COT_IsLock() || !COT_CanInteract() )
			return false;

		if ( locked )
		{
			if ( !COT_CanBeLocked() )
				return false;

			COT_CloseHierarchyRoot();
		}

		return true;
	}

	bool COT_CanBeLocked()
	{
		return COT_HasCode();
	}

	bool COT_IsLocked();

	bool COT_CanSetCode();

	bool COT_CanResetCode()
	{
		if ( COT_HasCode() && COT_CanSetCode() )
			return true;

		return false;
	}

	bool COT_ResetCode();

	void COT_CloseHierarchyRoot()
	{
		EntityAI root = GetHierarchyRoot();

		if ( root == this )
			return;

		ItemBase itemRoot;
		if ( Class.CastTo( itemRoot, root ) && itemRoot.IsOpen() )
		{
		#ifdef DZ_Expansion_BaseBuilding
			itemRoot.Expansion_Close( "*" );
		#else
			itemRoot.Close();
		#endif
		}

		Fence fence;
		if ( Class.CastTo( fence, root ) && fence.CanCloseFence() )
			fence.CloseFence();

		BuildingBase building;
		if ( Class.CastTo( building, root ) )
		{
			InventoryLocation invLoc = new InventoryLocation;
			GetInventory().GetCurrentInventoryLocation( invLoc );

			//! Find the door this lock is attached to
			int doorCount = building.GetDoorCount();
			bool found;
			for ( int doorIndex = 0; doorIndex < doorCount; ++doorIndex )
			{
				AdditionalDoorInfo doorInfo = building.GetDoorInfo( doorIndex );

				if ( doorInfo && doorInfo.m_RelatedInventorySlotNames )
				{
					foreach ( string slotName : doorInfo.m_RelatedInventorySlotNames )
					{
						if ( InventorySlots.GetSlotIdFromString( slotName ) == invLoc.GetSlot() )
						{
							found = true;

							if ( building.IsDoorOpen( doorIndex ) )
								building.CloseDoor( doorIndex );

							break;
						}
					}

					if ( found )
						break;
				}
			}
		}
	}
	
	override void OnRPC( PlayerIdentity sender, int rpc_type, ParamsReadContext ctx ) 
	{
		super.OnRPC( sender, rpc_type, ctx );

		//! We react to vanilla DigitalCodeLock RPCs for all supported lock types except DigitalCodeLock itself
		switch (rpc_type)
		{
			case ERPCs.RPC_CODE_LOCK_REQUEST:
				COT_HandleLockRequest( sender, ctx );
				break;

			case ERPCs.RPC_CODE_LOCK_RESPONSE:
				COT_HandleLockResponse( sender, ctx );
				break;

			case ERPCs.RPC_CODE_LOCK_UI_ACTION:
				COT_HandleLockAction( sender, ctx );
				break;
		}
	}

	void COT_HandleLockRequest( PlayerIdentity identity, ParamsReadContext ctx )
	{
		if ( IsInherited( DigitalCodeLock ) )  //! DigitalCodeLock handles its own RPCs
			return;

		if ( !g_Game.IsServer() )
			return;

		Param1<string> code = new Param1<string>( "" );
		if ( !ctx.Read( code ) )
			return;
		
		string pin = code.param1;

		bool success;

		if ( JMPermissions.HasRPC( JMConstants.PERM_ESP_OBJECT_SETCODE, identity ) )
		{
			if ( COT_HasCode() )
			{
				if ( COT_IsLocked() && pin == COT_GetCode() )
				{
					COT_SetLockState( false );
					success = true;
				}
			}
			else
			{
				int pinLength = pin.Length();

				if ( pinLength >= COT_GetMinCodeLength() && pinLength <= COT_GetMaxCodeLength() )
				{
					PlayerBase player = GetPlayerObjectByIdentity( identity );
					COT_SetCode( pin, player );
					success = true;
				}
			}
			
			if ( g_Game.IsDedicatedServer() )
			{
				SetSynchDirty();
			}
		}

		Param1<bool> response = new Param1<bool>( success );
		GetGame().RPCSingleParam( this, ERPCs.RPC_CODE_LOCK_RESPONSE, response, true, identity );
	}

	void COT_HandleLockResponse( PlayerIdentity identity, ParamsReadContext ctx )
	{
		if ( IsInherited( DigitalCodeLock ) )  //! DigitalCodeLock handles its own RPCs
			return;

		if ( g_Game.IsDedicatedServer() )
			return;

		Param1<bool> response = new Param1<bool>( false );
		if ( !ctx.Read( response ) )
			return;

		DigitalCodeLockUIBase codeLockUI = DigitalCodeLockUIBase.Cast( GetGame().GetUIManager().FindMenu( MENU_DIGITAL_CODELOCK ) );
		if ( codeLockUI )
		{
			bool codeResponse = response.param1;
			codeLockUI.CodeResponse( codeResponse );
		}
	}

	void COT_HandleLockAction( PlayerIdentity identity, ParamsReadContext ctx )
	{
		if ( IsInherited( DigitalCodeLock ) )  //! DigitalCodeLock handles its own RPCs
			return;

		if ( g_Game.IsDedicatedServer() )
			return;

		Param1<int> action = new Param1<int>( CodeLockUIAction.NONE );
		if ( !ctx.Read( action ) )
			return;
		
		CodeLockUIAction uiAction = action.param1;
		switch ( uiAction )
		{
			case CodeLockUIAction.CLEAR_INPUT:
			{
				StartItemSoundClient( SoundConstants.ITEM_DIGITAL_LOCK_CLEAR_INPUT );
				break;
			}
			case CodeLockUIAction.CONFIRM_INPUT:
			{
				StartItemSoundClient( SoundConstants.ITEM_DIGITAL_LOCK_CONFIRM_INPUT );
				break;
			}
			case CodeLockUIAction.DIGIT_INPUT:
			{
				StartItemSoundClient( SoundConstants.ITEM_DIGITAL_LOCK_DIGIT_INPUT );
				break;
			}
		}
	}

	//! FIX: Vanilla StartItemSoundClient checks for g_Game.IsClient() which is false in offline/SP
	override void StartItemSoundClient(int id, int slotId = InventorySlots.INVALID)
	{
		if ( g_Game.IsMultiplayer() )
			super.StartItemSoundClient( id, slotId );
		else if ( !g_Game.IsDedicatedServer() && m_ItemSoundHandler )
			m_ItemSoundHandler.PlayItemSoundClient( id, slotId );
	}

	//! Tracker hooks live on EEInit/EEDelete, not the ctor/dtor. Enforce
	//! destructors run on GC, not on deletion, so a dtor-based unregister lags
	//! and GetByClassname hands out entities that are already gone. EEDelete is
	//! deterministic. See docs/audits/2026-07-15-todo-mods-audit.md.
	override void EEInit()
	{
		super.EEInit();

		if (g_Game.IsServer())
			JMEntityTracker.Register(this);
	}

	override void EEItemAttached(EntityAI item, string slot_name)
	{
		super.EEItemAttached(item, slot_name);

		if (g_cotBase)
			g_cotBase.OnItemAttached(this, slot_name, item);
	}

	override void EEItemDetached(EntityAI item, string slot_name)
	{
		super.EEItemDetached(item, slot_name);

		if (g_cotBase)
			g_cotBase.OnItemDetached(this, slot_name, item);
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		if (g_cotBase)
			g_cotBase.OnVariablesSynchronized(this);
	}

	//! Thrown grenade: tag the player whose hands it just left.
	override void OnInventoryExit( Man player )
	{
		super.OnInventoryExit( player );

		if ( !g_Game.IsServer() || !COT_IsKillTrackable() )
			return;

		PlayerBase pb = PlayerBase.Cast( player );
		if ( pb )
			m_COT_KillOwner = pb;
	}

	//! Placed mine/claymore: tag whoever finished the placement.
	override void OnPlacementComplete( Man player, vector position = "0 0 0", vector orientation = "0 0 0" )
	{
		super.OnPlacementComplete( player, position, orientation );

		if ( !g_Game.IsServer() || !COT_IsKillTrackable() )
			return;

		PlayerBase pbp = PlayerBase.Cast( player );
		if ( pbp )
			m_COT_KillOwner = pbp;
	}

	override void EEDelete(EntityAI parent)
	{
		if (g_Game && g_Game.IsServer())
			JMEntityTracker.Unregister(this);

		super.EEDelete(parent);
	}

	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;

		PlayerBase sourcePlayer;
		if (source && Class.CastTo(sourcePlayer, source.GetHierarchyRootPlayer()) && sourcePlayer.COTGetReceiveDamageDealt() && !GetHierarchyRootPlayer())
		{
			sourcePlayer.ProcessDirectDamage(damageType, source, "Torso", ammo, "0 0 0", speedCoef);
			return false;
		}

		Transport transport;
		if (Class.CastTo(transport, GetHierarchyRoot()))
		{
			Human driver = transport.CrewMember(DayZPlayerConstants.VEHICLESEAT_DRIVER);
			if (driver && !driver.GetAllowDamage())
				return false;
		}

		return true;
	}

	override void OnEnergyConsumed()
	{
		PlayerBase player =  PlayerBase.Cast(GetHierarchyRootPlayer());
		if (player && player.COTHasGodMode())  //! UNLIMITED POWER
		{
			GetCompEM().SetEnergy0To1(1.0);
			return;
		}

		super.OnEnergyConsumed();
	}

	//! Prevent calling vanilla EntityAI::OnDebugSpawn
	//! Logic will use COT to spawn attachments, so it'll work like vanilla EXCEPT not have possibility of crashes due to bad items
	override void OnDebugSpawn()
	{
		if (!IsInherited(TentBase))
			GetCommunityOnlineToolsBase().SpawnCompatibleAttachments(this, null, 0);
	}

	override bool IsServerCheck(bool allow_client)
	{
		if (!allow_client)
			allow_client = s_COT_IsServerCheck_AllowClientOverride;

		return super.IsServerCheck(allow_client);
	}

	void COT_OnDebugSpawn(PlayerBase player)
	{
		s_COT_IsServerCheck_AllowClientOverride = true;

		OnDebugSpawnEx(DebugSpawnParams.WithPlayer(player));

		s_COT_IsServerCheck_AllowClientOverride = false;
	}
}

