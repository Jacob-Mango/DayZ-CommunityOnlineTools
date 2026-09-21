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
	protected ref JMWeatherSerialize settings;
	protected ref JMWeatherPreset m_CachedWeatherPreset = new JMWeatherPreset;
	protected bool m_bFreezeTime;

	//! Sandstorm (SandstormController.IsActive()) and, on Namalsk, EVR storm
	//! state are server-only - neither replicates to clients the way the
	//! vanilla WeatherPhenomenon forecasts (fog/rain/overcast/etc) do, so the
	//! Overview tab's live readout - which reads everything else straight off
	//! the client's own g_Game.GetWeather() - has no accurate client-side
	//! source for these two. Same request/response shape as Load() above:
	//! one RPC id, client sends empty, server answers with the two flags.
	bool m_LastSandstormActive;
	bool m_LastEVRStormActive;

	//! The admin behind an action, or NULL when the server applies it on its
	//! own - the startup preset, the dynamic chain, the mission host editing
	//! directly. GetId() on a null identity is not survivable, so every
	//! Exec_* asks here instead of dereferencing it.
	protected JMPlayerInstance GetCaller( PlayerIdentity ident )
	{
		if ( !ident )
			return NULL;

		return GetPermissionsManager().GetPlayer( ident.GetId() );
	}

	string GetInitialPreset()
	{
		if ( settings )
			return settings.InitialPreset;
		return "";
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

	array< ref JMWeatherPreset > GetPresets()
	{
		return settings.Presets;
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

	bool HasSettings()
	{
		return settings != NULL;
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

	bool IsLoaded()
	{
		return settings != NULL;
	}

	bool IsTimeFrozen()
	{
		return m_bFreezeTime;
	}

	void SetDate( int year, int month, int day, int hour, int minute )
	{
		JMWeatherDate payload = new JMWeatherDate;
		payload.Year = year;
		payload.Month = month;
		payload.Day = day;
		payload.Hour = hour;
		payload.Minute = minute;

		Submit( payload );
	}

	void SetDynamicFog( float distance, float height = 0, float bias = 0, float time = 0 )
	{
		JMWeatherDynamicFog payload = new JMWeatherDynamicFog;
		payload.Distance = distance;
		payload.Height = height;
		payload.Bias = bias;
		payload.Time = time;

		Submit( payload );
	}

	void SetDynamicWeather( JMWeatherDynamicConfig config )
	{
		if ( g_Game.IsServer() )
			Exec_DynamicWeather( config, NULL );
		else
			Send_DynamicWeather( config );
	}

	void SetFog( float forecast, float time = 0, float minDuration = 0 )
	{
		SubmitPhenomenon( new JMWeatherFog, forecast, time, minDuration );
	}

	void SetFreezeTime(bool state)
	{
		m_bFreezeTime = state;

		if ( g_Game.IsServer() )
			Exec_FreezeTime( m_bFreezeTime, NULL );
		else
			Send_FreezeTime( m_bFreezeTime );
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

	void SetOvercast( float forecast, float time = 0, float minDuration = 0 )
	{
		SubmitPhenomenon( new JMWeatherOvercast, forecast, time, minDuration );
	}

	void SetRain( float forecast, float time = 0, float minDuration = 0 )
	{
		SubmitPhenomenon( new JMWeatherRain, forecast, time, minDuration );
	}

	void SetRainThresholds( float tMin, float tMax, float tTime )
	{
		JMWeatherRainThreshold payload = new JMWeatherRainThreshold;
		payload.OvercastMin = tMin;
		payload.OvercastMax = tMax;
		payload.Time = tTime;

		Submit( payload );
	}

	void SetSandstorm( bool enabled, float duration, float fadeInTime = -1, float overcast = -1, float windMagnitude = -1 )
	{
		JMWeatherSandstorm payload = new JMWeatherSandstorm;
		if ( enabled )
			payload.Enabled = 1;
		else
			payload.Enabled = 0;
		payload.Duration = duration;
		payload.FadeInTime = fadeInTime;
		payload.OvercastValue = overcast;
		payload.WindMagnitudeValue = windMagnitude;

		Submit( payload );
	}

	void SetSnow( float forecast, float time = 0, float minDuration = 0 )
	{
		SubmitPhenomenon( new JMWeatherSnow, forecast, time, minDuration );
	}

	void SetSnowThresholds( float tMin, float tMax, float tTime )
	{
		JMWeatherSnowThreshold payload = new JMWeatherSnowThreshold;
		payload.OvercastMin = tMin;
		payload.OvercastMax = tMax;
		payload.Time = tTime;

		Submit( payload );
	}

	void SetStorm( float density, float threshold, float minTimeBetweenLightning )
	{
		JMWeatherStorm payload = new JMWeatherStorm;
		payload.Density = density;
		payload.Threshold = threshold;
		payload.MinTimeBetweenLightning = minTimeBetweenLightning;

		Submit( payload );
	}

	void SetWindDirection( float forecast, float time = 0, float minDuration = 0 )
	{
		SubmitPhenomenon( new JMWeatherWindDirection, forecast, time, minDuration );
	}

	void SetWindFunctionParams( float fnMin, float fnMax, float fnSpeed )
	{
		JMWeatherWindFunction payload = new JMWeatherWindFunction;
		payload.Min = fnMin;
		payload.Max = fnMax;
		payload.Speed = fnSpeed;

		Submit( payload );
	}

	void SetWindMagnitude( float forecast, float time = 0, float minDuration = 0 )
	{
		SubmitPhenomenon( new JMWeatherWindMagnitude, forecast, time, minDuration );
	}

	override void DescribeModule( JMModuleInfo info )
	{
		super.DescribeModule( info );

		info.Title = "#STR_COT_WEATHER_MODULE_NAME";
		info.WebhookTitle = "Weather Module";
		info.Icon = "cloud-sun";
		info.Layout = "JM/COT/GUI/layouts/weather_form.layout";
		info.Category = JMSideBarConfig.CATEGORY_WORLD;
		info.ViewPermission = JMConstants.PERM_WEATHER_VIEW;
		info.InputToggle = "UACOTToggleWeather";
		info.SetRPCRange( JMWeatherModuleRPC.INVALID, JMWeatherModuleRPC.COUNT );

		//! Called on both client and server as the module registers, before the mission loads.
		info.AddPermission( JMConstants.PERM_WEATHER_QUICKACTION );
		info.AddPermission( JMConstants.PERM_WEATHER_QUICKACTION_CLEAR );
		info.AddPermission( JMConstants.PERM_WEATHER_QUICKACTION_CLOUDY );
		info.AddPermission( JMConstants.PERM_WEATHER_QUICKACTION_STORM );
		info.AddPermission( JMConstants.PERM_WEATHER_QUICKACTION_DATE );
		info.AddPermission( JMConstants.PERM_WEATHER_FREEZETIME );
		info.AddPermission( JMConstants.PERM_WEATHER_DATE );
		info.AddPermission( JMConstants.PERM_WEATHER_WIND );
		info.AddPermission( JMConstants.PERM_WEATHER_WIND_FUNCPARAMS );
		info.AddPermission( JMConstants.PERM_WEATHER_STORM );
		info.AddPermission( JMConstants.PERM_WEATHER_SANDSTORM );
		info.AddPermission( JMConstants.PERM_WEATHER_OVERCAST );
		info.AddPermission( JMConstants.PERM_WEATHER_FOG );
		info.AddPermission( JMConstants.PERM_WEATHER_FOG_DYNAMIC );
		info.AddPermission( JMConstants.PERM_WEATHER_RAIN );
		info.AddPermission( JMConstants.PERM_WEATHER_RAIN_THRESHOLDS );
		info.AddPermission( JMConstants.PERM_WEATHER_SNOW );
		info.AddPermission( JMConstants.PERM_WEATHER_SNOW_THRESHOLDS );
		info.AddPermission( JMConstants.PERM_WEATHER_PRESET );
		info.AddPermission( JMConstants.PERM_WEATHER_PRESET_USE );
		info.AddPermission( JMConstants.PERM_WEATHER_PRESET_CREATE );
		info.AddPermission( JMConstants.PERM_WEATHER_PRESET_UPDATE );
		info.AddPermission( JMConstants.PERM_WEATHER_PRESET_REMOVE );
		info.AddPermission( JMConstants.PERM_WEATHER_PRESET_DYNAMIC );

		info.AddWebhookType( "SetWeather" );
		info.AddWebhookType( "SetTime" );
		info.AddWebhookType( "UsePreset" );
		info.AddWebhookType( "CreatePreset" );
		info.AddWebhookType( "UpdatePreset" );
		info.AddWebhookType( "RemovePreset" );
		info.AddWebhookType( "DynamicWeather" );
	}

	override void EnableUpdate()
	{
	}

	override void RegisterKeyMouseBindings()
	{
		super.RegisterKeyMouseBindings();

		Bind( new JMModuleBinding( "Input_FreezeTime",  "UAWeatherModuleFreezeTime",  true ) );
		Bind( new JMModuleBinding( "Input_CyclePreset", "UAWeatherModuleCyclePreset", true ) );
	}

	void Input_FreezeTime()
	{
		if ( !JMPermissions.Has( JMConstants.PERM_WEATHER_FREEZETIME ) )
			return;
		SetFreezeTime( !IsTimeFrozen() );
	}

	void Input_CyclePreset()
	{
		if ( !JMPermissions.Has( JMConstants.PERM_WEATHER_PRESET_USE ) )
			return;
		if ( !settings || !settings.Presets || settings.Presets.Count() == 0 )
			return;
		UsePreset( settings.Presets[0].Name );
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if (g_Game.IsServer())
		{
			if (!g_Game.IsMultiplayer() && !g_Game.IsMissionMainMenu())
				SetFreezeTime(true);  //! Freeze weather by default in SP/offline

			Load();
		}
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

	protected void Exec_Load( PlayerIdentity ident )
	{
		if ( !JMPermissions.Has( JMConstants.PERM_WEATHER_PRESET, ident ) )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( settings );
		rpc.Send( NULL, JMWeatherModuleRPC.Load, true, ident );
	}

	protected void RPC_Load( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( g_Game.IsDedicatedServer() )
		{
			if ( !senderRPC )
				return;
			Exec_Load( senderRPC );
		}
		else
		{
			if ( ctx.Read( settings ) )
			{
				OnSettingsUpdated();
			}
		}
	}

	void RequestSpecialWeatherStatus()
	{
		if ( !g_Game.IsClient() )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMWeatherModuleRPC.SpecialWeatherStatus, true, NULL );
	}

	protected void Exec_SpecialWeatherStatus( PlayerIdentity ident )
	{
		if ( !ident )
			return;

		if ( !JMPermissions.HasRPC( JMConstants.PERM_WEATHER_VIEW, ident ) )
			return;

	#ifndef DAYZ_1_29
		//! 1.30+
		bool sandstormActive = g_Game.GetWeather().GetSandstorm().IsActive();

		bool evrActive;
		JMNamalskEventManagerModule namalskModule;
		if ( CF_Modules<JMNamalskEventManagerModule>.Get( namalskModule ) )
			evrActive = namalskModule.IsEventActive( "EVRStorm" ) || namalskModule.IsEventActive( "EVRStormDeadly" );

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( sandstormActive );
		rpc.Write( evrActive );
		rpc.Send( NULL, JMWeatherModuleRPC.SpecialWeatherStatus, true, ident );
	#endif
	}

	protected void RPC_SpecialWeatherStatus( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( g_Game.IsDedicatedServer() )
		{
			Exec_SpecialWeatherStatus( senderRPC );
		}
		else
		{
			ctx.Read( m_LastSandstormActive );
			ctx.Read( m_LastEVRStormActive );
		}
	}

	void UsePreset( string name )
	{
		if ( g_Game.IsServer() )
			Exec_UsePreset( name, NULL );
		else
			Send_PresetName( JMWeatherModuleRPC.UsePreset, name );
	}

	void CreatePreset( JMWeatherPreset preset )
	{
		if ( g_Game.IsServer() )
			Exec_CreatePreset( preset, NULL );
		else
			Send_Preset( JMWeatherModuleRPC.CreatePreset, preset );
	}

	void UpdatePreset( JMWeatherPreset preset )
	{
		if ( g_Game.IsServer() )
			Exec_UpdatePreset( preset, NULL );
		else
			Send_Preset( JMWeatherModuleRPC.UpdatePreset, preset );
	}

	void RemovePreset( string name )
	{
		if ( g_Game.IsServer() )
			Exec_RemovePreset( name, NULL );
		else
			Send_PresetName( JMWeatherModuleRPC.RemovePreset, name );
	}

	// -------------------------------------------------------------------------
	//  Payloads
	//
	//  Every setter above ends in Submit(). A JMWeatherBase names its own RPC,
	//  permission and webhook line (see JMWeatherPreset.c), so adding a payload is
	//  the class itself plus one case in ReadPayload() - there is no Send / Exec /
	//  RPC trio per phenomenon to write.
	// -------------------------------------------------------------------------

	protected void SubmitPhenomenon( JMWeatherPhenomenon payload, float forecast, float time, float minDuration )
	{
		payload.Forecast = forecast;
		payload.Time = time;
		payload.MinDuration = minDuration;

		Submit( payload );
	}

	//! Apply the payload here when this process owns the weather, otherwise ask the server to.
	protected void Submit( JMWeatherBase payload )
	{
		if ( g_Game.IsServer() )
		{
			//! A listen-server host calling this directly (server console, or the
			//! host's own admin UI) has no RPC round trip to piggyback the client
			//! push on - without this, already-connected clients only pick up a
			//! pushed payload (the date) once the engine's own slow native
			//! replication catches up, same as the bug RPC_Payload's push exists
			//! to avoid.
			if ( payload.IsPushedToClients() && g_Game.IsMultiplayer() )
				Send_Payload( payload );

			Exec_Payload( payload, NULL );
		}
		else
		{
			Send_Payload( payload );
		}
	}

	protected void Send_Payload( JMWeatherBase payload )
	{
		ScriptRPC rpc = new ScriptRPC();
		payload.WriteTo( rpc );
		rpc.Send( NULL, payload.GetRPC(), true, NULL );
	}

	//! The payload an incoming RPC carries, or NULL when the id is not one of
	//! ours or the message is short. A mod adding a payload adds its case here.
	protected JMWeatherBase ReadPayload( int rpc_type, ParamsReadContext ctx )
	{
		JMWeatherStorm storm;
		JMWeatherSandstorm sandstorm;
		JMWeatherFog fog;
		JMWeatherDynamicFog dynamicFog;
		JMWeatherRain rain;
		JMWeatherRainThreshold rainThreshold;
		JMWeatherSnow snow;
		JMWeatherSnowThreshold snowThreshold;
		JMWeatherOvercast overcast;
		JMWeatherWindMagnitude windMagnitude;
		JMWeatherWindDirection windDirection;
		JMWeatherWindFunction windFunction;
		JMWeatherDate date;

		switch ( rpc_type )
		{
		case JMWeatherModuleRPC.Storm:
			if ( ctx.Read( storm ) )
				return storm;
			break;
		case JMWeatherModuleRPC.Sandstorm:
			if ( ctx.Read( sandstorm ) )
				return sandstorm;
			break;
		case JMWeatherModuleRPC.Fog:
			if ( ctx.Read( fog ) )
				return fog;
			break;
		case JMWeatherModuleRPC.DynamicFog:
			if ( ctx.Read( dynamicFog ) )
				return dynamicFog;
			break;
		case JMWeatherModuleRPC.Rain:
			if ( ctx.Read( rain ) )
				return rain;
			break;
		case JMWeatherModuleRPC.RainThresholds:
			if ( ctx.Read( rainThreshold ) )
				return rainThreshold;
			break;
		case JMWeatherModuleRPC.Snow:
			if ( ctx.Read( snow ) )
				return snow;
			break;
		case JMWeatherModuleRPC.SnowThresholds:
			if ( ctx.Read( snowThreshold ) )
				return snowThreshold;
			break;
		case JMWeatherModuleRPC.Overcast:
			if ( ctx.Read( overcast ) )
				return overcast;
			break;
		case JMWeatherModuleRPC.WindMagnitude:
			if ( ctx.Read( windMagnitude ) )
				return windMagnitude;
			break;
		case JMWeatherModuleRPC.WindDirection:
			if ( ctx.Read( windDirection ) )
				return windDirection;
			break;
		case JMWeatherModuleRPC.WindFunctionParams:
			if ( ctx.Read( windFunction ) )
				return windFunction;
			break;
		case JMWeatherModuleRPC.Date:
			if ( ctx.Read( date ) )
				return date;
			break;
		}

		return NULL;
	}

	protected void RPC_Payload( int rpc_type, ParamsReadContext ctx, PlayerIdentity senderRPC )
	{
		JMWeatherBase payload = ReadPayload( rpc_type, ctx );
		if ( !payload )
			return;

		//! A NULL senderRPC here isn't a missing-identity request - it's the server's own
		//! instant-sync push (see below) arriving on each client, which the engine delivers
		//! with no player attached. Apply it directly instead of falling into the permission
		//! gate below, which is for client -> server requests only. Without this branch the
		//! push was a silent no-op and clients only ever saw the new date once the engine's
		//! own slow native calendar replication eventually caught up.
		if ( !senderRPC )
		{
			if ( payload.IsPushedToClients() && !g_Game.IsServer() )
				Exec_Payload( payload, NULL );

			return;
		}

		if ( !g_Game.IsServer() )
			return;

		if ( !CanServe( senderRPC, payload.GetPermission() ) )
			return;

		//! Send the payload to the clients so it changes instantly, else the game may take
		//! a few secs to sync it. Not needed for weather!
		if ( payload.IsPushedToClients() && g_Game.IsDedicatedServer() )
			Send_Payload( payload );

		Exec_Payload( payload, senderRPC );
	}

	protected void Exec_Payload( JMWeatherBase payload, PlayerIdentity ident )
	{
		payload.Apply();
		payload.Log( ident );

		//! Time is frozen: Repeat_FreezeTime re-stamps the clock from
		//! m_CachedWeatherPreset every second. Without updating the cache
		//! here, that repeat overwrites this manual change back to the old
		//! date within ~1s of it being applied.
		JMWeatherDate date;
		if ( m_bFreezeTime && m_CachedWeatherPreset && Class.CastTo( date, payload ) )
		{
			m_CachedWeatherPreset.PDate.Year   = date.Year;
			m_CachedWeatherPreset.PDate.Month  = date.Month;
			m_CachedWeatherPreset.PDate.Day    = date.Day;
			m_CachedWeatherPreset.PDate.Hour   = date.Hour;
			m_CachedWeatherPreset.PDate.Minute = date.Minute;
		}

		SendWebhookColored( payload.GetWebhookType(), GetCaller( ident ), payload.Describe(), JMConstants.WEBHOOK_COLOR_INFO );
	}

	//! A request that arrived from a real sender who holds the permission.
	protected bool CanServe( PlayerIdentity sender, string permission )
	{
		if ( !sender )
			return false;

		return JMPermissions.HasRPC( permission, sender );
	}

	protected void Send_DynamicWeather( JMWeatherDynamicConfig config )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( config );
		rpc.Send( NULL, JMWeatherModuleRPC.DynamicWeather, true, NULL );
	}

	protected void Send_FreezeTime( bool state )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( state );
		rpc.Send( NULL, JMWeatherModuleRPC.FreezeTime, true, NULL );
	}

	protected void Send_Preset( int rpc_type, JMWeatherPreset preset )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( preset );
		rpc.Send( NULL, rpc_type, true, NULL );
	}

	protected void Send_PresetName( int rpc_type, string name )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( name );
		rpc.Send( NULL, rpc_type, true, NULL );
	}

	protected void Exec_FreezeTime( bool state, PlayerIdentity ident )
	{
		Weather weather = g_Game.GetWeather();

		if (state)
		{
			//! Step 1: Stop current phenomenon changes in progress (if any)
			m_CachedWeatherPreset.SetFromWorld();
			m_CachedWeatherPreset.StopCurrentChangesInProgress();

			//! Step 2: Need to set mission weather to false so weather update freeze actually does something
			weather.MissionWeather(false);

			g_Game.GetWorld().SetTimeMultiplier(0);
		}
		else
		{
			m_CachedWeatherPreset.ResumeCurrentChangesInProgress();

			g_Game.GetWorld().SetTimeMultiplier(-1);
		}

		weather.SetWeatherUpdateFreeze(state);
	}

	protected void Exec_UsePreset( string name, PlayerIdentity ident )
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
		JMPlayerInstance upInst = GetCaller( ident );

		SendWebhookColored( "UsePreset", upInst, "Applied weather preset: " + name, JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	protected void Exec_CreatePreset( JMWeatherPreset preset, PlayerIdentity ident )
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
		JMPlayerInstance cpInst = GetCaller( ident );
		SendWebhookColored( "CreatePreset", cpInst, "Created weather preset: " + preset.Name, JMConstants.WEBHOOK_COLOR_SUCCESS );

		settings.Save();
	}

	protected void Exec_UpdatePreset( JMWeatherPreset preset, PlayerIdentity ident )
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
		JMPlayerInstance updInst = GetCaller( ident );

		SendWebhookColored( "UpdatePreset", updInst, "Updated weather preset: " + preset.Name, JMConstants.WEBHOOK_COLOR_SUCCESS );

		settings.Save();
	}

	protected void Exec_RemovePreset( string name, PlayerIdentity ident )
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
		JMPlayerInstance rmInst = GetCaller( ident );
		SendWebhookColored( "RemovePreset", rmInst, "Removed weather preset: " + name, JMConstants.WEBHOOK_COLOR_WARNING );

		settings.Save();
	}

	protected void RPC_DynamicWeather( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherDynamicConfig config = new JMWeatherDynamicConfig;

		if ( !ctx.Read( config ) )
			return;

		if ( !g_Game.IsServer() )
			return;

		if ( !senderRPC )
			return;

		if ( !JMPermissions.HasRPC( JMConstants.PERM_WEATHER_PRESET_DYNAMIC, senderRPC ) )
			return;

		Exec_DynamicWeather( config, senderRPC );
	}

	protected void Exec_DynamicWeather( JMWeatherDynamicConfig config, PlayerIdentity ident )
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
		JMPlayerInstance dwInst = GetCaller( ident );

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

	protected void RPC_FreezeTime( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool state;
		if ( !ctx.Read( state ) )
			return;

		if (!g_Game.IsServer())
			return;

		if ( !CanServe( senderRPC, JMConstants.PERM_WEATHER_FREEZETIME ) )
			return;

		Exec_FreezeTime( state, senderRPC );
	}

	protected void RPC_UsePreset( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string name;
		if ( !ctx.Read( name ) )
			return;

		if (!g_Game.IsServer())
			return;

		if ( !CanServe( senderRPC, JMConstants.PERM_WEATHER_PRESET_USE ) )
			return;

		Exec_UsePreset( name, senderRPC );
	}

	protected void RPC_CreatePreset( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_CreatePreset").Add(senderRPC);
		#endif

		JMWeatherPreset preset;
		if ( !ctx.Read( preset ) )
			return;

		if ( !CanServe( senderRPC, JMConstants.PERM_WEATHER_PRESET_CREATE ) )
			return;

		if ( g_Game.IsDedicatedServer() )
			Send_Preset( JMWeatherModuleRPC.CreatePreset, preset );

		Exec_CreatePreset( preset, senderRPC );
	}

	protected void RPC_UpdatePreset( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherPreset preset;
		if ( !ctx.Read( preset ) )
			return;

		if ( !CanServe( senderRPC, JMConstants.PERM_WEATHER_PRESET_UPDATE ) )
			return;

		if ( g_Game.IsDedicatedServer() )
			Send_Preset( JMWeatherModuleRPC.UpdatePreset, preset );

		Exec_UpdatePreset( preset, senderRPC );
	}

	protected void RPC_RemovePreset( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string name;
		if ( !ctx.Read( name ) )
			return;

		if ( !CanServe( senderRPC, JMConstants.PERM_WEATHER_PRESET_REMOVE ) )
			return;

		if ( g_Game.IsDedicatedServer() )
			Send_PresetName( JMWeatherModuleRPC.RemovePreset, name );

		Exec_RemovePreset( name, senderRPC );
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
		case JMWeatherModuleRPC.SpecialWeatherStatus:
			RPC_SpecialWeatherStatus( ctx, sender, target );
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
		default:
			//! Everything else is a JMWeatherBase payload - see ReadPayload().
			RPC_Payload( rpc_type, ctx, sender );
			break;
		}
	}
}
