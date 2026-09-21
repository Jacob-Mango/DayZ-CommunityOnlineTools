//! One place for the "did that work?" feedback an admin action gives.
//!
//! Anything that puts text on the clipboard should go through Copy() so the
//! admin always sees that it happened; a button that can show its own state
//! (UIActionFeedbackButton) should keep doing that instead and call
//! g_Game.CopyToClipboard directly, otherwise the admin gets told twice.
class COTFeedback
{
	//! Copy `text` to the clipboard and show a local "Copied to clipboard" toast.
	static void Copy( string text )
	{
		g_Game.CopyToClipboard( text );

		Notify( "STR_COT_COPIED_CLIPBOARD" );
	}

	//! Local admin toast. `message` is a stringtable key (with or without the
	//! leading #) or plain text; `param` fills %1 in the translated string.
	static void Notify( string message, string param = "" )
	{
		if ( param != "" )
			COTCreateLocalAdminNotification( new StringLocaliser( message, param ) );
		else
			COTCreateLocalAdminNotification( new StringLocaliser( message ) );
	}
}
