//! "Ban Offline Player" tab of JMBanForm - the known-player dropdown and the
//! manual SteamID entry point. Back-reference to the owning form, same shape
//! as JMPlayerRowWidget.Menu.
class JMBanFormTabOffline: JMFormTab
{
    protected JMBanForm m_Form;
    protected UIActionScroller     m_OfflineScroller;
    protected UIActionDropdownList m_PlayerDropdown;
    protected ref UIActionPlayerPicker m_PlayerPicker;
    protected Widget               m_BanOfflineActionsRow;
    protected ref array<string> m_FilteredGuids = new array<string>();
    protected ref array<string> m_FilteredNames = new array<string>();

    void JMBanFormTabOffline( JMBanForm form )
    {
        m_Form = form;
    }

    override void OnCreate( Widget panel )
    {
        super.OnCreate( panel );

        //! Two cards, one per way of naming the player: picked from the
        //! players the server has seen, or typed in as a SteamID.
        m_OfflineScroller = UIActionManager.CreateScroller( panel );
        Widget content = m_OfflineScroller.GetContentWidget();

        UIActionCard card = UIActionManager.CreateCard( content, "Ban Offline Player" );
        Widget cardBody = card.GetContent();

        // Player dropdown - full width, populated on data arrival
        array<string> emptyList = new array<string>();
        emptyList.Insert( "Loading players..." );
        m_PlayerDropdown = UIActionManager.CreateDropdownBox( cardBody, m_Form.GetLayoutRoot(), "Select player...", emptyList );
        m_Form.AddOverlay( m_PlayerDropdown );

        m_PlayerPicker = UIActionManager.CreatePlayerPicker( this, "OnPlayerPickerSelection" );

        // Action row: Ban Selected, full width under the dropdown it acts on.
        m_BanOfflineActionsRow = UIActionManager.CreateWrapSpacerCompact( cardBody, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
        UIActionButton banSelBtn = UIActionManager.CreateButton( m_BanOfflineActionsRow, "#STR_COT_BANMANAGER_BAN_SELECTED", this, "OnClick_ManualBanSelected" );
        banSelBtn.SetWidth( 1.0 );
        banSelBtn.SetIcon( JMConstants.Lucide( "gavel" ) );
        banSelBtn.SetColor( JMTheme.DANGER_FILL );
        banSelBtn.SetTooltip( "#STR_COT_BANMANAGER_BAN_THE_PLAYER_SELECTED_IN_THE" );

        // Ban by ID - its own card: it does not use the dropdown at all.
        UIActionCard idCard = UIActionManager.CreateCard( content, "#STR_COT_BANMANAGER_BAN_BY_ID" );
        Widget idBody = idCard.GetContent();

        UIActionManager.CreateText( idBody, "#STR_COT_BANMANAGER_BAN_AN_OFFLINE_PLAYER_BY_TYPING" );

        UIActionButton banByIDBtn = UIActionManager.CreateButton( idBody, "#STR_COT_BANMANAGER_BAN_BY_ID", this, "OnClick_ManualBan" );
        banByIDBtn.SetWidth( 1.0 );
        banByIDBtn.SetIcon( JMConstants.Lucide( "user-x" ) );
        banByIDBtn.SetTooltip( "#STR_COT_BANMANAGER_BAN_AN_OFFLINE_PLAYER_BY_TYPING" );

        // The ban itself is executed through JMPlayerModule.Ban, so it is that
        // module's permission that gates it - JMBanModule only registers View
        // and Unban.
        m_Form.BindPermission( banSelBtn,  JMConstants.PERM_PLAYER_BAN );
        m_Form.BindPermission( banByIDBtn, JMConstants.PERM_PLAYER_BAN );

        m_OfflineScroller.UpdateScroller();
    }

    override void OnResize( float w, float h )
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

        JMSearchMatcher matcher = new JMSearchMatcher( filter );

        for ( int i = 0; i < m_Form.m_KnownNames.Count(); i++ )
        {
            if ( matcher.Matches( m_Form.m_KnownNames[i] ) || matcher.Matches( m_Form.m_KnownGuids[i] ) )
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

        m_Form.PromptInput( "#STR_COT_BANMANAGER_BAN_REASON", "Reason for banning " + m_Form.m_PendingPlayerName + " (" + m_Form.m_PendingSteamID + "):", "OnManualBan_GotReason" );
    }

    // -------------------------------------------------------------------------
    //  Ban Offline - manual SteamID entry
    // -------------------------------------------------------------------------

    void OnClick_ManualBan( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        m_Form.PromptInput( "#STR_COT_BANMANAGER_BAN_OFFLINE_PLAYER", "#STR_COT_BANMANAGER_ENTER_THE_STEAM_64_ID_OF", "OnManualBan_GotSteamID" );
    }

    void OnPlayerPickerSelection( UIActionPlayerPicker picker )
    {
        if ( !picker )
            return;

        array<JMPlayerInstance> selected = picker.GetSelectedPlayers();
        if ( selected && selected.Count() > 0 )
        {
            JMPlayerInstance inst = selected.Get( 0 );
            if ( inst )
            {
                m_Form.m_PendingSteamID    = inst.GetGUID();
                m_Form.m_PendingPlayerName = inst.GetName();
            }
        }
    }
}
