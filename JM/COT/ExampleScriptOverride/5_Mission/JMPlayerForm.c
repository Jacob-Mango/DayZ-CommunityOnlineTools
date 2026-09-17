#ifdef JM_CommunityOnlineTools
// Comprehensive Example: Sub-mod UI Extension Patterns in 5_Mission
modded class JMPlayerForm
{
	protected UIActionCard m_ExampleInjectedCard;
	protected UIActionCard m_ExampleTabCard;

	override void OnInit()
	{
		super.OnInit();

		// 1. Adding a new tab using clean 1-line dynamic tab registration
		AddTab("Example", JMConstants.Lucide("star"), "BuildExampleTab");

		// Register custom permission for injected panel control
		GetPermissionsManager().RegisterPermission("Admin.Player.InjectedPanel");
	}

	// =========================================================================
	// 1. Adding a panel/card inside an ALREADY EXISTING tab (General tab)
	// =========================================================================
	override Widget InitActionWidgetsIdentity(Widget actionsParent)
	{
		Widget parent = super.InitActionWidgetsIdentity(actionsParent);

		if (!parent)
			return parent;

		// Create a card inside the existing identity/general tab parent
		m_ExampleInjectedCard = UIActionManager.CreateCard(parent, "Injected Addon Panel");
		if (!m_ExampleInjectedCard)
			return parent;

		// Add card title bar action buttons
		m_ExampleInjectedCard.AddRefreshButton(this, "OnClick_InjectedRefresh");

		Widget content = m_ExampleInjectedCard.GetContent();
		if (!content)
			return parent;

		UIActionText infoText = UIActionManager.CreateText(content, "Addon Action:", "Injected custom controls into existing tab!");

		UIActionButton btn = UIActionManager.CreateButton(content, "Execute Addon Action", this, "OnClick_InjectedAction");

		// Bind permission directly to control
		RegisterPermission(btn, "Admin.Player.InjectedPanel");

		return parent;
	}

	void OnClick_InjectedRefresh(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		action.AnimateSpin();
	}

	void OnClick_InjectedAction(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		action.AnimateFeedback();
	}

	// =========================================================================
	// Lazy Builder for the newly added "Example" Tab
	// =========================================================================
	void BuildExampleTab(Widget parent)
	{
		if (!parent)
			return;

		UIActionManager.CreatePanel(parent, 0x00000000, 10);

		m_ExampleTabCard = UIActionManager.CreateCard(parent, "Example Tab Card");
		if (!m_ExampleTabCard)
			return;

		Widget body = m_ExampleTabCard.GetContent();
		if (!body)
			return;

		UIActionManager.CreateText(body, "Hello", "This is an example panel inside a new tab.");
	}

	// =========================================================================
	// 5. Adding custom action context & 6. Context menu / sub-menu overrides
	// =========================================================================
	override void OnPlayerRow_RightClick(string guid, int x, int y)
	{
		super.OnPlayerRow_RightClick(guid, x, y);

		if (!m_PlayerMenu)
			return;

		// Add custom context menu item ID
		m_PlayerMenu.AddItem("addon_custom_action", "Sub-Mod Quick Action", JMConstants.Lucide("sparkles"));
		
		// Add submenu item (indicated by trailing chevron)
		m_PlayerMenu.AddItem("addon_submenu_group", "Sub-Mod Actions...", JMConstants.Lucide("layers"), 0, true);

		// Dynamic permission check on custom context menu item
		bool canPerform = GetPermissionsManager().HasPermission("Admin.Player.InjectedPanel");
		m_PlayerMenu.SetItemEnabled("addon_custom_action", canPerform);
	}

	override void OnClick_PlayerMenu(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK || !m_PlayerMenu)
		{
			super.OnClick_PlayerMenu(eid, action);
			return;
		}

		string itemID = m_PlayerMenu.GetLastClickedId();

		if (itemID == "addon_custom_action")
		{
			// Custom action context execution
			COTCreateLocalAdminNotification(new StringLocaliser("Sub-mod action executed for player: " + m_PlayerMenuGUID), JMConstants.Lucide("sparkles"));
			return;
		}
		else if (itemID == "addon_submenu_group")
		{
			// Example opening secondary action context or sub-menu
			COTCreateLocalAdminNotification(new StringLocaliser("Sub-menu item opened!"), JMConstants.Lucide("layers"));
			return;
		}

		super.OnClick_PlayerMenu(eid, action);
	}
}
#endif
