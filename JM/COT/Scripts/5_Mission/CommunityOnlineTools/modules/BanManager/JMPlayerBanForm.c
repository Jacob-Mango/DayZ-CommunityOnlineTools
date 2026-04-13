// =============================================================================
//  JMPlayerBanForm.c
// =============================================================================

class JMBanForm : JMFormBase
{
    // Duration presets
    static const int DURATION_COUNT = 8;

    private static string GetDurationLabel( int idx )
    {
        switch ( idx )
        {
        case 0: return "Permanent";
        case 1: return "1 Hour";
        case 2: return "6 Hours";
        case 3: return "12 Hours";
        case 4: return "1 Day";
        case 5: return "3 Days";
        case 6: return "1 Week";
        case 7: return "1 Month";
        }
        return "Permanent";
    }

    private static int GetDurationSeconds( int idx )
    {
        switch ( idx )
        {
        case 0: return -1;
        case 1: return 3600;
        case 2: return 21600;
        case 3: return 43200;
        case 4: return 86400;
        case 5: return 259200;
        case 6: return 604800;
        case 7: return 2592000;
        }
        return -1;
    }

    // -------------------------------------------------------------------------
    //  Widgets
    // -------------------------------------------------------------------------

    private UIActionScroller              m_Scroller;
    private Widget                        m_ContentWidget;

    // Shared search bar (filters both dropdown and ban list)
    private UIActionEditableTextPreview   m_SearchBar;

    // Ban Offline section
    private UIActionDropdownList          m_PlayerDropdown;
    private Widget                        m_BanOfflineActionsRow;

    // Ban list
    private Widget                        m_BanListWrapper;
    private UIActionText                  m_ActiveBansHeader;

    // Action toolbar (above ban list)
    private UIActionButton                m_EditDurationBtn;
    private UIActionButton                m_UnbanBtn;

    // Duration picker panel (shown inline when Edit Duration is clicked)
    private Widget                        m_DurationPickerWrapper;
    private UIActionSelectBox             m_DurationSelect;
    private bool                          m_DurationPickerVisible;

    // Selection tracking — SteamID of the checked ban entry
    private string                        m_SelectedBanSteamID;
    private string                        m_SelectedBanPlayerName;

    // Checkboxes mapped by SteamID so we can clear them on re-selection
    private ref map< string, UIActionCheckbox > m_BanCheckboxes = new map< string, UIActionCheckbox >();

    // Pending flow state
    private string                        m_PendingSteamID;
    private string                        m_PendingPlayerName;

    // Cached ban list
    private autoptr array<ref JMPlayerBan> m_BanList = new array<ref JMPlayerBan>();

    // Known players for the dropdown
    private ref array<string>             m_KnownGuids    = new array<string>();
    private ref array<string>             m_KnownNames    = new array<string>();
    private ref array<string>             m_FilteredGuids = new array<string>();
    private ref array<string>             m_FilteredNames = new array<string>();

    private JMBanModule                   m_Module;

    // -------------------------------------------------------------------------
    //  SetModule
    // -------------------------------------------------------------------------

    protected override bool SetModule( JMRenderableModuleBase mdl )
    {
        return Class.CastTo( m_Module, mdl );
    }

    // -------------------------------------------------------------------------
    //  OnInit
    // -------------------------------------------------------------------------

    override void OnInit()
    {
        m_Scroller      = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
        m_ContentWidget = m_Scroller.GetContentWidget();

        // ---- Top bar: Refresh + shared search -------------------------------
        Widget topRow = UIActionManager.CreateGridSpacer( m_ContentWidget, 1, 2 );

        UIActionButton refreshBtn = UIActionManager.CreateButton( topRow, "Refresh", this, "OnClick_Refresh" );
        refreshBtn.SetWidth( 0.22 );

        m_SearchBar = UIActionManager.CreateEditableTextPreview( topRow, "Search name / SteamID...", this, "OnChange_Search" );
        m_SearchBar.SetWidth( 0.78 );
        m_SearchBar.SetEditBoxWidth( 0.72 );

        UIActionManager.CreatePanel( m_ContentWidget, 0xFF444444, 2 );

        // ---- Ban Offline section --------------------------------------------
        UIActionManager.CreateText( m_ContentWidget, "Ban Offline Player" );

        // Player dropdown — full width, populated on data arrival
        array<string> emptyList = new array<string>();
        emptyList.Insert( "Loading players..." );
        m_PlayerDropdown = UIActionManager.CreateDropdownBox( m_ContentWidget, layoutRoot, "Select player...", emptyList );

        // Action row: Ban Selected + Ban by ID
        m_BanOfflineActionsRow = UIActionManager.CreateGridSpacer( m_ContentWidget, 1, 2 );
        UIActionButton banSelBtn = UIActionManager.CreateButton( m_BanOfflineActionsRow, "Ban Selected", this, "OnClick_ManualBanSelected" );
        banSelBtn.SetWidth( 0.72 );
        banSelBtn.SetColor( COLOR_RED );

        UIActionButton banByIDBtn = UIActionManager.CreateButton( m_BanOfflineActionsRow, "Ban by ID", this, "OnClick_ManualBan" );
        banByIDBtn.SetWidth( 0.26 );
        banByIDBtn.SetPosition( 0.74 );

        UIActionManager.CreatePanel( m_ContentWidget, 0xFF444444, 2 );

        // ---- Active Bans header + action toolbar ----------------------------
        m_ActiveBansHeader = UIActionManager.CreateText( m_ContentWidget, "Active Bans" );

        // Duration picker — hidden by default, shown when Edit Duration clicked
        m_DurationPickerWrapper = UIActionManager.CreateGridSpacer( m_ContentWidget, 1, 2 );

        array<string> durationOptions = new array<string>();
        for ( int d = 0; d < DURATION_COUNT; d++ )
            durationOptions.Insert( GetDurationLabel( d ) );
        m_DurationSelect = UIActionManager.CreateSelectionBox( m_DurationPickerWrapper, "Duration:", durationOptions );
        m_DurationSelect.SetSelectorWidth( 0.65 );

        Widget durationBtnRow = UIActionManager.CreateGridSpacer( m_DurationPickerWrapper, 2, 1 );
        UIActionButton applyDurBtn = UIActionManager.CreateButton( durationBtnRow, "Apply", this, "OnClick_ApplyDuration" );
        UIActionButton cancelDurBtn = UIActionManager.CreateButton( durationBtnRow, "Cancel", this, "OnClick_CancelDuration" );

        m_DurationPickerWrapper.Show( false );
        m_DurationPickerVisible = false;

        // Toolbar: Edit Duration + Unban
        Widget toolbarRow = UIActionManager.CreateGridSpacer( m_ContentWidget, 1, 2 );
        m_EditDurationBtn = UIActionManager.CreateButton( toolbarRow, "Edit Duration", this, "OnClick_EditDuration" );
        m_EditDurationBtn.SetWidth( 0.72 );
        m_EditDurationBtn.Disable();

        m_UnbanBtn = UIActionManager.CreateButton( toolbarRow, "Unban", this, "OnClick_Unban" );
        m_UnbanBtn.SetWidth( 0.26 );
        m_UnbanBtn.SetPosition( 0.74 );
        m_UnbanBtn.SetColor( COLOR_RED );
        m_UnbanBtn.Disable();

        UIActionManager.CreatePanel( m_ContentWidget, 0xFF555555, 1 );

        // ---- Ban list (populated dynamically) ------------------------------
        m_BanListWrapper = UIActionManager.CreateGridSpacer( m_ContentWidget, 1, 1 );
        UIActionManager.CreateText( m_BanListWrapper, "Loading..." );

        m_Scroller.UpdateScroller();
    }

    // -------------------------------------------------------------------------
    //  OnShow
    // -------------------------------------------------------------------------

    override void OnShow()
    {
        super.OnShow();

        if ( m_Module )
            m_Module.RequestBanList();
    }

    override void OnClientPermissionsUpdated()
    {
        super.OnClientPermissionsUpdated();
        RebuildBanList();
    }

    override void OnSettingsUpdated()
    {
        super.OnSettingsUpdated();

        if ( m_Module )
            m_Module.RequestBanList();
    }

    // -------------------------------------------------------------------------
    //  PopulateBanList — called from module on RPC receive
    // -------------------------------------------------------------------------

    void PopulateBanList( array<ref JMPlayerBan> bans, array<string> playerGuids, array<string> playerNames )
    {
        m_BanList.Clear();
        foreach ( JMPlayerBan ban : bans )
            m_BanList.Insert( ban );

        m_KnownGuids.Clear();
        m_KnownNames.Clear();
        for ( int i = 0; i < playerGuids.Count(); i++ )
        {
            m_KnownGuids.Insert( playerGuids[i] );
            m_KnownNames.Insert( playerNames[i] );
        }

        string currentFilter = "";
        if ( m_SearchBar )
            currentFilter = m_SearchBar.GetText();

        RebuildPlayerDropdown( currentFilter );
        RebuildBanList();
    }

    // -------------------------------------------------------------------------
    //  RebuildPlayerDropdown
    // -------------------------------------------------------------------------

    private void RebuildPlayerDropdown( string filter )
    {
        if ( !m_PlayerDropdown )
            return;

        m_FilteredGuids.Clear();
        m_FilteredNames.Clear();

        string filterLow = filter;
        filterLow.ToLower();

        for ( int i = 0; i < m_KnownNames.Count(); i++ )
        {
            string nameLow = m_KnownNames[i];
            nameLow.ToLower();
            string guidLow = m_KnownGuids[i];
            guidLow.ToLower();

            if ( filter == "" || nameLow.IndexOf( filterLow ) != -1 || guidLow.IndexOf( filterLow ) != -1 )
            {
                m_FilteredGuids.Insert( m_KnownGuids[i] );
                m_FilteredNames.Insert( m_KnownNames[i] );
            }
        }

        array<string> displayList = new array<string>();
        if ( m_FilteredNames.Count() == 0 )
        {
            displayList.Insert( "No players found" );
        }
        else
        {
            for ( int j = 0; j < m_FilteredNames.Count(); j++ )
                displayList.Insert( m_FilteredNames[j] + "  (" + m_FilteredGuids[j] + ")" );
        }

        m_PlayerDropdown.SetItems( displayList );
    }

    // -------------------------------------------------------------------------
    //  RebuildBanList
    // -------------------------------------------------------------------------

    private void RebuildBanList()
    {
        // Clear selection whenever list rebuilds
        m_SelectedBanSteamID    = "";
        m_SelectedBanPlayerName = "";
        m_BanCheckboxes.Clear();
        UpdateToolbarState();
        HideDurationPicker();

        if ( m_BanListWrapper )
            delete m_BanListWrapper;

        m_BanListWrapper = UIActionManager.CreateGridSpacer( m_ContentWidget, 1, 1 );

        if ( !m_BanList || m_BanList.Count() == 0 )
        {
            if ( m_ActiveBansHeader )
                m_ActiveBansHeader.SetLabel( "Active Bans (0)" );
            UIActionManager.CreateText( m_BanListWrapper, "No active bans." );
            m_Scroller.UpdateScroller();
            return;
        }

        string filter = "";
        if ( m_SearchBar )
        {
            filter = m_SearchBar.GetText();
            filter.ToLower();
        }

        if ( m_ActiveBansHeader )
            m_ActiveBansHeader.SetLabel( "Active Bans (" + m_BanList.Count() + ")" );

        bool canUnban = GetPermissionsManager().HasPermission( "Admin.Ban.Unban" );

        // Column headers: checkbox col + 4 data cols
        Widget header = UIActionManager.CreateGridSpacer( m_BanListWrapper, 1, 5 );
        UIActionManager.CreateText( header, ""                 );  // checkbox column
        UIActionManager.CreateText( header, "Player / SteamID" );
        UIActionManager.CreateText( header, "Reason"           );
        UIActionManager.CreateText( header, "Expires"          );
        UIActionManager.CreateText( header, "Banned By"        );

        UIActionManager.CreatePanel( m_BanListWrapper, 0xFF555555, 1 );

        foreach ( JMPlayerBan ban : m_BanList )
        {
            if ( filter != "" )
            {
                string nameLower = ban.PlayerName;
                nameLower.ToLower();
                string idLower = ban.SteamID;
                idLower.ToLower();

                if ( nameLower.IndexOf( filter ) == -1 && idLower.IndexOf( filter ) == -1 )
                    continue;
            }

            Widget row = UIActionManager.CreateGridSpacer( m_BanListWrapper, 1, 5 );

            // Checkbox — only rendered if user has unban permission
            if ( canUnban )
            {
                UIActionCheckbox cb = UIActionManager.CreateCheckbox( row, "", this, "OnClick_BanRowCheckbox", false );
                cb.SetData( new JMStringData( ban.SteamID ) );
                cb.SetWidth( 0.06 );
                m_BanCheckboxes.Insert( ban.SteamID, cb );
            }
            else
            {
                UIActionManager.CreateText( row, "" );
            }

            // Player name + SteamID stacked
            Widget nameBlock = UIActionManager.CreateGridSpacer( row, 2, 1 );
            UIActionManager.CreateText( nameBlock, ban.PlayerName );
            UIActionManager.CreateText( nameBlock, ban.SteamID    );

            UIActionManager.CreateText( row, ban.Message           );
            UIActionManager.CreateText( row, ban.GetExpiryString() );
            UIActionManager.CreateText( row, ban.IssuedByName      );

            UIActionManager.CreatePanel( m_BanListWrapper, 0x22FFFFFF, 1 );
        }

        m_Scroller.UpdateScroller();
    }

    // -------------------------------------------------------------------------
    //  Ban row checkbox — only one can be checked at a time
    // -------------------------------------------------------------------------

    void OnClick_BanRowCheckbox( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        UIActionCheckbox cb;
        if ( !Class.CastTo( cb, action ) )
            return;

        JMStringData data;
        if ( !Class.CastTo( data, action.GetData() ) )
            return;

        string steamID = data.Value;

        if ( cb.IsChecked() )
        {
            // Uncheck any previously checked entry
            foreach ( string sid, UIActionCheckbox other : m_BanCheckboxes )
            {
                if ( sid != steamID && other && other.IsChecked() )
                    other.SetChecked( false );
            }

            // Find the player name from the ban list
            m_SelectedBanSteamID    = steamID;
            m_SelectedBanPlayerName = steamID;
            foreach ( JMPlayerBan ban : m_BanList )
            {
                if ( ban.SteamID == steamID )
                {
                    m_SelectedBanPlayerName = ban.PlayerName;
                    break;
                }
            }
        }
        else
        {
            // Unchecked — clear selection
            m_SelectedBanSteamID    = "";
            m_SelectedBanPlayerName = "";
            HideDurationPicker();
        }

        UpdateToolbarState();
    }

    // -------------------------------------------------------------------------
    //  Toolbar state
    // -------------------------------------------------------------------------

    private void UpdateToolbarState()
    {
        bool hasSelection = ( m_SelectedBanSteamID != "" );
        bool canUnban     = GetPermissionsManager().HasPermission( "Admin.Ban.Unban" );

        if ( m_EditDurationBtn )
        {
            if ( hasSelection && canUnban ) m_EditDurationBtn.Enable();
            else                            m_EditDurationBtn.Disable();
        }

        if ( m_UnbanBtn )
        {
            if ( hasSelection && canUnban ) m_UnbanBtn.Enable();
            else                            m_UnbanBtn.Disable();
        }
    }

    // -------------------------------------------------------------------------
    //  Duration picker (inline)
    // -------------------------------------------------------------------------

    void OnClick_EditDuration( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        if ( m_SelectedBanSteamID == "" )
            return;

        ShowDurationPicker();
    }

    private void ShowDurationPicker()
    {
        if ( !m_DurationPickerWrapper )
            return;

        m_DurationPickerWrapper.Show( true );
        m_DurationPickerVisible = true;

        if ( m_DurationSelect )
            m_DurationSelect.SetSelection( 0, false );

        if ( m_Scroller )
            m_Scroller.UpdateScroller();
    }

    private void HideDurationPicker()
    {
        if ( !m_DurationPickerWrapper )
            return;

        m_DurationPickerWrapper.Show( false );
        m_DurationPickerVisible = false;

        if ( m_Scroller )
            m_Scroller.UpdateScroller();
    }

    void OnClick_ApplyDuration( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        if ( !m_Module || m_SelectedBanSteamID == "" || !m_DurationSelect )
            return;

        int idx = m_DurationSelect.GetSelection();
        if ( idx < 0 || idx >= DURATION_COUNT )
            idx = 0;

        m_Module.EditBanDuration( m_SelectedBanSteamID, GetDurationSeconds( idx ) );

        COTCreateLocalAdminNotification( new StringLocaliser(
            "Updated ban duration for: " + m_SelectedBanPlayerName + " - " + GetDurationLabel( idx ) ) );

        HideDurationPicker();
        m_Module.RequestBanList();
    }

    void OnClick_CancelDuration( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        HideDurationPicker();
    }

    // -------------------------------------------------------------------------
    //  Unban
    // -------------------------------------------------------------------------

    void OnClick_Unban( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        if ( m_SelectedBanSteamID == "" )
            return;

        m_PendingSteamID    = m_SelectedBanSteamID;
        m_PendingPlayerName = m_SelectedBanPlayerName;

        CreateConfirmation_Two( JMConfirmationType.INFO, "Confirm Unban",
            "Remove the active ban for " + m_PendingPlayerName + " (" + m_PendingSteamID + ")?",
            "#STR_COT_GENERIC_NO", "", "#STR_COT_GENERIC_YES", "OnClick_UnbanConfirm" );
    }

    void OnClick_UnbanConfirm( JMConfirmation confirmation )
    {
        if ( m_PendingSteamID == "" )
            return;

        ScriptRPC rpc = new ScriptRPC();
        rpc.Write( m_PendingSteamID );
        rpc.Send( NULL, JMBanModuleRPC.UnbanPlayer, true, NULL );

        m_PendingSteamID    = "";
        m_PendingPlayerName = "";
    }

    // -------------------------------------------------------------------------
    //  Refresh
    // -------------------------------------------------------------------------

    void OnClick_Refresh( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        if ( m_Module )
            m_Module.RequestBanList();
    }

    // -------------------------------------------------------------------------
    //  Shared search — filters both dropdown and ban list
    // -------------------------------------------------------------------------

    void OnChange_Search( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CHANGE )
            return;

        if ( m_SearchBar )
            m_SearchBar.SetTextPreview( "" );

        string filter = "";
        if ( m_SearchBar )
            filter = m_SearchBar.GetText();

        RebuildPlayerDropdown( filter );
        RebuildBanList();
    }

    // -------------------------------------------------------------------------
    //  Ban Offline — from dropdown selection
    // -------------------------------------------------------------------------

    void OnClick_ManualBanSelected( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        if ( !m_PlayerDropdown || m_FilteredGuids.Count() == 0 )
            return;

        string selected = m_PlayerDropdown.GetText();
        if ( selected == "" || selected == "No players found" || selected == "Select player..." || selected == "Loading players..." )
            return;

        // Match the selected display string back to a GUID
        for ( int i = 0; i < m_FilteredNames.Count(); i++ )
        {
            string display = m_FilteredNames[i] + "  (" + m_FilteredGuids[i] + ")";
            if ( display == selected )
            {
                m_PendingSteamID    = m_FilteredGuids[i];
                m_PendingPlayerName = m_FilteredNames[i];
                break;
            }
        }

        if ( m_PendingSteamID == "" )
            return;

        CreateConfirmation_Two( JMConfirmationType.EDIT, "Ban Reason",
            "Reason for banning " + m_PendingPlayerName + " (" + m_PendingSteamID + "):",
            "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "OnManualBan_GotReason" );
    }

    // -------------------------------------------------------------------------
    //  Ban Offline — manual SteamID entry
    // -------------------------------------------------------------------------

    void OnClick_ManualBan( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        CreateConfirmation_Two( JMConfirmationType.EDIT, "Ban Offline Player",
            "Enter the Steam 64 ID of the player to ban:",
            "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "OnManualBan_GotSteamID" );
    }

    void OnManualBan_GotSteamID( JMConfirmation confirmation )
    {
        string steamID = confirmation.GetEditBoxValue();
        steamID.Trim();

        if ( steamID == "" )
            return;

        m_PendingSteamID    = steamID;
        m_PendingPlayerName = steamID;

        for ( int i = 0; i < m_KnownGuids.Count(); i++ )
        {
            if ( m_KnownGuids[i] == steamID )
            {
                m_PendingPlayerName = m_KnownNames[i];
                break;
            }
        }

        CreateConfirmation_Two( JMConfirmationType.EDIT, "Ban Reason",
            "Reason for banning " + m_PendingPlayerName + " (" + m_PendingSteamID + "):",
            "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "OnManualBan_GotReason" );
    }

    void OnManualBan_GotReason( JMConfirmation confirmation )
    {
        if ( !m_Module || m_PendingSteamID == "" )
            return;

        string reason = confirmation.GetEditBoxValue();
        reason.Trim();

        if ( reason == "" )
        {
            m_PendingSteamID    = "";
            m_PendingPlayerName = "";
            return;
        }

        JMPlayerInstance myInstance = GetPermissionsManager().GetClientPlayer();
        string myID   = "";
        string myName = "Admin";
        if ( myInstance )
        {
            myID   = myInstance.GetGUID();
            myName = myInstance.GetName();
        }

        JMPlayerModule playerModule = CF_Modules<JMPlayerModule>.Get();
        if ( playerModule )
        {
            array<string> guids = { m_PendingSteamID };
            playerModule.Ban( guids, reason, -1 );
        }

        m_Module.Ban( m_PendingSteamID, m_PendingPlayerName, reason, -1, myID, myName );

        m_PendingSteamID    = "";
        m_PendingPlayerName = "";

        COTCreateLocalAdminNotification( new StringLocaliser( "Permanent ban issued." ) );

        m_Module.RequestBanList();
    }
}
