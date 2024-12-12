class CommunityOnlineToolsBase
{
	static string s_HypeTrain_Loco_ClsName = "HypeTrain_LocomotiveBase";
	static typename s_HypeTrain_Loco_Type = s_HypeTrain_Loco_ClsName.ToType();

	private bool m_Loaded;

	private bool m_IsActive;
	private bool m_IsOpen;

	private string m_FileLogName;

	protected JMWebhookModule m_Webhook;

	protected ref map<string, bool> m_ActiveGUIDs = new map<string, bool>;

	void CommunityOnlineToolsBase()
	{
		m_Loaded = false;

		GetDayZGame().Event_OnRPC.Insert( OnRPC );
	}

	void ~CommunityOnlineToolsBase()
	{
		if (!g_Game)
			return;

		array< JMRenderableModuleBase > cotModules = GetModuleManager().GetCOTModules();
		for ( int i = 0; i < cotModules.Count(); i++ )
		{
			cotModules[i].Close();
		}

		GetDayZGame().Event_OnRPC.Remove( OnRPC );
	}

	void CreateNewLog()
	{
		if ( !FileExist( JMConstants.DIR_LOGS ) )
			MakeDirectory( JMConstants.DIR_LOGS );

		m_FileLogName = JMConstants.DIR_LOGS + "cot-" + JMDate.Now( true ).ToString( "YYYY-MM-DD-hh-mm-ss" ) + JMConstants.EXT_LOG;
		int fileLog = OpenFile( m_FileLogName, FileMode.WRITE );

		if ( fileLog != 0 )
			CloseFile( fileLog );
	}

	void CloseLog()
	{
		m_FileLogName = "";
	}

	void OnStart()
	{
		m_IsOpen = false;

		JMScriptInvokers.COT_ON_OPEN.Invoke( m_IsOpen );
		
		#ifndef CF_MODULE_PERMISSIONS
		if ( GetGame().IsServer() && GetGame().IsMultiplayer() )
		{
			GetPermissionsManager().LoadRoles();
		}
		#endif

		CreateNewLog();

		if ( IsMissionOffline() )
		{
			m_Webhook = NULL;
		} else
		{
			Class.CastTo( m_Webhook, GetModuleManager().GetModule( JMWebhookModule ) );
		}
	}

	void OnFinish()
	{
		m_IsOpen = false;

		JMScriptInvokers.COT_ON_OPEN.Invoke( m_IsOpen );

		CloseLog();
	}

	void OnLoaded()
	{
	}

	void OnUpdate( float timeslice )
	{
		if ( !m_Loaded && !GetDayZGame().IsLoading() )
		{
			m_Loaded = true;
			OnLoaded();
		}
	}

	bool IsActive()
	{
		return m_IsActive;
	}

	bool IsActive(Man player)
	{
		return IsActive(player.GetIdentity());
	}

	bool IsActive(PlayerIdentity identity)
	{
		return IsActive(identity.GetId());
	}

	bool IsActive(string guid)
	{
	#ifdef SERVER
		return m_ActiveGUIDs[guid];
	#else
		return IsActive();
	#endif
	}

	void SetActive( bool active )
	{
		m_IsActive = active;

		OnCOTActiveChanged( m_IsActive );
	}

	void ToggleActive()
	{
		m_IsActive = !m_IsActive;

		OnCOTActiveChanged( m_IsActive );
	}

	void OnCOTActiveChanged( bool active )
	{
	}

	bool IsOpen()
	{
		return m_IsOpen;
	}

	void SetOpen( bool open )
	{
		if ( open )
		{
			if ( GetGame().GetUIManager().GetMenu() )
				return;

			if ( !GetPermissionsManager().HasPermission( "COT.View" ) )
				return;

			if ( !GetCommunityOnlineToolsBase().IsActive() )
			{
				COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF" ) );
				return;
			}
		}

		m_IsOpen = open;

		JMScriptInvokers.COT_ON_OPEN.Invoke( m_IsOpen );
	}

	void ToggleOpen()
	{
		if ( !m_IsOpen )
		{
			if ( GetGame().GetUIManager().GetMenu() )
			{
				return;
			}

			if ( !GetPermissionsManager().HasPermission( "COT.View" ) )
			{
				return;
			}

			if ( !GetCommunityOnlineToolsBase().IsActive() )
			{
				COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF" ) );
				return;
			}
		}

		m_IsOpen = !m_IsOpen;

		JMScriptInvokers.COT_ON_OPEN.Invoke( m_IsOpen );
	}

	void LogServer( string text )
	{
		if ( GetGame().IsServer() )
		{
			GetGame().AdminLog( "[COT] " + text );
		}

		int fileLog = OpenFile( m_FileLogName, FileMode.APPEND );
		if ( fileLog != 0 )
		{
			FPrintln( fileLog, "[COT " + JMDate.Now( true ).ToString( "YYYY-MM-DD hh:mm:ss" ) + "] " + text );
			CloseFile( fileLog );
		}
	}

	void Log( JMPlayerInstance logInstacPlyer, string text )
	{
		if ( GetGame().IsMultiplayer() )
		{
			text = "" + logInstacPlyer.GetSteam64ID() + ": " + text;
		} else
		{
			text = "Offline: " + text;
		}

		if ( GetGame().IsServer() )
		{
			GetGame().AdminLog( "[COT] " + text );
		}

		int fileLog = OpenFile( m_FileLogName, FileMode.APPEND );
		if ( fileLog != 0 )
		{
			FPrintln( fileLog, "[COT " + JMDate.Now( true ).ToString( "YYYY-MM-DD hh:mm:ss" ) + "] " + text );
			CloseFile( fileLog );
		}
	}

	void Log( PlayerIdentity logIdentPlyer, string text )
	{
		if ( GetGame().IsMultiplayer() )
		{
			text = "" + logIdentPlyer.GetPlainId() + ": " + text;
		} else
		{
			text = "Offline: " + text;
		}

		if ( GetGame().IsServer() )
		{
			GetGame().AdminLog( "[COT] " + text );
		}

		int fileLog = OpenFile( m_FileLogName, FileMode.APPEND );
		if ( fileLog != 0 )
		{
			FPrintln( fileLog, "[COT " + JMDate.Now( true ).ToString( "YYYY-MM-DD hh:mm:ss" ) + "] " + text );
			CloseFile( fileLog );
		}
	}

	void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		switch (rpc_type)
		{
			case JMCOTBaseRPC.TransportSync:
			#ifndef SERVER
				vector position, orientation;
				if (!ctx.Read(position) || !ctx.Read(orientation))
					break;
				Transport transport;
				if (!Class.CastTo(transport, target))
					break;
				PrintFormat("OnRPC TransportSync %1 pos=%2 ori=%3", target, position.ToString(), orientation.ToString());
				transport.SetPosition(position);
				transport.SetOrientation(orientation);
			#endif
				break;
		}
	}

	void UpdateRole( JMRole role, PlayerIdentity toSendTo )
	{
	}

	void UpdateClient( string guid, PlayerIdentity sendTo )
	{
	}

	void RemoveClient( string guid )
	{
	}

	void RefreshClients()
	{
	}

	void RefreshClientPositions()
	{
	}

	void SetClient( JMPlayerInstance player )
	{
	}

	void SetClient( JMPlayerInstance player, PlayerIdentity identity )
	{
	}

	static void ForceDisableInputs(bool state, inout TIntArray skipIDs = null)
	{
		if (!skipIDs)
			skipIDs = new TIntArray;

		skipIDs.Insert(UAUIBack);

		TIntArray inputIDs = new TIntArray;

		GetUApi().GetActiveInputs(inputIDs);

		foreach (int inputID: inputIDs)
		{
			if (skipIDs.Find(inputID) == -1)
			{
				GetUApi().GetInputByID(inputID).ForceDisable(state);
			}
		}
	}

	static void HealEntityRecursive(Object obj, bool includeAttachments = true, bool includeCargo = true)
	{
		obj.SetFullHealth();

		EntityAI entity;
		if (!Class.CastTo(entity, obj) || !entity.GetInventory())
			return;

		int i;

		if (includeAttachments)
		{
			EntityAI attachment;
			CarWheel_Ruined ruinedWheel;

			for (i = entity.GetInventory().AttachmentCount() - 1; i >= 0; i--)
			{
				attachment = entity.GetInventory().GetAttachmentFromIndex(i);
				if (Class.CastTo(ruinedWheel, attachment))
				{
					string ruinedWheelType = ruinedWheel.GetType();
					string newWheelType = ruinedWheelType.Substring(0, ruinedWheelType.Length() - 7);

					if (GetGame().IsKindOf(newWheelType, "CarWheel"))
					{
						bool isLockedInSlot = false;
						InventoryLocation wheelLocation = new InventoryLocation();
						ruinedWheel.GetInventory().GetCurrentInventoryLocation(wheelLocation);
						int slotId = wheelLocation.GetSlot();
						if (entity.GetInventory().GetSlotLock(slotId))
						{
							isLockedInSlot = true;
							entity.GetInventory().SetSlotLock(slotId, false);
						}

						GetGame().ObjectDelete(ruinedWheel);
						entity.GetInventory().CreateAttachmentEx(newWheelType, slotId);

						if (isLockedInSlot)
							entity.GetInventory().SetSlotLock(slotId, true);
					}
				}
				else
				{
					HealEntityRecursive(attachment, true, includeCargo);
				}
			}
		}

		if (includeCargo)
		{
			CargoBase cargo = entity.GetInventory().GetCargo();
			if (cargo)
			{
				for (i = 0; i < cargo.GetItemCount(); i++)
				{
					HealEntityRecursive(cargo.GetItem(i), includeAttachments, true);
				}
			}
		}
	}

	static bool IsHypeTrain(Object obj)
	{
		if (s_HypeTrain_Loco_Type && obj.IsInherited(s_HypeTrain_Loco_Type))
			return true;

		return false;
	}

	static void Refuel(Object obj)
	{
		CarScript car;
		BoatScript boat;
		if (Class.CastTo(car, obj))
		{
			car.COT_Refuel();
		}
		else if (Class.CastTo(boat, obj))
		{
			boat.COT_Refuel();
		}
		else if (IsHypeTrain(obj))
		{
			int fuelQuantityMax;
			GetGame().GameScript.CallFunction(obj, "GetLiquidQuantityMax", fuelQuantityMax, null);
			GetGame().GameScript.CallFunction(obj, "SetLiquidQuantity", null, (float) fuelQuantityMax);
		}
	}

	void SpawnCompatibleAttachments(EntityAI entity, PlayerBase player, int depth = 3)
	{
	}

	static void ForceTransportPositionAndOrientation(Transport transport, vector position, vector orientation)
	{
		vector velocity = GetVelocity(transport);
		vector angularVelocity = dBodyGetAngularVelocity(transport);

		if (!dBodyIsActive(transport))
			dBodyActive(transport, ActiveState.ACTIVE);

		transport.SetPosition(position);
		transport.SetOrientation(orientation);
		transport.Synchronize();

		SetVelocity(transport, velocity);
		dBodySetAngularVelocity(transport, angularVelocity);
	}

	static void PlaceOnSurfaceAtPosition(EntityAI entity, vector position, bool aboveWater = true)
	{
		vector surface = Vector(position[0], GetGame().SurfaceY(position[0], position[2]), position[2]);

		vector entityMinMax[2];
		if (!entity.GetCollisionBox(entityMinMax))
			entity.ClippingInfo(entityMinMax);

		float entityOffsetY = entityMinMax[0][1];
		if (entityOffsetY > 0)
			entityOffsetY = 0;

		vector startPos = position;
		startPos[1] = startPos[1] - entityOffsetY;
		if (surface[1] > startPos[1])
			startPos[1] = surface[1];

		float waterDepth;
		if (aboveWater)
			waterDepth = GetGame().GetWaterDepth(surface);

		if (waterDepth > 0)
		{
			surface[1] = surface[1] + waterDepth;
			position = surface;
		}
		else
		{
			PhxInteractionLayers layerMask;
			layerMask |= PhxInteractionLayers.BUILDING;
			layerMask |= PhxInteractionLayers.DOOR;
			layerMask |= PhxInteractionLayers.VEHICLE;
			layerMask |= PhxInteractionLayers.ROADWAY;
			layerMask |= PhxInteractionLayers.TERRAIN;
			layerMask |= PhxInteractionLayers.ITEM_LARGE;
			layerMask |= PhxInteractionLayers.FENCE;
			vector hitPosition;
			vector hitNormal;

			if (DayZPhysics.RayCastBullet(startPos + "0 1 0", surface - "0 1 0", layerMask, entity, null, hitPosition, hitNormal, null))
			{
				position = hitPosition;
			} else {
				position = surface;
				hitNormal = GetGame().SurfaceGetNormal(surface[0], surface[2]);
			}
		}

		position[1] = position[1] - entityOffsetY;

		vector orientation = entity.GetOrientation();
		entity.SetOrientation(Vector(orientation[0], 0, 0));

		vector transform[4];
		entity.GetTransform(transform);
		transform[3] = position;
		entity.PlaceOnSurfaceRotated(transform, position, hitNormal[0] * -1, hitNormal[2] * -1, 0, true);
		entity.SetTransform(transform);

		Transport transport = Transport.Cast(entity);
		if (transport)
		{
			ForceTransportPositionAndOrientation(transport, position, entity.GetOrientation());
		}
	}
};


static ref CommunityOnlineToolsBase g_cotBase;

static CommunityOnlineToolsBase GetCommunityOnlineToolsBase()
{
	return g_cotBase;
}