modded class JMObjectSpawnerModule
{
	protected int m_ZenLastKeypress;

	void JMObjectSpawnerModule()
	{
		GetRPCManager().AddRPC("ZenCOT_RPC", "RPC_ZenShowOnMapRequest", this, SingeplayerExecutionType.Both);
		GetRPCManager().AddRPC("ZenCOT_RPC", "RPC_ZenShowOnMapSend", this, SingeplayerExecutionType.Both);
		GetRPCManager().AddRPC("ZenCOT_RPC", "RPC_ZenDeleteAllOnMap", this, SingeplayerExecutionType.Both);
	}

	override void RegisterKeyMouseBindings() 
	{
		super.RegisterKeyMouseBindings();

		Bind(new JMModuleBinding("ZenCopyPositionOnCursor", "UAZenCOT_CopyPosition", true));
	}

	static string ZenFormatLifetime(float lifetimeSeconds)
	{
		if (lifetimeSeconds <= 0)
			return "";

		int totalMinutes = Math.Floor(lifetimeSeconds 	/ 60.0);
		int totalHours   = Math.Floor(totalMinutes 		/ 60.0);
		int totalDays    = Math.Floor(totalHours   		/ 24.0);
		int totalWeeks   = Math.Floor(totalDays    		/ 7.0);

		string txt;

		// Weeks
		if (totalWeeks >= 1)
		{
			txt = " | ~" + totalWeeks.ToString() + " #STR_ZenCOT_Week";
			if (totalWeeks != 1)
				txt = " | ~" + totalWeeks.ToString() + " #STR_ZenCOT_Week_S";
				
			return txt;
		}

		// Days
		if (totalDays >= 1)
		{
			txt = " | ~" + totalDays.ToString() + " #STR_ZenCOT_Day";
			if (totalDays != 1)
				txt = " | ~" + totalDays.ToString() + " #STR_ZenCOT_Day_S";

			return txt;
		}

		// Hours
		if (totalHours >= 1)
		{
			txt = " | ~" + totalHours.ToString() + " #STR_ZenCOT_Hour";
			if (totalHours != 1)
				txt = " | ~" + totalHours.ToString() + " #STR_ZenCOT_Hour_S";

			return txt;
		}

		// Minutes (always show)
		txt = " | ~" + totalMinutes.ToString() + " #STR_ZenCOT_Minute";
		if (totalMinutes != 1)
			txt = " | ~" + totalMinutes.ToString() + " #STR_ZenCOT_Minute_S";

		return txt;
	}

	// Client -> server receives
	void RPC_ZenShowOnMapRequest(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!GetPermissionsManager().HasPermission("COT", sender))
			return;

		Param1<string> data;
		if (!ctx.Read(data))
		{
			Error("Failed to read data");
			return;
		}

		string className = data.param1;

		if (className == "")
			return;
		
		array<EntityAI> found = new array<EntityAI>;
		ZenEntityTracker.GetByClassname(className, found);
		
		if (!found || found.Count() == 0)
		{
			COTCreateNotification(sender, new StringLocaliser(string.Format(Widget.TranslateString("#STR_ZenCOT_NotFound"), className)));
			return;
		}

		array<string> names = new array<string>;
		array<vector> positions = new array<vector>;
		
		foreach (EntityAI entity : found)
		{
			if (!entity)
				continue;

			string lifetimeText = ZenFormatLifetime(entity.GetLifetime());

			string qtyText = "";

			if (entity.HasQuantity())
			{
				float qty = (entity.GetQuantityNormalized() * 100);
				qtyText = " | " + (int)(qty) + "%Q";
			}

			float hp = (entity.GetHealth() / entity.GetMaxHealth()) * 100;
			string hpText = " | " + (int)(hp) + "%H";

			names.Insert(ZenEntityTracker.ZenCOT_GetObjectName(entity) + qtyText + hpText + lifetimeText /*" id=" + entity.GetID()*/);
			positions.Insert(entity.GetPosition());
		}
		
		GetRPCManager().SendRPC("ZenCOT_RPC", "RPC_ZenShowOnMapSend", new Param2<ref array<string>, ref array<vector>>(names, positions), true, null);
	}

	// Client -> server receives 
	void RPC_ZenDeleteAllOnMap(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!GetPermissionsManager().HasPermission("Entity.Delete", sender))
			return;

		Param1<string> data;
		if (!ctx.Read(data))
		{
			Error("Failed to read data");
			return;
		}

		string className = data.param1;

		if (className == "")
			return;

		int deleteCount = ZenEntityTracker.DeleteAllEntities(className);
		COTCreateNotification(sender, new StringLocaliser("#STR_ZenCOT_Deleted " + deleteCount + "x " + className));
		Print("[ZenCOT] " + sender.GetId() + " requested full map delete of all items=" + className + " x" + deleteCount);
	}
	
	// Server -> client receives
	void RPC_ZenShowOnMapSend(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!GetPermissionsManager().HasPermission("Admin.Map.View"))
			return;
		
		Param2<ref array<string>, ref array<vector>> data;
		if (!ctx.Read(data))
		{
			Error("Failed to read data");
			return;
		}
		
		array<string> itemNames = data.param1;
		array<vector> itemPositions = data.param2;
	
		Print("[ZenCOT] Received marker count=" + itemNames.Count() + "/" + itemPositions.Count());
		
		if (itemNames.Count() != itemPositions.Count())
		{
			Error("Array mismatch!");
			return;
		}
	
		array<ref JMMapTempMark> marks = new array<ref JMMapTempMark>;
		for (int i = 0; i < itemNames.Count(); i++)
		{
			vector pos = itemPositions.Get(i); 
			string label = Widget.TranslateString(itemNames.Get(i));
	
			JMMapTempMark m = new JMMapTempMark();
			m.m_Pos   = pos;
			m.m_Label = label;
			marks.Insert(m);
		}
	
		JMMapModule mapMod;
		if (Class.CastTo(mapMod, GetModuleManager().GetModule(JMMapModule)))
		{
			mapMod.SetTempItemMarks(marks, "Markers");
			mapMod.Show();
		}
	}

	// Copy object info @ cursor to clipboard
	void ZenCopyPositionOnCursor(UAInput input)
	{
		if (!g_Game)
			return;
		
		if (!GetCommunityOnlineToolsBase().IsActive())
		{
			//COTCreateLocalAdminNotification(new StringLocaliser("STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF"));
			return;
		}

		if (g_Game.GetTime() - m_ZenLastKeypress < 1000)
		{
			return;
		}
			
		m_ZenLastKeypress = g_Game.GetTime();

		string notification = "";
		string pos;
		string ori;
		
		Object objTarget = ZenCOT_GetObjectAtCrosshair();
		if (!objTarget || g_Game.IsLeftCtrlDown())
			objTarget = g_Game.GetPlayer();

		if (!objTarget)
			return;

		string objType = ZenEntityTracker.ZenCOT_GetObjectName(objTarget);
		pos = objTarget.GetPosition().ToString(false);
		ori = objTarget.GetOrientation().ToString(false);
		notification = "#STR_ZenCOT_Copied \n\n" + objType + " @\n\n" + pos + "\n\n#STR_ZenCOT_Orientation " + ori;

		if (!g_Game.ZenCOT_IsLeftAltDown())
		{
			array<string> split = new array<string>;
			
			pos.Split(" ", split);
			if (split.Count() == 3)
			{
				pos = split[0] + ", " + split[1] + ", " + split[2];
			}

			split.Clear();

			ori.Split(" ", split);
			if (split.Count() == 3)
			{
				ori = split[0] + ", " + split[1] + ", " + split[2];
			}
		}

		g_Game.CopyToClipboard(objType + "\nPos: " + pos + "\nOri: " + ori + "\n\n");
		COTCreateLocalAdminNotification(new StringLocaliser(notification), "set:ccgui_enforce image:HudBuild", 10);
	}

	// Alternative to COT's get object at which ignores Land_ and static objects.
	Object ZenCOT_GetObjectAtCrosshair(float distance = 1000.0)
	{
	    vector rayStart = g_Game.GetCurrentCameraPosition();
	    vector rayEnd = rayStart + g_Game.GetCurrentCameraDirection() * distance;
	    auto objs = GetObjectsAt(rayStart, rayEnd, g_Game.GetPlayer(), 0, null);
		Object defaultObj;

	    if (objs != null && objs.Count() > 0)
		{
			defaultObj = objs.Get(0);
			foreach (Object obj : objs)
			{
				if (obj && ZenEntityTracker.ZenCOT_GetObjectName(obj) != "")
					return obj;
			}
			
	        return objs.Get(0);
		}

	    return NULL;
	}
	
	override private void Server_DeleteEntity( notnull Object obj, PlayerIdentity ident )
	{
		// NO EXPANSION MOD FOUND? LET VANILLA COT TAKE OVER.
		#ifndef DZ_Expansion_AI
		super.Server_DeleteEntity(obj, ident);
		return;
		#endif
		
		// COPY & PASTED FROM COT 14th April 2026 - ONLY CHANGE = ALLOWING EXPANSION AI PLAYERS TO BE DELETED.
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Entity.Delete", ident, instance ) )
			return;

		PlayerBase player;
		if ( Class.CastTo( player, obj ) && ( player.IsAlive() || player.GetIdentity() != null ) )
		{
			#ifndef DZ_Expansion_AI
			return;
			#else 
			if (!player.IsAI())
				return;
			#endif
		}

		string obtype = Object.GetDebugName( obj );

		vector transform[4];
		obj.GetTransform( transform );

		g_Game.ObjectDelete( obj );
		
		GetCommunityOnlineToolsBase().Log( ident, "Deleted Entity " + obtype + " at " + transform[3].ToString() );
		SendWebhook( "Delete", instance, "Deleted object " + obtype + " at " + transform[3].ToString() );
	}
	
	override Object GetObjectAtCursor(bool ignorePlayers = true, float distance = 3.0)
	{ 
		// NO EXPANSION MOD FOUND? LET VANILLA COT TAKE OVER.
		#ifndef DZ_Expansion_AI
		return super.GetObjectAtCursor(ignorePlayers, distance);
		#endif

		// COPY & PASTED FROM COT 14th April 2026 - ONLY CHANGE = ALLOWING EXPANSION AI PLAYERS TO BE DELETED.
		vector rayStart = g_Game.GetCurrentCameraPosition();
		
		DayZPlayer player = g_Game.GetPlayer();
		DayZPlayerCamera3rdPerson camera3rdPerson;
		if (player && !CurrentActiveCamera && Class.CastTo(camera3rdPerson, player.GetCurrentCamera()))
		{
			vector headPos = player.GetBonePositionWS(player.GetBoneIndexByName("Head"));
			distance += vector.Distance(rayStart, headPos);
		}

		vector rayEnd = rayStart + (g_Game.GetCurrentCameraDirection() * distance);

		RaycastRVParams rayInput = new RaycastRVParams(rayStart, rayEnd);

		//! Only ignore client player if not in freecam/spectator cam
		if (!CurrentActiveCamera || !CurrentActiveCamera.IsActive())
			rayInput.ignore = player;

		rayInput.flags = CollisionFlags.ALLOBJECTS;
		rayInput.radius = 0.1;
		array< ref RaycastRVResult > results = new array< ref RaycastRVResult >;

		Object obj;
		Object resultObj;
		TIntArray types = {ObjIntersectFire, ObjIntersectView};

		// Because way too many modders are too lazy...
		if (m_AllowRestrictedClassNames)
			types.Insert(ObjIntersectGeom);

		foreach (int type: types)
		{
			rayInput.type = type;

			if (!DayZPhysics.RaycastRVProxy(rayInput, results))
				continue;

			foreach ( RaycastRVResult result: results )
			{
				resultObj = result.obj;

				if ( resultObj == NULL )
					continue;

				if ((resultObj.IsBush() || resultObj.IsTree()) && type != ObjIntersectView)
					continue;

				EntityAI entity;
				if (Class.CastTo(entity, resultObj))
					resultObj = entity.GetHierarchyRoot();

				PlayerBase targetPlayer;
				if (PlayerBase.CastTo(targetPlayer, resultObj) && ignorePlayers)
				{
					#ifdef DZ_Expansion_AI
					if (!targetPlayer.IsAI())
						continue;
					#else 
						continue;
					#endif
				}

				string name = resultObj.GetType();

				if ( name == "" )
					continue;

				name.ToLower();

				if ( name == "#particlesourceenf" )
					continue;

				if ( !m_AllowRestrictedClassNames )
				{
					if (resultObj.ConfigGetInt("scope") != 2)
						continue;

					bool blacklisted = false;

					foreach (string blacklistedName: m_RestrictedClassNames)
					{
						if ( name.Contains(blacklistedName) )
						{
							blacklisted = true;
							break;
						}
					}

					if (blacklisted)
						continue;
				}

				obj = resultObj;
				break;
			}

			if (obj)
				break;
		}

		return obj;
	}

	// Add ability to hold CTRL down to delete far away targets.
	// Only difference = 1000.0 GetObjectAtCursor distance override (code last copied from COT on 22nd November 2025)
	override void DeleteCursor(UAInput input)
	{
		if (!g_Game)
			return;

		if (!g_Game.IsLeftCtrlDown() && !g_Game.ZenCOT_IsRightAltDown() && !g_Game.ZenCOT_IsLeftAltDown())
		{
			super.DeleteCursor(input);
			return;
		}

		if (!input.LocalPress())
			return;

		if (g_Game.GetUIManager().GetMenu())
			return;

		if (!GetPermissionsManager().HasPermission("Entity.Delete"))
			return;

		if (!GetCommunityOnlineToolsBase().IsActive())
		{
			COTCreateLocalAdminNotification(new StringLocaliser("STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF"));
			return;
		}

		Object obj = GetObjectAtCursor(true, 1000.0);

		if (!obj)
			return;

		// Insta-delete
		if (g_Game.ZenCOT_IsRightAltDown() && !g_Game.ZenCOT_IsLeftAltDown())
		{
			COTCreateLocalAdminNotification(new StringLocaliser("#STR_ZenCOT_Deleted " + obj + " @ " + obj.GetPosition().ToString(false)));
			DeleteEntity(obj);
			return;
		}

		m_AutoShow = !IsVisible();
		if (m_AutoShow)
			Show();

		JMObjectSpawnerForm form;
		if (!Class.CastTo(form, GetForm()))
			return;

		// Delete all items on map
		if (g_Game.ZenCOT_IsRightAltDown() && g_Game.ZenCOT_IsLeftAltDown())
		{
			form.m_ZenDeleteAllEntityType = obj.GetType();
			form.CreateConfirmation_Two(JMConfirmationType.INFO, "#STR_COT_GENERIC_CONFIRM", string.Format(Widget.TranslateString("#STR_ZenCOT_DeleteWarning"), form.m_ZenDeleteAllEntityType), "#STR_COT_GENERIC_NO", "ZenDeleteAll_No", "#STR_COT_GENERIC_YES", "ZenDeleteAll_Yes");
			return;
		}

		form.DeleteCursor(obj);
	}
}