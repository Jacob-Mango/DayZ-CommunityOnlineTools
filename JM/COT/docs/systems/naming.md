# Naming grammar

One vocabulary for lifecycle hooks, state setters and popups, so a new name can be guessed from the ones
already there.

## Adding and removing things - `Add<X>` / `Remove<X>ById` / `Remove<X>`

Every container that takes children uses the same four verbs, and ids are handed out by the container:

| Container | Add (returns the new id) | Remove |
|---|---|---|
| tab strip (`UIActionTabs`) | `AddTab( label, icon, panel )`, `AddPanel( tabId, panel )` | `RemoveTabById( id )`, `RemoveTab( panel )`, `RemovePanelById( id )`, `RemovePanel( panel )` |
| form (`JMFormBase`) | `AddTab( label, icon, buildFn, instance, controller )`, `AddPanel( tab, panel )`, `AddOverlay( control )` | `RemoveTabById( id )`, `RemoveTab( controller )`, `RemovePanelById( id )`, `RemovePanel( panel )`, `RemoveOverlay( control )` |
| context menu | `AddItem( id, ... )` | `RemoveItemById( id )` |
| filter menu | `AddPage( key, ... )` | `RemovePageById( key )` |
| card title bar | `AddCardHeaderAction( icon, ... )` (returns the button) | `RemoveCardHeaderAction( button )` |

`Add` returns the id; keep it instead of writing a number down (another mod may add to the same container).
Ids are stable - removing something never renumbers the rest - and never reused. `RemoveXById` takes the id,
`RemoveX` takes the object you added. The container only shows / hides / places what it is given; the widgets
you created stay yours to destroy.

## Lifecycle hooks (override these) - `On<Subject><Phase>`

| Subject | Hooks |
|---|---|
| form (`JMFormBase`) | `OnCreate` (legacy bridge: `OnInit`), `OnShow`, `OnHide`, `OnFocus`, `OnUnfocus`, `OnResize( w, h )`, `OnDestruct` |
| tab, on the form (indexed) | `OnTabCreate( tab, panel )`, `OnTabUpdate( tab )`, `OnTabFocus( tab )`, `OnTabUnfocus( tab )`, `OnTabResize( tab, w, h )`, `OnTabDestruct( tab )` |
| tab, as a class (`JMFormTab`) | `OnCreate( panel )`, `OnUpdate()`, `OnFocus()`, `OnUnfocus()`, `OnResize( w, h )`, `OnDestruct()` |

Phases: **Create** (once, build widgets) - **Focus** / **Unfocus** (became / stopped being the selected one) -
**Update** (repaint from current state; only while focused) - **Resize** - **Destruct** (form going away).

Tab indices are **assigned at runtime, never written as numbers**: another mod may append tabs to any form, so no
tab can know which index it will get. The strip is the one authority: a form keeps what `m_Tabs.AddTab( label, icon, panel )` returns for each tab (`m_TabIdX = m_Tabs.AddTab( ... )`); a
mod appends with the form's `AddTab( label, icon, buildFn, instance, controller )` and receives the next free index. A tab is
either a class extending `JMFormTab` (attached with `RegisterTab( id, tab )` or the `controller` argument of `AddTab`, after which it
reports its own index through `tab.GetTabId()`; the form forwards every hook), or has no class and the form overrides
the indexed hooks and compares `tab` against its reserved ids. Tabs added at runtime with
`AddTab( label, icon, "BuildFn" )` need neither: the builder is called on first show.

Order on a tab change (`HandleTabChange()`): `OnTabUnfocus( previous )` -> `OnTabCreate( new )` (first visit only)
-> `OnTabFocus( new )` -> `OnTabUpdate( new )`. Background refreshers call `UpdateTab( idx )` / `UpdateActiveTab()`;
they only run for the focused tab, so there is no `IsTabActive()` polling.

## State - `Set<Subject><State>( bool )` / `Is<Subject><State>()`

States: `Enabled`, `Visible`, `Focused`, `Selected`, `Expanded`, `Checked`.
Tabs: `SetTabEnabled`, `SetTabVisible`, `SetTabFocused` (on the form and on `UIActionTabs`).
Panels: `SetPanelEnabled( bool )` (right-hand panel + its "nothing selected" overlay).

## Doing the work - `Exec_<Action>` / `RPC_<Action>`

An action that can start on a client and finish on the server has three parts, always named the same way:

| Part | Name | Runs on |
|---|---|---|
| the public entry point a form or mod calls | `<Action>( ... )` (`SetGodMode`, `UsePreset`) | either - sends an RPC, or calls `Exec_` when this process owns the world |
| the work itself | `Exec_<Action>( ..., ident, instance )` | server (or the offline host) - the same method can run on either side |
| the incoming message | `RPC_<Action>( ctx, sender, target )` | server - reads, gates with `JMPermissions.HasRPC`, calls `Exec_` |

`Exec_` is reserved for work that can run on a client or on the server. Work that only ever runs on one side, and has a
counterpart on the other, is a `Server_<Action>` / `Client_<Action>` pair instead: `CommunityOnlineTools.Server_UpdateClient`
builds and sends the data, `Client_UpdateClient` receives and applies it (likewise `Server_SetClient` / `Client_SetClient`,
`Server_UpdateRole` / `Client_UpdateRole`, `JMPlayerModule.Server_StartSpectating` / `Client_StartSpectating`). A server-only
method with a client equivalent is not prefixed `Exec_`. `Server_` is therefore not retired - see
[deprecations.md](deprecations.md) for the names that went `Server_` -> `Exec_` -> `Server_` again.
Event handlers wired by name are `OnClick_<Control>` / `OnChange_<Control>`; the older `Click_<Control>` spelling survives only
where a modded form already overrides it.

When many actions share this shape, do not write the trio per action - register them; see "Repeated actions" in
`module-creation.md`.

## Popups (menu, prompt, dropdown, picker) - `Open*` / `Close` / `Toggle*` / `IsOpen`

`OpenAt( x, y )`, `OpenAtMouse()`, `OpenSlider( ... )`, `OpenOptions( ... )`, `ToggleAt( button )`.
`Show`/`Hide` are reserved for the visibility of an ordinary widget.

## Verbs

| Verb | Means |
|---|---|
| `Create<X>` | factory returning a new object (`UIActionManager.CreateSection`) |
| `Build<X>` | fill a container with widgets |
| `Init<X>` | one-time wiring of an object that already exists |
| `Update<X>` | repaint from current state, cheap |
| `Refresh<X>` | re-request the data (RPC / scan), then repaint |
| `Rebuild<X>` | destroy and recreate the widgets |
| `Declare` / `Bind` / `Has` | permissions: module declares, control binds, code checks |

Event handlers wired by name are `OnClick_<Control>` / `OnChange_<Control>`, or - with `SetOnClick` / `SetOnChange` -
`void On<Control>( UIActionBase action )` that fires for that one event.
