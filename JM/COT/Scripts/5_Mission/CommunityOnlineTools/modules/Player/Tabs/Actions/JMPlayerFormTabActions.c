//! "Actions" tab of JMPlayerForm - state toggles, disease/bleeding controls and
//! the one-shot commands (heal, kill, strip, kick, ban ...) for the selected
//! player. Back-reference to the owning form, same shape as JMPlayerRowWidget.Menu.
//!
//! Confirmation callbacks (the Multi/Single/Self trios) are looked up by name on
//! the FORM, so each keeps a thin forwarder there.
class JMPlayerFormTabActions: JMFormTab
{
	protected JMPlayerForm m_Form;

	//! Scale prompt, lazily built the first time it is needed - see m_PlayerMenu.
	//! One shared slider serves all three routing modes; which target set the
	//! next confirm applies to is remembered here rather than baked into three
	//! separate prompt ids.
	UIActionValuePrompt m_ScalePrompt;
	int                 m_ScalePendingMode;

	//! One checkbox per JMPlayerToggle the module registers, by toggle id. Built from the
	//! registry, so a toggle a mod adds gets its checkbox, permission gate and repaint
	//! without this class knowing about it.
	protected ref map< string, UIActionCheckbox > m_ToggleChecks = new map< string, UIActionCheckbox >;
	UIActionButton m_RepairTransport;
	UIActionButton m_SpectatePlayer;
	UIActionButton m_HealPlayer;
	UIActionButton m_StripPlayer;
	UIActionButton m_DryPlayer;
	UIActionButton m_VomitPlayer;
	UIActionButton m_SetScalePlayer;

	// Disease Add/Remove controls (status-aware dropdown)
	UIActionDropdown m_DiseaseAgent;
	UIActionButton m_DiseaseAdd;
	UIActionButton m_DiseaseRemove;
	UIActionImageButton m_DiseaseClear;
	ref array< int > m_DiseaseAgentIds;

	// Bleed-from-body-part controls
	UIActionDropdown m_BleedingPart;
	UIActionButton m_BleedApply;
	UIActionImageButton m_BleedClear;
	UIActionFeedbackButton m_CopyExpLoadout;
	UIActionButton m_KillPlayer;
	UIActionButton m_SendMessage;
	UIActionButton m_KickPlayer;
	UIActionButton m_BanPlayer;
	static const int SCALE_MODE_MULTI  = 0;
	static const int SCALE_MODE_SINGLE = 1;
	static const int SCALE_MODE_SELF   = 2;

	// Default inject count - high enough to cross every disease's activation threshold
	const int DISEASE_INJECT_COUNT = 200;

	//! Direct children this tab's builder adds to its panel, with headroom. Over-
	//! provisioning is free under Size-To-Content-V; running short silently clips the
	//! last rows with no error anywhere. The form sizes the panel from this.
	static const int PANEL_ROWS = 14;

	void JMPlayerFormTabActions( JMPlayerForm form )
	{
		m_Form = form;
	}

	//! The checkbox for a toggle id, or NULL when it has none (a toggle without a UI, or one
	//! not registered).
	UIActionCheckbox GetToggleCheckbox( string id )
	{
		return m_ToggleChecks.Get( id );
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		InitActionWidgetsFlags( panel );
		InitActionWidgetsDiseases( panel );
		InitActionWidgetsQuick( panel );
	}

	override void OnFocus()
	{
		if ( m_Form.m_LastSelectedGuid != "" )
		{
			m_Form.m_Module.RequestDiseaseMask( m_Form.m_LastSelectedGuid );
			m_Form.m_Module.RequestBleedingState( m_Form.m_LastSelectedGuid );
		}
	}

	//! The state toggles, repainted from the selected player.
	void RefreshFlags()
	{
		array< ref JMPlayerToggle > registeredToggles = m_Form.m_Module.GetToggles();
		foreach ( JMPlayerToggle toggle : registeredToggles )
		{
			UIActionCheckbox checkbox = GetToggleCheckbox( toggle.GetId() );
			if ( checkbox )
				checkbox.SetChecked( toggle.Read( m_Form.m_SelectedInstance ) != 0 );
		}
	}

	void BindPermissions()
	{
		array< ref JMPlayerToggle > registeredToggles = m_Form.m_Module.GetToggles();
		foreach ( JMPlayerToggle toggle : registeredToggles )
			m_Form.BindPermission( GetToggleCheckbox( toggle.GetId() ), toggle.GetPermission() );

		m_Form.BindPermission( m_HealPlayer,            JMConstants.PERM_PLAYER_HEAL                  );
		m_Form.BindPermission( m_RepairTransport,       JMConstants.PERM_TRANSPORT_REPAIR             );
		m_Form.BindPermission( m_DryPlayer,             JMConstants.PERM_PLAYER_DRY                   );
		m_Form.BindPermission( m_SpectatePlayer,        JMConstants.PERM_PLAYER_SPECTATE              );
		m_Form.BindPermission( m_VomitPlayer,           JMConstants.PERM_PLAYER_VOMIT                 );
		m_Form.BindPermission( m_SetScalePlayer,        JMConstants.PERM_PLAYER_SCALE                 );
		m_Form.BindPermission( m_KillPlayer,            JMConstants.PERM_PLAYER_SET_HEALTH            );
		m_Form.BindPermission( m_StripPlayer,           JMConstants.PERM_PLAYER_STRIP                 );
		m_Form.BindPermission( m_KickPlayer,            JMConstants.PERM_PLAYER_KICK                  );
		m_Form.BindPermission( m_BanPlayer,             JMConstants.PERM_PLAYER_BAN                   );

		m_Form.BindPermission( m_DiseaseAgent,          JMConstants.PERM_PLAYER_DISEASE_ADD           );
		m_Form.BindPermission( m_DiseaseAdd,            JMConstants.PERM_PLAYER_DISEASE_ADD           );
		m_Form.BindPermission( m_DiseaseRemove,         JMConstants.PERM_PLAYER_DISEASE_REMOVE        );
		m_Form.BindPermission( m_DiseaseClear,          JMConstants.PERM_PLAYER_DISEASE_REMOVE        );
		m_Form.BindPermission( m_BleedingPart,          JMConstants.PERM_PLAYER_BLEED_ADD             );
		m_Form.BindPermission( m_BleedApply,            JMConstants.PERM_PLAYER_BLEED_ADD             );
		m_Form.BindPermission( m_BleedClear,            JMConstants.PERM_PLAYER_STOPBLEEDING          );
	}

	void DisableForOffline()
	{
		if ( m_SpectatePlayer )
			m_SpectatePlayer.Disable();
	}

	//! The state toggles - one checkbox per JMPlayerToggle in the module's registry. Split out
	//! of InitActionWidgetsStats so they can live on the Actions tab next to kick and ban,
	//! which is where an admin looks for them. Expansion never touched this grid, only the
	//! slider one.
	Widget InitActionWidgetsFlags( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_FLAGS" );
		Widget section0 = section0Card.GetContent();

		array< JMPlayerToggle > shown = {};

		array< ref JMPlayerToggle > registeredToggles = m_Form.m_Module.GetToggles();
		foreach ( JMPlayerToggle toggle : registeredToggles )
		{
			if ( toggle.IsInUI() )
				shown.Insert( toggle );
		}

		//! Two columns; an odd count leaves the last cell empty rather than truncating.
		Widget actions2 = UIActionManager.CreateGridSpacer( section0, ( shown.Count() + 1 ) / 2, 2 );

		foreach ( JMPlayerToggle shownToggle : shown )
		{
			UIActionCheckbox checkbox = UIActionManager.CreateCheckbox( actions2, shownToggle.GetCheckboxKey(), this, "OnClick_Toggle", false, 1, JMConstants.Lucide( shownToggle.GetIcon() ) );
			checkbox.SetTooltip( shownToggle.GetTooltipKey() );
			checkbox.SetUserData( shownToggle );

			m_ToggleChecks.Set( shownToggle.GetId(), checkbox );
		}

		return parent;
	}

	Widget InitActionWidgetsDiseases( Widget actionsParent )
	{
		// disease header + dropdown + buttons + bleed header + dropdown + apply
		// + divider
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 8, 1 );

		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_DISEASES_HEADER" );
		Widget section0 = section0Card.GetContent();
		// Static agent id list mirrors PluginTransmissionAgents eAgents bitmask
		m_DiseaseAgentIds = {
			eAgents.CHOLERA,
			eAgents.INFLUENZA,
			eAgents.SALMONELLA,
			eAgents.BRAIN,
			eAgents.FOOD_POISON,
			eAgents.CHEMICAL_POISON,
			eAgents.WOUND_AGENT,
			eAgents.NERVE_AGENT,
			eAgents.HEAVYMETAL
		};

		// Dropdown shows each disease with a green or red icon depending on whether the
		// currently selected player is infected. Click "Add" to inject, "Remove" to clear.
		m_DiseaseAgent = UIActionManager.CreateDropdown( section0, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_DISEASES_AGENT", m_Form.GetWindow().GetWidgetRoot(), this, "" );
		m_Form.AddOverlay( m_DiseaseAgent );
		m_DiseaseAgent.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_DISEASE_AGENT" );

		// Default-state rows: all green (no data until selection pushes mask)
		RebuildDiseaseDropdown( 0, 0, 0, 0, 0, 0, 0, 0, 0 );

		// Add and Remove act on whichever agent the dropdown is showing, so they
		// stay in the body under it. Clear-all does not read the dropdown at all
		// - it wipes the card - so it moves up into the title bar with the other
		// card-level destructive actions, and keeps the caption it lost as the
		// tooltip an icon-only button needs.
		m_DiseaseClear = section0Card.AddDeleteButton( this, "Click_ClearAllDiseases" );
		m_DiseaseClear.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_DISEASE_CLEAR" );
		m_DiseaseClear.SetColor( JMTheme.DANGER_FILL );

		Widget actions = UIActionManager.CreateGridSpacer( section0, 1, 2 );
		m_DiseaseAdd = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_DISEASE_ADD", this, "Click_AddDisease" );
		m_DiseaseAdd.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_DISEASE_ADD" );
		m_DiseaseRemove = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_DISEASE_REMOVE", this, "Click_RemoveDisease" );
		m_DiseaseRemove.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_DISEASE_REMOVE" );

		// ---- Bleed-from-body-part ----
		UIActionCard section1Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_BLEEDING" );
		Widget section1 = section1Card.GetContent();

		// Stop-all does not read the part dropdown - it clears the whole card -
		// so it belongs in the title bar next to the other card-level wipes,
		// exactly where Diseases puts its own. Bound straight to the existing
		// Click_StopBleeding: "stop every bleed on this player" already had a
		// handler and a confirmation, it just had no button on this card.
		m_BleedClear = section1Card.AddDeleteButton( this, "Click_StopBleeding" );
		m_BleedClear.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_BLEED_CLEAR" );
		m_BleedClear.SetColor( JMTheme.DANGER_FILL );

		m_BleedingPart = UIActionManager.CreateDropdown( section1, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_BLEEDING_PART", m_Form.GetWindow().GetWidgetRoot(), this, "" );
		m_Form.AddOverlay( m_BleedingPart );
		m_BleedingPart.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_BLEEDING_PART" );

		// Populate with zone list (default fallback entries until server push arrives)
		m_BleedingPart.AddEntry( BleedPartLabel( "All" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "Head" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "Torso" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "LeftArm" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "RightArm" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "LeftLeg" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "RightLeg" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "LeftFoot" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.AddEntry( BleedPartLabel( "RightFoot" ), JMConstants.ICON_HEALTH_NORMAL, ARGB(255, 73, 184, 117) );
		m_BleedingPart.SetSelection( 0, false );

		Widget bleedActions = UIActionManager.CreateGridSpacer( section1, 1, 1 );
		m_BleedApply = UIActionManager.CreateButton( bleedActions, "#STR_COT_PLAYER_MODULE_BLEED_APPLY", this, "Click_ApplyBleed" );
		m_BleedApply.SetWidth( 1.0 );
		m_BleedApply.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_BLEED_APPLY" );

		return parent;
	}

	Widget InitActionWidgetsQuick( Widget actionsParent )
	{
		// header + friendly + gap + misc + gap + danger header + danger + divider
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 9, 1 );

		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_HEADER" );
		Widget section0 = section0Card.GetContent();
		// Friendly actions at the top - 3 buttons in one row (Stop Bleeding moved to per-part Bleed dropdown)
		Widget actionsFriendly = UIActionManager.CreateGridSpacer( section0, 1, 3 );
		m_HealPlayer = UIActionManager.CreateButton( actionsFriendly, "#STR_COT_PLAYER_MODULE_ACTION_HEAL",    this, "Click_HealPlayer"    );
		m_HealPlayer.SetWidth( 0.33 );
		m_HealPlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_HEAL" );
		m_RepairTransport = UIActionManager.CreateButton( actionsFriendly, "#STR_COT_PLAYER_MODULE_ACTION_REPAIR", this, "Click_RepairTransport" );
		m_RepairTransport.SetWidth( 0.33 );
		m_RepairTransport.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_REPAIR" );
		m_DryPlayer = UIActionManager.CreateButton( actionsFriendly, "#STR_COT_PLAYER_MODULE_ACTION_DRY",     this, "Click_DryPlayer"     );
		m_DryPlayer.SetWidth( 0.33 );
		m_DryPlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_DRY" );

		// Misc actions inbetween - 4 or 5 buttons; #ifdef adds a row if Expansion is loaded
		UIActionManager.CreateSpacerPx( section0, 10 );
	#ifdef DZ_Expansion_Core
		Widget actionsMisc = UIActionManager.CreateGridSpacer( section0, 3, 2 );
	#else
		Widget actionsMisc = UIActionManager.CreateGridSpacer( section0, 2, 2 );
	#endif
		m_SpectatePlayer = UIActionManager.CreateButton( actionsMisc, "#STR_COT_PLAYER_MODULE_ACTION_SPECTATE", this, "Click_SpectatePlayer" );
		m_SpectatePlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_SPECTATE" );
		m_SendMessage 	 = UIActionManager.CreateButton( actionsMisc, "#STR_COT_PLAYER_MODULE_ACTION_MESSAGE",  this, "Click_SendMessage"    );
		m_SendMessage.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_MESSAGE" );
		m_VomitPlayer 	 = UIActionManager.CreateButton( actionsMisc, "#STR_COT_PLAYER_MODULE_ACTION_VOMIT",  this, "Click_VomitPlayer"    );
		m_VomitPlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_VOMIT" );
		m_SetScalePlayer = UIActionManager.CreateButton( actionsMisc, "#STR_COT_PLAYER_MODULE_ACTION_SCALE",  this, "Click_ScalePlayer"    );
		m_SetScalePlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_SCALE" );
	#ifdef DZ_Expansion_Core
		m_CopyExpLoadout = UIActionManager.CreateFeedbackButton( actionsMisc, "Exp.Loadout", "#STR_COT_COPIED", JMConstants.ICON_CHECK_MARK, this, "Click_CopyExpLoadout" );
		// This one can fail, so the swap is fired from the callback instead.
		m_CopyExpLoadout.SetAutoFeedback( false );
	#endif

		// Destructive actions at the bottom - 4 buttons in 2 rows x 2 cols
		UIActionManager.CreateSpacerPx( section0, 10 );
		UIActionCard section1Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_DANGER" );
		Widget section1 = section1Card.GetContent();
		section1Card.SetRingColor( JMTheme.DANGER );
		Widget actionsDestructive = UIActionManager.CreateGridSpacer( section1, 2, 2 );
		m_KillPlayer  = UIActionManager.CreateButton( actionsDestructive, "#STR_COT_PLAYER_MODULE_ACTION_KILL",  this, "Click_KillPlayer"  );
		m_KillPlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_KILL" );
		m_StripPlayer = UIActionManager.CreateButton( actionsDestructive, "#STR_COT_PLAYER_MODULE_ACTION_STRIP", this, "Click_StripPlayer" );
		m_StripPlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_STRIP" );
		m_KickPlayer  = UIActionManager.CreateButton( actionsDestructive, "#STR_COT_PLAYER_MODULE_ACTION_KICK",  this, "Click_KickPlayer"  );
		m_KickPlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_KICK" );
		m_BanPlayer   = UIActionManager.CreateButton( actionsDestructive, "#STR_COT_PLAYER_MODULE_ACTION_BAN",   this, "Click_BanPlayer"   );
		m_BanPlayer.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_BAN" );

		m_KillPlayer.SetColor(JMTheme.DANGER_FILL);
		m_StripPlayer.SetColor(JMTheme.DANGER_FILL);
		m_KickPlayer.SetColor(JMTheme.DANGER_FILL);
		m_BanPlayer.SetColor(JMTheme.DANGER_FILL);

		return parent;
	}

	void Click_StripPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.CreateConfirmation_Three( JMConfirmationType.INFO, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_CLEAR_INVENTORY", "", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_EVERYTHING", "StripConfirm", "#STR_COT_CARGO_ONLY", "ClearCargoConfirm" );
	}

	void StripConfirm(JMConfirmation confirmation = NULL)
	{
		m_Form.ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_CLEAR_INVENTORY", this, "StripTargets" );
	}

	void StripTargets( array<string> targets, JMConfirmation confirmation = NULL )
	{
		m_Form.m_Module.Strip( targets );
	}

	void ClearCargoConfirm(JMConfirmation confirmation = NULL)
	{
		m_Form.ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_CLEAR_INVENTORY", this, "ClearCargoTargets" );
	}

	void ClearCargoTargets( array<string> targets, JMConfirmation confirmation = NULL )
	{
		m_Form.m_Module.ClearCargo( targets );
	}

	void Click_DryPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_DRY", this, "DryTargets", false );
	}

	void DryTargets( array<string> targets, JMConfirmation confirmation = NULL )
	{
		m_Form.m_Module.Dry( targets );
	}

	void Click_KillPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_KILL", this, "KillPlayerTargets" );
	}

	void KillPlayerTargets( array<string> targets, JMConfirmation confirmation = NULL )
	{
		m_Form.m_Module.SetHealth( 0, targets );
	}

	void Click_VomitPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		m_Form.CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_VOMIT", "VomitPlayerMultiConfirm", "VomitPlayerSingleConfirm", "VomitPlayerSelfConfirm");
	}

	void VomitPlayerMultiConfirm()
	{
		m_Form.PromptInput( "#STR_COT_PLAYER_MODULE_SET_VOMIT_DURATION_HEADER", "#STR_COT_PLAYER_MODULE_SET_VOMIT_DURATION_BODY", "VomitPlayerMulti" );
	}

	void VomitPlayerSingleConfirm()
	{
		m_Form.PromptInput( "#STR_COT_PLAYER_MODULE_SET_VOMIT_DURATION_HEADER", "#STR_COT_PLAYER_MODULE_SET_VOMIT_DURATION_BODY", "VomitPlayerSingle" );
	}

	void VomitPlayerSelfConfirm()
	{
		m_Form.PromptInput( "#STR_COT_PLAYER_MODULE_SET_VOMIT_DURATION_HEADER", "#STR_COT_PLAYER_MODULE_SET_VOMIT_DURATION_BODY", "VomitPlayerSelf" );
	}

	void VomitPlayerMulti(JMConfirmation confirmation)
	{
		float value;
		if (confirmation.GetEditBoxValueFloat(value, 1, 120))
			m_Form.m_Module.Vomit(value, JM_GetSelected().GetPlayers() );
	}

	void VomitPlayerSingle(JMConfirmation confirmation)
	{
		float value;
		if (confirmation.GetEditBoxValueFloat(value, 1, 120))
			m_Form.m_Module.Vomit(value, {JM_GetSelected().GetPlayers()[0]} );
	}

	void VomitPlayerSelf(JMConfirmation confirmation)
	{
		float value;
		if (confirmation.GetEditBoxValueFloat(value, 1, 120))
			m_Form.m_Module.Vomit(value, {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

	void Click_ScalePlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_SETSCALE", "ScalePlayerMultiConfirm", "ScalePlayerSingleConfirm", "ScalePlayerSelfConfirm", false);
	}

	void ScalePlayerMultiConfirm()
	{
		ShowScalePrompt( SCALE_MODE_MULTI );
	}

	void ScalePlayerSingleConfirm()
	{
		ShowScalePrompt( SCALE_MODE_SINGLE );
	}

	void ScalePlayerSelfConfirm()
	{
		ShowScalePrompt( SCALE_MODE_SELF );
	}

	//! Lazily built the first time it is needed, exactly like m_PlayerMenu -
	//! nothing before this ever needed a scale slider.
	void ShowScalePrompt( int mode )
	{
		if ( !m_ScalePrompt )
		{
			if ( !m_Form.GetWindow() )
				return;

			m_ScalePrompt = UIActionManager.CreateOverlayPrompt( m_Form, this, "OnConfirm_ScalePrompt" );

			if ( !m_ScalePrompt )
				return;
		}

		m_ScalePendingMode = mode;

		// Range matches the server's own clamp in Exec_SetScale/RPC_SetScale -
		// a slider that could ask for more than the server will ever apply
		// would just teach the wrong ceiling.
		m_ScalePrompt.OpenSlider( "scale", "#STR_COT_PLAYER_MODULE_SET_SCALE_HEADER", "#STR_COT_PLAYER_MODULE_ACTION_SCALE", 0.1, 10, 1.0, 0.05, "%1" );
	}

	void OnConfirm_ScalePrompt( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_ScalePrompt )
			return;

		if ( m_ScalePrompt.GetPromptId() != "scale" )
			return;

		float value = m_ScalePrompt.GetSliderValue();

		if ( m_ScalePendingMode == SCALE_MODE_MULTI )
			m_Form.m_Module.SetScale( value, JM_GetSelected().GetPlayers() );
		else if ( m_ScalePendingMode == SCALE_MODE_SINGLE )
			m_Form.m_Module.SetScale( value, {JM_GetSelected().GetPlayers()[0]} );
		else
			m_Form.m_Module.SetScale( value, {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

	void Click_SendMessage( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_SEND_MESSAGE", "SendMessageMultiConfirm", "SendMessageSingleConfirm", "SendMessageSelfConfirm", false);
	}

	void SendMessageMultiConfirm()
	{
		m_Form.CreateConfirmation_Three( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_CONFIRM_MESSAGE", "SendMessageMulti", "#STR_COT_CONFIRM_NOTIFICATION", "SendNotifMulti" );
	}

	void SendMessageSingleConfirm()
	{
		m_Form.CreateConfirmation_Three( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_CONFIRM_MESSAGE", "SendMessageSingle", "#STR_COT_CONFIRM_NOTIFICATION", "SendNotifSingle" );
	}

	void SendMessageSelfConfirm()
	{
		m_Form.CreateConfirmation_Three( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_CONFIRM_MESSAGE", "SendMessageSelf", "#STR_COT_CONFIRM_NOTIFICATION", "SendNotifSelf" );
	}

	void SendMessageMulti(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text != "" )
			m_Form.m_Module.DoMessage( JM_GetSelected().GetPlayers(), text);
	}

	void SendMessageSingle(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text != "" )
			m_Form.m_Module.DoMessage( {JM_GetSelected().GetPlayers(true)[0]}, text);
	}

	void SendMessageSelf(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text != "" )
			m_Form.m_Module.DoMessage( {GetPermissionsManager().GetClientPlayer().GetGUID()}, text);
	}

	void SendNotifMulti(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text != "" )
			m_Form.m_Module.DoNotif( JM_GetSelected().GetPlayers(), text);
	}

	void SendNotifSingle(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text != "" )
			m_Form.m_Module.DoNotif( {JM_GetSelected().GetPlayers(true)[0]}, text);
	}

	void SendNotifSelf(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text != "" )
			m_Form.m_Module.DoNotif( {GetPermissionsManager().GetClientPlayer().GetGUID()}, text);
	}

	void Click_CopyExpLoadout( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

	#ifdef DZ_Expansion_Core
		JMESPModule module = CF_Modules<JMESPModule>.Get();
		if (module.CopyToClipboardExpLoadout(JMPlayerInstance))
			m_CopyExpLoadout.ShowFeedback();
	#endif
	}

	void Click_KickPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_KICK", "KickPlayerMultiConfirm", "KickPlayerSingleConfirm", "KickPlayerSelfConfirm", false);
	}

	void KickPlayerMultiConfirm(JMConfirmation confirmation = NULL)
	{
		m_Form.PromptInput( "#STR_COT_PLAYER_MODULE_KICK_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_KICK_MESSAGE_BODY", "KickPlayerMulti", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_KICK" );
	}

	void KickPlayerSingleConfirm(JMConfirmation confirmation = NULL)
	{
		m_Form.PromptInput( "#STR_COT_PLAYER_MODULE_KICK_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_KICK_MESSAGE_BODY", "KickPlayerSingle", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_KICK" );
	}

	void KickPlayerSelfConfirm(JMConfirmation confirmation = NULL)
	{
		COTCreateLocalAdminNotification( new StringLocaliser( "#STR_COT_PLAYER_MODULE_CANT_KICK_SELF" ) );
	}

	void KickPlayerMulti(JMConfirmation confirmation)
	{
		m_Form.m_Module.Kick( JM_GetSelected().GetPlayers(), confirmation.GetEditBoxValue() );
	}

	void KickPlayerSingle(JMConfirmation confirmation)
	{
		m_Form.m_Module.Kick( {JM_GetSelected().GetPlayers()[0]}, confirmation.GetEditBoxValue() );
	}

	void Click_BanPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_BAN", "BanPlayerMultiConfirm", "BanPlayerSingleConfirm", "BanPlayerSelfConfirm", false);
	}

	void BanPlayerMultiConfirm()
	{
		m_Form.PromptInput( "#STR_COT_PLAYER_MODULE_BAN_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_BAN_MESSAGE_BODY", "BanPlayerMulti", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_BAN" );
	}

	void BanPlayerSingleConfirm()
	{
		m_Form.PromptInput( "#STR_COT_PLAYER_MODULE_BAN_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_BAN_MESSAGE_BODY", "BanPlayerSingle", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_BAN" );
	}

	void BanPlayerSelfConfirm(JMConfirmation confirmation)
	{
		COTCreateLocalAdminNotification( new StringLocaliser( "#STR_COT_PLAYER_MODULE_CANT_BAN_SELF" ) );
	}

	void BanPlayerMulti(JMConfirmation confirmation)
	{
		m_Form.m_Module.Ban( JM_GetSelected().GetPlayers(), confirmation.GetEditBoxValue(), -1 );
	}

	void BanPlayerSingle(JMConfirmation confirmation)
	{
		m_Form.m_Module.Ban( {JM_GetSelected().GetPlayers()[0]}, confirmation.GetEditBoxValue(), -1 );
	}

	void Click_StopBleeding( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_STOP_BLEEDING", this, "StopBleedingTargets", false );
	}

	void StopBleedingTargets( array<string> targets, JMConfirmation confirmation = NULL )
	{
		m_Form.m_Module.StopBleeding( targets );
	}

	void Click_AddDisease( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		int agentIdx = m_DiseaseAgent.GetSelection();
		if ( agentIdx < 0 || agentIdx >= m_DiseaseAgentIds.Count() )
			return;

		m_Form.ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_DISEASES_ADD", this, "AddDiseaseTargets", false );
	}

	void AddDiseaseTargets( array<string> targets, JMConfirmation confirmation = NULL )
	{
		int agentIdx = m_DiseaseAgent.GetSelection();
		int agent = m_DiseaseAgentIds[agentIdx];
		m_Form.m_Module.AddDisease( agent, DISEASE_INJECT_COUNT, targets );
	}

	void Click_RemoveDisease( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		int agentIdx = m_DiseaseAgent.GetSelection();
		if ( agentIdx < 0 || agentIdx >= m_DiseaseAgentIds.Count() )
			return;

		m_Form.ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_DISEASES_REMOVE", this, "RemoveDiseaseTargets", false );
	}

	void RemoveDiseaseTargets( array<string> targets, JMConfirmation confirmation = NULL )
	{
		int agentIdx = m_DiseaseAgent.GetSelection();
		int agent = m_DiseaseAgentIds[agentIdx];
		m_Form.m_Module.RemoveDisease( agent, targets );
	}

	// ---------------- Bleed-from-body-part handlers ----------------

	void Click_ApplyBleed( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		string sel = m_BleedingPart.GetSelectedText();
		if ( sel == "" )
			return;

		if ( sel == "All" )
			m_Form.ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_BLEEDING_STOP", this, "ApplyBleedTargets", false );
		else
			m_Form.ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_BLEEDING_APPLY", this, "ApplyBleedTargets", false );
	}

	void ApplyBleedTargets( array<string> targets, JMConfirmation confirmation = NULL )
	{
		string sel = m_BleedingPart.GetSelectedText();
		if ( sel == "All" )
			m_Form.m_Module.StopBleeding( targets );
		else
			m_Form.m_Module.AddBleedingPart( sel, targets );
	}

	// ---------------- Disease dropdown rebuild ----------------

	void RebuildDiseaseDropdown( int cholera, int influenza, int salmonella, int brain, int foodPoison, int chemPoison, int wound, int nerve, int heavyMetal )
	{
		// Arrives from a server response, which does not wait for the Actions
		// tab to have been opened.
		if ( !m_DiseaseAgent )
			return;

		if ( !m_DiseaseAgent )
			return;

		// Don't rebuild while the user is interacting with the popup - clearing
		// and re-adding entries pops each row's widget creation in the dropdown
		// which gives the appearance of an auto-close. Defer until the next
		// user-triggered action.
		if ( m_DiseaseAgent.IsOpen() )
			return;

		// Cache selection so we can restore it after rebuild - otherwise
		// calling SetSelection() forces the dropdown closed even with sendEvent=false,
		// which makes the form feel like it's auto-refreshing the popup.
		int prevSel = m_DiseaseAgent.GetSelection();

		m_DiseaseAgent.ClearEntries();

		// (count, label, agentId)
		int counts[9];
		counts[0] = cholera;
		counts[1] = influenza;
		counts[2] = salmonella;
		counts[3] = brain;
		counts[4] = foodPoison;
		counts[5] = chemPoison;
		counts[6] = wound;
		counts[7] = nerve;
		counts[8] = heavyMetal;

		string labels[9];
		labels[0] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_CHOLERA";
		labels[1] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_INFLUENZA";
		labels[2] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_SALMONELLA";
		labels[3] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_BRAIN_KURU";
		labels[4] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_FOOD_POISONING";
		labels[5] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_CHEMICAL_POISONING";
		labels[6] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_WOUND_INFECTION";
		labels[7] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_NERVE_AGENT";
		labels[8] = "#STR_COT_PLAYER_MODULE_DISEASE_NAME_HEAVY_METAL_POISONING";

		int COLOR_OK = ARGB(255, 73, 184, 117);   // green = not infected
		int COLOR_BAD = ARGB(255, 217, 90, 90);  // red = infected

		for ( int i = 0; i < 9; i++ )
		{
			string icon;
			int colour;
			if ( counts[i] > 0 )
			{
				icon = JMConstants.ICON_HEALTH_DECREASE;
				colour = COLOR_BAD;
			}
			else
			{
				icon = JMConstants.ICON_HEALTH_NORMAL;
				colour = COLOR_OK;
			}

			m_DiseaseAgent.AddEntry( labels[i] + " (" + counts[i].ToString() + ")", icon, colour );
		}

		// Restore prior selection without firing CHANGE (which would also close the popup).
		// If prior selection was -1 or out of range, fall back to 0.
		int restore = prevSel;
		if ( restore < 0 || restore >= 9 )
			restore = 0;
		m_DiseaseAgent.SetSelection( restore, false );

		//! This arrives from a server RPC round-trip kicked off by OnChange_Tab,
		//! strictly after OnChange_Tab's own one-shot UpdateScroller() call has
		//! already measured and settled the Actions tab's content height. Nine
		//! freshly-added dropdown rows can change that height, so the scroller
		//! needs telling again or it's stuck showing the pre-response size
		//! (typically "no scrollbar yet" on the very first open, before this
		//! response has ever arrived).
		if ( m_Form.m_ActionListScroller )
			m_Form.m_ActionListScroller.UpdateScroller();
	}

	// Called from the module when the server pushes the selected player's mask.
	void OnDiseaseMaskUpdated( string guid, int cholera, int influenza, int salmonella, int brain, int foodPoison, int chemPoison, int wound, int nerve, int heavyMetal )
	{
		if ( guid == m_Form.m_LastSelectedGuid )
			RebuildDiseaseDropdown( cholera, influenza, salmonella, brain, foodPoison, chemPoison, wound, nerve, heavyMetal );
	}

	// Called from the module when the server pushes the selected player's
	// bleeding state. Rebuilds the body-part dropdown with red icons on the
	// parts that are actively bleeding.
	void OnBleedingStateUpdated( string guid, array< string > names, array< int > bits, int bleedingBits, array< int > activePartIndices )
	{
		if ( guid == m_Form.m_LastSelectedGuid )
			RebuildBleedDropdown( names, bits, activePartIndices );
	}

	//! Maps a raw bleed zone identifier - the static fallback list above, or a
	//! zone name the server pushes over RPC - to its localization key. A zone
	//! name this list does not recognize comes back unchanged rather than
	//! blanked, since the dropdown still has to show something for it.
	string BleedPartLabel( string zoneName )
	{
		if ( zoneName == "All" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_ALL";
		if ( zoneName == "Head" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_HEAD";
		if ( zoneName == "Torso" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_TORSO";
		if ( zoneName == "LeftArm" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_LEFT_ARM";
		if ( zoneName == "RightArm" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_RIGHT_ARM";
		if ( zoneName == "LeftLeg" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_LEFT_LEG";
		if ( zoneName == "RightLeg" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_RIGHT_LEG";
		if ( zoneName == "LeftFoot" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_LEFT_FOOT";
		if ( zoneName == "RightFoot" )
			return "#STR_COT_PLAYER_MODULE_BLEED_PART_RIGHT_FOOT";

		return zoneName;
	}

	void RebuildBleedDropdown( array< string > names, array< int > bits, array< int > activePartIndices )
	{
		if ( !m_BleedingPart )
			return;

		if ( !m_BleedingPart )
			return;

		int prevSel = m_BleedingPart.GetSelection();

		m_BleedingPart.ClearEntries();

		int COLOR_OK  = ARGB(255, 73, 184, 117);
		int COLOR_BAD = ARGB(255, 217, 90, 90);

		// First entry: "All" - green = no bleeders, red = bleeders present.
		bool anyBleeding = activePartIndices.Count() > 0;
		if ( anyBleeding )
			m_BleedingPart.AddEntry( BleedPartLabel( "All" ), JMConstants.ICON_BLEEDING_WOUND, COLOR_BAD );
		else
			m_BleedingPart.AddEntry( BleedPartLabel( "All" ), JMConstants.ICON_BLEEDING_WOUND, COLOR_OK );

		for ( int i = 0; i < names.Count(); i++ )
		{
			string nm = names.Get( i );
			bool active = false;
			for ( int a = 0; a < activePartIndices.Count(); a++ )
			{
				if ( activePartIndices.Get( a ) == i )
				{
					active = true;
					break;
				}
			}
			if ( active )
				m_BleedingPart.AddEntry( BleedPartLabel( nm ), JMConstants.ICON_BLEEDING_WOUND, COLOR_BAD );
			else
				m_BleedingPart.AddEntry( BleedPartLabel( nm ), JMConstants.ICON_BLEEDING_WOUND, COLOR_OK );
		}

		int restore = prevSel;
		int maxEntries = names.Count() + 1;
		if ( restore < 0 || restore >= maxEntries )
			restore = 0;
		m_BleedingPart.SetSelection( restore, false );

		//! Same reasoning as RebuildDiseaseDropdown's own trailing call - this
		//! also arrives after OnChange_Tab's scroller measurement already ran.
		if ( m_Form.m_ActionListScroller )
			m_Form.m_ActionListScroller.UpdateScroller();
	}

	void Click_ClearAllDiseases( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_DISEASES_CLEAR", this, "ClearAllDiseasesTargets", false );
	}

	void ClearAllDiseasesTargets( array<string> targets, JMConfirmation confirmation = NULL )
	{
		m_Form.m_Module.RemoveAllDiseases( targets );
	}

	void Click_HealPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_HEAL", this, "HealTargets", false );
	}

	void HealTargets( array<string> targets, JMConfirmation confirmation = NULL )
	{
		m_Form.m_Module.Heal( targets );
	}

	void Click_SpectatePlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		TStringArray players = JM_GetSelected().GetPlayers();
		string guid = players[0];

		if (guid == GetPermissionsManager().GetClientGUID() && CurrentActiveCamera)
		{
			//! Selected player is ourself and we are in spectator cam, or we are in freecam and previous camera was spectator cam

			if (CurrentActiveCamera.IsInherited(JMSpectatorCamera) || (COT_PreviousActiveCamera && COT_PreviousActiveCamera.IsInherited(JMSpectatorCamera)))
			{
				//! We are spectating something else. Stop spectating.
				m_Form.m_Module.EndSpectating();
				return;
			}
		}

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer(guid);
		PlayerBase player;
		if (instance)
			player = instance.PlayerObject;

		m_Form.m_Module.Click_Spectate(action, player, guid);
	}

	void Click_RepairTransport( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.m_Module.RepairTransport( JM_GetSelected().GetPlayersOrSelf() );
	}

	//! Every toggle checkbox lands here; which toggle it is comes from the checkbox itself.
	void OnClick_Toggle( UIEvent eid, UIActionBase action )
	{
		Class data;
		action.GetUserData( data );

		JMPlayerToggle toggle;
		if ( eid == UIEvent.CLICK && Class.CastTo( toggle, data ) )
			ClickToggle( toggle.GetId() );
	}

	//! Send the state of one toggle's checkbox for whoever is selected.
	void ClickToggle( string id )
	{
		UIActionCheckbox checkbox = GetToggleCheckbox( id );
		if ( !checkbox )
			return;

		m_Form.UpdateLastChangeTime();

		m_Form.m_Module.SetToggleById( id, checkbox.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}
}
