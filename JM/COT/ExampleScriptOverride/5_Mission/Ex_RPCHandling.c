#ifdef JM_CommunityOnlineTools
// Example: Client <-> Server RPC handling within a custom COT module.
//
// This is its OWN standalone module, not a `modded class` addition to
// JMCustomExampleModule (see JMCustomExampleModule.c) - Enforce Script cannot
// resolve a `modded class` extending a base class defined in the same script
// folder/compile pass, so every module gets its own class in its own file.
enum JMCustomRPC
{
	INVALID = 20100,
	ExecuteSubModAction,
	COUNT
}

class JMCustomExampleRPCModule: JMRenderableModuleBase
{
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
		super.OnRPC( sender, target, rpc_type, ctx );

		switch ( rpc_type )
		{
			case JMCustomRPC.ExecuteSubModAction:
				RPC_ExecuteSubModAction( ctx, sender, target );
				break;
		}
	}

	// Client sends RPC request to server
	void SendSubModActionToServer( string targetGuid, int value )
	{
		if ( !g_Game.IsClient() )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( targetGuid );
		rpc.Write( value );
		rpc.Send( NULL, JMCustomRPC.ExecuteSubModAction, true, NULL );
	}

	// Server processes incoming RPC
	private void RPC_ExecuteSubModAction( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !g_Game.IsServer() )
			return;

		string targetGuid;
		int value;

		if ( !ctx.Read( targetGuid ) || !ctx.Read( value ) )
			return;

		// Security: Always check permissions on server before executing RPC payload
		if ( !GetPermissionsManager().HasPermission( JMConstants.PERM_RPC_EXECUTE, sender ) )
			return;

		// Execute server-side logic
		COTCreateNotification( sender, new StringLocaliser( "RPC executed by admin " + sender.GetName() ), JMConstants.Lucide( "check" ) );
	}
}
#endif
