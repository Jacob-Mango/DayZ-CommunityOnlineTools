---
name: Module Creation
description: Step-by-step guide to creating a new COT module: class hierarchy, all required overrides, RPC enum offsets, registration, input bindings, settings persistence
type: project
---

## Class Hierarchy

```
JMModuleBase (4_World)
    └── JMRenderableModuleBase (4_World)   ← extends with window/form management
            └── JMMyModule (5_Mission)      ← your concrete module
```

## Files to Create

```
JM/COT/Scripts/5_Mission/CommunityOnlineTools/modules/MyModule/
├── JMMyModule.c        ← module logic + RPC handling
├── JMMyForm.c          ← UI form
├── Components/         ← parts of the form/module (added as the form grows)
└── Tabs/<Name>/        ← one folder per tab: JMMyFormTab<Name>.c and the helpers only it uses

JM/COT/GUI/layouts/
└── my_form.layout      ← UI layout XML
```

Only create `Components/` and `Tabs/` when there is something to put in them. The folder rules and how a form is
structured (tab classes, components) are in [module-folder-layout.md](module-folder-layout.md) and
[../ui/module-form-patterns.md](../ui/module-form-patterns.md).

## Step 1 — Module Class Template

A module states what it is once, in `DescribeModule()`, and lists its admin actions in `RegisterActions()`
(Step 1b). The base classes answer every getter (`GetTitle`, `GetLayoutRoot`, `GetLucideIcon`, `HasAccess`,
`GetRPCMin/Max`, `GetWebhookTitle/Types`, `DeclarePermissions`...) from that declaration, so a module does not
override them one by one. A getter the module still overrides wins, which is how a mod's `modded class`
override and a module migrating one getter at a time both keep working.

```c
class JMMyModule : JMRenderableModuleBase
{
    // Runs once, before the mission loads, on BOTH client and server: keep it pure data.
    override void DescribeModule( JMModuleInfo info )
    {
        super.DescribeModule( info );

        info.Title = "My Module";                                        // or a localisation key
        info.Icon = "sparkles";                                          // sidebar icon (lucide.dev name)
        info.Layout = "JM/COT/GUI/layouts/my_form.layout";
        info.Category = JMSideBarConfig.CATEGORY_OTHER;
        info.ViewPermission = JMConstants.PERM_MYMODULE_VIEW;            // HasAccess() and its registration
        info.InputToggle = "UACOTToggleMyModule";                        // optional keybind (Inputs.xml)
        info.WebhookTitle = "My Module";
        info.SetRPCRange( JMMyModuleRPC.INVALID, JMMyModuleRPC.COUNT );

        info.AddPermission( JMConstants.PERM_MYMODULE_ACTION );          // any other node the module owns
        info.AddWebhookType( "Something" );                              // types not sent by an action
    }

    override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
    {
        // Do NOT call super.OnRPC() here: it applies a per-sender 100 ms limit that drops the
        // 2nd..nth RPC of any burst (see rpc.md, "The rate limiter will silently eat your second RPC").
        // Throttle expensive handlers with JMRPCThrottle instead.

        switch ( rpc_type )
        {
        case JMMyModuleRPC.RequestData:  RPC_RequestData( ctx, sender, target );  break;
        case JMMyModuleRPC.Data:         RPC_Data( ctx, sender, target );         break;
        default:                         RunAction( sender, rpc_type, ctx );      break;   // see Step 1b
        }
    }

    // Called by the form's base OnShow() every time the window opens - the form needs no OnShow of its own.
    override void RequestData()
    {
        ScriptRPC rpc = new ScriptRPC();
        rpc.Send( NULL, JMMyModuleRPC.RequestData, true, NULL );
    }

    // Server-side handler (named after its RPC; the work itself belongs in Exec_<Action>)
    protected void RPC_RequestData( ParamsReadContext ctx, PlayerIdentity sender, Object target )
    {
        if ( !IsMissionHost() ) return;
        if ( !JMPermissions.HasRPC( JMConstants.PERM_MYMODULE_VIEW, sender ) ) return;

        // Build response
        ScriptRPC rpc = new ScriptRPC();
        rpc.Write( "some_value" );
        rpc.Send( NULL, JMMyModuleRPC.Data, true, sender );
    }

    // Client-side handler for server response
    protected void RPC_Data( ParamsReadContext ctx, PlayerIdentity sender, Object target )
    {
        if ( IsMissionHost() ) return;

        string value;
        if ( !ctx.Read( value ) ) return;

        JMMyForm form;
        if ( Class.CastTo( form, GetForm() ) )
            form.Populate( value );
    }
}
```

## Step 1b — Admin actions (client -> server)

An action that changes something is a `JMModuleAction` subclass, not a Send / Exec / RPC trio. The class names its RPC,
its permission, how the payload is written and read, what it does, and how it reads in the log and the webhook. The
module's base class does the rest: `SubmitAction()` sends it (or runs it when this process is the host), `RunAction()`
reads it, gates it on the permission, applies it, then writes the log line and the webhook.

```c
class JMMyModuleDelete: JMModuleAction
{
    string Name;

    override int GetRPC() { return JMMyModuleRPC.Delete; }
    override string GetPermission() { return JMConstants.PERM_MYMODULE_DELETE; }   // empty = REFUSED, not open
    override string GetWebhookType() { return "Delete"; }
    override int GetWebhookColor() { return JMConstants.WEBHOOK_COLOR_DANGER; }

    override void Write( ParamsWriteContext ctx ) { ctx.Write( Name ); }
    override bool Read( ParamsReadContext ctx ) { return ctx.Read( Name ); }   // false drops the action

    override bool Apply( JMModuleBase module, PlayerIdentity sender, JMPlayerInstance admin )
    {
        JMMyModule mine = JMMyModule.Cast( module );
        return mine && mine.DeleteByName( Name );                             // false = nothing happened: no log, no webhook
    }

    override string Describe() { return "Deleted " + Name; }                  // DescribeWebhook() when the webhook reads differently
}
```

```c
// In the module:
override void RegisterActions()
{
    super.RegisterActions();
    DefineAction( JMMyModuleDelete );      // permission and webhook type are declared for you
}

void Delete( string name )                 // the public method the form calls keeps its name
{
    JMMyModuleDelete action = new JMMyModuleDelete();
    action.Name = name;
    SubmitAction( action );
}
```

Rules: the action needs a constructor with no arguments (the server builds one to read into); `Write` and `Read` must put
down and take back the same fields in the same order; never pass a non-`NULL` target (see rpc.md). Several actions of a
module share one file when they are tiny subclasses of one base (see `JMCompensationsActions.c`).

**Where it does not fit:** an action whose permission depends on its payload (Namalsk: `Namalsk.<event>.Start`), one that
needs a notification back to the sender or a panel refresh afterwards (Ban Manager), or one that logs once per target
(Loadout spawns). Those keep a hand-written handler; the surrounding module still uses `DescribeModule()`.

## Step 2 — RPC Enum

Pick a unique offset **not used by any existing module**:

```c
enum JMMyModuleRPC
{
    INVALID = 11000,   // ← choose an unused offset; check with Workbench/Batchfiles/CheckRPCRanges.ps1

    // Client → Server
    RequestData,
    DoAction,

    // Server → Client
    Data,
    ActionResult,

    COUNT
}
```

See `rpc.md` for full list of used offsets.

## Step 3 — Register in JMModuleConstructor

File: `JM/COT/Scripts/5_Mission/CommunityOnlineTools/modules/JMModuleConstructor.c`

```c
modded class JMModuleConstructor
{
    override void RegisterModules( out TTypenameArray modules )
    {
        super.RegisterModules( modules );
        modules.Insert( JMMyModule );
    }
}
```

## Optional: Input Key Bindings

```c
override void RegisterKeyMouseBindings()
{
    Bind( new JMModuleBinding( "MyAction", "UAMyModuleAction", true ) );
}

void MyAction( UAInput input )
{
    if ( !input.LocalPress() ) return;
    // handle keypress
}
```

## Optional: Player Selection Integration

Subscribe in constructor, not OnMissionLoaded:
```c
void JMMyModule()
{
    // ...
    JMScriptInvokers.MENU_PLAYER_CHECKBOX.Insert( OnPlayer_Checked );
    JMScriptInvokers.MENU_PLAYER_BUTTON.Insert( OnPlayer_Button );
}

void OnPlayer_Checked( string guid, bool checked )
{
    if ( checked )
        JM_GetSelected().AddPlayer( guid );
    else
        JM_GetSelected().RemovePlayer( guid );
}
```

## Optional: Settings Persistence

Read and write JSON through `JMJsonFile<T>` (`Scripts/3_Game/CommunityOnlineTools/JMJsonFile.c`), not `JsonFileLoader<T>`
directly. It creates every missing folder above the path before a save (`MakeDirectory` is not recursive, and a first save
on a fresh profile used to write nothing) and logs the parser's reason when a file does not load.

```c
static JMMySerialize Load()
{
    JMMySerialize settings = new JMMySerialize();      // constructor defaults apply to fields the file omits

    if ( FileExist( FILE_PATH ) )
        JMJsonFile<JMMySerialize>.Load( FILE_PATH, settings );   // false, and logged, when the file does not parse
    else
    {
        settings.Defaults();
        settings.Save();
    }

    return settings;
}

void Save()
{
    JMJsonFile<JMMySerialize>.Save( FILE_PATH, this );
}
```

Decide what an unreadable file means before you write the `else`: `Load()` returns false for a missing file *and* a
malformed one. Falling back to `Defaults()` + `Save()` on the second overwrites the admin's data (Weather keeps its
`FileExist` branch for that reason).

## Optional: Webhook Integration

Call from server-side RPC handlers after executing an action:
```c
// Simple message
SendWebhook( "ActionType", "Player X did something" );

// With player info
JMPlayerInstance instance;
JMPermissions.Has( JMConstants.PERM_MYMODULE_ACTION, sender, instance );
SendWebhook( "ActionType", instance, "Performed action on target" );
```

## Repeated actions: register them, do not copy them

When a module has many actions that differ only in a few facts, do not write a Send / Exec / RPC trio per action. Describe
the action once and let one code path send, apply and receive it. The three built-in examples:

| Module | Registry | What one entry supplies | What is written once |
|---|---|---|---|
| Weather | `JMWeatherBase` payload classes (`JMWeatherPreset.c`) | RPC id, permission, webhook wording, how it applies | `Submit()`, `RPC_Payload()`, `Exec_Payload()` |
| Player toggles | `JMPlayerToggle` via `JMPlayerModule.RegisterToggles()` | RPC id, permission, wording, icon, how it applies and reads back | `SetToggle()`, `Exec_Toggle()`, `RPC_Toggle()`, the Actions-tab checkbox |
| Player stats | `JMPlayerStat` via `JMPlayerModule.RegisterStats()` | permission, range, colour bands, how it applies and reads back | `SetStat()`, `Exec_SetStat()`, `RPC_SetStat()`, the General-tab slider row |

A mod adds an entry by overriding the `Register...()` method, calling `super`, and adding its own - see the header comment of
`JMPlayerToggle.c` and `JMPlayerStat.c`. Entries keep their original RPC ids, so the wire format did not change when the
per-action methods were folded away.

Any other client -> server action is a `JMModuleAction` (Step 1b): compare `JMCompensationsActions.c`,
`JMTerritoryActions.c` and `JMLoadoutActions.c`.

When two things share a shape, that shape belongs in one registry or one helper - `JMPermissions` for permission checks,
`COT_CameraRaycast()` for a camera pick, the registries above for actions, `JMJsonFile<T>` for settings files,
`JMSearchMatcher` for a list's search box, `UIActionManager.CreateSearchRow` / `CreateSearchFlexRow` for its header row,
`JMExpansionLoadoutFile` for an Expansion prefab on disk.

## JMRenderableModuleBase Full Override Reference

Set these through `DescribeModule( JMModuleInfo info )` (Step 1). Each row names the `JMModuleInfo` member that feeds it; the
getter itself only needs overriding for something the descriptor cannot express (a `HasAccess()` that is not a single
permission, a title that changes at runtime).

| Method | `JMModuleInfo` member | Purpose |
|--------|-----------------------|---------|
| `GetLayoutRoot()` | `Layout` | Path to .layout file |
| `GetTitle()` | `Title` | Window title |
| `GetLucideIcon()` | `Icon` | Sidebar icon; `GetIconName()`, `ImageIsIcon()` and `ImageHasPath()` follow it |
| `GetCategory()` | `Category` | Sidebar group; defaults to `CATEGORY_OTHER` |
| `HasAccess()` | `ViewPermission` | Permission gate for showing the module; also registered for you |
| `GetRPCMin()` / `GetRPCMax()` | `SetRPCRange( min, max )` | RPC range (INVALID / COUNT) |
| `GetWebhookTitle()` | `WebhookTitle` | Discord webhook name |
| `GetWebhookTypes()` | `AddWebhookType( t )`, plus every action's `GetWebhookType()` | Webhook event type strings |
| `DeclarePermissions()` | `AddPermission( p )`, plus every action's `GetPermission()` | Nodes the module owns; override only to call `super` and add unusual ones |
| `GetInputToggle()` | `InputToggle` | Keyboard shortcut string |
| `HasButton()` | `HasButton` | false = no sidebar button |

| Method | Required | Purpose |
|--------|----------|---------|
| `OnRPC(...)` | Yes | RPC dispatch switch; `default:` calls `RunAction( sender, rpc_type, ctx )` |
| `RegisterActions()` | No | `DefineAction( Type )` for each `JMModuleAction` |
| `RequestData()` | No | What the form displays; the form's base `OnShow()` calls it |
| `ImageHasPath()` | No | true = full file path for icon |
| `GetImageSet()` | No | Image set name if using imageset |
| `OnMissionStart()` | No | Server init (load data, etc.) |
| `OnMissionLoaded()` | No | Client post-load init |
| `OnUpdate(timeslice)` | No | Per-frame (call `EnableUpdate()` first) |
| `OnClientDisconnect(...)` | No | Handle player disconnect |
| `OnClientPermissionsUpdated()` | No | Re-check access when perms change |
| `RegisterKeyMouseBindings()` | No | Bind keyboard shortcuts |
