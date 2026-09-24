class JMPlayerModule: JMRenderableModuleBase
{
	PlayerBase m_SpectatorClient;
	ref map<string, PlayerBase> m_Spectators = new map<string, PlayerBase>();
	JMCameraBase m_SpectatorCamera;

	// -----------------------------------------------------------------------
	// Batched stat setter - single RPC replaces 7 individual SetX methods.
	// Public entry points below are kept as thin wrappers so call sites and
	// form code don't need to change. Each stat is a JMPlayerStat (JMPlayerStat.c).
	// -----------------------------------------------------------------------

	protected ref array< ref JMPlayerStat > m_Stats;

	// -----------------------------------------------------------------------
	//  Toggles
	//
	//  Every on/off state an admin can set on a player is a JMPlayerToggle (see
	//  JMPlayerToggle.c). Sending, applying and receiving one is the same code
	//  for all of them, so it lives here once. The named setters below are thin
	//  wrappers kept so call sites and mods do not need to change.
	// -----------------------------------------------------------------------

	protected ref array< ref JMPlayerToggle > m_Toggles;

	// ---------------- Expansion player info (server -> client) ----------------
	//
	// Money, faction, reputation and group all live on the SERVER: the ATM
	// balance is a JSON file, the group is a server-side party object, and the
	// two PlayerBase getters only answer for a player the caller has in its
	// network bubble - which an admin looking at a roster row generally does
	// not. So this is a request/response pair like the session statistics, and
	// like them it is only sent when the Statistics tab is actually open.
	//
	// The payload is a list of id/value string pairs rather than a fixed struct
	// so that which of the four Expansion mods happen to be loaded changes the
	// LENGTH of the answer and nothing else - a server with only Hardline sends
	// one row, and no build of the client has to agree about the shape.

	//! Sanity bound on a payload the client has not built itself. Four rows are
	//! defined today; the slack is for a sub-mod that adds its own.
	static const int EXP_INFO_MAX_ROWS = 32;
	static const string EXP_INFO_MONEY      = "money";
	static const string EXP_INFO_FACTION    = "faction";
	static const string EXP_INFO_REPUTATION = "reputation";
	static const string EXP_INFO_GROUP      = "group";

	void JMPlayerModule()
	{
		JMScriptInvokers.MENU_PLAYER_CHECKBOX.Insert( OnPlayer_Checked );
		JMScriptInvokers.MENU_PLAYER_BUTTON.Insert( OnPlayer_Button );
	}

	void ~JMPlayerModule()
	{
		if ( !g_Game ) return;

		JMScriptInvokers.MENU_PLAYER_CHECKBOX.Remove( OnPlayer_Checked );
		JMScriptInvokers.MENU_PLAYER_BUTTON.Remove( OnPlayer_Button );
	}

	// Helpers exposed to the form for the body-part dropdown labels. Reads
	// from any connected player's BleedingSourcesManagerServer (which carries
	// the modded COT_GetZoneSelectionName / COT_GetZoneCount). Falls back to
	// the local player if no other is available.
	int GetBleedingZoneCount()
	{
		return GetBleedingZoneCountFor( NULL );
	}

	// Variants that take an explicit target player - used by the form to populate
	// the dropdown from the selected player (whose BleedingSourcesManagerServer
	// is always constructed on the server).
	int GetBleedingZoneCountFor( PlayerBase player )
	{
		if ( !player )
			player = PlayerBase.Cast( g_Game.GetPlayer() );
		if ( !player )
			return 0;
		BleedingSourcesManagerServer bms = player.GetBleedingManagerServer();
		if ( !bms )
			return 0;
		return bms.COT_GetZoneCount();
	}

	string GetBleedingZoneSelectionName( int idx )
	{
		return GetBleedingZoneSelectionNameFor( NULL, idx );
	}

	string GetBleedingZoneSelectionNameFor( PlayerBase player, int idx )
	{
		if ( !player )
			player = PlayerBase.Cast( g_Game.GetPlayer() );
		if ( !player )
			return "";
		BleedingSourcesManagerServer bms = player.GetBleedingManagerServer();
		if ( !bms )
			return "";
		return bms.COT_GetZoneSelectionName( idx );
	}

	//! The stat for a wire id, or NULL when it is not one of ours.
	JMPlayerStat GetStat( int type )
	{
		array< ref JMPlayerStat > registeredStats = GetStats();
		foreach ( JMPlayerStat stat : registeredStats )
		{
			if ( stat.GetType() == type )
				return stat;
		}

		return NULL;
	}

	//! Every stat, in the order the General tab shows them.
	array< ref JMPlayerStat > GetStats()
	{
		if ( !m_Stats )
		{
			m_Stats = new array< ref JMPlayerStat >;

			RegisterStats( m_Stats );
		}

		return m_Stats;
	}

	JMPlayerToggle GetToggle( string id )
	{
		array< ref JMPlayerToggle > registeredToggles = GetToggles();
		foreach ( JMPlayerToggle toggle : registeredToggles )
		{
			if ( toggle.GetId() == id )
				return toggle;
		}

		return NULL;
	}

	//! The toggle a JMPlayerModuleRPC id belongs to, or NULL when it is not a toggle's.
	JMPlayerToggle GetToggleByRPC( int rpc_type )
	{
		array< ref JMPlayerToggle > registeredToggles = GetToggles();
		foreach ( JMPlayerToggle toggle : registeredToggles )
		{
			if ( toggle.GetRPC() == rpc_type )
				return toggle;
		}

		return NULL;
	}

	//! Every toggle, in the order the Actions tab shows them.
	array< ref JMPlayerToggle > GetToggles()
	{
		if ( !m_Toggles )
		{
			m_Toggles = new array< ref JMPlayerToggle >;

			RegisterToggles( m_Toggles );
		}

		return m_Toggles;
	}

	//! Is `target` the player who sent this RPC?
	//!
	//! The voice RPCs carry the speaking player as their target, and a client
	//! chooses what it puts there. Without this check any player can name an
	//! invisible admin as the target and flip that admin's invisibility mode
	//! from across the map, by sending a packet they never spoke into.
	protected bool IsOwnPlayer( Object target, PlayerIdentity senderRPC )
	{
		if ( !target || !senderRPC )
			return false;

		PlayerBase player;
		if ( !Class.CastTo( player, target ) )
			return false;

		if ( !player.GetIdentity() )
			return false;

		return player.GetIdentity().GetId() == senderRPC.GetId();
	}

	void SetAdminNVG( bool value, array< string > guids ) { SetToggleById( JMPlayerToggle.ADMINNVG, value, guids ); }

	void SetBlood(      float v, array< string > guids ) { SetStat( JMStatType.Blood,      v, guids ); }

	void SetBloodyHands( bool value, array< string > guids ) { SetToggleById( JMPlayerToggle.BLOODYHANDS, value, guids ); }

	void SetBrokenLegs( bool value, array< string > guids ) { SetToggleById( JMPlayerToggle.BROKENLEGS, value, guids ); }

	void SetCannotBeTargetedByAI( bool value, array< string > guids ) { SetToggleById( JMPlayerToggle.CANNOTBETARGETEDBYAI, value, guids ); }

	void SetEnergy(     float v, array< string > guids ) { SetStat( JMStatType.Energy,     v, guids ); }

	void SetFreeze( bool value, array< string > guids ) { SetToggleById( JMPlayerToggle.FREEZE, value, guids ); }

	void SetGodMode( bool value, array< string > guids ) { SetToggleById( JMPlayerToggle.GODMODE, value, guids ); }

	// Public wrappers - keep existing call sites working without changes
	void SetHealth(     float v, array< string > guids ) { SetStat( JMStatType.Health,     v, guids ); }

	void SetHeatBuffer( float v, array< string > guids ) { SetStat( JMStatType.HeatBuffer, v, guids ); }

	void SetInvisible( int value, array< string > guids ) { SetToggleById( JMPlayerToggle.INVISIBILITY, value, guids ); }

	protected bool SetItemFoodStage( ItemBase asItem, int stage, out string verb )
	{
		if ( !asItem || !asItem.HasFoodStage() )
			return false;

		if ( stage <= FoodStageType.NONE || stage >= FoodStageType.COUNT )
			return false;

		Edible_Base asFood;
		if ( !Class.CastTo( asFood, asItem ) || !asFood.GetFoodStage() )
			return false;

		asFood.GetFoodStage().ChangeFoodStage( stage );

		verb = "Set state " + typename.EnumToString( FoodStageType, stage );
		return true;
	}

	protected bool SetItemHealth( EntityAI entity, float value, out string verb )
	{
		if ( !entity )
			return false;

		float health = Math.Clamp( value, 0, entity.GetMaxHealth( "", "" ) );
		entity.SetHealth( "", "", health );

		verb = "Set health " + health.ToString();
		return true;
	}

	protected bool SetItemLiquidType( ItemBase asItem, int liquidType, out string verb )
	{
		if ( !asItem || !asItem.IsLiquidContainer() )
			return false;

		// Two independent checks, and both matter: the first rejects a type no
		// config defines, the second rejects a real liquid this container was
		// never meant to hold - a client is free to send either.
		if ( !Liquid.GetNutritionalProfileByType( liquidType ) )
			return false;

		if ( ( liquidType & asItem.GetLiquidContainerMask() ) == 0 )
			return false;

		asItem.SetLiquidType( liquidType );

		verb = "Set liquid " + Liquid.GetLiquidClassname( liquidType );
		return true;
	}

	//! Ammo count for a magazine or an ammo pile, quantity for everything else.
	//! A magazine answers GetQuantity with 1 of 1 - itself - so the two cannot
	//! share a path.
	protected bool SetItemQuantity( EntityAI entity, ItemBase asItem, float value, out string verb )
	{
		Magazine asMag;
		if ( Class.CastTo( asMag, entity ) )
		{
			int rounds = Math.Clamp( Math.Round( value ), 0, asMag.GetAmmoMax() );
			asMag.ServerSetAmmoCount( rounds );
			verb = "Set quantity " + rounds.ToString();
			return true;
		}

		if ( !asItem || !asItem.HasQuantity() )
			return false;

		float quantity = Math.Clamp( value, asItem.GetQuantityMin(), asItem.GetQuantityMax() );
		asItem.SetQuantity( quantity );

		// Batteries and the like carry their charge as energy, not quantity, and
		// the two have to move together or the item reads full and behaves flat.
		if ( asItem.GetCompEM() && asItem.GetQuantityMax() > 0 )
			asItem.GetCompEM().SetEnergy0To1( quantity / asItem.GetQuantityMax() );

		verb = "Set quantity " + quantity.ToString();
		return true;
	}

	void SetPermissions( JMPermission permission, array< string > guids )
	{
		auto trace = CF_Trace_0(this, "SetPermissions");

		if ( IsMissionHost() )
		{
			Exec_SetPermissions( permission, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			permission.OnSend(rpc);
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetPermissions, true, NULL );
		}
	}

	void SetReceiveDamageDealt( bool value, array< string > guids ) { SetToggleById( JMPlayerToggle.RECEIVEDAMAGEDEALT, value, guids ); }

	void SetRemoveCollision( bool value, array< string > guids ) { SetToggleById( JMPlayerToggle.REMOVECOLLISION, value, guids ); }

	// nameRestrictions: optional map of role -> required in-game name (case-sensitive).
	void SetRoles( array< string > roles, array< string > guids, map< string, string > nameRestrictions = NULL )
	{
		if ( IsMissionHost() )
		{
			Exec_SetRoles( roles, guids, NULL, NULL, nameRestrictions );
		} else
		{
			// Serialize nameRestrictions as two parallel arrays
			array< string > nrKeys   = new array< string >;
			array< string > nrValues = new array< string >;
			if ( nameRestrictions )
			{
				for ( int ni = 0; ni < nameRestrictions.Count(); ni++ )
				{
					nrKeys.Insert( nameRestrictions.GetKey( ni ) );
					nrValues.Insert( nameRestrictions.GetElement( ni ) );
				}
			}

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( roles );
			rpc.Write( guids );
			rpc.Write( nrKeys );
			rpc.Write( nrValues );
			rpc.Send( NULL, JMPlayerModuleRPC.SetRoles, true, NULL );
		}
	}

	void SetScale( float value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetScale( value, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetScale, true, NULL );
		}
	}

	void SetShock(      float v, array< string > guids ) { SetStat( JMStatType.Shock,      v, guids ); }

	void SetStamina(    float v, array< string > guids ) { SetStat( JMStatType.Stamina,    v, guids ); }

	void SetStat( int type, float value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetStat( type, value, guids, NULL );
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( type );
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetStat, true, NULL );
		}
	}

	void SetToggle( JMPlayerToggle toggle, int value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_Toggle( toggle, value, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			toggle.WriteValue( rpc, value );
			rpc.Write( guids );
			rpc.Send( NULL, toggle.GetRPC(), true, NULL );
		}
	}

	void SetToggleById( string id, int value, array< string > guids )
	{
		JMPlayerToggle toggle = GetToggle( id );
		if ( toggle )
			SetToggle( toggle, value, guids );
	}

	void SetUnlimitedAmmo( bool value, array< string > guids ) { SetToggleById( JMPlayerToggle.UNLIMITEDAMMO, value, guids ); }

	void SetUnlimitedStamina( bool value, array< string > guids ) { SetToggleById( JMPlayerToggle.UNLIMITEDSTAMINA, value, guids ); }

	void SetWater(      float v, array< string > guids ) { SetStat( JMStatType.Water,      v, guids ); }

	//! Called on both client and server as the module registers, before the mission loads.
	override void DeclarePermissions()
	{
		super.DeclarePermissions();

		JMPermissions.Register( JMConstants.PERM_PLAYER_HEAL );
		JMPermissions.Register( JMConstants.PERM_PLAYER_HEAL_ATTACHMENTS );
		JMPermissions.Register( JMConstants.PERM_PLAYER_HEAL_CARGO );
		JMPermissions.Register( JMConstants.PERM_PLAYER_GODMODE );
		JMPermissions.Register( JMConstants.PERM_PLAYER_FREEZE );
#ifndef DAYZ_1_29
		JMPermissions.Register( JMConstants.PERM_PLAYER_RAGDOLL );
#endif
		JMPermissions.Register( JMConstants.PERM_PLAYER_INVISIBILITY );
		JMPermissions.Register( JMConstants.PERM_PLAYER_UNLIMITEDAMMO );
		JMPermissions.Register( JMConstants.PERM_PLAYER_UNLIMITEDSTAMINA );
		JMPermissions.Register( JMConstants.PERM_PLAYER_SPECTATE );
		JMPermissions.Register( JMConstants.PERM_PLAYER_STRIP );
		JMPermissions.Register( JMConstants.PERM_PLAYER_CLEARCARGO );		
		JMPermissions.Register( JMConstants.PERM_PLAYER_DRY );
		JMPermissions.Register( JMConstants.PERM_PLAYER_STOPBLEEDING );
		JMPermissions.Register( JMConstants.PERM_PLAYER_BROKENLEGS );
		JMPermissions.Register( JMConstants.PERM_PLAYER_RECEIVEDAMAGEDEALT );
		JMPermissions.Register( JMConstants.PERM_PLAYER_KICK );
		JMPermissions.Register( JMConstants.PERM_PLAYER_BAN );
		JMPermissions.Register( JMConstants.PERM_PLAYER_MESSAGE );
		JMPermissions.Register( JMConstants.PERM_PLAYER_NOTIF );
		JMPermissions.Register( JMConstants.PERM_PLAYER_CANNOTBETARGETEDBYAI );
		JMPermissions.Register( JMConstants.PERM_PLAYER_ACCESSINVENTORY );

		//! New permissions default to INHERIT, so no existing role gains these on
		//! upgrade - an admin has to grant them before the inventory tab can do
		//! anything beyond listing.
		JMPermissions.Register( JMConstants.PERM_PLAYER_INVENTORY_DELETE );
		JMPermissions.Register( JMConstants.PERM_PLAYER_INVENTORY_REPAIR );
		JMPermissions.Register( JMConstants.PERM_PLAYER_INVENTORY_TAKE );

		//! Covers every in-place edit of an item the admin already sees:
		//! unjamming, quantity, temperature, food stage and liquid type. They
		//! are one permission because they are one kind of act - rewriting a
		//! carried item's state - and splitting them would leave a role able to
		//! set a canned bean rotten but not burned.
		JMPermissions.Register( JMConstants.PERM_PLAYER_INVENTORY_MODIFY );

		JMPermissions.Register( JMConstants.PERM_PLAYER_STATISTICS_VIEW );
		JMPermissions.Register( JMConstants.PERM_PLAYER_REMOVECOLLISION );
		JMPermissions.Register( JMConstants.PERM_PLAYER_ADMINNVG );

		JMPermissions.Register( JMConstants.PERM_PLAYER_TELEPORT_POSITION );
		JMPermissions.Register( JMConstants.PERM_PLAYER_TELEPORT_SENDERTO );
		JMPermissions.Register( JMConstants.PERM_PLAYER_TELEPORT_PREVIOUS );

		JMPermissions.Register( JMConstants.PERM_PLAYER_PERMISSIONS );
		JMPermissions.Register( JMConstants.PERM_PLAYER_ROLES );

		JMPermissions.Register( JMConstants.PERM_PLAYER_SET_HEALTH );
		JMPermissions.Register( JMConstants.PERM_PLAYER_SET_SHOCK );
		JMPermissions.Register( JMConstants.PERM_PLAYER_SET_BLOOD );
		JMPermissions.Register( JMConstants.PERM_PLAYER_SET_ENERGY );
		JMPermissions.Register( JMConstants.PERM_PLAYER_SET_WATER );
		JMPermissions.Register( JMConstants.PERM_PLAYER_SET_STAMINA );
		JMPermissions.Register( JMConstants.PERM_PLAYER_SET_HEATBUFFER );
		JMPermissions.Register( JMConstants.PERM_PLAYER_SET_BLOODYHANDS );

		JMPermissions.Register( JMConstants.PERM_PLAYER_DISEASE_ADD );
		JMPermissions.Register( JMConstants.PERM_PLAYER_DISEASE_REMOVE );

		JMPermissions.Register( JMConstants.PERM_PLAYER_BLEED_ADD );
		JMPermissions.Register( JMConstants.PERM_PLAYER_BLEED_STOP );

		JMPermissions.Register( JMConstants.PERM_TRANSPORT_REPAIR );

		JMPermissions.Register( JMConstants.PERM_PLAYER_VOMIT );
		JMPermissions.Register( JMConstants.PERM_PLAYER_SCALE );
		JMPermissions.Register( "COT" );

		//! The nodes above keep their order in the role editor. A toggle or stat a mod adds through
		//! RegisterToggles() / RegisterStats() brings its own permission, declared here after them.
		array< ref JMPlayerToggle > registeredToggles = GetToggles();
		foreach ( JMPlayerToggle toggle : registeredToggles )
			JMPermissions.Register( toggle.GetPermission() );

		array< ref JMPlayerStat > registeredStats = GetStats();
		foreach ( JMPlayerStat stat : registeredStats )
			JMPermissions.Register( stat.GetPermission() );
	}

	override void EnableUpdate()
	{
	}

	override void RegisterKeyMouseBindings() 
	{
		super.RegisterKeyMouseBindings();
		
		Bind( new JMModuleBinding( "InputHeal",			"UAPlayerModuleHeal",		true 	) );
		Bind( new JMModuleBinding( "InputToggleGodMode",	"UAPlayerModuleGodMode",	true 	) );
		Bind( new JMModuleBinding( "InputToggleInvisibility",	"UAPlayerModuleInvisibility",	true 	) );
		Bind( new JMModuleBinding( "InputToggleCannotBeTargetedByAI",	"UAPlayerModuleCannotBeTargetedByAI",	true 	) );
		Bind( new JMModuleBinding( "InputToggleUnlimitedStamina",	"UAPlayerModuleUnlimitedStamina",	true 	) );
		Bind( new JMModuleBinding( "InputToggleUnlimitedAmmo",	"UAPlayerModuleUnlimitedAmmo",	true 	) );
		Bind( new JMModuleBinding( "InputToggleAdminNV",	"UAPlayerModuleAdminNV",	true 	) );
		Bind( new JMModuleBinding( "InputFreezePlayer",		"UAPlayerModuleFreezePlayer",		true 	) );
		Bind( new JMModuleBinding( "EndSpectating",      "UAPlayerModuleStopSpectating", true ) );
		Bind( new JMModuleBinding( "Input_HealSelf",     "UAPlayerModuleHealSelf",       true ) );
		Bind( new JMModuleBinding( "Input_ToggleFly",    "UAPlayerModuleToggleFly",      true ) );
	}

	void OnPlayer_Checked( string guid, bool checked )
	{
		if ( checked )
			JM_GetSelected().AddPlayer( guid );
		else
			JM_GetSelected().RemovePlayer( guid );
	}

	void OnPlayer_Button( string guid, bool check )
	{
		JM_GetSelected().ClearPlayers();

		if ( check )
			JM_GetSelected().AddPlayer( guid );
	}

	override bool HasAccess()
	{
		return true;
	}

	override void DescribeModule( JMModuleInfo info )
	{
		super.DescribeModule( info );

		info.Title = "#STR_COT_PLAYER_MODULE_NAME";
		info.WebhookTitle = "Player Management Module";
		info.Icon = "user-cog";
		info.Layout = "JM/COT/GUI/layouts/player_form.layout";
		info.Category = JMSideBarConfig.CATEGORY_PLAYERS;
		info.InputToggle = "UACOTTogglePlayer";
		info.SetRPCRange( JMPlayerModuleRPC.INVALID, JMPlayerModuleRPC.COUNT );

		info.AddWebhookType( "Set" );
		info.AddWebhookType( "Vehicle" );
		info.AddWebhookType( "Teleport" );
		info.AddWebhookType( "Inventory" );
		info.AddWebhookType( "PF" );
		info.AddWebhookType( "Kick" );
		info.AddWebhookType( "Message" );
		info.AddWebhookType( "Notif" );
		info.AddWebhookType( "Ban" );
		info.AddWebhookType( "Disease" );
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMPlayerModuleRPC.SetStat:
			RPC_SetStat( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.RepairTransport:
			RPC_RepairTransport( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.TeleportTo:
			RPC_TeleportTo( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.TeleportSenderTo:
			RPC_TeleportSenderTo( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.TeleportToPrevious:
			RPC_TeleportToPrevious( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.StartSpectating:
			RPC_StartSpectating( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.EndSpectating:
			RPC_EndSpectating( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.EndSpectating_Finish:
			RPC_EndSpectating_Finish( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Heal:
			RPC_Heal( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Strip:
			RPC_Strip( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.ClearCargo:
			RPC_ClearCargo( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Dry:
			RPC_Dry( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.StopBleeding:
			RPC_StopBleeding( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetPermissions:
			RPC_SetPermissions( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetRoles:
			RPC_SetRoles( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Kick:
			RPC_Kick( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.KickMessage:
			RPC_KickMessage( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Ban:
			RPC_Ban( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.BanMessage:
			RPC_BanMessage( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Message:
			RPC_Message( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Notif:
			RPC_Notif( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.VONStartedTransmitting:
			RPC_VONStartedTransmitting( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.VONStoppedTransmitting:
			RPC_VONStoppedTransmitting( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Vomit:
			RPC_Vomit( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetScale:
			RPC_SetScale( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.AddDisease:
			RPC_AddDisease( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.RemoveDisease:
			RPC_RemoveDisease( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.RemoveAllDiseases:
			RPC_RemoveAllDiseases( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.ActivateModifier:
			RPC_ActivateModifier( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.DeactivateModifier:
			RPC_DeactivateModifier( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SendDiseaseMask:
			RPC_SendDiseaseMask( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.AddBleedingPart:
			RPC_AddBleedingPart( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.StopBleedingPart:
			RPC_StopBleedingPart( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SendBleedingState:
			RPC_SendBleedingState( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.RequestInventory:
			RPC_RequestInventory( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.InventoryDelete:
			RPC_InventoryDelete( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.InventoryTake:
			RPC_InventoryTake( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.InventoryModify:
			RPC_InventoryModify( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.InventoryGroupOp:
			RPC_InventoryGroupOp( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.RequestExpansionInfo:
			RPC_RequestExpansionInfo( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.RequestPlayerStats:
			RPC_RequestPlayerStats( ctx, sender, target );
			break;
		default:
			//! Every remaining id belongs to a JMPlayerToggle - see RegisterToggles().
			JMPlayerToggle toggle = GetToggleByRPC( rpc_type );
			if ( toggle )
				RPC_Toggle( toggle, ctx, sender );
			break;
		}
	}

	override void OnClientDisconnect( PlayerBase player, PlayerIdentity identity, string uid )
	{
		if ( m_Spectators.Contains( uid ) )
			m_Spectators.Remove( uid );
	}

	void DoMessage( array< string > guids, string messageText )
	{
		if ( IsMissionHost() )
		{
			Exec_Message( guids, NULL, NULL, messageText );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Write(messageText);
			rpc.Send( NULL, JMPlayerModuleRPC.Message, true, NULL );
		}
	}

	protected void Exec_Message( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL, string messageText = ""  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		foreach (JMPlayerInstance player: players)
		{
			if (!player.PlayerObject)
				continue;

			GetCommunityOnlineToolsBase().Log( ident, "Sent message to " + player.GetName() + " (" + player.GetGUID() + "): " + messageText );
			SendWebhookColored( "Message", instance, "Sent message to " + player.FormatSteamWebhook() + ": " + messageText, JMConstants.WEBHOOK_COLOR_INFO );

			Message( player.PlayerObject, messageText );
		}
	}

	protected void RPC_Message( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		string messageText;
		if (!ctx.Read(messageText))
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_MESSAGE, senderRPC, instance ) )
			return;

		Exec_Message( guids, senderRPC, instance, messageText );
	}

	protected void SendMessage(PlayerIdentity identity, string messageText)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(messageText);
		rpc.Send(NULL, JMPlayerModuleRPC.Message, true, identity);
	}

	void DoNotif( array< string > guids, string NotifText )
	{
		if ( IsMissionHost() )
		{
			Exec_Notif( guids, NULL, NULL, NotifText );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Write(NotifText);
			rpc.Send( NULL, JMPlayerModuleRPC.Notif, true, NULL );
		}
	}

	protected void Exec_Notif( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL, string NotifText = ""  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		foreach (JMPlayerInstance player: players)
		{
			if (!player.PlayerObject)
				continue;

			if ( ident )
			{
				GetCommunityOnlineToolsBase().Log( ident, "Sent notification to " + player.GetName() + " (" + player.GetGUID() + "): " + NotifText );
				SendWebhookColored( "Notif", instance, "Sent notification to " + player.FormatSteamWebhook() + ": " + NotifText, JMConstants.WEBHOOK_COLOR_INFO );
			}

			NotificationSystem.Create( new StringLocaliser( "#STR_COT_NOTIFICATION_MESSAGE_FROM_ADMIN" ), new StringLocaliser( NotifText ), "JM//COT//gui//textures//cot_icon.edds", COLOR_RED, 10, player.PlayerObject.GetIdentity() );
		}
	}

	protected void RPC_Notif( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_NOTIF, senderRPC, instance ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		string NotifText;
		if (!ctx.Read(NotifText))
			return;

		Exec_Notif( guids, senderRPC, instance, NotifText );
	}

	protected void SendNotif(PlayerIdentity identity, string NotifText)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(NotifText);
		rpc.Send(NULL, JMPlayerModuleRPC.Notif, true, identity);
	}

	//! Configure `stat`, add it to the registry and hand it back for the WithBands() chain.
	//! A helper rather than `new X().Define(...)` because Enforce cannot call a method on a
	//! `new` expression.
	protected JMPlayerStat DefineStat( array< ref JMPlayerStat > stats, JMPlayerStat stat, int type, string name, string permission, float min, float max )
	{
		stat.Define( type, name, permission, min, max );
		stats.Insert( stat );

		return stat;
	}

	//! Override, call super, and add your own with DefineStat(). See JMPlayerStat.
	void RegisterStats( array< ref JMPlayerStat > stats )
	{
		DefineStat( stats, new JMPlayerStatHealth(), JMStatType.Health, "Health", JMConstants.PERM_PLAYER_SET_HEALTH, JMConstants.STAT_HEALTH_MIN, JMConstants.STAT_HEALTH_MAX ).WithBands( PlayerConstants.SL_HEALTH_HIGH, PlayerConstants.SL_HEALTH_NORMAL, PlayerConstants.SL_HEALTH_LOW );
		DefineStat( stats, new JMPlayerStatBlood(), JMStatType.Blood, "Blood", JMConstants.PERM_PLAYER_SET_BLOOD, JMConstants.STAT_BLOOD_MIN, JMConstants.STAT_BLOOD_MAX ).WithBands( PlayerConstants.SL_BLOOD_HIGH, PlayerConstants.SL_BLOOD_NORMAL, PlayerConstants.SL_BLOOD_LOW );
		DefineStat( stats, new JMPlayerStatEnergy(), JMStatType.Energy, "Energy", JMConstants.PERM_PLAYER_SET_ENERGY, JMConstants.STAT_ENERGY_MIN, JMConstants.STAT_ENERGY_MAX ).WithBands( PlayerConstants.SL_ENERGY_HIGH, PlayerConstants.SL_ENERGY_NORMAL, PlayerConstants.SL_ENERGY_LOW );
		DefineStat( stats, new JMPlayerStatWater(), JMStatType.Water, "Water", JMConstants.PERM_PLAYER_SET_WATER, JMConstants.STAT_WATER_MIN, JMConstants.STAT_WATER_MAX ).WithBands( PlayerConstants.SL_WATER_HIGH, PlayerConstants.SL_WATER_NORMAL, PlayerConstants.SL_WATER_LOW );
		DefineStat( stats, new JMPlayerStatShock(), JMStatType.Shock, "Shock", JMConstants.PERM_PLAYER_SET_SHOCK, JMConstants.STAT_SHOCK_MIN, JMConstants.STAT_SHOCK_MAX ).WithBands( 75, 0, 0 );
		DefineStat( stats, new JMPlayerStatStamina(), JMStatType.Stamina, "Stamina", JMConstants.PERM_PLAYER_SET_STAMINA, JMConstants.STAT_STAMINA_MIN, JMConstants.STAT_STAMINA_MAX ).WithBands( 75, 50, 25 );
		DefineStat( stats, new JMPlayerStatHeatBuffer(), JMStatType.HeatBuffer, "HeatBuffer", JMConstants.PERM_PLAYER_SET_HEATBUFFER, JMConstants.STAT_HEATBUFFER_MIN, JMConstants.STAT_HEATBUFFER_MAX );
	}

	protected void Exec_SetStat( int type, float value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		JMPlayerStat stat = GetStat( type );
		if ( !stat )
			return;

		value = stat.Clamp( value );
		string statName = stat.GetName();

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );
		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			stat.Apply( player, value );

			GetCommunityOnlineToolsBase().Log( ident, "Set " + statName + " To " + value + " [guid=" + players[i].GetGUID() + "]" );
			SendWebhookColored( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " " + statName + " to " + value, JMConstants.WEBHOOK_COLOR_WARNING );

			players[i].Update();
		}
	}

	protected void RPC_SetStat( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int type;
		if ( !ctx.Read( type ) )
			return;

		JMPlayerStat stat = GetStat( type );
		if ( !stat )
			return;

		float value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		if ( !senderRPC )
			return;

		JMPlayerInstance instance;
		if ( !JMPermissions.HasRPC( stat.GetPermission(), senderRPC, instance ) )
			return;

		Exec_SetStat( type, value, guids, senderRPC, instance );
	}

	//! Configure `toggle`, add it to the registry and hand it back for the With...() chain. A
	//! helper rather than `new X().Define(...)` because Enforce cannot call a method on a `new`
	//! expression.
	protected JMPlayerToggle DefineToggle( array< ref JMPlayerToggle > toggles, JMPlayerToggle toggle, string id, int rpc, string permission, string name, string key )
	{
		toggle.Define( id, rpc, permission, name, key );
		toggles.Insert( toggle );

		return toggle;
	}

	//! Override, call super, and add your own with DefineToggle(). See JMPlayerToggle.
	void RegisterToggles( array< ref JMPlayerToggle > toggles )
	{
		DefineToggle( toggles, new JMPlayerToggleRemoveCollision(), JMPlayerToggle.REMOVECOLLISION, JMPlayerModuleRPC.SetRemoveCollision, JMConstants.PERM_PLAYER_REMOVECOLLISION, "remove collision", "REMOVE_COLLISION" ).WithIcon( "ghost" );
		DefineToggle( toggles, new JMPlayerToggleGodMode(), JMPlayerToggle.GODMODE, JMPlayerModuleRPC.SetGodMode, JMConstants.PERM_PLAYER_GODMODE, "god mode", "GODMODE" ).WithIcon( "crown" ).WithInputHint( "STR_COT_INPUT_GODMODE_SELF" );
		DefineToggle( toggles, new JMPlayerToggleFreeze(), JMPlayerToggle.FREEZE, JMPlayerModuleRPC.SetFreeze, JMConstants.PERM_PLAYER_FREEZE, "freeze", "FREEZE" ).WithIcon( "snowflake" ).WithInputHint( "STR_COT_INPUT_FREEZE_PLAYER" );
		DefineToggle( toggles, new JMPlayerToggleInvisibility(), JMPlayerToggle.INVISIBILITY, JMPlayerModuleRPC.SetInvisible, JMConstants.PERM_PLAYER_INVISIBILITY, "invisibility", "INVISIBLE" ).WithIcon( "eye-off" ).WithInputHint( "STR_COT_INPUT_INVISIBILITY_SELF" );
		DefineToggle( toggles, new JMPlayerToggleBloodyHands(), JMPlayerToggle.BLOODYHANDS, JMPlayerModuleRPC.SetBloodyHands, JMConstants.PERM_PLAYER_SET_BLOODYHANDS, "bloody hands", "BLOODY_HANDS" ).WithIcon( "droplet" );
		DefineToggle( toggles, new JMPlayerToggleCannotBeTargetedByAI(), JMPlayerToggle.CANNOTBETARGETEDBYAI, JMPlayerModuleRPC.SetCannotBeTargetedByAI, JMConstants.PERM_PLAYER_CANNOTBETARGETEDBYAI, "cannot be targeted by AI", "IGNORED_BY_AI" ).WithIcon( "bot-off" ).WithInputHint( "STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_IGNORED_BY_AI" );
		DefineToggle( toggles, new JMPlayerToggleBrokenLegs(), JMPlayerToggle.BROKENLEGS, JMPlayerModuleRPC.SetBrokenLegs, JMConstants.PERM_PLAYER_BROKENLEGS, "broken legs", "BROKEN_LEGS" ).WithIcon( "bone-fracture" ).WithClientSync();
		DefineToggle( toggles, new JMPlayerToggleUnlimitedStamina(), JMPlayerToggle.UNLIMITEDSTAMINA, JMPlayerModuleRPC.SetUnlimitedStamina, JMConstants.PERM_PLAYER_UNLIMITEDSTAMINA, "unlimited stamina", "UNLIMITED_STAMINA" ).WithIcon( "zap" ).WithInputHint( "STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_UNLIMITED_STAMINA" ).WithClientSync();
		DefineToggle( toggles, new JMPlayerToggleReceiveDamageDealt(), JMPlayerToggle.RECEIVEDAMAGEDEALT, JMPlayerModuleRPC.SetReceiveDamageDealt, JMConstants.PERM_PLAYER_RECEIVEDAMAGEDEALT, "receive damage dealt", "RECEIVE_DAMAGE_DEALT" ).WithIcon( "swords" );
		DefineToggle( toggles, new JMPlayerToggleUnlimitedAmmo(), JMPlayerToggle.UNLIMITEDAMMO, JMPlayerModuleRPC.SetUnlimitedAmmo, JMConstants.PERM_PLAYER_UNLIMITEDAMMO, "unlimited ammo", "UNLIMITED_AMMO" ).WithIcon( "infinity" ).WithInputHint( "STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_UNLIMITED_AMMO" ).WithClientSync();
		DefineToggle( toggles, new JMPlayerToggleAdminNVG(), JMPlayerToggle.ADMINNVG, JMPlayerModuleRPC.SetAdminNVG, JMConstants.PERM_PLAYER_ADMINNVG, "admin night vision", "NVG" ).WithIcon( "eye" ).WithInputHint( "STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_NVG" ).WithClientSync();
	#ifndef DAYZ_1_29
		DefineToggle( toggles, new JMPlayerToggleRagdoll(), JMPlayerToggle.RAGDOLL, JMPlayerModuleRPC.SetRagdoll, JMConstants.PERM_PLAYER_RAGDOLL, "ragdoll", "RAGDOLL" );
	#endif
	}

	protected void Exec_Toggle( JMPlayerToggle toggle, int value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		array<JMPlayerInstance> affectedPlayers = {};

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			toggle.Apply( player, value );

			ProcessToggle( toggle.GetName(), value, players[i], affectedPlayers, ident, instance );

			toggle.OnApplied( players[i] );
		}

		ShowToggleNotification( toggle.GetName(), value, affectedPlayers, ident );
	}

	protected void RPC_Toggle( JMPlayerToggle toggle, ParamsReadContext ctx, PlayerIdentity senderRPC )
	{
		int value;
		if ( !toggle.ReadValue( ctx, value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( toggle.GetPermission(), senderRPC, instance ) )
			return;

		Exec_Toggle( toggle, value, guids, senderRPC, instance );
	}

	//! A keybind that flips one of the local admin's own toggles.
	protected void InputToggle( string id, UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		JMPlayerToggle toggle = GetToggle( id );
		if ( !toggle )
			return;

		JMPlayerInstance instance;
		if ( !JMPermissions.Has( toggle.GetPermission(), instance ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			ShowInactiveNotification( toggle.GetInputHint() );
			return;
		}

		bool value = !toggle.ReadEntity( instance.PlayerObject, instance );
		array< string > guids = {instance.GetGUID()};

		SetToggle( toggle, value, guids );
	}

	void InputFreezePlayer( UAInput input ) { InputToggle( JMPlayerToggle.FREEZE, input ); }

	void InputToggleGodMode( UAInput input ) { InputToggle( JMPlayerToggle.GODMODE, input ); }

	void InputToggleInvisibility( UAInput input ) { InputToggle( JMPlayerToggle.INVISIBILITY, input ); }

	void InputToggleCannotBeTargetedByAI( UAInput input ) { InputToggle( JMPlayerToggle.CANNOTBETARGETEDBYAI, input ); }

	void InputToggleUnlimitedStamina( UAInput input ) { InputToggle( JMPlayerToggle.UNLIMITEDSTAMINA, input ); }

	void InputToggleUnlimitedAmmo( UAInput input ) { InputToggle( JMPlayerToggle.UNLIMITEDAMMO, input ); }

	void InputToggleAdminNV( UAInput input ) { InputToggle( JMPlayerToggle.ADMINNVG, input ); }

#ifndef DAYZ_1_29
	void SetRagdoll( bool value, array< string > guids ) { SetToggleById( JMPlayerToggle.RAGDOLL, value, guids ); }
#endif

	void RepairTransport( array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_RepairTransport( guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.RepairTransport, true, NULL );
		}
	}

	protected void Exec_RepairTransport( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< EntityAI > vehicles = {};

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			EntityAI vehicle;
			if ( !Class.CastTo( vehicle, player.GetParent() ) )
				continue;

			if ( vehicles.Find( vehicle ) > -1 )
				continue;

			vehicles.Insert( vehicle );

			CommunityOnlineToolsBase.HealEntityRecursive(vehicle);
			CommunityOnlineToolsBase.Refuel(vehicle);
			
			JMObjectSpawnerModule objSpawnerModule;
			if (CF_Modules<JMObjectSpawnerModule>.Get(objSpawnerModule))
				objSpawnerModule.SpawnCompatibleAttachments(vehicle, null, 0);

			GetCommunityOnlineToolsBase().Log( ident, "Repaired Transport [guid=" + players[i].GetGUID() + "]" );
			SendWebhookColored( "Vehicle", instance, "Repaired " + players[i].FormatSteamWebhook() + " vehicle", JMConstants.WEBHOOK_COLOR_SUCCESS );

			players[i].Update();
		}
	}

	protected void RPC_RepairTransport( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target  )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_TRANSPORT_REPAIR, senderRPC, instance ) )
			return;

		Exec_RepairTransport( guids, senderRPC, instance );
	}

	void TeleportTo( vector position, array< string > guids )
	{
		//! Recorded here rather than at the call sites: every teleport the
		//! player manager offers - the coordinate box, bring-to-me, the row
		//! menu - ends up in this one method, so this is the only place that
		//! has to know an undo exists.
		JMTeleportHistory.PushPlayers( guids );

		if ( IsMissionHost() )
		{
			Exec_TeleportTo( position, guids, NULL );
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( position );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.TeleportTo, true, NULL );
		}
	}

	protected void Exec_TeleportTo( vector position, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.SetLastPosition();

			player.SetWorldPosition( position );

			GetCommunityOnlineToolsBase().Log( ident, "Teleported [guid=" + players[i].GetGUID() + "] to " + position );

			SendWebhookColored( "Teleport", instance, "Teleported " + players[i].FormatSteamWebhook() + " to " + position.ToString(), JMConstants.WEBHOOK_COLOR_TELEPORT );

			players[i].Update();
		}
	}

	protected void RPC_TeleportTo( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		vector position;
		if ( !ctx.Read( position ) )
			return;

		if ( !CommunityOnlineToolsBase.IsValidWorldPosition( position ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_TELEPORT_POSITION, senderRPC, instance ) )
			return;

		Exec_TeleportTo( position, guids, senderRPC, instance );
	}

	void TeleportSenderTo( string guid )
	{
		//! This one moves the ADMIN, not the target, so it is the admin's own
		//! position that is about to stop being true.
		JMTeleportHistory.PushSelf();

		if ( IsMissionHost() )
		{
			Exec_TeleportSenderTo( guid, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guid );
			rpc.Send( NULL, JMPlayerModuleRPC.TeleportSenderTo, true, NULL );
		}
	}

	protected void Exec_TeleportSenderTo( string guid, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		JMPlayerInstance other = GetPermissionsManager().GetPlayer( guid );

		PlayerBase player = PlayerBase.Cast( other.PlayerObject );
		if ( player == NULL )
			return;

		vector position = player.GetPosition();

		if ( Class.CastTo( player, GetPlayerObjectByIdentity( ident ) ) )
		{
			player.SetLastPosition();

			player.SetWorldPosition( position );

			GetCommunityOnlineToolsBase().Log( ident, "Teleported to " + position + " [guid=" + other.GetGUID() + "]" );
			SendWebhookColored( "Teleport", instance, "Teleported the admin to " + other.FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_TELEPORT );
		}
	}

	protected void RPC_TeleportSenderTo( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string guid;
		if ( !ctx.Read( guid ) )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_TELEPORT_SENDERTO, senderRPC, instance ) )
			return;

		Exec_TeleportSenderTo( guid, senderRPC, instance );
	}

	void TeleportToPrevious( array< string > guids )
	{
		//! The server keeps its own single "last position" and this is the call
		//! that spends it - the teleport-back keybind. Spend the client's
		//! matching step too, or the undo list would keep offering the place
		//! the target has just been sent back to.
		if ( guids )
		{
			for ( int i = 0; i < guids.Count(); i++ )
				JMTeleportHistory.Pop( JMTeleportHistory.PlayerKey( guids[i] ), 0, JMTeleportHistory.PlayerPosition( guids[i] ) );
		}

		if ( IsMissionHost() )
		{
			Exec_TeleportToPrevious( guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.TeleportToPrevious, true, NULL );
		}
	}

	protected void Exec_TeleportToPrevious( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL || !player.HasLastPosition() )
				continue;

			vector position = player.GetLastPosition();

			player.SetLastPosition();

			player.SetWorldPosition( position );

			GetCommunityOnlineToolsBase().Log( ident, "Teleported [guid=" + players[i].GetGUID() + "] to " + position + " [previous]" );
			SendWebhookColored( "Teleport", instance, "Teleported " + players[i].FormatSteamWebhook() + " to their previous position", JMConstants.WEBHOOK_COLOR_TELEPORT );

			players[i].Update();
		}
	}

	protected void RPC_TeleportToPrevious( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_TELEPORT_PREVIOUS, senderRPC, instance ) )
			return;

		Exec_TeleportToPrevious( guids, senderRPC, instance );
	}

	//! Client
	void Click_Spectate(UIActionBase action, Object target, string guid = "")
	{
		bool shouldSpectate;

		if (target || guid != "")
		{
			shouldSpectate = true;

			if (CurrentActiveCamera)
			{
				if (CurrentActiveCamera.IsInherited(JMSpectatorCamera))
					shouldSpectate = !target || CurrentActiveCamera.SelectedTarget != target;
				else if (COT_PreviousActiveCamera && COT_PreviousActiveCamera.IsInherited(JMSpectatorCamera))
					shouldSpectate = !target || COT_PreviousActiveCamera.SelectedTarget != target;
			}
		}

		action.Disable();

		if (shouldSpectate)
		{
			g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(action.Enable, 1000);

			if (!target)
				StartSpectating(guid);
			else
				StartSpectating(target);

			action.SetButton("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_STOP_SPECTATE");
		}
		else
		{
			g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(action.Enable, 3000);

			EndSpectating();

			action.SetButton("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_SPECTATE");
		}
	}

	//! @note this allows to start spectating players that are not in netbubble
	void StartSpectating( string guid )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "StartSpectating").Add(guid);
#endif

		if (GetPlayer().GetCommand_Vehicle())
		{
			COTCreateLocalAdminNotification(new StringLocaliser("#STR_COT_PLAYER_MODULE_SPECTATE_BLOCKED_VEHICLE"));
			return;
		}

		if ( IsMissionHost() )
		{
			if ( IsMissionOffline() )
			{
				Message( GetPlayer(), Widget.TranslateString( "#STR_COT_PLAYER_MODULE_SPECTATE_OFFLINE_UNAVAILABLE" ) );
			}
		} else
		{
			if (!JMPermissions.Has(JMConstants.PERM_PLAYER_SPECTATE))
				return;

			m_SpectatorClient = GetPlayer();

			m_SpectatorClient.COT_TempDisableOnSelectPlayer();
			m_SpectatorClient.COT_RememberVehicle();

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guid );
			rpc.Send( null, JMPlayerModuleRPC.StartSpectating, true, NULL );
		}
	}

	void StartSpectating(Object spectateObject)
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "StartSpectating").Add(spectateObject);
#endif

		if (GetPlayer().GetCommand_Vehicle())
		{
			COTCreateLocalAdminNotification(new StringLocaliser("#STR_COT_PLAYER_MODULE_SPECTATE_BLOCKED_VEHICLE"));
			return;
		}

		if ( IsMissionHost() )
		{
			if ( IsMissionOffline() )
			{
				Message( GetPlayer(), Widget.TranslateString( "#STR_COT_PLAYER_MODULE_SPECTATE_OFFLINE_UNAVAILABLE" ) );
			}
		} else
		{
			if (!JMPermissions.Has(JMConstants.PERM_PLAYER_SPECTATE))
				return;

			m_SpectatorClient = GetPlayer();

			m_SpectatorClient.COT_TempDisableOnSelectPlayer();
			m_SpectatorClient.COT_RememberVehicle();

			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( spectateObject, JMPlayerModuleRPC.StartSpectating, true, NULL );
		}
	}

	//! @note methods meant to only run on server should not be prefixed Exec_,
	//! that's reserved for methods that can run both on client or server
	protected void Server_StartSpectating( string guid, PlayerIdentity ident )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "Server_StartSpectating").Add(guid).Add(ident);
#endif

		JMPlayerInstance spectateInstance = GetPermissionsManager().GetPlayer( guid );
		if ( !spectateInstance )
			return;

		PlayerBase spectatePlayer = spectateInstance.PlayerObject;
		if ( !spectatePlayer )
			return;

		Server_StartSpectating(spectatePlayer, ident);
	}

	//! @note methods meant to only run on server should not be prefixed Exec_,
	//! that's reserved for methods that can run both on client or server
	protected void Server_StartSpectating(Object spectateObject, PlayerIdentity ident)
	{
#ifdef JM_COT_DIAG_LOGGING
		Print(spectateObject);
#endif
		PlayerBase playerSpectator = GetPlayerObjectByIdentity( ident );
		if ( !playerSpectator )
			return;

#ifdef JM_COT_DIAG_LOGGING
		Print(playerSpectator);
#endif

		if ( playerSpectator == spectateObject )
		{
			COTCreateNotification(ident, new StringLocaliser("#STR_COT_PLAYER_MODULE_CANT_SPECTATE_SELF"));
			return;
		}

		if (m_Spectators[ident.GetId()] != playerSpectator)
		{
			playerSpectator.COT_RememberVehicle();
			playerSpectator.SetLastPosition();

			m_Spectators[ident.GetId()] = playerSpectator;
		}

		playerSpectator.m_JM_SpectatedObject = spectateObject;
		playerSpectator.m_JM_CameraPosition = vector.Zero;

		playerSpectator.COT_TempDisableOnSelectPlayer();

		g_Game.SelectPlayer( ident, NULL );

		vector transform[4];
		GetCommunityOnlineToolsBase().GetHeadTransform(spectateObject, transform);
		vector position = transform[3];

		g_Game.SelectSpectator( ident, "JMSpectatorCamera", position );

		playerSpectator.COTSetGodMode( true, false );  //! Enable godmode and remember previous state of GetAllowDamage
		playerSpectator.COTUpdateSpectatorPosition();

		Server_OnStartSpectating(spectateObject, ident);

		ScriptRPC rpc = new ScriptRPC();
		int networkLow, networkHigh;
		spectateObject.GetNetworkID(networkLow, networkHigh);
		rpc.Write(networkLow);
		rpc.Write(networkHigh);
		rpc.Send( NULL, JMPlayerModuleRPC.StartSpectating, true, ident );

		PlayerBase spectatePlayer;
		if (Class.CastTo(spectatePlayer, spectateObject) && spectatePlayer.GetAuthenticatedPlayer())
			GetCommunityOnlineToolsBase().Log( ident, "Spectating [guid=" + spectatePlayer.GetAuthenticatedPlayer().GetGUID() + "]" );
		else
			GetCommunityOnlineToolsBase().Log( ident, "Spectating " + spectateObject );
	}

	void Server_OnStartSpectating(Object spectateObject, PlayerIdentity ident)
	{
	}

	protected void Client_StartSpectating(Object spectateObject)
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "Client_StartSpectating").Add(spectateObject.ToString());
#endif

		CF_Log.Debug("JMPlayerModule::Client_StartSpectating " + spectateObject);
		
		if (COT_PreviousActiveCamera)
		{
			COT_PreviousActiveCamera.SetActive( false );

		#ifdef DIAG
			ErrorEx("g_Game.ObjectDeleteOnClient(COT_PreviousActiveCamera)", ErrorExSeverity.INFO);
		#endif
			g_Game.ObjectDeleteOnClient(COT_PreviousActiveCamera);

			COT_PreviousActiveCamera = null;
		}

		if ( CurrentActiveCamera )
		{
			CurrentActiveCamera.SelectedTarget( spectateObject );
			CurrentActiveCamera.SetActive( true );
			m_SpectatorCamera = CurrentActiveCamera;
			
#ifdef JM_COT_DIAG_LOGGING
			Print(g_Game.GetPlayer());
#endif
			if ( GetPlayer() )
			{
#ifdef JM_COT_DIAG_LOGGING
				Print("Disabling input controller");
#endif
				GetPlayer().GetInputController().SetDisabled( true );
			}
		}

		Client_OnStartSpectating(spectateObject);
	}

	void Client_OnStartSpectating(Object spectateObject)
	{
	}

	protected void RPC_StartSpectating( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "RPC_StartSpectating").Add(senderRPC).Add(target.ToString());
#endif

		if ( IsMissionHost() )
		{
			if (!target)
			{
				string guid;
				if (!ctx.Read(guid))
					return;
			}

			JMPlayerInstance instance;
			if ( !senderRPC )
				return;
			if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_SPECTATE, senderRPC ) )
				return;

			if (!target)
				Server_StartSpectating(guid, senderRPC);
			else
				Server_StartSpectating(target, senderRPC);
		} else
		{
			int networkLow, networkHigh;
			if ( !ctx.Read( networkLow ) )
				return;
				
			if ( !ctx.Read( networkHigh ) )
				return;

			COT_PreviousActiveCamera = CurrentActiveCamera;

			CF_Log.Debug("Starting spectate, waiting for spectate object");
			Client_Check_StartSpectating(networkLow, networkHigh);
		}
	}

	void Client_Check_StartSpectating(int networkLow, int networkHigh)
	{
		Object spectateObject = g_Game.GetObjectByNetworkId(networkLow, networkHigh);
		if (!Class.CastTo(CurrentActiveCamera, Camera.GetCurrentCamera()) || !spectateObject)
			g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Client_Check_StartSpectating, 34, false, networkLow, networkHigh);
		else if (CurrentActiveCamera.IsInherited(JMSpectatorCamera))
			Client_StartSpectating(spectateObject);
	}

	void UpdateSpectatorPositions()
	{
		if ( !m_Spectators.Count() )
			return;

		foreach ( PlayerBase playerSpectator: m_Spectators )
		{
			if ( playerSpectator && playerSpectator.m_JM_SpectatedObject)
				playerSpectator.COTUpdateSpectatorPosition();
		}
	}

	void EndSpectating()
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "EndSpectating");
#endif

		if (!g_Game.IsMultiplayer())
			return;

		if (!JMPermissions.Has(JMConstants.PERM_PLAYER_SPECTATE))
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMPlayerModuleRPC.EndSpectating, true, NULL );
	}

	protected void Exec_EndSpectating( PlayerIdentity ident )
	{
		Server_EndSpectating( ident );
	}

	//! DEPRECATED - override Exec_EndSpectating( ident ) instead. Holds the body so
	//! DayZ-Expansion AI's `override Server_EndSpectating` keeps working.
	protected void Server_EndSpectating( PlayerIdentity ident )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "Exec_EndSpectating").Add(ident);
#endif

		PlayerBase playerSpectator = m_Spectators[ident.GetId()];
		CF_Log.Debug("JMPlayerModule::Exec_EndSpectating - spectator " + playerSpectator);
		if (!playerSpectator)
			return;

		playerSpectator.m_JM_SpectatedObject = null;
		m_Spectators.Remove( ident.GetId() );

#ifdef JM_COT_DIAG_LOGGING
		Print(playerSpectator);
#endif

		GetCommunityOnlineToolsBase().Log( ident, "Stopped spectating" );

		bool switchToPreviousCamera = true;
		int waitForPlayerIdleTimeout;
		CF_Log.Debug("JMPlayerModule::Exec_EndSpectating - freecam position " + playerSpectator.m_JM_CameraPosition);
		if (playerSpectator.m_JM_CameraPosition == vector.Zero)
		{
			switchToPreviousCamera = false;

			vector spectatorPosition = playerSpectator.GetPosition();
			playerSpectator.COTResetSpectator();

			if ( playerSpectator.HasLastPosition() )
			{
				if (COT_SurfaceIsWater(playerSpectator.GetLastPosition()))
					waitForPlayerIdleTimeout = 250;
				else
					waitForPlayerIdleTimeout = 5000;
			}

			if (!waitForPlayerIdleTimeout)
				g_Game.SelectPlayer(ident, playerSpectator);
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(switchToPreviousCamera);
		rpc.Write(waitForPlayerIdleTimeout);
		rpc.Send( NULL, JMPlayerModuleRPC.EndSpectating, true, ident );
	}

	protected void Client_EndSpectating(bool switchToPreviousCamera, int waitForPlayerIdleTimeout )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "Client_EndSpectating");
#endif
		CF_Log.Debug("JMPlayerModule::Client_EndSpectating - switch to prev cam requested? " + switchToPreviousCamera);
		CF_Log.Debug("JMPlayerModule::Client_EndSpectating - current cam " + CurrentActiveCamera);
		CF_Log.Debug("JMPlayerModule::Client_EndSpectating - prev cam " + COT_PreviousActiveCamera);
		m_SpectatorCamera.SelectedTarget( NULL );

		if ( CurrentActiveCamera == m_SpectatorCamera )
		{
			CurrentActiveCamera.SetActive( false );

		#ifdef DIAG
			ErrorEx("g_Game.ObjectDeleteOnClient(CurrentActiveCamera)", ErrorExSeverity.INFO);
		#endif
			g_Game.ObjectDeleteOnClient(CurrentActiveCamera);

			CurrentActiveCamera = NULL;

			if (COT_PreviousActiveCamera && COT_PreviousActiveCamera.IsInherited(JMCinematicCamera) && switchToPreviousCamera)
			{
				CF_Log.Debug("JMPlayerModule::Client_EndSpectating - switching to prev cam " + COT_PreviousActiveCamera);
				CurrentActiveCamera = COT_PreviousActiveCamera;
				CurrentActiveCamera.SetActive(true);
				waitForPlayerIdleTimeout = 0;
			}
			else
			{
				CF_Log.Debug("JMPlayerModule::Client_EndSpectating - leaving current cam " + CurrentActiveCamera);
				PPEffects.ResetDOFOverride();

				CF_Log.Debug("JMPlayerModule::Client_EndSpectating - player " + m_SpectatorClient);
				CF_Log.Debug("JMPlayerModule::Client_EndSpectating - player is game player? " + (m_SpectatorClient == g_Game.GetPlayer()));
				if ( m_SpectatorClient )
				{
					m_SpectatorClient.GetInputController().SetDisabled( false );
				}
			}
		}
		else if (CurrentActiveCamera)
		{
			waitForPlayerIdleTimeout = 0;
		}

		if (COT_PreviousActiveCamera && COT_PreviousActiveCamera.IsInherited(JMSpectatorCamera))
		{
		#ifdef DIAG
			ErrorEx("g_Game.ObjectDeleteOnClient(COT_PreviousActiveCamera)", ErrorExSeverity.INFO);
		#endif
			g_Game.ObjectDeleteOnClient(COT_PreviousActiveCamera);
		}

		COT_PreviousActiveCamera = NULL;

		m_SpectatorCamera = NULL;

		if (waitForPlayerIdleTimeout)
		{
			CF_Log.Debug("JMPlayerModule::Client_EndSpectating - waiting for player to be idle");
			m_SpectatorClient.COT_EnableBonePositionUpdate(true);
			Client_Check_EndSpectating(m_SpectatorClient, waitForPlayerIdleTimeout);
		}

		if (waitForPlayerIdleTimeout > 1000)
			COTCreateLocalAdminNotification(new StringLocaliser("#STR_COT_PLAYER_MODULE_SPECTATE_STOPPING"));
		else
			COTCreateLocalAdminNotification(new StringLocaliser("#STR_COT_PLAYER_MODULE_SPECTATE_STOPPED"));

		CF_Log.Debug("JMPlayerModule::Client_EndSpectating - stopped spectating");
	}

	void Client_Check_EndSpectating(PlayerBase playerSpectator, int waitForPlayerIdleTimeout)
	{
		if (!playerSpectator.COT_IsAnimationIdle() && waitForPlayerIdleTimeout > 0)
		{
			g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( Client_Check_EndSpectating, 250, false, playerSpectator, waitForPlayerIdleTimeout - 250 );
		}
		else
		{
			CF_Log.Debug("JMPlayerModule::Client_Check_EndSpectating - player idle");
			playerSpectator.COT_EnableBonePositionUpdate(false);
			COTCreateLocalAdminNotification(new StringLocaliser("#STR_COT_PLAYER_MODULE_SPECTATE_STOPPED_HINT"), "set:ccgui_enforce image:HudBuild", 5);

			ScriptRPC rpc = new ScriptRPC();
			rpc.Send(NULL, JMPlayerModuleRPC.EndSpectating_Finish, true, NULL);
		}
	}

	protected void RPC_EndSpectating( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "RPC_EndSpectating").Add(senderRPC).Add(target);
#endif
		CF_Log.Debug("JMPlayerModule::RPC_EndSpectating");
		if ( IsMissionHost() )
		{
			JMPlayerInstance instance;
			if ( !senderRPC )
				return;
			if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_SPECTATE, senderRPC ) )
				return;

			Exec_EndSpectating( senderRPC );
		} else
		{
			bool switchToPreviousCamera;
			if ( !ctx.Read( switchToPreviousCamera ) )
				return;

			int waitForPlayerIdleTimeout;
			if ( !ctx.Read( waitForPlayerIdleTimeout ) )
				return;

			Client_EndSpectating(switchToPreviousCamera, waitForPlayerIdleTimeout);
		}
	}

	protected void RPC_EndSpectating_Finish( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "RPC_EndSpectating_Finish").Add(senderRPC).Add(target);
#endif
		CF_Log.Debug("JMPlayerModule::RPC_EndSpectating_Finish");
		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_SPECTATE, senderRPC ) )
			return;

		g_Game.SelectPlayer(senderRPC, senderRPC.GetPlayer());
	}

	void ProcessToggle(string toggle, bool value, JMPlayerInstance player, array<JMPlayerInstance> affectedPlayers, PlayerIdentity ident, JMPlayerInstance instance)
	{
		affectedPlayers.Insert(player);

		GetCommunityOnlineToolsBase().Log( ident, "Set " + toggle + " to " + value + " [guid=" + player.GetGUID() + "]" );

		if ( value )
			SendWebhookColored( "Set", instance, "Gave " + player.FormatSteamWebhook() + " " + toggle, JMConstants.WEBHOOK_COLOR_WARNING );
		else
			SendWebhookColored( "Set", instance, "Removed " + player.FormatSteamWebhook() + " " + toggle, JMConstants.WEBHOOK_COLOR_WARNING );

		player.Update();
	}

	//! Maps a toggle's internal id (as passed to ProcessToggle/ShowToggleNotification,
	//! e.g. "god mode", "unlimited ammo") to its localization key. An id this
	//! list does not know about comes back unchanged rather than blanked.
	protected string ToggleDisplayLabel( string toggle )
	{
		array< ref JMPlayerToggle > registeredToggles = GetToggles();
		foreach ( JMPlayerToggle candidate : registeredToggles )
		{
			if ( candidate.GetName() == toggle )
				return candidate.GetLabelKey();
		}

		return toggle;
	}

	void ShowToggleNotification(string toggle, bool value, array<JMPlayerInstance> players, PlayerIdentity ident)
	{
		string toggleLabel = Widget.TranslateString( ToggleDisplayLabel( toggle ) );

		string message;
		if ( players.Count() > 0 )
		{
			if ( value )
				message = COT_String.TranslateEx( "#STR_COT_PLAYER_MODULE_TOGGLE_ENABLED_FMT" , toggleLabel );
			else
				message = COT_String.TranslateEx( "#STR_COT_PLAYER_MODULE_TOGGLE_DISABLED_FMT" , toggleLabel );

			if ( players.Count() > 1 )
				message += " " + COT_String.TranslateEx( "#STR_COT_PLAYER_MODULE_TOGGLE_FOR_N_PLAYERS_FMT" , players.Count().ToString() );
			else if ( ident && ident.GetId() != players[0].GetGUID() )
				message += " " + COT_String.TranslateEx( "#STR_COT_PLAYER_MODULE_TOGGLE_FOR_PLAYER_FMT" , players[0].GetName() );
			else
				message += " " + Widget.TranslateString( "#STR_COT_PLAYER_MODULE_TOGGLE_FOR_YOURSELF" );
		}
		else
		{
			message = COT_String.TranslateEx( "#STR_COT_PLAYER_MODULE_TOGGLE_FAILED_FMT" , toggleLabel );
		}

		COTCreateNotification( ident, new StringLocaliser( message ) );
	}

	protected void RPC_VONStartedTransmitting( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if (!g_Game.IsServer())
			return;

		if (!IsOwnPlayer(target, senderRPC))
			return;

		CF_Log.Info("%1::RPC_VONStartedTransmitting target %2", ToString(), target.ToString());
		PlayerBase player;
		if (!Class.CastTo(player, target) || !player.COTIsInvisible(JMInvisibilityType.DisableSimulation))
			return;

		player.COTSetInvisibilityOnly(JMInvisibilityType.Interactive);
	}

	protected void RPC_VONStoppedTransmitting( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if (!g_Game.IsServer())
			return;

		if (!IsOwnPlayer(target, senderRPC))
			return;

		CF_Log.Info("%1::RPC_VONStoppedTransmitting target %2", ToString(), target.ToString());
		PlayerBase player;
		if (!Class.CastTo(player, target) || !player.COTIsInvisible(JMInvisibilityType.Interactive))
			return;

		player.COTSetInvisibilityOnly(JMInvisibilityType.DisableSimulation);
	}

	void Vomit(float value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_Vomit(value, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.Vomit, true, NULL );
		}
	}

	protected void Exec_Vomit(float value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;
			
			if(!player.GetCommand_Vehicle())
			{
				SymptomBase vomitSymptom = player.GetSymptomManager().QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_VOMIT);
				if(vomitSymptom != NULL)
					vomitSymptom.SetDuration(value);
			}

			players[i].Update();

			GetCommunityOnlineTools().SetClient( players[i] );
		}
	}

	protected void RPC_Vomit( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		float value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_VOMIT, senderRPC, instance ) )
			return;

		Exec_Vomit(value, guids, senderRPC, instance );
	}

	protected void Exec_SetScale( float value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetScale(value);

			players[i].Update();

			GetCommunityOnlineTools().SetClient( players[i] );
		}
	}

	protected void RPC_SetScale( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		float value;
		if ( !ctx.Read( value ) )
			return;

		value = Math.Clamp( value, 0.1, 10.0 );

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_SCALE, senderRPC, instance ) )
			return;

		Exec_SetScale( value, guids, senderRPC, instance );
	}

	void InputHeal( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		JMPlayerInstance instance;
		if (!JMPermissions.Has(JMConstants.PERM_PLAYER_HEAL, instance))
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			ShowInactiveNotification("STR_COT_INPUT_HEAL_SELF");
			return;
		}

		array< string > guids = {instance.GetGUID()};

		Heal(guids);
	}

	void Input_HealSelf( UAInput input )
	{
		if ( !input.LocalPress() ) return;

		JMPlayerInstance instance;
		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_HEAL, instance ) ) return;

		array< string > guids = {instance.GetGUID()};
		Heal( guids );
	}

	void Input_ToggleFly( UAInput input )
	{
		if ( !input.LocalPress() ) return;

		JMPlayerInstance instance;
		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_REMOVECOLLISION, instance ) ) return;

		bool value = !instance.GetRemoveCollision();
		array< string > guids = {instance.GetGUID()};
		SetRemoveCollision( value, guids );
	}

	void Heal( array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_Heal( guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.Heal, true, NULL );
		}
	}

	protected void Exec_Heal( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		bool includeAttachments = JMPermissions.Has( JMConstants.PERM_PLAYER_HEAL_ATTACHMENTS, ident );
		bool includeCargo = JMPermissions.Has( JMConstants.PERM_PLAYER_HEAL_CARGO, ident );
		
		int healedPlayers;

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			bool allowDamage = player.GetAllowDamage();
			if (!allowDamage)
				player.SetAllowDamage(true);

			if ( player.GetBleedingManagerServer() )
				player.GetBleedingManagerServer().RemoveAllSources();

			CommunityOnlineToolsBase.HealEntityRecursive(player, includeAttachments, includeCargo);
			player.SetBrokenLegs(eBrokenLegs.NO_BROKEN_LEGS);
			player.COTRemoveAllDiseases();

			player.GetStatEnergy().Set( player.GetStatEnergy().GetMax() );
			player.GetStatWater().Set( player.GetStatWater().GetMax() );

			healedPlayers++;

			GetCommunityOnlineToolsBase().Log( ident, "Healed [guid=" + players[i].GetGUID() + "]" );

			SendWebhookColored( "Set", instance, "Healed " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_SUCCESS );

			if (!allowDamage)
				player.SetAllowDamage(false);

			players[i].Update();
		}

		string message;
		if ( healedPlayers > 0 )
			message = Widget.TranslateString( "#STR_COT_PLAYER_MODULE_HEAL_SUCCESS" );
		else
			message = Widget.TranslateString( "#STR_COT_PLAYER_MODULE_HEAL_FAILED" );

		if ( players.Count() > 1 )
			message += " " + COT_String.TranslateEx( "#STR_COT_PLAYER_MODULE_HEAL_N_PLAYERS_FMT" , players.Count().ToString() );
		else if ( ident && ident.GetId() != guids[0] )
			message += " " + COT_String.TranslateEx( "#STR_COT_PLAYER_MODULE_HEAL_PLAYER_FMT" , players[0].GetName() );
		else
			message += " " + Widget.TranslateString( "#STR_COT_PLAYER_MODULE_HEAL_YOURSELF" );

		COTCreateNotification( ident, new StringLocaliser( message ) );
	}

	protected void RPC_Heal( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_HEAL, senderRPC, instance ) )
			return;

		Exec_Heal( guids, senderRPC, instance );
	}

	void Ban( array< string > guids, string messageText = "", int duration = -1 )
	{
		if ( IsMissionHost() )
		{
			Exec_Ban( guids, NULL, NULL, messageText, duration );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(guids);
			rpc.Write(messageText);
			rpc.Write(duration);
			rpc.Send( NULL, JMPlayerModuleRPC.Ban, true, NULL );
		}
	}

	protected void Exec_Ban( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL, string messageText = "", int duration = -1 )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		bool cantBanAdmin;
		foreach (JMPlayerInstance player: players)
		{
			if (!player.PlayerObject)
				continue;
			
			if (player.PlayerObject.GetIdentity() == ident)
				continue;
			
			// Because we dont want to ban other staff members
			if ( JMPermissions.Has( "COT", player.PlayerObject.GetIdentity(), player ) )
			{
				cantBanAdmin = true;
				continue;
			}

			SendBanMessage(player.PlayerObject.GetIdentity(), messageText, duration);

			//! Kick and Ban player after delay so client can still receive kickmessage RPC
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_Ban_Single, 500, false, player, ident, instance, messageText, duration);
		}

		if (cantBanAdmin)
			COTCreateNotification(ident, new StringLocaliser("#STR_COT_PLAYER_MODULE_CANT_BAN_ADMINS"));

		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SyncEvents.SendPlayerList, 1500);
	}

	protected void Exec_Ban_Single(JMPlayerInstance player, PlayerIdentity ident, JMPlayerInstance instance = NULL, string messageText = "", int duration = -1)
	{
		if (!g_Game || !player.PlayerObject)
			return;

		auto missionServer = MissionServer.Cast(g_Game.GetMission());

		if (!missionServer)
			return;

		player.PlayerObject.COTSetIsBeingKicked(true);

		g_Game.SendLogoutTime(player.PlayerObject, 0);

		missionServer.PlayerDisconnected(player.PlayerObject, player.PlayerObject.GetIdentity(), player.PlayerObject.GetIdentity().GetId());

		GetCommunityOnlineToolsBase().Log( ident, "Banned [guid=" + player.GetGUID() + "]" );

		SendWebhookColored( "Ban", instance, "Banned " + player.FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_DANGER );
	}

	protected void RPC_Ban( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		string messageText;
		if (!ctx.Read(messageText))
			return;

		int duration;
		if ( !ctx.Read( duration ) )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_BAN, senderRPC, instance ) )
			return;

		Exec_Ban( guids, senderRPC, instance, messageText, duration );
	}

	protected void SendBanMessage(PlayerIdentity identity, string messageText, int duration = -1)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(messageText);
		rpc.Send(NULL, JMPlayerModuleRPC.BanMessage, true, identity);
	}

	void Kick( array< string > guids, string messageText )
	{
		if ( IsMissionHost() )
		{
			Exec_Kick( guids, NULL, NULL, messageText );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Write(messageText);
			rpc.Send( NULL, JMPlayerModuleRPC.Kick, true, NULL );
		}
	}

	protected void Exec_Kick( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL, string messageText = ""  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		foreach (JMPlayerInstance player: players)
		{
			if (!player.PlayerObject)
				continue;

			SendKickMessage(player.PlayerObject.GetIdentity(), messageText);

			//! Kick player after delay so client can still receive kickmessage RPC
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_Kick_Single, 500, false, player, ident, instance);
		}

		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SyncEvents.SendPlayerList, 1500);
	}

	protected void Exec_Kick_Single(JMPlayerInstance player, PlayerIdentity ident, JMPlayerInstance instance = NULL)
	{
		if (!g_Game || !player.PlayerObject)
			return;

		auto missionServer = MissionServer.Cast(g_Game.GetMission());

		if (!missionServer)
			return;

		player.PlayerObject.COTSetIsBeingKicked(true);

		g_Game.SendLogoutTime(player.PlayerObject, 0);

		missionServer.PlayerDisconnected(player.PlayerObject, player.PlayerObject.GetIdentity(), player.PlayerObject.GetIdentity().GetId());

		GetCommunityOnlineToolsBase().Log( ident, "Kicked [guid=" + player.GetGUID() + "]" );

		SendWebhookColored( "Kick", instance, "Kicked " + player.FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_MODERATION );
	}

	protected void RPC_Kick( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		string messageText;
		if (!ctx.Read(messageText))
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_KICK, senderRPC, instance ) )
			return;

		Exec_Kick( guids, senderRPC, instance, messageText );
	}

	protected void SendKickMessage(PlayerIdentity identity, string messageText)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(messageText);
		rpc.Send(NULL, JMPlayerModuleRPC.KickMessage, true, identity);
	}

	//! Because there is no way to check on client if the sender had permissions to send a kick message,
	//! we queue the message for deferred display on the main menu screen.
	//! That way, if the RPC is abused (hacking etc), it won't affect the player during gameplay.
	protected void RPC_KickMessage(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		auto trace = CF_Trace_0(this, "RPC_KickMessage");

		if (g_Game.IsDedicatedServer())
			return;

		string messageText;
		if (!ctx.Read(messageText))
			return;

		if (messageText)
			messageText = "#STR_COT_NOTIFICATION_KICKED_BY_ADMIN: " + messageText;
		else
			messageText = "#STR_COT_NOTIFICATION_KICKED_BY_ADMIN";
		
		JMDeferredMessage.QueuedMessages.Clear();
		JMDeferredMessage.Queue("#STR_COT_NOTIFICATION_TITLE_ADMIN", messageText);
	}

	protected void RPC_BanMessage(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		auto trace = CF_Trace_0(this, "RPC_BanMessage");

		if (g_Game.IsDedicatedServer())
			return;

		string messageText;
		if (!ctx.Read(messageText))
			return;

		if (messageText)
			messageText = "#STR_COT_NOTIFICATION_BANNED_BY_ADMIN: " + messageText;
		else
			messageText = "#STR_COT_NOTIFICATION_BANNED_BY_ADMIN";
		
		JMDeferredMessage.QueuedMessages.Clear();
		JMDeferredMessage.Queue("#STR_COT_NOTIFICATION_TITLE_ADMIN", messageText);
	}

	void Strip( array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_Strip( guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.Strip, true, NULL );
		}
	}

	protected void Exec_Strip( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COT_RemoveAllItems();

			GetCommunityOnlineToolsBase().Log( ident, "Stripped [guid=" + players[i].GetGUID() + "]" );

			SendWebhookColored( "Inventory", instance, "Stripped " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_WARNING );

			players[i].Update();
		}
	}

	protected void RPC_Strip( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_STRIP, senderRPC, instance ) )
			return;

		Exec_Strip( guids, senderRPC, instance );
	}

	void ClearCargo( array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_ClearCargo( guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.ClearCargo, true, NULL );
		}
	}

	protected void Exec_ClearCargo( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COT_ClearCargo();

			GetCommunityOnlineToolsBase().Log( ident, "Cleared Cargo [guid=" + players[i].GetGUID() + "]" );

			SendWebhookColored( "Inventory", instance, "Cleared Cargo " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_WARNING );

			players[i].Update();
		}
	}

	protected void RPC_ClearCargo( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_CLEARCARGO, senderRPC, instance ) )
			return;

		Exec_ClearCargo( guids, senderRPC, instance );
	}

	void Dry( array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_Dry( guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.Dry, true, NULL );
		}
	}

	protected void Exec_Dry( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTResetItemWetness();

			GetCommunityOnlineToolsBase().Log( ident, "Dried [guid=" + players[i].GetGUID() + "]" );

			SendWebhookColored( "Inventory", instance, "Dried " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_INFO );

			players[i].Update();
		}
	}

	protected void RPC_Dry( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_DRY, senderRPC, instance ) )
			return;

		Exec_Dry( guids, senderRPC, instance );
	}

	void StopBleeding( array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_StopBleeding( guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.StopBleeding, true, NULL );
		}
	}

	protected void Exec_StopBleeding( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.GetBleedingManagerServer().RemoveAllSources();

			GetCommunityOnlineToolsBase().Log( ident, "Bleeding stopped [guid=" + players[i].GetGUID() + "]" );

			SendWebhookColored( "Set", instance, "Stopped " + players[i].FormatSteamWebhook() + " bleeding.", JMConstants.WEBHOOK_COLOR_SUCCESS );

			players[i].Update();
		}
	}

	protected void RPC_StopBleeding( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_STOPBLEEDING, senderRPC, instance ) )
			return;

		Exec_StopBleeding( guids, senderRPC, instance );
	}

	protected void Exec_SetPermissions( JMPermission permission, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		auto trace = CF_Trace_0(this, "Exec_SetPermissions");

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			players[i].CopyPermissions( permission );
			players[i].Save();

			players[i].Update();

			GetCommunityOnlineTools().SetClient( players[i] );

			GetCommunityOnlineToolsBase().Log( ident, "Updated permissions [guid=" + players[i].GetGUID() + "]" );

			SendWebhookColored( "PF", instance, "Updated permissions for " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_MODERATION );
		}
	}

	protected void RPC_SetPermissions( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !senderRPC )
			return;

		auto trace = CF_Trace_1(this, "RPC_SetPermissions").Add(senderRPC.GetId());

		JMPermission permission = new JMPermission( JMConstants.PERM_ROOT );
		permission.CopyPermissions(GetPermissionsManager().RootPermission);
		if ( !permission.OnReceive( ctx ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_PERMISSIONS, senderRPC, instance ) )
			return;

		Exec_SetPermissions( permission, guids, senderRPC, instance );
	}

	protected void Exec_SetRoles( array< string > roles, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL, map< string, string > nameRestrictions = NULL )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			players[i].LoadRoles( roles, nameRestrictions );

			players[i].Update();

			GetCommunityOnlineTools().SetClient( players[i] );

			GetCommunityOnlineToolsBase().Log( ident, "Updated roles [guid=" + players[i].GetGUID() + "]" );

			SendWebhookColored( "PF", instance, "Updated roles for " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_MODERATION );
		}
	}

	protected void RPC_SetRoles( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > roles;
		if ( !ctx.Read( roles ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		// Read optional name restriction arrays (added in later version - gracefully skip if absent)
		array< string > nrKeys   = new array< string >;
		array< string > nrValues = new array< string >;
		ctx.Read( nrKeys );
		ctx.Read( nrValues );

		map< string, string > nameRestrictions = new map< string, string >;
		int nrCount = Math.Min( nrKeys.Count(), nrValues.Count() );
		for ( int ni = 0; ni < nrCount; ni++ )
			nameRestrictions.Insert( nrKeys[ni], nrValues[ni] );

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_ROLES, senderRPC, instance ) )
			return;

		Exec_SetRoles( roles, guids, senderRPC, instance, nameRestrictions );
	}

	// ---------------- Disease Add / Remove ----------------
	// Bulk friendly: one RPC carries the agent id, count, and GUID list.
	// Server applies via PlayerBase.COTAddDisease/COTRemoveDisease helpers.

	void AddDisease( int agent, float count, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_AddDisease( agent, count, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( agent );
			rpc.Write( count );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.AddDisease, true, NULL );
		}
	}

	protected void Exec_AddDisease( int agent, float count, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		array<JMPlayerInstance> affectedPlayers = {};
		int affected = 0;

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTAddDisease( agent, count );

			affectedPlayers.Insert( players[i] );
			affected++;

			GetCommunityOnlineToolsBase().Log( ident, "Added disease agent=" + agent.ToString() + " count=" + count.ToString() + " [guid=" + players[i].GetGUID() + "]" );
			SendWebhookColored( "Disease", instance, "Added disease (agent=" + agent.ToString() + ", count=" + count.ToString() + ") for " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_WARNING );

			players[i].Update();
		}

		if ( affected > 0 )
		{
			foreach ( JMPlayerInstance pi : affectedPlayers )
			{
				GetCommunityOnlineTools().SetClient( pi );
			}
		}
	}

	protected void RPC_AddDisease( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int agent;
		if ( !ctx.Read( agent ) )
			return;

		float count;
		if ( !ctx.Read( count ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_DISEASE_ADD, senderRPC, instance ) )
			return;

		Exec_AddDisease( agent, count, guids, senderRPC, instance );
	}

	void RemoveDisease( int agent, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_RemoveDisease( agent, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( agent );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.RemoveDisease, true, NULL );
		}
	}

	protected void Exec_RemoveDisease( int agent, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		array<JMPlayerInstance> affectedPlayers = {};
		int affected = 0;

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTRemoveDisease( agent );

			affectedPlayers.Insert( players[i] );
			affected++;

			GetCommunityOnlineToolsBase().Log( ident, "Removed disease agent=" + agent.ToString() + " [guid=" + players[i].GetGUID() + "]" );
			SendWebhookColored( "Disease", instance, "Removed disease (agent=" + agent.ToString() + ") for " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_WARNING );

			players[i].Update();
		}

		if ( affected > 0 )
		{
			foreach ( JMPlayerInstance pi : affectedPlayers )
			{
				GetCommunityOnlineTools().SetClient( pi );
			}
		}
	}

	protected void RPC_RemoveDisease( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int agent;
		if ( !ctx.Read( agent ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_DISEASE_REMOVE, senderRPC, instance ) )
			return;

		Exec_RemoveDisease( agent, guids, senderRPC, instance );
	}

	// ---------------- Modifier Activate / Deactivate ----------------
	// For diseases with no eAgents backing (Heatstroke stages, Sandstorm Exposure) -
	// toggled directly through eModifiers rather than the agent pool.

	void ActivateModifier( int modifier_id, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_ActivateModifier( modifier_id, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( modifier_id );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.ActivateModifier, true, NULL );
		}
	}

	protected void Exec_ActivateModifier( int modifier_id, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		array<JMPlayerInstance> affectedPlayers = {};
		int affected = 0;

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTActivateModifier( modifier_id );

			affectedPlayers.Insert( players[i] );
			affected++;

			GetCommunityOnlineToolsBase().Log( ident, "Activated modifier=" + modifier_id.ToString() + " [guid=" + players[i].GetGUID() + "]" );
			SendWebhookColored( "Disease", instance, "Activated modifier (id=" + modifier_id.ToString() + ") for " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_WARNING );

			players[i].Update();
		}

		if ( affected > 0 )
		{
			foreach ( JMPlayerInstance pi : affectedPlayers )
			{
				GetCommunityOnlineTools().SetClient( pi );
			}
		}
	}

	protected void RPC_ActivateModifier( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int modifier_id;
		if ( !ctx.Read( modifier_id ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_DISEASE_ADD, senderRPC, instance ) )
			return;

		Exec_ActivateModifier( modifier_id, guids, senderRPC, instance );
	}

	void DeactivateModifier( int modifier_id, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_DeactivateModifier( modifier_id, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( modifier_id );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.DeactivateModifier, true, NULL );
		}
	}

	protected void Exec_DeactivateModifier( int modifier_id, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		array<JMPlayerInstance> affectedPlayers = {};
		int affected = 0;

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTDeactivateModifier( modifier_id );

			affectedPlayers.Insert( players[i] );
			affected++;

			GetCommunityOnlineToolsBase().Log( ident, "Deactivated modifier=" + modifier_id.ToString() + " [guid=" + players[i].GetGUID() + "]" );
			SendWebhookColored( "Disease", instance, "Deactivated modifier (id=" + modifier_id.ToString() + ") for " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_SUCCESS );

			players[i].Update();
		}

		if ( affected > 0 )
		{
			foreach ( JMPlayerInstance pi : affectedPlayers )
			{
				GetCommunityOnlineTools().SetClient( pi );
			}
		}
	}

	protected void RPC_DeactivateModifier( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int modifier_id;
		if ( !ctx.Read( modifier_id ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_DISEASE_REMOVE, senderRPC, instance ) )
			return;

		Exec_DeactivateModifier( modifier_id, guids, senderRPC, instance );
	}

	void RemoveAllDiseases( array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_RemoveAllDiseases( guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.RemoveAllDiseases, true, NULL );
		}
	}

	protected void Exec_RemoveAllDiseases( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		array<JMPlayerInstance> affectedPlayers = {};
		int affected = 0;

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTRemoveAllDiseases();

			affectedPlayers.Insert( players[i] );
			affected++;

			GetCommunityOnlineToolsBase().Log( ident, "Cleared all diseases [guid=" + players[i].GetGUID() + "]" );
			SendWebhookColored( "Disease", instance, "Cleared all diseases for " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_SUCCESS );

			players[i].Update();
		}

		if ( affected > 0 )
		{
			foreach ( JMPlayerInstance pi : affectedPlayers )
			{
				GetCommunityOnlineTools().SetClient( pi );
			}
		}
	}

	protected void RPC_RemoveAllDiseases( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_DISEASE_REMOVE, senderRPC, instance ) )
			return;

		Exec_RemoveAllDiseases( guids, senderRPC, instance );
	}

	// ---------------- Disease mask sync (server -> client) ----------------
	// Client requests the current disease bitmask for a single player (the one
	// currently selected in the form). Server reads the live AgentPool bits
	// and pushes them back. Cheap: one bitmask per form-show.

	void RequestDiseaseMask( string guid )
	{
		if ( IsMissionHost() )
		{
			SendDiseaseMaskTo( NULL, guid );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( guid );
		rpc.Send( NULL, JMPlayerModuleRPC.SendDiseaseMask, true, NULL );
	}

	protected void RPC_SendDiseaseMask( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		// Server-side: client requested a mask - read the guid and send back
		if ( IsMissionHost() )
		{
			string guid;
			if ( !ctx.Read( guid ) )
				return;

			JMPlayerInstance instance;
			if ( !senderRPC )
				return;
			if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_DISEASE_ADD, senderRPC, instance ) )
				return;

			SendDiseaseMaskTo( senderRPC, guid );
			return;
		}

		// Client-side: server pushed a mask - populate the form
		string maskGuid;
		int cholera, influenza, salmonella, brain, foodPoison, chemPoison, wound, nerve, heavyMetal;

		if ( !ctx.Read( maskGuid ) )    return;
		if ( !ctx.Read( cholera ) )     return;
		if ( !ctx.Read( influenza ) )   return;
		if ( !ctx.Read( salmonella ) )  return;
		if ( !ctx.Read( brain ) )       return;
		if ( !ctx.Read( foodPoison ) )  return;
		if ( !ctx.Read( chemPoison ) )  return;
		if ( !ctx.Read( wound ) )       return;
		if ( !ctx.Read( nerve ) )       return;
		if ( !ctx.Read( heavyMetal ) )  return;

		JMPlayerForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.OnDiseaseMaskUpdated( maskGuid, cholera, influenza, salmonella, brain, foodPoison, chemPoison, wound, nerve, heavyMetal );
	}

	protected void SendDiseaseMaskTo( PlayerIdentity to, string guid )
	{
		int mask = 0;
		int cholera = 0, influenza = 0, salmonella = 0, brain = 0;
		int foodPoison = 0, chemPoison = 0, wound = 0, nerve = 0, heavyMetal = 0;

		if ( guid != "" )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( { guid } );
			if ( players.Count() > 0 )
			{
				PlayerBase player = PlayerBase.Cast( players[0].PlayerObject );
				if ( player )
				{
					cholera    = player.COTGetAgentCount( eAgents.CHOLERA );
					influenza  = player.COTGetAgentCount( eAgents.INFLUENZA );
					salmonella = player.COTGetAgentCount( eAgents.SALMONELLA );
					brain      = player.COTGetAgentCount( eAgents.BRAIN );
					foodPoison = player.COTGetAgentCount( eAgents.FOOD_POISON );
					chemPoison = player.COTGetAgentCount( eAgents.CHEMICAL_POISON );
					wound      = player.COTGetAgentCount( eAgents.WOUND_AGENT );
					nerve      = player.COTGetAgentCount( eAgents.NERVE_AGENT );
					heavyMetal = player.COTGetAgentCount( eAgents.HEAVYMETAL );
				}
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( guid );
		rpc.Write( cholera );
		rpc.Write( influenza );
		rpc.Write( salmonella );
		rpc.Write( brain );
		rpc.Write( foodPoison );
		rpc.Write( chemPoison );
		rpc.Write( wound );
		rpc.Write( nerve );
		rpc.Write( heavyMetal );
		rpc.Send( NULL, JMPlayerModuleRPC.SendDiseaseMask, true, to );
	}

	protected void RPC_ReceiveDiseaseMask( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		// Removed - combined into RPC_SendDiseaseMask above.
	}

	// ---------------- Session statistics (server -> client) ----------------
	//
	// Playtime, deaths and lifespan live in the per-player JSON on the server
	// and are pulled on demand, the same way the disease mask is.
	//
	// Deliberately NOT folded into JMPlayerInstance.OnSendHealth: that runs for
	// every roster entry several times a second, so seven more ints per player
	// would be a real bandwidth cost on a full server for numbers nobody is
	// looking at unless the Statistics tab happens to be open.

	void RequestPlayerStats( string guid )
	{
		if ( IsMissionHost() )
		{
			SendPlayerStatsTo( NULL, guid );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( guid );
		rpc.Send( NULL, JMPlayerModuleRPC.RequestPlayerStats, true, NULL );
	}

	protected void RPC_RequestPlayerStats( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		// Server-side: a client asked for a player history.
		if ( IsMissionHost() )
		{
			string guid;
			if ( !ctx.Read( guid ) )
				return;

			JMPlayerInstance instance;
			if ( !senderRPC )
				return;
			if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_STATISTICS_VIEW, senderRPC, instance ) )
				return;

			SendPlayerStatsTo( senderRPC, guid );
			return;
		}

		// Client-side: the server pushed a history.
		string statsGuid;
		int playtime, sessions, firstSeen, lastSeen, deaths, longestLife, currentLife;

		if ( !ctx.Read( statsGuid ) )   return;
		if ( !ctx.Read( playtime ) )    return;
		if ( !ctx.Read( sessions ) )    return;
		if ( !ctx.Read( firstSeen ) )   return;
		if ( !ctx.Read( lastSeen ) )    return;
		if ( !ctx.Read( deaths ) )      return;
		if ( !ctx.Read( longestLife ) ) return;
		if ( !ctx.Read( currentLife ) ) return;

		JMPlayerStats stats = new JMPlayerStats();
		stats.TotalPlaytimeSec     = playtime;
		stats.SessionCount         = sessions;
		stats.FirstSeenUnix        = firstSeen;
		stats.LastSeenUnix         = lastSeen;
		stats.Deaths               = deaths;
		stats.LongestLifeSec       = longestLife;
		stats.CurrentLifeStartUnix = currentLife;

		JMPlayerForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.OnPlayerStatsUpdated( statsGuid, stats );
	}

	protected void SendPlayerStatsTo( PlayerIdentity to, string guid )
	{
		int playtime = 0;
		int sessions = 0;
		int firstSeen = 0;
		int lastSeen = 0;
		int deaths = 0;
		int longestLife = 0;
		int currentLife = 0;

	#ifndef CF_MODULE_PERMISSIONS
		if ( guid != "" )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( { guid } );
			if ( players.Count() > 0 )
			{
				JMPlayerStats stats = players[0].GetStats();
				if ( stats )
				{
					// The live figure, not the stored one: an admin looking at a
					// connected player expects the session in progress to count.
					playtime    = stats.GetLivePlaytimeSeconds();
					sessions    = stats.SessionCount;
					firstSeen   = stats.FirstSeenUnix;
					lastSeen    = stats.LastSeenUnix;
					deaths      = stats.Deaths;
					longestLife = stats.LongestLifeSec;
					currentLife = stats.CurrentLifeStartUnix;
				}
			}
		}
	#endif

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( guid );
		rpc.Write( playtime );
		rpc.Write( sessions );
		rpc.Write( firstSeen );
		rpc.Write( lastSeen );
		rpc.Write( deaths );
		rpc.Write( longestLife );
		rpc.Write( currentLife );
		rpc.Send( NULL, JMPlayerModuleRPC.RequestPlayerStats, true, to );
	}

	void RequestExpansionInfo( string guid )
	{
		if ( IsMissionHost() )
		{
			SendExpansionInfoTo( NULL, guid );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( guid );
		rpc.Send( NULL, JMPlayerModuleRPC.RequestExpansionInfo, true, NULL );
	}

	protected void RPC_RequestExpansionInfo( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		// Server-side: a client asked for the Expansion block.
		if ( IsMissionHost() )
		{
			string guid;
			if ( !ctx.Read( guid ) )
				return;

			JMPlayerInstance instance;
			if ( !senderRPC )
				return;
			if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_STATISTICS_VIEW, senderRPC, instance ) )
				return;

			SendExpansionInfoTo( senderRPC, guid );
			return;
		}

		// Client-side: the server pushed a block.
		string infoGuid;
		int count;

		if ( !ctx.Read( infoGuid ) ) return;
		if ( !ctx.Read( count ) )    return;

		if ( count < 0 || count > EXP_INFO_MAX_ROWS )
			return;

		array< string > ids    = new array< string >;
		array< string > values = new array< string >;

		for ( int i = 0; i < count; i++ )
		{
			string id;
			string value;

			// A short read leaves the rest of the payload unusable; render
			// nothing rather than a half-decoded block.
			if ( !ctx.Read( id ) )    return;
			if ( !ctx.Read( value ) ) return;

			ids.Insert( id );
			values.Insert( value );
		}

		JMPlayerForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.OnExpansionInfoUpdated( infoGuid, ids, values );
	}

	protected void SendExpansionInfoTo( PlayerIdentity to, string guid )
	{
		array< string > ids    = new array< string >;
		array< string > values = new array< string >;

	#ifdef DZ_Expansion_Core
		if ( guid != "" )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( { guid } );
			if ( players.Count() > 0 )
				CollectExpansionInfo( guid, PlayerBase.Cast( players[0].PlayerObject ), ids, values );
		}
	#endif

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( guid );
		rpc.Write( ids.Count() );

		for ( int i = 0; i < ids.Count(); i++ )
		{
			rpc.Write( ids[i] );
			rpc.Write( values[i] );
		}

		rpc.Send( NULL, JMPlayerModuleRPC.RequestExpansionInfo, true, to );
	}

#ifdef DZ_Expansion_Core
	//! Every block is independently guarded: the four features ship as four
	//! separate Expansion mods and a server can be running any subset of them.
	//! A feature that is loaded but has nothing to say for this player - no ATM
	//! record, no group - contributes no row rather than an empty one.
	protected void CollectExpansionInfo( string guid, PlayerBase player, out array< string > ids, out array< string > values )
	{
	#ifdef DZ_Expansion_Market
		ExpansionMarketModule marketModule = ExpansionMarketModule.Cast( CF_ModuleCoreManager.Get( ExpansionMarketModule ) );
		if ( marketModule )
		{
			// The ATM file is keyed by the same identity id COT calls the GUID.
			ExpansionMarketATM_Data atm = marketModule.GetPlayerATMData( guid );
			if ( atm )
			{
				ids.Insert( EXP_INFO_MONEY );
				values.Insert( atm.MoneyDeposited.ToString() );
			}
		}
	#endif

	#ifdef DZ_Expansion_AI
		if ( player && player.GetGroup() && player.GetGroup().GetFaction() )
		{
			ids.Insert( EXP_INFO_FACTION );
			values.Insert( player.GetGroup().GetFaction().GetName() );
		}
	#endif

	#ifdef DZ_Expansion_Hardline
		if ( player )
		{
			ids.Insert( EXP_INFO_REPUTATION );
			values.Insert( player.Expansion_GetReputation().ToString() );
		}
	#endif

	#ifdef DZ_Expansion_Groups
		if ( player && player.Expansion_GetParty() )
		{
			ids.Insert( EXP_INFO_GROUP );
			values.Insert( player.Expansion_GetParty().GetPartyName() );
		}
	#endif
	}
#endif

	// ---------------- Inventory listing and item operations ----------------
	//
	// The client has no inventory data of its own: JMPlayerInstance syncs stats
	// and position, never cargo. So the tab asks for a snapshot when it is
	// opened and after every operation, and never on a timer - an unrequested
	// ~10 KB per second per connected admin is not something a full server can
	// afford for a panel nobody may be looking at.
	//
	// Same request/response shape as SendDiseaseMask: one RPC id, one handler,
	// branching on IsMissionHost() for the server and client halves.

	void RequestInventory( string guid )
	{
		if ( IsMissionHost() )
		{
			SendInventoryTo( NULL, guid );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( guid );
		rpc.Send( NULL, JMPlayerModuleRPC.RequestInventory, true, NULL );
	}

	protected void RPC_RequestInventory( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		// Server-side: a client asked for a listing.
		if ( IsMissionHost() )
		{
			string guid;
			if ( !ctx.Read( guid ) )
				return;

			JMPlayerInstance instance;
			if ( !senderRPC )
				return;
			if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_ACCESSINVENTORY, senderRPC, instance ) )
				return;

			SendInventoryTo( senderRPC, guid );
			return;
		}

		// Client-side: the server pushed a listing.
		string listGuid;
		bool truncated;
		int count;

		if ( !ctx.Read( listGuid ) )  return;
		if ( !ctx.Read( truncated ) ) return;
		if ( !ctx.Read( count ) )     return;

		if ( count < 0 || count > JMConstants.INVENTORY_MAX_ITEMS )
			return;

		array< ref JMPlayerInventoryItem > items = new array< ref JMPlayerInventoryItem >;

		for ( int i = 0; i < count; i++ )
		{
			JMPlayerInventoryItem item = new JMPlayerInventoryItem();

			// A short read means the rest of the payload is unusable; render
			// nothing rather than a half-decoded list.
			if ( !item.OnReceive( ctx ) )
				return;

			items.Insert( item );
		}

		JMPlayerForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.OnInventoryUpdated( listGuid, items, truncated );
	}

	protected void SendInventoryTo( PlayerIdentity to, string guid )
	{
		array< ref JMPlayerInventoryItem > items = new array< ref JMPlayerInventoryItem >;
		bool truncated = false;

		if ( guid != "" )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( { guid } );
			if ( players.Count() > 0 )
			{
				PlayerBase player = PlayerBase.Cast( players[0].PlayerObject );
				if ( player && player.GetInventory() )
					truncated = CollectInventory( player, items );
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( guid );
		rpc.Write( truncated );
		rpc.Write( items.Count() );

		foreach ( JMPlayerInventoryItem item : items )
			item.OnSend( rpc );

		rpc.Send( NULL, JMPlayerModuleRPC.RequestInventory, true, to );
	}

	//! Flatten the player inventory into `items`. Returns true when the cap cut
	//! the listing short.
	//!
	//! PREORDER visits a parent before its children, so by the time a child is
	//! reached its parent is already in `indexOf` and ParentIndex resolves with
	//! no recursion and no second pass.
	protected bool CollectInventory( PlayerBase player, array< ref JMPlayerInventoryItem > items )
	{
		array< EntityAI > entities = {};
		player.GetInventory().EnumerateInventory( InventoryTraversalType.PREORDER, entities );

		map< EntityAI, int > indexOf = new map< EntityAI, int >;

		foreach ( EntityAI entity : entities )
		{
			if ( !entity )
				continue;

			// The traversal includes the player itself.
			if ( entity == player )
				continue;

			if ( items.Count() >= JMConstants.INVENTORY_MAX_ITEMS )
				return true;

			JMPlayerInventoryItem item = new JMPlayerInventoryItem();

			int netLow, netHigh;
			entity.GetNetworkID( netLow, netHigh );
			item.NetIdLow  = netLow;
			item.NetIdHigh = netHigh;
			item.Type      = entity.GetType();

			EntityAI parent = entity.GetHierarchyParent();
			int parentIndex = -1;
			if ( parent && indexOf.Contains( parent ) )
				parentIndex = indexOf.Get( parent );

			item.ParentIndex = parentIndex;

			int depth = 0;
			if ( parentIndex >= 0 )
				depth = items[parentIndex].Depth + 1;

			item.Depth = depth;

			int flags = 0;
			if ( entity.GetInventory() && entity.GetInventory().IsInCargo() )
				flags = flags | JMPlayerInventoryItem.FLAG_CARGO;

			InventoryLocation loc = new InventoryLocation();
			if ( entity.GetInventory() && entity.GetInventory().GetCurrentInventoryLocation( loc ) )
			{
				if ( loc.GetType() == InventoryLocationType.ATTACHMENT )
				{
					flags = flags | JMPlayerInventoryItem.FLAG_ATTACHMENT;
					item.SlotName = InventorySlots.GetSlotName( loc.GetSlot() );
				}
			}

			if ( entity.IsWeapon() )
				flags = flags | JMPlayerInventoryItem.FLAG_WEAPON;

			if ( entity.IsMagazine() )
				flags = flags | JMPlayerInventoryItem.FLAG_MAGAZINE;

			// GetHealthLevel is the coarse 0-4 state; the health value itself is
			// what the admin wants, on the same 0-100 scale the vitals use.
			item.Health      = entity.GetHealth( "", "" );
			item.MaxHealth   = entity.GetMaxHealth( "", "" );
			item.HealthLevel = entity.GetHealthLevel();

			if ( item.MaxHealth > 0 && item.Health <= 0 )
				flags = flags | JMPlayerInventoryItem.FLAG_RUINED;

			// A magazine's quantity is its AMMO COUNT. Reading GetQuantity on
			// one answers 1 of 1 - the magazine itself - which is how a full
			// 30-round mag reached the admin as "1/1".
			Magazine asMag;
			ItemBase asItem;
			Class.CastTo( asItem, entity );

			if ( Class.CastTo( asMag, entity ) )
			{
				item.Quantity    = asMag.GetAmmoCount();
				item.QuantityMax = asMag.GetAmmoMax();

				// An ammo pile that reaches 0 deletes itself, so its floor is 1
				// round - the same floor the object spawner's quantity slider
				// uses for one.
				if ( asMag.IsAmmoPile() && asMag.GetAmmoMax() > 1 )
					item.QuantityMin = 1;
			}
			else if ( asItem )
			{
				item.Quantity    = asItem.GetQuantity();
				item.QuantityMax = asItem.GetQuantityMax();
				item.QuantityMin = asItem.GetQuantityMin();
			}

			// State the per-item edits need. All of it is read off the LIVE
			// entity: a jam and a half-drunk canteen are runtime facts the
			// client cannot look up in a config.
			Weapon_Base asWeapon;
			if ( Class.CastTo( asWeapon, entity ) && asWeapon.IsJammed() )
				flags = flags | JMPlayerInventoryItem.FLAG_JAMMED;

			if ( asItem )
			{
				item.Temperature = asItem.GetTemperature();

				if ( asItem.IsLiquidContainer() )
				{
					flags = flags | JMPlayerInventoryItem.FLAG_LIQUID;
					item.LiquidType = asItem.GetLiquidType();
				}

				Edible_Base asFood;
				if ( asItem.HasFoodStage() && Class.CastTo( asFood, asItem ) && asFood.GetFoodStage() )
				{
					flags = flags | JMPlayerInventoryItem.FLAG_FOOD;
					item.Stage = asFood.GetFoodStage().GetFoodStageType();
				}
			}

			item.Flags = flags;

			indexOf.Insert( entity, items.Count() );
			items.Insert( item );
		}

		return false;
	}

	//! Resolve a client-supplied network ID to an entity.
	//!
	//! SECURITY: the ID is matched ONLY against the named target player own
	//! inventory. Resolving it against the world instead would let a client
	//! delete or take any object on the server by guessing an ID, with the
	//! permission check passing because the permission is per-admin, not
	//! per-object.
	protected EntityAI ResolveInventoryItem( PlayerBase player, int netLow, int netHigh )
	{
		if ( !player || !player.GetInventory() )
			return NULL;

		array< EntityAI > entities = {};
		player.GetInventory().EnumerateInventory( InventoryTraversalType.PREORDER, entities );

		foreach ( EntityAI entity : entities )
		{
			if ( !entity || entity == player )
				continue;

			int entLow, entHigh;
			entity.GetNetworkID( entLow, entHigh );

			if ( entLow == netLow && entHigh == netHigh )
				return entity;
		}

		return NULL;
	}

	//! Shared client half of the three item operations.
	protected void SendInventoryOp( int rpcId, string guid, int netLow, int netHigh )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( guid );
		rpc.Write( netLow );
		rpc.Write( netHigh );
		rpc.Send( NULL, rpcId, true, NULL );
	}

	//! Shared server half: read the request, check the permission, resolve the
	//! entity against the target inventory. Returns NULL when any step fails.
	protected EntityAI ReadInventoryOp( ParamsReadContext ctx, PlayerIdentity senderRPC, string permission, out JMPlayerInstance instance, out JMPlayerInstance targetInstance, out string guid )
	{
		int netLow, netHigh;

		if ( !ctx.Read( guid ) )     return NULL;
		if ( !ctx.Read( netLow ) )   return NULL;
		if ( !ctx.Read( netHigh ) )  return NULL;

		if ( !JMPermissions.HasRPC( permission, senderRPC, instance ) )
			return NULL;

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( { guid } );
		if ( players.Count() == 0 )
			return NULL;

		targetInstance = players[0];

		PlayerBase player = PlayerBase.Cast( targetInstance.PlayerObject );
		if ( !player )
			return NULL;

		return ResolveInventoryItem( player, netLow, netHigh );
	}

	void InventoryDelete( string guid, int netLow, int netHigh )
	{
		if ( IsMissionHost() )
		{
			Exec_InventoryDelete( guid, netLow, netHigh, NULL, NULL );
			return;
		}

		SendInventoryOp( JMPlayerModuleRPC.InventoryDelete, guid, netLow, netHigh );
	}

	protected void Exec_InventoryDelete( string guid, int netLow, int netHigh, PlayerIdentity ident, JMPlayerInstance instance )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( { guid } );
		if ( players.Count() == 0 )
			return;

		PlayerBase player = PlayerBase.Cast( players[0].PlayerObject );
		EntityAI entity = ResolveInventoryItem( player, netLow, netHigh );
		if ( !entity )
			return;

		FinishInventoryOp( "Deleted", entity.GetType(), guid, players[0], ident, instance, ident );
		entity.DeleteSafe();
	}

	// ---------------- Whole-container operations ----------------
	//
	// The per-item RPCs answer with a full listing each, so "delete everything
	// in this backpack" sent one request and got one listing back PER ITEM -
	// thirty round trips and thirty listings for one click. One request that
	// names the CONTAINER does the whole band server-side and answers once.

	void InventoryGroupOp( string guid, int netLow, int netHigh, int op )
	{
		if ( IsMissionHost() )
		{
			Exec_InventoryGroupOp( guid, netLow, netHigh, op, NULL, NULL );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( guid );
		rpc.Write( netLow );
		rpc.Write( netHigh );
		rpc.Write( op );
		rpc.Send( NULL, JMPlayerModuleRPC.InventoryGroupOp, true, NULL );
	}

	//! Which permission a group operation is gated behind.
	protected string GroupOpPermission( int op )
	{
		if ( op == JMInventoryGroupOp.TAKE_ALL )
			return JMConstants.PERM_PLAYER_INVENTORY_TAKE;

		if ( op == JMInventoryGroupOp.REPAIR_ALL )
			return JMConstants.PERM_PLAYER_INVENTORY_REPAIR;

		return JMConstants.PERM_PLAYER_INVENTORY_DELETE;
	}

	protected string GroupOpVerb( int op )
	{
		if ( op == JMInventoryGroupOp.TAKE_ALL )
			return "Took all from";

		if ( op == JMInventoryGroupOp.REPAIR_ALL )
			return "Repaired all in";

		return "Deleted all in";
	}

	//! Everything the container holds DIRECTLY - one level, cargo and
	//! attachments both, which is exactly what the band under it draws. Nothing
	//! deeper: a backpack inside the backpack is one entity, and taking or
	//! deleting it carries its own contents with it.
	protected void CollectDirectChildren( EntityAI container, out array< EntityAI > children )
	{
		if ( !container || !container.GetInventory() )
			return;

		array< EntityAI > all = {};
		container.GetInventory().EnumerateInventory( InventoryTraversalType.PREORDER, all );

		foreach ( EntityAI entity : all )
		{
			if ( !entity || entity == container )
				continue;

			if ( entity.GetHierarchyParent() != container )
				continue;

			children.Insert( entity );
		}
	}

	protected void Exec_InventoryGroupOp( string guid, int netLow, int netHigh, int op, PlayerIdentity ident, JMPlayerInstance instance )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( { guid } );
		if ( players.Count() == 0 )
			return;

		PlayerBase player = PlayerBase.Cast( players[0].PlayerObject );
		EntityAI container = ResolveInventoryItem( player, netLow, netHigh );
		if ( !container )
			return;

		ApplyInventoryGroupOp( container, op, guid, players[0], ident, instance, ident );
	}

	protected void ApplyInventoryGroupOp( EntityAI container, int op, string guid, JMPlayerInstance targetInstance, PlayerIdentity ident, JMPlayerInstance instance, PlayerIdentity replyTo )
	{
		array< EntityAI > children = {};
		CollectDirectChildren( container, children );

		PlayerBase admin;
		if ( op == JMInventoryGroupOp.TAKE_ALL )
		{
			// Offline the acting admin is the client player; over the wire it is
			// whoever the permission check resolved the sender to.
			if ( instance )
				admin = PlayerBase.Cast( instance.PlayerObject );
			else if ( GetPermissionsManager().GetClientPlayer() )
				admin = PlayerBase.Cast( GetPermissionsManager().GetClientPlayer().PlayerObject );
		}

		foreach ( EntityAI child : children )
		{
			if ( !child )
				continue;

			if ( op == JMInventoryGroupOp.DELETE_ALL )
				child.DeleteSafe();
			else if ( op == JMInventoryGroupOp.REPAIR_ALL )
				RepairEntity( child );
			else if ( admin )
				TakeEntity( admin, child );
		}

		// One log line and one listing for the whole band, not one per item.
		FinishInventoryOp( GroupOpVerb( op ), container.GetType(), guid, targetInstance, ident, instance, replyTo );
	}

	protected void RPC_InventoryGroupOp( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		string guid;
		int netLow, netHigh, op;

		if ( !ctx.Read( guid ) )    return;
		if ( !ctx.Read( netLow ) )  return;
		if ( !ctx.Read( netHigh ) ) return;
		if ( !ctx.Read( op ) )      return;

		if ( op < 0 || op >= JMInventoryGroupOp.COUNT )
			return;

		JMPlayerInstance instance;
		if ( !JMPermissions.HasRPC( GroupOpPermission( op ), senderRPC, instance ) )
			return;

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( { guid } );
		if ( players.Count() == 0 )
			return;

		PlayerBase player = PlayerBase.Cast( players[0].PlayerObject );
		if ( !player )
			return;

		EntityAI container = ResolveInventoryItem( player, netLow, netHigh );
		if ( !container )
			return;

		ApplyInventoryGroupOp( container, op, guid, players[0], senderRPC, instance, senderRPC );
	}

	protected void RPC_InventoryDelete( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		JMPlayerInstance targetInstance;
		string guid;

		EntityAI entity = ReadInventoryOp( ctx, senderRPC, JMConstants.PERM_PLAYER_INVENTORY_DELETE, instance, targetInstance, guid );
		if ( !entity )
			return;

		FinishInventoryOp( "Deleted", entity.GetType(), guid, targetInstance, senderRPC, instance, senderRPC );
		entity.DeleteSafe();
	}

	//! Full health and bone dry, matching what the Heal action does to worn
	//! attachments so the two do not disagree about what "repaired" means.
	protected void RepairEntity( EntityAI entity )
	{
		entity.SetHealth( "", "", entity.GetMaxHealth( "", "" ) );

		ItemBase asItem;
		if ( Class.CastTo( asItem, entity ) )
			asItem.SetWet( 0 );
	}

	void InventoryTake( string guid, int netLow, int netHigh )
	{
		if ( IsMissionHost() )
		{
			Exec_InventoryTake( guid, netLow, netHigh, NULL, NULL );
			return;
		}

		SendInventoryOp( JMPlayerModuleRPC.InventoryTake, guid, netLow, netHigh );
	}

	protected void Exec_InventoryTake( string guid, int netLow, int netHigh, PlayerIdentity ident, JMPlayerInstance instance )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( { guid } );
		if ( players.Count() == 0 )
			return;

		PlayerBase player = PlayerBase.Cast( players[0].PlayerObject );
		EntityAI entity = ResolveInventoryItem( player, netLow, netHigh );
		if ( !entity )
			return;

		PlayerBase admin = PlayerBase.Cast( GetPermissionsManager().GetClientPlayer().PlayerObject );
		TakeEntity( admin, entity );
		FinishInventoryOp( "Took", entity.GetType(), guid, players[0], ident, instance, ident );
	}

	protected void RPC_InventoryTake( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		JMPlayerInstance targetInstance;
		string guid;

		EntityAI entity = ReadInventoryOp( ctx, senderRPC, JMConstants.PERM_PLAYER_INVENTORY_TAKE, instance, targetInstance, guid );
		if ( !entity )
			return;

		PlayerBase admin;
		if ( instance )
			admin = PlayerBase.Cast( instance.PlayerObject );

		TakeEntity( admin, entity );
		FinishInventoryOp( "Took", entity.GetType(), guid, targetInstance, senderRPC, instance, senderRPC );
	}

	//! Move the item into the admin inventory, or drop it at their feet when
	//! there is no room. Falling back to the ground rather than failing silently
	//! matters: a full admin inventory is the common case, and an item that
	//! simply never moves reads as a broken button.
	protected void TakeEntity( PlayerBase admin, EntityAI entity )
	{
		if ( !admin )
			return;

		if ( admin.GetHumanInventory() && admin.GetHumanInventory().TakeEntityToInventory( InventoryMode.SERVER, FindInventoryLocationType.ANY, entity ) )
			return;

		entity.PlaceOnSurface();
		entity.SetPosition( admin.GetPosition() );
	}

	// ---------------- In-place item edits ----------------
	//
	// One request shape for all five: the item is addressed by the same network
	// ID, checked against the same permission, and answered with the same fresh
	// listing. Only the op code and the value differ, so they share a handler
	// instead of being five near-identical copies of the take/repair pair.
	//
	// The value is a float on the wire even for the three ops that are really
	// integers - a quantity, a food stage, a liquid type. Rounding once on the
	// server is cheaper than a second numeric field nothing else would use.

	void InventoryModify( string guid, int netLow, int netHigh, int op, float value )
	{
		if ( IsMissionHost() )
		{
			Exec_InventoryModify( guid, netLow, netHigh, op, value, NULL, NULL );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( guid );
		rpc.Write( netLow );
		rpc.Write( netHigh );
		rpc.Write( op );
		rpc.Write( value );
		rpc.Send( NULL, JMPlayerModuleRPC.InventoryModify, true, NULL );
	}

	protected void Exec_InventoryModify( string guid, int netLow, int netHigh, int op, float value, PlayerIdentity ident, JMPlayerInstance instance )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( { guid } );
		if ( players.Count() == 0 )
			return;

		PlayerBase player = PlayerBase.Cast( players[0].PlayerObject );
		EntityAI entity = ResolveInventoryItem( player, netLow, netHigh );
		if ( !entity )
			return;

		string verb;
		if ( !ApplyInventoryModify( entity, op, value, verb ) )
			return;

		FinishInventoryOp( verb, entity.GetType(), guid, players[0], ident, instance, ident );
	}

	protected void RPC_InventoryModify( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		JMPlayerInstance targetInstance;
		string guid;

		// Reads the three shared arguments and answers the permission; the op
		// and its value are still in the stream behind them.
		EntityAI entity = ReadInventoryOp( ctx, senderRPC, JMConstants.PERM_PLAYER_INVENTORY_MODIFY, instance, targetInstance, guid );
		if ( !entity )
			return;

		int op;
		float value;

		if ( !ctx.Read( op ) )    return;
		if ( !ctx.Read( value ) ) return;

		string verb;
		if ( !ApplyInventoryModify( entity, op, value, verb ) )
			return;

		FinishInventoryOp( verb, entity.GetType(), guid, targetInstance, senderRPC, instance, senderRPC );
	}

	//! Apply one edit. Returns false when the op does not apply to this item -
	//! a caller then logs nothing, because nothing happened.
	//!
	//! Every op re-checks the item against the request rather than trusting the
	//! client's menu: the listing the admin right-clicked can be seconds old,
	//! and by now the canteen may be a rifle.
	protected bool ApplyInventoryModify( EntityAI entity, int op, float value, out string verb )
	{
		ItemBase asItem;
		Class.CastTo( asItem, entity );

		// The three integer ops round once, here, rather than each doing it to
		// its own argument at the call site.
		int intValue = Math.Round( value );

		switch ( op )
		{
		case JMInventoryModifyOp.UNJAM:
			return UnjamWeapon( entity, verb );

		case JMInventoryModifyOp.QUANTITY:
			return SetItemQuantity( entity, asItem, value, verb );

		case JMInventoryModifyOp.TEMPERATURE:
			if ( !asItem )
				return false;

			float temperature = Math.Clamp( value, GameConstants.STATE_COLD_LVL_FOUR, GameConstants.STATE_HOT_LVL_FOUR );
			asItem.SetTemperatureEx( new TemperatureData( temperature ) );
			verb = "Set temperature " + temperature.ToString();
			return true;

		case JMInventoryModifyOp.FOOD_STAGE:
			return SetItemFoodStage( asItem, intValue, verb );

		case JMInventoryModifyOp.LIQUID_TYPE:
			return SetItemLiquidType( asItem, intValue, verb );

		case JMInventoryModifyOp.HEALTH:
			return SetItemHealth( entity, value, verb );
		}

		return false;
	}

	//! Clear a jam without the player having to hold the weapon.
	//!
	//! SetJammed alone is not enough: the jam is also a state the weapon's FSM
	//! is sitting in, and the next stable state re-reads it back out of there.
	//! RandomizeFSMState reselects a stable state from what the weapon is now -
	//! same magazine, same chambers, no jam - which is the only script-side way
	//! to leave that state for a weapon nobody is holding.
	protected bool UnjamWeapon( EntityAI entity, out string verb )
	{
		Weapon_Base weapon;
		if ( !Class.CastTo( weapon, entity ) )
			return false;

		if ( !weapon.IsJammed() )
			return false;

		weapon.SetJammed( false );
		weapon.RandomizeFSMState();
		weapon.Synchronize();

		verb = "Unjammed";
		return true;
	}

	//! Log, webhook, and push a fresh listing back to whoever asked.
	//!
	//! The refresh is unconditional and happens on every operation including the
	//! ones that half-worked, because it is the only way the admin finds out
	//! whether the item actually moved.
	protected void FinishInventoryOp( string verb, string type, string guid, JMPlayerInstance targetInstance, PlayerIdentity ident, JMPlayerInstance instance, PlayerIdentity replyTo )
	{
		GetCommunityOnlineToolsBase().Log( ident, "Inventory " + verb + " [guid=" + guid + "] [item=" + type + "]" );

		if ( targetInstance )
			SendWebhookColored( "Inventory", instance, verb + " " + type + " - " + targetInstance.FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_WARNING );

		SendInventoryTo( replyTo, guid );
	}

	// ---------------- Bleed-from-body-part ----------------
	// Body-part selections are looked up by name (string) end-to-end. Server
	// resolves the selectionName -> bit via the COT mod of the bleeding manager
	// (COT_GetZoneSelectionName / COT_GetZoneBit), guaranteeing the part
	// the admin picks matches the part that actually starts bleeding.

	void AddBleedingPart( string selectionName, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_AddBleedingPart( selectionName, guids, NULL );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( selectionName );
		rpc.Write( guids );
		rpc.Send( NULL, JMPlayerModuleRPC.AddBleedingPart, true, NULL );
	}

	protected void RPC_AddBleedingPart( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string selectionName;
		array< string > guids;

		if ( !ctx.Read( selectionName ) ) return;
		if ( !ctx.Read( guids ) )          return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_BLEED_ADD, senderRPC, instance ) )
			return;

		Exec_AddBleedingPart( selectionName, guids, senderRPC );
	}

	protected void Exec_AddBleedingPart( string selectionName, array< string > guids, PlayerIdentity ident )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );
		array< JMPlayerInstance > affectedPlayers = new array< JMPlayerInstance >;
		int affected = 0;

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( !player )
				continue;

			bool ok = player.COTAddBleedingByName( selectionName );
			if ( !ok )
				continue;

			affectedPlayers.Insert( players[i] );
			affected++;

			JMPlayerInstance instance;
			JMPermissions.Has( JMConstants.PERM_PLAYER_BLEED_ADD, ident, instance );

			GetCommunityOnlineToolsBase().Log( ident, "Added bleeding on " + selectionName + " [guid=" + players[i].GetGUID() + "]" );
			SendWebhookColored( "Bleed", instance, "Added bleeding on " + selectionName + " for " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_WARNING );

			players[i].Update();
		}

		if ( affected > 0 )
		{
			foreach ( JMPlayerInstance pi : affectedPlayers )
			{
				GetCommunityOnlineTools().SetClient( pi );
			}
		}
	}

	void StopBleedingPart( array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_StopBleedingPart( guids, NULL );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( guids );
		rpc.Send( NULL, JMPlayerModuleRPC.StopBleedingPart, true, NULL );
	}

	protected void RPC_StopBleedingPart( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !senderRPC )
			return;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_BLEED_STOP, senderRPC, instance ) )
			return;

		Exec_StopBleedingPart( guids, senderRPC );
	}

	protected void Exec_StopBleedingPart( array< string > guids, PlayerIdentity ident )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );
		array< JMPlayerInstance > affectedPlayers = new array< JMPlayerInstance >;
		int affected = 0;

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( !player )
				continue;

			BleedingSourcesManagerServer bms = player.GetBleedingManagerServer();
			if ( !bms )
				continue;

			// Vanilla has no public "remove just part X" - RemoveAllSources clears all
			bms.RemoveAllSources();

			affectedPlayers.Insert( players[i] );
			affected++;

			JMPlayerInstance instance;
			JMPermissions.Has( JMConstants.PERM_PLAYER_BLEED_STOP, ident, instance );

			GetCommunityOnlineToolsBase().Log( ident, "Stopped bleeding [guid=" + players[i].GetGUID() + "]" );
			SendWebhookColored( "Bleed", instance, "Stopped bleeding for " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_SUCCESS );

			players[i].Update();
		}

		if ( affected > 0 )
		{
			foreach ( JMPlayerInstance pi : affectedPlayers )
			{
				GetCommunityOnlineTools().SetClient( pi );
			}
		}
	}

	// ---------------- Per-body-part bleeding state (server -> client) ----------------
	// Client requests the current bleeding-state for the selected player; server
	// pushes back the list of registered zones + which bits are active so the
	// dropdown rows can render a red icon for bleeding parts.

	void RequestBleedingState( string guid )
	{
		if ( IsMissionHost() )
		{
			SendBleedingStateTo( NULL, guid );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( guid );
		rpc.Send( NULL, JMPlayerModuleRPC.SendBleedingState, true, NULL );
	}

	protected void RPC_SendBleedingState( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string reqGuid;
			if ( !ctx.Read( reqGuid ) )
				return;

			JMPlayerInstance instance;
			if ( !senderRPC )
				return;
			if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_BLEED_ADD, senderRPC, instance ) )
				return;

			SendBleedingStateTo( senderRPC, reqGuid );
			return;
		}

		// Client receives the populated state.
		//
		// Read order MUST match SendBleedingStateTo's write order exactly:
		//   guid, names.Count, names[], bits[], bleedingBits,
		//   activePartIndices.Count, activePartIndices[]
		//
		// A single misaligned read corrupts every subsequent field and the
		// engine's string deserializer throws "String CORRUPTED". Every read
		// is checked and we `return` (not `break`) on the first failure so a
		// desync surfaces at the exact field rather than cascading into a
		// garbage string-length read.
		string guidIn;
		int count;
		int activePartCount;

		if ( !ctx.Read( guidIn ) )          return;
		if ( !ctx.Read( count ) )           return;

		// Sanity-bound the count so a corrupted/garbage length can't drive a
		// multi-million-iteration read loop before the deserializer trips.
		if ( count < 0 || count > 512 )     return;

		array< string > names = new array< string >;
		for ( int na = 0; na < count; na++ )
		{
			string s;
			if ( !ctx.Read( s ) ) return;
			names.Insert( s );
		}

		array< int > bits = new array< int >;
		for ( int nb = 0; nb < count; nb++ )
		{
			int b;
			if ( !ctx.Read( b ) ) return;
			bits.Insert( b );
		}

		int bleedingBits = 0;
		if ( !ctx.Read( bleedingBits ) )    return;
		if ( !ctx.Read( activePartCount ) ) return;
		if ( activePartCount < 0 || activePartCount > 512 ) return;

		array< int > activeParts = new array< int >;
		for ( int np = 0; np < activePartCount; np++ )
		{
			int p;
			if ( !ctx.Read( p ) ) return;
			activeParts.Insert( p );
		}

		JMPlayerForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.OnBleedingStateUpdated( guidIn, names, bits, bleedingBits, activeParts );
	}

	protected void SendBleedingStateTo( PlayerIdentity to, string guid )
	{
		array< string > names = new array< string >;
		array< int > bits = new array< int >;
		int bleedingBits = 0;
		array< int > activePartIndices = new array< int >;

		if ( guid != "" )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( { guid } );
			if ( players.Count() > 0 )
			{
				PlayerBase player = PlayerBase.Cast( players[0].PlayerObject );
				if ( player )
				{
					BleedingSourcesManagerServer bms = player.GetBleedingManagerServer();
					bleedingBits = player.GetBleedingBits();

					if ( bms )
					{
						int count = bms.COT_GetZoneCount();
						for ( int i = 0; i < count; i++ )
						{
							string nm = bms.COT_GetZoneSelectionName( i );
							int bit = bms.COT_GetZoneBit( i );
							names.Insert( nm );
							bits.Insert( bit );
							if ( ( bit & bleedingBits ) != 0 )
								activePartIndices.Insert( i );
						}
					}
				}
			}
		}

		if ( names.Count() == 0 )
		{
			names.Insert( "Head" );
			names.Insert( "Torso" );
			names.Insert( "LeftArm" );
			names.Insert( "RightArm" );
			names.Insert( "LeftLeg" );
			names.Insert( "RightLeg" );
			names.Insert( "LeftFoot" );
			names.Insert( "RightFoot" );
			for ( int db = 0; db < 8; db++ )
			{
				bits.Insert( 1 << db );
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( guid );
		rpc.Write( names.Count() );
		for ( int n = 0; n < names.Count(); n++ )
			rpc.Write( names[n] );
		for ( int b = 0; b < bits.Count(); b++ )
			rpc.Write( bits[b] );
		rpc.Write( bleedingBits );
		rpc.Write( activePartIndices.Count() );
		for ( int p = 0; p < activePartIndices.Count(); p++ )
			rpc.Write( activePartIndices[p] );
		rpc.Send( NULL, JMPlayerModuleRPC.SendBleedingState, true, to );
	}
}