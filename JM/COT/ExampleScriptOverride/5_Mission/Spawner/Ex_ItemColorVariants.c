#ifdef JM_CommunityOnlineTools
// Example: adding custom color variant mappings for items in the object spawner module (JMObjectSpawnerModule).
modded class JMObjectSpawnerModule
{
	override void RegisterColorVariants()
	{
		super.RegisterColorVariants();

		TStringArray variants = new TStringArray;
		variants.Insert( "OffroadHatchback" );
		variants.Insert( "OffroadHatchback_Blue" );
		variants.Insert( "OffroadHatchback_White" );

		m_ColorVariants.Insert( "OffroadHatchback", variants );
	}
}
#endif
