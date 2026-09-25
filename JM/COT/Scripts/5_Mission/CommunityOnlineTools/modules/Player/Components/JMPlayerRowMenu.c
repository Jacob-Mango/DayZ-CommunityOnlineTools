//! The player list's right-click menu, and the status repairs the Active Statuses
//! card shares with it. The form keeps OnPlayerRow_RightClick and m_PlayerMenu as
//! the extension point sub-mods already use; this class does the work behind them.
class JMPlayerRowMenu
{
	protected JMPlayerForm m_Form;

	//! Right-click menu for the player list, and the player it was opened on.
	//! The GUID is captured at open time and every item acts on THAT player, so
	//! the menu can never run on a different one than the row under the pointer
	//! - the selection set can change underneath it while it is up.
	protected UIActionContextMenu m_Menu;
	protected string              m_Guid;

	//! Context-menu item ids. Strings rather than an enum because that is what
	//! UIActionContextMenu round-trips.
	//! Quick actions on the row the pointer is over. Ids, not indices: the menu
	//! is rebuilt per click and the freeze entry changes label with the player.
	static const string ROW_MENU_HEAL       = "heal";
	static const string ROW_MENU_TP_TO_ME   = "tp_to_me";
	static const string ROW_MENU_TP_ME_TO   = "tp_me_to";
	static const string ROW_MENU_TP_UNDO    = "tp_undo";
	static const string ROW_MENU_TP_REDO    = "tp_redo";
	static const string ROW_MENU_FOCUS_MAP  = "focus_map";
	static const string ROW_MENU_COPY_GUID  = "copy_guid";
	static const string ROW_MENU_COPY_STEAM = "copy_steam";
	static const string ROW_MENU_MESSAGE    = "message";
	static const string ROW_MENU_CLEARCARGO = "clear_cargo";
	static const string ROW_MENU_STRIP      = "strip";
	static const string ROW_MENU_KICK       = "kick";
	static const string ROW_MENU_BAN        = "ban";
	static const string ROW_MENU_FREEZE     = "freeze";
	static const string ROW_MENU_RAGDOLL    = "ragdoll";
	static const string ROW_MENU_SPECTATE   = "spectate";

	//! Status repairs. Listed on the row menu only while the player is actually
	//! in that state - an admin scanning a roster wants the fix for what is
	//! wrong with this one, not the full catalogue of what could be.
	static const string ROW_MENU_FIX_LEGS   = "fix_legs";
	static const string ROW_MENU_STOP_BLEED = "stop_bleed";
	static const string ROW_MENU_CURE       = "cure";
	static const string ROW_MENU_WAKE       = "wake";

	//! GUID a row-menu message prompt is pending for, between ShowAt() (in
	//! RunRowMessage) and the confirmation's own callback - same pattern as
	//! JMESPForm.m_PendingMsgPlayerGUID / PromptSendMessage, which already
	//! solved this identically for the world right-click player menu.
	protected string m_PendingRowMsgGuid;

	void JMPlayerRowMenu( JMPlayerForm form )
	{
		m_Form = form;
	}

	//! Grey rather than drop: SetItemEnabled on an id the menu does not carry is
	//! a no-op, so this can be called unconditionally after every build.
	void SetStatusRepairPermissions( UIActionContextMenu menu )
	{
		if ( !menu )
			return;

		menu.SetItemEnabled( ROW_MENU_WAKE,       JMPermissions.Has( JMConstants.PERM_PLAYER_SET_SHOCK ) );
		menu.SetItemEnabled( ROW_MENU_FIX_LEGS,   JMPermissions.Has( JMConstants.PERM_PLAYER_BROKENLEGS ) );
		menu.SetItemEnabled( ROW_MENU_STOP_BLEED, JMPermissions.Has( JMConstants.PERM_PLAYER_BLEED_ADD ) );
		menu.SetItemEnabled( ROW_MENU_CURE,       JMPermissions.Has( JMConstants.PERM_PLAYER_DISEASE_REMOVE ) );
	}

	// -------------------------------------------------------------------------
	//  Player list quick actions
	// -------------------------------------------------------------------------

	//! A row was right-clicked. Built fresh every time because the freeze entry
	//! reads the player state, and greyed rather than dropped where a permission
	//! is missing so the menu keeps one shape whoever opens it.
	void Open( string guid, int x, int y )
	{
		if ( guid == "" )
			return;

		if ( !m_Form || !m_Form.m_Module )
		{
			Error("[JMPlayerRowMenu] Open failed: m_Form or m_Form.m_Module is null!");
			return;
		}

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( guid );
		if ( !instance )
			return;

		if ( !m_Menu )
		{
			if ( !m_Form.GetWindow() )
			{
				Error("[JMPlayerRowMenu] Open failed: m_Form.GetWindow() returned null!");
				return;
			}

			//! Anchored to the window root, not to the list: a menu parented to
			//! the scroller would be clipped by it a row from the bottom.
			m_Menu = UIActionManager.CreateOverlayMenu( m_Form, this, "OnClick_PlayerMenu" );

			if ( !m_Menu )
			{
				Error("[JMPlayerRowMenu] Open failed: Could not create UIActionContextMenu (m_Menu is null)!");
				return;
			}
		}

		m_Guid = guid;

		//! Sub-mods read the menu off the form, not off this class.
		m_Form.SetPlayerMenu( m_Menu, m_Guid );

		m_Menu.ClearItems();

		//! The repairs come first and only for the states the player is in, so
		//! the entry an admin opened the menu for is the one under the cursor.
		AddStatusRepairItems( m_Menu, instance );

		//! Two entries are meaningless on your own row and are left out rather
		//! than greyed: "teleport me to" would be a no-op walk to where you are
		//! already standing, and spectating yourself is what the game already
		//! does. "Teleport to me" stays - it is how an admin recalls a character
		//! they left somewhere, and it reads the same either way.
		bool isSelf = ( guid == GetPermissionsManager().GetClientGUID() );

		m_Menu.AddItem( ROW_MENU_HEAL,     "#STR_COT_PLAYER_MODULE_ACTION_HEAL",     JMConstants.Lucide( "heart-pulse" ) );
		m_Menu.AddItem( ROW_MENU_TP_TO_ME, "#STR_COT_PLAYER_MODULE_TELEPORT_TO_ME",  JMConstants.Lucide( "move-down-left" ) );

		if ( !isSelf )
			m_Menu.AddItem( ROW_MENU_TP_ME_TO, "#STR_COT_PLAYER_MODULE_TELEPORT_ME_TO",  JMConstants.Lucide( "footprints" ) );

		//! Greyed rather than dropped when there is nothing to take back: an
		//! entry that comes and goes moves every row under it between one
		//! opening of the menu and the next.
		m_Menu.AddItem( ROW_MENU_TP_UNDO, "#STR_COT_TELEPORT_UNDO", JMConstants.Lucide( "undo-2" ) );
		m_Menu.AddItem( ROW_MENU_TP_REDO, "#STR_COT_TELEPORT_REDO", JMConstants.Lucide( "redo-2" ) );

		//! Moves the Position tab's map, not the player - the one way to tell
		//! two sets of coordinates apart is to look at them.
		m_Menu.AddItem( ROW_MENU_FOCUS_MAP, "#STR_COT_TELEPORT_HISTORY_FOCUS", JMConstants.Lucide( "locate-fixed" ) );

		if ( instance.IsFrozen() )
			m_Menu.AddItem( ROW_MENU_FREEZE, "#STR_COT_PLAYER_MODULE_ACTION_UNFREEZE", JMConstants.Lucide( "snowflake" ) );
		else
			m_Menu.AddItem( ROW_MENU_FREEZE, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_FREEZE", JMConstants.Lucide( "snowflake" ) );

#ifndef DAYZ_1_29
		if ( instance.IsRagdoll() )
			m_Menu.AddItem( ROW_MENU_RAGDOLL, "#STR_COT_PLAYER_MODULE_ACTION_UNRAGDOLL", JMConstants.Lucide( "bone" ) );
		else
			m_Menu.AddItem( ROW_MENU_RAGDOLL, "#STR_COT_PLAYER_MODULE_ACTION_RAGDOLL", JMConstants.Lucide( "bone" ) );
#endif

		if ( !isSelf )
			m_Menu.AddItem( ROW_MENU_SPECTATE, "#STR_COT_PLAYER_MODULE_ACTION_SPECTATE", JMConstants.Lucide( "eye" ) );

		m_Menu.AddItem( ROW_MENU_COPY_GUID,  "#STR_COT_ESP_MODULE_MENU_COPY_GUID", JMConstants.Lucide( "copy" ) );
		m_Menu.AddItem( ROW_MENU_COPY_STEAM, "#STR_COT_ESP_MODULE_MENU_COPY_STEAM", JMConstants.Lucide( "copy" ) );

		//! A context menu cannot be typed into, so the body comes off the
		//! clipboard - the same route the ESP menu takes for the same reason.
		m_Menu.AddItem( ROW_MENU_MESSAGE, "#STR_COT_ESP_MODULE_MENU_MESSAGE", JMConstants.Lucide( "message-square" ) );

		//! Destructive, and tinted like it. These four are the reason the row
		//! menu is worth having at all - an admin dealing with someone should
		//! not have to change tab to act.
		m_Menu.AddItem( ROW_MENU_CLEARCARGO, "#STR_COT_PLAYER_MODULE_ACTION_CLEAR_CARGO", JMConstants.Lucide( "package-x" ), JMTheme.DANGER );
		m_Menu.AddItem( ROW_MENU_STRIP,      "#STR_COT_PLAYER_MODULE_ACTION_STRIP",       JMConstants.Lucide( "shirt" ),     JMTheme.DANGER );

		if ( !isSelf )
		{
			m_Menu.AddItem( ROW_MENU_KICK, "#STR_COT_PLAYER_MODULE_ACTION_KICK", JMConstants.Lucide( "door-open" ), JMTheme.DANGER );
			m_Menu.AddItem( ROW_MENU_BAN,  "#STR_COT_PLAYER_MODULE_ACTION_BAN",  JMConstants.Lucide( "gavel" ),     JMTheme.DANGER );
		}

		JMContextMenuRegistry.Populate( "PlayerRoster", m_Menu );

		//! SetItemEnabled on an id the menu is not carrying is a no-op, so the
		//! two self-only omissions above need no special case here.
		m_Menu.SetItemEnabled( ROW_MENU_HEAL,     JMPermissions.Has( JMConstants.PERM_PLAYER_HEAL ) );
		m_Menu.SetItemEnabled( ROW_MENU_TP_TO_ME, JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION ) );
		m_Menu.SetItemEnabled( ROW_MENU_TP_ME_TO, JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_SENDERTO ) );
		m_Menu.SetItemEnabled( ROW_MENU_TP_UNDO,  JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION ) && JMTeleportHistory.Has( JMTeleportHistory.PlayerKey( instance.GetGUID() ) ) );
		m_Menu.SetItemEnabled( ROW_MENU_TP_REDO,  JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION ) && JMTeleportHistory.HasRedo( JMTeleportHistory.PlayerKey( instance.GetGUID() ) ) );
		m_Menu.SetItemEnabled( ROW_MENU_MESSAGE,    JMPermissions.Has( JMConstants.PERM_PLAYER_MESSAGE ) );
		m_Menu.SetItemEnabled( ROW_MENU_CLEARCARGO, JMPermissions.Has( JMConstants.PERM_PLAYER_CLEARCARGO ) );
		m_Menu.SetItemEnabled( ROW_MENU_STRIP,      JMPermissions.Has( JMConstants.PERM_PLAYER_STRIP ) );
		m_Menu.SetItemEnabled( ROW_MENU_KICK,       JMPermissions.Has( JMConstants.PERM_PLAYER_KICK ) );
		m_Menu.SetItemEnabled( ROW_MENU_BAN,        JMPermissions.Has( JMConstants.PERM_PLAYER_BAN ) );
		m_Menu.SetItemEnabled( ROW_MENU_FREEZE,   JMPermissions.Has( JMConstants.PERM_PLAYER_FREEZE ) );
#ifndef DAYZ_1_29
		m_Menu.SetItemEnabled( ROW_MENU_RAGDOLL,  JMPermissions.Has( JMConstants.PERM_PLAYER_RAGDOLL ) );
#endif
		m_Menu.SetItemEnabled( ROW_MENU_SPECTATE, JMPermissions.Has( JMConstants.PERM_PLAYER_SPECTATE ) );

		SetStatusRepairPermissions( m_Menu );

		JMScriptInvokers.ON_PLAYER_CONTEXT_MENU.Invoke( m_Menu, instance, guid );

		m_Menu.OpenAt( x, y );
	}

	//! Opens a text-entry prompt for one player, same as the ESP module's own
	//! player menu already does. Used to read from the clipboard instead - "a
	//! context menu cannot be typed into" - but a context menu can raise a
	//! confirmation dialog just fine, which is exactly what JMESPForm's
	//! PromptSendMessage does, so this matches it instead of asking the admin
	//! to pre-copy the message text before right-clicking.
	protected void RunRowMessage( string guid )
	{
		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_MESSAGE ) )
			return;

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( guid );
		if ( !instance )
			return;

		m_PendingRowMsgGuid = guid;

		m_Form.PromptInput( "#STR_COT_PLAYER_MODULE_MESSAGE_HEADER", "#STR_COT_ESP_MODULE_MESSAGE_PROMPT" + " " + instance.GetName() + ":", "OnRowMessage_Confirm" );
	}

	void OnRowMessage_Confirm( JMConfirmation confirmation = NULL )
	{
		if ( !confirmation )
			return;

		string msgText = confirmation.GetEditBoxValue();
		if ( msgText == "" || m_PendingRowMsgGuid == "" )
			return;

		m_Form.m_Module.DoMessage( { m_PendingRowMsgGuid }, msgText );
		m_PendingRowMsgGuid = "";
	}

	//! The repair entries for whatever is currently wrong with a player, added
	//! to `menu` in the order the list badges use. A healthy player contributes
	//! nothing, which is the point: the menu is a list of what can be fixed.
	//!
	//! A corpse gets none of them - reviving is not one of these calls, and
	//! stopping the bleeding of a dead man is not an action worth offering.
	protected void AddStatusRepairItems( UIActionContextMenu menu, JMPlayerInstance instance )
	{
		if ( !menu || !instance || instance.IsDead() )
			return;

		if ( instance.IsUnconscious() )
			AddStatusRepairItem( menu, ROW_MENU_WAKE );

		if ( instance.HasBrokenLegs() )
			AddStatusRepairItem( menu, ROW_MENU_FIX_LEGS );

		if ( instance.IsBleeding() )
			AddStatusRepairItem( menu, ROW_MENU_STOP_BLEED );

		if ( instance.IsSick() )
			AddStatusRepairItem( menu, ROW_MENU_CURE );
	}

	//! One repair entry, so the row menu and the status glyphs word the same fix
	//! the same way. An unknown id adds nothing.
	void AddStatusRepairItem( UIActionContextMenu menu, string id )
	{
		if ( !menu )
			return;

		if ( id == ROW_MENU_WAKE )
			menu.AddItem( ROW_MENU_WAKE, "#STR_COT_PLAYER_MODULE_ACTION_WAKE", JMConstants.Lucide( "bed" ) );
		else if ( id == ROW_MENU_FIX_LEGS )
			menu.AddItem( ROW_MENU_FIX_LEGS, "#STR_COT_PLAYER_MODULE_ACTION_FIX_LEGS", JMConstants.Lucide( "bone-fracture" ) );
		else if ( id == ROW_MENU_STOP_BLEED )
			menu.AddItem( ROW_MENU_STOP_BLEED, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_STOP_BLEEDING", JMConstants.Lucide( "droplet" ) );
		else if ( id == ROW_MENU_CURE )
			menu.AddItem( ROW_MENU_CURE, "#STR_COT_PLAYER_MODULE_ACTION_CURE", JMConstants.Lucide( "thermometer" ) );
	}

	//! Run one status repair against one guid. Shared by the player-list row
	//! menu and the Active Statuses card, which offer the same four fixes.
	//! Returns false for an id that is not a repair so the caller can go on
	//! matching its own entries.
	bool RunStatusRepair( string id, string guid )
	{
		if ( guid == "" || !m_Form.m_Module )
			return false;

		if ( id == ROW_MENU_WAKE )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_SET_SHOCK ) )
				return true;

			m_Form.UpdateLastChangeTime();
			m_Form.m_Module.SetShock( JMPlayerFormTabGeneral.SHOCK_CONSCIOUS, { guid } );

			return true;
		}

		if ( id == ROW_MENU_FIX_LEGS )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_BROKENLEGS ) )
				return true;

			m_Form.UpdateLastChangeTime();
			m_Form.m_Module.SetBrokenLegs( false, { guid } );

			return true;
		}

		if ( id == ROW_MENU_STOP_BLEED )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_BLEED_ADD ) )
				return true;

			m_Form.UpdateLastChangeTime();
			m_Form.m_Module.StopBleeding( { guid } );

			return true;
		}

		if ( id == ROW_MENU_CURE )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_DISEASE_REMOVE ) )
				return true;

			m_Form.UpdateLastChangeTime();
			m_Form.m_Module.RemoveAllDiseases( { guid } );

			return true;
		}

		return false;
	}

	//! Every branch addresses m_Guid alone. None of these go through
	//! the confirmation dialog the equivalent buttons use: a context menu IS the
	//! confirmation - it was opened on one named row and names the action.
	void OnClick_PlayerMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_Menu || !m_Form || !m_Form.m_Module )
		{
			Error("[JMPlayerRowMenu] OnClick_PlayerMenu failed: m_Menu, m_Form, or m_Form.m_Module is null!");
			return;
		}

		string id   = m_Menu.GetLastClickedId();
		string guid = m_Guid;

		if ( guid == "" )
			return;

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( guid );
		if ( !instance )
			return;

		//! Enforce has no block scope, so everything the branches need is
		//! declared once here rather than inside the branch that uses it.
		vector toMe = vector.Zero;
		PlayerBase spectated;

		if ( RunStatusRepair( id, guid ) )
			return;

		if ( id == ROW_MENU_HEAL )
		{
			if ( JMPermissions.Has( JMConstants.PERM_PLAYER_HEAL ) )
				m_Form.m_Module.Heal( { guid } );

			return;
		}

		if ( id == ROW_MENU_TP_TO_ME )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION ) )
				return;

			toMe = GetCurrentPosition();

			if ( toMe != vector.Zero )
				m_Form.m_Module.TeleportTo( toMe, { guid } );

			return;
		}

		if ( id == ROW_MENU_TP_REDO )
		{
			JMTeleportHistory.RedoPlayer( guid, 0 );

			return;
		}

		if ( id == ROW_MENU_FOCUS_MAP )
		{
			JMPlayerInstance focusInstance = GetPermissionsManager().GetPlayer( guid );

			if ( focusInstance )
				if ( m_Form.m_TabPositionCtrl )
					m_Form.m_TabPositionCtrl.FocusMap( focusInstance.GetPosition() );

			return;
		}

		if ( id == ROW_MENU_COPY_GUID )
		{
			COTFeedback.Copy( guid );

			return;
		}

		if ( id == ROW_MENU_COPY_STEAM )
		{
			JMPlayerInstance steamInstance = GetPermissionsManager().GetPlayer( guid );

			if ( steamInstance )
				COTFeedback.Copy( steamInstance.GetSteam64ID() );

			return;
		}

		if ( id == ROW_MENU_MESSAGE )
		{
			RunRowMessage( guid );

			return;
		}

		if ( id == ROW_MENU_CLEARCARGO )
		{
			if ( JMPermissions.Has( JMConstants.PERM_PLAYER_CLEARCARGO ) )
				m_Form.m_Module.ClearCargo( { guid } );

			return;
		}

		if ( id == ROW_MENU_STRIP )
		{
			if ( JMPermissions.Has( JMConstants.PERM_PLAYER_STRIP ) )
				m_Form.m_Module.Strip( { guid } );

			return;
		}

		if ( id == ROW_MENU_KICK )
		{
			if ( JMPermissions.Has( JMConstants.PERM_PLAYER_KICK ) )
				m_Form.m_Module.Kick( { guid }, "" );

			return;
		}

		if ( id == ROW_MENU_BAN )
		{
			if ( JMPermissions.Has( JMConstants.PERM_PLAYER_BAN ) )
				m_Form.m_Module.Ban( { guid }, "" );

			return;
		}

		if ( id == ROW_MENU_TP_UNDO )
		{
			//! One step per use, so repeated right-clicks walk back through the
			//! whole history rather than needing five separate menu entries.
			JMTeleportHistory.UndoPlayer( guid, 0 );

			return;
		}

		if ( id == ROW_MENU_TP_ME_TO )
		{
			if ( JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_SENDERTO ) )
				m_Form.m_Module.TeleportSenderTo( guid );

			return;
		}

		if ( id == ROW_MENU_FREEZE )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_FREEZE ) )
				return;

			m_Form.UpdateLastChangeTime();
			m_Form.m_Module.SetFreeze( !instance.IsFrozen(), { guid } );

			return;
		}

#ifndef DAYZ_1_29
		if ( id == ROW_MENU_RAGDOLL )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_RAGDOLL ) )
				return;

			m_Form.UpdateLastChangeTime();
			m_Form.m_Module.SetRagdoll( !instance.IsRagdoll(), { guid } );

			return;
		}
#endif

		if ( id == ROW_MENU_SPECTATE )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_SPECTATE ) )
				return;

			spectated = instance.PlayerObject;
			m_Form.m_Module.Click_Spectate( action, spectated, guid );
		}
	}
}
