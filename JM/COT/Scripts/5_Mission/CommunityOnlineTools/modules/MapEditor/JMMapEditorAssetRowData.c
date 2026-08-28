// =============================================================================
//  JMMapEditorAssetRowData.c
//
//  UIActionData carrier for an asset-browser row in the Map Editor's left
//  sidebar. Holds the classname to spawn + a display name for tooltips.
// =============================================================================

class JMMapEditorAssetRowData : UIActionData
{
	string ClassName;
	string DisplayName;

	void JMMapEditorAssetRowData( string className, string displayName )
	{
		ClassName = className;
		DisplayName = displayName;
	}
}