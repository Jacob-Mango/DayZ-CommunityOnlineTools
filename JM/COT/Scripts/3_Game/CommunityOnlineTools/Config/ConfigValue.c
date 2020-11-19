#ifndef CF_MODULE_CONFIG
class ConfigValue : ConfigEntry
{
	override string GetType()
	{
		return "VALUE";
	}
	
	override bool IsValue()
	{
		return true;
	}
};
#endif