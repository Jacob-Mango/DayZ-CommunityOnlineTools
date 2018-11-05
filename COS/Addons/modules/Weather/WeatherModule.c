class WeatherModule: EditorModule
{
    void WeatherModule()
    {    
        GetRPCManager().AddRPC( "COS_Weather", "Weather_SetStorm", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COS_Weather", "Weather_SetFog", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COS_Weather", "Weather_SetOvercast", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COS_Weather", "Weather_SetWindFunctionParams", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COS_Weather", "Weather_SetDate", this, SingeplayerExecutionType.Client );
    }

    override string GetLayoutRoot()
    {
        return "COS/Modules/Weather/gui/layouts/WeatherMenu.layout";
    }
    
    void Weather_SetDate( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( sender, "Weather.SetDate" ) )
            return;

        Param5< int, int, int, int, int > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COS_Weather", "Weather_SetDate", new Param5< int, int, int, int, int >( data.param1, data.param2, data.param3, data.param4, data.param5 ), true );
        }

        GetGame().GetWorld().SetDate( data.param1, data.param2, data.param3, data.param4, data.param5 );
    }

    void Weather_SetWindFunctionParams( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( sender, "Weather.SetWindFunctionParams" ) )
            return;

        Param3< float, float, float > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COS_Weather", "Weather_SetWindFunctionParams", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );
        }

        GetGame().GetWeather().SetWindFunctionParams( data.param1, data.param2, data.param3 );
    }

    void Weather_SetOvercast( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( sender, "Weather.SetOvercast" ) )
            return;
        
        Param3< float, float, float > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COS_Weather", "Weather_SetOvercast", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );
        }

        GetGame().GetWeather().GetOvercast().Set( data.param1, data.param2, data.param3 );
    }

    void Weather_SetFog( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( sender, "Weather.SetFog" ) )
            return;
        
        Param3< float, float, float > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COS_Weather", "Weather_SetFog", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );
        }

        GetGame().GetWeather().GetFog().Set( data.param1, data.param2, data.param3 );
    }

    void Weather_SetStorm( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( sender, "Weather.SetStorm" ) )
            return;
        
        Param3< float, float, float > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COS_Weather", "Weather_SetStorm", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );
        }

        GetGame().GetWeather().SetStorm( data.param1, data.param2, data.param3 );
    }

}