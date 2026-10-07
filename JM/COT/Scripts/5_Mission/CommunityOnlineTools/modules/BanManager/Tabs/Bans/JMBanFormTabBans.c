//! "Active Bans" tab of JMBanForm - the ban list, its selection, and the
//! inline duration picker. Holds a back-reference to the owning form (same
//! shape as JMPlayerRowWidget.Menu) so it can reach the form's module and
//! toolbar without either class needing to inherit the other.
//!
//! Laid out as a roster over a detail pane (ban_form.layout):
//!
//!   ban_list_left    one row per ban - player name, expiry
//!   ban_list_detail  the selected ban as a card: who, why, until when,
//!                    by whom; Unban in its title bar, Edit Duration and
//!                    the duration picker under it
class JMBanFormTabBans: JMFormTab
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

    //! DEPRECATED - rows are no longer checkboxes; selection is the list's
    //! own. Kept, always empty, for sub-mods that iterate it.
    protected ref map< string, UIActionCheckbox > m_BanCheckboxes = new map< string, UIActionCheckbox >();

    // Selection tracking - SteamID of the selected ban entry
    protected string m_SelectedBanSteamID;
    protected string m_SelectedBanPlayerName;

    // Cached ban list
    protected autoptr array<ref JMPlayerBan> m_BanList = new array<ref JMPlayerBan>();

    // ---- Roster ----------------------------------------------------------------
    protected Widget              m_ListHost;
    protected UIActionItemList    m_List;

    //! SteamID for every row currently in the list, in row order.
    protected ref TStringArray    m_ListSteamIDs = new TStringArray();

    // ---- Detail ----------------------------------------------------------------
    protected Widget                m_DetailHost;
    protected Widget                m_DetailEmpty;
    protected Widget                m_DetailWrapper;
    protected UIActionCard          m_DetailCard;
    protected UIActionKeyValueList  m_DetailValues;

    void JMBanFormTabBans( JMBanForm form )
    {
        m_Form = form;
    }

    void SetBans( array<ref JMPlayerBan> bans, string filter )
    {
        m_BanList.Clear();
        foreach ( JMPlayerBan ban : bans )
            m_BanList.Insert( ban );

        Rebuild( filter );
    }

    override void OnCreate( Widget panel )
    {
        super.OnCreate( panel );

        m_ListHost   = panel.FindAnyWidget( "ban_list_left" );
        m_DetailHost = panel.FindAnyWidget( "ban_list_detail" );

        // ---- Roster -------------------------------------------------------
        m_List = UIActionManager.CreateItemList( m_ListHost, this, "OnClick_BanRow" );
        m_List.SetEmptyText( "#STR_COT_BANMANAGER_NO_ACTIVE_BANS" );

        // ---- Detail -------------------------------------------------------
        m_Scroller      = UIActionManager.CreateScroller( m_DetailHost );
        m_ContentWidget = m_Scroller.GetContentWidget();

        //! What the pane says while nothing is selected: the count, and how to
        //! get further.
        m_DetailEmpty = UIActionManager.CreateGridSpacer( m_ContentWidget, 1, 1 );
        m_ActiveBansHeader = UIActionManager.CreateText( m_DetailEmpty, "#STR_COT_BANMANAGER_ACTIVE_BANS", "Select a ban on the left" );

        m_DetailWrapper = UIActionManager.CreateGridSpacer( m_ContentWidget, 1, 1 );

        m_DetailCard = UIActionManager.CreateCard( m_DetailWrapper, "" );
        Widget cardBody = m_DetailCard.GetContent();

        m_DetailValues = UIActionManager.CreateKeyValueList( cardBody );

        UIActionManager.CreateDivider( cardBody, JMTheme.DIVIDER_DARK, 1 );

        m_Form.BuildSelectionActions( m_DetailCard, cardBody );

        m_DurationPickerWrapper = UIActionManager.CreateGridSpacer( cardBody, 1, 2 );

        array<string> durationOptions = new array<string>();
        for ( int d = 0; d < JMBanForm.DURATION_COUNT; d++ )
            durationOptions.Insert( JMBanForm.GetDurationLabel( d ) );
        m_DurationSelect = UIActionManager.CreateSelectionBox( m_DurationPickerWrapper, "#STR_COT_BANMANAGER_DURATION", durationOptions );
        m_DurationSelect.SetSelectorWidth( 0.65 );

        Widget durationBtnRow = UIActionManager.CreateGridSpacer( m_DurationPickerWrapper, 1, 2 );
        UIActionButton applyDurBtn = UIActionManager.CreateButton( durationBtnRow, "#STR_COT_GENERIC_APPLY", this, "OnClick_ApplyDuration" );
        applyDurBtn.SetColor( JMTheme.SUCCESS_FILL );
        applyDurBtn.SetTooltip( "#STR_COT_BANMANAGER_SAVE_THE_NEW_BAN_DURATION_FOR" );
        UIActionButton cancelDurBtn = UIActionManager.CreateButton( durationBtnRow, "#STR_COT_GENERIC_CANCEL", this, "OnClick_CancelDuration" );
        cancelDurBtn.SetTooltip( "#STR_COT_BANMANAGER_CANCEL_DURATION_EDITING_WITHOUT_SAVING" );

        m_DurationPickerWrapper.Show( false );
        m_DurationPickerVisible = false;

        //! Kept for sub-mods that add rows to it; the roster replaced the
        //! table it used to hold.
        m_BanListWrapper = UIActionManager.CreateGridSpacer( m_ContentWidget, 1, 1 );

        ShowDetail( null );

        m_Scroller.UpdateScroller();
    }

    override void OnResize( float w, float h )
    {
        //! h is the whole form's height. The roster gets what is left under
        //! the search row and the (possibly wrapped) tab strip, and is told
        //! that height rather than measuring it.
        if ( m_List )
        {
            float listH = h - JMBanForm.TOP_HEIGHT - m_Form.GetPinnedStripHeight( m_Form.GetBottomTabStrip() );
            if ( listH > 0 )
                m_List.SetViewportHeight( listH );
        }

        if ( m_Scroller )
            m_Scroller.UpdateScroller();
    }

    // -------------------------------------------------------------------------
    //  Rebuild
    // -------------------------------------------------------------------------

    void Rebuild( string filter )
    {
        // Arrives from a server response, which does not wait for the Active
        // Bans tab to have been opened.
        if ( !m_List )
            return;

        HideDurationPicker();

        if ( m_ActiveBansHeader )
            m_ActiveBansHeader.SetLabel( "Active Bans (" + m_BanList.Count() + ")" );

        m_ListSteamIDs.Clear();

        array<string> labels = new array<string>();
        array<string> subs   = new array<string>();

        JMSearchMatcher matcher = new JMSearchMatcher( filter );

        foreach ( JMPlayerBan ban : m_BanList )
        {
            if ( !matcher.Matches( ban.PlayerName ) && !matcher.Matches( ban.SteamID ) )
                continue;

            labels.Insert( ban.PlayerName );
            subs.Insert( ban.GetExpiryString() );
            m_ListSteamIDs.Insert( ban.SteamID );
        }

        m_List.SetItems( labels, subs );

        //! Keep the selection across a refresh while the ban is still listed;
        //! a lifted or filtered-out ban drops it.
        int row = m_ListSteamIDs.Find( m_SelectedBanSteamID );
        m_List.SetSelectedIndex( row, false );

        if ( row < 0 )
            ShowDetail( null );
        else
            ShowDetail( FindBan( m_SelectedBanSteamID ) );
    }

    protected JMPlayerBan FindBan( string steamID )
    {
        foreach ( JMPlayerBan ban : m_BanList )
        {
            if ( ban.SteamID == steamID )
                return ban;
        }

        return null;
    }

    //! Fill the detail card with `ban`, or show the placeholder for null.
    protected void ShowDetail( JMPlayerBan ban )
    {
        bool hasBan = false;

        if ( !ban )
        {
            m_SelectedBanSteamID    = "";
            m_SelectedBanPlayerName = "";
        }
        else
        {
            m_SelectedBanSteamID    = ban.SteamID;
            m_SelectedBanPlayerName = ban.PlayerName;
            hasBan = true;
        }

        if ( m_DetailEmpty )
            m_DetailEmpty.Show( !hasBan );

        if ( m_DetailWrapper )
            m_DetailWrapper.Show( hasBan );

        if ( ban && m_DetailCard )
            m_DetailCard.SetLabel( ban.PlayerName );

        if ( ban && m_DetailValues )
        {
            m_DetailValues.SetValue( "#STR_COT_BANMANAGER_PLAYER_STEAMID", ban.SteamID );
            m_DetailValues.SetValue( "#STR_COT_BANMANAGER_REASON",         ban.Message );
            m_DetailValues.SetValue( "#STR_COT_BANMANAGER_EXPIRES",        ban.GetExpiryString() );
            m_DetailValues.SetValue( "#STR_COT_BANMANAGER_BANNED_BY",      ban.IssuedByName );
        }

        UpdateToolbarState();

        if ( m_Scroller )
            m_Scroller.UpdateScroller();
    }

    // -------------------------------------------------------------------------
    //  Roster selection
    // -------------------------------------------------------------------------

    void OnClick_BanRow( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        int row = m_List.GetSelectedIndex();
        if ( row < 0 || row >= m_ListSteamIDs.Count() )
            return;

        string steamID = m_ListSteamIDs[row];
        if ( steamID == m_SelectedBanSteamID )
            return;

        HideDurationPicker();
        ShowDetail( FindBan( steamID ) );
    }

    //! DEPRECATED - rows are no longer checkboxes, so nothing raises this.
    //! Selects the ban carried in the action's data, as a row click would.
    void OnClick_BanRowCheckbox( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;

        JMStringData data;
        if ( !Class.CastTo( data, action.GetData() ) )
            return;

        ShowDetail( FindBan( data.Value ) );
    }

    // -------------------------------------------------------------------------
    //  Toolbar state - the buttons themselves are the form's (built onto the
    //  detail card by JMBanForm.BuildSelectionActions), so this only computes
    //  whether they should be enabled.
    // -------------------------------------------------------------------------

    protected void UpdateToolbarState()
    {
        bool hasSelection = ( m_SelectedBanSteamID != "" );
        bool canUnban      = JMPermissions.Has( JMConstants.PERM_BAN_UNBAN );

        m_Form.SetToolbarEnabled( hasSelection && canUnban );
    }

    // -------------------------------------------------------------------------
    //  Duration picker (inline)
    // -------------------------------------------------------------------------

    //! Called by the form's OnClick_EditDuration forwarder (the button itself
    //! is form-owned, though it now sits on this tab's detail card).
    void RequestEditDuration()
    {
        if ( m_SelectedBanSteamID == "" )
            return;

        ShowDurationPicker();
    }

    protected void ShowDurationPicker()
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

    protected void HideDurationPicker()
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
