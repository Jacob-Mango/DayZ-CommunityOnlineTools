# Filter menus

Every "filter button next to a search box" in COT is one `UIActionFilterMenu`: a single context menu driven as a
small stack of named pages (a root list, optionally drill-downs into checkbox lists, back). One class, one way to
build it, one way for a mod to add a row.

## Building one (from a form)

```
void OnClick_Filter( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CLICK )
        return;

    if ( !m_FilterMenu )
    {
        // created under the form, registered as an overlay (tab change / hide closes it),
        // opens under the button, serves the registry scope
        m_FilterMenu = UIActionManager.CreateOverlayFilterMenu( this, m_FilterButton.GetLayoutRoot(), JMFilterRegistry.VEHICLES );
        if ( !m_FilterMenu )
            return;

        m_FilterMenu.AddPage( "root", this, "BuildFilterRootPage", "", false, "", this, "OnFilterRootChange" );
    }

    m_FilterMenu.ToggleAt( m_FilterButton.GetLayoutRoot() );   // a second click closes it
}

void BuildFilterRootPage( UIActionFilterMenu menu )
{
    menu.AddToggleRow( "cars", "Cars", m_ShowCars, JMConstants.Lucide( "car" ) );   // dims when off
}

void OnFilterRootChange( string id )   // leaf row clicked: flip your state; the menu repaints itself
{
    if ( id == "cars" )
        m_ShowCars = !m_ShowCars;
}
```

Rows: `AddRow( id, label, icon, colour, enabled, gotoPage )` (a non-empty `gotoPage` makes a drill-down row),
`AddToggleRow( id, label, on, icon )`. `AddPage( ..., closeOnPick )` closes the menu after a pick (a command, not a
toggle). Never rebuild the menu from inside its own click handler - the menu defers the rebuild for you.

## Extending one (from a mod)

```
JMFilterRegistry.Register( JMFilterRegistry.PLAYERS, "only_admins", "Only Admins", "", 0, this, "OnAdmins", "IsAdmins" );

void OnAdmins( string id ) { m_OnlyAdmins = !m_OnlyAdmins; UpdatePlayerList( true ); }
bool IsAdmins()            { return m_OnlyAdmins; }   // optional - makes the row a checkbox
```

| Scope | Menu |
|---|---|
| `PLAYERS` | Player list filters |
| `VEHICLES` | Vehicles type filters |
| `TELEPORT` | Teleport map filters (root page) |
| `OBJECTS` | Object Spawner category menu (root page) |
| `ESP` | ESP tracked-object filters (root page) |
| `ITEMS` | Loot Analysis category menu |

Registering the same id again replaces the row. Rows appear on the menu's root page after the built-in ones, and
their callbacks run instead of the page's own change handler. See `ExampleScriptOverride/5_Mission/Ex_Filter*.c`.
