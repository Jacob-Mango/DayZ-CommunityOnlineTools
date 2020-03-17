enum JMWeatherModuleRPC
{
    INVALID = 10280,
	Load,
    Storm,
	Fog,
	Rain,
	RainThresholds,
	Overcast,
	Wind,
	WindFunctionParams,
	Date,
	UsePreset,
	CreatePreset,
	UpdatePreset,
	RemovePreset,
    COUNT
};

enum JMWeatherTypes
{
	INVALID,
	Phenomenon,
	Mission,
	Preset,
	COUNT
};

class JMWeatherModule: JMRenderableModuleBase
{
	private ref JMWeatherSerialize settings;

	void JMWeatherModule()
	{
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

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Weather.View" );
	}

	override string GetInputToggle()
	{
		return "UACOTToggleWeather";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/weather_form.layout";
	}
	
	override string GetTitle()
	{
		return "Weather";
	}
	
	override string GetIconName()
	{
		return "W";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		Load();
	}

	bool HasSettings()
	{
		return settings != NULL;
	}

	array< ref JMWeatherPreset > GetPresets()
	{
		return settings.Presets;
	}
	
	override void OnSettingsUpdated()
	{
		super.OnSettingsUpdated();

		if ( settings )
		{
			if ( !settings.Presets )
				return;

			for ( int i = 0; i < settings.Presets.Count(); i++ )
			{
				JMWeatherPreset location = settings.Presets[i];

				string permission = location.Permission;
				permission.Replace( " ", "." );
				GetPermissionsManager().RegisterPermission( "Weather.Preset." + permission );
			}
		}
	}

	void Load()
	{
		if ( IsMissionClient() && !IsMissionOffline() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMWeatherModuleRPC.Load, true, NULL );
		} else
		{
			settings = JMWeatherSerialize.Load();

			OnSettingsUpdated();
		}
	}

	private void Server_Load( PlayerIdentity ident )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( settings );
		rpc.Send( NULL, JMWeatherModuleRPC.Load, true, ident );
	}

	private void RPC_Load( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			Server_Load( senderRPC );
		}

		if ( IsMissionClient() )
		{
			if ( ctx.Read( settings ) )
			{
				OnSettingsUpdated();
			}
		}
	}

	void SetStorm( float density, float threshold, float timeOut )
	{
		JMWeatherStorm wBase = new JMWeatherStorm;
		wBase.Density = density;
		wBase.Threshold = threshold;
		wBase.TimeOut = timeOut;

		if ( IsMissionOffline() )
		{
			Exec_SetStorm( wBase );
		} else
		{
			Send_SetStorm( wBase );
			
			if ( IsMissionHost() )
			{
				Exec_SetStorm( wBase );
			}
		} 
	}

	void SetFog( float forecast, float time = 0, float minDuration = 0 )
	{
		JMWeatherFog wBase = new JMWeatherFog;
		wBase.Forecast = forecast;
		wBase.Time = time;
		wBase.MinDuration = minDuration;

		if ( IsMissionOffline() )
		{
			Exec_SetFog( wBase );
		} else
		{
			Send_SetFog( wBase );
			
			if ( IsMissionHost() )
			{
				Exec_SetFog( wBase );
			}
		} 
	}

	void SetRain( float forecast, float time = 0, float minDuration = 0 )
	{
		JMWeatherRain wBase = new JMWeatherRain;
		wBase.Forecast = forecast;
		wBase.Time = time;
		wBase.MinDuration = minDuration;

		if ( IsMissionOffline() )
		{
			Exec_SetRain( wBase );
		} else
		{
			Send_SetRain( wBase );
			
			if ( IsMissionHost() )
			{
				Exec_SetRain( wBase );
			}
		} 
	}

	void SetRainThresholds( float tMin, float tMax, float tTime )
	{
		JMWeatherRainThreshold wBase = new JMWeatherRainThreshold;
		wBase.OvercastMin = tMin;
		wBase.OvercastMax = tMax;
		wBase.Time = tTime;
		
		if ( IsMissionOffline() )
		{
			Exec_SetRainThresholds( wBase );
		} else
		{
			Send_SetRainThresholds( wBase );
			
			if ( IsMissionHost() )
			{
				Exec_SetRainThresholds( wBase );
			}
		} 
	}

	void SetOvercast( float forecast, float time = 0, float minDuration = 0 )
	{
		JMWeatherOvercast wBase = new JMWeatherOvercast;
		wBase.Forecast = forecast;
		wBase.Time = time;
		wBase.MinDuration = minDuration;

		if ( IsMissionOffline() )
		{
			Exec_SetOvercast( wBase );
		} else
		{
			Send_SetOvercast( wBase );
			
			if ( IsMissionHost() )
			{
				Exec_SetOvercast( wBase );
			}
		} 
	}

	void SetWind( vector dir, float speed, float maxSpeed = -1 )
	{
		JMWeatherWind wBase = new JMWeatherWind;

		wBase.Dir = dir;
		wBase.Speed = speed;
		if ( maxSpeed == -1 )
		{
			wBase.MaxSpeed = GetGame().GetWeather().GetWindMaximumSpeed();
		} else
		{
			wBase.MaxSpeed = maxSpeed;
		}

		if ( IsMissionOffline() )
		{
			Exec_SetWind( wBase );
		} else
		{
			Send_SetWind( wBase );

			if ( IsMissionHost() )
			{
				Exec_SetWind( wBase );
			}
		}
	}

	void SetWindFunctionParams( float fnMin, float fnMax, float fnSpeed )
	{
		JMWeatherWindFunction wBase = new JMWeatherWindFunction;
		wBase.Min = fnMin;
		wBase.Max = fnMax;
		wBase.Speed = fnSpeed;

		if ( IsMissionOffline() )
		{
			Exec_SetWindFunctionParams( wBase );
		} else
		{
			Send_SetWindFunctionParams( wBase );

			if ( IsMissionHost() )
			{
				Exec_SetWindFunctionParams( wBase );
			}
		} 
	}

	void SetDate( int year, int month, int day, int hour, int minute )
	{
		JMWeatherDate wBase = new JMWeatherDate;
		wBase.Year = year;
		wBase.Month = month;
		wBase.Day = day;
		wBase.Hour = hour;
		wBase.Minute = minute;

		if ( IsMissionOffline() )
		{
			Exec_SetDate( wBase );
		} else
		{
			Send_SetDate( wBase );

			if ( IsMissionHost() )
			{
				Exec_SetDate( wBase );
			}
		}
	}

	void UsePreset( string name )
	{
		if ( IsMissionOffline() )
		{
			Exec_UsePreset( name );
		} else
		{
			Send_UsePreset( name );

			if ( IsMissionHost() )
			{
				Exec_UsePreset( name );
			}
		}
	}

	void CreatePreset( JMWeatherPreset preset )
	{
		if ( IsMissionHost() )
		{
			Exec_CreatePreset( preset );
		} else
		{
			Send_CreatePreset( preset );
		}
	}

	void UpdatePreset( JMWeatherPreset preset )
	{
		if ( IsMissionHost() )
		{
			Exec_UpdatePreset( preset );
		} else
		{
			Send_UpdatePreset( preset );
		}
	}

	void RemovePreset( string name )
	{
		if ( IsMissionHost() )
		{
			Exec_RemovePreset( name );
		} else
		{
			Send_RemovePreset( name );
		}
	}
	
	private void Send_SetStorm( JMWeatherStorm wBase )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wBase );
		rpc.Send( NULL, JMWeatherModuleRPC.Storm, true, NULL );
	}
	
	private void Send_SetFog( JMWeatherFog wBase )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wBase );
		rpc.Send( NULL, JMWeatherModuleRPC.Fog, true, NULL );
	}
	
	private void Send_SetRain( JMWeatherRain wBase )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wBase );
		rpc.Send( NULL, JMWeatherModuleRPC.Rain, true, NULL );
	}
	
	private void Send_SetRainThresholds( JMWeatherRainThreshold wBase )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wBase );
		rpc.Send( NULL, JMWeatherModuleRPC.RainThresholds, true, NULL );
	}
	
	private void Send_SetOvercast( JMWeatherOvercast wBase )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wBase );
		rpc.Send( NULL, JMWeatherModuleRPC.Overcast, true, NULL );
	}
	
	private void Send_SetWind( JMWeatherWind wBase )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wBase );
		rpc.Send( NULL, JMWeatherModuleRPC.Wind, true, NULL );
	}

	private void Send_SetWindFunctionParams( JMWeatherWindFunction wBase )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wBase );
		rpc.Send( NULL, JMWeatherModuleRPC.WindFunctionParams, true, NULL );
	}

	private void Send_SetDate( JMWeatherDate wBase )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wBase );
		rpc.Send( NULL, JMWeatherModuleRPC.Date, true, NULL );
	}

	private void Send_UsePreset( string name )
	{
		ScriptRPC rpc = new ScriptRPC();

		rpc.Write( name );

		rpc.Send( NULL, JMWeatherModuleRPC.UsePreset, true, NULL );
	}

	private void Send_CreatePreset( JMWeatherPreset preset )
	{
		ScriptRPC rpc = new ScriptRPC();

		rpc.Write( preset );

		rpc.Send( NULL, JMWeatherModuleRPC.CreatePreset, true, NULL );
	}

	private void Send_UpdatePreset( JMWeatherPreset preset )
	{
		ScriptRPC rpc = new ScriptRPC();

		rpc.Write( preset );

		rpc.Send( NULL, JMWeatherModuleRPC.UpdatePreset, true, NULL );
	}

	private void Send_RemovePreset( string name )
	{
		ScriptRPC rpc = new ScriptRPC();

		rpc.Write( name );

		rpc.Send( NULL, JMWeatherModuleRPC.RemovePreset, true, NULL );
	}
	
	private void Exec_SetStorm( JMWeatherStorm wBase, PlayerIdentity ident = NULL )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetFog( JMWeatherFog wBase, PlayerIdentity ident = NULL )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetRain( JMWeatherRain wBase, PlayerIdentity ident = NULL )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetRainThresholds( JMWeatherRainThreshold wBase, PlayerIdentity ident = NULL )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetOvercast( JMWeatherOvercast wBase, PlayerIdentity ident = NULL )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetWind( JMWeatherWind wBase, PlayerIdentity ident = NULL )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetWindFunctionParams( JMWeatherWindFunction wBase, PlayerIdentity ident = NULL )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetDate( JMWeatherDate wBase, PlayerIdentity ident = NULL )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_UsePreset( string name, PlayerIdentity ident = NULL )
	{
		array< ref JMWeatherPreset > presets = GetPresets();
		JMWeatherPreset preset;

		for ( int i = 0; i < presets.Count(); i++ )
		{
			if ( presets[i].Permission == name )
			{
				preset = presets[i];
				break;
			}
		}

		if ( preset == NULL )
			return;

		preset.Apply();
		preset.Log( ident );
	}

	private void Exec_CreatePreset( JMWeatherPreset preset, PlayerIdentity ident = NULL )
	{
		if ( preset == NULL )
			return;

		array< ref JMWeatherPreset > presets = GetPresets();
		for ( int i = 0; i < presets.Count(); i++ )
		{
			if ( presets[i].Permission == preset.Permission )
			{
				return;
			}
		}

		GetPresets().Insert( preset );

		GetCommunityOnlineToolsBase().Log( ident, "Created Weather Preset " + preset.Name + " (Permission: " + preset.Permission + ")" );

		settings.Save();
	}

	private void Exec_UpdatePreset( JMWeatherPreset preset, PlayerIdentity ident = NULL )
	{
		if ( preset == NULL )
			return;

		array< ref JMWeatherPreset > presets = GetPresets();
		int index = -1;

		for ( int i = 0; i < presets.Count(); i++ )
		{
			if ( presets[i].Permission == preset.Permission )
			{
				index = i;
				break;
			}
		}

		if ( index == -1 )
			return;
		
		GetPresets().Remove( index );
		GetPresets().InsertAt( preset, index );

		GetCommunityOnlineToolsBase().Log( ident, "Updated Weather Preset " + preset.Name + " (Permission: " + preset.Permission + ")" );

		settings.Save();
	}

	private void Exec_RemovePreset( string name, PlayerIdentity ident = NULL )
	{
		array< ref JMWeatherPreset > presets = GetPresets();
		int index = -1;

		for ( int i = 0; i < presets.Count(); i++ )
		{
			if ( presets[i].Permission == name )
			{
				index = i;
				break;
			}
		}

		if ( index == -1 )
			return;
		
		GetPresets().Remove( index );

		GetCommunityOnlineToolsBase().Log( ident, "Removed Weather Preset (Permission: " + name + ")" );

		settings.Save();
	}
	
	private void RPC_SetStorm( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		JMWeatherStorm p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( IsMissionHost() )
		{
			if ( !GetPermissionsManager().HasPermission( "Weather.Storm", senderRPC ) )
				return;

			Send_SetStorm( p1 );
		}

		Exec_SetStorm( p1, senderRPC );
    }

	private void RPC_SetFog( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		JMWeatherFog p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( IsMissionHost() )
		{
			if ( !GetPermissionsManager().HasPermission( "Weather.Fog", senderRPC ) )
				return;

			Send_SetFog( p1 );
		}

		Exec_SetFog( p1, senderRPC );
    }

	private void RPC_SetRain( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		JMWeatherRain p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( IsMissionHost() )
		{
			if ( !GetPermissionsManager().HasPermission( "Weather.Rain", senderRPC ) )
				return;

			Send_SetRain( p1 );
		}

		Exec_SetRain( p1, senderRPC );
    }

	private void RPC_SetRainThresholds( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		JMWeatherRainThreshold p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( IsMissionHost() )
		{
			if ( !GetPermissionsManager().HasPermission( "Weather.Rain.Thresholds", senderRPC ) )
				return;

			Send_SetRainThresholds( p1 );
		}

		Exec_SetRainThresholds( p1, senderRPC );
    }

	private void RPC_SetOvercast( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		JMWeatherOvercast p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( IsMissionHost() )
		{
			if ( !GetPermissionsManager().HasPermission( "Weather.Overcast", senderRPC ) )
				return;

			Send_SetOvercast( p1 );
		}

		Exec_SetOvercast( p1, senderRPC );
    }

	private void RPC_SetWind( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		JMWeatherWind p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( IsMissionHost() )
		{
			if ( !GetPermissionsManager().HasPermission( "Weather.Wind", senderRPC ) )
				return;

			Send_SetWind( p1 );
		}

		Exec_SetWind( p1 );
    }

	private void RPC_SetWindFunctionParams( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		JMWeatherWindFunction p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( IsMissionHost() )
		{
			if ( !GetPermissionsManager().HasPermission( "Weather.Wind.FunctionParams", senderRPC ) )
				return;

			Send_SetWindFunctionParams( p1 );
		}

		Exec_SetWindFunctionParams( p1, senderRPC );
    }

	private void RPC_SetDate( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		JMWeatherDate p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( IsMissionHost() )
		{
			if ( !GetPermissionsManager().HasPermission( "Weather.Date", senderRPC ) )
				return;

			Send_SetDate( p1 );
		}

		Exec_SetDate( p1, senderRPC );
    }

	private void RPC_UsePreset( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		string p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( IsMissionHost() )
		{
			if ( !GetPermissionsManager().HasPermission( "Weather.Preset." + p1, senderRPC ) )
				return;

			Send_UsePreset( p1 );
		}

		Exec_UsePreset( p1, senderRPC );
    }

	private void RPC_CreatePreset( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		JMWeatherPreset p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( IsMissionHost() )
		{
			if ( !GetPermissionsManager().HasPermission( "Weather.Preset.Create", senderRPC ) )
				return;

			Send_CreatePreset( p1 );
		}

		Exec_CreatePreset( p1, senderRPC );
    }

	private void RPC_UpdatePreset( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		JMWeatherPreset p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( IsMissionHost() )
		{
			if ( !GetPermissionsManager().HasPermission( "Weather.Preset.Update", senderRPC ) )
				return;

			if ( !GetPermissionsManager().HasPermission( "Weather.Preset." + p1.Permission, senderRPC ) )
				return;

			Send_UpdatePreset( p1 );
		}

		Exec_UpdatePreset( p1, senderRPC );
    }

	private void RPC_RemovePreset( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
    {
		string p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( IsMissionHost() )
		{
			if ( !GetPermissionsManager().HasPermission( "Weather.Preset.Remove", senderRPC ) )
				return;

			if ( !GetPermissionsManager().HasPermission( "Weather.Preset." + p1, senderRPC ) )
				return;

			Send_RemovePreset( p1 );
		}

		Exec_RemovePreset( p1, senderRPC );
    }

	override int GetRPCMin()
	{
		return JMWeatherModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMWeatherModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ref ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMWeatherModuleRPC.Load:
			RPC_Load( ctx, sender, target );
			break
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
		case JMWeatherModuleRPC.WindFunctionParams:
			RPC_SetWindFunctionParams( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.Date:
			RPC_SetDate( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.UsePreset:
			RPC_UsePreset( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.CreatePreset:
			RPC_CreatePreset( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.UpdatePreset:
			RPC_UpdatePreset( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.RemovePreset:
			RPC_RemovePreset( ctx, sender, target );
			break;
		}

		GetGame().GetWeather().MissionWeather( true );
    }
}