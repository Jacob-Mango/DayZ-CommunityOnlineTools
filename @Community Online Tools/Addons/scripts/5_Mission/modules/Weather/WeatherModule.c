class WeatherModule: EditorModule
{
    void WeatherModule()
    {    
        GetRPCManager().AddRPC( "COT_Weather", "Weather_SetStorm", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Weather", "Weather_SetFog", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Weather", "Weather_SetRain", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Weather", "Weather_SetOvercast", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Weather", "Weather_SetWindFunctionParams", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Weather", "Weather_SetDate", this, SingeplayerExecutionType.Client );

        GetPermissionsManager().RegisterPermission( "Weather.SetDate" );
        GetPermissionsManager().RegisterPermission( "Weather.SetWindFunctionParams" );
        GetPermissionsManager().RegisterPermission( "Weather.SetOvercast" );
        GetPermissionsManager().RegisterPermission( "Weather.SetFog" );
        GetPermissionsManager().RegisterPermission( "Weather.SetRain" );
        GetPermissionsManager().RegisterPermission( "Weather.SetStorm" );
    
        GetPermissionsManager().RegisterPermission( "Weather.View" );
    }

    override bool HasAccess()
    {
        return GetPermissionsManager().HasPermission( "Weather.View" );
    }

    override string GetLayoutRoot()
    {
        return "JM/COT/gui/layouts/Weather/WeatherMenu.layout";
    }
    
    void Weather_SetDate( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param5< int, int, int, int, int > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Client )
        {
            GetGame().GetWorld().SetDate( data.param1, data.param2, data.param3, data.param4, data.param5 );
            return;
        }

        if ( !GetPermissionsManager().HasPermission( "Weather.SetDate", sender ) )
            return;

        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COT_Weather", "Weather_SetDate", new Param5< int, int, int, int, int >( data.param1, data.param2, data.param3, data.param4, data.param5 ), true );

            GetGame().GetWorld().SetDate( data.param1, data.param2, data.param3, data.param4, data.param5 );

            COTLog( sender, "Set the date to " + data.param1 + "/" + data.param2 + "/" + data.param3 + " " + data.param4 + ":" + data.param5 );
        }
    }

    void Weather_SetWindFunctionParams( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param3< float, float, float > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Client )
        {
            GetGame().GetWeather().SetWindFunctionParams( data.param1, data.param2, data.param3 );
            return;
        }

        if ( !GetPermissionsManager().HasPermission( "Weather.SetWindFunctionParams", sender ) )
            return;

        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COT_Weather", "Weather_SetWindFunctionParams", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );

            GetGame().GetWeather().SetWindFunctionParams( data.param1, data.param2, data.param3 );

            COTLog( sender, "Set wind to " + data.param1 + " " + data.param2 + " " + data.param3 );
        }
    }

    void Weather_SetOvercast( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param3< float, float, float > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Client )
        {
            GetGame().GetWeather().GetOvercast().Set( data.param1, data.param2, data.param3 );
            return;
        }

        if ( !GetPermissionsManager().HasPermission( "Weather.SetOvercast", sender ) )
            return;
        
        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COT_Weather", "Weather_SetOvercast", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );

            GetGame().GetWeather().GetOvercast().Set( data.param1, data.param2, data.param3 );

            COTLog( sender, "Set overcast to " + data.param1 + " " + data.param2 + " " + data.param3 );
        }
    }

    void Weather_SetFog( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param3< float, float, float > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Client )
        {
            GetGame().GetWeather().GetFog().Set( data.param1, data.param2, data.param3 );
            return;
        }

        if ( !GetPermissionsManager().HasPermission( "Weather.SetFog", sender ) )
            return;
        
        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COT_Weather", "Weather_SetFog", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );

            GetGame().GetWeather().GetFog().Set( data.param1, data.param2, data.param3 );

            COTLog( sender, "Set fog to " + data.param1 + " " + data.param2 + " " + data.param3 );
        }
    }

    void Weather_SetRain( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param3< float, float, float > data;
        if ( !ctx.Read( data ) ) return;

        if( type == CallType.Client )
        {
            GetGame().GetWeather().GetRain().Set( data.param1, data.param2, data.param3 );
            return;
        }

        if ( !GetPermissionsManager().HasPermission( "Weather.SetRain", sender ) )
            return;
        
        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COT_Weather", "Weather_SetRain", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );

            GetGame().GetWeather().GetRain().Set( data.param1, data.param2, data.param3 );

            COTLog( sender, "Set rain to " + data.param1 + " " + data.param2 + " " + data.param3 );
        }
    }

    void Weather_SetStorm( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param3< float, float, float > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Client )
        {
            GetGame().GetWeather().SetStorm( data.param1, data.param2, data.param3 );
            return;
        }

        if ( !GetPermissionsManager().HasPermission( "Weather.SetStorm", sender ) )
            return;
        
        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COT_Weather", "Weather_SetStorm", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );

            GetGame().GetWeather().SetStorm( data.param1, data.param2, data.param3 );

            COTLog( sender, "Set storm to " + data.param1 + " " + data.param2 + " " + data.param3 );
        }
    }

}