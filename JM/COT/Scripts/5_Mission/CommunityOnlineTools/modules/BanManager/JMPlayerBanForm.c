// =============================================================================
//  JMPlayerBanForm.c
// =============================================================================

class JMBanForm : JMFormBase
{
    //! Tab indices - what the strip's AddTab() returned for each tab, never written as numbers.
    protected int m_TabIdBans;
    protected int m_TabIdOffline;

    // Duration presets
    static const int DURATION_COUNT = 8;

    // -------------------------------------------------------------------------
    //  Widgets
    // -------------------------------------------------------------------------

    protected UIActionTabs                  m_Tabs;
    protected Widget                        m_TabListPanel;
    protected Widget                        m_TabOfflinePanel;

    //! Per-tab logic, one class per tab file - JMBanFormTabBans.c /
    //! JMBanFormTabOffline.c. protected, not private: sub-mods reach for
    //! these through the form.
    protected ref JMBanFormTabBans    m_TabBans;
    protected ref JMBanFormTabOffline m_TabOffline;
    protected ref UIActionFlexRow         m_SearchRow;
    // Shared search bar (filters both dropdown and ban list)
    protected UIActionSearchBox             m_SearchBar;

    // Action toolbar (above ban list) - lives outside any tab's content panel,
    // so its click handlers stay on the form and forward into JMBanFormTabBans.
    protected UIActionButton                m_EditDurationBtn;
    protected UIActionConfirmInline         m_UnbanBtn;

    // Pending flow state for the offline-ban confirmation chain
    // (OnClick_ManualBan(Selected) -> OnManualBan_GotSteamID -> OpenBanReasonPopup
    // -> OnManualBan_GotReason). Public: JMBanFormTabOffline writes it, and it
    // has to stay readable by the confirmation-callback methods below, which
    // in turn have to stay on the form - JMConfirmation dispatches its named
    // callbacks against whatever object it was Init()'d with, not against
    // whichever widget/tab raised the popup, so moving these off the form
    // would silently break the callback.
    string                        m_PendingSteamID;
    string                        m_PendingPlayerName;

    // Known players (from the last ban-list RPC), needed both by the Offline
    // tab's dropdown and by this form's own OnManualBan_GotSteamID lookup.
    ref array<string>                 m_KnownGuids    = new array<string>();
    ref array<string>                 m_KnownNames    = new array<string>();

    //! protected, not private: sub-mods reach for the module through the form.
    //! Also read directly by JMBanFormTabBans/JMBanFormTabOffline through
    //! their back-reference, which needs public rather than protected.
    JMBanModule                 m_Module;

    static string GetDurationLabel( int idx )
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

    static int GetDurationSeconds( int idx )
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
    //  Toolbar - lives above the tab strip, so its handlers stay here and
    //  forward into whichever tab actually owns the selection/duration state.
    // -------------------------------------------------------------------------

    void SetToolbarEnabled( bool enabled )
    {
        if ( m_EditDurationBtn )
        {
            if ( enabled ) m_EditDurationBtn.Enable();
            else           m_EditDurationBtn.Disable();
        }

        if ( m_UnbanBtn )
        {
            if ( enabled ) m_UnbanBtn.Enable();
            else           m_UnbanBtn.Disable();
        }
    }

    // -------------------------------------------------------------------------
    //  SetModule
    // -------------------------------------------------------------------------

    protected override bool SetModule( JMRenderableModuleBase mdl )
    {
        return Class.CastTo( m_Module, mdl );
    }

    // -------------------------------------------------------------------------
    //  OnCreate
    // -------------------------------------------------------------------------

    override void OnCreate()
    {
        InitWidgetsTop();
        InitWidgetsBottom();
    }

    //! Filter and the actions that apply to the current selection. Archetype B:
    //! a fixed header over a data grid, not a roster over a detail pane - the
    //! ban list is a table, and it wants the whole width.
    protected void InitWidgetsTop()
    {
        Widget top = layoutRoot.FindAnyWidget( "panel_top" );

        JMSearchRow toolbar = UIActionManager.CreateSearchFlexRow( top, "Search name / SteamID...", this, "OnChange_Search", "OnClick_Refresh" );

        m_SearchRow = toolbar.Row;
        m_SearchBar = toolbar.Search;

        // Toolbar: Unban (delete-style icon) + Edit Duration. Both act on the
        // checked rows, so they belong with the filter, above the grid.
        Widget toolbarRow = UIActionManager.CreateWrapSpacer( top, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

        m_UnbanBtn = UIActionManager.CreateDeleteConfirmIcon( toolbarRow, this, "OnClick_Unban" );
        m_UnbanBtn.SetTooltip( "#STR_COT_BANMANAGER_LIFT_THE_SELECTED_BAN_S_IMMEDIATELY" );
        m_UnbanBtn.Disable();

        m_EditDurationBtn = UIActionManager.CreateButton( toolbarRow, "#STR_COT_BANMANAGER_EDIT_DURATION", this, "OnClick_EditDuration" );
        m_EditDurationBtn.SetWidth( 1.0 );
        m_EditDurationBtn.Disable();
        m_EditDurationBtn.SetTooltip( "#STR_COT_BANMANAGER_CHANGE_THE_DURATION_OF_THE_SELECTED" );

        BindPermission( m_UnbanBtn, JMConstants.PERM_BAN_UNBAN );
    }

    protected void InitWidgetsBottom()
    {
        m_TabListPanel    = layoutRoot.FindAnyWidget( "ban_list_panel" );
        m_TabOfflinePanel = layoutRoot.FindAnyWidget( "ban_offline_panel" );


        m_BottomTabStrip = layoutRoot.FindAnyWidget( "panel_bottom_tabs" );
        m_BottomContent  = layoutRoot.FindAnyWidget( "panel_bottom_content" );

        m_Tabs = UIActionManager.CreateTabStrip( m_BottomTabStrip, this, "OnChange_Tab" );

        m_TabIdBans = m_Tabs.AddTab( "Active Bans", JMConstants.Lucide( "ban" ), m_TabListPanel );
        m_TabIdOffline = m_Tabs.AddTab( "Ban Offline Player", JMConstants.Lucide( "user-x" ), m_TabOfflinePanel );

        DeclareTabs( 2 );

        m_Tabs.SetSelection( m_TabIdBans, false );

        InitTabFocus( m_TabIdBans );
    }

    override protected void OnTabCreate( int tab, Widget panel )
    {
        if ( tab == m_TabIdBans )
        {
            m_TabBans = new JMBanFormTabBans( this );
            RegisterTab( m_TabIdBans, m_TabBans );
            m_TabBans.OnCreate( panel );
        }
        else if ( tab == m_TabIdOffline )
        {
            m_TabOffline = new JMBanFormTabOffline( this );
            RegisterTab( m_TabIdOffline, m_TabOffline );
            m_TabOffline.OnCreate( panel );
        }
    }

    protected override COT_ScriptedWidgetEventHandler GetTabStrip()
    {
    	return m_Tabs;
    }

    void OnChange_Tab( UIEvent eid, UIActionBase action )
    {
        if ( eid == UIEvent.CHANGE )
            HandleTabChange();
    }

    override void OnResize( float w, float h )
    {
        super.OnResize( w, h );

        PinBottomPanelGeometry( h - 80 );

        ResizeTabs( w, h );
    }

    // -------------------------------------------------------------------------
    //  OnShow - the ban list is requested by the base OnShow() through
    //  JMBanModule.RequestData(), so the form has nothing of its own to do.
    // -------------------------------------------------------------------------

    override void OnClientPermissionsUpdated()
    {
        super.OnClientPermissionsUpdated();

        if ( m_TabBans )
            m_TabBans.Rebuild( CurrentFilter() );
    }

    override void OnSettingsUpdated()
    {
        super.OnSettingsUpdated();

        if ( m_Module )
            m_Module.RequestBanList();
    }

    protected string CurrentFilter()
    {
        if ( !m_SearchBar )
            return "";

        return m_SearchBar.GetText();
    }

    // -------------------------------------------------------------------------
    //  PopulateBanList - called from module on RPC receive
    // -------------------------------------------------------------------------

    void PopulateBanList( array<ref JMPlayerBan> bans, array<string> playerGuids, array<string> playerNames )
    {
        m_KnownGuids.Clear();
        m_KnownNames.Clear();
        for ( int i = 0; i < playerGuids.Count(); i++ )
        {
            m_KnownGuids.Insert( playerGuids[i] );
            m_KnownNames.Insert( playerNames[i] );
        }

        string currentFilter = CurrentFilter();

        if ( m_TabOffline )
            m_TabOffline.Rebuild( currentFilter );

        if ( m_TabBans )
            m_TabBans.SetBans( bans, currentFilter );
    }

    void OnClick_EditDuration( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        if ( m_TabBans )
            m_TabBans.RequestEditDuration();
    }

    // ConfirmInline (Unban button) already provides the confirm gesture,
    // so we act on CHANGE directly without a popup.
    void OnClick_Unban( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CHANGE )
            return;

        if ( m_TabBans )
            m_TabBans.RequestUnban();
    }

    void OnClick_Refresh( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        if ( m_Module )
            m_Module.RequestBanList();
    }

    // -------------------------------------------------------------------------
    //  Shared search - filters both dropdown and ban list
    // -------------------------------------------------------------------------

    void OnChange_Search( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CHANGE )
            return;

        if ( m_SearchBar )
            m_SearchBar.SetTextPreview( "" );

        string filter = CurrentFilter();

        if ( m_TabOffline )
            m_TabOffline.Rebuild( filter );

        if ( m_TabBans )
            m_TabBans.Rebuild( filter );
    }

    // -------------------------------------------------------------------------
    //  Ban Offline - confirmation-callback chain
    //
    //  These three stay on the form (not on JMBanFormTabOffline) because
    //  JMConfirmation dispatches its named callbacks against the object its
    //  owning window was Init()'d with, not against whatever raised the
    //  popup - moving them to the tab class would silently break the chain.
    //  JMBanFormTabOffline still triggers the chain by calling
    //  CreateConfirmation_Two(...) on this form through its back-reference.
    // -------------------------------------------------------------------------

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

        // NOTE: chaining a second EDIT-type CreateConfirmation_Two from the
        // callback of the first EDIT-type confirmation was silently failing
        // to open the popup (engine bug - the inner modal handler doesn't
        // release the focus lock before the chained CreateConfirmation_Two
        // tries to acquire it again). Workaround: defer the second popup
        // by one CallLater tick so the first modal is fully torn down.
        DeferCall( "OpenBanReasonPopup", 50 );
    }

    protected void OpenBanReasonPopup()
    {
        if ( m_PendingSteamID == "" )
            return;

        PromptInput( "#STR_COT_BANMANAGER_BAN_REASON", "Reason for banning " + m_PendingPlayerName + " (" + m_PendingSteamID + "):", "OnManualBan_GotReason" );
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
