class COTPPERequester_CameraNV: PPERequester_CameraNV
{
	static const int COT_OFF = 0;
	static const int COT_ON  = 1;
	
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
				SetTargetValueFloat(PPEExceptions.EXPOSURE,PPEExposureNative.PARAM_INTENSITY,false,7.0 * m_UGExposureCoef,PPEExposureNative.L_0_NVG_OPTIC,PPOperators.ADD);
				SetTargetValueFloat(PostProcessEffectType.FilmGrain,PPEFilmGrain.PARAM_SHARPNESS,false,0.0,PPEFilmGrain.L_1_NVG,PPOperators.SET);
				SetTargetValueFloat(PostProcessEffectType.FilmGrain,PPEFilmGrain.PARAM_GRAINSIZE,false,0.0,PPEFilmGrain.L_2_NVG,PPOperators.SET);
				SetTargetValueFloat(PPEExceptions.NVLIGHTPARAMS,PPELightIntensityParamsNative.PARAM_LIGHT_MULT,false,0.0,PPELightIntensityParamsNative.L_0_NVG,PPOperators.HIGHEST);
				SetTargetValueFloat(PPEExceptions.NVLIGHTPARAMS,PPELightIntensityParamsNative.PARAM_NOISE_MULT,false,0.0,PPELightIntensityParamsNative.L_1_NVG,PPOperators.HIGHEST);
			break;
		}
		
		m_CurrentMode = mode;
	}
}