# ExampleScriptOverride

A small addon that extends COT the way a sub-mod would. Each file shows one thing and stays short (under ~100
lines - the form and the RPC round trip are the longest); it is part of the normal build, so anything here that
stops compiling is a broken public API.

| I want to... | File |
|---|---|
| add my own sidebar module | `Module/JMCustomExampleModule.c` + `Module/Ex_ModuleRegistration.c` |
| give it a form (layout, tabs) | `Module/JMCustomExampleForm.c` + `GUI/layouts/JMCustomExampleForm.layout` |
| send a request to the server and get an answer back | `Network/Ex_RPCHandling.c` (also webhooks) |
| save settings to JSON | `Network/Ex_ConfigPersistence.c` |
| declare custom permission keys | `3_Game/ExamplePermissions.c` (+ `DeclarePermissions()` in the module) |
| hide a control without a permission | `Network/Ex_CustomPermissions.c` |
| add a tab to an existing form | `UI/Ex_CustomTabWithPanels.c` |
| inject a card into an existing tab | `UI/Ex_InjectedPanelExistingTab.c` |
| put buttons in a card's title bar | `UI/Ex_PanelHeaderButtons.c` |
| show tooltips | `UI/Ex_HintTooltip.c` |
| open a value prompt or a menu | `UI/Ex_Popups.c` |
| make a tab its own class (create/focus/update/unfocus) | `UI/Ex_TabController.c` (+ `SetTabEnabled`, `UpdateActiveTab` in `JMCustomExampleForm.c`) |
| build a search toolbar over a list | `UI/Ex_SearchRow.c` |
| add to the player right-click menu | `ContextMenus/Ex_PlayerContextMenu.c` |
| add to the 3D world right-click menu | `ContextMenus/Ex_ESPContextMenu.c` |
| add to the vehicle map/list right-click menu | `ContextMenus/Ex_VehicleContextMenu.c` |
| handle map click/right-click events | `ContextMenus/Ex_MapClickAction.c` |
| add custom map marker layers | `Filters/Ex_MapMarkerFilter.c` |
| add item quick-spawn presets | `Spawner/Ex_ItemSpawnerPreset.c` |
| add item color variant mappings | `Spawner/Ex_ItemColorVariants.c` |
| display custom player statistics | `Player/Ex_PlayerStatTracker.c` |
| add right-click actions to player inventory items | `Player/Ex_PlayerInventoryAction.c` |
| add admin toggle controls to player general tab | `Player/Ex_PlayerToggleState.c` |
| register custom vehicle category filters | `Vehicles/Ex_VehicleFilter.c` |
| dispatch custom Discord webhooks | `Webhooks/Ex_CustomWebhookPayload.c` |
| audit/log admin activity | `Logging/Ex_AdminAuditLog.c` |
| add custom anti-cheat detection signals | `AntiCheat/Ex_AntiCheatSignal.c` |
| register custom weather presets | `Weather/Ex_WeatherPreset.c` |
| register automated server events | `Events/Ex_EventRegistration.c` |
| register map editor object prefabs | `MapEditor/Ex_MapEditorObjectGroup.c` |
| track custom action history for undo/redo | `History/Ex_CustomActionHistory.c` |
| create sortable data tables | `UI/Ex_CustomDataTable.c` |
| create collapsible card sections | `UI/Ex_CustomCollapsibleSection.c` |
| create numeric slider controls | `UI/Ex_CustomSlider.c` |
| inject buttons into sidebar footer | `UI/Ex_InjectedFooterButton.c` |
| add a row to any filter menu (Player, Vehicles, Teleport, Object Spawner, ESP, Loot) | `Filters/Ex_FilterPlayerList.c` - `JMFilterRegistry.Register( scope, ... )`, a checkbox when it has a state callback |
| ...a plain click row | `Filters/Ex_FilterObjectSpawner.c` |
| ...and toggle a custom map marker layer with it | `Filters/Ex_FilterTeleportMenu.c` |
| add an ESP category | `Filters/Ex_FilterESP.c` |

Rules of thumb these examples follow: string callback names are checked at runtime, so a typo fails silently -
copy the handler name; permissions are hidden-by-`BindPermission` in the UI but must be re-checked on the server
(`JMPermissions.HasRPC`); a `modded class` must be declared in the same script layer (3_Game / 4_World / 5_Mission) as the class it extends.
