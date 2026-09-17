//! "Active Bans" tab of JMBanForm - the ban list, its checkbox selection, and
//! the inline duration picker. Holds a back-reference to the owning form (same
//! shape as JMPlayerRowWidget.Menu) so it can reach the form's module and
//! toolbar without either class needing to inherit the other.
class JMBanFormTabBans
{
    protected JMBanForm m_Form;

    protected UIActionScroller m_Scroller;
    protected Widget           m_ContentWidget;
    protected UIActionText     m_ActiveBansHeader;

    // Duration picker panel (shown inline when Edit Duration is clicked)
    protected Widget            m_DurationPickerWrapper;
    protected UIActionSelectBox m_DurationSelect;
    protected bool              m_DurationPickerVisible;

    protected Widget m_BanListWrapper;

    // Checkboxes mapped by SteamID so we can clear them on re-selection
    protected ref map< string, UIActionCheckbox > m_BanCheckboxes = new map< string, UIActionCheckbox >();

    // Selection tracking - SteamID of the checked ban entry
    protected string m_SelectedBanSteamID;
    protected string m_SelectedBanPlayerName;

    // Cached ban list
    protected autoptr array<ref JMPlayerBan> m_BanList = new array<ref JMPlayerBan>();

    void JMBanFormTabBans( JMBanForm form )
    {
        m_Form = form;
    }

    void Build( Widget parentPanel )
    {
        m_Scroller      = UIActionManager.CreateScroller( parentPanel );
        m_ContentWidget = m_Scroller.GetContentWidget();

        m_ActiveBansHeader = UIActionManager.CreateText( m_ContentWidget, "Active Bans" );

        m_DurationPickerWrapper = UIActionManager.CreateGridSpacer( m_ContentWidget, 1, 2 );

        array<string> durationOptions = new array<string>();
        for ( int d = 0; d < JMBanForm.DURATION_COUNT; d++ )
            durationOptions.Insert( JMBanForm.GetDurationLabel( d ) );
        m_DurationSelect = UIActionManager.CreateSelectionBox( m_DurationPickerWrapper, "Duration:", durationOptions );
        m_DurationSelect.SetSelectorWidth( 0.65 );

        Widget durationBtnRow = UIActionManager.CreateGridSpacer( m_DurationPickerWrapper, 1, 2 );
        UIActionButton applyDurBtn = UIActionManager.CreateButton( durationBtnRow, "Apply", this, "OnClick_ApplyDuration" );
        applyDurBtn.SetColor( JMTheme.SUCCESS_FILL );
        applyDurBtn.SetTooltip( "Save the new ban duration for the selected entries" );
        UIActionButton cancelDurBtn = UIActionManager.CreateButton( durationBtnRow, "Cancel", this, "OnClick_CancelDuration" );
        cancelDurBtn.SetTooltip( "Cancel duration editing without saving" );

        m_DurationPickerWrapper.Show( false );
        m_DurationPickerVisible = false;

        UIActionManager.CreatePanel( m_ContentWidget, JMTheme.DIVIDER_LIGHT, 1 );

        // ---- Ban list (populated dynamically) ------------------------------
        m_BanListWrapper = UIActionManager.CreateGridSpacer( m_ContentWidget, 1, 1 );
        UIActionManager.CreateText( m_BanListWrapper, "Loading..." );

        m_Scroller.UpdateScroller();
    }

    void OnResize()
    {
        if ( m_Scroller )
            m_Scroller.UpdateScroller();
    }

    void SetBans( array<ref JMPlayerBan> bans, string filter )
    {
        m_BanList.Clear();
        foreach ( JMPlayerBan ban : bans )
            m_BanList.Insert( ban );

        Rebuild( filter );
    }

    // -------------------------------------------------------------------------
    //  Rebuild
    // -------------------------------------------------------------------------

    void Rebuild( string filter )
    {
        // Arrives from a server response, which does not wait for the Active
        // Bans tab to have been opened.
        if ( !m_ContentWidget )
            return;

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

        filter.ToLower();

        if ( m_ActiveBansHeader )
            m_ActiveBansHeader.SetLabel( "Active Bans (" + m_BanList.Count() + ")" );

        bool canUnban = JMPermissions.Has( JMConstants.PERM_BAN_UNBAN );

        // Column headers: checkbox col + 4 data cols
        Widget header = UIActionManager.CreateGridSpacer( m_BanListWrapper, 1, 5 );
        UIActionManager.CreateText( header, ""                 );  // checkbox column
        UIActionManager.CreateText( header, "Player / SteamID" );
        UIActionManager.CreateText( header, "Reason"           );
        UIActionManager.CreateText( header, "Expires"          );
        UIActionManager.CreateText( header, "Banned By"        );

        UIActionManager.CreatePanel( m_BanListWrapper, JMTheme.DIVIDER_LIGHT, 1 );

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

            // Checkbox - only rendered if user has unban permission
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
    //  Ban row checkbox - only one can be checked at a time
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
            // Unchecked - clear selection
            m_SelectedBanSteamID    = "";
            m_SelectedBanPlayerName = "";
            HideDurationPicker();
        }

        UpdateToolbarState();
    }

    // -------------------------------------------------------------------------
    //  Toolbar state - the buttons themselves live on the form, above the tab
    //  strip, so this only computes whether they should be enabled.
    // -------------------------------------------------------------------------

    private void UpdateToolbarState()
    {
        bool hasSelection = ( m_SelectedBanSteamID != "" );
        bool canUnban      = JMPermissions.Has( JMConstants.PERM_BAN_UNBAN );

        m_Form.SetToolbarEnabled( hasSelection && canUnban );
    }

    // -------------------------------------------------------------------------
    //  Duration picker (inline)
    // -------------------------------------------------------------------------

    //! Called by the form's OnClick_EditDuration forwarder (the button itself
    //! is form-level chrome above the tab strip).
    void RequestEditDuration()
    {
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

        if ( !m_Form.m_Module || m_SelectedBanSteamID == "" || !m_DurationSelect )
            return;

        int idx = m_DurationSelect.GetSelection();
        if ( idx < 0 || idx >= JMBanForm.DURATION_COUNT )
            idx = 0;

        m_Form.m_Module.EditBanDuration( m_SelectedBanSteamID, JMBanForm.GetDurationSeconds( idx ) );

        COTCreateLocalAdminNotification( new StringLocaliser( "Updated ban duration for: " + m_SelectedBanPlayerName + " - " + JMBanForm.GetDurationLabel( idx ) ) );

        HideDurationPicker();
        m_Form.m_Module.RequestBanList();
    }

    void OnClick_CancelDuration( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        HideDurationPicker();
    }

    // -------------------------------------------------------------------------
    //  Unban - called by the form's OnClick_Unban forwarder.
    // -------------------------------------------------------------------------

    void RequestUnban()
    {
        if ( m_SelectedBanSteamID == "" || !m_Form.m_Module )
            return;

        m_Form.m_Module.Unban( m_SelectedBanSteamID );
    }
}
