#ifdef JM_CommunityOnlineTools
// Example: Centralizing custom permission string constants on JMConstants
modded class JMConstants
{
	static const string PERM_CUSTOM_VIEW          = "Admin.CustomModule.View";
	static const string PERM_CUSTOM_ACTION        = "Admin.CustomModule.Action";
	static const string PERM_PLAYER_INJECTED_PANEL= "Admin.Player.InjectedPanel";
	static const string PERM_PLAYER_INJECTED_TAB  = "Admin.Player.InjectedTab";
	static const string PERM_RPC_EXECUTE          = "Admin.CustomModule.RPCExecute";
}
#endif
