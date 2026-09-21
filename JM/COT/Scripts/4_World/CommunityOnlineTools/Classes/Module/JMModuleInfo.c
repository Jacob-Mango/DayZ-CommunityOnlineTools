//! Everything a module declares about itself, in one place.
//!
//! A module used to override a dozen getters to say the same handful of facts (title, icon,
//! layout, category, the permission that opens it, its RPC range, its webhook wording). It now
//! fills one of these in Describe() and JMModuleBase / JMRenderableModuleBase answer every getter
//! from it:
//!
//!   override void Describe( JMModuleInfo info )
//!   {
//!       info.Title = "My Module";
//!       info.Icon = "sparkles";
//!       info.Layout = "JM\\COT\\GUI\\layouts\\my_form.layout";
//!       info.Category = JMSideBarConfig.CATEGORY_OTHER;
//!       info.ViewPermission = JMConstants.PERM_MYMODULE_VIEW;
//!       info.InputToggle = "UACOTToggleMyModule";
//!       info.WebhookTitle = "My Module";
//!       info.SetRPCRange( JMMyModuleRPC.INVALID, JMMyModuleRPC.COUNT );
//!       info.AddPermission( JMConstants.PERM_MYMODULE_ACTION );
//!       info.AddWebhookType( "Something" );
//!   }
//!
//! A getter the module still overrides wins, so a module can migrate one getter at a time and a
//! mod's `modded class` override keeps working. Describe() runs once and must be pure data: it is
//! called before the mission loads, on both the client and the server.
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
