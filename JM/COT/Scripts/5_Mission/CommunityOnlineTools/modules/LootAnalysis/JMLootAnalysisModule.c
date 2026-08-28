// ---------------------------------------------------------------------------
//  CE XML data containers (parsed once at server start)
// ---------------------------------------------------------------------------

// types.xml entry: one item -> its usages and category
class JMCETypeData
{
	string m_ClassName;
	ref array<string> m_Usages;   // e.g. ["Military","Police"]
	string m_Category;            // e.g. "weapons"

	void JMCETypeData()
	{
		m_Usages = new array<string>;
	}
}

// mapgroupproto.xml entry: one building type -> its accepted usages
class JMCEGroupProto
{
	string m_GroupName;
	ref array<string> m_Usages;

	void JMCEGroupProto()
	{
		m_Usages = new array<string>;
	}
}

// ---------------------------------------------------------------------------

class JMLootDistributionData
{
	string m_ClassName;
	int m_TotalCount;
	float m_AverageHealth;
	float m_AverageQuantity;
	float m_AverageLifetime;
	ref array<vector> m_Positions;

	void JMLootDistributionData()
	{
		m_Positions = new array<vector>;
	}
}

class JMLootAnalysisModule: JMRenderableModuleBase
{
	// CE data cached at startup (server only)
	private ref map<string, ref JMCETypeData>  m_CETypes;       // classname -> type data
	private ref map<string, ref JMCEGroupProto> m_CEProtos;     // group name -> gprProto data
	private ref map<string, ref array<vector>>  m_CEPositions;  // group name -> world positions
	private bool m_CEDataLoaded = false;

	void JMLootAnalysisModule()
	{
		GetPermissionsManager().RegisterPermission("Admin.LootAnalysis.View");
		GetPermissionsManager().RegisterPermission("Admin.LootAnalysis.ItemScan");
		GetPermissionsManager().RegisterPermission("Admin.LootAnalysis.Distribution");
		GetPermissionsManager().RegisterPermission("Admin.LootAnalysis.Delete");

		GetRPCManager().AddRPC("JM_COT_RPC", "RPC_RequestItemScan", this, SingeplayerExecutionType.Both);
		GetRPCManager().AddRPC("JM_COT_RPC", "RPC_SendItemScanResults", this, SingeplayerExecutionType.Both);
		GetRPCManager().AddRPC("JM_COT_RPC", "RPC_RequestLootDistribution", this, SingeplayerExecutionType.Both);
		GetRPCManager().AddRPC("JM_COT_RPC", "RPC_SendLootDistribution", this, SingeplayerExecutionType.Both);
		GetRPCManager().AddRPC("JM_COT_RPC", "RPC_DeleteAllItems", this, SingeplayerExecutionType.Both);
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission("Admin.LootAnalysis.View");
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/loot_analysis_form.layout";
	}

	override string GetCategory()
	{
		return "World";
	}

	override string GetTitle()
	{
		return "Loot Analysis";
	}

	override string GetIconName()
	{
		return JMConstants.Lucide( "package-search" );
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
	}

	override string GetWebhookTitle()
	{
		return "Loot Analysis";
	}

	override void GetWebhookTypes( out array<string> types )
	{
		types.Insert( "ItemScan"     );
		types.Insert( "Distribution" );
		types.Insert( "Delete"       );
	}

	// -----------------------------------------------------------------------
	//  CE data loading - called on server mission load
	// -----------------------------------------------------------------------

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if (IsMissionHost())
			Server_LoadCEData();
	}

	private void Server_LoadCEData()
	{
		m_CETypes     = new map<string, ref JMCETypeData>;
		m_CEProtos    = new map<string, ref JMCEGroupProto>;
		m_CEPositions = new map<string, ref array<vector>>;

		Server_ParseAllTypesXml();
		Server_ParseMapGroupProtoXml();
		Server_ParseMapGroupPosXml();

		m_CEDataLoaded = true;
	}

	// Discover every types.xml the mission registers, not just the vanilla one.
	//
	// $mission:db/types.xml is only the base game's list. Mods and custom maps
	// register their own via cfgeconomycore.xml:
	//
	//   <economycore>
	//     <ce folder="mymod">
	//       <file name="mymod_types.xml" type="types" />
	//     </ce>
	//   </economycore>
	//
	// Parsing only the hardcoded path silently drops every modded item from the
	// analysis. See docs/audits/2026-07-15-todo-mods-audit.md.
	private void Server_ParseAllTypesXml()
	{
		array<string> files = new array<string>;
		files.Insert("$mission:db/types.xml");

		Server_FindRegisteredTypesFiles(files);

		int parsed = 0;
		foreach (string path : files)
		{
			if (Server_ParseTypesXml(path))
				parsed++;
		}

		Print("[LootAnalysis] parsed " + parsed + "/" + files.Count() + " types file(s), " + m_CETypes.Count() + " CE type(s)");
	}

	// Walk cfgeconomycore.xml for <ce folder="X"><file name="Y" type="types"/>
	// and append $mission:X/Y for each. A missing or ce-less cfgeconomycore is
	// normal (vanilla missions have none) -- not an error.
	private void Server_FindRegisteredTypesFiles(inout array<string> files)
	{
		CF_XML_Document coreDoc;
		if (!CF_XML.ReadDocument("$mission:cfgeconomycore.xml", coreDoc))
			return;

		array<CF_XML_Tag> coreRoots = coreDoc.Get("economycore");
		if (!coreRoots || coreRoots.Count() == 0)
			return;

		CF_XML_Element coreEl = coreRoots[0].GetContent();
		if (!coreEl)
			return;

		array<CF_XML_Tag> ceTags = coreEl.Get("ce");
		if (!ceTags)
			return;

		foreach (CF_XML_Tag ceTag : ceTags)
		{
			CF_XML_Attribute folderAttr = ceTag.GetAttribute("folder");
			if (!folderAttr)
				continue;

			string folder = folderAttr.GetValue();
			if (folder == "")
				continue;

			CF_XML_Element ceEl = ceTag.GetContent();
			if (!ceEl)
				continue;

			array<CF_XML_Tag> fileTags = ceEl.Get("file");
			if (!fileTags)
				continue;

			foreach (CF_XML_Tag fileTag : fileTags)
			{
				CF_XML_Attribute typeAttr = fileTag.GetAttribute("type");
				if (!typeAttr || typeAttr.GetValue() != "types")
					continue;

				CF_XML_Attribute nameAttr2 = fileTag.GetAttribute("name");
				if (!nameAttr2)
					continue;

				string fileName = nameAttr2.GetValue();
				if (fileName == "")
					continue;

				files.Insert("$mission:" + folder + "/" + fileName);
			}
		}
	}

	// Parse one types.xml using CF_XML. Later files override earlier entries,
	// matching the CE's own last-registered-wins behaviour.
	// <types>
	//   <type name="AK101">
	//     <category name="weapons"/>
	//     <usage name="Military"/>
	//   </type>
	// </types>
	private bool Server_ParseTypesXml(string path)
	{
		CF_XML_Document doc;
		if (!CF_XML.ReadDocument(path, doc))
		{
			Print("[LootAnalysis] WARNING: could not parse " + path);
			return false;
		}

		// doc root is <types> - its tags are <type name="..."> entries
		// The document element wraps the root tag; Get("types") returns the root
		array<CF_XML_Tag> typesRoots = doc.Get("types");
		if (!typesRoots || typesRoots.Count() == 0)
		{
			Print("[LootAnalysis] WARNING: no <types> root in " + path);
			return false;
		}

		CF_XML_Element typesEl = typesRoots[0].GetContent();
		if (!typesEl)
			return false;

		array<CF_XML_Tag> typeTags = typesEl.Get("type");
		if (!typeTags)
			return false;

		foreach (CF_XML_Tag typeTag : typeTags)
		{
			CF_XML_Attribute nameAttr = typeTag.GetAttribute("name");
			if (!nameAttr)
				continue;

			string typeName = nameAttr.GetValue();
			if (typeName == "")
				continue;

			JMCETypeData data = new JMCETypeData();
			data.m_ClassName = typeName;

			CF_XML_Element typeEl = typeTag.GetContent();
			if (typeEl)
			{
				// Collect usages
				array<CF_XML_Tag> usageTags = typeEl.Get("usage");
				if (usageTags)
				{
					foreach (CF_XML_Tag usageTag : usageTags)
					{
						CF_XML_Attribute usageAttr = usageTag.GetAttribute("name");
						if (usageAttr)
						{
							string usageName = usageAttr.GetValue();
							if (usageName != "")
								data.m_Usages.Insert(usageName);
						}
					}
				}

				// Collect category (take the first one)
				array<CF_XML_Tag> catTags = typeEl.Get("category");
				if (catTags && catTags.Count() > 0)
				{
					CF_XML_Attribute catAttr = catTags[0].GetAttribute("name");
					if (catAttr)
						data.m_Category = catAttr.GetValue();
				}
			}

			string typeNameLower = typeName;
			typeNameLower.ToLower();
			//! Set, not Insert: a modded types file may redefine a vanilla
			//! entry, and Insert would leave both in the map.
			m_CETypes.Set(typeNameLower, data);
		}

		return true;
	}

	// Parse $mission:mapgroupproto.xml using CF_XML
	// <prototype>
	//   <group name="Land_Shed_M1" lootmax="2">
	//     <usage name="Industrial"/>
	//     <usage name="Farm"/>
	//   </group>
	// </prototype>
	private void Server_ParseMapGroupProtoXml()
	{
		CF_XML_Document doc;
		if (!CF_XML.ReadDocument("$mission:mapgroupproto.xml", doc))
		{
			Print("[LootAnalysis] WARNING: could not parse $mission:mapgroupproto.xml");
			return;
		}

		array<CF_XML_Tag> protoRoots = doc.Get("prototype");
		if (!protoRoots || protoRoots.Count() == 0)
		{
			Print("[LootAnalysis] WARNING: no <prototype> root in mapgroupproto.xml");
			return;
		}

		CF_XML_Element protoEl = protoRoots[0].GetContent();
		if (!protoEl)
			return;

		array<CF_XML_Tag> groupTags = protoEl.Get("group");
		if (!groupTags)
			return;

		foreach (CF_XML_Tag groupTag : groupTags)
		{
			CF_XML_Attribute nameAttr = groupTag.GetAttribute("name");
			if (!nameAttr)
				continue;

			string groupName = nameAttr.GetValue();
			if (groupName == "")
				continue;

			JMCEGroupProto groupProtoEntry = new JMCEGroupProto();
			groupProtoEntry.m_GroupName = groupName;

			CF_XML_Element groupEl = groupTag.GetContent();
			if (groupEl)
			{
				array<CF_XML_Tag> usageTags = groupEl.Get("usage");
				if (usageTags)
				{
					foreach (CF_XML_Tag usageTag : usageTags)
					{
						CF_XML_Attribute usageAttr = usageTag.GetAttribute("name");
						if (usageAttr)
						{
							string usageName = usageAttr.GetValue();
							if (usageName != "")
								groupProtoEntry.m_Usages.Insert(usageName);
						}
					}
				}
			}

			string groupNameLower = groupName;
			groupNameLower.ToLower();
			m_CEProtos.Insert(groupNameLower, groupProtoEntry);
		}
	}

	// Parse $mission:mapgrouppos.xml using CF_XML
	// <map>
	//   <group name="Land_Shed_M1" pos="80.26 113.79 4422.18" .../>
	// </map>
	private void Server_ParseMapGroupPosXml()
	{
		CF_XML_Document doc;
		if (!CF_XML.ReadDocument("$mission:mapgrouppos.xml", doc))
		{
			Print("[LootAnalysis] WARNING: could not parse $mission:mapgrouppos.xml");
			return;
		}

		array<CF_XML_Tag> mapRoots = doc.Get("map");
		if (!mapRoots || mapRoots.Count() == 0)
		{
			Print("[LootAnalysis] WARNING: no <map> root in mapgrouppos.xml");
			return;
		}

		CF_XML_Element mapEl = mapRoots[0].GetContent();
		if (!mapEl)
			return;

		array<CF_XML_Tag> groupTags = mapEl.Get("group");
		if (!groupTags)
			return;

		foreach (CF_XML_Tag groupTag : groupTags)
		{
			CF_XML_Attribute nameAttr = groupTag.GetAttribute("name");
			if (!nameAttr)
				continue;

			string groupName = nameAttr.GetValue();
			if (groupName == "")
				continue;

			CF_XML_Attribute posAttr = groupTag.GetAttribute("pos");
			if (!posAttr)
				continue;

			// CF_XML_Attribute.ValueAsVector() parses "X Y Z" directly
			vector pos = posAttr.ValueAsVector();

			string groupNameLower = groupName;
			groupNameLower.ToLower();

			array<vector> positions;
			if (!m_CEPositions.Find(groupNameLower, positions))
			{
				positions = new array<vector>;
				m_CEPositions.Insert(groupNameLower, positions);
			}
			positions.Insert(pos);
		}
	}

	// -----------------------------------------------------------------------
	//  RPC handlers
	// -----------------------------------------------------------------------

	// Client -> Server: Request item scan
	private void RPC_RequestItemScan(CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Admin.LootAnalysis.ItemScan", senderRPC, instance))
			return;

		Param1<string> data;
		if (!ctx.Read(data))
			return;

		string className = data.param1;
		if (className == "")
			return;

		GetCommunityOnlineToolsBase().Log(senderRPC, "Item scan: " + className);
		SendWebhook("ItemScan", instance, "Scanned for: " + className);

		Server_ScanItems(className, senderRPC);
	}

	// Server -> Client: Send item scan results
	private void RPC_SendItemScanResults(CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!GetPermissionsManager().HasPermission("Admin.LootAnalysis.ItemScan"))
			return;

		Param2<ref array<string>, ref array<vector>> data;
		if (!ctx.Read(data))
			return;

		array<string> itemNames = data.param1;
		array<vector> itemPositions = data.param2;

		Client_ShowItemsOnMap(itemNames, itemPositions);
	}

	// Client -> Server: Request loot distribution (CE spawn locations)
	private void RPC_RequestLootDistribution(CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Admin.LootAnalysis.Distribution", senderRPC, instance))
			return;

		Param1<string> data;
		if (!ctx.Read(data))
			return;

		string className = data.param1;
		if (className == "")
		{
			COTCreateNotification(senderRPC, new StringLocaliser("No classname provided for distribution"));
			return;
		}

		GetCommunityOnlineToolsBase().Log(senderRPC, "CE spawn lookup: " + className);
		SendWebhook("Distribution", instance, "CE spawn lookup: " + className);

		Server_GetCESpawnLocations(className, senderRPC);
	}

	// Server -> Client: Send loot distribution results
	private void RPC_SendLootDistribution(CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!GetPermissionsManager().HasPermission("Admin.LootAnalysis.Distribution"))
			return;

		Param1<ref array<ref JMLootDistributionData>> data;
		if (!ctx.Read(data))
			return;

		array<ref JMLootDistributionData> distributionData = data.param1;
		Client_ShowLootDistribution(distributionData);
	}

	// Client -> Server: Delete all items of type
	private void RPC_DeleteAllItems(CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!GetPermissionsManager().HasPermission("Admin.LootAnalysis.Delete", senderRPC))
			return;

		Param1<string> data;
		if (!ctx.Read(data))
			return;

		string className = data.param1;
		if (className == "")
			return;

		int deleteCount = JMEntityTracker.DeleteAllEntities(className);
		COTCreateNotification(senderRPC, new StringLocaliser("Deleted " + deleteCount + "x " + className));

		GetCommunityOnlineToolsBase().Log(senderRPC, "Deleted all items: " + className + " (count: " + deleteCount + ")");

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer(senderRPC.GetId());
		if (instance)
		{
			SendWebhookColored("Delete", instance, "Deleted all items: " + className + " (count: " + deleteCount + ")", JMConstants.WEBHOOK_COLOR_CRITICAL);
		}
	}

	// -----------------------------------------------------------------------
	//  Server logic
	// -----------------------------------------------------------------------

	// Server: Scan for spawned items and send positions to client
	private void Server_ScanItems(string className, PlayerIdentity senderRPC)
	{
		array<EntityAI> found = new array<EntityAI>;
		JMEntityTracker.GetByClassname(className, found);

		if (!found || found.Count() == 0)
		{
			COTCreateNotification(senderRPC, new StringLocaliser("No spawned items found: " + className));
			return;
		}

		array<string> names = new array<string>;
		array<vector> positions = new array<vector>;

		foreach (EntityAI entity : found)
		{
			if (!entity)
				continue;

			string displayName = "";

			// Add lifetime info
			float lifetime = entity.GetLifetime();
			if (lifetime > 0)
			{
				int totalMinutes = Math.Floor(lifetime / 60.0);
				int totalHours = Math.Floor(totalMinutes / 60.0);
				int totalDays = Math.Floor(totalHours / 24.0);

				if (totalDays >= 1)
					displayName = "~" + totalDays + "d";
				else if (totalHours >= 1)
					displayName = "~" + totalHours + "h";
				else
					displayName = "~" + totalMinutes + "m";
			}

			// Add quantity info
			if (entity.HasQuantity())
			{
				float qty = entity.GetQuantityNormalized() * 100;
				displayName += " | " + (int)qty + "%Q";
			}

			// Add health info
			float maxHealth = entity.GetMaxHealth();
			if (maxHealth > 0)
			{
				float hp = (entity.GetHealth() / maxHealth) * 100;
				displayName += " | " + (int)hp + "%H";
			}

			names.Insert(displayName);
			positions.Insert(entity.GetPosition());
		}

		GetRPCManager().SendRPC("JM_COT_RPC", "RPC_SendItemScanResults", new Param2<ref array<string>, ref array<vector>>(names, positions), true, senderRPC);
	}

	// Server: Find CE potential spawn locations for a classname using XML data
	private void Server_GetCESpawnLocations(string className, PlayerIdentity senderRPC)
	{
		if (!m_CEDataLoaded)
		{
			COTCreateNotification(senderRPC, new StringLocaliser("CE data not loaded yet. Try again in a moment."));
			return;
		}

		// Look up type data (case-insensitive)
		string classNameLower = className;
		classNameLower.ToLower();

		JMCETypeData typeData;
		if (!m_CETypes.Find(classNameLower, typeData) || !typeData)
		{
			COTCreateNotification(senderRPC, new StringLocaliser("No CE data found for: " + className));
			return;
		}

		if (typeData.m_Usages.Count() == 0)
		{
			COTCreateNotification(senderRPC, new StringLocaliser("Item has no CE usages defined: " + className));
			return;
		}

		// Find all building groups that have at least one matching usage
		// Build a fast lookup set from the item's usages
		TStringSet itemUsages = new TStringSet;
		foreach (string usage : typeData.m_Usages)
		{
			string usageLower = usage;
			usageLower.ToLower();
			itemUsages.Insert(usageLower);
		}

		// Collect matching group names
		TStringSet matchingGroups = new TStringSet;
		for (int pi = 0; pi < m_CEProtos.Count(); pi++)
		{
			string protoKey = m_CEProtos.GetKey(pi);
			JMCEGroupProto gprProto = m_CEProtos.GetElement(pi);
			if (!gprProto)
				continue;

			foreach (string protoUsage : gprProto.m_Usages)
			{
				string protoUsageLower = protoUsage;
				protoUsageLower.ToLower();

				if (itemUsages.Find(protoUsageLower) > -1)
				{
					matchingGroups.Insert(protoKey);
					break;
				}
			}
		}

		if (matchingGroups.Count() == 0)
		{
			COTCreateNotification(senderRPC, new StringLocaliser("No matching CE buildings found for: " + className));
			return;
		}

		// Collect positions of all matching building groups
		array<string> markerNames = new array<string>;
		array<vector> markerPositions = new array<vector>;

		for (int gi = 0; gi < matchingGroups.Count(); gi++)
		{
			string groupKey = matchingGroups[gi];
			array<vector> groupPositions;
			if (!m_CEPositions.Find(groupKey, groupPositions) || !groupPositions)
				continue;

			// Reconstruct the display name from the gprProto data
			JMCEGroupProto groupProto;
			m_CEProtos.Find(groupKey, groupProto);
			string displayGroupName = groupKey;
			if (groupProto)
				displayGroupName = groupProto.m_GroupName;

			foreach (vector gPos : groupPositions)
			{
				markerNames.Insert("");
				markerPositions.Insert(gPos);
			}
		}

		if (markerPositions.Count() == 0)
		{
			COTCreateNotification(senderRPC, new StringLocaliser("No CE building positions found for: " + className));
			return;
		}

		// Send as item scan results so client reuses the same map marker pipeline
		GetRPCManager().SendRPC("JM_COT_RPC", "RPC_SendItemScanResults", new Param2<ref array<string>, ref array<vector>>(markerNames, markerPositions), true, senderRPC);
	}

	// -----------------------------------------------------------------------
	//  Client logic
	// -----------------------------------------------------------------------

	// Client: Show items on map (used by both Item Scan and Distribution)
	private void Client_ShowItemsOnMap(array<string> itemNames, array<vector> itemPositions)
	{
		if (itemNames.Count() != itemPositions.Count())
			return;

		string message = "Found " + itemNames.Count() + " locations. Opening map...";
		COTCreateLocalAdminNotification(new StringLocaliser(message));

		JMMapModule mapMod;
		if (!Class.CastTo(mapMod, GetModuleManager().GetModule(JMMapModule)))
			return;

		// Show() is synchronous - form is valid immediately after
		if (!mapMod.IsVisible())
			mapMod.Show();

		mapMod.SetLootMarkers(itemNames, itemPositions);
	}

	// Client: Show loot distribution (legacy entity-scan path, kept for compatibility)
	private void Client_ShowLootDistribution(array<ref JMLootDistributionData> distributionData)
	{
		JMLootAnalysisForm form;
		if (Class.CastTo(form, GetForm()))
		{
			form.UpdateDistributionData(distributionData);
		}
	}

	// -----------------------------------------------------------------------
	//  Public API for form
	// -----------------------------------------------------------------------

	void RequestItemScan(string className)
	{
		GetRPCManager().SendRPC("JM_COT_RPC", "RPC_RequestItemScan", new Param1<string>(className), true);
	}

	void RequestLootDistribution(string className)
	{
		GetRPCManager().SendRPC("JM_COT_RPC", "RPC_RequestLootDistribution", new Param1<string>(className), true);
	}

	void DeleteAllItems(string className)
	{
		GetRPCManager().SendRPC("JM_COT_RPC", "RPC_DeleteAllItems", new Param1<string>(className), true);
	}

	void ShowItemsOnMap(string className)
	{
		RequestItemScan(className);
	}

	void ShowCESpawnLocations(string className)
	{
		RequestLootDistribution(className);
	}
}
