// =============================================================================
//  UIActionDropdownList
//
//  Historically an editable-text combobox with a floating TextListboxWidget.
//  Now a thin alias over UIActionSearchBox with its suggestion list enabled,
//  which provides the same "type to filter, pick from list" behaviour with
//  the new animated popup look.
//
//  Back-compat shims:
//    SetItems(array<string>)         -> SetSuggestions + open list anchor
//    InitListWidget(Widget)          -> InitSuggestionList
//    GetList()                        -> null (no TextListboxWidget any more)
// =============================================================================
class UIActionDropdownList: UIActionSearchBox
{
	void SetItems( notnull array<string> items )
	{
		SetSuggestions( items );
	}

	void InitListWidget( Widget parent )
	{
		if ( parent )
			InitSuggestionList( parent );
	}

	TextListboxWidget GetList()
	{
		return null;
	}
}
