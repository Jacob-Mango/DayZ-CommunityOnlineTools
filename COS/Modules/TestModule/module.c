class TestModule : Module
{
    void TestModule()
    {

    }

    void onUpdate( float timeslice )
    {
        string world = "N/A";

        GetGame().GetWorldName( world );

        //Print( "We are on map: " + world );

        if ( GetGame().IsClient() )
        {
            PlayerBase player = GetGame().GetPlayer();
            string name = player.GetIdentity().GetName();
            //Print( "Player's name is " + name );
        }
    }
}

void RegisterModules( ModuleManager moduleManager )
{
    moduleManager.RegisterModule( new TestModule );
}