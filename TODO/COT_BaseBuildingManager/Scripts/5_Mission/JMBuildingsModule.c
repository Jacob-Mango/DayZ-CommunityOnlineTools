// ============================================================================
//  Metadonnees d'une construction (envoyees serveur -> client)
// ============================================================================
class JMBuildMetaData
{
	int m_NetworkIDLow;
	int m_NetworkIDHigh;

	int m_PersistentIDA;
	int m_PersistentIDB;
	int m_PersistentIDC;
	int m_PersistentIDD;

	string m_ClassName;
	vector m_Position;
	vector m_Orientation;

	float m_Health;
	float m_MaxHealth;
	bool m_IsRuined;

	// Duree de vie RESTANTE avant despawn (en secondes), via GetLifetime().
	// Decompte reel gere par le moteur (CE), remis a son max quand un joueur
	// rafraichit l'objet. C'est la meme valeur que celle affichee par ZenCOT.
	float m_Lifetime;

	// 0 = Base building (cloture, mirador, mat...), 1 = Tente
	int m_Category;

	[NonSerialized()]
	string m_DisplayName;

	[NonSerialized()]
	EntityAI m_Entity;

	static JMBuildMetaData Create(EntityAI entity, int category)
	{
		JMBuildMetaData meta = new JMBuildMetaData();
		meta.AcquireFrom(entity, category);
		return meta;
	}

	void AcquireFrom(EntityAI entity, int category)
	{
		m_Entity = entity;
		m_Category = category;

		entity.GetNetworkID(m_NetworkIDLow, m_NetworkIDHigh);
		entity.GetPersistentID(m_PersistentIDA, m_PersistentIDB, m_PersistentIDC, m_PersistentIDD);

		m_ClassName = entity.GetType();
		m_Position = entity.GetPosition();
		m_Orientation = entity.GetOrientation();

		m_Health = entity.GetHealth("", "");
		m_MaxHealth = entity.GetMaxHealth("", "");
		m_IsRuined = entity.IsRuined();
		// Duree de vie RESTANTE reelle avant despawn (secondes) — meme source que ZenCOT.
		// GetLifetime() decompte ; GetLifetimeMax() ne renvoyait que le max configure.
		m_Lifetime = entity.GetLifetime();
	}

	void SetDisplayName()
	{
		if (g_Game.ConfigIsExisting("cfgVehicles " + m_ClassName + " displayName"))
			g_Game.ConfigGetText("cfgVehicles " + m_ClassName + " displayName", m_DisplayName);
		else
			m_DisplayName = m_ClassName;

		if (m_DisplayName == string.Empty)
			m_DisplayName = m_ClassName;
	}

	float GetHealthPercent()
	{
		if (m_MaxHealth <= 0)
			return 0;
		return (m_Health / m_MaxHealth) * 100.0;
	}

	// Duree de vie max convertie en jours (86400 s = 1 jour).
	float GetLifetimeDays()
	{
		return m_Lifetime / 86400.0;
	}

	void Write(ParamsWriteContext ctx)
	{
		ctx.Write(m_NetworkIDLow);
		ctx.Write(m_NetworkIDHigh);
		ctx.Write(m_PersistentIDA);
		ctx.Write(m_PersistentIDB);
		ctx.Write(m_PersistentIDC);
		ctx.Write(m_PersistentIDD);
		ctx.Write(m_ClassName);
		ctx.Write(m_Position);
		ctx.Write(m_Orientation);
		ctx.Write(m_Health);
		ctx.Write(m_MaxHealth);
		ctx.Write(m_IsRuined);
		ctx.Write(m_Category);
		ctx.Write(m_Lifetime);
	}

	bool Read(ParamsReadContext ctx)
	{
		if (!ctx.Read(m_NetworkIDLow)) return false;
		if (!ctx.Read(m_NetworkIDHigh)) return false;
		if (!ctx.Read(m_PersistentIDA)) return false;
		if (!ctx.Read(m_PersistentIDB)) return false;
		if (!ctx.Read(m_PersistentIDC)) return false;
		if (!ctx.Read(m_PersistentIDD)) return false;
		if (!ctx.Read(m_ClassName)) return false;
		if (!ctx.Read(m_Position)) return false;
		if (!ctx.Read(m_Orientation)) return false;
		if (!ctx.Read(m_Health)) return false;
		if (!ctx.Read(m_MaxHealth)) return false;
		if (!ctx.Read(m_IsRuined)) return false;
		if (!ctx.Read(m_Category)) return false;
		if (!ctx.Read(m_Lifetime)) return false;
		return true;
	}
}

// ============================================================================
//  Module COT
// ============================================================================
class JMBuildingsModule: JMRenderableModuleBase
{
	private ref array<ref JMBuildMetaData> m_Buildings;

	void JMBuildingsModule()
	{
		GetPermissionsManager().RegisterPermission("BaseBuilding.View");
		GetPermissionsManager().RegisterPermission("BaseBuilding.Delete");
		GetPermissionsManager().RegisterPermission("BaseBuilding.Delete.Group");
		GetPermissionsManager().RegisterPermission("BaseBuilding.Delete.All");

		m_Buildings = new array<ref JMBuildMetaData>;
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission("BaseBuilding.View");
	}

	override string GetLayoutRoot()
	{
		return "COT_BaseBuildingManager/GUI/layouts/buildings/Buildings_Menu.layout";
	}

	override string GetTitle()
	{
		return "#STR_BBM_MODULE_NAME";
	}

	override string GetIconName()
	{
		return "COT_BaseBuildingManager\\GUI\\textures\\Tent.paa";
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
	}

	// ---- Collecte serveur ----
	private void UpdateBuildingsMetaData()
	{
		if (m_Buildings.Count() > 0)
			m_Buildings.Clear();

		// Base building (cloture, mirador, mat, portail, pieces moddees...)
		auto node = BaseBuildingBase.s_COT_AllBuildings.m_Head;
		while (node)
		{
			BaseBuildingBase bb = node.m_Value;
			if (bb && !bb.IsSetForDeletion())
				m_Buildings.Insert(JMBuildMetaData.Create(bb, 0));
			node = node.m_Next;
		}

		// Tentes : uniquement celles qui sont DEPLOYEES (montees). On exclut les
		// tentes empaquetees/repliees (au sol ou dans un inventaire) dont l'etat
		// est PACKED, qui ne sont pas de vraies constructions posees.
		auto tnode = TentBase.s_COT_AllTents.m_Head;
		while (tnode)
		{
			TentBase tent = tnode.m_Value;
			if (tent && tent.GetState() == TentBase.PITCHED && !tent.IsSetForDeletion())
				m_Buildings.Insert(JMBuildMetaData.Create(tent, 1));
			tnode = tnode.m_Next;
		}
	}

	void UpdateBuildingsMetaData_SP()
	{
		UpdateBuildingsMetaData();

		foreach (auto meta: m_Buildings)
		{
			meta.SetDisplayName();
		}

		JMBuildingsMenu form;
		if (Class.CastTo(form, GetForm()))
			form.LoadGroups();
	}

	override int GetRPCMin()
	{
		return JMBuildingsModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMBuildingsModuleRPC.COUNT;
	}

	override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
	{
		switch (rpc_type)
		{
		case JMBuildingsModuleRPC.RequestServerBuildings:
			RPC_RequestServerBuildings(ctx, sender, target);
			break;
		case JMBuildingsModuleRPC.SendServerBuildings:
			RPC_SendServerBuildings(ctx, sender, target);
			break;
		case JMBuildingsModuleRPC.DeleteBuilding:
			RPC_DeleteBuilding(ctx, sender, target);
			break;
		case JMBuildingsModuleRPC.DeleteBuildingGroup:
			RPC_DeleteBuildingGroup(ctx, sender, target);
			break;
		case JMBuildingsModuleRPC.DeleteBuildingAll:
			RPC_DeleteBuildingAll(ctx, sender, target);
			break;
		}
	}

	// ---- Liste ----
	void RequestServerBuildings()
	{
		if (g_Game.IsClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Send(NULL, JMBuildingsModuleRPC.RequestServerBuildings, true);
		}
		else
		{
			UpdateBuildingsMetaData_SP();
		}
	}

	void RPC_RequestServerBuildings(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("BaseBuilding.View", senderRPC))
			return;

		UpdateBuildingsMetaData();

		auto rpc = new ScriptRPC();
		rpc.Write(m_Buildings.Count());
		foreach (auto bb: m_Buildings)
		{
			bb.Write(rpc);
		}
		rpc.Send(NULL, JMBuildingsModuleRPC.SendServerBuildings, true, senderRPC);
	}

	private void RPC_SendServerBuildings(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionClient())
			return;

		int count;
		if (!ctx.Read(count))
		{
			Error("Couldn't read buildings count");
			return;
		}

		m_Buildings.Clear();

		while (count)
		{
			auto bb = new JMBuildMetaData();
			if (!bb.Read(ctx))
			{
				Error("Couldn't read building");
				return;
			}
			bb.SetDisplayName();
			m_Buildings.Insert(bb);
			count--;
		}

		JMBuildingsMenu form;
		if (Class.CastTo(form, GetForm()))
			form.LoadGroups();
	}

	// ---- Suppression individuelle ----
	void DeleteBuilding(JMBuildMetaData meta)
	{
		if (!meta)
			return;

		if (g_Game.IsClient())
		{
			DeleteBuilding(meta.m_NetworkIDLow, meta.m_NetworkIDHigh);
		}
		else
		{
			g_Game.ObjectDelete(meta.m_Entity);
			UpdateBuildingsMetaData_SP();
		}
	}

	void DeleteBuilding(int netLow, int netHigh)
	{
		if (IsMissionClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Write(netLow);
			rpc.Write(netHigh);
			rpc.Send(NULL, JMBuildingsModuleRPC.DeleteBuilding, true);
		}
	}

	private void RPC_DeleteBuilding(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("BaseBuilding.Delete", senderRPC))
			return;

		int netLow;
		if (!ctx.Read(netLow))
			return;
		int netHigh;
		if (!ctx.Read(netHigh))
			return;

		Object obj = g_Game.GetObjectByNetworkId(netLow, netHigh);
		if (!obj)
			return;

		g_Game.ObjectDelete(obj);

		RPC_RequestServerBuildings(ctx, senderRPC, target);
	}

	// ---- Suppression d'un groupe (liste de network IDs) ----
	void DeleteBuildingGroup(array<ref JMBuildMetaData> group)
	{
		if (!group || group.Count() == 0)
			return;

		if (g_Game.IsClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Write(group.Count());
			foreach (JMBuildMetaData meta: group)
			{
				rpc.Write(meta.m_NetworkIDLow);
				rpc.Write(meta.m_NetworkIDHigh);
			}
			rpc.Send(NULL, JMBuildingsModuleRPC.DeleteBuildingGroup, true);
		}
		else
		{
			foreach (JMBuildMetaData m: group)
			{
				if (m.m_Entity)
					g_Game.ObjectDelete(m.m_Entity);
			}
			UpdateBuildingsMetaData_SP();
		}
	}

	private void RPC_DeleteBuildingGroup(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("BaseBuilding.Delete.Group", senderRPC))
			return;

		int count;
		if (!ctx.Read(count))
			return;

		array<Object> toDelete = new array<Object>;
		while (count)
		{
			int netLow;
			int netHigh;
			if (!ctx.Read(netLow))
				return;
			if (!ctx.Read(netHigh))
				return;

			Object obj = g_Game.GetObjectByNetworkId(netLow, netHigh);
			if (obj)
				toDelete.Insert(obj);

			count--;
		}

		foreach (Object o: toDelete)
		{
			g_Game.ObjectDelete(o);
		}

		RPC_RequestServerBuildings(ctx, senderRPC, target);
	}

	// ---- Suppression totale ----
	void DeleteBuildingAll()
	{
		if (g_Game.IsClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Send(NULL, JMBuildingsModuleRPC.DeleteBuildingAll, true);
		}
		else
		{
			Exec_DeleteBuildingAll();
			UpdateBuildingsMetaData_SP();
		}
	}

	private void RPC_DeleteBuildingAll(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("BaseBuilding.Delete.All", senderRPC))
			return;

		Exec_DeleteBuildingAll();

		RPC_RequestServerBuildings(ctx, senderRPC, target);
	}

	void Exec_DeleteBuildingAll()
	{
		// Collecte puis suppression (eviter d'invalider l'iterateur pendant la suppression).
		array<EntityAI> toDelete = new array<EntityAI>;

		auto node = BaseBuildingBase.s_COT_AllBuildings.m_Head;
		while (node)
		{
			if (node.m_Value)
				toDelete.Insert(node.m_Value);
			node = node.m_Next;
		}

		auto tnode = TentBase.s_COT_AllTents.m_Head;
		while (tnode)
		{
			// Meme filtre que l'affichage : on ne supprime que les tentes deployees.
			if (tnode.m_Value && tnode.m_Value.GetState() == TentBase.PITCHED)
				toDelete.Insert(tnode.m_Value);
			tnode = tnode.m_Next;
		}

		foreach (EntityAI e: toDelete)
		{
			g_Game.ObjectDelete(e);
		}
	}

	array<ref JMBuildMetaData> GetServerBuildings()
	{
		return m_Buildings;
	}
}
