#ifdef CF_WINDOWS
modded class CF_Windows
{
	override bool CheckForFocus()
	{
		if (super.CheckForFocus()) return true;

		if (JMStatics.COT_MENU && JMStatics.COT_MENU.IsVisible()) return true;

		return false;
	}

	override bool CheckWidgetForFocus(Widget widget)
	{
		if (super.CheckWidgetForFocus(widget)) return true;

		if (widget == JMStatics.ESP_CONTAINER) return true;
		if (widget == JMStatics.COT_MENU) return true;

		return false;
	}
};
#endif
