enum JMWeatherTypes
{
	INVALID,
	Phenomenon,
	Mission,
	Preset,
	COUNT
}

class JMWeatherModule: JMRenderableModuleBase
{
	private ref JMWeatherSerialize settings;
	protected ref JMWeatherPreset m_CachedWeatherPreset;

	protected bool m_bFreezeTime;

	void JMWeatherModule()
	{
		JMPermissions.Register( JMConstants.PERM_WEATHER_QUICKACTION );
		JMPermissions.Register( JMConstants.PERM_WEATHER_QUICKACTION_CLEAR );
		JMPermissions.Register( JMConstants.PERM_WEATHER_QUICKACTION_CLOUDY );
		JMPermissions.Register( JMConstants.PERM_WEATHER_QUICKACTION_STORM );
		JMPermissions.Register( JMConstants.PERM_WEATHER_QUICKACTION_DATE );

		JMPermissions.Register( JMConstants.PERM_WEATHER_FREEZETIME );

		JMPermissions.Register( JMConstants.PERM_WEATHER_DATE );

		JMPermissions.Register( JMConstants.PERM_WEATHER_WIND );
		JMPermissions.Register( JMConstants.PERM_WEATHER_WIND_FUNCPARAMS );

		JMPermissions.Register( JMConstants.PERM_WEATHER_STORM );
		JMPermissions.Register( JMConstants.PERM_WEATHER_SANDSTORM );
		JMPermissions.Register( JMConstants.PERM_WEATHER_OVERCAST );

		JMPermissions.Register( JMConstants.PERM_WEATHER_FOG );
		JMPermissions.Register( JMConstants.PERM_WEATHER_FOG_DYNAMIC );

		JMPermissions.Register( JMConstants.PERM_WEATHER_RAIN );
		JMPermissions.Register( JMConstants.PERM_WEATHER_RAIN_THRESHOLDS );

		JMPermissions.Register( JMConstants.PERM_WEATHER_SNOW );
		JMPermissions.Register( JMConstants.PERM_WEATHER_SNOW_THRESHOLDS );

		JMPermissions.Register( JMConstants.PERM_WEATHER_PRESET );
		JMPermissions.Register( JMConstants.PERM_WEATHER_PRESET_USE );
		JMPermissions.Register( JMConstants.PERM_WEATHER_PRESET_CREATE );
		JMPermissions.Register( JMConstants.PERM_WEATHER_PRESET_UPDATE );
		JMPermissions.Register( JMConstants.PERM_WEATHER_PRESET_REMOVE );
		JMPermissions.Register( JMConstants.PERM_WEATHER_PRESET_DYNAMIC );

		JMPermissions.Register( JMConstants.PERM_WEATHER_VIEW );
	}

	override void EnableUpdate()
	{
	}

	override bool HasAccess()
	{
		return JMPermissions.Has( JMConstants.PERM_WEATHER_VIEW );
	}

	override string GetInputToggle()
	{
		return "UACOTToggleWeather";
	}

	override void RegisterKeyMouseBindings()
	{
		super.RegisterKeyMouseBindings();

		Bind( new JMModuleBinding( "Input_FreezeTime",  "UAWeatherModuleFreezeTime",  true ) );
		Bind( new JMModuleBinding( "Input_CyclePreset", "UAWeatherModuleCyclePreset", true ) );
	}

	void Input_FreezeTime()
	{
		if ( !GetPermissionsManager().HasPermission( "Weather.FreezeTime" ) )
			return;
		SetFreezeTime( !IsTimeFrozen() );
	}

	void Input_CyclePreset()
	{
		if ( !GetPermissionsManager().HasPermission( "Weather.Preset.Use" ) )
			return;
		if ( !settings || !settings.Presets || settings.Presets.Count() == 0 )
			return;
		UsePreset( settings.Presets[0].Name );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/weather_form.layout";
	}
	
	override string GetCategory()
	{
		return "World";
	}

	override string GetTitle()
	{
		return "#STR_COT_WEATHER_MODULE_NAME";
	}

	override string GetWebhookTitle()
	{
		return "Weather Module";
	}

	override void GetWebhookTypes( out array<string> types )
	{
		types.Insert( "SetWeather" );
		types.Insert( "SetTime" );
		types.Insert( "UsePreset" );
		types.Insert( "CreatePreset" );
		types.Insert( "UpdatePreset" );
		types.Insert( "RemovePreset" );
		types.Insert( "DynamicWeather" );
	}

	override string GetIconName()
	{
		return JMConstants.Lucide( "cloud-sun" );
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

		if (g_Game.IsServer())
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

	string GetInitialPreset()
	{
		if ( settings )
			return settings.InitialPreset;
		return "";
	}

	void SetInitialPreset( string name )
	{
		if ( settings )
		{
			settings.InitialPreset = name;
			if ( g_Game.IsServer() )
				settings.Save();
		}
	}

	// -------------------------------------------------------------------------
	//  Dynamic weather
	// -------------------------------------------------------------------------

	bool IsDynamicEnabled()
	{
		if ( settings )
			return settings.DynamicEnabled;

		return false;
	}

	//! The stored preset by name, or NULL. Callers read the dynamic block off
	//! it directly - it is the preset that owns its own chain settings now.
	JMWeatherPreset GetPreset( string name )
	{
		if ( name == "" || !settings || !settings.Presets )
			return NULL;

		for ( int i = 0; i < settings.Presets.Count(); i++ )
		{
			if ( settings.Presets[i].Name == name )
				return settings.Presets[i];
		}

		return NULL;
	}

	void SetDynamicWeather( JMWeatherDynamicConfig config )
	{
		if ( g_Game.IsServer() )
			Exec_DynamicWeather( config, NULL );
		else
			Send_DynamicWeather( config );
	}

	void Load()
	{
		if ( g_Game.IsClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMWeatherModuleRPC.Load, true, NULL );
		}
		else
		{
			settings = JMWeatherSerialize.Load();

			SeedWeather();

			RestartDynamicWeather();

			OnSettingsUpdated();
		}
	}

	bool IsLoaded()
	{
		return settings != NULL;
	}
	
	bool IsTimeFrozen()
	{
		return m_bFreezeTime;
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
		if ( g_Game.IsDedicatedServer() )
		{
			if ( !senderRPC )
				return;
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

	//! Sandstorm (SandstormController.IsActive()) and, on Namalsk, EVR storm
	//! state are server-only - neither replicates to clients the way the
	//! vanilla WeatherPhenomenon forecasts (fog/rain/overcast/etc) do, so the
	//! Overview tab's live readout - which reads everything else straight off
	//! the client's own g_Game.GetWeather() - has no accurate client-side
	//! source for these two. Same request/response shape as Load() above:
	//! one RPC id, client sends empty, server answers with the two flags.
	bool m_LastSandstormActive;
	bool m_LastEVRStormActive;

	void RequestSpecialWeatherStatus()
	{
		if ( !g_Game.IsClient() )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMWeatherModuleRPC.SpecialWeatherStatus, true, NULL );
	}

	private void Server_SpecialWeatherStatus( PlayerIdentity ident )
	{
		if ( !ident )
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.View", ident ) )
			return;

		bool sandstormActive = g_Game.GetWeather().GetSandstorm().IsActive();

		bool evrActive;
		JMNamalskEventManagerModule namalskModule;
		if ( CF_Modules<JMNamalskEventManagerModule>.Get( namalskModule ) )
			evrActive = namalskModule.IsEventActive( "EVRStorm" ) || namalskModule.IsEventActive( "EVRStormDeadly" );

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( sandstormActive );
		rpc.Write( evrActive );
		rpc.Send( NULL, JMWeatherModuleRPC.SpecialWeatherStatus, true, ident );
	}

	private void RPC_SpecialWeatherStatus( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( g_Game.IsDedicatedServer() )
		{
			Server_SpecialWeatherStatus( senderRPC );
		}
		else
		{
			ctx.Read( m_LastSandstormActive );
			ctx.Read( m_LastEVRStormActive );
		}
	}

	void SetFreezeTime(bool state)
	{
		m_bFreezeTime = state;
		
		if ( g_Game.IsServer() )
			Exec_FreezeTime( m_bFreezeTime, NULL );
		else
			Send_FreezeTime( m_bFreezeTime );
	}

	void SetStorm( float density, float threshold, float minTimeBetweenLightning )
	{
		JMWeatherStorm wBase = new JMWeatherStorm;
		wBase.Density = density;
		wBase.Threshold = threshold;
		wBase.MinTimeBetweenLightning = minTimeBetweenLightning;

		if ( g_Game.IsServer() )
		{
			Exec_SetStorm( wBase, NULL );
		} else
		{
			Send_SetStorm( wBase );
		} 
	}

	void SetSandstorm( bool enabled, float duration, float fadeInTime = -1, float overcast = -1, float windMagnitude = -1 )
	{
		JMWeatherSandstorm wBase = new JMWeatherSandstorm;
		if ( enabled )
			wBase.Enabled = 1;
		else
			wBase.Enabled = 0;
		wBase.Duration = duration;
		wBase.FadeInTime = fadeInTime;
		wBase.OvercastValue = overcast;
		wBase.WindMagnitudeValue = windMagnitude;

		if ( g_Game.IsServer() )
		{
			Exec_SetSandstorm( wBase, NULL );
		} else
		{
			Send_SetSandstorm( wBase );
		}
	}

	void SetFog( float forecast, float time = 0, float minDuration = 0 )
	{
		JMWeatherFog wBase = new JMWeatherFog;
		wBase.Forecast = forecast;
		wBase.Time = time;
		wBase.MinDuration = minDuration;

		if ( g_Game.IsServer() )
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

		if ( g_Game.IsServer() )
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

		if ( g_Game.IsServer() )
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
		
		if ( g_Game.IsServer() )
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

		if ( g_Game.IsServer() )
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
		
		if ( g_Game.IsServer() )
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

		if ( g_Game.IsServer() )
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

		if ( g_Game.IsServer() )
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

		if ( g_Game.IsServer() )
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

		if ( g_Game.IsServer() )
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

		if ( g_Game.IsServer() )
		{
			//! A listen-server host calling this directly (server console, or the
			//! host's own admin UI) has no RPC round trip to piggyback the client
			//! push on - without this, already-connected clients only pick up the
			//! new date once the engine's own slow native calendar replication
			//! catches up, same as the bug RPC_SetDate's push exists to avoid.
			if ( g_Game.IsMultiplayer() )
				Send_SetDate( wBase );

			Exec_SetDate( wBase, NULL );
		} else
		{
			Send_SetDate( wBase );
		}
	}

	void UsePreset( string name )
	{
		if ( g_Game.IsServer() )
		{
			Exec_UsePreset( name, NULL );
		} else
		{
			Send_UsePreset( name );
		}
	}

	void CreatePreset( JMWeatherPreset preset )
	{
		if ( g_Game.IsServer() )
		{
			Exec_CreatePreset( preset, NULL );
		} else
		{
			Send_CreatePreset( preset );
		}
	}

	void UpdatePreset( JMWeatherPreset preset )
	{
		if ( g_Game.IsServer() )
		{
			Exec_UpdatePreset( preset, NULL );
		} else
		{
			Send_UpdatePreset( preset );
		}
	}

	void RemovePreset( string name )
	{
		if ( g_Game.IsServer() )
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
	
	private void Send_SetSandstorm( JMWeatherSandstorm wBase )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( wBase );
		rpc.Send( NULL, JMWeatherModuleRPC.Sandstorm, true, NULL );
	}

	private void Send_DynamicWeather( JMWeatherDynamicConfig config )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( config );
		rpc.Send( NULL, JMWeatherModuleRPC.DynamicWeather, true, NULL );
	}

	private void Send_FreezeTime( bool state )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( state );
		rpc.Send( NULL, JMWeatherModuleRPC.FreezeTime, true, NULL );
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
	
	private void Exec_FreezeTime( bool state, PlayerIdentity ident )
	{
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Repeat_FreezeTime);

		if (state)
		{
			//! Only the date is re-applied, so only the date is captured.
			//! A full SetFromWorld() would also snapshot phenomena this never
			//! writes back, and would read them on a path that has no reason
			//! to touch the weather at all.
			m_CachedWeatherPreset = new JMWeatherPreset;
			m_CachedWeatherPreset.PDate.SetFromWorld();
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Repeat_FreezeTime, 1000, true);
		}

		if ( IsMissionHost() )
		{
			string ftMsg;
			if ( state ) ftMsg = "Froze time"; else ftMsg = "Unfroze time";
			GetCommunityOnlineToolsBase().Log( ident, ftMsg );
			JMPlayerInstance ftInst = GetPermissionsManager().GetPlayer( ident.GetId() );
			SendWebhookColored( "SetTime", ftInst, ftMsg, JMConstants.WEBHOOK_COLOR_INFO );
		}
	}

	//! The DATE only, not the whole preset.
	//!
	//! This used to call m_CachedWeatherPreset.Apply(), which re-writes storm,
	//! overcast, fog, dynamic fog, rain, rain thresholds, snow, snow
	//! thresholds, wind magnitude, wind direction and the wind function - all
	//! of it, once a second, for as long as the clock was frozen. A control
	//! labelled "Freeze Time" was in practice pinning the entire weather state
	//! and continuously overwriting whatever the mission's weather controller
	//! was doing, which is one of the ways cfgweather.xml appeared to be
	//! ignored. Freezing the clock means re-stamping the clock.
	protected void Repeat_FreezeTime()
	{
		if ( m_CachedWeatherPreset )
			m_CachedWeatherPreset.PDate.Apply();
	}
	
	private void Exec_SetStorm( JMWeatherStorm wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( ident.GetId() );
		SendWebhookColored( "SetWeather", inst, "Set storm density=" + wBase.Density + " threshold=" + wBase.Threshold, JMConstants.WEBHOOK_COLOR_INFO );
	}

	private void Exec_SetSandstorm( JMWeatherSandstorm wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( ident.GetId() );
		SendWebhookColored( "SetWeather", inst, "Set sandstorm enabled=" + wBase.Enabled + " duration=" + wBase.Duration, JMConstants.WEBHOOK_COLOR_INFO );
	}

	private void Exec_SetFog( JMWeatherFog wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( ident.GetId() );
		SendWebhookColored( "SetWeather", inst, "Set fog=" + wBase.Forecast, JMConstants.WEBHOOK_COLOR_INFO );
	}

	private void Exec_SetDynamicFog( JMWeatherDynamicFog wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( ident.GetId() );
		SendWebhookColored( "SetWeather", inst, "Set dynamic fog dist=" + wBase.Distance + " height=" + wBase.Height, JMConstants.WEBHOOK_COLOR_INFO );
	}

	private void Exec_SetRain( JMWeatherRain wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( ident.GetId() );
		SendWebhookColored( "SetWeather", inst, "Set rain=" + wBase.Forecast, JMConstants.WEBHOOK_COLOR_INFO );
	}

	private void Exec_SetRainThresholds( JMWeatherRainThreshold wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( ident.GetId() );
		SendWebhookColored( "SetWeather", inst, "Set rain thresholds min=" + wBase.OvercastMin + " max=" + wBase.OvercastMax, JMConstants.WEBHOOK_COLOR_INFO );
	}

	private void Exec_SetSnow( JMWeatherSnow wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( ident.GetId() );
		SendWebhookColored( "SetWeather", inst, "Set snow=" + wBase.Forecast, JMConstants.WEBHOOK_COLOR_INFO );
	}

	private void Exec_SetSnowThresholds( JMWeatherSnowThreshold wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( ident.GetId() );
		SendWebhookColored( "SetWeather", inst, "Set snow thresholds min=" + wBase.OvercastMin + " max=" + wBase.OvercastMax, JMConstants.WEBHOOK_COLOR_INFO );
	}

	private void Exec_SetOvercast( JMWeatherOvercast wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( ident.GetId() );
		SendWebhookColored( "SetWeather", inst, "Set overcast=" + wBase.Forecast, JMConstants.WEBHOOK_COLOR_INFO );
	}

	private void Exec_SetWindMagnitude( JMWeatherWindMagnitude wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( ident.GetId() );
		SendWebhookColored( "SetWeather", inst, "Set wind magnitude=" + wBase.Forecast, JMConstants.WEBHOOK_COLOR_INFO );
	}

	private void Exec_SetWindDirection( JMWeatherWindDirection wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( ident.GetId() );
		SendWebhookColored( "SetWeather", inst, "Set wind direction=" + wBase.Forecast, JMConstants.WEBHOOK_COLOR_INFO );
	}

	private void Exec_SetWindFunctionParams( JMWeatherWindFunction wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );
		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( ident.GetId() );
		SendWebhookColored( "SetWeather", inst, "Set wind function params", JMConstants.WEBHOOK_COLOR_INFO );
	}

	private void Exec_SetDate( JMWeatherDate wBase, PlayerIdentity ident )
	{
		wBase.Apply();
		wBase.Log( ident );

		//! Time is frozen: Repeat_FreezeTime re-stamps the clock from
		//! m_CachedWeatherPreset every second. Without updating the cache
		//! here, that repeat overwrites this manual change back to the old
		//! date within ~1s of it being applied.
		if ( m_bFreezeTime && m_CachedWeatherPreset )
		{
			m_CachedWeatherPreset.PDate.Year   = wBase.Year;
			m_CachedWeatherPreset.PDate.Month  = wBase.Month;
			m_CachedWeatherPreset.PDate.Day    = wBase.Day;
			m_CachedWeatherPreset.PDate.Hour   = wBase.Hour;
			m_CachedWeatherPreset.PDate.Minute = wBase.Minute;
		}

		//! ident is NULL whenever the server applies a date on its own
		//! (e.g. SetDate called server-side) - GetId() on a null identity
		//! is not survivable.
		JMPlayerInstance inst;

		if ( ident )
			inst = GetPermissionsManager().GetPlayer( ident.GetId() );

		SendWebhookColored( "SetTime", inst, "Set date " + wBase.Year + "/" + wBase.Month + "/" + wBase.Day + " " + wBase.Hour + ":" + wBase.Minute, JMConstants.WEBHOOK_COLOR_INFO );
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

		//! ident is NULL whenever the server applies a preset on its own - the
		//! startup preset, or the dynamic chain - and GetId() on a null
		//! identity is not survivable.
		JMPlayerInstance upInst;

		if ( ident )
			upInst = GetPermissionsManager().GetPlayer( ident.GetId() );

		SendWebhookColored( "UsePreset", upInst, "Applied weather preset: " + name, JMConstants.WEBHOOK_COLOR_SUCCESS );
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

		if (!g_Game.IsServer())
			return;

		GetCommunityOnlineToolsBase().Log( ident, "Created Weather Preset " + preset.Name );
		JMPlayerInstance cpInst = GetPermissionsManager().GetPlayer( ident.GetId() );
		SendWebhookColored( "CreatePreset", cpInst, "Created weather preset: " + preset.Name, JMConstants.WEBHOOK_COLOR_SUCCESS );

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
		
		//! The incoming preset was built from the weather tabs, which know
		//! nothing about the chain - so it arrives with default chain settings
		//! that would overwrite whatever this preset was configured with. The
		//! two editors stay disjoint: Save Preset never touches the dynamic
		//! block, Save Dynamic Weather never touches the weather values.
		preset.CopyDynamicFrom( presets[index] );

		GetPresets().Remove( index );
		GetPresets().InsertAt( preset, index );

		OnSettingsUpdated();

		if (!g_Game.IsServer())
			return;

		GetCommunityOnlineToolsBase().Log( ident, "Updated Weather Preset " + preset.Name );

		//! ident is NULL when the mission host edits a preset itself.
		JMPlayerInstance updInst;

		if ( ident )
			updInst = GetPermissionsManager().GetPlayer( ident.GetId() );

		SendWebhookColored( "UpdatePreset", updInst, "Updated weather preset: " + preset.Name, JMConstants.WEBHOOK_COLOR_SUCCESS );

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

		//! Every edge into or out of the deleted preset is dead weight now, and
		//! a chain parked on it would stall on a node that no longer resolves.
		PruneTransitions( name );

		OnSettingsUpdated();

		if (!g_Game.IsServer())
			return;

		RestartDynamicWeather();

		GetCommunityOnlineToolsBase().Log( ident, "Removed Weather Preset (Name: " + name + ")" );
		JMPlayerInstance rmInst = GetPermissionsManager().GetPlayer( ident.GetId() );
		SendWebhookColored( "RemovePreset", rmInst, "Removed weather preset: " + name, JMConstants.WEBHOOK_COLOR_WARNING );

		settings.Save();
	}
	
	private void RPC_DynamicWeather( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherDynamicConfig config = new JMWeatherDynamicConfig;

		if ( !ctx.Read( config ) )
			return;

		if ( !g_Game.IsServer() )
			return;

		if ( !senderRPC )
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Preset.Dynamic", senderRPC ) )
			return;

		Exec_DynamicWeather( config, senderRPC );
	}

	private void Exec_DynamicWeather( JMWeatherDynamicConfig config, PlayerIdentity ident )
	{
		if ( !settings || !config )
			return;

		settings.DynamicEnabled = config.Enabled;

		//! An empty PresetName is the master switch on its own - sent when the
		//! toggle is flipped with no preset selected to speak for.
		JMWeatherPreset target = GetPreset( config.PresetName );

		if ( target )
		{
			target.InRotation = config.InRotation;

			target.DurationMin   = Math.Max( config.DurationMin, JMWeatherSerialize.DYNAMIC_MIN_INTERVAL );
			target.DurationMax   = Math.Max( config.DurationMax, target.DurationMin );
			target.TransitionMin = Math.Max( config.TransitionMin, 0 );
			target.TransitionMax = Math.Max( config.TransitionMax, target.TransitionMin );

			if ( config.NextStates )
				target.NextStates = config.NextStates;
		}

		//! A chain that has never run, or whose node was deleted out from under
		//! it, starts from the configured preset.
		if ( settings.CurrentPreset == "" || !HasPreset( settings.CurrentPreset ) )
			settings.CurrentPreset = RandomPreset();

		OnSettingsUpdated();

		if ( !g_Game.IsServer() )
			return;

		RestartDynamicWeather();

		string dwMsg;

		if ( target )
			dwMsg = "Set dynamic weather for preset " + target.Name + " (hold " + target.DurationMin + "-" + target.DurationMax + "s, fade " + target.TransitionMin + "-" + target.TransitionMax + "s)";
		else if ( config.Enabled )
			dwMsg = "Enabled dynamic weather";
		else
			dwMsg = "Disabled dynamic weather";

		GetCommunityOnlineToolsBase().Log( ident, dwMsg );

		//! ident is NULL when the mission host edits this itself.
		JMPlayerInstance dwInst;

		if ( ident )
			dwInst = GetPermissionsManager().GetPlayer( ident.GetId() );

		SendWebhookColored( "DynamicWeather", dwInst, dwMsg, JMConstants.WEBHOOK_COLOR_INFO );

		settings.Save();
	}

	//! What the weather starts as when the settings load.
	//!
	//! With dynamic weather on, the chain starts from a RANDOM preset every
	//! boot - the whole point of the chain is that no two server sessions run
	//! the same sequence, so resuming the previous one or always opening on the
	//! same preset would defeat it. InitialPreset is only consulted when the
	//! chain is off, where it keeps its original "apply this on startup"
	//! meaning for anyone still setting it in Weather.json.
	protected void SeedWeather()
	{
		if ( !settings )
			return;

		if ( !settings.DynamicEnabled )
		{
			if ( settings.InitialPreset != "" )
				Exec_UsePreset( settings.InitialPreset, NULL );

			return;
		}

		string start = RandomPreset();

		if ( start == "" )
			return;

		settings.CurrentPreset = start;

		//! Entered like any other state, with its own fade and hold, so the
		//! first state of a session behaves exactly like every one after it.
		EnterPreset( start );
	}

	//! A random preset from the rotation.
	//!
	//! Presets can opt out, which is how one that is only ever meant to be
	//! applied by hand stays off the start-of-session dice while remaining
	//! reachable from any preset that names it. If NOTHING is in rotation the
	//! flag is treated as unset rather than as "no weather at all".
	protected string RandomPreset()
	{
		if ( !settings || !settings.Presets || settings.Presets.Count() == 0 )
			return "";

		array<string> pool = new array<string>;

		int i;

		for ( i = 0; i < settings.Presets.Count(); i++ )
		{
			if ( settings.Presets[i].InRotation )
				pool.Insert( settings.Presets[i].Name );
		}

		if ( pool.Count() == 0 )
		{
			for ( i = 0; i < settings.Presets.Count(); i++ )
				pool.Insert( settings.Presets[i].Name );
		}

		return pool[ Math.RandomInt( 0, pool.Count() ) ];
	}

	bool HasPreset( string name )
	{
		if ( name == "" || !settings || !settings.Presets )
			return false;

		for ( int i = 0; i < settings.Presets.Count(); i++ )
		{
			if ( settings.Presets[i].Name == name )
				return true;
		}

		return false;
	}

	//! (Re)arm the roll timer. Safe to call on a client - it only ever removes
	//! the callback there, since the chain is driven entirely by the server.
	protected void RestartDynamicWeather()
	{
		g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( Repeat_DynamicWeather );

		if ( !g_Game.IsServer() || !settings || !settings.DynamicEnabled )
			return;

		ScheduleDynamicRoll( RollDuration( GetPreset( settings.CurrentPreset ) ) );
	}

	//! Every state holds for its own draw, so the timer is re-armed one roll at
	//! a time. A repeating CallLater could only ever run on a fixed period,
	//! which is exactly the thing the duration range replaces.
	protected void ScheduleDynamicRoll( int seconds )
	{
		g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( Repeat_DynamicWeather );
		g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( Repeat_DynamicWeather, seconds * 1000, false );
	}

	//! How long the given preset holds. Falls back to the defaults when there
	//! is no preset to ask - a chain with an empty or unresolved current state
	//! still has to come back and try again.
	protected int RollDuration( JMWeatherPreset preset )
	{
		int low  = JMWeatherSerialize.DYNAMIC_DEFAULT_DURATION_MIN;
		int high = JMWeatherSerialize.DYNAMIC_DEFAULT_DURATION_MAX;

		if ( preset )
		{
			low  = preset.DurationMin;
			high = preset.DurationMax;
		}

		int seconds = RandomBetween( low, high );

		if ( seconds < JMWeatherSerialize.DYNAMIC_MIN_INTERVAL )
			seconds = JMWeatherSerialize.DYNAMIC_MIN_INTERVAL;

		return seconds;
	}

	protected int RandomBetween( int low, int high )
	{
		if ( high <= low )
			return low;

		//! RandomInt's upper bound is exclusive, so the top of the range would
		//! otherwise never come up.
		return Math.RandomInt( low, high + 1 );
	}

	protected void Repeat_DynamicWeather()
	{
		if ( !settings || !settings.DynamicEnabled )
			return;

		string current = settings.CurrentPreset;

		if ( current == "" || !HasPreset( current ) )
			current = RandomPreset();

		JMWeatherPreset from = GetPreset( current );

		string next;

		if ( from )
			next = from.RollNext();

		JMWeatherPreset entered = GetPreset( next );

		//! The timing belongs to the preset being ENTERED, so the next state
		//! has to be picked before there is a hold to schedule. A preset knows
		//! how long it fades in over and how long it stays; the one it came
		//! from has no say in either.
		if ( entered )
		{
			settings.CurrentPreset = entered.Name;

			EnterPreset( entered.Name );

			settings.Save();

			ScheduleDynamicRoll( RollDuration( entered ) );
			return;
		}

		//! Nothing to move to - this preset names no candidates, or the one it
		//! drew has since been deleted. Hold where we are and come back, rather
		//! than stopping the chain dead.
		ScheduleDynamicRoll( RollDuration( from ) );
	}

	//! Apply a preset as the chain entering it: its own fade, its own hold.
	//!
	//! The hold is handed to the engine as the phenomena's MinDuration so the
	//! weather is not released back to cfgweather.xml before the chain returns
	//! for it. ScheduleDynamicRoll draws its own value from the same range,
	//! which is close enough - the two only have to agree in scale, and an
	//! exact match would mean the release and the roll racing each other.
	protected void EnterPreset( string name )
	{
		JMWeatherPreset preset = GetPreset( name );

		if ( !preset )
			return;

		int transition = RandomBetween( preset.TransitionMin, preset.TransitionMax );
		int duration   = RollDuration( preset );

		preset.ApplyTimed( transition, duration );

		//! Typed nulls, not a bare NULL: Log is overloaded on both
		//! PlayerIdentity and JMPlayerInstance, and a bare NULL matches
		//! neither on its own.
		PlayerIdentity noIdent;
		JMPlayerInstance noInstance;

		GetCommunityOnlineToolsBase().Log( noIdent, "Dynamic weather moved to preset " + name );
		SendWebhookColored( "DynamicWeather", noInstance, "Weather moved to preset: " + name, JMConstants.WEBHOOK_COLOR_INFO );
	}

	protected void PruneTransitions( string name )
	{
		if ( !settings )
			return;

		//! The deleted preset took its own candidates with it. What is left is
		//! every OTHER preset still pointing at it, which would otherwise draw
		//! a state that cannot be entered.
		if ( settings.Presets )
		{
			for ( int i = 0; i < settings.Presets.Count(); i++ )
				settings.Presets[i].PruneNextStates( name );
		}

		if ( settings.CurrentPreset == name )
			settings.CurrentPreset = "";

		if ( settings.InitialPreset == name )
			settings.InitialPreset = "";
	}

	private void RPC_FreezeTime( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool state;
		if ( !ctx.Read( state ) )
			return;

		if (!g_Game.IsServer())
			return;

		if ( !senderRPC )
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.FreezeTime", senderRPC ) )
			return;

		Exec_FreezeTime( state, senderRPC );
	}
	
	private void RPC_SetStorm( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherStorm p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!g_Game.IsServer())
			return;

		if ( !senderRPC )
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Storm", senderRPC ) )
			return;

		Exec_SetStorm( p1, senderRPC );
	}

	private void RPC_SetSandstorm( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherSandstorm p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!g_Game.IsServer())
			return;

		if ( !senderRPC )
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Sandstorm", senderRPC ) )
			return;

		Exec_SetSandstorm( p1, senderRPC );
	}

	private void RPC_SetFog( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherFog p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!g_Game.IsServer())
			return;

		if ( !senderRPC )
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

		if (!g_Game.IsServer())
			return;

		if ( !senderRPC )
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

		if (!g_Game.IsServer())
			return;

		if ( !senderRPC )
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

		if (!g_Game.IsServer())
			return;

		if ( !senderRPC )
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

		if (!g_Game.IsServer())
			return;

		if ( !senderRPC )
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

		if (!g_Game.IsServer())
			return;

		if ( !senderRPC )
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

		if (!g_Game.IsServer())
			return;

		if ( !senderRPC )
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

		if (!g_Game.IsServer())
			return;

		if ( !senderRPC )
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Wind", senderRPC ) )
			return;

		Exec_SetWindMagnitude( p1, senderRPC );
	}

	private void RPC_SetWindDirection( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherWindDirection p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!g_Game.IsServer())
			return;

		if ( !senderRPC )
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Wind", senderRPC ) )
			return;

		Exec_SetWindDirection( p1, senderRPC );
	}

	private void RPC_SetWindFunctionParams( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherWindFunction p1;
		if ( !ctx.Read( p1 ) )
			return;

		if (!g_Game.IsServer())
			return;

		if ( !senderRPC )
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

		//! A NULL senderRPC here isn't a missing-identity request - it's the server's own
		//! instant-sync push (see below) arriving on each client, which the engine delivers
		//! with no player attached. Apply it directly instead of falling into the permission
		//! gate below, which is for client -> server requests only. Without this branch the
		//! push was a silent no-op and clients only ever saw the new date once the engine's
		//! own slow native calendar replication eventually caught up.
		if ( !senderRPC )
		{
			if ( !g_Game.IsServer() )
				Exec_SetDate( p1, NULL );

			return;
		}

		if ( !GetPermissionsManager().HasPermission( "Weather.Date", senderRPC ) )
			return;

		if (g_Game.IsDedicatedServer())
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

		if (!g_Game.IsServer())
			return;

		if ( !senderRPC )
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

		if ( !senderRPC )
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Preset.Create", senderRPC ) )
			return;

		if ( g_Game.IsDedicatedServer() )
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

		if ( !senderRPC )
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Preset.Update", senderRPC ) )
			return;

		if ( g_Game.IsDedicatedServer() )
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

		if ( !senderRPC )
			return;

		if ( !GetPermissionsManager().HasPermission( "Weather.Preset.Remove", senderRPC ) )
			return;

		if ( g_Game.IsDedicatedServer() )
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
			break;
		case JMWeatherModuleRPC.FreezeTime:
			RPC_FreezeTime( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.Storm:
			RPC_SetStorm( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.Sandstorm:
			RPC_SetSandstorm( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.SpecialWeatherStatus:
			RPC_SpecialWeatherStatus( ctx, sender, target );
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
		case JMWeatherModuleRPC.DynamicWeather:
			RPC_DynamicWeather( ctx, sender, target );
			break;
		}
	}
}
