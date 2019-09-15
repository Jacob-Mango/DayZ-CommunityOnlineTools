class JMWeatherForm extends JMFormBase
{
	private static const int m_DaysInMonth [ 12 ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	void JMWeatherForm()
	{
	}

	void ~JMWeatherForm()
	{
	}

	override void OnInit( bool fromMenu )
	{
		super.OnInit( fromMenu );
	}
	
	override string GetTitle()
	{
		return "Weather";
	}
	
	override string GetIconName()
	{
		return "W";
	}

	override bool ImageIsIcon()
	{
		return false;
	}
}