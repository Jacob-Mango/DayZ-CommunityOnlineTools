//! One tab of a UIActionTabs strip: its stable id and the state/widgets the strip keeps for it.
//! Ids are handed out by UIActionTabs.AddTab, only ever grow and are never reused, so an id stays
//! valid for as long as the tab exists - even after other tabs are removed.
class UIActionTabEntry
{
	int    Id;
	string Caption;
	string IconPath;
	bool   Enabled = true;
	bool   Visible = true;
	string Permission;
	ButtonWidget Button;
	Widget       Fill;
	Widget       Outline;
	ImageWidget  IconWidget;
	TextWidget   LabelWidget;
}
