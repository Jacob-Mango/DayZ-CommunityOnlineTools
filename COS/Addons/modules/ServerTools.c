modded class ServerTools
{
    override void RegisterModules()
    {
        super.RegisterModules();
        
        GetModuleManager().RegisterModule( new ObjectModule );
        GetModuleManager().RegisterModule( new TeleportModule );
        GetModuleManager().RegisterModule( new GameModule );
        GetModuleManager().RegisterModule( new CameraTool );
    }
}