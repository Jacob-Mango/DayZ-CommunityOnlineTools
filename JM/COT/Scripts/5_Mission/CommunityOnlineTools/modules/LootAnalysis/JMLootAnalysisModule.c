// ---------------------------------------------------------------------------
//  CE XML data containers (parsed once at server start)
// ---------------------------------------------------------------------------

// types.xml entry: one item -> its usages, category and CE tuning values
class JMCETypeData
{
	string m_ClassName;
	ref array<string> m_Usages;   // e.g. ["Military","Police"]
	ref array<string> m_Tiers;    // <value name="Tier1"/> etc
	string m_Category;            // e.g. "weapons"
	int m_Nominal   = -1;
	int m_Min       = -1;
	int m_QuantMin  = -1;
	int m_QuantMax  = -1;
	int m_Lifetime  = -1;
	int m_Restock   = -1;
	int m_Cost      = -1;

	//! Which types.xml this entry was parsed from - a mission can register
	//! several (see Exec_FindRegisteredTypesFiles). Save_ItemTypeInfo needs
	//! this to know which file to patch; nothing else reads it.
	string m_SourceFile;

	void JMCETypeData()
	{
		m_Usages = new array<string>;
		m_Tiers  = new array<string>;
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

//! Sent server -> client alongside a scan / distribution request answers the
//! form's stats panel, not the map markers - so it is its own small RPC
//! rather than another field bolted onto the marker payloads.
class JMLootItemTypeInfo
{
	string m_ClassName;
	bool m_Found;
	string m_Category;
	int m_Nominal;
	int m_Min;
	int m_QuantMin;
	int m_QuantMax;
	int m_Lifetime;
	int m_Restock;
	int m_Cost;
	ref array<string> m_Usages;
	ref array<string> m_Tiers;

	void JMLootItemTypeInfo()
	{
		m_Usages = new array<string>;
		m_Tiers  = new array<string>;
	}
}

// ---------------------------------------------------------------------------

class JMLootAnalysisModule: JMRenderableModuleBase
{
	// CE data cached at startup (server only)
	protected ref map<string, ref JMCETypeData>  m_CETypes;       // classname -> type data
	protected ref map<string, ref JMCEGroupProto> m_CEProtos;     // group name -> gprProto data
	protected ref map<string, ref array<vector>>  m_CEPositions;  // group name -> world positions
	protected bool m_CEDataLoaded = false;

	//! Classname the last RequestItemScan was for (client only). The scan
	//! reply carries per-entity health/quantity/lifetime/attachments but not
	//! the classname itself - every entity in one reply is the same class the
	//! client already asked for, so it is cheaper to remember it here than to
	//! round-trip it back over the RPC.
	protected string m_LastScanClassName;

	void JMLootAnalysisModule()
	{
		GetRPCManager().AddRPC("JM_COT_RPC", "RPC_RequestItemScan", this, SingeplayerExecutionType.Both);
		GetRPCManager().AddRPC("JM_COT_RPC", "RPC_SendItemScanResults", this, SingeplayerExecutionType.Both);
		GetRPCManager().AddRPC("JM_COT_RPC", "RPC_RequestLootDistribution", this, SingeplayerExecutionType.Both);
		GetRPCManager().AddRPC("JM_COT_RPC", "RPC_SendLootSpawnLocations", this, SingeplayerExecutionType.Both);
		GetRPCManager().AddRPC("JM_COT_RPC", "RPC_SendItemTypeInfo", this, SingeplayerExecutionType.Both);
		GetRPCManager().AddRPC("JM_COT_RPC", "RPC_DeleteAllItems", this, SingeplayerExecutionType.Both);
		GetRPCManager().AddRPC("JM_COT_RPC", "RPC_SaveItemTypeInfo", this, SingeplayerExecutionType.Both);
		GetRPCManager().AddRPC("JM_COT_RPC", "RPC_RequestItemTypeInfo", this, SingeplayerExecutionType.Both);
	}

	override void EnableUpdate()
	{
	}

	//! Called on both client and server as the module registers, before the mission loads.
	override void DeclarePermissions()
	{
		super.DeclarePermissions();

		JMPermissions.Register(JMConstants.PERM_LOOTANALYSIS_VIEW);
		JMPermissions.Register(JMConstants.PERM_LOOTANALYSIS_ITEMSCAN);
		JMPermissions.Register(JMConstants.PERM_LOOTANALYSIS_DISTRIBUTION);
		JMPermissions.Register(JMConstants.PERM_LOOTANALYSIS_DELETE);
		//! Separate from Delete: writing to the server's own types.xml is a
		//! different order of trust than deleting spawned entities.
		JMPermissions.Register(JMConstants.PERM_LOOTANALYSIS_EDIT);
	}

	override void DescribeModule( JMModuleInfo info )
	{
		super.DescribeModule( info );

		info.Title = "Loot Analysis";
		info.WebhookTitle = "Loot Analysis";
		info.Icon = "package-search";
		info.Layout = "JM/COT/GUI/layouts/loot_analysis_form.layout";
		info.Category = JMSideBarConfig.CATEGORY_WORLD;
		info.ViewPermission = JMConstants.PERM_LOOTANALYSIS_VIEW;

		info.AddWebhookType( "ItemScan" );
		info.AddWebhookType( "Distribution" );
		info.AddWebhookType( "Delete" );
	}

	// -----------------------------------------------------------------------
	//  CE data loading - called on server mission load
	// -----------------------------------------------------------------------

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if (IsMissionHost())
			Exec_LoadCEData();
	}

	protected void Exec_LoadCEData()
	{
		m_CETypes     = new map<string, ref JMCETypeData>;
		m_CEProtos    = new map<string, ref JMCEGroupProto>;
		m_CEPositions = new map<string, ref array<vector>>;

		Exec_ParseAllTypesXml();
		Exec_ParseMapGroupProtoXml();
		Exec_ParseMapGroupPosXml();

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
	protected void Exec_ParseAllTypesXml()
	{
		array<string> files = new array<string>;
		files.Insert("$mission:db/types.xml");

		Exec_FindRegisteredTypesFiles(files);

		int parsed = 0;
		foreach (string path : files)
		{
			if (Exec_ParseTypesXml(path))
				parsed++;
		}

		Print("[LootAnalysis] parsed " + parsed + "/" + files.Count() + " types file(s), " + m_CETypes.Count() + " CE type(s)");
	}

	// Walk cfgeconomycore.xml for <ce folder="X"><file name="Y" type="types"/>
	// and append $mission:X/Y for each. A missing or ce-less cfgeconomycore is
	// normal (vanilla missions have none) -- not an error.
	protected void Exec_FindRegisteredTypesFiles(inout array<string> files)
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
	protected bool Exec_ParseTypesXml(string path)
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
			data.m_SourceFile = path;

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

				// Collect tiers - <value name="Tier1"/> etc
				array<CF_XML_Tag> valueTags = typeEl.Get("value");
				if (valueTags)
				{
					foreach (CF_XML_Tag valueTag : valueTags)
					{
						CF_XML_Attribute valueAttr = valueTag.GetAttribute("name");
						if (valueAttr)
						{
							string tierName = valueAttr.GetValue();
							if (tierName != "")
								data.m_Tiers.Insert(tierName);
						}
					}
				}

				// CE tuning values - simple <tag>123</tag> text-content elements
				data.m_Nominal  = ParseIntTag(typeEl, "nominal");
				data.m_Min      = ParseIntTag(typeEl, "min");
				data.m_QuantMin = ParseIntTag(typeEl, "quantmin");
				data.m_QuantMax = ParseIntTag(typeEl, "quantmax");
				data.m_Lifetime = ParseIntTag(typeEl, "lifetime");
				data.m_Restock  = ParseIntTag(typeEl, "restock");
				data.m_Cost     = ParseIntTag(typeEl, "cost");
			}

			string typeNameLower = typeName;
			typeNameLower.ToLower();
			//! Set, not Insert: a modded types file may redefine a vanilla
			//! entry, and Insert would leave both in the map.
			m_CETypes.Set(typeNameLower, data);
		}

		return true;
	}

	//! Read a simple text-content element - <tag>123</tag>, not <tag name="..."/>.
	//! CF_XML has no GetText()/GetInnerText(); the inner text is reached by
	//! calling GetContent() twice - once on the CF_XML_Tag to get its child
	//! CF_XML_Element, once on that element to get the string it holds.
	protected int ParseIntTag(CF_XML_Element parent, string tagName, int defaultVal = -1)
	{
		array<CF_XML_Tag> tags = parent.Get(tagName);
		if (!tags || tags.Count() == 0)
			return defaultVal;

		CF_XML_Element content = tags[0].GetContent();
		if (!content)
			return defaultVal;

		string text = content.GetContent();
		if (text == "")
			return defaultVal;

		return text.ToInt();
	}

	// Parse $mission:mapgroupproto.xml using CF_XML
	// <prototype>
	//   <group name="Land_Shed_M1" lootmax="2">
	//     <usage name="Industrial"/>
	//     <usage name="Farm"/>
	//   </group>
	// </prototype>
	protected void Exec_ParseMapGroupProtoXml()
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
	protected void Exec_ParseMapGroupPosXml()
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
	protected void RPC_RequestItemScan(CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		JMPlayerInstance instance;
		if (!senderRPC)
			return;

		if (!JMPermissions.HasRPC(JMConstants.PERM_LOOTANALYSIS_ITEMSCAN, senderRPC, instance))
			return;

		Param1<string> data;
		if (!ctx.Read(data))
			return;

		string className = data.param1;
		if (className == "")
			return;

		GetCommunityOnlineToolsBase().Log(senderRPC, "Item scan: " + className);
		SendWebhook("ItemScan", instance, "Scanned for: " + className);

		Exec_ScanItems(className, senderRPC);
		Exec_SendItemTypeInfo(className, senderRPC);
	}

	// Server -> Client: Send item scan results
	//
	// Health/quantity/lifetime/attachments ride as three more parallel arrays
	// rather than one array<ref DTO> - this codebase has no precedent for
	// serializing an array of custom classes over a Param, and parallel
	// primitive arrays are the pattern AddDistributionMarkers already proved
	// working for per-marker usage types. Attachments are ";"-joined per
	// entity, one string per entry, and split back out client-side.
	protected void RPC_SendItemScanResults(CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!JMPermissions.Has(JMConstants.PERM_LOOTANALYSIS_ITEMSCAN))
			return;

		Param8<ref array<string>, ref array<vector>, ref array<float>, ref array<float>, ref array<int>, ref array<string>, ref array<int>, ref array<string>> data;
		if (!ctx.Read(data))
			return;

		Client_ShowItemsOnMap(data.param1, data.param2, data.param3, data.param4, data.param5, data.param6, data.param7, data.param8);
	}

	// Client -> Server: Request loot distribution (CE spawn locations)
	protected void RPC_RequestLootDistribution(CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		JMPlayerInstance instance;
		if (!senderRPC)
			return;

		if (!JMPermissions.HasRPC(JMConstants.PERM_LOOTANALYSIS_DISTRIBUTION, senderRPC, instance))
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

		Exec_GetCESpawnLocations(className, senderRPC);
		Exec_SendItemTypeInfo(className, senderRPC);
	}

	// Server -> Client: Send CE spawn-location markers, one entry per matching
	// building group position, each tagged with the usage that matched it so
	// the client can colour/legend markers by spawn type.
	protected void RPC_SendLootSpawnLocations(CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!JMPermissions.Has(JMConstants.PERM_LOOTANALYSIS_DISTRIBUTION))
			return;

		Param3<ref array<string>, ref array<vector>, ref array<string>> data;
		if (!ctx.Read(data))
			return;

		Client_ShowLootSpawnLocations(data.param1, data.param2, data.param3);
	}

	// Server -> Client: Send the CE tuning values for the classname a scan or
	// distribution request just asked about - feeds the form's stats panel.
	protected void RPC_SendItemTypeInfo(CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		Param1<ref JMLootItemTypeInfo> data;
		if (!ctx.Read(data))
			return;

		JMLootAnalysisForm form;
		if (Class.CastTo(form, GetForm()))
			form.SetItemTypeInfo(data.param1);
	}

	// Client -> Server: Delete all items of type
	protected void RPC_DeleteAllItems(CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!senderRPC)
			return;

		if (!JMPermissions.HasRPC(JMConstants.PERM_LOOTANALYSIS_DELETE, senderRPC))
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

	// Client -> Server: Save edited Nominal/Min/Lifetime/Restock back to the
	// classname's source types.xml.
	protected void RPC_SaveItemTypeInfo(CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		JMPlayerInstance instance;
		if (!senderRPC)
			return;

		if (!JMPermissions.HasRPC(JMConstants.PERM_LOOTANALYSIS_EDIT, senderRPC, instance))
			return;

		Param5<string, int, int, int, int> data;
		if (!ctx.Read(data))
			return;

		string className = data.param1;
		if (className == "")
			return;

		bool ok = Exec_SaveTypeXmlValues(className, data.param2, data.param3, data.param4, data.param5);

		if (ok)
		{
			COTCreateNotification(senderRPC, new StringLocaliser("Saved " + className + " - restart or reload the economy for it to affect spawns"));
			GetCommunityOnlineToolsBase().Log(senderRPC, "Edited CE data: " + className);
			SendWebhookColored("Edit", instance, "Edited CE data: " + className, JMConstants.WEBHOOK_COLOR_CRITICAL);
		}
		else
		{
			COTCreateNotification(senderRPC, new StringLocaliser("Failed to save " + className + " - see server log"));
		}

		//! Re-send fresh info either way, so the card reflects exactly what
		//! is now on disk (or the untouched values, if the save failed).
		Exec_SendItemTypeInfo(className, senderRPC);
	}

	// Client -> Server: reload Item Info for a classname without re-running a
	// scan or distribution search - the header Reload button. Gated on either
	// Loot Analysis permission rather than one specific tab's, since the same
	// button exists on both Item Scan's and Distribution's Item Info card.
	protected void RPC_RequestItemTypeInfo(CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		JMPlayerInstance instance;
		if (!senderRPC)
			return;

		bool canScan = JMPermissions.HasRPC(JMConstants.PERM_LOOTANALYSIS_ITEMSCAN, senderRPC, instance);
		bool canDist = JMPermissions.HasRPC(JMConstants.PERM_LOOTANALYSIS_DISTRIBUTION, senderRPC, instance);
		if (!canScan && !canDist)
			return;

		Param1<string> data;
		if (!ctx.Read(data))
			return;

		string className = data.param1;
		if (className == "")
			return;

		Exec_SendItemTypeInfo(className, senderRPC);
	}

	// -----------------------------------------------------------------------
	//  Server logic
	// -----------------------------------------------------------------------

	// Server: Scan for spawned items and send positions to client
	protected void Exec_ScanItems(string className, PlayerIdentity senderRPC)
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

		// Parallel arrays, one entry per entity, matched by index - feeds the
		// map marker hover preview (JMLootAnalysisHoverInfo). -1 marks "this
		// entity has no such stat" the way maxHealth<=0 already did for %H.
		array<float> healthPct = new array<float>;
		array<float> quantityPct = new array<float>;
		array<int> lifetimeSeconds = new array<int>;
		array<string> attachmentsJoined = new array<string>;

		//! One string per entity: the entity's own preview state, then one per attachment (same order as
		//! attachmentsJoined), ";"-separated - see JMLootPreviewState. Lets the client draw the model as
		//! it really is (ruined, half-eaten, rotten) instead of pristine.
		array<string> previewStates = new array<string>;

		//! -1 for anything that is not a firearm - same "N/A" sentinel every
		//! other per-entity stat here uses. A firearm's own GetQuantityNormalized()
		//! means nothing (it is not stacked/eaten), so ammo replaces quantity
		//! for these entries entirely rather than riding alongside it.
		array<int> ammoCount = new array<int>();

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

				lifetimeSeconds.Insert((int)lifetime);
			}
			else
			{
				lifetimeSeconds.Insert(-1);
			}

			// Add quantity info
			if (entity.HasQuantity())
			{
				float qty = entity.GetQuantityNormalized();
				displayName += " | " + (int)(qty * 100) + "%Q";
				quantityPct.Insert(qty);
			}
			else
			{
				quantityPct.Insert(-1);
			}

			// Ammo count (firearms only) - magazine rounds across every muzzle
			// plus a chambered round, the same total the in-hand ammo counter
			// itself reads off Weapon_Base.
			if (entity.IsWeapon())
			{
				Weapon_Base wpn = Weapon_Base.Cast(entity);
				int ammo = 0;

				if (wpn)
				{
					int muzzles = wpn.GetMuzzleCount();
					for (int mi = 0; mi < muzzles; mi++)
					{
						Magazine mag = Magazine.Cast(wpn.GetMagazine(mi));
						if (mag)
							ammo += mag.GetAmmoCount();

						if (!wpn.IsChamberEmpty(mi))
							ammo += 1;
					}
				}

				ammoCount.Insert(ammo);
			}
			else
			{
				ammoCount.Insert(-1);
			}

			// Add health info
			float maxHealth = entity.GetMaxHealth();
			if (maxHealth > 0)
			{
				float hpFrac = entity.GetHealth() / maxHealth;
				displayName += " | " + (int)(hpFrac * 100) + "%H";
				healthPct.Insert(hpFrac);
			}
			else
			{
				healthPct.Insert(-1);
			}

			string attachCsv = "";
			string stateCsv = JMLootPreviewState.FromEntity(entity).Encode();
			int attachCount = entity.GetInventory().AttachmentCount();
			for (int a = 0; a < attachCount; a++)
			{
				EntityAI attachment = entity.GetInventory().GetAttachmentFromIndex(a);
				if (!attachment)
					continue;

				if (attachCsv != "")
					attachCsv += ";";
				attachCsv += attachment.GetType();
				stateCsv += ";" + JMLootPreviewState.FromEntity(attachment).Encode();
			}
			attachmentsJoined.Insert(attachCsv);
			previewStates.Insert(stateCsv);

			names.Insert(displayName);
			positions.Insert(entity.GetPosition());
		}

		GetRPCManager().SendRPC("JM_COT_RPC", "RPC_SendItemScanResults", new Param8<ref array<string>, ref array<vector>, ref array<float>, ref array<float>, ref array<int>, ref array<string>, ref array<int>, ref array<string>>(names, positions, healthPct, quantityPct, lifetimeSeconds, attachmentsJoined, ammoCount, previewStates), true, senderRPC);
	}

	// Server: Find CE potential spawn locations for a classname using XML data
	protected void Exec_GetCESpawnLocations(string className, PlayerIdentity senderRPC)
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

		// Collect matching group names, each with the usage tag that matched it -
		// that tag is what the client colours/legends the marker by.
		ref map<string, string> matchingGroups = new map<string, string>;
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
					matchingGroups.Insert(protoKey, protoUsage);
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
		array<string> markerTypes = new array<string>;

		for (int gi = 0; gi < matchingGroups.Count(); gi++)
		{
			string groupKey = matchingGroups.GetKey(gi);
			string matchedUsage = matchingGroups.GetElement(gi);

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
				markerNames.Insert(displayGroupName);
				markerPositions.Insert(gPos);
				markerTypes.Insert(matchedUsage);
			}
		}

		if (markerPositions.Count() == 0)
		{
			COTCreateNotification(senderRPC, new StringLocaliser("No CE building positions found for: " + className));
			return;
		}

		GetRPCManager().SendRPC("JM_COT_RPC", "RPC_SendLootSpawnLocations", new Param3<ref array<string>, ref array<vector>, ref array<string>>(markerNames, markerPositions, markerTypes), true, senderRPC);
	}

	// Server: look up the CE tuning values for a classname and send them to the
	// client's stats panel. A miss (no CE data loaded, or the item is not in
	// types.xml) still sends a reply - m_Found = false - so the panel can say
	// so instead of showing stale numbers from the previous lookup.
	protected void Exec_SendItemTypeInfo(string className, PlayerIdentity senderRPC)
	{
		JMLootItemTypeInfo info = new JMLootItemTypeInfo();
		info.m_ClassName = className;

		string classNameLower = className;
		classNameLower.ToLower();

		JMCETypeData typeData;
		if (m_CEDataLoaded && m_CETypes.Find(classNameLower, typeData) && typeData)
		{
			info.m_Found    = true;
			info.m_Category = typeData.m_Category;
			info.m_Nominal  = typeData.m_Nominal;
			info.m_Min      = typeData.m_Min;
			info.m_QuantMin = typeData.m_QuantMin;
			info.m_QuantMax = typeData.m_QuantMax;
			info.m_Lifetime = typeData.m_Lifetime;
			info.m_Restock  = typeData.m_Restock;
			info.m_Cost     = typeData.m_Cost;
			info.m_Usages   = typeData.m_Usages;
			info.m_Tiers    = typeData.m_Tiers;
		}

		GetRPCManager().SendRPC("JM_COT_RPC", "RPC_SendItemTypeInfo", new Param1<ref JMLootItemTypeInfo>(info), true, senderRPC);
	}

	// -----------------------------------------------------------------------
	//  Server: save edited CE values back to their source types.xml
	//
	//  CF_XML is read-only, so this does not re-serialize the document - it
	//  patches the raw file TEXT instead, replacing only the inner text of
	//  the four tags that changed inside the one <type name="X">...</type>
	//  block, byte-identical everywhere else (comments, spacing, every other
	//  type untouched). A full re-serialize would risk losing whatever an
	//  admin's own CE tooling wrote into that file; a targeted text patch
	//  cannot touch anything it was not told to.
	// -----------------------------------------------------------------------

	protected bool Exec_SaveTypeXmlValues(string className, int nominal, int min, int lifetime, int restock)
	{
		string key = className;
		key.ToLower();

		JMCETypeData typeData;
		if (!m_CEDataLoaded || !m_CETypes.Find(key, typeData) || !typeData)
		{
			Print("[LootAnalysis] Save failed: unknown classname " + className);
			return false;
		}

		if (typeData.m_SourceFile == "")
		{
			Print("[LootAnalysis] Save failed: no source file recorded for " + className);
			return false;
		}

		string path = typeData.m_SourceFile;

		string content;
		if (!Exec_ReadFileText(path, content))
		{
			Print("[LootAnalysis] Save failed: could not read " + path);
			return false;
		}

		string openType  = "<type name=\"" + className + "\">";
		string closeType = "</type>";

		int typeStart = content.IndexOf(openType);
		if (typeStart < 0)
		{
			Print("[LootAnalysis] Save failed: could not locate " + openType + " in " + path);
			return false;
		}

		int closeTagStart = content.IndexOfFrom(typeStart, closeType);
		if (closeTagStart < 0)
		{
			Print("[LootAnalysis] Save failed: could not find closing </type> for " + className + " in " + path);
			return false;
		}

		int blockEnd = closeTagStart + closeType.Length();
		string block = content.Substring(typeStart, blockEnd - typeStart);

		string newBlock = block;
		newBlock = ReplaceTagValue(newBlock, "nominal", nominal);
		newBlock = ReplaceTagValue(newBlock, "min", min);
		newBlock = ReplaceTagValue(newBlock, "lifetime", lifetime);
		newBlock = ReplaceTagValue(newBlock, "restock", restock);

		if (newBlock == block)
			return true; // nothing actually changed - not a failure, just a no-op

		//! Backed up before the live file is touched - the one file operation
		//! here that must never be skipped.
		Exec_WriteFileText(path + ".bak", content);

		string newContent = content.Substring(0, typeStart) + newBlock + content.Substring(blockEnd, content.Length() - blockEnd);

		if (!Exec_WriteFileText(path, newContent))
		{
			Print("[LootAnalysis] Save failed: could not write " + path);
			return false;
		}

		//! Keep the cache in step with what is now on disk, so the next
		//! Item Info request (this one's own reply included) already
		//! reflects it without needing a fresh parse of the file.
		typeData.m_Nominal  = nominal;
		typeData.m_Min      = min;
		typeData.m_Lifetime = lifetime;
		typeData.m_Restock  = restock;

		return true;
	}

	//! Replaces the inner text of the FIRST <tagName>...</tagName> found in
	//! `block` - safe to call unscoped like this only because the caller
	//! already scoped `block` to one <type> element, so there is exactly one
	//! nominal/min/lifetime/restock tag to find. Leaves `block` untouched if
	//! the tag is not present at all rather than inventing one - a type with
	//! no <min> today gets no <min> written in, matching how ParseIntTag
	//! reads it as -1 rather than 0.
	protected string ReplaceTagValue(string block, string tagName, int newValue)
	{
		string openTag  = "<" + tagName + ">";
		string closeTag = "</" + tagName + ">";

		int openIdx = block.IndexOf(openTag);
		if (openIdx < 0)
			return block;

		int valueStart = openIdx + openTag.Length();
		int closeIdx = block.IndexOfFrom(valueStart, closeTag);
		if (closeIdx < 0)
			return block;

		string before = block.Substring(0, valueStart);
		string after  = block.Substring(closeIdx, block.Length() - closeIdx);

		return before + newValue.ToString() + after;
	}

	//! Whole-file read via the engine's line-based FGets, rejoined with "\n" -
	//! the only text file I/O this engine exposes from script (see
	//! JMLoadoutModule's own loadout loader for the same pattern).
	protected bool Exec_ReadFileText(string path, out string content)
	{
		if (!FileExist(path))
			return false;

		FileHandle handle = OpenFile(path, FileMode.READ);
		if (!handle)
			return false;

		content = "";
		string line;
		bool first = true;

		while (FGets(handle, line) > 0)
		{
			if (!first)
				content += "\n";

			content += line;
			first = false;
		}

		CloseFile(handle);
		return true;
	}

	protected bool Exec_WriteFileText(string path, string content)
	{
		FileHandle handle = OpenFile(path, FileMode.WRITE);
		if (!handle)
			return false;

		FPrintln(handle, content);
		CloseFile(handle);
		return true;
	}

	// -----------------------------------------------------------------------
	//  Client logic
	// -----------------------------------------------------------------------

	// Client: Show item scan results on the Item Scan tab's map
	protected void Client_ShowItemsOnMap(array<string> itemNames, array<vector> itemPositions, array<float> healthPct, array<float> quantityPct, array<int> lifetimeSeconds, array<string> attachmentsJoined, array<int> ammoCount, array<string> previewStates)
	{
		if (itemNames.Count() != itemPositions.Count())
			return;

		JMLootAnalysisForm form;
		if (!Class.CastTo(form, GetForm()))
			return;

		array<ref JMLootScanItemDetail> details = new array<ref JMLootScanItemDetail>;

		for (int i = 0; i < itemPositions.Count(); i++)
		{
			TStringArray attachments = new TStringArray;
			if (i < attachmentsJoined.Count() && attachmentsJoined[i] != "")
				attachmentsJoined[i].Split(";", attachments);

			float health = -1;
			if (i < healthPct.Count())
				health = healthPct[i];

			float quantity = -1;
			if (i < quantityPct.Count())
				quantity = quantityPct[i];

			int lifetime = -1;
			if (i < lifetimeSeconds.Count())
				lifetime = lifetimeSeconds[i];

			int ammo = -1;
			if (i < ammoCount.Count())
				ammo = ammoCount[i];

			JMLootScanItemDetail detail = new JMLootScanItemDetail(m_LastScanClassName, health, quantity, lifetime, attachments, ammo);

			//! First token is the item's own state, then one per attachment.
			if (i < previewStates.Count())
			{
				TStringArray stateTokens = new TStringArray;
				previewStates[i].Split(";", stateTokens);

				if (stateTokens.Count() > 0)
					detail.m_State = JMLootPreviewState.Decode(stateTokens[0]);

				for (int t = 1; t < stateTokens.Count(); t++)
					detail.m_AttachmentStates.Insert(JMLootPreviewState.Decode(stateTokens[t]));
			}

			details.Insert(detail);
		}

		form.AddItemScanMarkers(itemNames, itemPositions, details);

		COTCreateLocalAdminNotification(new StringLocaliser("Found " + itemPositions.Count() + " locations"));
	}

	// Client: Show CE spawn-location results on the Distribution tab's map
	protected void Client_ShowLootSpawnLocations(array<string> names, array<vector> positions, array<string> types)
	{
		if (names.Count() != positions.Count())
			return;

		JMLootAnalysisForm form;
		if (!Class.CastTo(form, GetForm()))
			return;

		form.AddDistributionMarkers(names, positions, types);

		COTCreateLocalAdminNotification(new StringLocaliser("Found " + positions.Count() + " potential spawn locations"));
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

	//! Edited Nominal/Min/Lifetime/Restock, patched into className's source
	//! types.xml server-side - see Exec_SaveTypeXmlValues.
	void SaveItemTypeInfo(string className, int nominal, int min, int lifetime, int restock)
	{
		GetRPCManager().SendRPC("JM_COT_RPC", "RPC_SaveItemTypeInfo", new Param5<string, int, int, int, int>(className, nominal, min, lifetime, restock), true);
	}

	//! Item Info's header Reload button - re-fetches CE data for whichever
	//! classname is currently shown, without re-running a scan/distribution
	//! search (those also touch map markers, which a plain data refresh should
	//! not disturb).
	void RequestItemTypeInfo(string className)
	{
		GetRPCManager().SendRPC("JM_COT_RPC", "RPC_RequestItemTypeInfo", new Param1<string>(className), true);
	}

	void ShowItemsOnMap(string className)
	{
		m_LastScanClassName = className;
		RequestItemScan(className);
	}

	void ShowCESpawnLocations(string className)
	{
		RequestLootDistribution(className);
	}
}
