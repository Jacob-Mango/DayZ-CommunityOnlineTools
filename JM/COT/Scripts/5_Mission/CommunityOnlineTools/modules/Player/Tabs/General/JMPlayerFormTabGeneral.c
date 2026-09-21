//! "General" tab of JMPlayerForm - identity, vitals (stats) and active statuses of
//! the selected player. Back-reference to the owning form, same shape as
//! JMPlayerRowWidget.Menu.
//!
//! Extension points stay on the form: InitActionWidgetsIdentity(),
//! InitActionWidgetsStats(), RefreshStats() and Click_ApplyStats() are virtual
//! hooks there (modded classes override them and call super), and forward to
//! this class. The stats sliders are mirrored on the form through
//! JMPlayerForm.SetLegacyStatWidgets for the same reason.
class JMPlayerFormTabGeneral: JMFormTab
{
	protected JMPlayerForm m_Form;
	UIActionFeedbackButton m_GUID;
	UIActionFeedbackButton m_Name;
	UIActionFeedbackButton m_Steam64ID;
	UIActionButton m_SteamProfile;
#ifdef GAMELABS
	UIActionFeedbackButton m_CFToolsID;
#endif
#ifdef GAMELABS
	UIActionButton m_CFProfile;
#endif

	//! Read-only mirror of the four JMPlayerVariables flags worth seeing at a
	//! glance. The checkboxes that set them live on the Actions tab; these say
	//! what is on without making the admin go and look.
	Widget m_FlagStrip;
	UIActionBadge m_BadgeGodMode;
	UIActionBadge m_BadgeFrozen;
	UIActionBadge m_BadgeInvisible;
	UIActionBadge m_BadgeUnconscious;
	UIActionText  m_IdentityRole;

	//! Jumps to the Role Manager with this player already selected. Reading the
	//! role here and having to go and FIND the same player in another window to
	//! change it was the one identity row with no way to act on what it said.
	UIActionImageButton m_IdentityRoleEdit;
	UIActionStagedIcon m_BadgeStatusDead;
	UIActionStagedIcon m_BadgeStatusUnconscious;
	UIActionStagedIcon m_BadgeStatusBrokenLeg;
	UIActionStagedIcon m_BadgeStatusBleeding;
	UIActionStagedIcon m_BadgeStatusSick;

	//! The whole Active Statuses card. Hidden outright while the player is in
	//! none of the five states - an empty card is a heading over a blank strip,
	//! which reads as a panel that failed to load rather than as "nothing wrong".
	Widget m_StatusCardRoot;

	//! Right-click menu for one Active Statuses glyph, and the guid it was
	//! opened against. Separate from m_PlayerMenu: that one belongs to a list
	//! row and can be open over a different player than the card is showing.
	UIActionContextMenu m_StatusMenu;
	string m_StatusMenuGUID;

	//! protected, not private: DayZ-Expansion's `modded class JMPlayerForm`
	//! (DayZExpansion_AI, DayZExpansion_Hardline) re-sorts these widgets via
	//! GetLayoutRoot().SetSort() when injecting its faction/reputation rows.
	//! A modded class cannot touch a private member of the class it mods.
	UIActionImageButton m_ApplyStats;
	UIActionSlider m_Health;
	UIActionSlider m_Blood;
	UIActionSlider m_Energy;
	UIActionSlider m_Water;
	UIActionSlider m_Shock;

	//! protected, not private: DayZ-Expansion's `modded class JMPlayerForm` uses
	//! m_Stamina.GetLayoutRoot().GetParent() as the anchor to inject its own rows.
	UIActionSlider m_Stamina;
	UIActionSlider m_HeatComfort;
	UIActionSlider m_HeatBuffer;

	//! One row per settable vital: its JMPlayerStat, its slider above, and whether the admin has
	//! dragged it since the last refresh. Rows drive the repaint, the colour and Apply, so a stat
	//! costs no per-stat code here. HeatComfort is read-only and stays outside them.
	protected ref array< ref JMPlayerStatRow > m_StatRows = new array< ref JMPlayerStatRow >;

	//! protected, not private: DayZ-Expansion's `modded class JMPlayerForm`
	//! (DayZExpansion_AI, DayZExpansion_Hardline, DayZExpansion_PersonalStorage)
	//! does, inside its InitActionWidgetsStats override:
	//!
	//!     m_ApplyStats.GetLayoutRoot().SetSort(sort + 1);
	//!     m_RefreshStats.GetLayoutRoot().SetSort(sort + 2);
	//!
	//! COT dropped this button at some point and never kept the name, so all
	//! three of those addons have failed to compile against COT ever since.
	//!
	//! It is a real button sitting beside Apply, NOT an alias of the Position
	//! tab's refresh icon: SetSort reorders a widget among ITS OWN siblings, so
	//! pointing this at the position toolbar would have Expansion silently
	//! shuffling the coordinate icons instead. Both now live in the variables
	//! card's title bar rather than in the slider grid, which makes Expansion's
	//! SetSort a harmless no-op - the pair is already last in reading order -
	//! and leaves the member names it compiles against intact.
	UIActionImageButton m_RefreshStats;
	static const float IDENTITY_SHORT_WIDTH  = 0.40;
	static const float IDENTITY_BUTTON_LEFT  = 0.64;
	static const float IDENTITY_BUTTON_WIDTH = 0.34;

	//! The role row gives up its last few percent to the edit button beside it.
	static const float IDENTITY_ROLE_VALUE_WIDTH = 0.92;
	static const float IDENTITY_ROLE_EDIT_LEFT   = 0.93;

	//! The edit glyph is SHORTER than the row it sits in - every other identity
	//! value fills its row, so only this one has slack to distribute. Its
	//! position is a fraction of the parent, so the pixel gap above and below is
	//! converted here rather than hardcoded; left at the layout default of 0 the
	//! button hung two pixels above the text it belongs to.
	//! ( IDENTITY_ROW_HEIGHT 32 - HEADER_ACTION_PX 28 ) / 2, over the row height.
	//! Written out rather than computed: these are another class's static consts
	//! and a const initialiser is not the place to depend on their evaluation
	//! order. Revisit if either constant moves.
	static const float IDENTITY_ROLE_EDIT_TOP = 0.0625;

	//! Shock value the unconscious chip wakes a player with. The slider it
	//! shares its permission with runs 0..100, and full is the only value that
	//! is certain to clear the engine own consciousness threshold.
	static const float SHOCK_CONSCIOUS = 100;

	//! Identity row geometry, in the row's own fractions. The value column stops
	//! short of the right edge so the card's inner padding stays visible down the
	//! whole column, and the short value + its profile button split what is left
	//! of the row after the label.
	static const float IDENTITY_LABEL_WIDTH  = 0.22;
	static const float IDENTITY_VALUE_WIDTH  = 0.76;

	//! Direct children this tab's builder adds to its panel, with headroom. Over-
	//! provisioning is free under Size-To-Content-V; running short silently clips the
	//! last rows with no error anywhere. The form sizes the panel from this.
	static const int PANEL_ROWS = 8;

	void JMPlayerFormTabGeneral( JMPlayerForm form )
	{
		m_Form = form;
	}

	//! An active flag reads as a warning, not as a success: every one of these
	//! is a state the player should not normally be in.
	void SetFlagBadge( UIActionBadge badge, bool active )
	{
		if ( active )
			badge.SetStatus( "#STR_COT_PLAYER_MODULE_BADGE_ON", JMTheme.DANGER );
		else
			badge.SetStatus( "#STR_COT_PLAYER_MODULE_BADGE_OFF", JMTheme.SUCCESS_DIM );
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		//! Through the form so a modded class's override of the hook wraps the build.
		m_Form.CallInitActionWidgetsIdentity( panel );
		m_Form.CallInitActionWidgetsStats( panel );

		//! Last card on the tab. It is the one card here that is empty
		//! most of the time - a healthy player has no statuses - and a
		//! card that usually says nothing should not sit above the
		//! identity and the stats an admin opened the tab for.
		InitActionWidgetsStatuses( panel );
	}

	//! Repaint identity badges and the vitals sliders from the selected player.
	void Refresh( bool force = false )
	{
		RefreshIdentityBadges();

		foreach ( JMPlayerStatRow row : m_StatRows )
		{
			if ( force )
				row.Dirty = false;

			row.Refresh( m_Form.m_SelectedInstance );
		}

		if ( m_HeatComfort )
		{
			m_HeatComfort.SetCurrent( m_Form.m_SelectedInstance.GetHeatComfort() );
			m_HeatComfort.SetColor( HeatComfortColor( m_HeatComfort.GetCurrent() ) );
			m_HeatComfort.SetAlpha( 1.0 );
		}
	}

	//! Colour of the read-only heat comfort slider: blue below neutral, red above.
	static int HeatComfortColor( float comfort )
	{
		// COLD
		if ( comfort <= PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_CRITICAL )
			return Colors.TEMPERATURE_COLD_LVL_FOUR;

		if ( comfort <= PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_WARNING )
			return Colors.TEMPERATURE_COLD_LVL_TWO;

		if ( comfort <= PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_EMPTY )
			return Colors.TEMPERATURE_COLD_LVL_ONE;

		// NEUTRAL
		if ( comfort < PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_WARNING )
			return Colors.TEMPERATURE_NEUTAL;

		// HOT
		if ( comfort >= PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_CRITICAL )
			return Colors.TEMPERATURE_HOT_LVL_FOUR;

		return Colors.TEMPERATURE_HOT_LVL_TWO;
	}

	//! Permission-gate the controls this tab owns.
	void BindPermissions()
	{
		foreach ( JMPlayerStatRow row : m_StatRows )
			m_Form.BindPermission( row.Slider, row.Stat.GetPermission() );

		// The identity chips are those same three toggles plus shock, so they
		// answer to the same permissions - an admin who cannot set a flag must
		// not be handed a second way to try.
		m_Form.BindPermission( m_BadgeGodMode,          JMConstants.PERM_PLAYER_GODMODE               );
		m_Form.BindPermission( m_BadgeFrozen,           JMConstants.PERM_PLAYER_FREEZE                );
		m_Form.BindPermission( m_BadgeInvisible,        JMConstants.PERM_PLAYER_INVISIBILITY          );
		m_Form.BindPermission( m_BadgeUnconscious,      JMConstants.PERM_PLAYER_SET_SHOCK             );

		//! Roles are edited in the Role Manager, so the jump is only offered to
		//! an admin who can actually open it.
		m_Form.BindPermission( m_IdentityRoleEdit,      JMConstants.PERM_ROLES_VIEW                   );
	}

	//! Fill the identity rows for the player now selected.
	void ShowInstance( JMPlayerInstance instance )
	{
		ShowIdentityWidgets();
		m_GUID.SetButton( instance.GetGUID() );
		m_Name.SetButton( instance.GetName() );
		m_Steam64ID.SetButton( instance.GetSteam64ID() );

		if ( m_IdentityRole )
			m_IdentityRole.SetText( m_Form.RoleDisplayName( m_Form.GetPrimaryRole( instance ) ) );

		#ifdef GAMELABS
		m_CFToolsID.SetButton(instance.PlayerObject.GetUpstreamIdentity());
		#endif
	}

	//! Identity card: a section header, a strip of read-only state badges, then
	//! one labelled row per identifier.
	//!
	//! The value of each identifier is the CAPTION of its copy button - both
	//! UpdateUI (SetButton) and the Click_Copy* handlers (GetButton) go through
	//! that, so the button is the value store as well as the affordance. Do not
	//! "clean this up" into a separate text widget without moving all six call
	//! sites with it.
	Widget InitActionWidgetsIdentity( Widget actionsParent )
	{
		// header + 2 badge rows + name + guid + steam + role [+ cftools] + divider
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 10, 1 );

		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_IDENTITY" );
		Widget section0 = section0Card.GetContent();
		// Two rows of two, not one row of four. UIActionBadge reserves a fixed
		// 80px pill on the right of whatever cell it lands in, so four cells
		// across this panel leave about forty pixels for the label and every
		// flag name is cut to four characters. Two per row gives the label the
		// room it needs, and this tab has vertical space to spare.
		Widget flagRowTop = UIActionManager.CreateRow( section0, JMFormBase.IDENTITY_ROW_HEIGHT );
		m_FlagStrip = UIActionManager.CreateGridSpacer( flagRowTop, 1, 2 );
		m_BadgeGodMode     = UIActionManager.CreateBadge( m_FlagStrip, "#STR_COT_PLAYER_MODULE_BADGE_GODMODE", "", JMTheme.SUCCESS_DIM );
		m_BadgeFrozen      = UIActionManager.CreateBadge( m_FlagStrip, "#STR_COT_PLAYER_MODULE_BADGE_FROZEN", "", JMTheme.SUCCESS_DIM );

		Widget flagRowBottom = UIActionManager.CreateRow( section0, JMFormBase.IDENTITY_ROW_HEIGHT );
		Widget flagStripBottom = UIActionManager.CreateGridSpacer( flagRowBottom, 1, 2 );
		m_BadgeInvisible   = UIActionManager.CreateBadge( flagStripBottom, "#STR_COT_PLAYER_MODULE_BADGE_INVISIBLE", "", JMTheme.SUCCESS_DIM );
		m_BadgeUnconscious = UIActionManager.CreateBadge( flagStripBottom, "#STR_COT_PLAYER_MODULE_BADGE_UNCONSCIOUS", "", JMTheme.SUCCESS_DIM );

		// The four flags are not just a readout. An admin who has just read
		// "GODMODE ON" is usually about to turn it off, and making them cross to
		// the Actions tab for a checkbox that sets what they are already looking
		// at is the long way round. The chips toggle it in place; the checkboxes
		// stay, and both drive the same module call.
		MakeFlagBadgeClickable( m_BadgeGodMode,     "Click_BadgeGodMode",     "#STR_COT_PLAYER_MODULE_TT_GODMODE"   );
		MakeFlagBadgeClickable( m_BadgeFrozen,      "Click_BadgeFrozen",      "#STR_COT_PLAYER_MODULE_TT_FREEZE"    );
		MakeFlagBadgeClickable( m_BadgeInvisible,   "Click_BadgeInvisible",   "#STR_COT_PLAYER_MODULE_TT_INVISIBLE" );
		MakeFlagBadgeClickable( m_BadgeUnconscious, "Click_BadgeUnconscious", ""                                    );

		// One row = ONE fixed-height panel holding the label and the value
		// button, placed by fraction. NOT a nested grid per row: a grid row whose
		// height is content-driven, holding a panel whose height is also
		// content-driven, resolves circularly and the rows blow out to hundreds
		// of pixels each - which is what put the GUID row a full screen below the
		// Name row and gave the tab a scrollbar with nothing in it.
		Widget rowName = UIActionManager.CreateRow( section0, JMFormBase.IDENTITY_ROW_HEIGHT );
		UIActionManager.CreateText( rowName, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_NAME", "" );
		m_Name = UIActionManager.CreateFeedbackButton( rowName, "", "#STR_COT_COPIED", JMConstants.ICON_CHECK_MARK, this, "Click_CopyPlayerName" );
		m_Name.SetWidth( IDENTITY_VALUE_WIDTH );
		m_Name.SetPosition( IDENTITY_LABEL_WIDTH );
		m_Name.SetIcon( JMConstants.ICON_STACK );
		m_Name.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_COPY_NAME" );

		Widget rowGuid = UIActionManager.CreateRow( section0, JMFormBase.IDENTITY_ROW_HEIGHT );
		UIActionManager.CreateText( rowGuid, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_GUID", "" );
		m_GUID = UIActionManager.CreateFeedbackButton( rowGuid, "", "#STR_COT_COPIED", JMConstants.ICON_CHECK_MARK, this, "Click_CopyPlayerGUID" );
		m_GUID.SetWidth( IDENTITY_VALUE_WIDTH );
		m_GUID.SetPosition( IDENTITY_LABEL_WIDTH );
		m_GUID.SetIcon( JMConstants.ICON_STACK );
		m_GUID.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_COPY_GUID" );

		Widget rowSteam = UIActionManager.CreateRow( section0, JMFormBase.IDENTITY_ROW_HEIGHT );
		UIActionManager.CreateText( rowSteam, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_STEAMID", "" );
		m_Steam64ID = UIActionManager.CreateFeedbackButton( rowSteam, "", "#STR_COT_COPIED", JMConstants.ICON_CHECK_MARK, this, "Click_CopyPlayerSteam64ID" );
		m_Steam64ID.SetWidth( IDENTITY_SHORT_WIDTH );
		m_Steam64ID.SetPosition( IDENTITY_LABEL_WIDTH );
		m_Steam64ID.SetIcon( JMConstants.ICON_STACK );
		m_Steam64ID.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_COPY_STEAMID" );
		m_SteamProfile = UIActionManager.CreateButton( rowSteam, "#STR_COT_PLAYER_MODULE_OPEN_PROFILE_WEB", this, "Click_OpenPlayerSteamProfile" );
		m_SteamProfile.SetWidth( IDENTITY_BUTTON_WIDTH );
		m_SteamProfile.SetPosition( IDENTITY_BUTTON_LEFT );
		m_SteamProfile.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_OPEN_STEAM_PROFILE" );

		// Which role bucket the player list filed this player under. It is the
		// one piece of identity the admin cannot read off the card otherwise,
		// and the General tab is exactly where "who is this" belongs.
		// A row, not a bare CreateText, because the edit button has to sit on
		// the same line as the value: the label/value pair stops short of the
		// right edge and the button takes what is left.
		Widget rowRole = UIActionManager.CreateRow( section0, JMFormBase.IDENTITY_ROW_HEIGHT );
		m_IdentityRole = UIActionManager.CreateText( rowRole, "#STR_COT_PLAYER_MODULE_IDENTITY_ROLE", "" );
		m_IdentityRole.SetWidth( IDENTITY_ROLE_VALUE_WIDTH );

		m_IdentityRoleEdit = UIActionManager.CreateEditButton( rowRole, this, "Click_EditPlayerRoles", "#STR_COT_PLAYER_MODULE_TT_EDIT_ROLES" );
		m_IdentityRoleEdit.SetFixedSize( JMFormBase.HEADER_ACTION_PX, JMFormBase.HEADER_ACTION_PX );
		m_IdentityRoleEdit.SetPosition( IDENTITY_ROLE_EDIT_LEFT, IDENTITY_ROLE_EDIT_TOP );

		#ifdef GAMELABS
		Widget rowCF = UIActionManager.CreateRow( section0, JMFormBase.IDENTITY_ROW_HEIGHT );
		UIActionManager.CreateText( rowCF, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_CFTOOLS", "" );
		m_CFToolsID = UIActionManager.CreateFeedbackButton( rowCF, "", "#STR_COT_COPIED", JMConstants.ICON_CHECK_MARK, this, "Click_CopyCFToolsID" );
		m_CFToolsID.SetWidth( IDENTITY_SHORT_WIDTH );
		m_CFToolsID.SetPosition( IDENTITY_LABEL_WIDTH );
		m_CFToolsID.SetIcon( JMConstants.ICON_STACK );
		m_CFProfile = UIActionManager.CreateButton( rowCF, "#STR_COT_PLAYER_MODULE_OPEN_PROFILE_WEB", this, "Click_OpenPlayerCFProfile" );
		m_CFProfile.SetWidth( IDENTITY_BUTTON_WIDTH );
		m_CFProfile.SetPosition( IDENTITY_BUTTON_LEFT );
		#endif

		ShowIdentityWidgets();

		return parent;
	}

	//! Glyphs, not chips. Each status is one bit of information - it is on or it
	//! is not - and a labelled pill spent a whole cell saying a word the icon
	//! says at a glance. The same glyphs the player list uses, so a row badge
	//! and this card read as the same vocabulary; the word each one stood for
	//! survives as its tooltip.
	//!
	//! Only the bad states are here. "Healthy" was a sixth glyph that meant
	//! "none of the other five", which is what an empty card already says, and
	//! it was the one entry with no action behind a right-click.
	Widget InitActionWidgetsStatuses( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 2, 1 );

		UIActionCard statusCard = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_ACTIVE_STATUSES" );
		Widget statusContent = statusCard.GetContent();
		Widget statusRow = UIActionManager.CreateRow( statusContent, JMFormBase.IDENTITY_ROW_HEIGHT );
		Widget statusGrid = UIActionManager.CreateGridSpacer( statusRow, 1, 5 );

		m_BadgeStatusDead        = CreateStatusIcon( statusGrid, "skull",          "#STR_COT_PLAYER_MODULE_STATUS_DEAD",        JMTheme.DANGER,  "" );
		m_BadgeStatusUnconscious = CreateStatusIcon( statusGrid, "bed",            "#STR_COT_PLAYER_MODULE_STATUS_UNCONSCIOUS", JMTheme.WARNING, "Click_StatusUnconscious" );
		m_BadgeStatusBrokenLeg   = CreateStatusIcon( statusGrid, "bone-fracture",  "#STR_COT_PLAYER_MODULE_STATUS_BROKEN_LEGS", JMTheme.WARNING, "Click_StatusBrokenLegs" );
		m_BadgeStatusBleeding    = CreateStatusIcon( statusGrid, "droplet",        "#STR_COT_PLAYER_MODULE_STATUS_BLEEDING",    JMTheme.DANGER,  "Click_StatusBleeding" );
		m_BadgeStatusSick        = CreateStatusIcon( statusGrid, "thermometer",    "#STR_COT_PLAYER_MODULE_STATUS_SICK",        JMTheme.DANGER,  "Click_StatusSick" );

		// Hidden until RefreshIdentityBadges finds a state to show. Built here
		// all the same: the card is cheap and rebuilding it per refresh would
		// drop the glyph the pointer is over mid right-click.
		m_StatusCardRoot = parent;
		m_StatusCardRoot.Show( false );

		return parent;
	}

	//! Repaint the identity badges from the synced flag bitmask. Called from
	//! RefreshStats, so it must stay cheap - four comparisons and at most four
	//! SetStatus calls.
	void RefreshIdentityBadges()
	{
		if ( !m_Form.m_SelectedInstance )
			return;

		if ( m_BadgeGodMode )
			SetFlagBadge( m_BadgeGodMode, m_Form.m_SelectedInstance.HasGodMode() );

		if ( m_BadgeFrozen )
			SetFlagBadge( m_BadgeFrozen, m_Form.m_SelectedInstance.IsFrozen() );

		if ( m_BadgeInvisible )
			SetFlagBadge( m_BadgeInvisible, m_Form.m_SelectedInstance.HasInvisibility() );

		if ( m_BadgeUnconscious )
			SetFlagBadge( m_BadgeUnconscious, m_Form.m_SelectedInstance.IsUnconscious() );

		bool isDead = m_Form.m_SelectedInstance.IsDead();
		bool isUncon = m_Form.m_SelectedInstance.IsUnconscious();
		bool hasBrokenLeg = m_Form.m_SelectedInstance.HasBrokenLegs();
		bool isBleeding = m_Form.m_SelectedInstance.IsBleeding();
		bool isSick = m_Form.m_SelectedInstance.IsSick();

		if ( m_BadgeStatusDead )
			m_BadgeStatusDead.SetVisible( isDead );
		if ( m_BadgeStatusUnconscious )
			m_BadgeStatusUnconscious.SetVisible( isUncon && !isDead );
		if ( m_BadgeStatusBrokenLeg )
			m_BadgeStatusBrokenLeg.SetVisible( hasBrokenLeg && !isDead );
		if ( m_BadgeStatusBleeding )
			m_BadgeStatusBleeding.SetVisible( isBleeding && !isDead );
		if ( m_BadgeStatusSick )
			m_BadgeStatusSick.SetVisible( isSick && !isDead );

		// Death hides the other four, so it alone decides the card when it is
		// set - otherwise any of the remaining states keeps it on screen.
		bool anyStatus = isDead;
		if ( !isDead )
			anyStatus = isUncon || hasBrokenLeg || isBleeding || isSick;

		if ( m_StatusCardRoot && m_StatusCardRoot.IsVisible() != anyStatus )
		{
			m_StatusCardRoot.Show( anyStatus );

			// The card sits in the General tab scroller, so appearing or going
			// away changes the height of everything under it.
			if ( m_Form.m_ActionListScroller )
				m_Form.m_ActionListScroller.UpdateScroller();
		}
	}

	//! One status glyph for the Active Statuses card. A single stage: these do
	//! not cycle, they are shown or hidden by RefreshIdentityBadges.
	//!
	//! `callback` answers UIEvent.CLICK_RIGHT and opens the repair menu for that
	//! status. Pass "" for a status with nothing to undo - death is the only one.
	UIActionStagedIcon CreateStatusIcon( Widget parent, string icon, string tooltip, int color, string callback )
	{
		UIActionStagedIcon status = UIActionManager.CreateStagedIcon( parent, this, callback );
		if ( !status )
			return NULL;

		status.AddStage( JMConstants.Lucide( icon ), color );

		//! Resolved here rather than handed to the tooltip as a key: the tooltip
		//! sizes its panel from the string's length, and a "#STR_..." key is not
		//! the length of the word it stands for.
		status.SetTooltip( Widget.TranslateString( tooltip ) );

		return status;
	}

	void Click_StatusUnconscious( UIEvent eid, UIActionBase action )
	{
		OpenStatusMenu( eid, action, JMPlayerRowMenu.ROW_MENU_WAKE );
	}

	void Click_StatusBrokenLegs( UIEvent eid, UIActionBase action )
	{
		OpenStatusMenu( eid, action, JMPlayerRowMenu.ROW_MENU_FIX_LEGS );
	}

	void Click_StatusBleeding( UIEvent eid, UIActionBase action )
	{
		OpenStatusMenu( eid, action, JMPlayerRowMenu.ROW_MENU_STOP_BLEED );
	}

	void Click_StatusSick( UIEvent eid, UIActionBase action )
	{
		OpenStatusMenu( eid, action, JMPlayerRowMenu.ROW_MENU_CURE );
	}

	//! Right-clicking a status glyph offers the one action that clears it. A
	//! one-entry menu on purpose: the glyph already named the problem, so the
	//! menu only has to name the fix and give it a deliberate second click.
	void OpenStatusMenu( UIEvent eid, UIActionBase action, string repairId )
	{
		if ( eid != UIEvent.CLICK_RIGHT || !m_Form.m_SelectedInstance || !m_Form.GetWindow() )
			return;

		UIActionStagedIcon icon = UIActionStagedIcon.Cast( action );
		if ( !icon )
			return;

		if ( !m_StatusMenu )
		{
			m_StatusMenu = UIActionManager.CreateOverlayMenu( m_Form, this, "OnClick_StatusMenu" );

			if ( !m_StatusMenu )
				return;
		}

		m_StatusMenuGUID = m_Form.m_SelectedInstance.GetGUID();

		m_StatusMenu.ClearItems();
		m_Form.AddStatusRepairItem( m_StatusMenu, repairId );

		if ( m_StatusMenu.GetItemCount() == 0 )
			return;

		m_Form.SetStatusRepairPermissions( m_StatusMenu );

		m_StatusMenu.OpenAt( icon.GetLastRightClickX(), icon.GetLastRightClickY() );
	}

	void OnClick_StatusMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_StatusMenu )
			return;

		m_Form.RunStatusRepair( m_StatusMenu.GetLastClickedId(), m_StatusMenuGUID );
	}

	void MakeFlagBadgeClickable( UIActionBadge badge, string callback, string tooltip )
	{
		if ( !badge )
			return;

		badge.SetCallback( this, callback );
		badge.SetClickable( true );

		if ( tooltip != "" )
			badge.SetTooltip( tooltip );
	}

	//! Flip one flag from its chip.
	//!
	//! The new value is the negation of what the SELECTED PLAYER has right now,
	//! not of what the chip is painted as: the paint is a frame of synced state
	//! and can lag a refresh behind, and a toggle that read its own label would
	//! send the value the player already has.
	//!
	//! The badge is repainted immediately rather than waiting for the next sync,
	//! so the click has an answer; RefreshIdentityBadges overwrites it with the
	//! truth a moment later, including when the server refused the change.
	void Click_BadgeGodMode( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK )
			FlipFlagBadge( JMPlayerToggle.GODMODE, m_BadgeGodMode );
	}

	void Click_BadgeFrozen( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK )
			FlipFlagBadge( JMPlayerToggle.FREEZE, m_BadgeFrozen );
	}

	void Click_BadgeInvisible( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK )
			FlipFlagBadge( JMPlayerToggle.INVISIBILITY, m_BadgeInvisible );
	}

	protected void FlipFlagBadge( string toggleId, UIActionBadge badge )
	{
		if ( !m_Form.m_SelectedInstance )
			return;

		JMPlayerToggle toggle = m_Form.m_Module.GetToggle( toggleId );
		if ( !toggle || !JMPermissions.Has( toggle.GetPermission() ) )
			return;

		int value = !toggle.Read( m_Form.m_SelectedInstance );

		m_Form.UpdateLastChangeTime();

		m_Form.m_Module.SetToggle( toggle, value, JM_GetSelected().GetPlayersOrSelf() );

		if ( m_Form.m_TabActionsCtrl )
		{
			UIActionCheckbox checkbox = m_Form.m_TabActionsCtrl.GetToggleCheckbox( toggleId );
			if ( checkbox )
				checkbox.SetChecked( value != 0 );
		}

		SetFlagBadge( badge, value != 0 );
	}

	//! There is no "set unconscious" call anywhere in the module, because
	//! consciousness IS shock: the engine drops a player as shock reaches zero
	//! and brings them round as it climbs back. So this chip is the shock slider
	//! driven to one end or the other, and it is gated on the shock permission
	//! for exactly that reason.
	void Click_BadgeUnconscious( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_SelectedInstance )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_SET_SHOCK ) )
			return;

		bool wake = m_Form.m_SelectedInstance.IsUnconscious();
		float shock = 0;

		if ( wake )
			shock = SHOCK_CONSCIOUS;

		m_Form.UpdateLastChangeTime();

		m_Form.m_Module.SetShock( shock, JM_GetSelected().GetPlayersOrSelf() );

		if ( m_Shock )
			m_Shock.SetCurrent( shock );

		SetFlagBadge( m_BadgeUnconscious, !wake );
	}

	void ShowIdentityWidgets()
	{
		m_Name.Show();
		m_GUID.Show();
		m_Steam64ID.Show();
		m_SteamProfile.Show();

		#ifdef GAMELABS
		m_CFToolsID.Show();
		m_CFProfile.Show();
		#endif
	}

	void HideIdentityWidgets()
	{
		// Nothing is selected, so there are no statuses to be active.
		if ( m_StatusCardRoot )
			m_StatusCardRoot.Show( false );

		m_Name.Hide();
		m_GUID.Hide();
		m_Steam64ID.Hide();
		m_SteamProfile.Hide();

		#ifdef GAMELABS
		m_CFToolsID.Hide();
		m_CFProfile.Hide();
		#endif
	}

	//! Vitals and temperatures.
	//!
	//! Name, signature and return type are frozen: DayZ-Expansion modded forms
	//! override this exact shape, call super, then inject their own rows into
	//! m_Stamina.GetLayoutRoot().GetParent() - the slider grid below - and
	//! re-sort them against m_ApplyStats and m_RefreshStats. Which tab this is
	//! called with does not matter to that; the anchor is internal.
	Widget InitActionWidgetsStats( Widget actionsParent )
	{
		// header + sliders + temp header + temps + apply + divider
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 8, 1 );

		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_HEADER" );
		Widget section0 = section0Card.GetContent();
		// 6 sliders in 5 rows of 2 - two spare cells, which is where Expansion
		// puts its faction / reputation rows plus their spacer panel.
		Widget actions = UIActionManager.CreateGridSpacer( section0, 5, 2 );

		m_Health = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_HEALTH", 0, 100, this, "OnChange_Stat" );
		m_Water = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_WATER", 0, PlayerConstants.SL_WATER_MAX, this, "OnChange_Stat" );

		m_Shock = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_SHOCK", 0, 100, this, "OnChange_Stat" );
		m_Energy = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_FOOD", 0, PlayerConstants.SL_ENERGY_MAX, this, "OnChange_Stat" );

		m_Blood = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_BLOOD", 0, 5000, this, "OnChange_Stat" );
		m_Stamina = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_STAMINA", 0, CfgGameplayHandler.GetStaminaMax(), this, "OnChange_Stat" );

		UIActionCard section1Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_TEMPERATURES" );
		Widget section1 = section1Card.GetContent();
		Widget actionsTemps = UIActionManager.CreateGridSpacer( section1, 1, 2 );
		m_HeatComfort = UIActionManager.CreateSlider( actionsTemps, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_HEATCOMFORT", -0.75, 0.75, this );
		m_HeatComfort.SetFormat("");
		m_HeatComfort.Disable(); // You cant force the game, so this slider is READ ONLY
		m_HeatComfort.SetDisableAlpha(0.0);

		m_HeatBuffer = UIActionManager.CreateSlider( actionsTemps, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_HEATBUFFER", 0, 3, this, "OnChange_Stat" );
		m_HeatBuffer.SetStepValue(0.1);

		//! Registration order is the order Apply sends them in.
		AddStatRow( JMStatType.Health, m_Health );
		AddStatRow( JMStatType.Blood, m_Blood );
		AddStatRow( JMStatType.Energy, m_Energy );
		AddStatRow( JMStatType.Water, m_Water );
		AddStatRow( JMStatType.Shock, m_Shock );
		AddStatRow( JMStatType.Stamina, m_Stamina );
		AddStatRow( JMStatType.HeatBuffer, m_HeatBuffer );

		m_Health.SetSliderWidth(0.5);
		m_Shock.SetSliderWidth(0.5);
		m_Blood.SetSliderWidth(0.5);
		m_Energy.SetSliderWidth(0.5);
		m_Water.SetSliderWidth(0.5);
		m_Stamina.SetSliderWidth(0.5);
		m_HeatComfort.SetSliderWidth(0.5);
		m_HeatBuffer.SetSliderWidth(0.5);

		// Apply and Refresh act on the whole card - every slider in it at once -
		// so they sit in its title bar with the other card-level quick actions
		// instead of costing a full row of height under the last slider.
		// Icon-only, so both keep the caption they lost as their tooltip.
		//
		// Created on the VARIABLES card, not the temperatures one: that is the
		// panel the sliders belong to and the one an admin reads them off.
		m_RefreshStats = section0Card.AddRefreshButton( this, "Click_RefreshVitals", "#STR_COT_PLAYER_MODULE_TT_REFRESH_STATS" );
		m_ApplyStats   = section0Card.AddApplyButton( m_Form, "Click_ApplyStats", "#STR_COT_PLAYER_MODULE_APPLY" );

		//! Legacy mirror for modded forms - see JMPlayerForm.SetLegacyStatWidgets.
		m_Form.SetLegacyStatWidgets( m_Stamina, m_ApplyStats, m_RefreshStats );

		return parent;
	}

	void Click_RefreshVitals( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.SpinRefreshIcon( action );
		m_Form.RefreshStats( true );
	}

    void Click_CopyPlayerName( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		//! No toast: the button swaps to "Copied!" where the click happened.
		g_Game.CopyToClipboard(m_Name.GetButton());
	}

    void Click_CopyPlayerGUID( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		g_Game.CopyToClipboard(m_GUID.GetButton());
	}

    void Click_CopyPlayerSteam64ID( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		g_Game.CopyToClipboard(m_Steam64ID.GetButton());
	}

    void Click_OpenPlayerSteamProfile( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		g_Game.OpenURL("https://steamcommunity.com/profiles/" + m_Steam64ID.GetButton());
	}

#ifdef GAMELABS
	void Click_CopyCFToolsID( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		g_Game.CopyToClipboard(m_CFToolsID.GetButton());
	}
#endif

#ifdef GAMELABS
    void Click_OpenPlayerCFProfile( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		g_Game.OpenURL(m_Form.m_SelectedInstance.PlayerObject.GetUpstreamIdentityHotlink());
	}
#endif

	void Click_ApplyStats( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_APPLY", this, "ApplyStatsTargets", false );
	}

	void ApplyStatsTargets( array<string> targets, JMConfirmation confirmation = NULL )
	{
		ApplyStats(targets);
	}

	void ApplyStats(TStringArray guids)
	{
		m_Form.UpdateLastChangeTime();

		foreach ( JMPlayerStatRow row : m_StatRows )
		{
			if ( !row.Dirty )
				continue;

			row.Dirty = false;

			m_Form.m_Module.SetStat( row.Stat.GetType(), row.GetValue(), guids );
		}

		//! Apply is otherwise silent: the sliders already read the value they
		//! were dragged to, so nothing on screen changes when the RPC goes out
		//! and an admin cannot tell a click that landed from one that missed.
		//! The check mark on the pill is the receipt.
		if ( m_ApplyStats )
			m_ApplyStats.AnimateFeedback();
	}

	//! Tie a slider to the stat it edits. A slider that was not built (a modded form can drop one)
	//! simply has no row.
	protected void AddStatRow( int type, UIActionSlider slider )
	{
		JMPlayerStat stat = m_Form.m_Module.GetStat( type );

		if ( stat && slider )
			m_StatRows.Insert( new JMPlayerStatRow( stat, slider ) );
	}

	//! Every stat slider lands here. Dragging one marks it dirty - so a refresh leaves it alone
	//! until Apply sends it - and repaints its colour.
	void OnChange_Stat( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		foreach ( JMPlayerStatRow row : m_StatRows )
		{
			if ( row.Slider == action )
			{
				ChangeStat( row.Stat.GetType() );
				return;
			}
		}
	}

	//! Mark one stat as edited by the admin.
	void ChangeStat( int type )
	{
		m_Form.UpdateLastChangeTime();

		foreach ( JMPlayerStatRow row : m_StatRows )
		{
			if ( row.Stat.GetType() == type )
			{
				row.Dirty = true;
				row.Paint();
			}
		}
	}

	//! Open the Role Manager on the selected player.
	//!
	//! The window is created by the module, exactly as the sidebar creates it,
	//! so an already-open Role Manager is reused rather than duplicated - and
	//! the deep link is handed over AFTER Show(), because the form does not
	//! exist until the window has built it.
	void Click_EditPlayerRoles( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_Form.m_SelectedInstance )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_ROLES_VIEW ) )
			return;

		JMRoleManagerModule roleModule;
		if ( !Class.CastTo( roleModule, GetModuleManager().GetModule( JMRoleManagerModule ) ) )
			return;

		roleModule.Show();

		JMRoleManagerForm roleForm;
		if ( !Class.CastTo( roleForm, roleModule.GetForm() ) )
			return;

		roleForm.OpenPlayer( m_Form.m_SelectedInstance.GetGUID() );
	}
}
