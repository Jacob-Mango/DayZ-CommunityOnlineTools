// =============================================================================
//  JMActionHistoryModule
//
//  The trigger for JMActionHistory's shared undo/redo stack: a parameterless
//  client -> server request (Ctrl+Z / Ctrl+Y in MissionGameplay), and the
//  place each step is reported - a notification to the admin, a log line and a
//  webhook, the same trail the original action left.
//
//  No UI of its own. It is a module, rather than living on the ESP module that
//  used to host the RPC, so sub-mods that push their own entries do not have to
//  depend on ESP being there.
//
//  RPCs (JMActionHistoryModuleRPC @ 10960):
//    Undo, Redo   client -> server, no payload
// =============================================================================
class JMActionHistoryModule: JMModuleBase
{
	override void DescribeModule( JMModuleInfo info )
	{
		super.DescribeModule( info );

		info.WebhookTitle = "Action History";
		info.SetRPCRange( JMActionHistoryModuleRPC.INVALID, JMActionHistoryModuleRPC.COUNT );

		info.AddPermission( JMConstants.PERM_ACTIONHISTORY_UNDOOTHERS );

		info.AddWebhookType( "Undo" );
		info.AddWebhookType( "Redo" );
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMActionHistoryModuleRPC.Undo:
			RPC_Step( true, sender );
			break;
		case JMActionHistoryModuleRPC.Redo:
			RPC_Step( false, sender );
			break;
		}
	}

	//! Undo the most recent action - whatever it is (delete, heal, teleport,
	//! ...), not just an ESP-flavoured one.
	void UndoLastAction()
	{
		Request( true );
	}

	void RedoLastAction()
	{
		Request( false );
	}

	protected void Request( bool undo )
	{
		if ( IsMissionOffline() )
		{
			Exec_Step( undo, NULL );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();

		if ( undo )
			rpc.Send( NULL, JMActionHistoryModuleRPC.Undo, true, NULL );
		else
			rpc.Send( NULL, JMActionHistoryModuleRPC.Redo, true, NULL );
	}

	//! No standalone permission check here - JMActionHistory checks each
	//! stepped entry against the permission ITS OWN action required, which is
	//! the correct gate: undoing is not a new grant of capability.
	protected void RPC_Step( bool undo, PlayerIdentity sender )
	{
		if ( !IsMissionHost() || !sender )
			return;

		Exec_Step( undo, sender );
	}

	protected void Exec_Step( bool undo, PlayerIdentity ident )
	{
		JMActionHistoryEntry entry;
		int skipped;
		JMActionHistoryResult result;

		//! "Undo"/"Redo" is the webhook type and log verb; the lower-case form
		//! reads inside a sentence.
		string type = "Redo";
		string verb = "redo";
		string icon = JMConstants.Lucide( "redo-2" );
		if ( undo )
		{
			type = "Undo";
			verb = "undo";
			icon = JMConstants.Lucide( "undo-2" );
			result = JMActionHistory.Undo( ident, entry, skipped );
		} else
		{
			result = JMActionHistory.Redo( ident, entry, skipped );
		}

		string what;
		if ( entry )
			what = entry.GetDescription();

		string suffix;
		if ( skipped > 0 )
			suffix = " (skipped " + skipped + " whose target is gone)";

		string message;

		switch ( result )
		{
		case JMActionHistoryResult.OK:
			message = type + " done: " + what + suffix;
			LogStep( type, ident, what );
			break;
		case JMActionHistoryResult.EMPTY:
			message = "Nothing to " + verb + suffix;
			break;
		case JMActionHistoryResult.DENIED:
			message = "No permission to " + verb + ": " + what;
			break;
		case JMActionHistoryResult.FAILED:
			message = "Could not " + verb + ": " + what + " (target is gone)" + suffix;
			break;
		}

		Notify( ident, message, icon );
	}

	//! The same trail the original action left: a log line, and a webhook
	//! attributed to the admin who pressed the key.
	protected void LogStep( string type, PlayerIdentity ident, string what )
	{
		GetCommunityOnlineToolsBase().Log( ident, "ActionHistory action=" + type + " entry=" + what );

		if ( !ident )
			return;

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( ident.GetId() );
		SendWebhookColored( type, instance, type + " " + what, JMConstants.WEBHOOK_COLOR_INFO );
	}

	protected void Notify( PlayerIdentity ident, string message, string icon )
	{
		if ( ident )
			COTCreateNotification( ident, new StringLocaliser( message ), icon );
		else
			COTCreateLocalAdminNotification( new StringLocaliser( message ), icon );
	}
}
