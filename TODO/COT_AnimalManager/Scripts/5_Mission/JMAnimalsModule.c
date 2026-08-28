class JMAnimalMetaData
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
	bool m_IsAlive;

	[NonSerialized()]
	string m_DisplayName;

	[NonSerialized()]
	EntityAI m_Entity;

	static JMAnimalMetaData Create(EntityAI entity)
	{
		JMAnimalMetaData meta = new JMAnimalMetaData();
		meta.AcquireFrom(entity);
		return meta;
	}

	void AcquireFrom(EntityAI entity)
	{
		m_Entity = entity;

		entity.GetNetworkID(m_NetworkIDLow, m_NetworkIDHigh);
		entity.GetPersistentID(m_PersistentIDA, m_PersistentIDB, m_PersistentIDC, m_PersistentIDD);

		m_ClassName = entity.GetType();
		m_Position = entity.GetPosition();
		m_Orientation = entity.GetOrientation();

		m_Health = entity.GetHealth("", "");
		m_MaxHealth = entity.GetMaxHealth("", "");
		m_IsAlive = entity.IsAlive();
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
		ctx.Write(m_IsAlive);
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
		if (!ctx.Read(m_IsAlive)) return false;
		return true;
	}
}

class JMAnimalsModule: JMRenderableModuleBase
{
	private ref array<ref JMAnimalMetaData> m_Animals;

	void JMAnimalsModule()
	{
		GetPermissionsManager().RegisterPermission("Animals.View");
		GetPermissionsManager().RegisterPermission("Animals.Delete");
		GetPermissionsManager().RegisterPermission("Animals.Delete.All");
		GetPermissionsManager().RegisterPermission("Animals.Delete.Radius");
		GetPermissionsManager().RegisterPermission("Animals.Teleport");
		GetPermissionsManager().RegisterPermission("Animals.Heal");

		m_Animals = new array<ref JMAnimalMetaData>;
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission("Animals.View");
	}

	override string GetLayoutRoot()
	{
		return "COT_AnimalManager/GUI/layouts/animals/Animals_Menu.layout";
	}

	override string GetTitle()
	{
		return "#STR_GMA_MODULE_NAME";
	}

	override string GetIconName()
	{
		return "COT_AnimalManager\\GUI\\textures\\Animal.paa";
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
	}

	private void UpdateAnimalsMetaData()
	{
		if (m_Animals.Count() > 0)
			m_Animals.Clear();

		auto node = AnimalBase.s_GMA_AllAnimals.m_Head;
		while (node)
		{
			AnimalBase animal = node.m_Value;
			if (animal && !animal.IsSetForDeletion())
				m_Animals.Insert(JMAnimalMetaData.Create(animal));
			node = node.m_Next;
		}
	}

	void UpdateAnimalsMetaData_SP()
	{
		UpdateAnimalsMetaData();

		foreach (auto meta: m_Animals)
		{
			meta.SetDisplayName();
		}

		JMAnimalsMenu form;
		if (Class.CastTo(form, GetForm()))
			form.LoadAnimals();
	}

	override int GetRPCMin()
	{
		return JMAnimalsModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMAnimalsModuleRPC.COUNT;
	}

	override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
	{
		switch (rpc_type)
		{
		case JMAnimalsModuleRPC.RequestServerAnimals:
			RPC_RequestServerAnimals(ctx, sender, target);
			break;
		case JMAnimalsModuleRPC.SendServerAnimals:
			RPC_SendServerAnimals(ctx, sender, target);
			break;
		case JMAnimalsModuleRPC.DeleteAnimal:
			RPC_DeleteAnimal(ctx, sender, target);
			break;
		case JMAnimalsModuleRPC.DeleteAnimalAll:
			RPC_DeleteAnimalAll(ctx, sender, target);
			break;
		case JMAnimalsModuleRPC.DeleteAnimalRadius:
			RPC_DeleteAnimalRadius(ctx, sender, target);
			break;
		case JMAnimalsModuleRPC.TeleportToAnimal:
			RPC_TeleportToAnimal(ctx, sender, target);
			break;
		case JMAnimalsModuleRPC.HealAnimal:
			RPC_HealAnimal(ctx, sender, target);
			break;
		}
	}

	// ---- Liste ----
	void RequestServerAnimals()
	{
		if (g_Game.IsClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Send(NULL, JMAnimalsModuleRPC.RequestServerAnimals, true);
		}
		else
		{
			UpdateAnimalsMetaData_SP();
		}
	}

	void RPC_RequestServerAnimals(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("Animals.View", senderRPC))
			return;

		UpdateAnimalsMetaData();

		auto rpc = new ScriptRPC();
		rpc.Write(m_Animals.Count());
		foreach (auto animal: m_Animals)
		{
			animal.Write(rpc);
		}
		rpc.Send(NULL, JMAnimalsModuleRPC.SendServerAnimals, true, senderRPC);
	}

	private void RPC_SendServerAnimals(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionClient())
			return;

		int count;
		if (!ctx.Read(count))
		{
			Error("Couldn't read animals count");
			return;
		}

		m_Animals.Clear();

		while (count)
		{
			auto animal = new JMAnimalMetaData();
			if (!animal.Read(ctx))
			{
				Error("Couldn't read animal");
				return;
			}
			animal.SetDisplayName();
			m_Animals.Insert(animal);
			count--;
		}

		JMAnimalsMenu form;
		if (Class.CastTo(form, GetForm()))
			form.LoadAnimals();
	}

	// ---- Suppression individuelle ----
	void DeleteAnimal(JMAnimalMetaData meta)
	{
		if (g_Game.IsClient())
		{
			DeleteAnimal(meta.m_NetworkIDLow, meta.m_NetworkIDHigh);
		}
		else
		{
			g_Game.ObjectDelete(meta.m_Entity);
			UpdateAnimalsMetaData_SP();
		}
	}

	void DeleteAnimal(int netLow, int netHigh)
	{
		if (IsMissionClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Write(netLow);
			rpc.Write(netHigh);
			rpc.Send(NULL, JMAnimalsModuleRPC.DeleteAnimal, true);
		}
	}

	private void RPC_DeleteAnimal(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("Animals.Delete", senderRPC))
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

		RPC_RequestServerAnimals(ctx, senderRPC, target);
	}

	// ---- Suppression totale ----
	void DeleteAnimalAll()
	{
		if (g_Game.IsClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Send(NULL, JMAnimalsModuleRPC.DeleteAnimalAll, true);
		}
		else
		{
			Exec_DeleteAnimalAll();
			UpdateAnimalsMetaData_SP();
		}
	}

	private void RPC_DeleteAnimalAll(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("Animals.Delete.All", senderRPC))
			return;

		Exec_DeleteAnimalAll();

		RPC_RequestServerAnimals(ctx, senderRPC, target);
	}

	void Exec_DeleteAnimalAll()
	{
		// Collecte puis suppression (eviter d'invalider l'iterateur pendant la suppression).
		array<AnimalBase> toDelete = new array<AnimalBase>;
		auto node = AnimalBase.s_GMA_AllAnimals.m_Head;
		while (node)
		{
			if (node.m_Value)
				toDelete.Insert(node.m_Value);
			node = node.m_Next;
		}

		foreach (AnimalBase a: toDelete)
		{
			g_Game.ObjectDelete(a);
		}
	}

	// ---- Suppression par rayon autour de l'admin ----
	void DeleteAnimalRadius(float radius)
	{
		if (g_Game.IsClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Write(radius);
			rpc.Send(NULL, JMAnimalsModuleRPC.DeleteAnimalRadius, true);
		}
		else
		{
			Exec_DeleteAnimalRadius(PlayerBase.Cast(g_Game.GetPlayer()), radius);
			UpdateAnimalsMetaData_SP();
		}
	}

	private void RPC_DeleteAnimalRadius(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("Animals.Delete.Radius", senderRPC))
			return;

		float radius;
		if (!ctx.Read(radius))
			return;

		PlayerBase player;
		if (!Class.CastTo(player, senderRPC.GetPlayer()))
			return;

		Exec_DeleteAnimalRadius(player, radius);

		RPC_RequestServerAnimals(ctx, senderRPC, target);
	}

	void Exec_DeleteAnimalRadius(PlayerBase player, float radius)
	{
		if (!player)
			return;
		if (radius <= 0)
			return;

		vector center = player.GetWorldPosition();

		array<AnimalBase> toDelete = new array<AnimalBase>;
		auto node = AnimalBase.s_GMA_AllAnimals.m_Head;
		while (node)
		{
			AnimalBase animal = node.m_Value;
			if (animal)
			{
				vector d = animal.GetWorldPosition() - center;
				d[1] = 0; // distance horizontale (au sol)
				if (d.Length() <= radius)
					toDelete.Insert(animal);
			}
			node = node.m_Next;
		}

		foreach (AnimalBase a: toDelete)
		{
			g_Game.ObjectDelete(a);
		}
	}

	// ---- Teleport ----
	void RequestTeleportToAnimal(JMAnimalMetaData meta)
	{
		if (g_Game.IsClient())
		{
			RequestTeleportToAnimal(meta.m_NetworkIDLow, meta.m_NetworkIDHigh);
		}
		else
		{
			Exec_TeleportToAnimal(PlayerBase.Cast(g_Game.GetPlayer()), meta.m_Entity);
		}
	}

	void RequestTeleportToAnimal(int netLow, int netHigh)
	{
		if (IsMissionClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Write(netLow);
			rpc.Write(netHigh);
			rpc.Send(NULL, JMAnimalsModuleRPC.TeleportToAnimal, true);
		}
	}

	private void RPC_TeleportToAnimal(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("Animals.Teleport", senderRPC))
			return;

		int netLow;
		if (!ctx.Read(netLow))
			return;
		int netHigh;
		if (!ctx.Read(netHigh))
			return;

		PlayerBase player;
		if (!Class.CastTo(player, senderRPC.GetPlayer()))
			return;

		Object obj = g_Game.GetObjectByNetworkId(netLow, netHigh);
		if (!obj)
			return;

		Exec_TeleportToAnimal(player, obj);
	}

	void Exec_TeleportToAnimal(PlayerBase player, Object obj)
	{
		if (!player || !obj)
			return;

		vector pos = obj.GetPosition();
		vector minMax[2];
		obj.ClippingInfo(minMax);

		player.SetLastPosition();
		pos = pos + minMax[1];
		pos[1] = g_Game.SurfaceRoadY3D(pos[0], pos[1], pos[2], RoadSurfaceDetection.UNDER);
		player.SetWorldPosition(pos);
	}

	// ---- Soin ----
	void HealAnimal(JMAnimalMetaData meta)
	{
		if (!meta)
			return;

		if (g_Game.IsClient())
		{
			HealAnimal(meta.m_NetworkIDLow, meta.m_NetworkIDHigh);
		}
		else
		{
			Exec_HealAnimal(meta.m_Entity);
			UpdateAnimalsMetaData_SP();
		}
	}

	void HealAnimal(int netLow, int netHigh)
	{
		if (IsMissionClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Write(netLow);
			rpc.Write(netHigh);
			rpc.Send(NULL, JMAnimalsModuleRPC.HealAnimal, true);
		}
	}

	private void RPC_HealAnimal(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("Animals.Heal", senderRPC))
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

		EntityAI entity;
		if (!Class.CastTo(entity, obj))
			return;

		Exec_HealAnimal(entity);

		RPC_RequestServerAnimals(ctx, senderRPC, target);
	}

	void Exec_HealAnimal(EntityAI entity)
	{
		if (!entity)
			return;

		// Remet la sante globale au maximum (ne ressuscite pas un animal deja mort).
		entity.SetHealth("", "", entity.GetMaxHealth("", ""));
		entity.SetHealth("", "Health", entity.GetMaxHealth("", "Health"));
	}

	array<ref JMAnimalMetaData> GetServerAnimals()
	{
		return m_Animals;
	}
}
