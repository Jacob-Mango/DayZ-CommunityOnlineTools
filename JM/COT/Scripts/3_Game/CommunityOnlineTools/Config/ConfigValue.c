#ifndef CF_COT_MOVE
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