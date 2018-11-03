class PositionModule: EditorModule
{
    void PositionModule()
    {
        GetRPCManager().AddRPC( "COS_Teleport", "SetPosition", this, SingeplayerExecutionType.Server );
    }

    override string GetLayoutRoot()
    {
        return "COS\\Modules\\Position\\gui\\layouts\\PositionMenu.layout";
    }
    
    void SetPosition( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionManager().HasPermission( sender, "Position.SetPosition" ) )
            return;

        Param1< vector > data;
        if ( !ctx.Read( data ) ) return;

        if( type == CallType.Server )
        {    
            target.SetPosition( data.param1 );
        }
    }
}