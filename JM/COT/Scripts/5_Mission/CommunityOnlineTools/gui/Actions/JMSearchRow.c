//! The controls of a list's toolbar row, as returned by UIActionManager.CreateSearchRow().
//! Refresh and Filter are null when the row was built without them.
class JMSearchRow
{
	UIActionSearchBox Search;
	UIActionImageButton Filter;
	UIActionImageButton Refresh;

	//! The row itself, so the caller can Add() more controls and SetGap(). Only set by
	//! CreateSearchFlexRow(); null for the wrap-spacer row CreateSearchRow() builds.
	ref UIActionFlexRow Row;
}
