//! Enforce compiles GameLib/Game/World/Mission as SEPARATE modules - a
//! #define in a 3_Game file (see StaticFunctions.c) is invisible here. Kept
//! in exact sync with that file's block; both must change together.
#define COT_DEBUGLOGS

class CommunityOnlineToolsBase
{
	static string s_HypeTrain_Loco_ClsName = "HypeTrain_LocomotiveBase";
	static typename s_HypeTrain_Loco_Type = s_HypeTrain_Loco_ClsName.ToType();
	protected bool m_Loaded;
	protected bool m_IsActive;
	protected bool m_IsOpen;
	protected string m_FileLogName;
	protected JMWebhookModule m_Webhook;
	protected ref map<string, bool> m_ActiveGUIDs = new map<string, bool>;

	//! Below this gap between two accepted SetOpen() calls, a new request is
	//! dropped instead of applied. Two independent input bindings reach this
	//! method - ToggleMenu ("Toggle
	//! Sidebar") and CloseCOT (vanilla UI-back) - and a real repro showed
	//! SetOpen(true)/SetOpen(false) alternating multiple times with no gap
	//! at all in the log between them (no unrelated engine/script lines
	//! interleaved), each full cycle playing the sidebar's slide-open and
	//! slide-closed animation back to back - the "open close open close"
	//! flicker. Whatever combination of bindings/held keys produces that
	//! burst, a plain minimum-interval debounce on the single chokepoint
	//! both paths already go through stops it outright, and 250ms is far
	//! below anything a deliberate toggle-close-toggle sequence would need.
	protected static const int MIN_TOGGLE_INTERVAL_MS = 250;
	protected int m_LastOpenChangeMs;

	//! Anything a client sends as a position, orientation or scale has to pass
	//! through here before it reaches an entity.
	//!
	//! A permission says an admin may move things. It does not say the numbers
	//! arrived intact - and a hand-written packet, or a mod sending garbage,
	//! can carry NaN or an astronomical coordinate. Those do not fail loudly:
	//! they put an entity somewhere physics cannot resolve, which desyncs or
	//! crashes the clients that stream it in, not the sender.
	//!
	//! NaN needs its own test. Every comparison against NaN is false, so a
	//! plain range check passes it - the value is neither below the floor nor
	//! above the ceiling. Only "is it equal to itself" catches it.
	static const float COT_SANE_MAGNITUDE = 1000000000;

	//! A position the map can actually hold.
	//!
	//! The margin is generous on purpose - offshore boats, objects staged past
	//! the coastline and modded maps that spawn outside their own bounds are
	//! all legitimate. This rejects the impossible, not the unusual.
	static const float COT_WORLD_MARGIN = 5000;
	static const float COT_WORLD_FLOOR = -2000;
	static const float COT_WORLD_CEILING = 20000;

	//! Scale is sent by the client and applied to a spawned object. A zero or
	//! a negative inverts or collapses the model, and a huge one covers the map
	//! in geometry every nearby client then has to render.
	static const float COT_SCALE_MIN = 0.01;
	static const float COT_SCALE_MAX = 10.0;

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

	//! The read side of SetFuel01: fuel as a fraction of the tank, or -1 for an
	//! object whose tank this cannot read (a train keeps its fuel as a liquid
	//! quantity rather than a vehicle fluid, and has no fraction to hand back).
	static float GetFuel01(Object obj)
	{
		CarScript car;
		BoatScript boat;
	#ifndef DAYZ_1_29
		MotorbikeScript bike;
	#endif
		if (Class.CastTo(car, obj))
			return car.GetFluidFraction(CarFluid.FUEL);

		if (Class.CastTo(boat, obj))
			return boat.GetFluidFraction(BoatFluid.FUEL);

	#ifndef DAYZ_1_29
		if (Class.CastTo(bike, obj))
			return bike.GetFluidFraction(MotorbikeFluid.FUEL);
	#endif

		return -1;
	}

	void GetHeadTransform(Object obj, out vector transform[4], bool includeOffset = false)
	{
		vector transform[4];
		vector position;
		float offset;

		Human human;
		DayZCreature creature;

		if (Class.CastTo(human, obj))
			human.GetBoneTransformWS(human.GetBoneIndexByName("Head"), transform);
		else if (Class.CastTo(creature, obj))
			creature.GetBoneTransformWS(creature.GetBoneIndexByName("Head"), transform);
		else
			obj.GetTransform(transform);

		position = transform[3];

		if (human || creature)
		{
			offset = 0.12;
		}
		else
		{
			vector minMax[2];

			if (obj.GetCollisionBox(minMax))
				offset = -vector.Distance(minMax[0], minMax[1]) * 0.5;
			else
				offset = -obj.ClippingInfo(minMax);

			float height = minMax[1][1];
			position[1] = position[1] + height;

			includeOffset = true;
		}

		if (includeOffset)
			position = position + obj.GetDirection() * offset;

		transform[3] = position;
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

	static bool IsFiniteFloat(float value)
	{
		if (value != value)
			return false;

		if (value > COT_SANE_MAGNITUDE || value < -COT_SANE_MAGNITUDE)
			return false;

		return true;
	}

	static bool IsFiniteVector(vector value)
	{
		if (!IsFiniteFloat(value[0]) || !IsFiniteFloat(value[1]) || !IsFiniteFloat(value[2]))
			return false;

		return true;
	}

	static bool IsHypeTrain(Object obj)
	{
		if (s_HypeTrain_Loco_Type && obj.IsInherited(s_HypeTrain_Loco_Type))
			return true;

		return false;
	}

	bool IsOpen()
	{
		return m_IsOpen;
	}

	static bool IsValidWorldPosition(vector pos)
	{
		if (!IsFiniteVector(pos))
			return false;

		float worldSize = 15360;

		if (g_Game.GetWorld())
			worldSize = g_Game.GetWorld().GetWorldSize();

		if (pos[0] < -COT_WORLD_MARGIN || pos[0] > worldSize + COT_WORLD_MARGIN)
			return false;

		if (pos[2] < -COT_WORLD_MARGIN || pos[2] > worldSize + COT_WORLD_MARGIN)
			return false;

		if (pos[1] < COT_WORLD_FLOOR || pos[1] > COT_WORLD_CEILING)
			return false;

		return true;
	}

	void SetActive( bool active )
	{
		#ifdef COT_DEBUGLOGS
		Print("[COT_DBG] SetActive(" + active.ToString() + ") - IsOpen()=" + m_IsOpen.ToString() + " (SetOpen(true) refuses to open while inactive, but does not itself close an already-open sidebar)");
		#endif

		m_IsActive = active;

		OnCOTActiveChanged( m_IsActive );
	}

	void SetClient( JMPlayerInstance player )
	{
	}

	void SetClient( JMPlayerInstance player, PlayerIdentity identity )
	{
	}

	//! Coolant - a radiator fluid found on cars. Motorbike has no coolant
	//! tank (see MotorbikeScript.c's COT_Refuel note).
	static void SetCoolant01(Object obj, float fraction)
	{
		CarScript car;
		if (Class.CastTo(car, obj))
		{
			car.COT_SetCarFluid01(CarFluid.COOLANT, fraction);
		}
	}

	//! Fuel as a fraction of the tank rather than "fill it up".
	//!
	//! A train carries its fuel as a liquid quantity instead of a vehicle
	//! fluid, so it is set through the same scripted call Refuel uses.
	static void SetFuel01(Object obj, float fraction)
	{
		CarScript car;
		BoatScript boat;
	#ifndef DAYZ_1_29
		MotorbikeScript bike;
	#endif
		if (Class.CastTo(car, obj))
		{
			car.COT_SetCarFluid01(CarFluid.FUEL, fraction);
		}
		else if (Class.CastTo(boat, obj))
		{
			boat.COT_SetBoatFluid01(BoatFluid.FUEL, fraction);
		}
	#ifndef DAYZ_1_29
		else if (Class.CastTo(bike, obj))
		{
			bike.COT_SetBikeFluid01(MotorbikeFluid.FUEL, fraction);
		}
	#endif
		else if (IsHypeTrain(obj))
		{
			int fuelQuantityMax;
			g_Game.GameScript.CallFunction(obj, "GetLiquidQuantityMax", fuelQuantityMax, null);
			g_Game.GameScript.CallFunction(obj, "SetLiquidQuantity", null, (float) fuelQuantityMax * Math.Clamp(fraction, 0.0, 1.0));
		}
	}

	static void SetLockWheels(Object obj, bool lockState)
	{
		CarScript car;
		if (Class.CastTo(car, obj))
		{
			car.COT_SetLockWheels(lockState);
			return;
		}

	#ifndef DAYZ_1_29
		MotorbikeScript bike;
		if (Class.CastTo(bike, obj))
		{
			bike.COT_SetLockWheels(lockState);
		}
	#endif
	}

	void SetOpen( bool open )
	{
		#ifdef COT_DEBUGLOGS
		Print("[COT_DBG] SetOpen(" + open.ToString() + ") requested, current=" + m_IsOpen.ToString());
		string cotDbgStack;
		DumpStackString(cotDbgStack);
		Print("[COT_DBG] SetOpen call stack:\n" + cotDbgStack);
		#endif

		if ( open == m_IsOpen )
			return;

		int nowMs = g_Game.GetTime();
		if ( nowMs - m_LastOpenChangeMs < MIN_TOGGLE_INTERVAL_MS )
		{
			#ifdef COT_DEBUGLOGS
			Print("[COT_DBG] SetOpen(" + open.ToString() + ") dropped - debounce, dt=" + (nowMs - m_LastOpenChangeMs));
			#endif
			return;
		}

		if ( open )
		{
			if ( g_Game.GetUIManager().GetMenu() )
			{
				#ifdef COT_DEBUGLOGS
				Print("[COT_DBG] SetOpen(true) blocked - UIManager has an active menu");
				#endif
				return;
			}

			if ( !JMPermissions.Has( JMConstants.PERM_COT_VIEW ) )
				return;

			if ( !GetCommunityOnlineToolsBase().IsActive() )
			{
				ShowInactiveNotification( "STR_COT_INPUT_TOGGLE_SIDEBAR" );
				return;
			}
		}

		m_LastOpenChangeMs = nowMs;
		m_IsOpen = open;

		#ifdef COT_DEBUGLOGS
		Print("[COT_DBG] SetOpen(" + open.ToString() + ") applied, invoking COT_ON_OPEN");
		#endif

		JMScriptInvokers.COT_ON_OPEN.Invoke( m_IsOpen );

		if ( !m_IsOpen )
		{
			JMScriptInvokers.COT_ON_CLOSE.Invoke();
		}
	}

	void CreateNewLog()
	{
		if ( !FileExist( JMConstants.DIR_LOGS ) )
			MakeDirectory( JMConstants.DIR_LOGS );

		m_FileLogName = JMConstants.DIR_LOGS + "cot-" + JMDate.Now().ToString( "YYYY-MM-DD-hh-mm-ss" ) + JMConstants.EXT_LOG;
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
		#ifdef COT_DEBUGLOGS
		Print("[COT_DBG] OnStart() - forcing m_IsOpen=false and invoking COT_ON_OPEN directly, bypassing SetOpen()'s debounce/logging");
		#endif

		m_IsOpen = false;

		JMScriptInvokers.COT_ON_OPEN.Invoke( m_IsOpen );

		#ifndef CF_MODULE_PERMISSIONS
		if ( g_Game.IsServer() && g_Game.IsMultiplayer() )
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
		#ifdef COT_DEBUGLOGS
		string cotDbgFinishStack;
		DumpStackString(cotDbgFinishStack);
		Print("[COT_DBG] OnFinish() - forcing m_IsOpen=false, bypassing SetOpen(). Call stack:\n" + cotDbgFinishStack);
		#endif

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

	void ToggleActive()
	{
		#ifdef COT_DEBUGLOGS
		Print("[COT_DBG] ToggleActive() " + m_IsActive.ToString() + " -> " + (!m_IsActive).ToString());
		#endif

		m_IsActive = !m_IsActive;

		OnCOTActiveChanged( m_IsActive );
	}

	void OnCOTActiveChanged( bool active )
	{
	}

	void ShowInactiveNotification(string inputLoc)
	{
		StringLocaliser title = new StringLocaliser("STR_COT_NOTIFICATION_TITLE_ADMIN");

		string inputName = "UACOTModuleToggleCOT";
		int deviceType = EInputDeviceType.MOUSE_AND_KEYBOARD;
		map<int, ref TStringArray> buttonMap = InputUtils.GetComboButtonNamesFromInput(inputName, deviceType);
		string combo;
		if (buttonMap)
		{
			foreach (int altIdx, TStringArray buttons: buttonMap)
			{
				foreach (int btnIdx, string button: buttons)
				{
					if (btnIdx > 0)
						combo += " + ";

					combo += button;
				}

				break;  //! We are only interested in the 1st combo
			}
		}

		StringLocaliser message = new StringLocaliser("STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF", inputLoc, combo);
		string icon = "set:ccgui_enforce image:HudBuild";
		float time = 1.5;

		NotificationSystem.Create(title, message, icon, ARGB( 255, 221, 38, 38 ), time, null);
	}

	void ToggleOpen()
	{
		SetOpen(!m_IsOpen);
	}

	void LogServer( string text )
	{
		if ( g_Game.IsServer() )
		{
			g_Game.AdminLog( "[COT] " + text );
		}

		int fileLog = OpenFile( m_FileLogName, FileMode.APPEND );
		if ( fileLog != 0 )
		{
			FPrintln( fileLog, "[COT " + JMDate.Now().ToString( "YYYY-MM-DD hh:mm:ss" ) + "] " + text );
			CloseFile( fileLog );
		}
	}

	void Log( JMPlayerInstance logInstacPlyer, string text )
	{
		if ( g_Game.IsMultiplayer() )
		{
			text = "" + logInstacPlyer.GetSteam64ID() + ": " + text;
		} else
		{
			text = "Offline: " + text;
		}

		if ( g_Game.IsServer() )
		{
			g_Game.AdminLog( "[COT] " + text );
		}

		int fileLog = OpenFile( m_FileLogName, FileMode.APPEND );
		if ( fileLog != 0 )
		{
			FPrintln( fileLog, "[COT " + JMDate.Now().ToString( "YYYY-MM-DD hh:mm:ss" ) + "] " + text );
			CloseFile( fileLog );
		}
	}

	void Log( PlayerIdentity logIdentPlyer, string text )
	{
		if ( g_Game.IsMultiplayer() && logIdentPlyer )
		{
			text = "" + logIdentPlyer.GetPlainId() + ": " + text;
		} else
		{
			text = "Offline: " + text;
		}

		if ( g_Game.IsServer() )
		{
			g_Game.AdminLog( "[COT] " + text );
		}

		int fileLog = OpenFile( m_FileLogName, FileMode.APPEND );
		if ( fileLog != 0 )
		{
			FPrintln( fileLog, "[COT " + JMDate.Now().ToString( "YYYY-MM-DD hh:mm:ss" ) + "] " + text );
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

	static void ForceDisableInputs(bool state, inout TIntArray skipIDs = null)
	{
		if (!skipIDs)
			skipIDs = new TIntArray;

		skipIDs.Insert(UAUIBack);
		skipIDs.Insert(UAUISelect);
		skipIDs.Insert(UAUITabLeft);
		skipIDs.Insert(UAUITabRight);

		TIntArray inputIDs = new TIntArray;

		GetUApi().GetActiveInputs(inputIDs);

		#ifdef COT_DEBUGLOGS
		Print("[COT_DBG] ForceDisableInputs(" + state.ToString() + ") activeInputs=" + inputIDs.Count());
		#endif

		foreach (int inputID: inputIDs)
		{
			if (skipIDs.Find(inputID) == -1)
			{
				#ifdef COT_DEBUGLOGS_VERBOSE
				Print("[COT_DBG]   ForceDisable id=" + inputID + " -> " + state.ToString());
				#endif
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

					if (g_Game.IsKindOf(newWheelType, "CarWheel"))
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

						g_Game.ObjectDelete(ruinedWheel);
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

			Man player;
			if (Class.CastTo(player, obj))
			{
				EntityAI entityInHands = player.GetHumanInventory().GetEntityInHands();
				if (entityInHands)
					HealEntityRecursive(entityInHands, true, includeCargo);
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

	static float SanitizeScale(float scale)
	{
		if (!IsFiniteFloat(scale))
			return 1.0;

		return Math.Clamp(scale, COT_SCALE_MIN, COT_SCALE_MAX);
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
			g_Game.GameScript.CallFunction(obj, "GetLiquidQuantityMax", fuelQuantityMax, null);
			g_Game.GameScript.CallFunction(obj, "SetLiquidQuantity", null, (float) fuelQuantityMax);
		}
	}

	//! Car only - Motorbike has no coolant tank (see MotorbikeScript.c's
	//! COT_Refuel note).
	static void RefillCoolant(Object obj)
	{
		CarScript car;
		if (Class.CastTo(car, obj))
		{
			car.COT_RefillCoolant();
		}
	}

	//! Delete everything in an object's cargo, the object itself kept.
	//!
	//! The traversal walks nested containers too, so a backpack inside a tent
	//! goes with the tent's contents rather than surviving inside it. The
	//! entity the clear was asked for is skipped explicitly: an item sitting in
	//! somebody else's cargo answers IsInCargo about ITSELF, and without this
	//! "clear this crate" would delete the crate.
	static int ClearCargo(EntityAI entity)
	{
		if (!entity || !entity.GetInventory())
			return 0;

		array<EntityAI> entities = {};
		entity.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, entities);

		int count = 0;

		foreach (EntityAI child: entities)
		{
			if (!child || child == entity)
				continue;

			if (child.GetInventory() && child.GetInventory().IsInCargo())
			{
				child.DeleteSafe();
				count++;
			}
		}

		return count;
	}

	static bool AreWheelsLocked(Object obj)
	{
		CarScript car;
		if (Class.CastTo(car, obj))
		{
			return car.COT_AreWheelsLocked();
		}

	#ifndef DAYZ_1_29
		MotorbikeScript bike;
		if (Class.CastTo(bike, obj))
		{
			return bike.COT_AreWheelsLocked();
		}
	#endif
		return false;
	}

	static void RepairEntityRecursive(EntityAI ent)
	{
		if (!ent)
			return;

		ent.SetHealth01("", "", 1.0);

		if (!ent.GetInventory())
			return;

		for (int i = 0; i < ent.GetInventory().AttachmentCount(); ++i)
		{
			EntityAI att = ent.GetInventory().GetAttachmentFromIndex(i);
			if (att)
				RepairEntityRecursive(att);
		}

		CargoBase cargo = ent.GetInventory().GetCargo();
		if (cargo)
		{
			for (int j = 0; j < cargo.GetItemCount(); ++j)
			{
				EntityAI cargoItem = cargo.GetItem(j);
				if (cargoItem)
					RepairEntityRecursive(cargoItem);
			}
		}
	}

	void SpawnCompatibleAttachments(EntityAI entity, PlayerBase player, int depth = 3)
	{
	}

	void SpawnCompatibleAttachmentsWithColor(EntityAI entity, PlayerBase player, int depth = 3, string preferredColor = "")
	{
	}

	//! Returns the (possibly replaced) entity - see JMObjectSpawnerModule's
	//! implementation for why the input reference does not always survive.
	EntityAI RecolorEntityAndAttachments(EntityAI entity, PlayerBase player, string newColor, int depth = 3)
	{
		return entity;
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
		vector surface = Vector(position[0], g_Game.SurfaceY(position[0], position[2]), position[2]);

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
			waterDepth = g_Game.GetWaterDepth(surface);

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
				hitNormal = g_Game.SurfaceGetNormal(surface[0], surface[2]);
			}
		}

		position[1] = position[1] - entityOffsetY;

		vector orientation = entity.GetOrientation();
		entity.SetOrientation(Vector(orientation[0], 0, 0));

		if (entity.IsMan() || entity.IsDayZCreature())
		{
			PlayerBase player;
			if (Class.CastTo(player, entity))
			{
				player.SetLastPosition();
				player.SetWorldPosition(position);
			}
			else
			{
				entity.SetPosition(position);
			}
		}
		else
		{
			vector transform[4];
			entity.GetTransform(transform);
			transform[3] = position;
			entity.PlaceOnSurfaceRotated(transform, position, hitNormal[0] * -1, hitNormal[2] * -1, 0, true);
			entity.SetTransform(transform);

			Transport transport;
			if (Class.CastTo(transport, entity))
			{
				ForceTransportPositionAndOrientation(transport, position, entity.GetOrientation());
			}
		}

	}
}


static ref CommunityOnlineToolsBase g_cotBase;

static CommunityOnlineToolsBase GetCommunityOnlineToolsBase()
{
	return g_cotBase;
}

static CommunityOnlineToolsBase GetCOTBase()
{
	return g_cotBase;
}

typedef CommunityOnlineToolsBase COT_Base;
