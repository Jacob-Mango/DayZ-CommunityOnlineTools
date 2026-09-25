# Deprecations

COT keeps old API names working while callers move to the new one. The convention
matches DayZ Expansion's:

- **Still works, has a replacement**: the old method is a thin forwarding wrapper that calls
  `JMDeprecated.WarnOnce( this, "Old() is deprecated. Please use New()." )` and then `New()`.
  The warning lands in the RPT once per message (`[COT] WARNING (<Class>): ...`), never per call.
- **No longer does anything**: bodyless `[Obsolete("Use New(args)")] void Old(args);`.
- **Zero users anywhere** (COT + Expansion): deleted outright.

Third-party mods: a `[COT] WARNING` line in your RPT names the class that made the call and what to use
instead. Nothing on this page is removed before the next major COT release.

| Old | New | Notes |
|---|---|---|
| `JMFormBase.InitTabState( n )` | `DeclareTabs( n )` | `DeclareTabs` also replaces `ExtendTabState`. |
| `JMFormBase.ExtendTabState( n )` | `DeclareTabs( n )` | Append tabs (e.g. from a modded form's `OnInit`). |
| `JMFormBase.BindPanelPermission( c, p )` | `BindPermission( c, p )` | Pure alias; had no callers. |
| `JMPlayerForm.BuildTabIfNeeded( i )` | `EnsureTabCreated( i )` + override `OnTabCreate( i, panel )` | An override of the old name is no longer called. |
| `JMFormBase.OnTabChanged( i )` | `OnTabFocus` / `OnTabUpdate` / `OnTabUnfocus` | Still called at the end of `OnTabFocus`, so existing overrides keep running; no warning (it is an override point, not a call). |
| `JMFormBase.ShowUI()` / `HideUI()` | `SetPanelEnabled( true / false )` | |
| `UIActionContextMenu.ShowAt( x, y )` | `OpenAt( x, y )` | |
| `UIActionValuePrompt.ShowSlider(...)` / `ShowOptions(...)` | `OpenSlider(...)` / `OpenOptions(...)` | |
| `UIActionFilterMenu.ToggleAtButton( w )` | `ToggleAt( w )` | |
| `JMFilterRegistry.RegisterPlayerFilter / RegisterItemFilter / RegisterObjectFilter / RegisterTeleportFilter / RegisterESPFilter( ... )` | `Register( JMFilterRegistry.PLAYERS / ITEMS / OBJECTS / TELEPORT / ESP, ... )` | One call for every scope; also `VEHICLES`. |
| `JMFilterRegistry.GetPlayerFilters() / GetItemFilters() / ...` | `Get( scope )` | Never returns null. |
| `JMObjectSpawnerForm.CategoryTable / CategoryGroupTable / CategoryGroupMembers` | `JMItemCategoryPicker.*` | The category table, group menu and recent chips are one shared class, also used by Loot Analysis. The form's `BuildCategoryRootPage`, `OnCategoryRootChange`, `BuildCategoryGroupPage`, `OnCategoryGroupChange`, `SelectCategory` and `OnClick_RecentChip` are gone - add menu rows with `JMFilterRegistry.Register( JMFilterRegistry.OBJECTS, ... )`. |
| Hard-coded tab numbers (`static const int TAB_X`) | `m_TabIdX = m_Tabs.AddTab( label, icon, panel )` + `RegisterTab( id, tab )`; on a form `AddTab( label, icon, fn, instance, controller )` | Tab indices are assigned at runtime so mods that append tabs cannot collide. |
| `UIActionTabs.AddContent( panel )` / `SetTabs( labels )` / `SetTabIcons( icons )` / `GetContentPanel( i )` | `AddTab( label, icon, panel )` / `AddPanel( tabId, panel )` / `GetPanel( tabId )` | Also `RemoveTabById`, `RemoveTab( panel )`, `RemovePanelById`, `RemovePanel( panel )`. Ids are stable and never reused. |
| `UIActionManager.CreateTabs( parent, labels, ... )` | `CreateTabStrip( parent, instance, fn )` + `AddTab` per tab | |
| `JMFormBase.RegisterOverlay( c )` / `UnregisterOverlay( c )`; `JMStatics.RegisterOverlay( w )` / `UnregisterOverlay( w )` | `AddOverlay( c )` / `RemoveOverlay( c )` | Same Add / Remove vocabulary as tabs and panels. |
| `JMPlayerForm.<member of the General / Position / Inventory / Actions tabs>` | The same member on `JMPlayerFormTabGeneral` / `JMPlayerFormTabPosition` / `JMPlayerFormTabInventory` / `JMPlayerFormTabActions` (reach them through `m_TabGeneralCtrl` etc.; each is `null` until its tab is first opened) | Every non-private method that moved keeps a forwarding stub on the form. The stub warns once (`JMPlayerForm.X() is deprecated. It moved to JMPlayerFormTabY.X()`) and forwards. Fields did **not** move with a forwarder - a modded class that reads one (`m_Health`, `m_PositionX` ...) has to go through the tab. |

## Player form extension points (not deprecated)

These stay on `JMPlayerForm`, are still virtual, and forward to the General tab, so existing `modded class JMPlayerForm`
overrides (DayZ-Expansion AI / Hardline / PersonalStorage, `ExampleScriptOverride`) keep working unchanged:
`InitActionWidgetsIdentity`, `InitActionWidgetsStats` (still `private`), `RefreshStats( force )`, `Click_ApplyStats`,
`OnClientPermissionsUpdated`, and the fields `m_Stamina`, `m_ApplyStats`, `m_RefreshStats`, `m_SelectedInstance`, `m_Module`.
The stats tab hands its three widgets to the form once the card is built (`SetLegacyStatWidgets`), so
`m_Stamina.GetLayoutRoot().GetParent()` still finds the slider grid.

Also kept as plain (non-warning) forwarders because something looks them up **by name on the form**: every `JMConfirmation`
callback (`StripMulti`, `HealSingle`, `ApplyStatsSelf` ...), `UpdateMapMarkers` (queued with `CallLater`), and the module RPC
entry points `OnInventoryUpdated`, `OnDiseaseMaskUpdated`, `OnBleedingStateUpdated`.

## Behaviour notes

- `JMFormBase.OnInit()` is the legacy name of `OnCreate()`. The base `OnCreate()` calls `OnInit()`, so an override of either
  works on a form that does not override `OnCreate` itself. Every in-repo form except `JMPlayerForm` overrides `OnCreate()`
  directly without calling `super`, so `JMFormBase.Init()` runs `OnInit()` itself right after `OnCreate()` for those forms
  (once - a form whose `OnCreate()` did reach the base never runs it twice). A third-party
  `modded class JMWeatherForm { override void OnInit() { super.OnInit(); ... } }` therefore keeps working, and still sees the
  form's own widgets already built. New code overrides `OnCreate()`; it does not need `super.OnCreate()` (the base only marks the
  legacy `OnInit()` as run and calls it). `JMPlayerForm` keeps its body in
  `OnInit()` because it is the form modded most often - both `override void OnInit()` and `override void OnCreate()`
  fragments work on it, as long as they call `super`.

## Old server entry points (`Server_*`)

Some `Server_X` methods were renamed `Exec_X`. Where a third party is known to override one, the old name stays as the method that
holds the body and `Exec_X` forwards to it, so `override Server_X` keeps running and `super.Server_X` still works. Neither is
warned about - they are override points, not calls. New code overrides `Exec_X`.

| Old (still the body) | New (forwards to it) | Known user |
|---|---|---|
| `JMObjectSpawnerModule.Server_DeleteEntity( obj, ident )` | `Exec_DeleteEntity( obj, ident )` | ZenCOT (lets Expansion AI be deleted) |
| `JMPlayerModule.Server_EndSpectating( ident )` | `Exec_EndSpectating( ident )` | DayZ-Expansion AI |

Every other `Server_X` that became `Exec_X` had no user in COT, Expansion or the `TODO/` addons and was renamed outright.

**Renamed back to `Server_`.** `Exec_` is only for work that runs on both a client and the server (see
[naming.md](naming.md)). A method that only runs on the server and has a `Client_` counterpart keeps its `Server_` name, so these
went back (abd4087e), with no forwarder:

| Was | Is |
|---|---|
| `CommunityOnlineTools.Exec_UpdateClient( guid, sendTo )` | `Server_UpdateClient( guid, sendTo )` |
| `CommunityOnlineTools.Exec_SetClient( player )` / `( player, identity )` | `Server_SetClient( player )` / `( player, identity )` |
| `CommunityOnlineTools.Exec_UpdateRole( role, toSendTo )` | `Server_UpdateRole( role, toSendTo )` |
| `JMPlayerModule.Exec_StartSpectating( guid, ident )` | `Server_StartSpectating( guid, ident )` |
| `JMPlayerModule.Exec_StartSpectating( object, ident )` - it only forwarded | `Server_StartSpectating( object, ident )` now holds the body again |

All are `protected`. `JMPlayerModule.Server_StartSpectating( object, ident )` is what DayZ-Expansion AI overrides, so it needs no
forwarder any more; `Server_StartSpectating` is no longer marked deprecated. `Exec_EndSpectating` and `Exec_DeleteEntity` still
forward to their `Server_` bodies as in the table above.

## Removed with no forwarder

A forwarder would be wrong here; these need a caller change.

| Removed | Use instead | Why no forwarder |
|---|---|---|
| `JMPlayerBan.Save( ban, guid )` | `CF_Modules<JMBanModule>.Get().Ban( steamID, name, message, seconds, issuedBy, issuedByName )` / `.Unban( steamID )` | Forwards to `JMBanModule` (`ban.BanDuration` is still the old absolute expiry; `-1` stays permanent, an expiry in the past lifts the ban). Only the writer is forwarded - the one-JSON-file-per-ban layout under `JMConstants.DIR_BANS` is gone, so code that *lists* bans from that folder sees nothing; read `Bans.json` or ask `JMBanModule`. It finds `JMBanModule` by name (it lives above `3_Game`), so it is a no-op if that module is not loaded. |
| `modded class JMPlayerBan { string SteamID; }` | Nothing - COT declares `SteamID` | Redeclaring it is a "multiple declaration" compile error. |
| `JMVehiclesMenu`, `JMVehiclesMapMarker` | `modded class JMVehiclesForm` / `JMVehiclesFormTabMap` / `JMVehiclesFormTabActions` | Compile stubs only (`JMVehiclesMenu.c`, `JMVehiclesMapMarker.c`): a `modded class` of either still compiles, so it cannot take the whole Mission script module down, but COT never creates one - the mod's buttons, map recolouring and handlers do not appear or run. The form is split into tabs and the map is a `UIActionMap` (`AddMarker`, `SetMarkerColor`, `ClearLayer`). The stubs warn once if anything constructs them. |
| `JMObjectSpawnerForm.m_ItemDataList`, `m_ClassList.GetSelectedRow()` / `GetItemText()` | `GetCurrentSelection()` | The class list is a `UIActionItemList` (`GetSelectedIndex()`, `GetSelectedItem()`). |
| `JMPlayerForm` list state: `m_PlayerList`, `m_PlayerListFilter`, `m_PlayerListCount` / `Peak` / `Selected`, `m_PlayerListSort`, `m_PlayerListScroller`, `m_PlayerListRows`, `m_RoleMembers`, `m_CollapsedRoles`, `m_FilterMenu`, `m_FilterShow*`, `m_PlayerPref*`, `m_PlayersPref1/2` | `m_Roster` (`JMPlayerRoster`) and `m_Roster.GetFilter()` (`JMPlayerRosterFilter`) | The player list moved out of the form. Still on the form and unchanged: `UpdatePlayerList`, `PassesListFilters` (protected, overridable), `OnPlayerRow_RightClick`, `m_PlayerMenu`, `m_PlayerMenuGUID`, `GetPrimaryRole`, `RoleDisplayName`, `IsRoleCollapsed`, `OnRoleHeader_Toggled/Checked`, `RunStatusRepair`, `AddStatusRepairItem`, `SetStatusRepairPermissions`. |
| `JMPlayerForm.ROW_MENU_*`, `LIST_FILTER_*`, `ROLE_UNGROUPED`, `GROUP_ANIM_DURATION`; `JMPlayerForm.INV_*`, `SCALE_MODE_*`, `DISEASE_INJECT_COUNT`, `HISTORY_*`, `MAP_*` (except `MAP_HEIGHT`), `COORD_DECIMALS_*`, `IDENTITY_*` (except `IDENTITY_LABEL_WIDTH` / `IDENTITY_VALUE_WIDTH`) | The same name on the class that owns it: `JMPlayerRowMenu`, `JMPlayerRosterFilter`, `JMPlayerRoster`, `JMPlayerFormTabInventory`, `JMPlayerFormTabActions`, `JMPlayerFormTabPosition`, `JMPlayerFormTabGeneral` | A `static const` cannot forward or warn. `SHOCK_CONSCIOUS`, `MAP_HEIGHT` and the two `IDENTITY_*` widths stay because other forms read them. |
| `JMEntityManagerForm.TAB_MAP` / `TAB_ACTIONS` / `TAB_INFO` | The id `AddTab()` returned, or `JMFormTab.GetTabId()` from inside a tab | A tab id is handed out by the tab strip, never numbered by a form. A `static const` cannot forward or warn. |
| `JMWeatherForm.TAB_OVERVIEW` / `TAB_SKY` / `TAB_PRECIPITATION` / `TAB_WIND` / `TAB_TIME` / `TAB_PRESETS` / `TAB_COUNT`; `JMESPForm.TAB_FILTERS` / `TAB_OBJECTS` / `TAB_SETTINGS` | The id `AddTab()` returned, or `JMFormTab.GetTabId()` from inside a tab | Same as above. |
| `JMWeatherForm` tab constants: `DEFAULT_LIGHTNING_INTERVAL`, `DEFAULT_SANDSTORM_*`, `CLEAR_THRESHOLD_MIN/MAX`, `QUICK_CELL_W`, `QUICK_ROW_H`, `SPECIAL_WEATHER_POLL_INTERVAL`, `ROW_LIVE_WORLD`, `ROW_CREATE_NEW`, `ROW_FIRST_PRESET`, `DURATION_MAX_MINUTES`, `TRANSITION_MAX_SECONDS`, `CHANCE_SLIDER_W`, `ADD_DROPDOWN_W`, `ADD_BUTTON_W/H` | The same name on `JMWeatherFormTabSky`, `JMWeatherFormTabOverview` or `JMWeatherFormTabPresets` | A `static const` cannot forward or warn. |
| `JMWeatherForm.CARDINAL_DIRECTIONS`, `CardinalFor`, `PI2DEG`, `DEG2PI`; `JMWeatherForm.m_DaysInMonth`, `FormatClock` | `JMWeatherFormTabWind.*`; `JMWeatherFormTabTime.*` | Static helpers used by one tab only. |
| `JMESPForm.OnRightClick_CategoryRow`, `OnChange_CategoryRow`, `OpenCategoryColourPopup` | `JMESPFormTabFilters` (the view-type rows are built there and are the callback target) | The rows call their own tab now. `JMESPForm.IconForViewType` still works, deprecated; use `JMESPViewTypeWidget.IconForViewType`. `JMESPViewTypeWidget.Build` takes the `JMESPFormTabFilters` tab instead of the form. |
| `JMTeleportForm.SUB_LOCATIONS` / `SUB_VEHICLES` / `SUB_PLAYERS`, `FILTER_MAP_MARKERS` / `FILTER_HELI_CRASHES` / `FILTER_TOXIC_ZONES` / `FILTER_VEH_*`; `JMTeleportForm.CATEGORY_NEW_ROW` | The same names on `JMTeleportFilter`; `JMTeleportLocationPopup.CATEGORY_NEW_ROW` | The filter and the save-location popup are their own classes. Still on the form, unchanged: `RefreshMapMarkers` (protected hook), `m_Map`, `GetMap()`, `RefreshOverlays`, `ApplyOverlays`, `MAP_LAYER_*`, `COLOR_*`, `OnClick_CategoryFilter`, `Click_OpenSavePopup`. |
| `JMLootAnalysisForm.MAP_LAYER_SCAN` / `MAP_LAYER_DIST`; `MENU_MK_*` | `JMLootAnalysisFormTabItemScan.MAP_LAYER_SCAN`; `JMLootAnalysisFormTabDistribution.MAP_LAYER_DIST`; `JMLootMarkerMenu.MENU_MK_*` | Owned by the tab or component that uses them. `m_MarkerMenu*` and `ShowMarkerMenu` moved to `JMLootMarkerMenu` (the form now holds it as `m_MarkerMenu`). |
| `JMVehiclesForm.TYPE_FILTER_*`, and the protected `m_FilterButton`, `m_FilterMenu`, `m_TypeFilter` (bitmask), `m_FilterShowOther`, `IsTypeShown`, `PassesTypeFilter`, `OnClick_TypeFilters`, `BuildFilterRootPage`, `OnFilterRootChange` | `m_TypeFilter` (`JMVehiclesTypeFilter`): `Passes( vehicle )`, `Reset()`, `TYPE_FILTER_*` | The type filter is its own class. The form's `m_TypeFilter` is now the component, not an `int`. A `modded class JMVehiclesForm` reading the old protected members needs updating. |
| `JMObjectSpawnerForm` property card: `m_PropsHost`, `m_PropsGrid`, `m_QuantityItem`, `m_HealthItem`, `m_RangeToggle`, `m_TemperatureItem`, `m_FoodStage`, `m_LiquidType`, `m_AmmoType`, `m_Disinfect`, the `m_*Pct` and `m_*Value` fields, `RebuildProperties`, `CaptureProperties`, `Click_Set*`, `HEALTH_GRADIENT_STOPS` | `m_Props` (`JMObjectSpawnerProperties`): `Build`, `Rebuild( classname )`, `ReadSpawnValues`, `RefreshPreviewHealth`, `RefreshPreviewFoodStage` | The card is its own class. New public getters on the form for it: `GetPreviewItem()`, `GetModule()`, `GetRightScroller()`. |
| `JMCameraForm` Traveling panel: `m_Waypoints`, `m_WaypointID`, `m_WaypointSelectBox`, `m_PathSelectBox`, `m_PathNames`, the `m_WP_Slider*` fields, `InitCameraTraveling`, the waypoint / path / travel handlers, `FlushWaypointEdits`, `UpdateUIWaypoint`, `UpdateDurationLabel` | `m_Travel` (`JMCameraTravelPanel`) | The panel is its own class. New public `JMCameraForm.GetModule()`. `SetEnableFullmapCamera`, `GetSelectedBookmarkName` and the Effects / Bookmarks / Settings members are unchanged. |
| `JMCompensationsModule.Exec_SpawnCursor / Exec_SpawnTarget / Exec_SpawnPlayers / Exec_Delete` and `RPC_SpawnCursor / RPC_SpawnTarget / RPC_SpawnPlayers / RPC_Delete`; `JMLoadoutModule.Exec_Delete` / `RPC_Delete`; the `SetLevel` case of `JMTerritoryModule.OnRPC` | The public `SpawnCursor` / `SpawnTarget` / `SpawnPlayers` / `Delete` / `SendSetLevel` methods, unchanged; or `SubmitAction( new JMCompensationDelete() )` etc. | These were `protected` and only reachable from a subclass. They are now `JMModuleAction` classes (`JMCompensationsActions.c`, `JMLoadoutActions.c`, `JMTerritoryActions.c`); the permission check, log line and webhook happen in `JMModuleBase.ExecAction`. The wire format and RPC ids are unchanged. New work-only methods on `JMCompensationsModule`: `SpawnAtPosition`, `SpawnOnEntity`, `SpawnOnPlayers`, `DeleteCompensation`. |
| `JMTerritoryForm` / `JMPlayerBanForm` / `JMRoleManagerForm` / `JMAntiCheatForm` / `JMCompensationsForm` `OnShow()` overrides | `override void RequestData()` on the module | The base `OnShow()` calls `JMRenderableModuleBase.RequestData()`. A `modded class` of one of these forms that calls `super.OnShow()` is unaffected. `JMCompensationsForm.OnShow` used to skip `super.OnShow()`; it no longer overrides it. |

`JMPlayerBan.DeleteBanFile( guid, steamID )` now returns `bool` (whether a timed ban was marked expired) instead of nothing, so callers that
branch on it compile. It no longer deletes a file.

## Third-party RPC ranges

`docs/systems/rpc.md` lists the ids the `TODO/` addons use. `COT_AnimalManager` (was 10600) and `Mu_XMLEditor` (was 10480) collided with
`JMWeatherModuleRPC` and `JMCompensationsModuleRPC` and were moved; run `Workbench/Batchfiles/CheckRPCRanges.ps1 -ScriptsRoot <repo>`
after adding a mod that owns an `...RPC` enum.

## Still supported

- Registering permissions from a module's constructor (`GetPermissionsManager().RegisterPermission( ... )`) still works - the
  constructor runs before the mission loads on both sides. `DeclarePermissions()` is the preferred spelling and is what new modules use.

## Not deprecated (but easy to confuse)

| API | Use it for |
|---|---|
| `JMFormBase.BindPermission( control, perm )` | Normal case: evaluate now and track every permission change. |
| `JMFormBase.UpdatePermission( control, perm )` | Evaluate ONCE, untracked - rebuilt row lists, or re-applying from your own `OnClientPermissionsUpdated()` (what Expansion's PersonalStorage / AI / Hardline forms do). |

## Behaviour changes worth knowing

- `JMPermissions.Has`, `HasRPC` and `Register` no longer return `false` quietly when `GetPermissionsManager()` is `NULL`, or
  when `HasRPC` gets no sender identity on a dedicated server (7b46e56f). They go straight to the manager and fail loudly (a
  script exception in the log). A `NULL` there is a bug in the calling mod, and the sender of an RPC received on the server is
  never `NULL`. Code that relied on the quiet `false` must pass the sender.
- `JMPermissionManager.HasPermission( perm, ident )` and `HasPermissionRPC( perm, ident )` shortcut to `true` only when
  `IsMissionOffline()` (271da598). They used `IsTrustedLocalHost()`, which also covered a mission host that is not a dedicated
  server; that protected wrapper is gone. A listen-server host now goes through the regular lookup like anyone else.
- `JMPermissionManager.HasPermission( perm )` (no `PlayerIdentity`) on a **server** used to raise a script
  `Error`. It now returns `false` and logs one `[COT] WARNING` - pass the sender:
  `HasPermission( perm, sender )` or `JMPermissions.Has( perm, sender )`.
- `JMRenderableModuleBase.ImageIsIcon()` / `ImageHasPath()` now follow `GetLucideIcon()`; modules that
  already override them are unaffected.
- Every module getter (`GetTitle`, `GetLayoutRoot`, `GetLucideIcon`, `GetCategory`, `HasAccess`, `HasButton`, `GetInputToggle`,
  `GetWebhookTitle`, `GetWebhookTypes`, `GetRPCMin/Max`, `DeclarePermissions`) now answers from the module's `DescribeModule()`
  declaration (`JMModuleInfo`). A getter a module or a `modded class` still overrides wins, so nothing needs to change;
  `super.GetTitle()` and friends return the declared value. Two things to know: `DescribeModule()` runs once, before the
  mission loads, so it must not read anything that only exists later; and `GetWebhookTypes` on a described module lists
  the declared types plus the ones its actions send, so an override that does not call `super` lists only its own.
- `JMModuleBase.SubmitAction()` on the host runs the action directly. `JMTerritoryModule.SendSetLevel()` used to send an RPC even
  from the host; it now applies the change in-process, like every other host-side action.
- Saving a settings file through `JMJsonFile<T>.Save()` creates the missing folders first. Teleport, Weather and ESP settings
  used to write nothing on a profile where their folder did not exist yet.
