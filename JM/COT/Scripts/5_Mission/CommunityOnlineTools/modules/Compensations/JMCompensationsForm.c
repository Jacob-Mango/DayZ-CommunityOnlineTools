class JMCompensationsForm: JMFormBase
{
	private UIActionScroller m_sclr_MainActions;
	private Widget m_ContentWrapper;
	private Widget m_ActionsWrapper;

	protected UIActionDropdown m_SpawnModeDropdown;
	protected ref array< string > m_SpawnModeText =
	{
		"#STR_COT_OBJECT_MODULE_CROSSHAIR",
		"#STR_COT_OBJECT_MODULE_SELECTED_PLAYERS"
	};

	protected UIActionSearchBox m_SearchBox;
	protected string m_SearchFilter = "";

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMCompensationsModule m_Module;
	private ref array< ref JMCompensationEntry > m_Compensations;
	private ref JMCompensationButtonData m_TempData;

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_sclr_MainActions = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
		m_ContentWrapper = m_sclr_MainActions.GetContentWidget();

		// Toolbar row: 32x32 refresh icon on the left, dropdown fills the remaining row width.
		// WrapSpacer, not GridSpacer - GridSpacer(1,2) splits the row into equal
		// halves and gives the refresh button a wide empty cell.
		//
		// Onresize + the dropdown's intrinsic min-width can briefly leave the
		// dropdown stretched wider than the actual content; WrapSpacer with
		// fractional < 1.0 siblings keeps them on one row at panel narrow.
		Widget toolbar = UIActionManager.CreateWrapSpacer( m_ContentWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		UIActionImageButton refreshBtn = UIActionManager.CreateRefreshButton( toolbar, this, "OnClick_Refresh", "#STR_COT_GENERIC_REFRESH" );
		refreshBtn.SetFixedSize( ICON_BUTTON_PX, ICON_BUTTON_PX );

		m_SpawnModeDropdown = UIActionManager.CreateDropdown( toolbar, "", layoutRoot, this, "OnClick_SpawnMode", m_SpawnModeText );
		// Fractional width fills the remaining row space; sum stays well under 1.0
		// so WrapSpacer never wraps the dropdown to a new line at narrow widths.
		m_SpawnModeDropdown.SetWidth( 0.85 );
		m_SpawnModeDropdown.SetSelection( 0, false );
		RegisterOverlay( m_SpawnModeDropdown );

		m_SearchBox = UIActionManager.CreateSearchBox( m_ContentWrapper, this, "OnSearchChanged", "Search by name or Steam ID" );

		m_sclr_MainActions.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_sclr_MainActions )
			m_sclr_MainActions.UpdateScroller();
	}

	override void OnShow()
	{
		m_Module.Load();
	}

	void OnSearchChanged( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_SearchFilter = action.GetText();
		m_SearchFilter.ToLower();

		OnSettingsUpdated();
	}

	override void OnSettingsUpdated()
	{
		m_Compensations = m_Module.GetCompensations();

		if ( m_ActionsWrapper )
			delete m_ActionsWrapper;

		m_ActionsWrapper = UIActionManager.CreateGridSpacer( m_ContentWrapper, 1, 1 );

		if ( !m_Compensations || m_Compensations.Count() < 1 )
		{
			UIActionManager.CreateText( m_ActionsWrapper, "No compensation backups found" );
			m_sclr_MainActions.UpdateScroller();
			return;
		}

		// Sort by steam ID, then by timestamp (newest first)
		m_Compensations.Sort();
		m_Compensations.Invert();

		string currentSteamID = "";
		string playerName;
		Widget playerSection;
		int visibleCount = 0;

		foreach ( JMCompensationEntry entry: m_Compensations )
		{
			// Apply search filter
			if ( m_SearchFilter != "" )
			{
				string steamIDLower = entry.m_SteamID;
				steamIDLower.ToLower();

				playerName = GetPlayerNameFromSteamID( entry.m_SteamID );
				playerName.ToLower();

				if ( steamIDLower.IndexOf( m_SearchFilter ) == -1 && playerName.IndexOf( m_SearchFilter ) == -1 )
					continue;
			}

			visibleCount++;

			// Create new section for each player.
			// Note: the player header (name + SteamID) is now rendered inline on
			// the same row as the FIRST entry's Spawn/Delete buttons (see below).
			// We only need to update the section marker + add vertical spacing
			// between players here.
			if ( entry.m_SteamID != currentSteamID )
			{
				currentSteamID = entry.m_SteamID;

				// Add vertical spacing between players (blank row above the first entry).
				if ( playerSection )
					UIActionManager.CreateText( m_ActionsWrapper, "" );
			}

			// Compensation entry - same row for delete + spawn + (entry name on first entry per player) + timestamp.
			// WrapSpacer so siblings can flow horizontally; fractional widths sum < 0.95 to avoid wrapping
			// (the delete icon is fixed 32px - that takes a real-pixel slice that isn't reflected in the
			// fractional budget, so we leave ~0.05 of headroom for it).
			Widget row = UIActionManager.CreateWrapSpacer( m_ActionsWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

			// Icon-only delete (32x32), no text label. Compact confirm labels
			// ("O" / "X") because the button is narrow - full "Confirm"/"Cancel"
			// would clip on small widths.
			UIActionConfirmInline delbttn = UIActionManager.CreateConfirmInline( row, "", this, "OnClick_Delete" );
			UIActionIconGrid.ApplyDeletePreset( delbttn );
			delbttn.SetButton( "" );
			delbttn.SetFixedSize( ICON_BUTTON_PX, ICON_BUTTON_PX );
			delbttn.CenterIcon( ICON_BUTTON_PX, 16 );
			delbttn.SetConfirmLabel( "O" );
			delbttn.SetCancelLabel( "X" );
			delbttn.SetTooltip( "Delete this compensation entry" );
			delbttn.SetData( new JMCompensationButtonData( entry.m_SteamID, entry.m_Timestamp ) );

			// Spawn button - sits directly after the delete icon. Fractional width
			// (not SetFixedSize) so the WrapSpacer can pack all four siblings on
			// the same row - mixing a fixed-pixel sibling with fractional siblings
			// causes the fractional ones to wrap to the next line.
			UIActionButton spwnbttn = UIActionManager.CreateButton( row, "Spawn", this, "OnClick_Spawn" );
			spwnbttn.SetData( new JMCompensationButtonData( entry.m_SteamID, entry.m_Timestamp ) );
			spwnbttn.SetWidth( 0.18 );

			// Player name appears on the same row as the first entry's Spawn/Delete.
			// Subsequent entries for the same player skip this label.
			UIActionText labelText;
			if ( entry.m_SteamID != currentSteamID )
			{
				playerName = GetPlayerNameFromSteamID( entry.m_SteamID );
				string headerText = playerName + " (" + entry.m_SteamID + ")";
				labelText = UIActionManager.CreateText( row, "", headerText );
				labelText.SetWidth( 0.35 );
				labelText.SetTextVAlign( UIActionVAlign.CENTER );
			}

			// Timestamp text expands to fill the remaining row width. Sum with
			// spawn-button + (optional label) width fractions stays under 0.95
			// so all siblings share the same WrapSpacer row.
			UIActionText tsText = UIActionManager.CreateText( row, "", entry.m_Timestamp );
			if ( labelText )
				tsText.SetWidth( 0.55 );
			else
				tsText.SetWidth( 0.78 );
			tsText.SetTextVAlign( UIActionVAlign.CENTER );
		}

		if ( visibleCount == 0 )
		{
			UIActionManager.CreateText( m_ActionsWrapper, "No compensations match your search" );
		}

		m_sclr_MainActions.UpdateScroller();
	}

	private string GetPlayerNameFromSteamID( string steamID )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();
		foreach ( JMPlayerInstance player: players )
		{
			if ( player.GetSteam64ID() == steamID )
				return player.GetName();
		}
		return steamID;
	}

	void OnClick_Delete( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UIActionConfirmInline button;
		if ( !Class.CastTo( button, action ) )
			return;

		JMCompensationButtonData data;
		if ( !Class.CastTo( data, button.GetData() ) )
			return;

		m_Module.Delete( data.m_SteamID, data.m_Timestamp );
		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallLater( m_Module.Load, 100, false );
	}

	void OnClick_SpawnMode( UIEvent eid, UIActionBase action )
	{
	}

	void OnClick_Spawn( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionButton button;
		if ( !Class.CastTo( button, action ) )
			return;

		JMCompensationButtonData data;
		if ( !Class.CastTo( data, button.GetData() ) )
			return;

		int spawnMode = m_SpawnModeDropdown.GetSelection();

		// Guard: if the dropdown is at -1 (not yet selected) or the cached value
		// is stale, fall back to Crosshair mode so the button always produces
		// visible feedback instead of silently no-oping.
		if ( spawnMode < 0 )
			spawnMode = 0;

		switch ( spawnMode )
		{
		case 0: // Crosshair
			vector position = GetCursorPos();
			m_Module.SpawnCursor( data.m_SteamID, data.m_Timestamp, position );
			break;
		case 1: // Selected Players
			array< string > selectedPlayers = JM_GetSelected().GetPlayersOrSelf();
			if ( selectedPlayers.Count() == 0 )
			{
				CreateConfirmation_One( JMConfirmationType.INFO, "No players selected", "Please select at least one player from the player list.", "#STR_COT_GENERIC_OK", "" );
				return;
			}

			// Build confirmation message with player names
			string playerNames = "";
			array< JMPlayerInstance > allPlayers = GetPermissionsManager().GetPlayers();
			foreach ( string guid: selectedPlayers )
			{
				foreach ( JMPlayerInstance player: allPlayers )
				{
					if ( player.GetGUID() == guid )
					{
						if ( playerNames != "" )
							playerNames += ", ";
						playerNames += player.GetName();
						break;
					}
				}
			}

			// Fall back to a comma-separated GUID list if we couldn't resolve any
			// names (player list may not be populated yet on first Spawn click).
			if ( playerNames == "" )
			{
				for ( int si = 0; si < selectedPlayers.Count(); si++ )
				{
					if ( playerNames != "" )
						playerNames += ", ";
					playerNames += selectedPlayers[si];
				}
			}

			m_TempData = data;
			string confirmMsg = string.Format( "Spawn compensation for %1 (%2) on:\n%3", data.m_SteamID, data.m_Timestamp, playerNames );
			CreateConfirmation_Two( JMConfirmationType.INFO, "Confirm Spawn", confirmMsg, "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_YES", "OnConfirmation_SpawnPlayers" );
			break;
		default:
			// Unknown mode (shouldn't happen - the dropdown only has 2 entries).
			// Surface it instead of silently doing nothing.
			CreateConfirmation_One( JMConfirmationType.INFO, "Unknown spawn mode", "Selected spawn mode is not handled. Re-select the mode and try again.", "#STR_COT_GENERIC_OK", "" );
			break;
		}
	}

	void OnConfirmation_SpawnPlayers( JMConfirmation confirmation )
	{
		if ( !m_TempData )
			return;

		array< string > selectedPlayers = JM_GetSelected().GetPlayersOrSelf();
		m_Module.SpawnPlayers( m_TempData.m_SteamID, m_TempData.m_Timestamp, selectedPlayers );
	}

	void OnClick_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionButton btn;
		if ( Class.CastTo( btn, action ) )
			btn.TriggerSpin( 2 );

		m_Module.Load();
	}
}
