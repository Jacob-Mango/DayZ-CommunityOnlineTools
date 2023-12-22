class CommunityOnlineToolsBase
{
	static string s_HypeTrain_Loco_ClsName = "HypeTrain_LocomotiveBase";
	static typename s_HypeTrain_Loco_Type = s_HypeTrain_Loco_ClsName.ToType();

	private bool m_Loaded;

	private bool m_IsActive;
	private bool m_IsOpen;

	private string m_FileLogName;

	protected JMWebhookModule m_Webhook;

	void CommunityOnlineToolsBase()
	{
		m_Loaded = false;

		GetDayZGame().Event_OnRPC.Insert( OnRPC );
	}

	void ~CommunityOnlineToolsBase()
	{
		array< JMRenderableModuleBase > cotModules = GetModuleManager().GetCOTModules();
		for ( int i = 0; i < cotModules.Count(); i++ )
		{
			cotModules[i].Hide();
		}

		GetDayZGame().Event_OnRPC.Remove( OnRPC );
	}

	void CreateNewLog()
	{
		if ( !FileExist( JMConstants.DIR_LOGS ) )
		{
			MakeDirectory( JMConstants.DIR_LOGS );
		}

		m_FileLogName = JMConstants.DIR_LOGS + "cot-" + JMDate.Now( true ).ToString( "YYYY-MM-DD-hh-mm-ss" ) + JMConstants.EXT_LOG;
		int fileLog = OpenFile( m_FileLogName, FileMode.WRITE );

		if ( fileLog != 0 )
		{
			CloseFile( fileLog );
		}
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

	static void OnDebugSpawn(EntityAI entity) 
	{
		if (entity.GetInventory() && entity.GetInventory().GetAttachmentSlotsCount() > 0)
		{
			//! First, try OnDebugSpawn
			if (entity.IsInherited(CarScript) || entity.IsInherited(ItemBase))
				entity.OnDebugSpawn();

			//! If no atts were spawned, do it ourself (except for weapons, in that case, respect if their OnDebugSpawn doesn't spawn any atts)
			if (!entity.GetInventory().AttachmentCount() && !entity.IsInherited(Weapon_Base))
				SpawnCompatibleAttachments(entity);
		}

		Refuel(entity);
	}

	//! @note this does what vanilla EntityAI::OnDebugSpawn *should* be doing (get inventorySlot as array, case-insensitive match of slot names)
	static void SpawnCompatibleAttachments(EntityAI entity) 
	{
		TIntArray slot_ids = {};
		int slot_id;

		TStringArray atts = {};
		entity.ConfigGetTextArray("attachments", atts);

		foreach (string att: atts)
		{
			slot_id = InventorySlots.GetSlotIdFromString(att);
			if (slot_id != InventorySlots.INVALID)
			{
				PrintFormat("Entity %1 has attachment slot %2 (ID %3)", entity.GetType(), att, slot_id);
				slot_ids.Insert(slot_id);
			}
		}

		/*
		array<string> mags = {};
		entity.ConfigGetTextArray("magazines", mags);

		foreach (string mag: mags)
		{
			slot_id = InventorySlots.GetSlotIdFromString(mag);
			if (slot_id != InventorySlots.INVALID)
				slot_ids.Insert(slot_id);
		}
		*/

		TStringArray all_paths = new TStringArray;

		all_paths.Insert(CFG_VEHICLESPATH);
		all_paths.Insert(CFG_MAGAZINESPATH);
		all_paths.Insert(CFG_WEAPONSPATH);

		string child_name;
		int scope;
		string path;
		int idx;
		EntityAI child;

		foreach (string config_path: all_paths)
		{
			int children_count = GetGame().ConfigGetChildrenCount(config_path);

			for (int i = 0; i < children_count; i++)
			{
				GetGame().ConfigGetChildName(config_path, i, child_name);
				path = config_path + " " + child_name;
				scope = GetGame().ConfigGetInt( config_path + " " + child_name + " scope" );

				if (scope != 0)
				{
					TStringArray inv_slots = {};
					GetGame().ConfigGetTextArray(config_path + " " + child_name + " inventorySlot", inv_slots);

					foreach (string inv_slot: inv_slots)
					{
						slot_id = InventorySlots.GetSlotIdFromString(inv_slot);
						if (slot_id != InventorySlots.INVALID)
						{
							idx = slot_ids.Find(slot_id);
							if (idx > -1)
							{
								PrintFormat("Trying to spawn %1 on %2", child_name, entity.GetType());
								child = entity.GetInventory().CreateAttachmentEx(child_name, slot_id);
								if (child)
								{
									slot_ids.Remove(idx);
									OnDebugSpawn(child);
								}
							}
						}
					}

					if (slot_ids.Count() == 0)
						break;
				}
			}

			if (slot_ids.Count() == 0)
				break;
		}
	}

	static void Refuel(Object obj)
	{
		CarScript car;
		if (Class.CastTo(car, obj))
		{
			car.COT_Refuel();
		}
		else if (IsHypeTrain(obj))
		{
			int fuelQuantityMax;
			GetGame().GameScript.CallFunction(obj, "GetLiquidQuantityMax", fuelQuantityMax, null);
			GetGame().GameScript.CallFunction(obj, "SetLiquidQuantity", null, (float) fuelQuantityMax);
		}
	}
};


static ref CommunityOnlineToolsBase g_cotBase;

static CommunityOnlineToolsBase GetCommunityOnlineToolsBase()
{
	return g_cotBase;
}