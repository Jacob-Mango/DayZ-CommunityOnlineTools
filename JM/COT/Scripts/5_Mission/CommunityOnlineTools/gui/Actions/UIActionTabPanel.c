//! A content panel attached to a tab of a UIActionTabs strip (UIActionTabs.AddPanel). The strip only
//! shows and hides it - the widget stays the caller's. Ids are stable and never reused.
class UIActionTabPanel
{
	int    Id;
	int    TabId;
	Widget PanelWidget;
}
