class JMWeatherBase
{
	//! Node the sender must hold for the server to accept it.
	string GetPermission()
	{
		return "";
	}

	//! What a payload needs to travel: the RPC that carries it, the node its sender
	//! must hold, and how it reads in the webhook. JMWeatherModule.Submit() does the
	//! rest, so a new payload is this class plus one case in ReadPayload().

	//! The RPC that carries this payload from an admin's client to the server.
	int GetRPC()
	{
		return JMWeatherModuleRPC.INVALID;
	}

	//! Webhook event type. Must be one of JMWeatherModule.GetWebhookTypes().
	string GetWebhookType()
	{
		return "SetWeather";
	}

	//! Whether the server, having accepted this from an admin, also pushes it to every
	//! client. Only the date needs it: the engine's own calendar replication takes a few
	//! seconds to reach clients, while weather phenomena replicate on their own.
	bool IsPushedToClients()
	{
		return false;
	}

	void SetFromWorld()
	{
	}

	//! The line the webhook carries, e.g. "Set fog=0.5".
	string Describe()
	{
		return "";
	}

	//! Serialize as the concrete type. The engine writes a class by the type it is
	//! handed, so the base class cannot do this once for everybody.
	void WriteTo( ParamsWriteContext ctx )
	{
	}

	void Apply()
	{
	}

	void StopCurrentChangeInProgress()
	{
	}

	void ResumeCurrentChangeInProgress()
	{
	}

	void Log( PlayerIdentity pidentLog )
	{
	}
}

class JMWeatherStorm: JMWeatherBase
{
	float Density;
	float Threshold;
	float MinTimeBetweenLightning;

	override int GetRPC() { return JMWeatherModuleRPC.Storm; }

	override string GetPermission() { return JMConstants.PERM_WEATHER_STORM; }

	override string Describe() { return "Set storm density=" + Density + " threshold=" + Threshold; }

	override void WriteTo( ParamsWriteContext ctx ) { ctx.Write( this ); }

	override void Apply()
	{
		if (Density != -1)
		{
			float clampedDensity = Math.Clamp( Density, 0.0, 1.0 );
			float clampedThreshold = Math.Clamp( Threshold, 0.0, 1.0 );
			float clampedMinTime = Math.Max( MinTimeBetweenLightning, 0.0 );
			g_Game.GetWeather().SetStorm( clampedDensity, clampedThreshold, clampedMinTime );
		}
	}

	//! Weather exposes SetStorm with no matching getter, so the storm state
	//! genuinely cannot be read back. -1 is this class's "not set" marker and
	//! makes Apply skip it, which is the only honest snapshot available.
	//!
	//! This used to hardcode 1.0 / 0.7 / 25.0 and present them as the world's
	//! values. Anything that snapshots and re-applies - Freeze Time above all,
	//! which re-applies once a second - was therefore CREATING a full-density
	//! thunderstorm on a server that had none, and the form showed those
	//! invented figures as if they were live.
	override void SetFromWorld()
	{
		Density = -1;
		Threshold = -1;
		MinTimeBetweenLightning = -1;
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "Storm " + Density + ", " + Threshold + ", " + MinTimeBetweenLightning );
		}
	}
}

class JMWeatherSandstorm: JMWeatherBase
{
	//! Tri-state: -1 not set (Apply is a no-op), 0 off, 1 on. A sandstorm has
	//! no intensity dial to speak of - Start/Stop is the whole control surface -
	//! so a plain bool would leave "not set" indistinguishable from "off".
	int Enabled;

	//! Seconds the sandstorm takes to fade in on Start, or out on Stop.
	float Duration;

	//! Wind/overcast/rain knobs the vanilla DEV_SET_SANDSTORM RPC forces
	//! alongside Start - -1 means "not set", Apply leaves that knob alone.
	float FadeInTime = -1;
	float OvercastValue = -1;
	float WindMagnitudeValue = -1;

	override int GetRPC() { return JMWeatherModuleRPC.Sandstorm; }

	override string GetPermission() { return JMConstants.PERM_WEATHER_SANDSTORM; }

	override string Describe() { return "Set sandstorm enabled=" + Enabled + " duration=" + Duration; }

	override void WriteTo( ParamsWriteContext ctx ) { ctx.Write( this ); }

	override void Apply()
	{
	#ifndef DAYZ_1_29
		//! 1.30+

		if (Enabled == -1)
			return;

		if (!g_Game.IsServer())
			return;

		float clampedDuration = Math.Max( Duration, 0.0 );
		Weather weather = g_Game.GetWeather();
		SandstormController sandstorm = weather.GetSandstorm();

		if (Enabled == 1)
		{
			float fadeInTime = Math.Max( FadeInTime, 0.0 );

			vector stormDir = sandstorm.GetDirection();
			float stormAngle = weather.WindDirectionToAngle( stormDir );
			weather.GetWindDirection().Set( stormAngle, fadeInTime, clampedDuration );

			if (OvercastValue != -1)
			{
				float overcast = Math.Max( OvercastValue, 0.8 );
				weather.GetOvercast().Set( overcast, fadeInTime, clampedDuration );
			}

			if (WindMagnitudeValue != -1)
			{
				float windMagnitude = Math.Max( WindMagnitudeValue, 18.0 );
				weather.GetWindMagnitude().Set( windMagnitude, fadeInTime, clampedDuration );
			}

			weather.GetRain().Set( 0, fadeInTime, clampedDuration );

			if (!sandstorm.IsActive())
				sandstorm.Start( fadeInTime, true );
		}
		else
		{
			if (sandstorm.IsActive())
				sandstorm.Stop( clampedDuration, true );
		}
	#endif
	}

	//! Start/Stop have no forecast to read back, only the live on/off state -
	//! Duration/FadeInTime/Overcast/WindMagnitude are what the NEXT call will
	//! use, not anything the world remembers, so they are left at the values
	//! the preset was authored with.
	override void SetFromWorld()
	{
	#ifndef DAYZ_1_29
		//! 1.30+

		//! Unlike Fog/Rain/etc (JMWeatherPhenomenon, reading vanilla
		//! WeatherPhenomenon forecasts the engine replicates to clients),
		//! SandstormController's active flag is server-only state with no
		//! client replication. A client calling IsActive() always reads
		//! false/stale, so this used to snap the checkbox back off on the
		//! very next refresh after an admin turned it on - the refresh raced
		//! JMWeatherForm's own m_DirtySandstorm guard, which ApplySandstorm()
		//! clears right after sending the RPC, before there is any ack to
		//! clear it on. Matches Apply()'s own client no-op above: a client
		//! leaves Enabled at its last known value instead of trusting a read
		//! it cannot make accurately.
		if (!g_Game.IsServer())
			return;

		if (g_Game.GetWeather().GetSandstorm().IsActive())
			Enabled = 1;
		else
			Enabled = 0;
	#endif
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "Sandstorm " + Enabled + ", " + Duration + ", fadeIn=" + FadeInTime + ", overcast=" + OvercastValue + ", windMagnitude=" + WindMagnitudeValue );
		}
	}
}

class JMWeatherPhenomenon: JMWeatherBase
{
	float Forecast;
	float Time;
	float MinDuration;
	[NonSerialized()]
	float Actual;

	//! Noun the webhook line is built around ("fog", "wind direction").
	string GetLabel()
	{
		return "";
	}

	WeatherPhenomenon GetPhenomenon()
	{
		switch (Type())
		{
			case JMWeatherFog:
				return g_Game.GetWeather().GetFog();
			case JMWeatherRain:
				return g_Game.GetWeather().GetRain();
			case JMWeatherSnow:
				return g_Game.GetWeather().GetSnowfall();
			case JMWeatherWindMagnitude:
				return g_Game.GetWeather().GetWindMagnitude();
			case JMWeatherWindDirection:
				return g_Game.GetWeather().GetWindDirection();
			case JMWeatherOvercast:
				return g_Game.GetWeather().GetOvercast();
		}

		return null;
	}

	override string Describe()
	{
		return "Set " + GetLabel() + "=" + Forecast;
	}

	//! Every phenomenon has the same three fields, so one write covers all of them.
	override void WriteTo( ParamsWriteContext ctx ) { ctx.Write( this ); }

	override void Apply()
	{
		if (Forecast != -1)
		{
			WeatherPhenomenon phenom = GetPhenomenon();
			if ( phenom )
			{
				float clampedForecast = Forecast;
				switch (Type())
				{
					case JMWeatherWindMagnitude:
					{
						float maxSpeed = g_Game.GetWeather().GetWindMaximumSpeed();
						clampedForecast = Math.Clamp( clampedForecast, 0.0, maxSpeed );
						break;
					}
					case JMWeatherWindDirection:
					{
						clampedForecast = Math.Clamp( clampedForecast, -Math.PI, Math.PI );
						break;
					}
					default:
					{
						float minLimit, maxLimit;
						phenom.GetLimits( minLimit, maxLimit );
						if ( maxLimit > minLimit )
							clampedForecast = Math.Clamp( clampedForecast, minLimit, maxLimit );
						else
							clampedForecast = Math.Clamp( clampedForecast, 0.0, 1.0 );
						break;
					}
				}

				phenom.Set( clampedForecast, Time, MinDuration );
			}
		}
	}

	//! Apply with a caller-supplied fade and hold, leaving the values the
	//! preset was saved with untouched.
	//!
	//! The dynamic chain draws fresh timing on every roll. Writing that draw
	//! into the stored preset would persist a random number into Weather.json
	//! and quietly rewrite what the admin authored, so it is stamped for the
	//! duration of the call and put back afterwards.
	void ApplyTimed( float timeOverride, float durationOverride )
	{
		float savedTime     = Time;
		float savedDuration = MinDuration;

		Time        = timeOverride;
		MinDuration = durationOverride;

		Apply();

		Time        = savedTime;
		MinDuration = savedDuration;
	}

	override void SetFromWorld()
	{
		Forecast = GetPhenomenon().GetForecast();

		//! NOT GetNextChange(). That is the countdown until the weather
		//! controller computes its next forecast - up to an hour - and Apply
		//! feeds this field back in as the INTERPOLATION time. Reading the
		//! world and re-applying it therefore produced multi-minute fades
		//! nobody asked for. A captured state is applied as it stands.
		Time = 0;

		MinDuration = 240.0;
		Actual = GetPhenomenon().GetActual();
	}

	override void StopCurrentChangeInProgress()
	{
		if (Actual != Forecast)
		{
			float forecast = Forecast;
			float time = Time;
			float minDuration = MinDuration;

			Forecast = Actual;
			Time = 0;
			MinDuration = GetPhenomenon().GetNextChange();

			Apply();

			Forecast = forecast;
			Time = time;
			MinDuration = minDuration;
		}
	}

	override void ResumeCurrentChangeInProgress()
	{
		if (Actual != Forecast)
		{
			float change = Forecast - Actual;
			float timeToForecast = Math.AbsFloat(change) * 1000;  //! Each 0.1% of change = one second
			float timeUntilNextChange = GetPhenomenon().GetNextChange();

			if (timeToForecast > timeUntilNextChange)
			{
				Forecast = Actual + Math.Sign(change) * timeUntilNextChange * 0.001;
				timeToForecast = timeUntilNextChange;
			}

			float time = Time;
			float minDuration = MinDuration;

			Time = timeToForecast;
			MinDuration = timeUntilNextChange - timeToForecast;

			Apply();

			Time = time;
			MinDuration = minDuration;
		}
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			string type = ClassName();
			type.Replace("JMWeather", "");
			GetCommunityOnlineToolsBase().Log( pidentLog, type + " " + Forecast + ", " + Time + ", " + MinDuration );
		}
	}
}

class JMWeatherFog: JMWeatherPhenomenon
{
	override int GetRPC() { return JMWeatherModuleRPC.Fog; }
	override string GetPermission() { return JMConstants.PERM_WEATHER_FOG; }
	override string GetLabel() { return "fog"; }
}

class JMWeatherDynamicFog: JMWeatherBase
{
	float Distance;
	float Height;
	float Bias;
	float Time;

	override int GetRPC() { return JMWeatherModuleRPC.DynamicFog; }

	override string GetPermission() { return JMConstants.PERM_WEATHER_FOG_DYNAMIC; }

	override string Describe() { return "Set dynamic fog dist=" + Distance + " height=" + Height; }

	override void WriteTo( ParamsWriteContext ctx ) { ctx.Write( this ); }

	override void Apply()
	{
		if (Distance != -1)
		{
			float dist = Math.Clamp( Distance, 0.0, 1.0 );
			float hgt = Math.Clamp( Height, 0.0, 1.0 );
			float t = Math.Max( Time, 0.0 );
			g_Game.GetWeather().SetDynVolFogDistanceDensity( dist, t );
			g_Game.GetWeather().SetDynVolFogHeightDensity( hgt, t );
			g_Game.GetWeather().SetDynVolFogHeightBias( Bias, t );
		}
	}

	override void SetFromWorld()
	{
		Distance = g_Game.GetWeather().GetDynVolFogDistanceDensity();
		Height = g_Game.GetWeather().GetDynVolFogHeightDensity();
		Bias = g_Game.GetWeather().GetDynVolFogHeightBias();
	}

	override void StopCurrentChangeInProgress()
	{
		Time = 0;

		Apply();
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "DynamicFog " + Distance + ", " + Height + ", " + Bias );
		}
	}
}

class JMWeatherRain: JMWeatherPhenomenon
{
	override int GetRPC() { return JMWeatherModuleRPC.Rain; }
	override string GetPermission() { return JMConstants.PERM_WEATHER_RAIN; }
	override string GetLabel() { return "rain"; }
}

class JMWeatherSnow: JMWeatherPhenomenon
{
	override int GetRPC() { return JMWeatherModuleRPC.Snow; }
	override string GetPermission() { return JMConstants.PERM_WEATHER_SNOW; }
	override string GetLabel() { return "snow"; }
}

class JMWeatherOvercast: JMWeatherPhenomenon
{
	override int GetRPC() { return JMWeatherModuleRPC.Overcast; }
	override string GetPermission() { return JMConstants.PERM_WEATHER_OVERCAST; }
	override string GetLabel() { return "overcast"; }
}

class JMWeatherWindMagnitude: JMWeatherPhenomenon
{
	override int GetRPC() { return JMWeatherModuleRPC.WindMagnitude; }
	override string GetPermission() { return JMConstants.PERM_WEATHER_WIND; }
	override string GetLabel() { return "wind magnitude"; }
}

class JMWeatherWindDirection: JMWeatherPhenomenon
{
	override int GetRPC() { return JMWeatherModuleRPC.WindDirection; }
	override string GetPermission() { return JMConstants.PERM_WEATHER_WIND; }
	override string GetLabel() { return "wind direction"; }
}

class JMWeatherWindFunction: JMWeatherBase
{
	float Min;
	float Max;
	float Speed;

	override int GetRPC() { return JMWeatherModuleRPC.WindFunctionParams; }

	override string GetPermission() { return JMConstants.PERM_WEATHER_WIND_FUNCPARAMS; }

	override string Describe() { return "Set wind function params"; }

	override void WriteTo( ParamsWriteContext ctx ) { ctx.Write( this ); }

	override void Apply()
	{
		if (Speed != -1)
		{
			float minF = Math.Clamp( Min, 0.0, 1.0 );
			float maxF = Math.Clamp( Max, 0.0, 1.0 );
			float spd = Math.Max( Speed, 0.0 );
			g_Game.GetWeather().SetWindFunctionParams( minF, maxF, spd );
		}
	}

	override void SetFromWorld()
	{
		g_Game.GetWeather().GetWindFunctionParams( Min, Max, Speed );
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "WindFunction " + Min + ", " + Max + ", " + Speed );
		}
	}
}

class JMWeatherDate: JMWeatherBase
{
	int Year;
	int Month;
	int Day;
	int Hour;
	int Minute;

	override int GetRPC() { return JMWeatherModuleRPC.Date; }

	override string GetPermission() { return JMConstants.PERM_WEATHER_DATE; }

	override string GetWebhookType() { return "SetTime"; }

	override string Describe() { return "Set date " + Year + "/" + Month + "/" + Day + " " + Hour + ":" + Minute; }

	override bool IsPushedToClients() { return true; }

	override void WriteTo( ParamsWriteContext ctx ) { ctx.Write( this ); }

	override void Apply()
	{
		if (Year != -1)
			g_Game.GetWorld().SetDate( Year, Month, Day, Hour, Minute );
	}

	override void SetFromWorld()
	{
		g_Game.GetWorld().GetDate( Year, Month, Day, Hour, Minute );
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "Date " + Year + "/" + Month + "/" + Day + " " + Hour + ":" + Minute );
		}
	}
}

class JMWeatherRainThreshold: JMWeatherBase
{
	float OvercastMin;
	float OvercastMax;
	float Time;

	override int GetRPC() { return JMWeatherModuleRPC.RainThresholds; }

	override string GetPermission() { return JMConstants.PERM_WEATHER_RAIN_THRESHOLDS; }

	override string Describe() { return "Set rain thresholds min=" + OvercastMin + " max=" + OvercastMax; }

	override void WriteTo( ParamsWriteContext ctx ) { ctx.Write( this ); }

	override void Apply()
	{
		if (Time != -1)
		{
			float minO = Math.Clamp( OvercastMin, 0.0, 1.0 );
			float maxO = Math.Clamp( OvercastMax, 0.0, 1.0 );
			float t = Math.Max( Time, 0.0 );
			g_Game.GetWeather().SetRainThresholds( minO, maxO, t );
		}
	}

	//! SetRainThresholds / SetSnowfallThresholds have no getters either, so
	//! there is nothing to read. -1 on Time makes Apply skip it - see the note
	//! on JMWeatherStorm.SetFromWorld.
	override void SetFromWorld()
	{
		OvercastMin = -1;
		OvercastMax = -1;
		Time = -1;
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "RainThreshold " + OvercastMin + ", " + OvercastMax + ", " + Time );
		}
	}
}

class JMWeatherSnowThreshold: JMWeatherBase
{
	float OvercastMin;
	float OvercastMax;
	float Time;

	override int GetRPC() { return JMWeatherModuleRPC.SnowThresholds; }

	override string GetPermission() { return JMConstants.PERM_WEATHER_SNOW_THRESHOLDS; }

	override string Describe() { return "Set snow thresholds min=" + OvercastMin + " max=" + OvercastMax; }

	override void WriteTo( ParamsWriteContext ctx ) { ctx.Write( this ); }

	override void Apply()
	{
		if (Time != -1)
		{
			float minO = Math.Clamp( OvercastMin, 0.0, 1.0 );
			float maxO = Math.Clamp( OvercastMax, 0.0, 1.0 );
			float t = Math.Max( Time, 0.0 );
			g_Game.GetWeather().SetSnowfallThresholds( minO, maxO, t );
		}
	}

	//! SetRainThresholds / SetSnowfallThresholds have no getters either, so
	//! there is nothing to read. -1 on Time makes Apply skip it - see the note
	//! on JMWeatherStorm.SetFromWorld.
	override void SetFromWorld()
	{
		OvercastMin = -1;
		OvercastMax = -1;
		Time = -1;
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "SnowfallThreshold " + OvercastMin + ", " + OvercastMax + ", " + Time );
		}
	}
}

class JMWeatherPreset
{
	string Name;
	float Time;
	autoptr JMWeatherDate PDate;
	autoptr JMWeatherStorm Storm;
	autoptr JMWeatherSandstorm PSandstorm;
	autoptr JMWeatherOvercast POvercast;
	autoptr JMWeatherFog PFog;
	autoptr JMWeatherDynamicFog PDynFog;
	autoptr JMWeatherRain PRain;
	autoptr JMWeatherRainThreshold RainThreshold;
	autoptr JMWeatherSnow PSnow;
	autoptr JMWeatherSnowThreshold SnowThreshold;
	autoptr JMWeatherWindMagnitude PWindMagnitude;
	autoptr JMWeatherWindDirection PWindDirection;
	autoptr JMWeatherWindFunction WindFunc;

	// --- Dynamic chain -------------------------------------------------------
	//! Everything the chain needs the moment it arrives here: whether this
	//! preset takes part at all, how long it fades in over, how long it holds,
	//! and where it can go next.
	//!
	//! Held on the preset rather than centrally so a preset describes itself.
	//! The timing belongs to the preset being ENTERED - a storm and a clear sky
	//! have no business sharing a hold.

	//! Whether the random pick at server start may land here. A preset meant to
	//! be applied by hand can stay out of the rotation and still be reachable
	//! from other presets.
	bool InRotation;

	//! Seconds. A roll draws from between the two; equal bounds mean fixed
	//! timing, which is a range of one rather than a special case.
	int DurationMin;
	int DurationMax;
	int TransitionMin;
	int TransitionMax;
	autoptr array< ref JMWeatherNextState > NextStates;

	void JMWeatherPreset()
	{
		InRotation = true;

		DurationMin   = JMWeatherSerialize.DYNAMIC_DEFAULT_DURATION_MIN;
		DurationMax   = JMWeatherSerialize.DYNAMIC_DEFAULT_DURATION_MAX;
		TransitionMin = JMWeatherSerialize.DYNAMIC_DEFAULT_TRANSITION_MIN;
		TransitionMax = JMWeatherSerialize.DYNAMIC_DEFAULT_TRANSITION_MAX;

		NextStates = new array< ref JMWeatherNextState >;

		PDate = new JMWeatherDate;

		Storm = new JMWeatherStorm;
		PSandstorm = new JMWeatherSandstorm;
		POvercast = new JMWeatherOvercast;

		PFog = new JMWeatherFog;
		PDynFog = new JMWeatherDynamicFog;

		PRain = new JMWeatherRain;
		RainThreshold = new JMWeatherRainThreshold;

		PSnow = new JMWeatherSnow;
		SnowThreshold = new JMWeatherSnowThreshold;

		PWindMagnitude = new JMWeatherWindMagnitude;
		PWindDirection = new JMWeatherWindDirection;
		WindFunc = new JMWeatherWindFunction;
	}

	array<JMWeatherBase> GetPhenomena()
	{
		array<JMWeatherBase> phenomena = {};

		typename e = Type();
		int cnt = e.GetVariableCount();
		JMWeatherBase phenomenom;

		for (int i = 0; i < cnt; ++i)
		{
			if (e.GetVariableValue(this, i, phenomenom))
				phenomena.Insert(phenomenom);
		}

		return phenomena;
	}

	void SetFromWorld()
	{
		array<JMWeatherBase> phenomena = GetPhenomena();

		foreach (JMWeatherBase phenomenom: phenomena)
		{
			phenomenom.SetFromWorld();
		}
	}

	void Apply()
	{
		array<JMWeatherBase> phenomena = GetPhenomena();

		foreach (JMWeatherBase phenomenom: phenomena)
		{
			phenomenom.Apply();
		}
	}

	//! Pick where the chain goes from here, or "" to stay put.
	//!
	//! The candidates are WEIGHTS drawn against their own total rather than
	//! against a fixed 100 - two candidates at 30 and 10 pick the first three
	//! times in four. The form keeps them summing to 100 so the numbers on
	//! screen are the real odds, but nothing here relies on that.
	//!
	//! A preset that lists ITSELF is how weather persists across several rolls;
	//! there is no implicit "stay put" remainder. "" comes back only when this
	//! preset names no candidates at all.
	string RollNext()
	{
		if ( !NextStates )
			return "";

		int i;
		int total = 0;

		for ( i = 0; i < NextStates.Count(); i++ )
			total += NextStates[i].Chance;

		if ( total <= 0 )
			return "";

		int roll = Math.RandomInt( 0, total );
		int cumulative = 0;

		for ( i = 0; i < NextStates.Count(); i++ )
		{
			cumulative += NextStates[i].Chance;

			if ( roll < cumulative )
				return NextStates[i].To;
		}

		return "";
	}

	//! Carry the dynamic block over from another copy of this preset.
	//!
	//! Saving a preset from the form builds a BRAND NEW JMWeatherPreset out of
	//! the tab controls, which know nothing about the chain - so without this
	//! the update path would reset the chain settings to defaults every time
	//! the weather values were saved. See JMWeatherModule.Exec_UpdatePreset.
	void CopyDynamicFrom( JMWeatherPreset other )
	{
		if ( !other )
			return;

		InRotation    = other.InRotation;
		DurationMin   = other.DurationMin;
		DurationMax   = other.DurationMax;
		TransitionMin = other.TransitionMin;
		TransitionMax = other.TransitionMax;

		NextStates.Clear();

		if ( !other.NextStates )
			return;

		JMWeatherNextState copy;

		for ( int i = 0; i < other.NextStates.Count(); i++ )
		{
			copy = new JMWeatherNextState;
			copy.To     = other.NextStates[i].To;
			copy.Chance = other.NextStates[i].Chance;

			NextStates.Insert( copy );
		}
	}

	//! Drop every candidate naming a preset that no longer exists.
	void PruneNextStates( string removed )
	{
		if ( !NextStates )
			return;

		for ( int i = NextStates.Count() - 1; i >= 0; i-- )
		{
			if ( NextStates[i].To == removed )
				NextStates.Remove( i );
		}
	}

	//! Apply, but with the dynamic chain's timing rather than the preset's own.
	//!
	//! Only the phenomena take the override - they are the parts that fade. The
	//! date, storm, threshold and wind-function blocks are stamped exactly as
	//! they were authored.
	void ApplyTimed( float transition, float duration )
	{
		PDate.Apply();

		Storm.Apply();
		PSandstorm.Apply();
		POvercast.ApplyTimed( transition, duration );

		PFog.ApplyTimed( transition, duration );
		PDynFog.Apply();

		PRain.ApplyTimed( transition, duration );
		RainThreshold.Apply();

		PSnow.ApplyTimed( transition, duration );
		SnowThreshold.Apply();

		PWindMagnitude.ApplyTimed( transition, duration );
		PWindDirection.ApplyTimed( transition, duration );
		WindFunc.Apply();
	}

	void StopCurrentChangesInProgress()
	{
		array<JMWeatherBase> phenomena = GetPhenomena();

		foreach (JMWeatherBase phenomenom: phenomena)
		{
			phenomenom.StopCurrentChangeInProgress();
		}
	}

	void ResumeCurrentChangesInProgress()
	{
		array<JMWeatherBase> phenomena = GetPhenomena();

		foreach (JMWeatherBase phenomenom: phenomena)
		{
			phenomenom.ResumeCurrentChangeInProgress();
		}
	}

	void Log( PlayerIdentity pidentLogPP )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLogPP, "Start Weather Preset " + Name );

			array<JMWeatherBase> phenomena = GetPhenomena();

			foreach (JMWeatherBase phenomenom: phenomena)
			{
				phenomenom.Log(pidentLogPP);
			}
		}
	}
}
