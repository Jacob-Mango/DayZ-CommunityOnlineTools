class JMMissionSelectorModule: JMRenderableModuleBase
{
	void JMMissionSelectorModule()
	{

	}

	override void EnableUpdate()
	{
	}

	override bool HasAccess()
	{
		return IsMissionOffline();
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/mission_selector_form.layout";
	}

	override string GetTitle()
	{
		return "Offline Mission";
	}
	
	override string GetIconName()
	{
		return "OM";
	}

	override bool ImageIsIcon()
	{
		return false;
	}
};