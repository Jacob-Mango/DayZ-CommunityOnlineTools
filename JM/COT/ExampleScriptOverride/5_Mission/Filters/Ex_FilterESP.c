#ifdef JM_CommunityOnlineTools
// Example: a new ESP category. Give it a name, a colour, a permission suffix and a test; ESP
// registers the "ESP.View.<Permission>" permission and adds the category to the filters tab.
class JMESPViewTypeExampleBarrels: JMESPViewTypeItemBase
{
	void JMESPViewTypeExampleBarrels()
	{
		Permission = "Item.ExampleBarrels";
		Localisation = "Example: Barrels";
		Colour = ARGB( 255, 51, 153, 255 );
	}

	// Which world objects belong to this category.
	override bool CheckLootCategory( Object obj )
	{
		return obj.IsKindOf( "Barrel_ColorBase" );
	}
}

modded class JMESPModule
{
	override void RegisterTypes( out TTypenameArray types )
	{
		super.RegisterTypes( types );

		types.Insert( JMESPViewTypeExampleBarrels );
	}
}
#endif
