// =============================================================================
//  JMBanForm.c
//
//  Admin GUI for the ban management module.
//
//  UI patterns confirmed from reading real COT source:
//
//  From JMExampleForm:
//    • UIActionManager.CreateScroller / CreateGridSpacer / CreateWrapSpacer
//    • UIActionManager.CreateText / CreateButton / CreateEditableText
//    • UIActionManager.CreateEditableTextPreview / CreateSelectionBox
//    • UIActionManager.CreatePanel (coloured separator)
//    • Event callbacks: void OnClick_X( UIEvent eid, UIActionBase action )
//    • Guard: if ( eid != UIEvent.CLICK ) return;
//    • COTCreateLocalAdminNotification for client-side toast
//
//  From JMLoadoutForm:
//    • m_ActionsWrapper deleted and rebuilt in OnSettingsUpdated
//    • SetData / GetData on buttons with a UIActionData subclass
//    • CreateConfirmation_Two( JMConfirmationType.INFO/EDIT, ... ) for confirms
//    • JMConfirmation.GetEditBoxValue() to read typed text back
//
//  From JMPlayerForm:
//    • OnClientPermissionsUpdated + UpdatePermission( widget, "perm.string" )
//    • CreateAdvancedPlayerConfirm / BanPlayerSingleConfirm pattern with EDIT
//    • SetColor( COLOR_RED ) on destructive buttons
//    • protected override bool SetModule( JMRenderableModuleBase mdl )
//
//  Layout file: JM/COT/GUI/layouts/ban_form.layout
//  Must contain a single widget named "panel".
// =============================================================================

// Carries SteamID + PlayerName through the unban confirmation flow.
// Must extend UIActionData (confirmed from JMLoadoutButtonData source).
class JMBanRowData : UIActionData
{
    string SteamID;
    string PlayerName;

    void JMBanRowData( string steamID, string playerName )
    {
        SteamID    = steamID;
        PlayerName = playerName;
    }
}

// =============================================================================

class JMBanForm : JMFormBase
{
    // -------------------------------------------------------------------------
    //  Widgets — created once in OnInit, never recreated
    // -------------------------------------------------------------------------

    private UIActionScroller            m_Scroller;
    private Widget                      m_ContentWidget;   // m_Scroller content

    // Toolbar
    private UIActionEditableTextPreview m_SearchFilter;
    private UIActionSelectBox           m_DurationSelect;

    // Ban list area — deleted and rebuilt each refresh (JMLoadoutForm pattern)
    private Widget                      m_BanListWrapper;

    // Duration options parallel to JMBanDuration enum
    protected ref array<string> m_DurationText =
    {
        "Permanent",
        "1 Hour",
        "6 Hours",
        "1 Day",
        "3 Days",
        "1 Week",
        "30 Days",
        "90 Days"
    };

    protected ref array<int> m_DurationSeconds =
    {
        -1,
        3600,
        21600,
        86400,
        259200,
        604800,
        2592000,
        7776000
    };

    // Currently selected duration value in seconds (-1 = permanent)
    private int m_SelectedDurationSeconds;

    // Temp carrier for the unban confirmation flow (JMLoadoutForm m_TempData pattern)
    private JMBanRowData m_TempData;

    // Temp carrier for the manual-ban two-step flow
    private string m_PendingSteamID;

    // Cached ban list — last received from server
    private autoptr array<ref JMPlayerBan> m_BanList = new array<ref JMPlayerBan>();

    // Typed module reference
    private JMBanModule m_Module;

    // -------------------------------------------------------------------------
    //  SetModule — typed cast, same pattern as every other form
    // -------------------------------------------------------------------------

    protected override bool SetModule( JMRenderableModuleBase mdl )
    {
        return Class.CastTo( m_Module, mdl );
    }

    // -------------------------------------------------------------------------
    //  OnInit — build the static chrome once
    // -------------------------------------------------------------------------

    override void OnInit()
    {
        m_SelectedDurationSeconds = -1;

        m_Scroller     = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
        m_ContentWidget = m_Scroller.GetContentWidget();

        // ---- Row 1: toolbar buttons ----------------------------------------
        Widget toolbar = UIActionManager.CreateGridSpacer( m_ContentWidget, 1, 2 );
            UIActionManager.CreateButton( toolbar, "Refresh",     this, "OnClick_Refresh"    );
            UIActionManager.CreateButton( toolbar, "Ban Offline", this, "OnClick_ManualBan"  );

        // ---- Row 2: duration selector + search -----------------------------
        Widget controlRow = UIActionManager.CreateGridSpacer( m_ContentWidget, 1, 2 );

            m_DurationSelect = UIActionManager.CreateSelectionBox( controlRow, "Duration:", m_DurationText, this, "OnClick_Duration" );
            m_DurationSelect.SetSelectorWidth( 1.0 );
            m_DurationSelect.SetSelection( 0, false );

            m_SearchFilter = UIActionManager.CreateEditableTextPreview( controlRow, "Search name / SteamID...", this, "OnChange_Search" );
            m_SearchFilter.SetWidth( 0.95 );
            m_SearchFilter.SetEditBoxWidth( 0.75 );

        // ---- Separator -----------------------------------------------------
        UIActionManager.CreatePanel( m_ContentWidget, 0xFF444444, 2 );

        // ---- Ban list area (populated by PopulateBanList) ------------------
        m_BanListWrapper = UIActionManager.CreateGridSpacer( m_ContentWidget, 1, 1 );
        UIActionManager.CreateText( m_BanListWrapper, "Loading..." );

        m_Scroller.UpdateScroller();
    }

    // -------------------------------------------------------------------------
    //  OnShow — request fresh data whenever the panel opens
    // -------------------------------------------------------------------------

    override void OnShow()
    {
        super.OnShow();

        if ( m_Module )
            m_Module.RequestBanList();
    }

    // -------------------------------------------------------------------------
    //  OnClientPermissionsUpdated — hide unban controls if no permission
    //  (JMPlayerForm pattern)
    // -------------------------------------------------------------------------

    override void OnClientPermissionsUpdated()
    {
        super.OnClientPermissionsUpdated();

        // Rebuild the list so Unban buttons appear/disappear correctly
        RebuildBanList();
    }

    // -------------------------------------------------------------------------
    //  OnSettingsUpdated — called by the module after any server-side mutation
    // -------------------------------------------------------------------------

    override void OnSettingsUpdated()
    {
        super.OnSettingsUpdated();

        if ( m_Module )
            m_Module.RequestBanList();
    }

    // -------------------------------------------------------------------------
    //  PopulateBanList — called from JMBanModule.RPC_BanList on the client
    // -------------------------------------------------------------------------

    void PopulateBanList( array<ref JMPlayerBan> bans )
    {
        m_BanList.Clear();
        foreach ( JMPlayerBan ban : bans )
            m_BanList.Insert( ban );
            
        RebuildBanList();
    }

    // -------------------------------------------------------------------------
    //  RebuildBanList — delete and recreate the list widget (JMLoadoutForm pattern)
    // -------------------------------------------------------------------------

    private void RebuildBanList()
    {
        if ( m_BanListWrapper )
            delete m_BanListWrapper;

        m_BanListWrapper = UIActionManager.CreateGridSpacer( m_ContentWidget, 1, 1 );

        if ( !m_BanList || m_BanList.Count() == 0 )
        {
            UIActionManager.CreateText( m_BanListWrapper, "No active bans." );
            m_Scroller.UpdateScroller();
            return;
        }

        string filter = "";
        if ( m_SearchFilter )
        {
            filter = m_SearchFilter.GetText();
            filter.ToLower();
        }

        bool canUnban = GetPermissionsManager().HasPermission( "Admin.Ban.Unban" );

        // ---- Column header -------------------------------------------------
        Widget header = UIActionManager.CreateGridSpacer( m_BanListWrapper, 1, 5 );
            UIActionManager.CreateText( header, "Player"    );
            UIActionManager.CreateText( header, "Reason"    );
            UIActionManager.CreateText( header, "Expires"   );
            UIActionManager.CreateText( header, "Issued By" );
            UIActionManager.CreateText( header, ""          );   // Unban column

        UIActionManager.CreatePanel( m_BanListWrapper, 0xFF555555, 1 );

        // ---- One row per ban -----------------------------------------------
        foreach ( JMPlayerBan ban : m_BanList )
        {
            // Apply search filter
            if ( filter != "" )
            {
                string nameLower = ban.PlayerName;
                nameLower.ToLower();
                string idLower = ban.SteamID;
                idLower.ToLower();

                if ( nameLower.IndexOf( filter ) == -1 && idLower.IndexOf( filter ) == -1 )
                    continue;
            }

            // Row: name+ID | reason | expiry | issued by | Unban button
            Widget row = UIActionManager.CreateGridSpacer( m_BanListWrapper, 1, 5 );

                // Player name (top) and SteamID (below) in a sub-grid
                Widget nameBlock = UIActionManager.CreateGridSpacer( row, 2, 1 );
                UIActionManager.CreateText( nameBlock, ban.PlayerName );
                UIActionManager.CreateText( nameBlock, ban.SteamID    );

                UIActionManager.CreateText( row, ban.Message           );
                UIActionManager.CreateText( row, ban.GetExpiryString() );
                UIActionManager.CreateText( row, ban.IssuedByName      );

                if ( canUnban )
                {
                    UIActionButton unbanBtn = UIActionManager.CreateButton( row, "Unban", this, "OnClick_Unban" );
                    unbanBtn.SetColor( COLOR_RED );
                    unbanBtn.SetData( new JMBanRowData( ban.SteamID, ban.PlayerName ) );
                }
                else
                {
                    UIActionManager.CreateText( row, "" );
                }

            UIActionManager.CreatePanel( m_BanListWrapper, 0x22FFFFFF, 1 );
        }

        m_Scroller.UpdateScroller();
    }

    // -------------------------------------------------------------------------
    //  Toolbar: Refresh
    // -------------------------------------------------------------------------

    void OnClick_Refresh( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        if ( m_Module )
            m_Module.RequestBanList();
    }

    // -------------------------------------------------------------------------
    //  Toolbar: Ban Offline — two-step EDIT confirmation (JMPlayerForm pattern)
    //  Step 1: enter SteamID
    //  Step 2: enter reason
    // -------------------------------------------------------------------------

    void OnClick_ManualBan( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        CreateConfirmation_Two( JMConfirmationType.EDIT, "Ban Offline Player", "Enter the Steam 64 ID of the player to ban:", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "OnManualBan_GotSteamID" );
    }

    void OnManualBan_GotSteamID( JMConfirmation confirmation )
    {
        string steamID = confirmation.GetEditBoxValue();
        steamID.Trim();

        if ( steamID == "" )
            return;

        m_PendingSteamID = steamID;

        CreateConfirmation_Two( JMConfirmationType.EDIT, "Ban Reason", "Reason for banning: " + steamID, "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "OnManualBan_GotReason" );
    }

    void OnManualBan_GotReason( JMConfirmation confirmation )
    {
        if ( !m_Module || m_PendingSteamID == "" )
            return;

        string reason = confirmation.GetEditBoxValue();
        reason.Trim();

        if ( reason == "" )
            return;

        JMPlayerInstance myInstance = GetPermissionsManager().GetClientPlayer();
        string myID   = "";
        string myName = "Admin";
        if ( myInstance )
        {
            myID   = myInstance.GetGUID();
            myName = myInstance.GetName();
        }

        // Use JMPlayerModule.Ban() for online players (it handles kick).
        // Our module's Ban() handles persistence for offline targets.
        JMPlayerModule playerModule;
        if ( Class.CastTo( playerModule, GetModuleManager().GetModule( JMPlayerModule ) ) )
        {
            array<string> guids = { m_PendingSteamID };
            playerModule.Ban( guids, reason, m_SelectedDurationSeconds );
        }

        // Always write to our persistent store as well
        m_Module.Ban( m_PendingSteamID, m_PendingSteamID, reason, m_SelectedDurationSeconds, myID, myName );

        m_PendingSteamID = "";

        COTCreateLocalAdminNotification( new StringLocaliser( "Ban issued." ) );

        m_Module.RequestBanList();
    }

    // -------------------------------------------------------------------------
    //  Unban — confirmation before sending RPC (JMLoadoutForm delete pattern)
    // -------------------------------------------------------------------------

    void OnClick_Unban( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        if ( !Class.CastTo( m_TempData, action.GetData() ) )
            return;

        CreateConfirmation_Two( JMConfirmationType.INFO, "Confirm Unban", "Remove the active ban for " + m_TempData.PlayerName + " (" + m_TempData.SteamID + ")?", "#STR_COT_GENERIC_NO", "", "#STR_COT_GENERIC_YES", "OnClick_UnbanConfirm" );
    }

    void OnClick_UnbanConfirm( JMConfirmation confirmation )
    {
        if ( !m_TempData )
            return;

        ScriptRPC rpc = new ScriptRPC();
        rpc.Write( m_TempData.SteamID );
        rpc.Send( NULL, JMBanModuleRPC.UnbanPlayer, true, NULL );

        m_TempData = NULL;
    }

    // -------------------------------------------------------------------------
    //  Duration selector
    // -------------------------------------------------------------------------

    void OnClick_Duration( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CHANGE )
            return;

        m_SelectedDurationSeconds = m_DurationSeconds[ m_DurationSelect.GetSelection() ];
    }

    // -------------------------------------------------------------------------
    //  Search filter
    // -------------------------------------------------------------------------

    void OnChange_Search( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CHANGE )
            return;

        m_SearchFilter.SetTextPreview( "" );
        RebuildBanList();
    }
}