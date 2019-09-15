enum JMWeatherModuleRPC
{
    INVALID = 10280,
    Storm,
	Fog,
	Rain,
	RainThresholds,
	Overcast,
	Wind,
	WindSpeed,
	WindFunctionParams,
	Date,
    COUNT
};

class JMWeatherModule: JMRenderableModuleBase
{
	void JMWeatherModule()
	{	
		GetPermissionsManager().RegisterPermission( "Weather.Date" );

		GetPermissionsManager().RegisterPermission( "Weather.Wind" );
		GetPermissionsManager().RegisterPermission( "Weather.Wind.FunctionParams" );
		GetPermissionsManager().RegisterPermission( "Weather.Wind.Speed" );

		GetPermissionsManager().RegisterPermission( "Weather.Storm" );
		GetPermissionsManager().RegisterPermission( "Weather.Overcast" );
		GetPermissionsManager().RegisterPermission( "Weather.Fog" );

		GetPermissionsManager().RegisterPermission( "Weather.Rain" );
		GetPermissionsManager().RegisterPermission( "Weather.Rain.Thresholds" );
	
		GetPermissionsManager().RegisterPermission( "Weather.View" );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Weather.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/weather_form.layout";
	}

	void SetStorm( float density, float threshold, float timeOut )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetStorm( density, threshold, timeOut );
		} else
		{
			Send_SetStorm( density, threshold, timeOut );
		} 
	}
	
	private void Send_SetStorm( float density, float threshold, float timeOut )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( density );
		rpc.Write( threshold );
		rpc.Write( timeOut );
		rpc.Send( NULL, JMWeatherModuleRPC.Storm, true, NULL );
	}

	void SetFog( float forecast, float time = 0, float minDuration = 0 )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetFog( forecast, time, minDuration );
		} else
		{
			Send_SetFog( forecast, time, minDuration );
		} 
	}
	
	private void Send_SetFog( float forecast, float time = 0, float minDuration = 0 )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( forecast );
		rpc.Write( time );
		rpc.Write( minDuration );
		rpc.Send( NULL, JMWeatherModuleRPC.Fog, true, NULL );
	}

	void SetRain( float forecast, float time = 0, float minDuration = 0 )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetRain( forecast, time, minDuration );
		} else
		{
			Send_SetRain( forecast, time, minDuration );
		} 
	}
	
	private void Send_SetRain( float forecast, float time = 0, float minDuration = 0 )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( forecast );
		rpc.Write( time );
		rpc.Write( minDuration );
		rpc.Send( NULL, JMWeatherModuleRPC.Rain, true, NULL );
	}

	void SetRainThresholds( float tMin, float tMax, float tTime )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetRainThresholds( tMin, tMax, tTime );
		} else
		{
			Send_SetRainThresholds( tMin, tMax, tTime );
		} 
	}
	
	private void Send_SetRainThresholds( float tMin, float tMax, float tTime )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( tMin );
		rpc.Write( tMax );
		rpc.Write( tTime );
		rpc.Send( NULL, JMWeatherModuleRPC.RainThresholds, true, NULL );
	}

	void SetOvercast( float forecast, float time = 0, float minDuration = 0 )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetOvercast( forecast, time, minDuration );
		} else
		{
			Send_SetOvercast( forecast, time, minDuration );
		} 
	}
	
	private void Send_SetOvercast( float forecast, float time = 0, float minDuration = 0 )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( forecast );
		rpc.Write( time );
		rpc.Write( minDuration );
		rpc.Send( NULL, JMWeatherModuleRPC.Overcast, true, NULL );
	}

	void SetWind( vector wind )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetWind( wind );
		} else
		{
			Send_SetWind( wind );
		} 
	}
	
	private void Send_SetWind( vector wind )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wind );
		rpc.Send( NULL, JMWeatherModuleRPC.Wind, true, NULL );
	}

	void SetWindSpeed( float speed )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetWindSpeed( speed );
		} else
		{
			Send_SetWindSpeed( speed );
		} 
	}
	
	private void Send_SetWindSpeed( float speed )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( speed );
		rpc.Send( NULL, JMWeatherModuleRPC.WindSpeed, true, NULL );
	}

	void SetWindFunctionParams( float fnMin, float fnMax, float fnSpeed )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetWindFunctionParams( fnMin, fnMax, fnSpeed );
		} else
		{
			Send_SetWindFunctionParams( fnMin, fnMax, fnSpeed );
		} 
	}

	private void Send_SetWindFunctionParams( float fnMin, float fnMax, float fnSpeed )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( fnMin );
		rpc.Write( fnMax );
		rpc.Write( fnSpeed );
		rpc.Send( NULL, JMWeatherModuleRPC.WindFunctionParams, true, NULL );
	}

	void SetDate( int year, int month, int day, int hour, int minute )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetDate( year, month, day, hour, minute );
		} else
		{
			Send_SetDate( year, month, day, hour, minute );
		}
	}

	private void Send_SetDate( int year, int month, int day, int hour, int minute )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( year );
		rpc.Write( month );
		rpc.Write( day );
		rpc.Write( hour );
		rpc.Write( minute );
		rpc.Send( NULL, JMWeatherModuleRPC.Date, true, NULL );
	}
	
	private void Exec_SetStorm( float density, float threshold, float timeOut, PlayerIdentity ident = NULL )
	{
		GetGame().GetWeather().SetStorm( density, threshold, timeOut );

		if ( IsMissionHost() )
			GetCommunityOnlineToolsBase().Log( ident, "Set storm to " + density + " " + threshold + " " + timeOut );
	}

	private void Exec_SetFog( float forecast, float time = 0, float minDuration = 0, PlayerIdentity ident = NULL )
	{
		GetGame().GetWeather().GetFog().Set( forecast, time, minDuration );

		if ( IsMissionHost() )
			GetCommunityOnlineToolsBase().Log( ident, "Set fog to " + forecast + " " + time + " " + minDuration );
	}

	private void Exec_SetRain( float forecast, float time = 0, float minDuration = 0, PlayerIdentity ident = NULL )
	{
		GetGame().GetWeather().GetRain().Set( forecast, time, minDuration );

		if ( IsMissionHost() )
			GetCommunityOnlineToolsBase().Log( ident, "Set rain to " + forecast + " " + time + " " + minDuration );
	}

	private void Exec_SetRainThresholds( float tMin, float tMax, float tTime, PlayerIdentity ident = NULL )
	{
		GetGame().GetWeather().SetRainThresholds( tMin, tMax, tTime );

		if ( IsMissionHost() )
			GetCommunityOnlineToolsBase().Log( ident, "Set wind function to " + tMin + " " + tMax + " " + tTime );
	}

	private void Exec_SetOvercast( float forecast, float time = 0, float minDuration = 0, PlayerIdentity ident = NULL )
	{
		GetGame().GetWeather().GetOvercast().Set( forecast, time, minDuration );

		if ( IsMissionHost() )
			GetCommunityOnlineToolsBase().Log( ident, "Set overcast to " + forecast + " " + time + " " + minDuration );
	}

	private void Exec_SetWind( vector wind, PlayerIdentity ident = NULL )
	{
		GetGame().GetWeather().SetWind( wind );

		if ( IsMissionHost() )
			GetCommunityOnlineToolsBase().Log( ident, "Set wind to " + wind );
	}

	private void Exec_SetWindSpeed( float speed, PlayerIdentity ident = NULL )
	{
		GetGame().GetWeather().SetWindSpeed( speed );

		if ( IsMissionHost() )
			GetCommunityOnlineToolsBase().Log( ident, "Set wind speed to " + speed );
	}

	private void Exec_SetWindFunctionParams( float fnMin, float fnMax, float fnSpeed, PlayerIdentity ident = NULL )
	{
		GetGame().GetWeather().SetWindFunctionParams( fnMin, fnMax, fnSpeed );

		if ( IsMissionHost() )
			GetCommunityOnlineToolsBase().Log( ident, "Set wind function to " + fnMin + " " + fnMax + " " + fnSpeed );
	}

	private void Exec_SetDate( int year, int month, int day, int hour, int minute, PlayerIdentity ident = NULL )
	{
		GetGame().GetWorld().SetDate( year, month, day, hour, minute );

		if ( IsMissionHost() )
			GetCommunityOnlineToolsBase().Log( ident, "Set date to " + year + "/" + month + "/" + day + " " + hour + ":" + minute );
	}
	
	private void RPC_SetStorm( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		float p1;
		if ( !ctx.Read( p1 ) )
			return;
		float p2;
		if ( !ctx.Read( p2 ) )
			return;
		float p3;
		if ( !ctx.Read( p3 ) )
			return;

		if ( IsMissionHost() )
			Send_SetStorm( p1, p2, p3 );

		Exec_SetStorm( p1, p2, p3 );
    }

	private void RPC_SetFog( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		float p1;
		if ( !ctx.Read( p1 ) )
			return;
		float p2;
		if ( !ctx.Read( p2 ) )
			return;
		float p3;
		if ( !ctx.Read( p3 ) )
			return;

		if ( IsMissionHost() )
			Send_SetFog( p1, p2, p3 );

		Exec_SetFog( p1, p2, p3 );
    }

	private void RPC_SetRain( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		float p1;
		if ( !ctx.Read( p1 ) )
			return;
		float p2;
		if ( !ctx.Read( p2 ) )
			return;
		float p3;
		if ( !ctx.Read( p3 ) )
			return;

		if ( IsMissionHost() )
			Send_SetRain( p1, p2, p3 );

		Exec_SetRain( p1, p2, p3 );
    }

	private void RPC_SetRainThresholds( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		float p1;
		if ( !ctx.Read( p1 ) )
			return;
		float p2;
		if ( !ctx.Read( p2 ) )
			return;
		float p3;
		if ( !ctx.Read( p3 ) )
			return;

		if ( IsMissionHost() )
			Send_SetRainThresholds( p1, p2, p3 );

		Exec_SetRainThresholds( p1, p2, p3 );
    }

	private void RPC_SetOvercast( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		float p1;
		if ( !ctx.Read( p1 ) )
			return;
		float p2;
		if ( !ctx.Read( p2 ) )
			return;
		float p3;
		if ( !ctx.Read( p3 ) )
			return;

		if ( IsMissionHost() )
			Send_SetOvercast( p1, p2, p3 );

		Exec_SetOvercast( p1, p2, p3 );
    }

	private void RPC_SetWind( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		vector p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( IsMissionHost() )
			Send_SetWind( p1 );

		Exec_SetWind( p1 );
    }

	private void RPC_SetWindSpeed( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		float p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( IsMissionHost() )
			Send_SetWindSpeed( p1 );

		Exec_SetWindSpeed( p1 );
    }

	private void RPC_SetWindFunctionParams( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		float p1;
		if ( !ctx.Read( p1 ) )
			return;
		float p2;
		if ( !ctx.Read( p2 ) )
			return;
		float p3;
		if ( !ctx.Read( p3 ) )
			return;

		if ( IsMissionHost() )
			Send_SetWindFunctionParams( p1, p2, p3 );

		Exec_SetWindFunctionParams( p1, p2, p3 );
    }

	private void RPC_SetDate( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		int p1;
		if ( !ctx.Read( p1 ) )
			return;
		int p2;
		if ( !ctx.Read( p2 ) )
			return;
		int p3;
		if ( !ctx.Read( p3 ) )
			return;
		int p4;
		if ( !ctx.Read( p4 ) )
			return;
		int p5;
		if ( !ctx.Read( p5 ) )
			return;

		if ( IsMissionHost() )
			Send_SetDate( p1, p2, p3, p4, p5 );

		Exec_SetDate( p1, p2, p3, p4, p5 );
    }

	int GetRPCMin()
	{
		return JMWeatherModuleRPC.INVALID;
	}

	int GetRPCMax()
	{
		return JMWeatherModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ref ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMWeatherModuleRPC.Storm:
			RPC_SetStorm( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.Fog:
			RPC_SetFog( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.Rain:
			RPC_SetRain( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.RainThresholds:
			RPC_SetRainThresholds( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.Overcast:
			RPC_SetOvercast( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.Wind:
			RPC_SetWind( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.WindSpeed:
			RPC_SetWindSpeed( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.WindFunctionParams:
			RPC_SetWindFunctionParams( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.Date:
			RPC_SetDate( ctx, sender, target );
			break;
		}
    }
}