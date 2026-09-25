#ifdef JM_CommunityOnlineTools
// Example: making COT load your modules. A module class does nothing until it is
// listed here - see JMCustomExampleModule.c (sidebar module + form) and
// Ex_RPCHandling.c (server-side RPC module with no UI).
modded class JMModuleConstructor
{
	override void RegisterModules( out TTypenameArray modules )
	{
		super.RegisterModules( modules );

		modules.Insert( JMCustomExampleModule );
		modules.Insert( JMCustomExampleRPCModule );
		modules.Insert( JMExampleModule );
		modules.Insert( JMItemStatsModule );
	}
}
#endif
