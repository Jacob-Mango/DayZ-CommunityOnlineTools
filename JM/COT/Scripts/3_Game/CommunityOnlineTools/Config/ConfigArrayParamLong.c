#ifndef CF_COT_MOVE
class ConfigArrayParamLong : ConfigArrayParam
{
	private int _value;

	override string GetType()
	{
		return "LONG_PARAM";
	}

	override bool IsLong()
	{
		return true;
	}

	override int GetLong()
	{
		return _value;
	}

	override void SetLong( int value )
	{
		_value = value;
	}
};
#endif