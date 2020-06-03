class JMWeatherOldModule: JMRenderableModuleBase
{
    void JMWeatherOldModule()
    {    
        GetRPCManager().AddRPC( "COT_Weather", "Weather_SetStorm", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Weather", "Weather_SetFog", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Weather", "Weather_SetRain", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Weather", "Weather_SetOvercast", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Weather", "Weather_SetWindFunctionParams", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Weather", "Weather_SetDate", this, SingeplayerExecutionType.Client );

		GetPermissionsManager().RegisterPermission( "Weather.Date" );

		GetPermissionsManager().RegisterPermission( "Weather.Wind" );
		GetPermissionsManager().RegisterPermission( "Weather.Wind.FunctionParams" );

		GetPermissionsManager().RegisterPermission( "Weather.Storm" );
		GetPermissionsManager().RegisterPermission( "Weather.Overcast" );
		GetPermissionsManager().RegisterPermission( "Weather.Fog" );

		GetPermissionsManager().RegisterPermission( "Weather.Rain" );
		GetPermissionsManager().RegisterPermission( "Weather.Rain.Thresholds" );

		GetPermissionsManager().RegisterPermission( "Weather.Preset" );
		GetPermissionsManager().RegisterPermission( "Weather.Preset.Create" );
		GetPermissionsManager().RegisterPermission( "Weather.Preset.Update" );
		GetPermissionsManager().RegisterPermission( "Weather.Preset.Remove" );

		GetPermissionsManager().RegisterPermission( "Weather.View" );
    }

	override string GetInputToggle()
	{
		return "UACOTToggleWeather";
	}

    override string GetLayoutRoot()
    {
        return "JM/COT/gui/layouts/weather_form_old.layout";
    }
	
	override string GetTitle()
	{
		return "Weather (TEMP)";
	}
	
	override string GetIconName()
	{
		return "W";
	}

	override bool ImageIsIcon()
	{
		return false;
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

        if ( !GetPermissionsManager().HasPermission( "Weather.Date", sender ) )
            return;

        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COT_Weather", "Weather_SetDate", new Param5< int, int, int, int, int >( data.param1, data.param2, data.param3, data.param4, data.param5 ), true );

            GetGame().GetWorld().SetDate( data.param1, data.param2, data.param3, data.param4, data.param5 );

            GetCommunityOnlineToolsBase().Log( sender, "Set the date to " + data.param1 + "/" + data.param2 + "/" + data.param3 + " " + data.param4 + ":" + data.param5 );
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

        if ( !GetPermissionsManager().HasPermission( "Weather.Wind.FunctionParams", sender ) )
            return;

        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COT_Weather", "Weather_SetWindFunctionParams", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );

            GetGame().GetWeather().SetWindFunctionParams( data.param1, data.param2, data.param3 );

            GetCommunityOnlineToolsBase().Log( sender, "Set wind to " + data.param1 + " " + data.param2 + " " + data.param3 );
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

        if ( !GetPermissionsManager().HasPermission( "Weather.Overcast", sender ) )
            return;
        
        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COT_Weather", "Weather_SetOvercast", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );

            GetGame().GetWeather().GetOvercast().Set( data.param1, data.param2, data.param3 );

            GetCommunityOnlineToolsBase().Log( sender, "Set overcast to " + data.param1 + " " + data.param2 + " " + data.param3 );
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

        if ( !GetPermissionsManager().HasPermission( "Weather.Fog", sender ) )
            return;
        
        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COT_Weather", "Weather_SetFog", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );

            GetGame().GetWeather().GetFog().Set( data.param1, data.param2, data.param3 );

            GetCommunityOnlineToolsBase().Log( sender, "Set fog to " + data.param1 + " " + data.param2 + " " + data.param3 );
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

        if ( !GetPermissionsManager().HasPermission( "Weather.Rain", sender ) )
            return;
        
        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COT_Weather", "Weather_SetRain", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );

            GetGame().GetWeather().GetRain().Set( data.param1, data.param2, data.param3 );

            GetCommunityOnlineToolsBase().Log( sender, "Set rain to " + data.param1 + " " + data.param2 + " " + data.param3 );
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

        if ( !GetPermissionsManager().HasPermission( "Weather.Storm", sender ) )
            return;
        
        if( type == CallType.Server && GetGame().IsMultiplayer() )
        {
            GetRPCManager().SendRPC( "COT_Weather", "Weather_SetStorm", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );

            GetGame().GetWeather().SetStorm( data.param1, data.param2, data.param3 );

            GetCommunityOnlineToolsBase().Log( sender, "Set storm to " + data.param1 + " " + data.param2 + " " + data.param3 );
        }
    }

}