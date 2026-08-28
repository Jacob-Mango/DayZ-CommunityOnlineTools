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
└── JMMyForm.c          ← UI form

JM/COT/GUI/layouts/
└── my_form.layout      ← UI layout XML
```

## Step 1 — Module Class Template

```c
class JMMyModule : JMRenderableModuleBase
{
    // Constructor: register permissions
    void JMMyModule()
    {
        GetPermissionsManager().RegisterPermission( "Admin.MyModule.View" );
        GetPermissionsManager().RegisterPermission( "Admin.MyModule.Action" );
    }

    // --- Required overrides ---
    override string GetLayoutRoot()   { return "JM/COT/GUI/layouts/my_form.layout"; }
    override string GetTitle()        { return "My Module"; }          // or localization key
    override string GetIconName()     { return "My Module"; }          // button label
    override bool ImageIsIcon()       { return false; }                // false = text on button
    override bool HasAccess()         { return GetPermissionsManager().HasPermission( "Admin.MyModule.View" ); }

    // --- Webhook (optional) ---
    override string GetWebhookTitle() { return "My Module"; }
    override void GetWebhookTypes( out array<string> types ) { }

    // --- RPC range ---
    override int GetRPCMin() { return JMMyModuleRPC.INVALID; }
    override int GetRPCMax() { return JMMyModuleRPC.COUNT; }

    override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
    {
        switch ( rpc_type )
        {
        case JMMyModuleRPC.RequestData:  RPC_RequestData( ctx, sender, target );  break;
        case JMMyModuleRPC.Data:         RPC_Data( ctx, sender, target );         break;
        }
    }

    // Client-side: request data from server
    void RequestData()
    {
        ScriptRPC rpc = new ScriptRPC();
        rpc.Send( NULL, JMMyModuleRPC.RequestData, true, NULL );
    }

    // Server-side handler
    private void RPC_RequestData( ParamsReadContext ctx, PlayerIdentity sender, Object target )
    {
        if ( !IsMissionHost() ) return;
        if ( !GetPermissionsManager().HasPermission( "Admin.MyModule.View", sender ) ) return;

        // Build response
        ScriptRPC rpc = new ScriptRPC();
        rpc.Write( "some_value" );
        rpc.Send( NULL, JMMyModuleRPC.Data, true, sender );
    }

    // Client-side handler for server response
    private void RPC_Data( ParamsReadContext ctx, PlayerIdentity sender, Object target )
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

## Step 2 — RPC Enum

Pick a unique offset **not used by any existing module**:

```c
enum JMMyModuleRPC
{
    INVALID = 10900,   // ← choose unused offset (next free after 10800 ban module)

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

```c
private ref JMMySerialize m_Settings;

override void OnMissionLoaded()
{
    m_Settings = new JMMySerialize();
    if ( IsMissionHost() )
        Load();
}

void Load()
{
    // JsonFileLoader<JMMySerialize>.ReadFile( path, m_Settings )
}

void Save()
{
    // JsonFileLoader<JMMySerialize>.WriteFile( path, m_Settings )
}
```

## Optional: Webhook Integration

Call from server-side RPC handlers after executing an action:
```c
// Simple message
SendWebhook( "ActionType", "Player X did something" );

// With player info
JMPlayerInstance instance;
GetPermissionsManager().HasPermission( "Admin.MyModule.Action", sender, instance );
SendWebhook( "ActionType", instance, "Performed action on target" );
```

## JMRenderableModuleBase Full Override Reference

| Method | Required | Purpose |
|--------|----------|---------|
| `GetLayoutRoot()` | Yes | Path to .layout file |
| `GetTitle()` | Yes | Window title |
| `GetIconName()` | Yes | Button label or icon name |
| `ImageIsIcon()` | Yes | true=icon, false=text label |
| `HasAccess()` | Yes | Permission gate for showing module |
| `GetRPCMin()` / `GetRPCMax()` | Yes | RPC range (INVALID / COUNT) |
| `OnRPC(...)` | Yes | RPC dispatch switch |
| `GetWebhookTitle()` | No | Discord webhook name |
| `GetWebhookTypes()` | No | Webhook event type strings |
| `GetInputToggle()` | No | Keyboard shortcut string |
| `HasButton()` | No | false = no sidebar button |
| `ImageHasPath()` | No | true = full file path for icon |
| `GetImageSet()` | No | Image set name if using imageset |
| `OnMissionStart()` | No | Server init (load data, etc.) |
| `OnMissionLoaded()` | No | Client post-load init |
| `OnUpdate(timeslice)` | No | Per-frame (call `EnableUpdate()` first) |
| `OnClientDisconnect(...)` | No | Handle player disconnect |
| `OnClientPermissionsUpdated()` | No | Re-check access when perms change |
| `RegisterKeyMouseBindings()` | No | Bind keyboard shortcuts |
