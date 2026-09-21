#ifdef JM_CommunityOnlineTools
// Example: keep custom permission keys as constants on JMConstants so the
// registration, the UI binding and the server check all spell them the same.
// (PERM_PLAYER_INJECTED_PANEL already exists in COT itself - only declare your own.)
modded class JMConstants
{
	static const string PERM_CUSTOM_VIEW           = "Admin.CustomModule.View";
	static const string PERM_CUSTOM_ACTION         = "Admin.CustomModule.Action";
	static const string PERM_RPC_EXECUTE           = "Admin.CustomModule.RPCExecute";
	static const string PERM_PLAYER_INJECTED_TAB   = "Admin.Player.InjectedTab";
}
#endif
