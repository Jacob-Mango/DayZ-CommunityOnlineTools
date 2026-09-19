//! "Active Flagged Players" tab of JMAntiCheatForm - detection rendering,
//! grouping/formatting, the right-click context menu, and the quick-action
//! buttons that raise the form's Confirm* dialogs. Back-reference to the
//! owning form, same shape as JMPlayerRowWidget.Menu.
class JMAntiCheatFormTabFlagged
{
	protected JMAntiCheatForm m_Form;

	//! A flagged player is two fixed-height rows: score+name+actions, then the
	//! guid and its fold button. Fixed-height PANELS, not wrap spacers - a wrap
	//! spacer pushes a child that does not fit onto a line of its own, which is
	//! what put the action strip and the fold button on rows by themselves. A
	//! panel positions its children outright and never wraps.
	static const float ROW_IDENTITY_PX  = 34;
	static const float ROW_GUID_PX      = 30;

	//! One detection line, and where its four pieces sit across it: clock button,
	//! reason, score delta, detail. The row has to clear the 32px clock button
	//! AND the 30px text layout inside it, so anything near either number packs
	//! the lines against each other and clips their descenders.
	static const float DETECTION_ROW_PX    = 40;

	//! One occurrence under an expanded group. Shorter than its group header -
	//! it is subordinate and reads as such - but still taller than the 30px text
	//! it carries.
	static const float OCCURRENCE_ROW_PX   = 34;

	static const float ACTION_WRAP_X    = 0.66;
	static const float ACTION_WRAP_W    = 0.34;

	protected UIActionScroller m_FlaggedScroller;
	protected Widget m_FlaggedContent;

	protected UIActionContextMenu m_ContextMenu;
	protected string m_ContextGuid;

	//! Which detection groups have their occurrences showing. Held here rather
	//! than on the widget because the whole list is torn down and rebuilt on
	//! every refresh, and an admin reading a detection should not have it fold
	//! shut under them a second later.
	protected ref TStringArray m_ExpandedSubGroups = new TStringArray;

	//! The live folds, so a click can open the one it belongs to without the list
	//! being torn down and rebuilt underneath it. Keyed the same way the expanded
	//! sets are: by guid for a player, by "guid|group" for one detection group.
	protected ref map< string, UIActionFoldPanel > m_Folds = new map< string, UIActionFoldPanel >;

	void JMAntiCheatFormTabFlagged( JMAntiCheatForm form )
	{
		m_Form = form;
	}

	void Build( Widget parentPanel )
	{
		if ( !parentPanel )
			return;

		m_FlaggedScroller = UIActionManager.CreateScroller( parentPanel );
		Widget scrollerContent = m_FlaggedScroller.GetContentWidget();

		UIActionCard card = UIActionManager.CreateCard( scrollerContent, "Cheat Detections & Flagged Players" );
		card.AddRefreshButton( m_Form, "OnClick_Refresh", "Refresh flagged detections" );

		m_FlaggedContent = card.GetContent();
		m_FlaggedScroller.UpdateScroller();
	}

	void OnResize()
	{
		if ( m_FlaggedScroller )
			m_FlaggedScroller.UpdateScroller();
	}

	//! Built on first use, not in Build(): the menu has to anchor to the window
	//! root so it can float over the scroller instead of being clipped by it,
	//! and the window is not guaranteed to be set yet while the form is being
	//! initialised.
	private bool EnsureContextMenu()
	{
		if ( m_ContextMenu )
			return true;

		CF_Window window = m_Form.GetWindow();
		if ( !window )
			return false;

		m_ContextMenu = UIActionManager.CreateContextMenu( m_Form.GetLayoutRoot(), window.GetWidgetRoot(), this, "OnClick_ContextMenu" );
		if ( !m_ContextMenu )
			return false;

		m_Form.RegisterOverlay( m_ContextMenu );

		return true;
	}

	//! Open or shut the occurrences under one detection group. The panel animates
	//! itself, so this only has to flip it and remember the choice - no rebuild,
	//! which is what used to make the fold jump: every widget in the card was
	//! destroyed and recreated on a click, mid-slide.
	void OnClick_ToggleSubGroup( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMAntiCheatSubGroupData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		string key = data.Guid + "|" + data.GroupKey;

		ToggleFold( key, m_ExpandedSubGroups, key, action );
	}

	//! Flip the fold registered under `foldKey`, record it in `state`, and turn
	//! the chevron on the button that was clicked. Shared by both folds because
	//! the only thing that differs between them is which set remembers them.
	private void ToggleFold( string foldKey, TStringArray state, string stateKey, UIActionBase button )
	{
		UIActionFoldPanel fold = m_Folds.Get( foldKey );
		if ( !fold )
			return;

		fold.Toggle();

		int idx = state.Find( stateKey );

		if ( fold.IsExpanded() )
		{
			if ( idx < 0 )
				state.Insert( stateKey );
		}
		else if ( idx >= 0 )
		{
			state.Remove( idx );
		}

		ApplyChevron( button, fold.IsExpanded() );
	}

	//! The chevron points at what a click will do next. Retinted in place rather
	//! than rebuilt, so the button the pointer is resting on stays the same widget.
	private void ApplyChevron( UIActionBase button, bool expanded )
	{
		UIActionImageButton icon;
		if ( !Class.CastTo( icon, button ) )
			return;

		if ( expanded )
		{
			icon.SetImage( JMConstants.Lucide( "chevron-up" ) );
			icon.SetTooltip( "Hide" );

			return;
		}

		icon.SetImage( JMConstants.Lucide( "chevron-down" ) );
		icon.SetTooltip( "Show" );
	}

	//! A fold is mid-slide, so the card is a different height than it was last
	//! frame and the scroller's range is stale.
	void OnChange_Fold( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_FlaggedScroller )
			m_FlaggedScroller.UpdateScroller();
	}

	// -----------------------------------------------------------------------
	//  Detection line parsing
	//
	//  A raw event reads:
	//      [2026-08-28 17:16Z] (+22) Unauthorized RPC: 3 denied in 30s
	//
	//  The stamp goes to the clock's tooltip and the delta gets its own red
	//  cell, so the line itself is left with the part that is actually prose.
	// -----------------------------------------------------------------------

	//! The bracketed stamp a detection line opens with, for the clock's tooltip.
	//! Returns the whole line when there is no stamp to lift out of it.
	private string ExtractTimestamp( string raw )
	{
		int open  = raw.IndexOf( "[" );
		int close = raw.IndexOf( "]" );

		if ( open < 0 || close <= open + 1 )
			return raw;

		return raw.Substring( open + 1, close - open - 1 );
	}

	//! The score this detection added, as "+22". "" when the line carries none.
	private string ExtractDelta( string raw )
	{
		int open = raw.IndexOf( "(+" );
		if ( open < 0 )
			return "";

		int close = raw.IndexOfFrom( open, ")" );
		if ( close <= open + 1 )
			return "";

		return raw.Substring( open + 1, close - open - 1 );
	}

	//! The line with its timestamp and its delta taken out, since both are shown
	//! elsewhere on the row now.
	private string StripDecorations( string raw )
	{
		string rest = raw;

		int close = rest.IndexOf( "]" );
		if ( close >= 0 )
			rest = rest.Substring( close + 1, rest.Length() - close - 1 );

		int open = rest.IndexOf( "(+" );
		if ( open >= 0 )
		{
			int dclose = rest.IndexOfFrom( open, ")" );
			if ( dclose > open )
				rest = rest.Substring( dclose + 1, rest.Length() - dclose - 1 );
		}

		return rest.Trim();
	}

	private int ExtractDeltaValue( string raw )
	{
		string deltaStr = ExtractDelta( raw );
		if ( deltaStr == "" )
			return 0;

		deltaStr.Replace( "+", "" );
		deltaStr.Replace( " ", "" );
		return deltaStr.ToInt();
	}

	private array<ref JMAntiCheatGroupEntry> GroupDetections( array<string> recentEvents )
	{
		array<ref JMAntiCheatGroupEntry> groups = new array<ref JMAntiCheatGroupEntry>;
		map<string, ref JMAntiCheatGroupEntry> groupMap = new map<string, ref JMAntiCheatGroupEntry>;

		if ( !recentEvents )
			return groups;

		foreach ( string evt : recentEvents )
		{
			string humanTitle = FormatHumanReason( evt );
			JMAntiCheatGroupEntry groupEntry = groupMap.Get( humanTitle );

			if ( !groupEntry )
			{
				groupEntry = new JMAntiCheatGroupEntry( humanTitle );
				groupMap.Insert( humanTitle, groupEntry );
				groups.Insert( groupEntry );
			}

			groupEntry.Count++;

			int points = ExtractDeltaValue( evt );
			groupEntry.TotalPoints += points;

			if ( groupEntry.LatestTechnical == "" )
			{
				groupEntry.LatestTechnical = StripDecorations( evt );
			}

			groupEntry.Occurrences.Insert( evt );
		}

		return groups;
	}

	private string FormatHumanReason( string raw )
	{
		if ( raw.IndexOf( "Unauthorized RPC" ) != -1 )
			return "Unauthorized Admin RPC Call";
		if ( raw.IndexOf( "Teleport" ) != -1 )
			return "Instant Teleportation";
		if ( raw.IndexOf( "Speed" ) != -1 )
			return "Movement Speed Hack";
		if ( raw.IndexOf( "Aimbot" ) != -1 || raw.IndexOf( "Headshot" ) != -1 )
			return "Suspicious Hit Accuracy";
		if ( raw.IndexOf( "Godmode" ) != -1 || raw.IndexOf( "Health" ) != -1 )
			return "Health Anomaly / Invulnerability";

		return raw;
	}

	// -----------------------------------------------------------------------
	//  Destructive quick actions
	//
	//  Clear, Kick and Ban used to be inline confirm buttons, which is what made
	//  them look unlike Freeze and Page. They are plain icon buttons now, so the
	//  guard they used to carry has to live somewhere - it moved to the window's
	//  confirmation dialog. The guid is parked on the form because the dialog
	//  callbacks are named methods and take no argument of their own, and those
	//  callbacks (Confirm*) have to stay on the form - see JMAntiCheatForm's
	//  file header.
	// -----------------------------------------------------------------------

	void OnClick_Clear( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMAntiCheatClearData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_Form.m_PendingGuid = data.Guid;

		m_Form.CreateConfirmation_Two( JMConfirmationType.INFO, "Clear Anti-Cheat Flags", "Clear every anti-cheat flag for " + PendingPlayerName() + "?", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "ConfirmClear" );
	}

	//! Name for the confirmation text, falling back to the guid when the flag is
	//! gone - the dialog is raised from a row, so it normally is not.
	private string PendingPlayerName()
	{
		if ( !m_Form.m_Module )
			return m_Form.m_PendingGuid;

		map< string, ref JMAntiCheatFlag > flags = m_Form.m_Module.GetClientFlags();
		if ( !flags )
			return m_Form.m_PendingGuid;

		JMAntiCheatFlag flag = flags.Get( m_Form.m_PendingGuid );
		if ( !flag || flag.PlayerName == "" )
			return m_Form.m_PendingGuid;

		return flag.PlayerName;
	}

	void OnClick_Freeze( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMAntiCheatClearData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		JMPlayerModule playerMod = JMPlayerModule.Cast( GetModuleManager().GetModule( JMPlayerModule ) );
		if ( playerMod )
		{
			playerMod.SetFreeze( true, { data.Guid } );
			action.AnimateFeedback();
		}
	}

	void OnClick_Kick( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMAntiCheatClearData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_Form.m_PendingGuid = data.Guid;

		m_Form.CreateConfirmation_Two( JMConfirmationType.INFO, "Kick Player", "Kick " + PendingPlayerName() + " from the server?", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "ConfirmKick" );
	}

	void OnClick_Ban( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMAntiCheatClearData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_Form.m_PendingGuid = data.Guid;

		m_Form.CreateConfirmation_Two( JMConfirmationType.INFO, "Ban Player", "Ban " + PendingPlayerName() + " from the server?", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "ConfirmBan" );
	}

	void OnClick_PlayerPage( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMAntiCheatClearData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		JM_GetSelected().ClearPlayers();
		JM_GetSelected().AddPlayer( data.Guid );

		JMPlayerModule playerMod = JMPlayerModule.Cast( GetModuleManager().GetModule( JMPlayerModule ) );
		if ( playerMod )
			playerMod.Show();
	}

	// -----------------------------------------------------------------------
	//  Context Menu
	// -----------------------------------------------------------------------

	void ShowPlayerContextMenu( string guid, int x, int y )
	{
		m_ContextGuid = guid;

		if ( !EnsureContextMenu() )
			return;

		m_ContextMenu.ClearItems();
		m_ContextMenu.AddItem( "freeze",      "Freeze Player",          JMConstants.Lucide( "snowflake" ) );
		m_ContextMenu.AddItem( "kick",        "Kick Player",            JMConstants.Lucide( "user-x" ),    JMTheme.WARNING );
		m_ContextMenu.AddItem( "ban",         "Ban Player",             JMConstants.Lucide( "ban" ),       JMTheme.DANGER );
		m_ContextMenu.AddItem( "clear",       "Clear Anti-Cheat Flag",  JMConstants.Lucide( "trash-2" ) );
		m_ContextMenu.AddItem( "player_page", "Open Player Page",       JMConstants.Lucide( "user" ) );

		m_ContextMenu.ShowAt( x, y );
	}

	void OnClick_ContextMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || m_ContextGuid == "" )
			return;

		string selectedId = m_ContextMenu.GetLastClickedId();

		//! Enforce has no block scope, so the module handle is declared once for
		//! the whole function rather than in each branch that wants it.
		JMPlayerModule playerMod = JMPlayerModule.Cast( GetModuleManager().GetModule( JMPlayerModule ) );

		if ( selectedId == "freeze" )
		{
			if ( playerMod ) playerMod.SetFreeze( true, { m_ContextGuid } );
		}
		//! The three destructive ones go through the same confirmation the quick
		//! action buttons raise. An admin who has learned that Ban asks first
		//! should not find a second route that does not.
		else if ( selectedId == "kick" )
		{
			m_Form.m_PendingGuid = m_ContextGuid;
			m_Form.CreateConfirmation_Two( JMConfirmationType.INFO, "Kick Player", "Kick " + PendingPlayerName() + " from the server?", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "ConfirmKick" );
		}
		else if ( selectedId == "ban" )
		{
			m_Form.m_PendingGuid = m_ContextGuid;
			m_Form.CreateConfirmation_Two( JMConfirmationType.INFO, "Ban Player", "Ban " + PendingPlayerName() + " from the server?", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "ConfirmBan" );
		}
		else if ( selectedId == "clear" )
		{
			m_Form.m_PendingGuid = m_ContextGuid;
			m_Form.CreateConfirmation_Two( JMConfirmationType.INFO, "Clear Anti-Cheat Flags", "Clear every anti-cheat flag for " + PendingPlayerName() + "?", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "ConfirmClear" );
		}
		else if ( selectedId == "player_page" )
		{
			JM_GetSelected().ClearPlayers();
			JM_GetSelected().AddPlayer( m_ContextGuid );

			if ( playerMod )
				playerMod.Show();
		}
	}

	// -----------------------------------------------------------------------
	//  Render Walker
	// -----------------------------------------------------------------------

	private void ClearWidgetChildren( Widget parent )
	{
		if ( !parent )
			return;

		Widget child = parent.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}
	}

	void Rebuild( array< string > order, map< string, ref JMAntiCheatFlag > flags, string filter, bool lowestFirst )
	{
		if ( !m_FlaggedContent )
			return;

		// Every fold below is about to be unlinked. They are re-registered as the
		// rows are rebuilt; until then this must not hand out a dead one.
		m_Folds.Clear();

		ClearWidgetChildren( m_FlaggedContent );
		Widget body = m_FlaggedContent;

		if ( !order || order.Count() == 0 )
		{
			Widget empty = UIActionManager.CreatePanel( body, 0x00000000, 60 );
			UIActionManager.CreateText( empty, "No flags detected.", "Server is clean. Detections appear here automatically when suspicious activity occurs." );
			m_FlaggedScroller.UpdateScroller();
			return;
		}

		array< string > sortedOrder = new array< string >;
		foreach ( string g : order )
			sortedOrder.Insert( g );

		if ( !lowestFirst ) // Highest Score First
		{
			// Bubble sort by TotalScore descending
			for ( int s1 = 0; s1 < sortedOrder.Count(); s1++ )
			{
				for ( int s2 = s1 + 1; s2 < sortedOrder.Count(); s2++ )
				{
					JMAntiCheatFlag f1 = flags.Get( sortedOrder[s1] );
					JMAntiCheatFlag f2 = flags.Get( sortedOrder[s2] );
					if ( f1 && f2 && f2.TotalScore > f1.TotalScore )
					{
						string tmp = sortedOrder[s1];
						sortedOrder[s1] = sortedOrder[s2];
						sortedOrder[s2] = tmp;
					}
				}
			}
		}
		else // Lowest Score First
		{
			for ( int l1 = 0; l1 < sortedOrder.Count(); l1++ )
			{
				for ( int l2 = l1 + 1; l2 < sortedOrder.Count(); l2++ )
				{
					JMAntiCheatFlag lf1 = flags.Get( sortedOrder[l1] );
					JMAntiCheatFlag lf2 = flags.Get( sortedOrder[l2] );
					if ( lf1 && lf2 && lf2.TotalScore < lf1.TotalScore )
					{
						string ltmp = sortedOrder[l1];
						sortedOrder[l1] = sortedOrder[l2];
						sortedOrder[l2] = ltmp;
					}
				}
			}
		}

		int rendered = 0;

		foreach ( string guid : sortedOrder )
		{
			JMAntiCheatFlag flag = flags.Get( guid );
			if ( !flag )
				continue;

			if ( filter != "" )
			{
				string nameLow = flag.PlayerName;
				nameLow.ToLower();
				string guidLow = guid;
				guidLow.ToLower();

				if ( nameLow.IndexOf( filter ) == -1 && guidLow.IndexOf( filter ) == -1 )
					continue;
			}

			rendered++;

			bool hasEvents = ( flag.RecentEvents && flag.RecentEvents.Count() > 0 );

			// -- Line 1: score, name, quick actions hard right ------------------
			//
			// Name and GUID were one UIActionText, whose label sits hard left and
			// whose value sits hard right; a 64-character GUID reached back far
			// enough to be drawn over the name. Two lines, one string each.
			Widget rowIdentity = UIActionManager.CreatePanel( body, 0x00000000, ROW_IDENTITY_PX );
			rowIdentity.SetUserData( new JMAntiCheatClearData( guid ) );

			UIActionBadge scoreBadge = UIActionManager.CreateBadge( rowIdentity, "", "Score: " + flag.TotalScore.ToString(), ScoreToColor( flag.TotalScore ) );
			scoreBadge.SetWidth( 0.12 );
			scoreBadge.SetTooltip( "Current cheat detection score. It decays over time, so it is lower than the detections below add up to." );

			UIActionText nameText = UIActionManager.CreateText( rowIdentity, flag.PlayerName );
			nameText.SetWidth( 0.50 );
			nameText.SetPosition( 0.14 );

			// Quick actions: Freeze / Kick / Ban / Player Page + Clear Flag, all
			// plain 32px icon buttons so the row reads as one set of controls.
			//
			// Created in reverse of how they read - the wrap spacer is aligned
			// right, so the first one built ends up furthest right.
			Widget actionWrap = UIActionManager.CreateWrapSpacer( rowIdentity, WidgetAlignment.WA_RIGHT, WidgetAlignment.WA_CENTER );

			// A plain Widget has no SetWidth/SetPosition - those are UIActionBase
			// methods. The panel's children are placed by fraction of its width.
			UIActionManager.SetWidthFraction( actionWrap, ACTION_WRAP_W );
			actionWrap.SetPos( ACTION_WRAP_X, 0 );

			UIActionImageButton freezeBtn = UIActionManager.CreateIconButton( actionWrap, JMConstants.Lucide( "snowflake" ), this, "OnClick_Freeze" );
			freezeBtn.SetFixedSize( JMFormBase.ICON_BUTTON_PX, JMFormBase.ICON_BUTTON_PX );
			freezeBtn.SetData( new JMAntiCheatClearData( guid ) );
			freezeBtn.SetTooltip( "Freeze player movement" );

			UIActionImageButton kickBtn = UIActionManager.CreateIconButton( actionWrap, JMConstants.Lucide( "user-x" ), this, "OnClick_Kick" );
			kickBtn.SetFixedSize( JMFormBase.ICON_BUTTON_PX, JMFormBase.ICON_BUTTON_PX );
			kickBtn.SetData( new JMAntiCheatClearData( guid ) );
			kickBtn.SetTooltip( "Kick player from server" );

			UIActionImageButton banBtn = UIActionManager.CreateIconButton( actionWrap, JMConstants.Lucide( "ban" ), this, "OnClick_Ban" );
			banBtn.SetFixedSize( JMFormBase.ICON_BUTTON_PX, JMFormBase.ICON_BUTTON_PX );
			banBtn.SetData( new JMAntiCheatClearData( guid ) );
			banBtn.SetTooltip( "Ban player from server" );

			UIActionImageButton pageBtn = UIActionManager.CreateIconButton( actionWrap, JMConstants.Lucide( "user" ), this, "OnClick_PlayerPage" );
			pageBtn.SetFixedSize( JMFormBase.ICON_BUTTON_PX, JMFormBase.ICON_BUTTON_PX );
			pageBtn.SetData( new JMAntiCheatClearData( guid ) );
			pageBtn.SetTooltip( "Open player detail page" );

			UIActionImageButton clearBtn = UIActionManager.CreateIconButton( actionWrap, JMConstants.ICON_TRASH_CAN, this, "OnClick_Clear" );
			clearBtn.SetFixedSize( JMFormBase.ICON_BUTTON_PX, JMFormBase.ICON_BUTTON_PX );
			clearBtn.SetData( new JMAntiCheatClearData( guid ) );
			clearBtn.SetTooltip( "Clear anti-cheat flags for this player" );
			m_Form.BindPermission( clearBtn, JMConstants.PERM_ANTICHEAT_CLEAR );

			// -- Line 2: guid ---------------------------------------------------
			Widget rowGuid = UIActionManager.CreatePanel( body, 0x00000000, ROW_GUID_PX );
			rowGuid.SetUserData( new JMAntiCheatClearData( guid ) );

			UIActionText guidText = UIActionManager.CreateText( rowGuid, "GUID: " + guid );
			guidText.SetWidth( 0.96 );

			// -- Detections ------------------------------------------------------
			//
			// Always on screen: a per-player fold on top of the per-group folds
			// was one layer of opening too many, and every row under it was one
			// an admin had already asked to see by opening the tab.
			if ( hasEvents )
			{
				array<ref JMAntiCheatGroupEntry> detectionGroups = GroupDetections( flag.RecentEvents );

				foreach ( JMAntiCheatGroupEntry group : detectionGroups )
				{
					string subKey = guid + "|" + group.Key;
					bool isSubExpanded = ( m_ExpandedSubGroups.Find( subKey ) >= 0 );

					Widget groupRow = UIActionManager.CreatePanel( body, JMTheme.SURFACE_SUNKEN, DETECTION_ROW_PX );
					groupRow.SetUserData( new JMAntiCheatClearData( guid ) );

					UIActionText evtText = UIActionManager.CreateText( groupRow, group.HumanTitle + "  " + group.Count.ToString() + "x" );
					evtText.SetWidth( 0.60 );
					evtText.SetPosition( 0.02 );
					evtText.SetLabelColor( JMTheme.TEXT_PRIMARY );

					// The weight these detections ADDED, which is deliberately not
					// the score above and cannot be made to match it: the score
					// decays hour by hour, and only the last 25 events are kept,
					// so the sum runs high after a quiet spell and low after a
					// busy one. Saying which number is which is the honest fix.
					if ( group.TotalPoints > 0 )
					{
						UIActionText evtDelta = UIActionManager.CreateText( groupRow, "+" + group.TotalPoints.ToString() );
						evtDelta.SetWidth( 0.10 );
						evtDelta.SetPosition( 0.63 );
						evtDelta.SetLabelColor( JMTheme.DANGER );
						evtDelta.SetTooltip( "Score these detections added when they fired. The player's score decays over time and only the last 25 detections are kept, so this does not add up to the score above." );
					}

					string subIcon = JMConstants.Lucide( "chevron-down" );
					string subTip  = "Show all " + group.Count.ToString() + " occurrence(s)";
					if ( isSubExpanded )
					{
						subIcon = JMConstants.Lucide( "chevron-up" );
						subTip  = "Hide occurrences";
					}

					Widget subExpandWrap = UIActionManager.CreateWrapSpacer( groupRow, WidgetAlignment.WA_RIGHT, WidgetAlignment.WA_CENTER );
					UIActionManager.SetWidthFraction( subExpandWrap, 0.06 );
					subExpandWrap.SetPos( 0.94, 0 );

					UIActionImageButton subExpandBtn = UIActionManager.CreateIconButton( subExpandWrap, subIcon, this, "OnClick_ToggleSubGroup" );
					subExpandBtn.SetFixedSize( JMFormBase.ICON_BUTTON_PX, JMFormBase.ICON_BUTTON_PX );
					subExpandBtn.SetData( new JMAntiCheatSubGroupData( guid, group.Key ) );
					subExpandBtn.SetTooltip( subTip );

					UIActionFoldPanel occurrenceFold = UIActionManager.CreateFoldPanel( body, this, "OnChange_Fold", isSubExpanded );
					occurrenceFold.SetColor( JMTheme.SURFACE_SUNKEN );

					m_Folds.Set( subKey, occurrenceFold );

					Widget occurrenceBody = occurrenceFold.GetContent();

					foreach ( string occStr : group.Occurrences )
					{
						Widget occRow = UIActionManager.CreatePanel( occurrenceBody, 0x00000000, OCCURRENCE_ROW_PX );
						occRow.SetUserData( new JMAntiCheatClearData( guid ) );

						// ASCII branch glyph: a non-ASCII byte anywhere in a .c
						// poisons the script type-pool, and this file's confirm
						// labels were already cut back for that.
						UIActionText occItem = UIActionManager.CreateText( occRow, "    |- " + ExtractTimestamp( occStr ) + "  " + ExtractDelta( occStr ), StripDecorations( occStr ) );
						occItem.SetWidth( 0.94 );
						occItem.SetPosition( 0.05 );
						occItem.SetLabelColor( JMTheme.TEXT_MUTED );
						occItem.SetValueColor( JMTheme.TEXT_MUTED );
					}
				}
			}

			UIActionManager.CreatePanel( body, 0x22FFFFFF, 1 );
		}

		if ( rendered == 0 && filter != "" )
		{
			Widget emptyFilter = UIActionManager.CreatePanel( body, 0x00000000, 40 );
			UIActionManager.CreateText( emptyFilter, "No flagged players match filter '" + filter + "'." );
		}

		m_FlaggedScroller.UpdateScroller();
	}

	private int ScoreToColor( int score )
	{
		if ( score >= 30 ) return ARGB( 255, 217, 90, 90 );
		if ( score >= 15 ) return ARGB( 255, 217, 164, 65 );
		return ARGB( 255, 73, 184, 117 );
	}
}
