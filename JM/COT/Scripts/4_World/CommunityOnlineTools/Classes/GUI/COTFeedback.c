//! Admin action feedback. Clipboard writes go through Copy() so the admin sees it
//! happened; buttons that show their own state (UIActionFeedbackButton) call
//! g_Game.CopyToClipboard directly, else the admin is told twice.
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
