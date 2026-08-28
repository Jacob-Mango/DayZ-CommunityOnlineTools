// =============================================================================
//  JMMapEditorRowData.c
//
//  UIActionData carrier for a list row's id (the placed-object id) so the
//  click handler can recover which row was clicked without a global state
//  lookup.
// =============================================================================

class JMMapEditorRowData : UIActionData
{
	int Id;

	void JMMapEditorRowData( int id )
	{
		Id = id;
	}
}