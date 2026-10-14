//! Everything a module declares about itself (title, icon, layout, category, permissions, RPC
//! range, webhook wording), filled in Describe(); JMModuleBase / JMRenderableModuleBase answer
//! every getter from it, but a getter the module still overrides wins. Describe() runs once,
//! before the mission loads, on client and server: it must be pure data.
class JMModuleInfo
{
	//! Window title and sidebar label. A localisation key without the leading '#', or plain text.
	string Title;

	//! Lucide icon name for the sidebar, e.g. "sparkles". Empty means the module has no icon.
	string Icon;

	//! Path of the form's .layout, empty for a module without a window.
	string Layout;

	//! JMSideBarConfig.CATEGORY_*. Empty falls back to CATEGORY_OTHER.
	string Category;

	//! Node that opens the module. Empty means everybody may open it.
	string ViewPermission;

	//! Keybind (Inputs.xml action name) that toggles the window. Empty means none.
	string InputToggle;

	//! Discord embed heading for the module's webhook lines.
	string WebhookTitle;

	//! False takes the module off the sidebar while keeping it registered.
	bool HasButton = true;

	//! First and last RPC ids the module owns - the enum's INVALID and COUNT.
	int RPCMin;
	int RPCMax;

	protected ref array< string > m_Permissions = new array< string >;
	protected ref array< string > m_WebhookTypes = new array< string >;

	//! A permission the module declares, in addition to the ones its actions bring with them.
	void AddPermission( string permission )
	{
		if ( m_Permissions.Find( permission ) == -1 )
			m_Permissions.Insert( permission );
	}

	void AddWebhookType( string type )
	{
		if ( m_WebhookTypes.Find( type ) == -1 )
			m_WebhookTypes.Insert( type );
	}

	array< string > GetPermissions()
	{
		return m_Permissions;
	}

	array< string > GetWebhookTypes()
	{
		return m_WebhookTypes;
	}

	void SetRPCRange( int rpcMin, int rpcMax )
	{
		RPCMin = rpcMin;
		RPCMax = rpcMax;
	}
}
