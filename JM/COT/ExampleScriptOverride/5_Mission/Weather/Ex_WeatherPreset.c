#ifdef JM_CommunityOnlineTools
// Example: registering custom weather presets in JMWeatherModule.
modded class JMWeatherModule
{
	override void InitPresets()
	{
		super.InitPresets();

		JMWeatherPreset preset = new JMWeatherPreset();
		preset.Name = "Toxic Storm";
		preset.Overcast = 1.0;
		preset.Rain = 0.8;
		preset.Fog = 0.9;
		preset.WindSpeed = 15.0;

		m_Presets.Insert( preset );
	}
}
#endif
