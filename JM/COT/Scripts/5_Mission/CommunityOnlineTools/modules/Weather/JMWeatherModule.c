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
		GetPermissionsManager().RegisterPermission( "Weather.QuickAction" );
		GetPermissionsManager().RegisterPermission( "Weather.QuickAction.Clear" );
		GetPermissionsManager().RegisterPermission( "Weather.QuickAction.Cloudy" );
		GetPermissionsManager().RegisterPermission( "Weather.QuickAction.Storm" );
		GetPermissionsManager().RegisterPermission( "Weather.QuickAction.Date" );

		GetPermissionsManager().RegisterPermission( "Weather.Date" );

		GetPermissionsManager().RegisterPermission( "Weather.Wind" );
		GetPermissionsManager().RegisterPermission( "Weather.Wind.FunctionParams" );

		GetPermissionsManager().RegisterPermission( "Weather.Storm" );
		GetPermissionsManager().RegisterPermission( "Weather.Overcast" );

		GetPermissionsManager().RegisterPermission( "Weather.Fog" );
		GetPermissionsManager().RegisterPermission( "Weather.Fog.Dynamic" );

		GetPermissionsManager().RegisterPermission( "Weather.Rain" );
		GetPermissionsManager().RegisterPermission( "Weather.Rain.Thresholds" );

		GetPermissionsManager().RegisterPermission( "Weather.Snow" );
		GetPermissionsManager().RegisterPermission( "Weather.Snow.Thresholds" );

		GetPermissionsManager().RegisterPermission( "Weather.Preset" );
		GetPermissionsManager().RegisterPermission( "Weather.Preset.Use" );
		GetPermissionsManager().RegisterPermission( "Weather.Preset.Create" );
		GetPermissionsManager().RegisterPermission( "Weather.Preset.Update" );
		GetPermissionsManager().RegisterPermission( "Weather.Preset.Remove" );

		GetPermissionsManager().RegisterPermission( "Weather.View" );
	}

	override void EnableUpdate()
	{
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
		return "#STR_COT_WEATHER_MODULE_NAME";
	}
	
	override string GetIconName()
	{
		return "JM\\COT\\GUI\\textures\\modules\\Weather.paa";
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if (GetGame().IsServer())
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

	void Load()
	{
		if ( GetGame().IsClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMWeatherModuleRPC.Load, true, NULL );
		}
		else
		{
			settings = JMWeatherSerialize.Load();

			OnSettingsUpdated();
		}
	}

	bool IsLoaded()
	{
		return settings != NULL;
	}

	private void Server_Load( PlayerIdentity ident )
	{
		if ( !GetPermissionsManager().HasPermission( "Weather.Preset", ident ) )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( settings );
		rpc.Send( NULL, JMWeatherModuleRPC.Load, true, ident );
	}

	private void RPC_Load( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( GetGame().IsDedicatedServer() )
		{
			Server_Load( senderRPC );
		}
		else
		{
			if ( ctx.Read( settings ) )
			{
				OnSettingsUpdated();
			}
		}
	}

	void SetStorm( float density, float threshold, float minTimeBetweenLightning )
	{
		JMWeatherStorm wBase = new JMWeatherStorm;
		wBase.Density = density;
		wBase.Threshold = threshold;
		wBase.MinTimeBetweenLightning = minTimeBetweenLightning;

		if ( GetGame().IsServer() )
		{
			Exec_SetStorm( wBase, NULL );
		} else
		{
			Send_SetStorm( wBase );
		} 
	}

	void SetFog( float forecast, float time = 0, float minDuration = 0 )
	{
		JMWeatherFog wBase = new JMWeatherFog;
		wBase.Forecast = forecast;
		wBase.Time = time;
		wBase.MinDuration = minDuration;

		if ( GetGame().IsServer() )
		{
			Exec_SetFog( wBase, NULL );
		} else
		{
			Send_SetFog( wBase );
		} 
	}

	void SetDynamicFog( float distance, float height = 0, float bias = 0, float time = 0 )
	{
		JMWeatherDynamicFog wBase = new JMWeatherDynamicFog;
		wBase.Distance = distance;
		wBase.Height = height;
		wBase.Bias = bias;
		wBase.Time = time;

		if ( GetGame().IsServer() )
		{
			Exec_SetDynamicFog( wBase, NULL );
		} else
		{
			Send_SetDynamicFog( wBase );
		}
	}

	void SetRain( float forecast, float time = 0, float minDuration = 0 )
	{
		JMWeatherRain wBase = new JMWeatherRain;
		wBase.Forecast = forecast;
		wBase.Time = time;
		wBase.MinDuration = minDuration;

		if ( GetGame().IsServer() )
		{
			Exec_SetRain( wBase, NULL );
		} else
		{
			Send_SetRain( wBase );
		} 
	}

	void SetRainThresholds( float tMin, float tMax, float tTime )
	{
		JMWeatherRainThreshold wBase = new JMWeatherRainThreshold;
		wBase.OvercastMin = tMin;
		wBase.OvercastMax = tMax;
		wBase.Time = tTime;
		
		if ( GetGame().IsServer() )
		{
			Exec_SetRainThresholds( wBase, NULL );
		} else
		{
			Send_SetRainThresholds( wBase );
		} 
	}

	void SetSnow( float forecast, float time = 0, float minDuration = 0 )
	{
		JMWeatherSnow wBase = new JMWeatherSnow;
		wBase.Forecast = forecast;
		wBase.Time = time;
		wBase.MinDuration = minDuration;

		if ( GetGame().IsServer() )
		{
			Exec_SetSnow( wBase, NULL );
		} else
		{
			Send_SetSnow( wBase );
		} 
	}

	void SetSnowThresholds( float tMin, float tMax, float tTime )
	{
		JMWeatherSnowThreshold wBase = new JMWeatherSnowThreshold;
		wBase.OvercastMin = tMin;
		wBase.OvercastMax = tMax;
		wBase.Time = tTime;
		
		if ( GetGame().IsServer() )
		{
			Exec_SetSnowThresholds( wBase, NULL );
		} else
		{
			Send_SetSnowThresholds( wBase );
		} 
	}

	void SetOvercast( float forecast, float time = 0, float minDuration = 0 )
	{
		JMWeatherOvercast wBase = new JMWeatherOvercast;
		wBase.Forecast = forecast;
		wBase.Time = time;
		wBase.MinDuration = minDuration;

		if ( GetGame().IsServer() )
		{
			Exec_SetOvercast( wBase, NULL );
		} else
		{
			Send_SetOvercast( wBase );
		} 
	}

	void SetWindMagnitude( float forecast, float time = 0, float minDuration = 0 )
	{
		JMWeatherWindMagnitude wBase = new JMWeatherWindMagnitude;
		wBase.Forecast = forecast;
		wBase.Time = time;
		wBase.MinDuration = minDuration;

		if ( GetGame().IsServer() )
		{
			Exec_SetWindMagnitude( wBase, NULL );
		} else
		{
			Send_SetWindMagnitude( wBase );
		}
	}

	void SetWindDirection( float forecast, float time = 0, float minDuration = 0 )
	{
		JMWeatherWindDirection wBase = new JMWeatherWindDirection;
		wBase.Forecast = forecast;
		wBase.Time = time;
		wBase.MinDuration = minDuration;

		if ( GetGame().IsServer() )
		{
			Exec_SetWindDirection( wBase, NULL );
		} else
		{
			Send_SetWindDirection( wBase );
		}
	}

	void SetWindFunctionParams( float fnMin, float fnMax, float fnSpeed )
	{
		JMWeatherWindFunction wBase = new JMWeatherWindFunction;
		wBase.Min = fnMin;
		wBase.Max = fnMax;
		wBase.Speed = fnSpeed;

		if ( GetGame().IsServer() )
		{
			Exec_SetWindFunctionParams( wBase, NULL );
		} else
		{
			Send_SetWindFunctionParams( wBase );
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

		if ( GetGame().IsServer() )
		{
			Exec_SetDate( wBase, NULL );
		} else
		{
			Send_SetDate( wBase );
		}
	}

	void UsePreset( string name )
	{
		if ( GetGame().IsServer() )
		{
			Exec_UsePreset( name, NULL );
		} else
		{
			Send_UsePreset( name );
		}
	}

	void CreatePreset( JMWeatherPreset preset )
	{
		if ( GetGame().IsServer() )
		{
			Exec_CreatePreset( preset, NULL );
		} else
		{
			Send_CreatePreset( preset );
		}
	}

	void UpdatePreset( JMWeatherPreset preset )
	{
		if ( GetGame().IsServer() )
		{
			Exec_UpdatePreset( preset, NULL );
		} else
		{
			Send_UpdatePreset( preset );
		}
	}

	void RemovePreset( string name )
	{
		if ( GetGame().IsServer() )
		{
			Exec_RemovePreset( name, NULL );
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
	
	private void Send_SetDynamicFog( JMWeatherDynamicFog wBase )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wBase );
		rpc.Send( NULL, JMWeatherModuleRPC.DynamicFog, true, NULL );
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
	
	private void Send_SetSnow( JMWeatherSnow wBase )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wBase );
		rpc.Send( NULL, JMWeatherModuleRPC.Snow, true, NULL );
	}
	
	private void Send_SetSnowThresholds( JMWeatherSnowThreshold wBase )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wBase );
		rpc.Send( NULL, JMWeatherModuleRPC.SnowThresholds, true, NULL );
	}
	
	private void Send_SetOvercast( JMWeatherOvercast wBase )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wBase );
		rpc.Send( NULL, JMWeatherModuleRPC.Overcast, true, NULL );
	}
	
	private void Send_SetWindMagnitude( JMWeatherWindMagnitude wBase )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wBase );
		rpc.Send( NULL, JMWeatherModuleRPC.WindMagnitude, true, NULL );
	}
	
	private void Send_SetWindDirection( JMWeatherWindDirection wBase )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wBase );
		rpc.Send( NULL, JMWeatherModuleRPC.WindDirection, true, NULL );
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
	
	private void Exec_SetStorm( JMWeatherStorm wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetFog( JMWeatherFog wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetDynamicFog( JMWeatherDynamicFog wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetRain( JMWeatherRain wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetRainThresholds( JMWeatherRainThreshold wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetSnow( JMWeatherSnow wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetSnowThresholds( JMWeatherSnowThreshold wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetOvercast( JMWeatherOvercast wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetWindMagnitude( JMWeatherWindMagnitude wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetWindDirection( JMWeatherWindDirection wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetWindFunctionParams( JMWeatherWindFunction wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_SetDate( JMWeatherDate wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
	}

	private void Exec_UsePreset( string name, PlayerIdentity ident )
	{
		array< ref JMWeatherPreset > presets = GetPresets();
		JMWeatherPreset preset;

		for ( int i = 0; i < presets.Count(); i++ )
		{
			if ( presets[i].Name == name )
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

	private void Exec_CreatePreset( JMWeatherPreset preset, PlayerIdentity ident )
	{
		if ( preset == NULL )
			return;

		array< ref JMWeatherPreset > presets = GetPresets();
		for ( int i = 0; i < presets.Count(); i++ )
		{
			if ( presets[i].Name == preset.Name )
			{
				return;
			}
		}

		GetPresets().Insert( preset );

		OnSettingsUpdated();

		if (!GetGame().IsServer())
			return;

		GetCommunityOnlineToolsBase().Log( ident, "Created Weather Preset " + preset.Name );

		settings.Save();
	}

	private void Exec_UpdatePreset( JMWeatherPreset preset, PlayerIdentity ident )
	{
		if ( preset == NULL )
			return;

		array< ref JMWeatherPreset > presets = GetPresets();
		int index = -1;

		for ( int i = 0; i < presets.Count(); i++ )
		{
			if ( presets[i].Name == preset.Name )
			{
				index = i;
				break;
			}
		}

		if ( index == -1 )
			return;
		
		GetPresets().Remove( index );
		GetPresets().InsertAt( preset, index );

		OnSettingsUpdated();

		if (!GetGame().IsServer())
			return;

		GetCommunityOnlineToolsBase().Log( ident, "Updated Weather Preset " + preset.Name );

		settings.Save();
	}

	private void Exec_RemovePreset( string name, PlayerIdentity ident )
	{
		array< ref JMWeatherPreset > presets = GetPresets();
		int index = -1;

		for ( int i = 0; i < presets.Count(); i++ )
		{
			if ( presets[i].Name == name )
			{
				index = i;
				break;
			}
		}

		if ( index == -1 )
			return;
		
		GetPresets().Remove( index );

		OnSettingsUpdated();

		if (!GetGame().IsServer())
			return;

		GetCommunityOnlineToolsBase().Log( ident, "Removed Weather Preset (Name: " + name + ")" );

		settings.Save();
	}
	
	private void RPC_SetStorm( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherStorm p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!GetGame().IsServer())
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Storm", senderRPC ) )
			return;

		Exec_SetStorm( p1, senderRPC );
	}

	private void RPC_SetFog( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherFog p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!GetGame().IsServer())
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Fog", senderRPC ) )
			return;

		Exec_SetFog( p1, senderRPC );
	}

	private void RPC_SetDynamicFog( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherDynamicFog p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!GetGame().IsServer())
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Fog.Dynamic", senderRPC ) )
			return;

		Exec_SetDynamicFog( p1, senderRPC );
	}

	private void RPC_SetRain( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherRain p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!GetGame().IsServer())
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Rain", senderRPC ) )
			return;

		Exec_SetRain( p1, senderRPC );
	}

	private void RPC_SetRainThresholds( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherRainThreshold p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!GetGame().IsServer())
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Rain.Thresholds", senderRPC ) )
			return;

		Exec_SetRainThresholds( p1, senderRPC );
	}

	private void RPC_SetSnow( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherSnow p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!GetGame().IsServer())
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Snow", senderRPC ) )
			return;

		Exec_SetSnow( p1, senderRPC );
	}

	private void RPC_SetSnowThresholds( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherSnowThreshold p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!GetGame().IsServer())
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Snow.Thresholds", senderRPC ) )
			return;

		Exec_SetSnowThresholds( p1, senderRPC );
	}

	private void RPC_SetOvercast( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherOvercast p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!GetGame().IsServer())
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Overcast", senderRPC ) )
			return;

		Exec_SetOvercast( p1, senderRPC );
	}

	private void RPC_SetWindMagnitude( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherWindMagnitude p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!GetGame().IsServer())
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Wind", senderRPC ) )
			return

		Exec_SetWindMagnitude( p1, senderRPC );
	}

	private void RPC_SetWindDirection( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherWindDirection p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!GetGame().IsServer())
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Wind", senderRPC ) )
			return

		Exec_SetWindDirection( p1, senderRPC );
	}

	private void RPC_SetWindFunctionParams( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherWindFunction p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!GetGame().IsServer())
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Wind.FunctionParams", senderRPC ) )
			return;

		Exec_SetWindFunctionParams( p1, senderRPC );
	}

	private void RPC_SetDate( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherDate p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Date", senderRPC ) )
			return;

		if (GetGame().IsDedicatedServer())
		{
			//! Send date to client so it changes instantly, else game may take a few secs to sync to client. Not needed for weather!
			Send_SetDate(p1);
		}

		Exec_SetDate( p1, senderRPC );
	}

	private void RPC_UsePreset( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!GetGame().IsServer())
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Preset.Use", senderRPC ) )
			return;

		Exec_UsePreset( p1, senderRPC );
	}

	private void RPC_CreatePreset( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_CreatePreset").Add(senderRPC);
		#endif

		JMWeatherPreset p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Preset.Create", senderRPC ) )
			return;

		if ( GetGame().IsDedicatedServer() )
		{
			Send_CreatePreset( p1 );
		}

		Exec_CreatePreset( p1, senderRPC );
	}

	private void RPC_UpdatePreset( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherPreset p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Preset.Update", senderRPC ) )
			return;

		if ( GetGame().IsDedicatedServer() )
		{
			Send_UpdatePreset( p1 );
		}

		Exec_UpdatePreset( p1, senderRPC );
	}

	private void RPC_RemovePreset( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string p1;
		if ( !ctx.Read( p1 ) )
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Preset.Remove", senderRPC ) )
			return;

		if ( GetGame().IsDedicatedServer() )
		{
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

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
	#ifdef DIAG
		auto trace = CF_Trace_1(this).Add(typename.EnumToString(JMWeatherModuleRPC, rpc_type));
	#endif

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
		case JMWeatherModuleRPC.DynamicFog:
			RPC_SetDynamicFog( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.Rain:
			RPC_SetRain( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.RainThresholds:
			RPC_SetRainThresholds( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.Snow:
			RPC_SetSnow( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.SnowThresholds:
			RPC_SetSnowThresholds( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.Overcast:
			RPC_SetOvercast( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.WindMagnitude:
			RPC_SetWindMagnitude( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.WindDirection:
			RPC_SetWindDirection( ctx, sender, target );
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
	}
};
