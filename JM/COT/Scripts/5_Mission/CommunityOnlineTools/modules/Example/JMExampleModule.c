class JMExampleModule: JMRenderableModuleBase
{
	void JMExampleModule()
	{
		GetPermissionsManager().RegisterPermission( "Admin.Example.View" );
		GetPermissionsManager().RegisterPermission( "Admin.Example.Button" );
		GetPermissionsManager().RegisterPermission( "Admin.Example.Dropdown" );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Admin.Example.View" );
	}

	/*
	// If you want to allow a keybind to open this menu this is how
	override string GetInputToggle()
	{
		return "UACOTToggleExample";
	}
	*/

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/Example_form.layout";
	}

	override string GetTitle()
	{
		return "Example Module";
	}
	
	override string GetIconName()
	{
		return "E";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override string GetWebhookTitle()
	{
		return "Example Module";
	}
};
