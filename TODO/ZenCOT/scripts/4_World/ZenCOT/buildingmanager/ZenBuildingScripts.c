// I normally organize my files better, but for an admin tool like this I wanted to get this done asap with minimal fluff.
// I also used ChatGPT to help rush it out which I normally hate - but it works just fine. This isn't exactly inspiring work anyway.

// Static construction utility fucntions
class ZenCOT_ConstructionUtil
{
	// Is this animation-part considered built?
	static bool IsPartBuilt(EntityAI obj, string partName)
	{
		if (!obj) 
			return false;

		float phase = obj.GetAnimationPhase(partName);

		// In vanilla construction: 0 = built, 1 = hidden/unbuilt
		return Math.AbsFloat(phase) < 0.001;
	}

	// Read the "nice" name for a part from config if it exists
	static string GetPartDisplayName(EntityAI obj, string partName)
	{
		if (!obj) 
			return partName;

		string root = "CfgVehicles " + obj.GetType() + " Construction";
		if (!g_Game.ConfigIsExisting(root))
			return partName;

		string mainPart;
		int mains = g_Game.ConfigGetChildrenCount(root);

		for (int i = 0; i < mains; i++)
		{
			g_Game.ConfigGetChildName(root, i, mainPart);
			string mainPath = root + " " + mainPart;
			int subs = g_Game.ConfigGetChildrenCount(mainPath);

			for (int j = 0; j < subs; j++)
			{
				string subPart;
				g_Game.ConfigGetChildName(mainPath, j, subPart);
				if (subPart != partName)
					continue;

				string namePath = mainPath + " " + subPart + " name";
				if (g_Game.ConfigIsExisting(namePath))
				{
					string raw;
					g_Game.ConfigGetTextRaw(namePath, raw);
					g_Game.FormatRawConfigStringKeys(raw);
					if (raw != "")
						return raw;
				}

				return partName;
			}
		}

		return partName;
	}

	// Collect "next stage" parts that can be built immediately according to config
	static void GatherNextStageParts(EntityAI obj, out array<string> outParts)
	{
		outParts = new array<string>;
		if (!obj)
			return;

		string root = "CfgVehicles " + obj.GetType() + " Construction";
		if (!g_Game.ConfigIsExisting(root))
			return;

		int mains = g_Game.ConfigGetChildrenCount(root);

		for (int i = 0; i < mains; i++)
		{
			string mainPart;
			g_Game.ConfigGetChildName(root, i, mainPart);
			string mainPath = root + " " + mainPart;

			int subs = g_Game.ConfigGetChildrenCount(mainPath);
			for (int j = 0; j < subs; j++)
			{
				string part;
				g_Game.ConfigGetChildName(mainPath, j, part);

				// already built? skip
				if (IsPartBuilt(obj, part))
					continue;

				// prerequisites
				array<string> required = new array<string>;
				g_Game.ConfigGetTextArray(mainPath + " " + part + " required_parts", required);

				bool prereqsOK = true;
				for (int r = 0; r < required.Count(); r++)
				{
					if (!IsPartBuilt(obj, required[r]))
					{
						prereqsOK = false;
						break;
					}
				}

				if (!prereqsOK)
					continue;

				// conflicts
				array<string> conflicts = new array<string>;
				g_Game.ConfigGetTextArray(mainPath + " " + part + " conflicted_parts", conflicts);

				bool hasConflict = false;
				for (int c = 0; c < conflicts.Count(); c++)
				{
					if (IsPartBuilt(obj, conflicts[c]))
					{
						hasConflict = true;
						break;
					}
				}

				if (hasConflict)
					continue;

				outParts.Insert(part);
			}
		}
	}

	static bool HasNextStageParts(EntityAI obj)
	{
		array<string> parts;
		GatherNextStageParts(obj, parts);
		return parts && parts.Count() > 0;
	}
}

// NOTE: This will select build component index PER object. 
// So if two admins are working on the same object, the last admin's selection action is the one that will be used if second admin uses F to build
// Unlikely to ever be an issue but worth mentioning obviously.
class ZenCOT_AdminBuildState
{
	protected static ref map<Object, int> m_SelectedIndex = new map<Object, int>;

	static int GetSelectedIndex(Object target, int maxCount)
	{
		if (!target || maxCount <= 0)
			return 0;

		if (!m_SelectedIndex)
			m_SelectedIndex = new map<Object, int>;

		if (!m_SelectedIndex.Contains(target))
			return 0;

		int idx = m_SelectedIndex.Get(target);
		if (idx < 0 || idx >= maxCount)
			return 0;

		return idx;
	}

	static void Cycle(Object target, int maxCount)
	{
		if (!target || maxCount <= 0)
			return;

		if (!m_SelectedIndex)
			m_SelectedIndex = new map<Object, int>;

		int idx = 0;
		if (m_SelectedIndex.Contains(target))
			idx = m_SelectedIndex.Get(target) + 1;

		if (idx >= maxCount)
			idx = 0;

		m_SelectedIndex.Set(target, idx);
	}
}

class ZenCOT_ActionAdminCycleBuild : ActionInteractBase
{
	void ZenCOT_ActionAdminCycleBuild()
	{
		m_Text = "[COT] #STR_ZenCOT_CycleBuild";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem   = new CCINone(); // bare hands
		m_ConditionTarget = new CCTCursor(UAMaxDistances.DEFAULT);
	}

	override typename GetInputType()
	{
		// quick F-press
		return InteractActionInput;
	}

	override bool IsInstant()
	{
		return true;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (g_Game.IsClient() && !GetCommunityOnlineToolsBase().IsActive())
		{
			return false;
		}

		if (!GetPermissionsManager().HasPermission("Admin.Zen.InstantBuild", player.GetIdentity()))
			return false;

		EntityAI eai = EntityAI.Cast(target.GetObject());
		if (!eai)
			return false;

		return ZenCOT_ConstructionUtil.HasNextStageParts(eai);
	}

	protected void DoCycle(PlayerBase player, EntityAI eai, bool clientSide)
	{
		array<string> parts;
		ZenCOT_ConstructionUtil.GatherNextStageParts(eai, parts);
		if (!parts || parts.Count() == 0)
			return;

		ZenCOT_AdminBuildState.Cycle(eai, parts.Count());

		if (clientSide)
		{
			int idx = ZenCOT_AdminBuildState.GetSelectedIndex(eai, parts.Count());
			string partName = parts[idx];
			string niceName = ZenCOT_ConstructionUtil.GetPartDisplayName(eai, partName);

			string title  = "#STR_ZenCOT_AdminBuildTitle";
			string detail = "#STR_ZenCOT_AdminBuildSelected: " + niceName;
			NotificationSystem.AddNotificationExtended(3, title, detail, "set:dayz_gui image:icon_hammer");
		}
	}

	override void Start(ActionData action_data)
	{
		super.Start(action_data);

		PlayerBase player = action_data.m_Player;
		if (!player)
			return;

		EntityAI eai = EntityAI.Cast(action_data.m_Target.GetObject());
		if (!eai)
			return;

		DoCycle(player, eai, false);
	}
}

class ZenCOT_ActionAdminBuild : ActionSingleUseBase
{
    void ZenCOT_ActionAdminBuild()
    {
        m_CommandUID      = DayZPlayerConstants.CMD_ACTIONFB_CHECKPULSE;
        m_FullBody        = true;
        m_StanceMask      = DayZPlayerConstants.STANCEMASK_ERECT;
        m_Text            = "[COT] #build"; // gets overridden in OnActionInfoUpdate
    }

    override void CreateConditionComponents()
    {
        m_ConditionItem   = new CCINone();
        m_ConditionTarget = new CCTCursor(UAMaxDistances.DEFAULT);
    }

    override typename GetInputType()
    {
        // Hold F
        return ContinuousInteractActionInput;
    }

    override bool IsInstant()
    {
        // fire once when the hold completes
        return true;
    }

    override bool HasTarget()
    {
        return true;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
		if (g_Game.IsClient() && !GetCommunityOnlineToolsBase().IsActive())
		{
			return false;
		}

		if (!GetPermissionsManager().HasPermission("Admin.Zen.InstantBuild", player.GetIdentity()))
			return false;

        BaseBuildingBase bb = BaseBuildingBase.Cast(target.GetObject());
        if (!bb)
            return false;

        return ZenCOT_ConstructionUtil.HasNextStageParts(bb);
    }

    override void OnActionInfoUpdate(PlayerBase player, ActionTarget target, ItemBase item)
    {
        super.OnActionInfoUpdate(player, target, item);

        EntityAI eai = EntityAI.Cast(target.GetObject());
        if (!eai)
        {
            m_Text = "[COT] #build";
            return;
        }

        array<string> parts;
        ZenCOT_ConstructionUtil.GatherNextStageParts(eai, parts);
        if (!parts || parts.Count() == 0)
        {
            m_Text = "[COT] #build";
            return;
        }

        int idx = ZenCOT_AdminBuildState.GetSelectedIndex(eai, parts.Count());
        string partName = parts[idx];
        string niceName = ZenCOT_ConstructionUtil.GetPartDisplayName(eai, partName);

        m_Text = "[COT] #build " + niceName;
    }

    // core logic used by both client/server callbacks
    protected void DoBuildInternal(PlayerBase player, EntityAI eai, bool clientSide)
    {
        array<string> parts;
        ZenCOT_ConstructionUtil.GatherNextStageParts(eai, parts);
        if (!parts || parts.Count() == 0)
            return;

        int idx = ZenCOT_AdminBuildState.GetSelectedIndex(eai, parts.Count());
        string partName = parts[idx];

        if (!clientSide)
        {
            BaseBuildingBase bb = BaseBuildingBase.Cast(eai);
            if (!bb)
                return;

            Construction construction = bb.GetConstruction();
            if (!construction)
                return;

            string damageZone;
            if (DamageSystem.GetDamageZoneFromComponentName(bb, partName, damageZone))
            {
                bb.SetAllowDamage(true);
                bb.SetHealthMax(damageZone);
                bb.ProcessInvulnerabilityCheck(bb.GetInvulnerabilityTypeString());
            }

            construction.DestroyCollisionTrigger();
            bb.OnPartBuiltServer(player, partName, AT_BUILD_PART);
        }
        else
        {
            // CLIENT: just feedback
            string niceName = ZenCOT_ConstructionUtil.GetPartDisplayName(eai, partName);
            NotificationSystem.AddNotificationExtended(3, "#STR_ZenCOT_AdminBuildTitle", "#build: " + niceName, "set:dayz_gui image:icon_hammer");
        }
    }

    // Called when F-hold completes (server)
    override void OnStartServer(ActionData action_data)
    {
        super.OnStartServer(action_data);

        PlayerBase player = action_data.m_Player;
        if (!player)
            return;

        EntityAI eai = EntityAI.Cast(action_data.m_Target.GetObject());
        if (!eai)
            return;

        DoBuildInternal(player, eai, false);
    }

    // Called when F-hold completes (client)
    override void OnStartClient(ActionData action_data)
    {
        super.OnStartClient(action_data);

        PlayerBase player = action_data.m_Player;
        EntityAI eai = EntityAI.Cast(action_data.m_Target.GetObject());
        if (!player || !eai)
            return;

        DoBuildInternal(player, eai, true);
    }
}

modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);

		actions.Insert(ZenCOT_ActionAdminCycleBuild);
		actions.Insert(ZenCOT_ActionAdminBuild);
	}
}

modded class ItemBase
{
	override void SetActions()
	{
		super.SetActions();

		AddAction(ZenCOT_ActionAdminCycleBuild);
		AddAction(ZenCOT_ActionAdminBuild);
	}
}
