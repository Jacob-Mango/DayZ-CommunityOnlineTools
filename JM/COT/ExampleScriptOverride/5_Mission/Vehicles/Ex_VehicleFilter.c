#ifdef JM_CommunityOnlineTools
// Example: registering custom vehicle filter types (JMVehiclesModule).
modded class JMVehiclesModule
{
	override void RegisterVehicleFilters()
	{
		super.RegisterVehicleFilters();

		//! Add custom sub-mod category filter
		JMFilterRegistry.Register( "Vehicles", "SubModBoats", "Sub-Mod Boats", JMConstants.Lucide( "ship" ) );
	}
}
#endif
