//! One on/off state an admin can put on a player (god mode, freeze...). A subclass owns what differs
//! (RPC, permission, wording, apply/read-back); JMPlayerModule SetToggle/Exec_Toggle/RPC_Toggle and
//! JMPlayerFormTabActions are shared. A mod adds one by overriding JMPlayerModule.RegisterToggles()
//! and calling DefineToggle( toggles, new MyToggle(), "MyToggle", MY_RPC, MY_PERM, "my toggle", "MYTOGGLE" ).
class JMPlayerToggle
{
	//! Stable ids. Never reused: mods look toggles up by them.
	static const string BLOODYHANDS = "BloodyHands";
	static const string GODMODE = "GodMode";
	static const string FREEZE = "Freeze";
	static const string RAGDOLL = "Ragdoll";
	static const string RECEIVEDAMAGEDEALT = "ReceiveDamageDealt";
	static const string CANNOTBETARGETEDBYAI = "CannotBeTargetedByAI";
	static const string INVISIBILITY = "Invisibility";
	static const string REMOVECOLLISION = "RemoveCollision";
	static const string UNLIMITEDAMMO = "UnlimitedAmmo";
	static const string ADMINNVG = "AdminNVG";
	static const string UNLIMITEDSTAMINA = "UnlimitedStamina";
	static const string BROKENLEGS = "BrokenLegs";
	protected string m_Id;
	protected int m_RPC;
	protected string m_Permission;
	protected string m_Name;
	protected string m_Key;
	protected string m_Icon;
	protected string m_InputHint;
	protected bool m_SendToClients;

	string GetCheckboxKey()
	{
		return "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_" + m_Key;
	}

	string GetIcon()
	{
		return m_Icon;
	}

	string GetId()
	{
		return m_Id;
	}

	string GetInputHint()
	{
		return m_InputHint;
	}

	//! Name of the toggle in a notification ("God mode enabled for ...").
	string GetLabelKey()
	{
		return "#STR_COT_PLAYER_MODULE_TOGGLE_" + m_Key;
	}

	string GetName()
	{
		return m_Name;
	}

	string GetPermission()
	{
		return m_Permission;
	}

	int GetRPC()
	{
		return m_RPC;
	}

	string GetTooltipKey()
	{
		return "#STR_COT_PLAYER_MODULE_TT_" + m_Key;
	}

	//! False for a toggle that is script-only and has no checkbox on the Actions tab.
	bool IsInUI()
	{
		return true;
	}

	//! id: stable id above; rpc: JMPlayerModuleRPC carrying a change; permission: node the SENDER needs
	//! (checkbox and server); name: lower-case wording for log/webhook lines ("god mode");
	//! key: suffix of the STR_COT_PLAYER_MODULE_* localisation keys (see GetLabelKey)
	JMPlayerToggle Define( string id, int rpc, string permission, string name, string key )
	{
		m_Id = id;
		m_RPC = rpc;
		m_Permission = permission;
		m_Name = name;
		m_Key = key;

		return this;
	}

	//! Lucide icon for the checkbox.
	JMPlayerToggle WithIcon( string icon )
	{
		m_Icon = icon;

		return this;
	}

	//! Localisation key of the message shown when the toggle is pressed from its
	//! keybind while COT is closed. "" means it has no keybind of its own.
	JMPlayerToggle WithInputHint( string hint )
	{
		m_InputHint = hint;

		return this;
	}

	//! Have the server push the changed player to the clients after applying it, for a
	//! toggle whose effect the client itself needs to see (night vision, ammo, stamina).
	JMPlayerToggle WithClientSync()
	{
		m_SendToClients = true;

		return this;
	}

	//! Server: put the state on the player.
	void Apply( PlayerBase player, int value )
	{
	}

	//! Client: what the synced copy of a player says the state is.
	int Read( JMPlayerInstance instance )
	{
		return 0;
	}

	//! Client: the state as the local entity has it right now. A keybind flips this
	//! rather than the synced copy, which trails the entity by a refresh. Defaults to
	//! the synced copy for a toggle whose entity has no getter.
	int ReadEntity( PlayerBase player, JMPlayerInstance instance )
	{
		return Read( instance );
	}

	//! Server: runs after the toggle was applied and logged for one player.
	void OnApplied( JMPlayerInstance instance )
	{
		if ( m_SendToClients )
			GetCommunityOnlineTools().SetClient( instance );
	}

	//! The value travels as a bool unless a toggle has more than two states.
	void WriteValue( ParamsWriteContext ctx, int value )
	{
		ctx.Write( value != 0 );
	}

	bool ReadValue( ParamsReadContext ctx, out int value )
	{
		bool state;
		if ( !ctx.Read( state ) )
			return false;

		value = state;

		return true;
	}
}
