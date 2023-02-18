/*#ifdef VPPADMINTOOLS
class JMVPPModuleBase : JMRenderableModuleBase
{
	override JMFormBase InitForm( Widget root )
	{
		JMFormBase form = new JMVPPFormBase();
		form.OnWidgetScriptInit( root );
		return form;
	}

	override string GetLayoutRoot()
	{
		return "";
	}

	override bool ImageIsIcon()
	{
		return true;
	}
	
	override string GetImageSet()
	{
		return "dayz_gui_vpp";
	}
	
	override string GetTitle()
	{
		return "[VPP] " + GetModifiedTitle();
	}

	string GetModifiedTitle()
	{
		return "";
	}

	string GetMenuClassName()
	{
		string name = ClassName();
		name.Replace( "JMVPP", "" );
		return name;
	}
};
#endif*/