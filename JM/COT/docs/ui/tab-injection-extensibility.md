# Global Architecture & Mentality: Mod Extensibility & Tab Injection

This document explains the overarching architectural goals and modular design philosophy behind Community Online Tools (COT) UI extensibility.

---

## 1. Core Mentality & Goals

### Zero-Touch Modification for Sub-Mods & Addons
Third-party DayZ server mods often need to inject custom administrative tools, data panels, or controls directly into COT's interface (such as `JMPlayerForm`). 

Historically, extending UI forms required fragile techniques:
- Overriding entire methods and hardcoding/re-declaring label and icon arrays.
- Re-creating layout widgets or manually managing tab index constants (`static int TAB_INDEX`).
- Overriding `BuildTabIfNeeded` with chained `if/else` checks and `super` calls.

**The Goal:** Provide a **1-line, contract-driven registration API** for addons that requires zero array re-declarations, zero hardcoded tab index bookkeeping, and no method override clutter.

---

## 2. Global Design Patterns

### A. Dynamic Tab Registration (`AddTab`)
`AddTab` lives on `JMFormBase`, so any form that exposes its tab strip (see `GetTabStrip()` /
`GetTabContentHost()` in [module-form-patterns.md](module-form-patterns.md)) accepts injected tabs:

```c
int tabIdx = form.AddTab("My Addon", JMConstants.Lucide("star"), "BuildMyAddonTab");
```

1. **Automatic Index Allocation:** The system calculates the next available index and returns it.
2. **Lazy Container Creation:** A dedicated container widget (`GridSpacerWidget`) is allocated and linked to the tab strip.
3. **Lazy Tab State Synchronization:** The form's lazy-build state is extended by `DeclareTabs(1)`.

---

### B. Event-Driven & Delegate Callback Dispatch
Rather than requiring every subclass to intercept when a tab is activated, `JMFormBase` keeps a callback
map (`m_CustomTabCallbacks`, one `JMCustomTabCallback` per injected tab: target instance + function name).
`EnsureTabCreated()` checks that map first and calls the builder, so the form's `OnTabCreate` needs no case:


When a user clicks on an injected tab for the first time:
- The tab is created exactly once (`OnTabCreate` is skipped for it; the builder runs instead).
- The registered builder (e.g. `BuildMyAddonTab(Widget parent)`) is invoked with the empty panel.
- `OnTabFocus` and `OnTabUpdate` then run on every selection.

---

## 3. Reference Addon Example

See `ExampleScriptOverride/` for a complete, compilable PBO addon template.

### A. Dynamic Tab Injection
```c
AddTab("Faction Manager", JMConstants.Lucide("shield"), "BuildFactionTab");
```

### B. Injecting a Panel/Card into an Existing Tab
Override an existing tab builder method (such as `InitActionWidgetsIdentity`) and call `super`:
```c
override protected void InitActionWidgetsIdentity(Widget actionsParent)
{
    super.InitActionWidgetsIdentity(actionsParent);

    UIActionCard card = UIActionManager.CreateCard(actionsParent, "Custom Panel");
    card.AddRefreshButton(this, "OnClick_RefreshCustomCard");
}
```

### C. Adding a Custom Module & Category
Create a module inheriting from `JMRenderableModuleBase`:
```c
class JMCustomExampleModule: JMRenderableModuleBase
{
    // Called on client and server before the mission loads - no constructor needed.
    override void DeclarePermissions()
    {
        JMPermissions.Register( JMConstants.PERM_CUSTOM_VIEW );   // a "3_Game" class of the addon declares the constant
    }

    override string GetCategory()
    {
        return "Custom Addons"; // Custom sidebar category
    }

    override string GetTitle()
    {
        return "Custom Addon Module";
    }
}
```

`ExampleScriptOverride/5_Mission/JMCustomExampleModule.c` shows the getter-by-getter form. A module in COT itself states the same
facts once in `DescribeModule( JMModuleInfo info )` (see [../systems/module-creation.md](../systems/module-creation.md)); the getters
above keep working and win over the declaration, so a mod can use either.

### D. Adding Custom Permissions & Inline Binding
```c
// In a module: override void DeclarePermissions() { JMPermissions.Register(JMConstants.PERM_PLAYER_INJECTED_PANEL); }

// Bind the control (hidden when the permission is missing, kept in step on every change):
UIActionButton btn = UIActionManager.CreateButton(parent, "Action", null, "");
btn.SetOnClick(this, "OnAction");          // void OnAction(UIActionBase action), CLICK only
BindPermission(btn, JMConstants.PERM_PLAYER_INJECTED_PANEL);
```

### E. Direct Callbacks on Context Menu Items
Override `OnPlayerRow_RightClick` and pass target instance + callback function name directly:
```c
override void OnPlayerRow_RightClick(string guid, int x, int y)
{
    super.OnPlayerRow_RightClick(guid, x, y);

    // Direct callback dispatch: no OnClick_PlayerMenu override required!
    m_PlayerMenu.AddItem("addon_custom_action", "Sub-Mod Action", JMConstants.Lucide("sparkles"), 0, false, this, "OnMyCustomAction");
}

void OnMyCustomAction(string itemId)
{
    // Executed directly on click!
}
```


---

## 4. Key Takeaways for Framework Authors
- **Preserve Existing UI State:** Never force callers to re-supply existing tab labels/icons: a mod just calls `AddTab()`.
- **Null-Guard Deferred Widgets:** Any timer-driven or RPC-driven update code must null-guard widgets belonging to lazy tabs (`if (m_MyWidget) m_MyWidget.SetText(...)`).
- **Use `protected` Access Modifiers:** Always expose key members (`m_Tabs`, `m_ActionsWrapper`, builder callbacks) as `protected` so third-party `modded class` scripts can extend them safely.

