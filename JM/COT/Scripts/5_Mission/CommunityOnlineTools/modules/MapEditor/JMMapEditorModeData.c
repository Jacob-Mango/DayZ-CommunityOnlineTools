// =============================================================================
//  JMMapEditorModeData.c
//
//  UIActionData carrier for the four Map Editor mode buttons. Stores the mode
//  int so a single OnClick handler can dispatch all four.
// =============================================================================

class JMMapEditorModeData : UIActionData
{
	int Mode;

	void JMMapEditorModeData( int mode )
	{
		Mode = mode;
	}
}