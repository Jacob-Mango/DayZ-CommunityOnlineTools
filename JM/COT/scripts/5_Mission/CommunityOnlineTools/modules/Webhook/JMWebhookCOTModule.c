enum JMWebhookCOTModuleRPC
{
	INVALID = 10260,
	Load,
    RemoveURL,
    AddURL,
	RemoveType,
    AddType,
	COUNT
};

class JMWebhookCOTModule: JMRenderableModuleBase
{
	void JMWebhookCOTModule()
	{
		GetPermissionsManager().RegisterPermission( "Webhook.View" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage.URL" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage.URL.Add" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage.URL.Remove" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage.Type" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage.Type.Add" );
		GetPermissionsManager().RegisterPermission( "Webhook.Manage.Type.Remove" );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Webhook.View" );
	}

	override string GetInputToggle()
	{
		return "UACOTToggleWebhook";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/webhook_form.layout";
	}

	override string GetTitle()
	{
		return "#STR_COT_WEBHOOK_MODULE_NAME";
	}
	
	override string GetIconName()
	{
		return "WH";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override string GetWebhookTitle()
	{
		return "Manage Webhooks";
	}

	override void GetWebhookTypes( out array< string > types )
	{
        types.Insert( "" );
	}

	override int GetRPCMin()
	{
		return JMWebhookCOTModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMWebhookCOTModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ref ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMWebhookCOTModuleRPC.Load:
			break;
		case JMWebhookCOTModuleRPC.RemoveURL:
			break;
		case JMWebhookCOTModuleRPC.AddURL:
			break;
		case JMWebhookCOTModuleRPC.RemoveType:
			break;
		case JMWebhookCOTModuleRPC.AddType:
			break;
		}
	}
};