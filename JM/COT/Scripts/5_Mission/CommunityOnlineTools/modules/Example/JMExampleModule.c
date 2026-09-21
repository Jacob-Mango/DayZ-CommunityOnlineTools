class JMExampleModule: JMRenderableModuleBase
{
	override void DescribeModule( JMModuleInfo info )
	{
		super.DescribeModule( info );

		info.Title = "Example Module";
		info.WebhookTitle = "Example Module";
		info.Icon = "info";
		info.Layout = "JM/COT/GUI/layouts/Example_form.layout";
		info.ViewPermission = JMConstants.PERM_EXAMPLE_VIEW;
		// If you want to allow a keybind to open this menu this is how
		info.InputToggle = "UACOTToggleExample";

		//! Called on both client and server as the module registers, before the mission loads.
		info.AddPermission( JMConstants.PERM_EXAMPLE_BUTTON );
		info.AddPermission( JMConstants.PERM_EXAMPLE_DROPDOWN );
	}

	/*
	*/

	
}
