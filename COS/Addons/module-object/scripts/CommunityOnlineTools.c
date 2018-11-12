modded class CommunityOnlineTools
{
    override void RegisterModules()
    {
        super.RegisterModules();
        
        GetModuleManager().RegisterModule( new ObjectModule );
    }
}