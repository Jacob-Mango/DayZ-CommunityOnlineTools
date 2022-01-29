[CF_RegisterModule(COTWeatherModule)]
class COTWeatherModule : COTModule
{
	static const int s_DaysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	static const int s_DaysTotal[12] = {31,59,90,120,151,181,212,243,273,304,334,365};

	int m_Hour;
	int m_Minute;
	float m_Time;
	string m_TimeString;

	int m_Day;
	int m_Month;
	int m_Year;
	float m_Date;
	string m_DateString;

	float m_Overcast;
	string m_OvercastString;

	float m_Rain;
	string m_RainString;

	float m_Fog;
	string m_FogString;

	float m_Wind;
	string m_WindString;

	float m_Temperature;
	string m_TemperatureString;

	override void OnInit()
	{
		super.OnInit();

		GetRPCManager().AddRPC("COT_Weather", "Weather_SetStorm", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("COT_Weather", "Weather_SetFog", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("COT_Weather", "Weather_SetRain", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("COT_Weather", "Weather_SetOvercast", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("COT_Weather", "Weather_SetWindFunctionParams", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("COT_Weather", "Weather_SetDate", this, SingeplayerExecutionType.Client);

		GetPermissionsManager().RegisterPermission("Weather.Date");

		GetPermissionsManager().RegisterPermission("Weather.Wind");
		GetPermissionsManager().RegisterPermission("Weather.Wind.FunctionParams");

		GetPermissionsManager().RegisterPermission("Weather.Storm");
		GetPermissionsManager().RegisterPermission("Weather.Overcast");
		GetPermissionsManager().RegisterPermission("Weather.Fog");

		GetPermissionsManager().RegisterPermission("Weather.Rain");
		GetPermissionsManager().RegisterPermission("Weather.Rain.Thresholds");

		GetPermissionsManager().RegisterPermission("Weather.Preset");
		GetPermissionsManager().RegisterPermission("Weather.Preset.Create");
		GetPermissionsManager().RegisterPermission("Weather.Preset.Update");
		GetPermissionsManager().RegisterPermission("Weather.Preset.Remove");

		GetPermissionsManager().RegisterPermission("Weather.View");
	}

	override void OnShow(Class sender, CF_EventArgs args)
	{
		super.OnShow(sender, args);

		Reset();
	}

	void Click_Refresh(Class sender, CF_EventArgs args)
	{
		Reset();
	}

	void Click_Apply(Class sender, CF_EventArgs args)
	{
		GetRPCManager().SendRPC("COT_Weather", "Weather_SetRain", new Param3<float, float, float>(m_Rain, 0, 0), true);
		GetRPCManager().SendRPC("COT_Weather", "Weather_SetStorm", new Param3<float, float, float>(m_Rain * m_Rain, 0.8, 4000), true);
		GetRPCManager().SendRPC("COT_Weather", "Weather_SetFog", new Param3<float, float, float>(m_Fog, 0, 0), true);
		GetRPCManager().SendRPC("COT_Weather", "Weather_SetOvercast", new Param3<float, float, float>(m_Overcast, 0, 0), true);
		GetRPCManager().SendRPC("COT_Weather", "Weather_SetDate", new Param5<int, int, int, int, int>(m_Year, m_Month, m_Day, m_Hour, m_Minute), true);

		Weather weather = GetGame().GetWeather();
		if (weather)
		{
			GetRPCManager().SendRPC("COT_Weather", "Weather_SetWindFunctionParams", new Param3<float, float, float>(m_Wind * weather.GetWindMaximumSpeed(), weather.GetWindMaximumSpeed(), 1), true);
		}

		Reset();
	}

	void OnChange(Class sender, CF_EventArgs args)
	{
		UpdateStrings();
	}

	void UpdateStrings()
	{
		int time = m_Time * 1439;
		m_Hour = time / 60;
		m_Minute = time % 60;

		int days = m_Date * 364;
		m_Month = 0;
		m_Day = 0;
		for (m_Month = 0; m_Month < 12; m_Month++)
		{
			if (days - s_DaysTotal[m_Month] < 0)
			{
				m_Day = s_DaysInMonth[m_Month] - (s_DaysTotal[m_Month] - days);
				break;
			}
		}

		CF_String minute = m_Minute.ToString();
		string sminute = minute.PadStringFront(2, "0");

		CF_String hour = m_Hour.ToString();
		string shour = hour.PadStringFront(2, "0");

		CF_String day = m_Day.ToString();
		string sday = day.PadStringFront(2, "0");

		CF_String month = (m_Month + 1).ToString();
		string smonth = month.PadStringFront(2, "0");

		CF_String year = m_Year.ToString();
		string syear = year.PadStringFront(4, "0");

		m_TimeString = shour + ":" + sminute;
		m_DateString = sday + "/" + smonth + "/" + syear;

		m_OvercastString = (m_Overcast * 100.0).ToString() + "%";
		m_RainString = (m_Rain * 100.0).ToString() + "%";
		m_FogString = (m_Fog * 100.0).ToString() + "%";
		m_WindString = (m_Wind * 100.0).ToString() + "%";
		m_TemperatureString = m_Temperature.ToString() + "°C";

		NotifyPropertyChanged("m_TimeString");
		NotifyPropertyChanged("m_DateString");
		NotifyPropertyChanged("m_OvercastString");
		NotifyPropertyChanged("m_RainString");
		NotifyPropertyChanged("m_FogString");
		NotifyPropertyChanged("m_WindString");
		NotifyPropertyChanged("m_TemperatureString");
	}

	void Reset()
	{
		m_Time = 0;
		m_Date = 0;
		m_Overcast = 0;
		m_Rain = 0;
		m_Fog = 0;
		m_Wind = 0;
		m_Temperature = 0;

		World world = GetGame().GetWorld();
		if (world)
		{
			world.GetDate(m_Year, m_Month, m_Day, m_Hour, m_Minute);

			m_Time = ((m_Hour * 60.0) + m_Minute) / 1439.0;
			
			m_Date = m_Day;
			for (int month = m_Month - 1; month >= 0; month--)
			{
				m_Date += s_DaysInMonth[month];
			}

			m_Date = m_Date / 364;
		}

		Weather weather = GetGame().GetWeather();
		if (weather)
		{
			m_Overcast = weather.GetOvercast().GetActual();
			m_Rain = weather.GetRain().GetActual();
			m_Fog = weather.GetFog().GetActual();
			m_Wind = weather.GetWindSpeed() / weather.GetWindMaximumSpeed();
		}

		Mission mission = GetGame().GetMission();
		if (mission)
		{
			WorldData worldData = mission.GetWorldData();
			if (worldData)
			{
				m_Temperature = worldData.GetBaseEnvTemperatureExact(m_Month, m_Day, m_Hour, m_Minute);
			}
		}

		NotifyPropertyChanged("m_Time");
		NotifyPropertyChanged("m_Date");
		NotifyPropertyChanged("m_Overcast");
		NotifyPropertyChanged("m_Rain");
		NotifyPropertyChanged("m_Fog");
		NotifyPropertyChanged("m_Wind");
		NotifyPropertyChanged("m_Temperature");

		UpdateStrings();
	}

	override string GetInputToggle()
	{
		return "UACOTToggleWeather";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/gui/layouts/weather.layout";
	}

	override string GetTitle()
	{
		return "#STR_COT_WEATHER_MODULE_NAME";
	}

	override string GetIconName()
	{
		return "W";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override string GetWebhookTitle()
	{
		return "Weather Module";
	}

	override void GetWebhookTypes(out array<string> types)
	{
		types.Insert("Date");
		types.Insert("Wind");
		types.Insert("Overcast");
		types.Insert("Fog");
		types.Insert("Rain");
		types.Insert("Storm");
	}

	void Weather_SetDate(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param5<int, int, int, int, int> data;
		if (!ctx.Read(data))
			return;

		if (type == CallType.Client)
		{
			GetGame().GetWorld().SetDate(data.param1, data.param2, data.param3, data.param4, data.param5);
			return;
		}

		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Weather.Date", sender, instance))
			return;

		if (type == CallType.Server && GetGame().IsMultiplayer())
		{
			GetRPCManager().SendRPC("COT_Weather", "Weather_SetDate", new Param5<int, int, int, int, int>(data.param1, data.param2, data.param3, data.param4, data.param5), true);

			GetGame().GetWorld().SetDate(data.param1, data.param2, data.param3, data.param4, data.param5);

			GetCommunityOnlineToolsBase().Log(sender, "Set the date to " + data.param1 + "/" + data.param2 + "/" + data.param3 + " " + data.param4 + ":" + data.param5);
			SendWebhook("Date", instance, "Set the date to " + data.param1 + "/" + data.param2 + "/" + data.param3 + " " + data.param4 + ":" + data.param5);
		}
	}

	void Weather_SetWindFunctionParams(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param3<float, float, float> data;
		if (!ctx.Read(data))
			return;

		if (type == CallType.Client)
		{
			GetGame().GetWeather().SetWindFunctionParams(data.param1, data.param2, data.param3);
			return;
		}

		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Weather.Wind.FunctionParams", sender, instance))
			return;

		if (type == CallType.Server && GetGame().IsMultiplayer())
		{
			GetRPCManager().SendRPC("COT_Weather", "Weather_SetWindFunctionParams", new Param3<float, float, float>(data.param1, data.param2, data.param3), true);

			GetGame().GetWeather().SetWindFunctionParams(data.param1, data.param2, data.param3);

			GetCommunityOnlineToolsBase().Log(sender, "Set wind to " + data.param1 + " " + data.param2 + " " + data.param3);
			SendWebhook("Wind", instance, "Set wind to " + data.param1 + " " + data.param2 + " " + data.param3);
		}
	}

	void Weather_SetOvercast(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param3<float, float, float> data;
		if (!ctx.Read(data))
			return;

		if (type == CallType.Client)
		{
			GetGame().GetWeather().GetOvercast().Set(data.param1, data.param2, data.param3);
			return;
		}

		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Weather.Overcast", sender, instance))
			return;

		if (type == CallType.Server && GetGame().IsMultiplayer())
		{
			GetRPCManager().SendRPC("COT_Weather", "Weather_SetOvercast", new Param3<float, float, float>(data.param1, data.param2, data.param3), true);

			GetGame().GetWeather().GetOvercast().Set(data.param1, data.param2, data.param3);

			GetCommunityOnlineToolsBase().Log(sender, "Set overcast to " + data.param1 + " " + data.param2 + " " + data.param3);
			SendWebhook("Overcast", instance, "Set overcast to " + data.param1 + " " + data.param2 + " " + data.param3);
		}
	}

	void Weather_SetFog(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param3<float, float, float> data;
		if (!ctx.Read(data))
			return;

		if (type == CallType.Client)
		{
			GetGame().GetWeather().GetFog().Set(data.param1, data.param2, data.param3);
			return;
		}

		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Weather.Fog", sender, instance))
			return;

		if (type == CallType.Server && GetGame().IsMultiplayer())
		{
			GetRPCManager().SendRPC("COT_Weather", "Weather_SetFog", new Param3<float, float, float>(data.param1, data.param2, data.param3), true);

			GetGame().GetWeather().GetFog().Set(data.param1, data.param2, data.param3);

			GetCommunityOnlineToolsBase().Log(sender, "Set fog to " + data.param1 + " " + data.param2 + " " + data.param3);
			SendWebhook("Fog", instance, "Set fog to " + data.param1 + " " + data.param2 + " " + data.param3);
		}
	}

	void Weather_SetRain(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param3<float, float, float> data;
		if (!ctx.Read(data))
			return;

		if (type == CallType.Client)
		{
			GetGame().GetWeather().GetRain().Set(data.param1, data.param2, data.param3);
			return;
		}

		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Weather.Rain", sender, instance))
			return;

		if (type == CallType.Server && GetGame().IsMultiplayer())
		{
			GetRPCManager().SendRPC("COT_Weather", "Weather_SetRain", new Param3<float, float, float>(data.param1, data.param2, data.param3), true);

			GetGame().GetWeather().GetRain().Set(data.param1, data.param2, data.param3);

			GetCommunityOnlineToolsBase().Log(sender, "Set rain to " + data.param1 + " " + data.param2 + " " + data.param3);
			SendWebhook("Rain", instance, "Set rain to " + data.param1 + " " + data.param2 + " " + data.param3);
		}
	}

	void Weather_SetStorm(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param3<float, float, float> data;
		if (!ctx.Read(data))
			return;

		if (type == CallType.Client)
		{
			GetGame().GetWeather().SetStorm(data.param1, data.param2, data.param3);
			return;
		}

		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Weather.Storm", sender, instance))
			return;

		if (type == CallType.Server && GetGame().IsMultiplayer())
		{
			GetRPCManager().SendRPC("COT_Weather", "Weather_SetStorm", new Param3<float, float, float>(data.param1, data.param2, data.param3), true);

			GetGame().GetWeather().SetStorm(data.param1, data.param2, data.param3);

			GetCommunityOnlineToolsBase().Log(sender, "Set storm to " + data.param1 + " " + data.param2 + " " + data.param3);
			SendWebhook("Storm", instance, "Set storm to " + data.param1 + " " + data.param2 + " " + data.param3);
		}
	}
};
