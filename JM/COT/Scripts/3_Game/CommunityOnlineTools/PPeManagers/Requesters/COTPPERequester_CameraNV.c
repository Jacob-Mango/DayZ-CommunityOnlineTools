class COTPPERequester_CameraNV: PPERequester_CameraNV
{
	static const int COT_OFF = 0;
	static const int COT_ON  = 1;

	float m_COT_UndergroundPresenceFactorTarget;
	float m_COT_UndergroundPresenceFactor;
	float m_COT_UndergroundPresenceFactorVel[1];

	float m_COT_TimeVisibility_Out[6] = {
		0,
		0.5,  //! sunrise start
		1,
		1,
		0.5,  //! sunset start
		0
	};

	float m_COT_LastDaylightVisibility;
	
	override protected void SetNVMode(int mode)
	{
		SetTargetValueFloat(PPEExceptions.EYEACCOM,PPEEyeAccomodationNative.PARAM_INTENSITY,false,1.0,PPEEyeAccomodationNative.L_0_NVG_GENERIC,PPOperators.SET);
		
		switch (mode)
		{
			default:
			case COT_OFF:
				SetTargetValueColor(PostProcessEffectType.Glow,PPEGlow.PARAM_COLORIZATIONCOLOR,{0.0,0.0,0.0,0.0},PPEGlow.L_23_NVG,PPOperators.MULTIPLICATIVE);
				SetTargetValueFloatDefault(PPEExceptions.EXPOSURE,PPEExposureNative.PARAM_INTENSITY);
				SetTargetValueFloatDefault(PostProcessEffectType.FilmGrain,PPEFilmGrain.PARAM_SHARPNESS);
				SetTargetValueFloatDefault(PostProcessEffectType.FilmGrain,PPEFilmGrain.PARAM_GRAINSIZE);
				SetTargetValueFloat(PPEExceptions.NVLIGHTPARAMS,PPELightIntensityParamsNative.PARAM_LIGHT_MULT,false,1.0,PPELightIntensityParamsNative.L_0_NVG,PPOperators.HIGHEST);
				SetTargetValueFloat(PPEExceptions.NVLIGHTPARAMS,PPELightIntensityParamsNative.PARAM_NOISE_MULT,false,0.0,PPELightIntensityParamsNative.L_1_NVG,PPOperators.HIGHEST);
			break;
			case COT_ON:
				SetTargetValueColor(PostProcessEffectType.Glow,PPEGlow.PARAM_COLORIZATIONCOLOR,{1.0,1.0,1.0,1.0},PPEGlow.L_23_NVG,PPOperators.MULTIPLICATIVE);
				m_COT_LastDaylightVisibility = -1;
				SetTargetValueFloat(PostProcessEffectType.FilmGrain,PPEFilmGrain.PARAM_SHARPNESS,false,0.0,PPEFilmGrain.L_1_NVG,PPOperators.SET);
				SetTargetValueFloat(PostProcessEffectType.FilmGrain,PPEFilmGrain.PARAM_GRAINSIZE,false,0.0,PPEFilmGrain.L_2_NVG,PPOperators.SET);
				SetTargetValueFloat(PPEExceptions.NVLIGHTPARAMS,PPELightIntensityParamsNative.PARAM_LIGHT_MULT,false,0.0,PPELightIntensityParamsNative.L_0_NVG,PPOperators.HIGHEST);
				SetTargetValueFloat(PPEExceptions.NVLIGHTPARAMS,PPELightIntensityParamsNative.PARAM_NOISE_MULT,false,0.0,PPELightIntensityParamsNative.L_1_NVG,PPOperators.HIGHEST);
			break;
		}
		
		m_CurrentMode = mode;
	}

	void COT_UpdateExposure()
	{
		switch (m_CurrentMode)
		{
			case COT_ON:
				float daylightVisibility = COT_GetDaylightVisibility();
				float targetFactor = Math.Pow(m_COT_UndergroundPresenceFactorTarget, 8);

				if (daylightVisibility != m_COT_LastDaylightVisibility || m_COT_UndergroundPresenceFactor != targetFactor)
				{
					m_COT_LastDaylightVisibility = daylightVisibility;

					m_COT_UndergroundPresenceFactor = Math.SmoothCD(m_COT_UndergroundPresenceFactor, targetFactor, m_COT_UndergroundPresenceFactorVel, 0.1, 1000, 0.016667);

					float exposure = 7.0 * m_UGExposureCoef * (1.0 - (daylightVisibility * (1.0 - m_COT_UndergroundPresenceFactor)));

					SetTargetValueFloat(PPEExceptions.EXPOSURE,PPEExposureNative.PARAM_INTENSITY,false,exposure,PPEExposureNative.L_0_NVG_GOGGLES,PPOperators.ADD);
				}

				break;
		}
	}

	//! 0.0 = night, 1.0 = day
	float COT_GetDaylightVisibility()
	{
		int year, month, day, hour, minute;
		g_Game.GetWorld().GetDate(year, month, day, hour, minute);

		return COT_GetDaylightVisibilityEx(month, hour, minute);
	}

	//! 0.0 = night, 1.0 = day
	float COT_GetDaylightVisibilityEx(int month, int hour, int minute)
	{
		WorldData worldData = g_Game.GetMission().GetWorldData();

		float sunriseTimeStart = worldData.GetApproxSunriseTime(month);
		float sunsetTimeStart = worldData.GetApproxSunsetTime(month);
//Print(sunriseTimeStart);
		if (sunriseTimeStart < 0 || sunriseTimeStart > 12)  //! invalid worldData
			sunriseTimeStart = 12;

		if (sunsetTimeStart < sunriseTimeStart || sunsetTimeStart > 23.9833333)  //! invalid worldData
			sunsetTimeStart = 16;

		float daylightHours = sunsetTimeStart - sunriseTimeStart;
		float timeScale = daylightHours / 16.0;

		float time[6] = {
			Math.Max(sunriseTimeStart - 1.5 * timeScale, 0.0),
			sunriseTimeStart,
			sunriseTimeStart + 1.0 * timeScale,
			sunsetTimeStart - 1.5 * timeScale,
			sunsetTimeStart,
			Math.Min(sunsetTimeStart + 1.0 * timeScale, 23.9833333)
		};

		return Math.COT_LookUp(hour + minute / 60.0, 6, time, m_COT_TimeVisibility_Out);
	}
}
