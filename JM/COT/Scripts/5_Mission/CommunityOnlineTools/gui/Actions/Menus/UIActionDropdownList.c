//  UIActionDropdownList - thin alias over UIActionSearchBox with its suggestion list enabled.
//  Back-compat shims: SetItems(array<string>) -> SetSuggestions + open list anchor;
//  InitListWidget(Widget) -> InitSuggestionList; GetList() -> null (no TextListboxWidget
//  any more).
class UIActionDropdownList: UIActionSearchBox
{
	TextListboxWidget GetList()
	{
		return null;
	}

	void SetItems( notnull array<string> items )
	{
		SetSuggestions( items );
	}

	void InitListWidget( Widget parent )
	{
		if ( parent )
			InitSuggestionList( parent );
	}
}
