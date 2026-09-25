#ifdef JM_CommunityOnlineTools
// Example: client -> server -> client round trip in a module with no UI.
// (It is its own class, not a `modded class JMCustomExampleModule`: Enforce cannot resolve a
// modded class whose base is defined in the same addon's compile pass, so give each module its own class.)
// Registered in Ex_ModuleRegistration.c; the button that calls it is in JMCustomExampleForm.c.
enum JMCustomRPC
{
	INVALID = 20100,
	ExecuteSubModAction,
	COUNT
}

class JMCustomExampleRPCModule: JMModuleBase
{
	protected ref JMCustomExampleSettings m_Settings;

	override void DeclarePermissions()
	{
		JMPermissions.Register( JMConstants.PERM_RPC_EXECUTE );
	}

	override int GetRPCMin()
	{
		return JMCustomRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMCustomRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		// super applies COT's per-sender rate limit
		super.OnRPC( sender, target, rpc_type, ctx );

		if ( rpc_type == JMCustomRPC.ExecuteSubModAction )
			RPC_ExecuteSubModAction( ctx, sender );
	}

	// Client -> server
	void SendSubModActionToServer( string targetGuid, int value )
	{
		if ( !g_Game.IsClient() )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( targetGuid );
		rpc.Write( value );
		rpc.Send( null, JMCustomRPC.ExecuteSubModAction, true, null );
	}

	// Server: read, authorize, act, persist, reply
	protected void RPC_ExecuteSubModAction( ParamsReadContext ctx, PlayerIdentity sender )
	{
		if ( !g_Game.IsServer() )
			return;

		string targetGuid;
		int value;

		if ( !ctx.Read( targetGuid ) || !ctx.Read( value ) )
			return;

		// The UI's BindPermission only hides the button. This is the real check:
		// HasRPC also reports a denied attempt to COT's anti-cheat.
		if ( !JMPermissions.HasRPC( JMConstants.PERM_RPC_EXECUTE, sender ) )
			return;

		if ( !m_Settings )
			m_Settings = JMCustomExampleSettings.Load();

		m_Settings.ActionsRun++;
		m_Settings.Save();

		SendWebhook( "CustomAction", GetPermissionsManager().GetPlayer( sender.GetId() ), "Executed action on " + targetGuid );

		// Server -> client
		COTCreateNotification( sender, new StringLocaliser( "Action #" + m_Settings.ActionsRun + " executed" ), JMConstants.Lucide( "check" ) );
	}

	override void GetWebhookTypes( out array<string> types )
	{
		types.Insert( "CustomAction" );
	}

	override string GetWebhookTitle()
	{
		return "Custom Sub-Mod Module";
	}
}
#endif
