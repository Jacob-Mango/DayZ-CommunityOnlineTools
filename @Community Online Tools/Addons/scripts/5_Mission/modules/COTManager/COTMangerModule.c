class COTManagerModule: EditorModule
{
    protected ref COTManagerSettings settings;
    
    void COTManagerModule()
    {
        GetRPCManager().AddRPC( "COT_Manager", "LoadData", this, SingeplayerExecutionType.Client );
    
        GetPermissionsManager().RegisterPermission( "COT.Manager.View" );
    }

    override bool HasAccess()
    {
        return GetPermissionsManager().HasPermission( "COT.Manager.View" );
    }
    
    override void ReloadSettings()
    {
        super.ReloadSettings();

        settings = COTManagerSettings.Load();
    }

    override void OnMissionLoaded()
    {
        super.OnMissionLoaded();

        if ( GetGame().IsClient() )
            GetRPCManager().SendRPC( "COT_Manager", "LoadData", new Param, true );
    }

    override void OnMissionFinish()
    {
        super.OnMissionFinish();

        if ( GetGame().IsServer() )
            settings.Save();
    }

    override void RegisterKeyMouseBindings() 
    {
    }

    override string GetLayoutRoot()
    {
        return "JM/COT/gui/layouts/COTManager/COTManagerMenu.layout";
    }

    void LoadData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if( type == CallType.Server )
        {
            if ( !GetPermissionsManager().HasPermission( "COT.Manager.View", sender ) )
                return;

            GetRPCManager().SendRPC( "COT_Manager", "LoadData", new Param1< ref COTManagerSettings >( settings ), true );
        }

        if( type == CallType.Client )
        {
            Param1< ref COTManagerSettings > data;
            if ( !ctx.Read( data ) ) return;

            settings = data.param1;
        }
    }
}