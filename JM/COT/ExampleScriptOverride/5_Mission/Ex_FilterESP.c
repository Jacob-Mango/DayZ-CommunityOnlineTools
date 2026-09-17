#ifdef JM_CommunityOnlineTools
// Example: Adding custom ESP view type filter category (JMESPModule)
class JMESPViewTypeSubModCustom: JMESPViewType
{
	void JMESPViewTypeSubModCustom()
	{
		Localisation = "Sub-Mod Structures";
		Colour = 0xFF3399FF;
		Permission = "SubModStructures";
	}

	override bool IsValid( Object obj, out JMESPMeta meta )
	{
		// Custom target condition for ESP rendering
		return false;
	}
}

modded class JMESPModule
{
	override void RegisterTypes( out TTypenameArray types )
	{
		super.RegisterTypes( types );

		// Register custom ESP view type category
		types.Insert( JMESPViewTypeSubModCustom );
	}
}
#endif
