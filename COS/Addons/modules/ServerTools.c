modded class ServerTools
{
    override void RegisterModules()
    {
        super.RegisterModules();
        
        GetModuleManager().RegisterModule( new ObjectModule );
        GetModuleManager().RegisterModule( new TeleportModule );
    }
}