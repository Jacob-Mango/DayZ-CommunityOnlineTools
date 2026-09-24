enum JMWeatherTypes
{
	INVALID,
	Phenomenon,
	Mission,
	Preset,
	COUNT
}

enum JMWeatherBehavior
{
	UseWorldData,
	UseForecastOnly,
	Static
}

class JMWeatherModule: JMRenderableModuleBase
{
	protected ref JMWeatherSerialize settings;
	protected ref JMWeatherPreset m_CachedWeatherPreset = new JMWeatherPreset;
	protected bool m_bFreezeTime;
	protected bool m_MissionWeather;

	//! The dynamic weather state machine. Server only - clients never run one;
	//! they read its position through m_Status.
	protected ref JMWeatherMachine m_Machine;

	//! Last known position of the machine. On the server it is refreshed from the
	//! machine on demand, on a client it is whatever the last status RPC said.
	protected ref JMWeatherDynamicStatus m_Status;

	//! Bumped by the server whenever the state machine's configuration changes,
	//! and carried on every status and settings reply. A client that sees a
	//! revision other than the one it last loaded asks for the settings again.
	protected int m_Revision;
	protected int m_LoadedRevision;

	//! Where Input_CyclePreset has got to in the state list.
	protected int m_CycleIndex = -1;

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

	//! The stored state by name, or NULL. A state is what used to be a preset: it
	//! owns its phases, and each phase owns the weather it applies.
	JMWeatherState GetState( string name )
	{
		if ( !settings )
			return NULL;

		return settings.GetState( name );
	}

	array< ref JMWeatherState > GetStates()
	{
		return settings.States;
	}

	bool HasState( string name )
	{
		return GetState( name ) != NULL;
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

	void SetDynamicFog( float distance, float height = 0, float bias = 0, float time = 0, float distanceHi = 0, float heightHi = 0, float biasHi = 0 )
	{
		JMWeatherDynamicFog payload = new JMWeatherDynamicFog;
		payload.Distance = distance;
		payload.Height = height;
		payload.Bias = bias;
		payload.Time = time;
		payload.DistanceHi = distanceHi;
		payload.HeightHi = heightHi;
		payload.BiasHi = biasHi;

		Submit( payload );
	}

	void SetDynamicWeather( JMWeatherDynamicConfig config )
	{
		if ( g_Game.IsServer() )
			Exec_DynamicWeather( config, NULL );
		else
			Send_DynamicWeather( config );
	}

	//! Switch between MissionWeather false (i.e. use WorldData weather state machine),
	//! MissionWeather true (i.e. forecast only) and static weather (continue current
	//! in-progress weather changes if any but stop computing changes in WorldData state machine)
	//! Note that static weather is not the same as freezing time since time still progresses
	void SetWeatherBehavior(JMWeatherBehavior mode)
	{
		if ( g_Game.IsServer() )
			Exec_SetWeatherBehavior( mode, NULL );
		else
			Send_SetWeatherBehavior( mode );
	}

	void SetFog( float forecast, float time = 0, float minDuration = 0, float forecastHi = 0 )
	{
		SubmitPhenomenon( new JMWeatherFog, forecast, time, minDuration, forecastHi );
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

	void SetOvercast( float forecast, float time = 0, float minDuration = 0, float forecastHi = 0 )
	{
		SubmitPhenomenon( new JMWeatherOvercast, forecast, time, minDuration, forecastHi );
	}

	void SetRain( float forecast, float time = 0, float minDuration = 0, float forecastHi = 0 )
	{
		SubmitPhenomenon( new JMWeatherRain, forecast, time, minDuration, forecastHi );
	}

	void SetRainThresholds( float tMin, float tMax, float tTime )
	{
		JMWeatherRainThreshold payload = new JMWeatherRainThreshold;
		payload.OvercastMin = tMin;
		payload.OvercastMax = tMax;
		payload.Time = tTime;

		Submit( payload );
	}

	void SetSandstorm( bool enabled, float duration, float fadeInTime = -1, float overcast = -1, float windMagnitude = -1, float durationHi = 0, float fadeInTimeHi = 0, float overcastHi = 0, float windMagnitudeHi = 0 )
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
		payload.DurationHi = durationHi;
		payload.FadeInTimeHi = fadeInTimeHi;
		payload.OvercastValueHi = overcastHi;
		payload.WindMagnitudeValueHi = windMagnitudeHi;

		Submit( payload );
	}

	void SetSnow( float forecast, float time = 0, float minDuration = 0, float forecastHi = 0 )
	{
		SubmitPhenomenon( new JMWeatherSnow, forecast, time, minDuration, forecastHi );
	}

	void SetSnowThresholds( float tMin, float tMax, float tTime )
	{
		JMWeatherSnowThreshold payload = new JMWeatherSnowThreshold;
		payload.OvercastMin = tMin;
		payload.OvercastMax = tMax;
		payload.Time = tTime;

		Submit( payload );
	}

	void SetStorm( float density, float threshold, float minTimeBetweenLightning, float densityHi = 0, float thresholdHi = 0, float minTimeBetweenLightningHi = 0 )
	{
		JMWeatherStorm payload = new JMWeatherStorm;
		payload.Density = density;
		payload.Threshold = threshold;
		payload.MinTimeBetweenLightning = minTimeBetweenLightning;
		payload.DensityHi = densityHi;
		payload.ThresholdHi = thresholdHi;
		payload.MinTimeBetweenLightningHi = minTimeBetweenLightningHi;

		Submit( payload );
	}

	void SetWindDirection( float forecast, float time = 0, float minDuration = 0, float forecastHi = 0 )
	{
		SubmitPhenomenon( new JMWeatherWindDirection, forecast, time, minDuration, forecastHi );
	}

	void SetWindFunctionParams( float fnMin, float fnMax, float fnSpeed, float fnMinHi = 0, float fnMaxHi = 0, float fnSpeedHi = 0 )
	{
		JMWeatherWindFunction payload = new JMWeatherWindFunction;
		payload.Min = fnMin;
		payload.Max = fnMax;
		payload.Speed = fnSpeed;
		payload.MinHi = fnMinHi;
		payload.MaxHi = fnMaxHi;
		payload.SpeedHi = fnSpeedHi;

		Submit( payload );
	}

	void SetWindMagnitude( float forecast, float time = 0, float minDuration = 0, float forecastHi = 0 )
	{
		SubmitPhenomenon( new JMWeatherWindMagnitude, forecast, time, minDuration, forecastHi );
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
		info.AddPermission( JMConstants.PERM_WEATHER_BEHAVIOR );
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

	//! A clean shutdown: record how much of the current phase is left, so the
	//! restart that follows resumes it instead of starting the phase over.
	override void OnMissionFinish()
	{
		super.OnMissionFinish();

		if ( m_Machine )
			m_Machine.Snapshot();
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
		if ( !settings || !settings.States || settings.States.Count() == 0 )
			return;

		m_CycleIndex = ( m_CycleIndex + 1 ) % settings.States.Count();

		UsePreset( settings.States[m_CycleIndex].Name );
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

			OnSettingsUpdated();
		}
	}

	protected void Exec_Load( PlayerIdentity ident )
	{
		if ( !JMPermissions.Has( JMConstants.PERM_WEATHER_PRESET, ident ) )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( settings );
		rpc.Write( m_Revision );
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
				ctx.Read( m_LoadedRevision );

				if ( !m_Status )
					m_Status = new JMWeatherDynamicStatus;

				m_Status.Enabled = settings.DynamicEnabled;

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

	//! Put a state's weather on NOW, as the person asked - with the weather's own
	//! stored fade and hold, not the phase timing the machine would use. An empty
	//! phase means the state's own entry roll. While the machine is running it
	//! carries on from the phase that was applied.
	void UsePreset( string name, string phase = "" )
	{
		JMWeatherDynamicControl control = new JMWeatherDynamicControl;

		control.Op    = JMWeatherDynamicControl.OP_APPLY;
		control.State = name;
		control.Phase = phase;

		SendDynamicControl( control );
	}

	// -------------------------------------------------------------------------
	//  Payloads
	//
	//  Every setter above ends in Submit(). A JMWeatherBase names its own RPC,
	//  permission and webhook line (see JMWeatherPreset.c), so adding a payload is
	//  the class itself plus one case in ReadPayload() - there is no Send / Exec /
	//  RPC trio per phenomenon to write.
	// -------------------------------------------------------------------------

	protected void SubmitPhenomenon( JMWeatherPhenomenon payload, float forecast, float time, float minDuration, float forecastHi = 0 )
	{
		payload.Forecast = forecast;
		payload.ForecastHi = forecastHi;
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
	protected JMWeatherBase NewPayload( int rpc_type )
	{
		switch ( rpc_type )
		{
		case JMWeatherModuleRPC.Storm:
			return new JMWeatherStorm;
		case JMWeatherModuleRPC.Sandstorm:
			return new JMWeatherSandstorm;
		case JMWeatherModuleRPC.Fog:
			return new JMWeatherFog;
		case JMWeatherModuleRPC.DynamicFog:
			return new JMWeatherDynamicFog;
		case JMWeatherModuleRPC.Rain:
			return new JMWeatherRain;
		case JMWeatherModuleRPC.RainThresholds:
			return new JMWeatherRainThreshold;
		case JMWeatherModuleRPC.Snow:
			return new JMWeatherSnow;
		case JMWeatherModuleRPC.SnowThresholds:
			return new JMWeatherSnowThreshold;
		case JMWeatherModuleRPC.Overcast:
			return new JMWeatherOvercast;
		case JMWeatherModuleRPC.WindMagnitude:
			return new JMWeatherWindMagnitude;
		case JMWeatherModuleRPC.WindDirection:
			return new JMWeatherWindDirection;
		case JMWeatherModuleRPC.WindFunctionParams:
			return new JMWeatherWindFunction;
		case JMWeatherModuleRPC.Date:
			return new JMWeatherDate;
		}

		return NULL;
	}

	protected JMWeatherBase ReadPayload( int rpc_type, ParamsReadContext ctx )
	{
		JMWeatherBase payload = NewPayload( rpc_type );

		if ( !payload )
			return NULL;

		//! The payload reads itself, field by field, in the order it wrote them.
		if ( !payload.ReadFrom( ctx ) )
			return NULL;

		return payload;
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
	
	protected void Send_SetWeatherBehavior( JMWeatherBehavior mode )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( mode );
		rpc.Send( NULL, JMWeatherModuleRPC.SetWeatherBehavior, true, NULL );
	}

	protected void Send_FreezeTime( bool state )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( state );
		rpc.Send( NULL, JMWeatherModuleRPC.FreezeTime, true, NULL );
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
			m_MissionWeather = weather.GetMissionWeather();

			if (m_MissionWeather)
				weather.MissionWeather(false);

			g_Game.GetWorld().SetTimeMultiplier(0);
		}
		else
		{
			m_CachedWeatherPreset.ResumeCurrentChangesInProgress();

			if (m_MissionWeather)
				weather.MissionWeather(true);

			g_Game.GetWorld().SetTimeMultiplier(-1);
		}

		weather.SetWeatherUpdateFreeze(state);

		//! Frozen weather means a frozen countdown: the phase in progress keeps
		//! the time it has left and carries on from there when time resumes.
		if ( m_Machine )
		{
			if ( state )
				m_Machine.Pause();
			else
				m_Machine.Unpause();
		}
	}

	//! Apply a state's phase weather now. The weather goes on exactly as authored -
	//! its own transition and hold - and the machine, if it is running, is moved to
	//! that phase so the next roll does not immediately overwrite it.
	protected void ApplyState( string stateName, string phaseName, PlayerIdentity ident )
	{
		JMWeatherState state = GetState( stateName );

		if ( !state )
			return;

		string target = phaseName;

		if ( target == "" )
			target = state.RollEntryPhase();

		JMWeatherPhase phase = state.GetPhase( target );

		if ( !phase )
			return;

		phase.Conditions.Apply();

		//! Named only for the log line - a phase's weather has no name of its own.
		string storedName = phase.Conditions.Name;
		phase.Conditions.Name = stateName + " > " + phase.Name;
		phase.Conditions.Log( ident );
		phase.Conditions.Name = storedName;

		if ( m_Machine )
			m_Machine.Adopt( stateName, target );

		//! ident is NULL whenever the server applies weather on its own - the
		//! startup state - and GetId() on a null identity is not survivable.
		JMPlayerInstance upInst = GetCaller( ident );

		SendWebhookColored( "UsePreset", upInst, "Applied weather preset: " + stateName + " > " + phase.Name, JMConstants.WEBHOOK_COLOR_SUCCESS );
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

		//! The switch is one permission, and what happens to a state is another: making
		//! one, changing one and deleting one are the old preset permissions.
		bool canSwitch = JMPermissions.HasRPC( JMConstants.PERM_WEATHER_PRESET_DYNAMIC, senderRPC );
		bool canEdit   = canSwitch;

		if ( config.Op == JMWeatherDynamicConfig.OP_SAVE && config.TargetName == "" )
			canEdit = JMPermissions.HasRPC( JMConstants.PERM_WEATHER_PRESET_CREATE, senderRPC );
		else if ( config.Op == JMWeatherDynamicConfig.OP_SAVE )
			canEdit = JMPermissions.HasRPC( JMConstants.PERM_WEATHER_PRESET_UPDATE, senderRPC );
		else if ( config.Op == JMWeatherDynamicConfig.OP_REMOVE )
			canEdit = JMPermissions.HasRPC( JMConstants.PERM_WEATHER_PRESET_REMOVE, senderRPC );

		if ( config.Op == JMWeatherDynamicConfig.OP_TOGGLE && !canSwitch )
			return;

		if ( config.Op != JMWeatherDynamicConfig.OP_TOGGLE && !canEdit )
			return;

		//! Saving a state carries the switch along, but only somebody who may flip it
		//! gets to - everyone else's message leaves it as it was.
		if ( config.Op != JMWeatherDynamicConfig.OP_TOGGLE && !canSwitch && settings )
			config.Enabled = settings.DynamicEnabled;

		Exec_DynamicWeather( config, senderRPC );
	}

	protected void Exec_DynamicWeather( JMWeatherDynamicConfig config, PlayerIdentity ident )
	{
		if ( !settings || !config )
			return;

		//! The master switch always applies, whatever else the message carries.
		settings.DynamicEnabled = config.Enabled;

		string dwMsg;

		if ( config.Enabled )
			dwMsg = "Enabled dynamic weather";
		else
			dwMsg = "Disabled dynamic weather";

		string renamedFrom;
		string renamedTo;

		switch ( config.Op )
		{
		case JMWeatherDynamicConfig.OP_SAVE:
		{
			//! The server keeps its own validated copy of the state, never the
			//! objects that arrived - see JMWeatherSerialize.NormalizeState.
			JMWeatherState saved = settings.SaveState( config.TargetName, config.State );

			if ( !saved )
			{
				//! Blank or duplicate name. Nothing about the states changed.
				dwMsg = "Refused dynamic weather state (blank or duplicate name)";
				break;
			}

			if ( config.TargetName != "" && config.TargetName != saved.Name )
			{
				renamedFrom = config.TargetName;
				renamedTo   = saved.Name;
			}

			dwMsg = "Saved dynamic weather state " + saved.Name + " (" + saved.Phases.Count() + " phases)";
			break;
		}
		case JMWeatherDynamicConfig.OP_REMOVE:
		{
			settings.RemoveState( config.TargetName );

			dwMsg = "Removed dynamic weather state " + config.TargetName;
			break;
		}
		}

		//! Every client that holds a copy of the configuration is now stale.
		m_Revision++;

		OnSettingsUpdated();

		if ( !g_Game.IsServer() )
			return;

		if ( m_Machine )
		{
			if ( renamedFrom != "" )
				m_Machine.OnStateRenamed( renamedFrom, renamedTo );

			//! Brings the machine in line without restarting a phase that is still
			//! valid, so an unrelated edit or the toggle alone does not reset the
			//! countdown - and switching on applies a phase at once.
			m_Machine.OnConfigChanged();
		}

		GetCommunityOnlineToolsBase().Log( ident, dwMsg );

		//! ident is NULL when the mission host edits this itself.
		JMPlayerInstance dwInst = GetCaller( ident );

		SendWebhookColored( "DynamicWeather", dwInst, dwMsg, JMConstants.WEBHOOK_COLOR_INFO );

		settings.Save();
	}
	
	protected void RPC_SetWeatherBehavior( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int mode;
		if ( !ctx.Read( mode ) )
			return;

		if (!g_Game.IsServer())
			return;

		if ( !JMPermissions.HasRPC( "Weather.Behavior", senderRPC ) )
			return;

		Exec_SetWeatherBehavior( mode, senderRPC );
	}

	protected void Exec_SetWeatherBehavior( JMWeatherBehavior mode, PlayerIdentity ident )
	{
		Weather weather = g_Game.GetWeather();

		switch (mode)
		{
			case JMWeatherBehavior.UseWorldData:
				weather.MissionWeather(false);
				weather.SetWeatherUpdateFreeze(false);
				break;

			case JMWeatherBehavior.UseForecastOnly:
				weather.MissionWeather(true);
				weather.SetWeatherUpdateFreeze(false);
				break;

			case JMWeatherBehavior.Static:
				weather.MissionWeather(false);  //! Need to set mission weather to false so weather update freeze actually does something
				weather.SetWeatherUpdateFreeze(true);
				break;
		}

		m_MissionWeather = weather.GetMissionWeather();
	}

	//! What the weather starts as when the settings load.
	//!
	//! With the dynamic machine on it carries on from the position it was saved
	//! at - same state, same phase, same time left on it - or begins from a
	//! random state when there is nothing valid to carry on from. With it off,
	//! InitialPreset keeps its original "apply this on startup" meaning for
	//! anyone still setting it in Weather.json.
	protected void SeedWeather()
	{
		if ( !settings )
			return;

		m_Machine = new JMWeatherMachine( this, settings, m_bFreezeTime );

		if ( !settings.DynamicEnabled )
		{
			if ( settings.InitialPreset != "" )
			{
				PlayerIdentity noSeedIdent;
				ApplyState( settings.InitialPreset, "", noSeedIdent );
			}

			return;
		}

		m_Machine.Boot();
	}

	//! Called by the machine each time it enters (or resumes) a phase.
	void OnMachineEntered( string stateName, string phaseName, bool resumed )
	{
		string verb = "moved to";

		if ( resumed )
			verb = "resumed at";

		//! Typed nulls, not a bare NULL: Log is overloaded on both
		//! PlayerIdentity and JMPlayerInstance, and a bare NULL matches
		//! neither on its own.
		PlayerIdentity noIdent;
		JMPlayerInstance noInstance;

		GetCommunityOnlineToolsBase().Log( noIdent, "Dynamic weather " + verb + " " + stateName + " > " + phaseName );
		SendWebhookColored( "DynamicWeather", noInstance, "Weather " + verb + ": " + stateName + " > " + phaseName, JMConstants.WEBHOOK_COLOR_INFO );
	}

	//! The machine's own position, for the status RPC and for the host's UI.
	void RefreshDynamicStatus()
	{
		if ( !m_Status )
			m_Status = new JMWeatherDynamicStatus;

		if ( g_Game.IsServer() && m_Machine )
			m_Machine.FillStatus( m_Status );

		m_Status.Revision   = m_Revision;
		m_Status.ReceivedAt = g_Game.GetTickTime();
	}

	JMWeatherDynamicStatus GetDynamicStatus()
	{
		if ( !m_Status )
			m_Status = new JMWeatherDynamicStatus;

		return m_Status;
	}

	JMWeatherSerialize GetSettings()
	{
		return settings;
	}

	//! Ask for the machine's position. The reply lands in GetDynamicStatus().
	void RequestDynamicStatus()
	{
		if ( g_Game.IsServer() )
		{
			RefreshDynamicStatus();
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMWeatherModuleRPC.DynamicStatus, true, NULL );
	}

	protected void Exec_DynamicStatus( PlayerIdentity ident )
	{
		if ( !ident || !m_Machine )
			return;

		if ( !JMPermissions.HasRPC( JMConstants.PERM_WEATHER_VIEW, ident ) )
			return;

		JMWeatherDynamicStatus status = new JMWeatherDynamicStatus;

		m_Machine.FillStatus( status );
		status.Revision = m_Revision;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( status );
		rpc.Send( NULL, JMWeatherModuleRPC.DynamicStatus, true, ident );
	}

	protected void RPC_DynamicStatus( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( g_Game.IsDedicatedServer() )
		{
			Exec_DynamicStatus( senderRPC );
			return;
		}

		JMWeatherDynamicStatus status;

		if ( !ctx.Read( status ) )
			return;

		status.ReceivedAt = g_Game.GetTickTime();
		m_Status = status;

		//! The configuration changed since this client last loaded it - another
		//! admin's edit, or this client's own. Ask for it again.
		if ( status.Revision != m_LoadedRevision )
		{
			m_LoadedRevision = status.Revision;
			Load();
		}
	}

	//! Move the machine: advance, restart, or jump to a state and phase.
	void SendDynamicControl( JMWeatherDynamicControl control )
	{
		if ( g_Game.IsServer() )
		{
			PlayerIdentity noIdent;
			Exec_DynamicControl( control, noIdent );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( control );
		rpc.Send( NULL, JMWeatherModuleRPC.DynamicControl, true, NULL );
	}

	protected void RPC_DynamicControl( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMWeatherDynamicControl control = new JMWeatherDynamicControl;

		if ( !ctx.Read( control ) )
			return;

		if ( !g_Game.IsServer() )
			return;

		if ( !senderRPC )
			return;

		//! Putting a state's weather on is the old "use preset"; moving the machine is
		//! the dynamic permission.
		string controlPermission = JMConstants.PERM_WEATHER_PRESET_DYNAMIC;

		if ( control.Op == JMWeatherDynamicControl.OP_APPLY )
			controlPermission = JMConstants.PERM_WEATHER_PRESET_USE;

		if ( !JMPermissions.HasRPC( controlPermission, senderRPC ) )
			return;

		Exec_DynamicControl( control, senderRPC );
	}

	protected void Exec_DynamicControl( JMWeatherDynamicControl control, PlayerIdentity ident )
	{
		if ( !control || !settings )
			return;

		//! Applying weather by hand works whether or not the machine is running.
		if ( control.Op == JMWeatherDynamicControl.OP_APPLY )
		{
			ApplyState( control.State, control.Phase, ident );
			return;
		}

		if ( !m_Machine || !settings.DynamicEnabled )
			return;

		string message;

		switch ( control.Op )
		{
		case JMWeatherDynamicControl.OP_ADVANCE:
			message = "Advanced dynamic weather";
			m_Machine.Advance();
			break;
		case JMWeatherDynamicControl.OP_RESTART:
			message = "Restarted dynamic weather";
			m_Machine.BeginRandom();
			break;
		case JMWeatherDynamicControl.OP_HOLD:
			message = "Paused dynamic weather";
			m_Machine.SetHeld( true );
			break;
		case JMWeatherDynamicControl.OP_RELEASE:
			message = "Resumed dynamic weather";
			m_Machine.SetHeld( false );
			break;
		case JMWeatherDynamicControl.OP_JUMP:
			message = "Jumped dynamic weather to " + control.State + " > " + control.Phase;

			if ( !m_Machine.JumpTo( control.State, control.Phase ) )
				return;

			break;
		default:
			return;
		}

		GetCommunityOnlineToolsBase().Log( ident, message );
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
		case JMWeatherModuleRPC.DynamicWeather:
			RPC_DynamicWeather( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.DynamicControl:
			RPC_DynamicControl( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.DynamicStatus:
			RPC_DynamicStatus( ctx, sender, target );
			break;
		case JMWeatherModuleRPC.SetWeatherBehavior:
			RPC_SetWeatherBehavior( ctx, sender, target );
			break;
		default:
			//! Everything else is a JMWeatherBase payload - see ReadPayload().
			RPC_Payload( rpc_type, ctx, sender );
			break;
		}
	}
}
