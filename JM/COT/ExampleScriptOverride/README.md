# ExampleScriptOverride

A small addon that extends COT the way a sub-mod would. Each file shows one thing and stays short (under ~100
lines - the form and the RPC round trip are the longest); it is part of the normal build, so anything here that
stops compiling is a broken public API.

| I want to... | File |
|---|---|
| add my own sidebar module | `JMCustomExampleModule.c` + `Ex_ModuleRegistration.c` |
| give it a form (layout, tabs) | `JMCustomExampleForm.c` + `GUI/layouts/JMCustomExampleForm.layout` |
| send a request to the server and get an answer back | `Ex_RPCHandling.c` (also webhooks) |
| save settings to JSON | `Ex_ConfigPersistence.c` |
| declare custom permission keys | `3_Game/ExamplePermissions.c` (+ `DeclarePermissions()` in the module) |
| hide a control without a permission | `Ex_CustomPermissions.c` |
| add a tab to an existing form | `Ex_CustomTabWithPanels.c` |
| inject a card into an existing tab | `Ex_InjectedPanelExistingTab.c` |
| put buttons in a card's title bar | `Ex_PanelHeaderButtons.c` |
| show tooltips | `Ex_HintTooltip.c` |
| open a value prompt or a menu | `Ex_Popups.c` |
| make a tab its own class (create/focus/update/unfocus) | `Ex_TabController.c` (+ `SetTabEnabled`, `UpdateActiveTab` in `JMCustomExampleForm.c`) |
| build a search toolbar over a list | `Ex_SearchRow.c` |
| add to the player right-click menu | `Ex_ContextMenu.c` |
| add a row to any filter menu (Player, Vehicles, Teleport, Object Spawner, ESP, Loot) | `Ex_FilterPlayerList.c` - `JMFilterRegistry.Register( scope, ... )`, a checkbox when it has a state callback |
| ...a plain click row | `Ex_FilterObjectSpawner.c` |
| ...and toggle a custom map marker layer with it | `Ex_FilterTeleportMenu.c` |
| add an ESP category | `Ex_FilterESP.c` |

Rules of thumb these examples follow: string callback names are checked at runtime, so a typo fails silently -
copy the handler name; permissions are hidden-by-`BindPermission` in the UI but must be re-checked on the server
(`JMPermissions.HasRPC`); a `modded class` must be declared in the same script layer (3_Game / 4_World / 5_Mission) as the class it extends.
