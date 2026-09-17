//! "Ban Offline Player" tab of JMBanForm - the known-player dropdown and the
//! manual SteamID entry point. Back-reference to the owning form, same shape
//! as JMPlayerRowWidget.Menu.
class JMBanFormTabOffline
{
    protected JMBanForm m_Form;

    protected UIActionScroller     m_OfflineScroller;
    protected UIActionDropdownList m_PlayerDropdown;
    protected Widget               m_BanOfflineActionsRow;

    protected ref array<string> m_FilteredGuids = new array<string>();
    protected ref array<string> m_FilteredNames = new array<string>();

    void JMBanFormTabOffline( JMBanForm form )
    {
        m_Form = form;
    }

    void Build( Widget parentPanel )
    {
        m_OfflineScroller = UIActionManager.CreateScroller( parentPanel );
        Widget offlineContent = m_OfflineScroller.GetContentWidget();

        UIActionCard card = UIActionManager.CreateCard( offlineContent, "Ban Offline Player" );
        Widget cardBody = card.GetContent();

        // Player dropdown - full width, populated on data arrival
        array<string> emptyList = new array<string>();
        emptyList.Insert( "Loading players..." );
        m_PlayerDropdown = UIActionManager.CreateDropdownBox( cardBody, m_Form.GetLayoutRoot(), "Select player...", emptyList );
        m_Form.RegisterOverlay( m_PlayerDropdown );

        // Action row: Ban Selected + Ban by ID
        // Fractional widths must sum < 1.0 in WrapSpacer or the second child
        // wraps. Use 0.71+0.28 = 0.99 to preserve the 72/28 visual proportion.
        m_BanOfflineActionsRow = UIActionManager.CreateWrapSpacer( cardBody, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
        UIActionButton banSelBtn = UIActionManager.CreateButton( m_BanOfflineActionsRow, "Ban Selected", this, "OnClick_ManualBanSelected" );
        banSelBtn.SetWidth( 0.71 );
        banSelBtn.SetColor( JMTheme.DANGER_FILL );
        banSelBtn.SetTooltip( "Ban the player selected in the dropdown above" );

        UIActionButton banByIDBtn = UIActionManager.CreateButton( m_BanOfflineActionsRow, "Ban by ID", this, "OnClick_ManualBan" );
        banByIDBtn.SetWidth( 0.28 );
        banByIDBtn.SetTooltip( "Ban an offline player by typing their SteamID or GUID" );

        // The ban itself is executed through JMPlayerModule.Ban, so it is that
        // module's permission that gates it - JMBanModule only registers View
        // and Unban.
        m_Form.RegisterPermission( banSelBtn,  JMConstants.PERM_PLAYER_BAN );
        m_Form.RegisterPermission( banByIDBtn, JMConstants.PERM_PLAYER_BAN );

        m_OfflineScroller.UpdateScroller();
    }

    void OnResize()
    {
        if ( m_OfflineScroller )
            m_OfflineScroller.UpdateScroller();
    }

    // -------------------------------------------------------------------------
    //  Rebuild - dropdown of known players, filtered by name/GUID
    // -------------------------------------------------------------------------

    void Rebuild( string filter )
    {
        if ( !m_PlayerDropdown )
            return;

        m_FilteredGuids.Clear();
        m_FilteredNames.Clear();

        string filterLow = filter;
        filterLow.ToLower();

        for ( int i = 0; i < m_Form.m_KnownNames.Count(); i++ )
        {
            string nameLow = m_Form.m_KnownNames[i];
            nameLow.ToLower();
            string guidLow = m_Form.m_KnownGuids[i];
            guidLow.ToLower();

            if ( filter == "" || nameLow.IndexOf( filterLow ) != -1 || guidLow.IndexOf( filterLow ) != -1 )
            {
                m_FilteredGuids.Insert( m_Form.m_KnownGuids[i] );
                m_FilteredNames.Insert( m_Form.m_KnownNames[i] );
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
    //  Ban Offline - from dropdown selection
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
                m_Form.m_PendingSteamID    = m_FilteredGuids[i];
                m_Form.m_PendingPlayerName = m_FilteredNames[i];
                break;
            }
        }

        if ( m_Form.m_PendingSteamID == "" )
            return;

        m_Form.CreateConfirmation_Two( JMConfirmationType.EDIT, "Ban Reason", "Reason for banning " + m_Form.m_PendingPlayerName + " (" + m_Form.m_PendingSteamID + "):", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "OnManualBan_GotReason" );
    }

    // -------------------------------------------------------------------------
    //  Ban Offline - manual SteamID entry
    // -------------------------------------------------------------------------

    void OnClick_ManualBan( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        m_Form.CreateConfirmation_Two( JMConfirmationType.EDIT, "Ban Offline Player", "Enter the Steam 64 ID of the player to ban:", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "OnManualBan_GotSteamID" );
    }
}
