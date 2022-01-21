[CF_RegisterModule(COTWeatherModule)]
class COTWeatherModule : COTModule
{
	static const int s_DaysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	int m_Hour;
	int m_Minute;
	int m_Time;
	string m_TimeString;

	int m_Day;
	int m_Month;
	int m_Year;
	int m_Date;
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
	}

	void OnChange(Class sender, CF_EventArgs args)
	{
		UpdateStrings();
	}

	void UpdateStrings()
	{
		int time = m_Time * 14.39;
		m_Hour = time / 60;
		m_Minute = time % 60;

		int start_day = m_Date * 3.64 + 1;

		for (int i = 0; i < 12; i++)
		{
			int days = s_DaysInMonth[i];
			if (start_day <= days)
			{
				m_Month = i + 1;
				m_Day = start_day;
				break;
			}

			start_day -= days;
		}

		m_TimeString = m_Hour.ToString() + ":" + m_Minute.ToString();
		m_DateString = m_Day.ToString() + "/" + m_Month.ToString() + "/" + m_Year.ToString();
		m_OvercastString = m_Overcast.ToString() + "%";
		m_RainString = m_Rain.ToString() + "%";
		m_FogString = m_Fog.ToString() + "%";
		m_WindString = m_Wind.ToString() + "%";
		m_TemperatureString = m_Temperature.ToString() + "°C";

		NotifyPropertyChanged("m_TimeString");
		NotifyPropertyChanged("m_DateString");
		NotifyPropertyChanged("m_OvercastString");
		NotifyPropertyChanged("m_RainString");
		NotifyPropertyChanged("m_FogString");
		NotifyPropertyChanged("m_TemperatureString");
	}

	void Reset()
	{
		World world = GetGame().GetWorld();
		if (world)
		{
			world.GetDate(m_Year, m_Month, m_Day, m_Hour, m_Minute);

			m_Time = ((m_Hour * 60) + m_Minute) / 14.39;

			float start_day = m_Date;
			int month_tmp = m_Month;
			while (month_tmp > 1)
			{
				month_tmp--;
				start_day += s_DaysInMonth[month_tmp];
			}

			m_Date = start_day / 3.64;
		}

		Weather weather = GetGame().GetWeather();
		if (weather)
		{
			m_Overcast = weather.GetOvercast().GetActual();
			m_Rain = weather.GetRain().GetActual();
			m_Fog = weather.GetFog().GetActual();
			m_Wind = weather.GetWindSpeed() / weather.GetWindMaximumSpeed();
		}

		UpdateStrings();
	}

	override string GetInputToggle()
	{
		return "UACOTToggleWeather";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/gui/layouts/weather_form_old.layout";
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
