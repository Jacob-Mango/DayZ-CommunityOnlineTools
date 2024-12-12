class JMWeatherOldForm: JMFormBase
{
	private static const int m_DaysInMonth [ 12 ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	protected ButtonWidget m_BtnSave;
	protected ButtonWidget m_BtnRefresh;
	protected ButtonWidget m_BtnCancel;
	protected ButtonWidget m_BtnPresetNight;
	protected ButtonWidget m_BtnPresetDusk;
	protected ButtonWidget m_BtnPresetDay;
	protected ButtonWidget m_BtnPresetDawn;
	protected ButtonWidget m_BtnPresetWeatherClear;
	protected ButtonWidget m_BtnPresetWeatherCloudy;
	protected ButtonWidget m_BtnPresetWeatherStorm;
	protected SliderWidget m_SldStartTime;
	protected TextWidget m_TxtStartTimeValue;
	protected SliderWidget m_SldStartDay;
	protected TextWidget m_TxtStartDayValue;
	protected SliderWidget m_SldOvercast;
	protected TextWidget m_TxtOvercastValue;
	protected SliderWidget m_SldRain;
	protected TextWidget m_TxtRainValue;
	protected SliderWidget m_SldSnow;
	protected TextWidget m_TxtSnowValue;
	protected SliderWidget m_SldFog;
	protected TextWidget m_TxtFogValue;
	protected SliderWidget m_SldWindForce;
	protected TextWidget m_TxtWindForceValue;
	protected Widget m_PnlTemperature;
	protected SliderWidget m_SldTemperature;
	protected TextWidget m_TxtTemperatureValue;	
	private CheckBoxWidget m_AutoRefresh;

	private int	  m_OrigYear;
	private int	  m_OrigMonth;
	private int	  m_OrigDay;
	private int	  m_OrigHour;
	private int	  m_OrigMinute;
	private float m_OrigOvercast;
	private float m_OrigRain;
	private float m_OrigSnow;
	private float m_OrigFog;
	private float m_OrigWindForce;
	private float m_OrigTemperature;

	private int	  m_CurrYear;
	private int	  m_CurrMonth;
	private int	  m_CurrDay;
	private int	  m_CurrHour;
	private int	  m_CurrMinute;
	private float m_CurrOvercast;
	private float m_CurrRain;
	private float m_CurrSnow;
	private float m_CurrFog;
	private float m_CurrWindForce;
	private float m_CurrTemperature;

	private JMWeatherOldModule m_Module;

	void JMWeatherOldForm()
	{
	}

	void ~JMWeatherOldForm()
	{
		if ( m_AutoRefresh && m_AutoRefresh.IsChecked() )
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(RefreshFields);
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_BtnSave			= ButtonWidget.Cast( layoutRoot.FindAnyWidget( "btn_save" ) );
		m_BtnCancel			= ButtonWidget.Cast( layoutRoot.FindAnyWidget( "btn_cancel" ) );
		m_BtnRefresh		= ButtonWidget.Cast( layoutRoot.FindAnyWidget( "btn_refresh" ) );

		m_SldStartTime		= SliderWidget.Cast( layoutRoot.FindAnyWidget( "sld_ppp_st_start_time" ) );
		m_TxtStartTimeValue	= TextWidget.Cast( layoutRoot.FindAnyWidget( "txt_ppp_st_start_time_value" ) );

		m_SldStartDay		= SliderWidget.Cast( layoutRoot.FindAnyWidget( "sld_ppp_st_start_day" ) );
		m_TxtStartDayValue	= TextWidget.Cast( layoutRoot.FindAnyWidget( "txt_ppp_st_start_day_value" ) );

		m_SldOvercast		= SliderWidget.Cast( layoutRoot.FindAnyWidget( "sld_ppp_st_overcast" ) );
		m_TxtOvercastValue	= TextWidget.Cast( layoutRoot.FindAnyWidget( "txt_ppp_st_overcast_value" ) );

		m_SldRain			= SliderWidget.Cast( layoutRoot.FindAnyWidget( "sld_ppp_st_rain" ) );
		m_TxtRainValue		= TextWidget.Cast( layoutRoot.FindAnyWidget( "txt_ppp_st_rain_value" ) );

		m_SldSnow			= SliderWidget.Cast( layoutRoot.FindAnyWidget( "sld_ppp_st_snow" ) );
		m_TxtSnowValue		= TextWidget.Cast( layoutRoot.FindAnyWidget( "txt_ppp_st_snow_value" ) );

		m_SldFog			= SliderWidget.Cast( layoutRoot.FindAnyWidget( "sld_ppp_st_fog" ) );
		m_TxtFogValue		= TextWidget.Cast( layoutRoot.FindAnyWidget( "txt_ppp_st_fog_value" ) );

		m_SldWindForce		= SliderWidget.Cast( layoutRoot.FindAnyWidget( "sld_ppp_st_wind_force" ) );
		m_TxtWindForceValue	= TextWidget.Cast( layoutRoot.FindAnyWidget( "txt_ppp_st_wind_force_value" ) );

		m_PnlTemperature		= layoutRoot.FindAnyWidget( "txt_ppp_st_temperature" );
		if ( !GetGame().IsMultiplayer() )
			m_PnlTemperature.Show(true);
		m_SldTemperature		= SliderWidget.Cast( layoutRoot.FindAnyWidget( "sld_ppp_st_temperature" ) );
		m_TxtTemperatureValue	= TextWidget.Cast( layoutRoot.FindAnyWidget( "txt_ppp_st_temperature_value" ) );
		
		m_BtnPresetNight		= ButtonWidget.Cast( layoutRoot.FindAnyWidget( "TimeButton1" ) );
		m_BtnPresetDusk			= ButtonWidget.Cast( layoutRoot.FindAnyWidget( "TimeButton2" ) );
		m_BtnPresetDay			= ButtonWidget.Cast( layoutRoot.FindAnyWidget( "TimeButton3" ) );
		m_BtnPresetDawn			= ButtonWidget.Cast( layoutRoot.FindAnyWidget( "TimeButton4" ) );
		
		m_BtnPresetWeatherClear	= ButtonWidget.Cast( layoutRoot.FindAnyWidget( "WeatherButton1" ) );
		m_BtnPresetWeatherCloudy= ButtonWidget.Cast( layoutRoot.FindAnyWidget( "WeatherButton2" ) );
		m_BtnPresetWeatherStorm	= ButtonWidget.Cast( layoutRoot.FindAnyWidget( "WeatherButton3" ) );
		
		m_AutoRefresh 			= CheckBoxWidget.Cast( layoutRoot.FindAnyWidget( "toggle_autorefresh" ) );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == m_BtnSave )
		{
			Save();
			return true;
		}

		if ( w == m_BtnRefresh )
		{
			RefreshFields();
			return true;
		}

		if ( w == m_AutoRefresh )
		{
			ToggleAutoRefresh();
			return true;
		}

		if ( w == m_BtnPresetNight )
		{
			PresetNight();
			return true;
		}

		if ( w == m_BtnPresetDusk )
		{
			PresetDusk();
			return true;
		}

		if ( w == m_BtnPresetDay )
		{
			PresetDay();
			return true;
		}

		if ( w == m_BtnPresetDawn )
		{
			PresetDawn();
			return true;
		}

		if ( w == m_BtnPresetWeatherClear )
		{
			PresetClearSky();
			return true;
		}

		if ( w == m_BtnPresetWeatherCloudy )
		{
			PresetCloudy();
			return true;
		}

		if ( w == m_BtnPresetWeatherStorm )
		{
			PresetStorm();
			return true;
		}

		return false;
	}

	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		if ( w == m_SldStartTime )
		{
			float slider_value_start_time = m_SldStartTime.GetCurrent() * 0.01;
			float start_time_f = slider_value_start_time * 1439;
			int start_time = start_time_f;
			m_CurrHour = start_time / 60;
			m_CurrMinute = start_time % 60;

			UpdateSliderStartTime( m_CurrHour, m_CurrMinute );

			//GetRPCManager().SendRPC( "COT_Weather", "Weather_SetDate", new Param5< int, int, int, int, int >( m_CurrYear, m_CurrMonth, m_CurrDay, m_CurrHour, m_CurrMinute ), true );

			return true;
		}
		else if ( w == m_SldStartDay )
		{
			float slider_value_start_day = m_SldStartDay.GetCurrent();
			float start_day_f = slider_value_start_day * 3.64 + 1;
			int start_day = start_day_f;

			for ( int i = 0; i < 12; i++ )
			{
				int days = m_DaysInMonth[i];
				if ( start_day <= days )
				{
					m_CurrMonth = i+1;
					m_CurrDay = start_day;
					break;
				}
				else
				{
					start_day -= days;
				}
			}

			UpdateSliderStartDay( m_CurrMonth, m_CurrDay );

			//GetRPCManager().SendRPC( "COT_Weather", "Weather_SetDate", new Param5< int, int, int, int, int >( m_CurrYear, m_CurrMonth, m_CurrDay, m_CurrHour, m_CurrMinute ), true );

			return true;
		}
		else if ( w == m_SldOvercast )
		{
			UpdateSliderOvercast();

			m_CurrOvercast = m_SldOvercast.GetCurrent() * 0.01;

			//GetRPCManager().SendRPC( "COT_Weather", "Weather_SetOvercast", new Param3< float, float, float >( m_CurrOvercast, 0, 0 ), true );

			//GetRPCManager().SendRPC( "COT_Weather", "Weather_SetDate", new Param5< int, int, int, int, int >( m_CurrYear, m_CurrMonth, m_CurrDay, m_CurrHour, m_CurrMinute + 5), true );

			return true;
		}
		else if ( w == m_SldRain )
		{
			UpdateSliderRain();

			m_CurrRain = m_SldRain.GetCurrent() * 0.01;

			//GetRPCManager().SendRPC( "COT_Weather", "Weather_SetRain", new Param3< float, float, float >( m_CurrRain, 0, 0 ), true );
			//GetRPCManager().SendRPC( "COT_Weather", "Weather_SetStorm", new Param3< float, float, float >( m_CurrRain * m_CurrRain, 0.8, 4000 ), true );

			return true;
		}
		else if ( w == m_SldSnow )
		{
			UpdateSliderSnow();

			m_CurrSnow = m_SldSnow.GetCurrent() * 0.01;

			//GetRPCManager().SendRPC( "COT_Weather", "Weather_SetSnow", new Param3< float, float, float >( m_CurrSnow, 0, 0 ), true );
			//GetRPCManager().SendRPC( "COT_Weather", "Weather_SetStorm", new Param3< float, float, float >( m_CurrSnow * m_CurrSnow, 0.8, 4000 ), true );

			return true;
		}
		else if ( w == m_SldFog )
		{
			UpdateSliderFog();

			m_CurrFog = m_SldFog.GetCurrent() * 0.01;

			//GetRPCManager().SendRPC( "COT_Weather", "Weather_SetFog", new Param3< float, float, float >( m_CurrFog, 0, 0 ), true );

			return true;
		}
		else if ( w == m_SldWindForce )
		{
			UpdateSliderWindForce();

			m_CurrWindForce = m_SldWindForce.GetCurrent() * 0.01;

			//GetRPCManager().SendRPC( "COT_Weather", "Weather_SetWindFunctionParams", new Param3< float, float, float >( m_OrigWindForce, m_CurrWindForce, 1 ), true );

			return true;
		}

		return false;
	}

	override void OnShow()
	{
		super.OnShow();

		RefreshFields();
	}

	override void OnHide()
	{
	}

	override void Update()
	{
	}

	void ToggleAutoRefresh()
	{
		if ( !m_AutoRefresh.IsChecked() )
		{
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(RefreshFields);
		} else {
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(RefreshFields, 1000, true);
		}
	}

	void Save()
	{
		m_OrigYear = m_CurrYear;
		m_OrigMonth = m_CurrMonth;
		m_OrigDay = m_CurrDay;
		m_OrigHour = m_CurrHour;
		m_OrigMinute = m_CurrMinute;
		m_OrigOvercast = m_CurrOvercast;
		m_OrigRain = m_CurrRain;
		m_OrigSnow = m_CurrSnow;
		m_OrigFog = m_CurrFog;
		m_OrigWindForce = m_CurrWindForce;
		
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetSnow", new Param3< float, float, float >( m_CurrSnow, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetRain", new Param3< float, float, float >( m_CurrRain, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetStorm", new Param3< float, float, float >( m_CurrRain * m_CurrRain, 0.8, 4000 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetFog", new Param3< float, float, float >( m_CurrFog, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetOvercast", new Param3< float, float, float >( m_CurrOvercast, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetDate", new Param5< int, int, int, int, int >( m_CurrYear, m_CurrMonth, m_CurrDay, m_CurrHour, m_CurrMinute ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetWindFunctionParams", new Param3< float, float, float >( m_OrigWindForce, m_CurrWindForce, 1 ), true );

		//! not possible rn. Maybe Adam could make it possible without me modding it :think:
		//GetRPCManager().SendRPC( "COT_Weather", "Weather_SetTemperature", new Param1< float >( Temperature ), true );
	}

	void PresetNight()
	{
		m_CurrHour = 0;
		m_CurrMinute = 0;
			
		m_OrigHour = m_CurrHour;
		m_OrigMinute = m_CurrMinute;

		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetDate", new Param5< int, int, int, int, int >( m_CurrYear, m_CurrMonth, m_CurrDay, m_CurrHour, m_CurrMinute ), true );
	}

	void PresetDusk()
	{
		m_CurrHour = 6;
		m_CurrMinute = 0;
			
		m_OrigHour = m_CurrHour;
		m_OrigMinute = m_CurrMinute;

		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetDate", new Param5< int, int, int, int, int >( m_CurrYear, m_CurrMonth, m_CurrDay, m_CurrHour, m_CurrMinute ), true );
	}

	void PresetDay()
	{
		m_CurrHour = 12;
		m_CurrMinute = 0;
			
		m_OrigHour = m_CurrHour;
		m_OrigMinute = m_CurrMinute;

		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetDate", new Param5< int, int, int, int, int >( m_CurrYear, m_CurrMonth, m_CurrDay, m_CurrHour, m_CurrMinute ), true );
	}

	void PresetDawn()
	{
		m_CurrHour = 18;
		m_CurrMinute = 0;
			
		m_OrigHour = m_CurrHour;
		m_OrigMinute = m_CurrMinute;

		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetDate", new Param5< int, int, int, int, int >( m_CurrYear, m_CurrMonth, m_CurrDay, m_CurrHour, m_CurrMinute ), true );
	}

	void PresetClearSky()
	{
		m_CurrOvercast = 0;
		m_CurrRain = 0;
		m_CurrSnow = 0;
		m_CurrFog = 0;
		m_CurrWindForce = 0;

		m_OrigOvercast = m_CurrOvercast;
		m_OrigRain = m_CurrRain;
		m_OrigSnow = m_CurrSnow;
		m_OrigFog = m_CurrFog;
		m_OrigWindForce = m_CurrWindForce;
		
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetSnow", new Param3< float, float, float >( m_CurrSnow, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetRain", new Param3< float, float, float >( m_CurrRain, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetStorm", new Param3< float, float, float >( m_CurrRain * m_CurrRain, 0.8, 4000 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetFog", new Param3< float, float, float >( m_CurrFog, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetOvercast", new Param3< float, float, float >( m_CurrOvercast, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetWindFunctionParams", new Param3< float, float, float >( m_OrigWindForce, m_CurrWindForce, 1 ), true );

	}

	void PresetCloudy()
	{
		m_CurrOvercast = 0.5;
		m_CurrRain = 0;
		m_CurrSnow = 0;
		m_CurrFog = 0.1;
		m_CurrWindForce = 0.2;

		m_OrigOvercast = m_CurrOvercast;
		m_OrigSnow = m_CurrSnow;
		m_OrigRain = m_CurrRain;
		m_OrigFog = m_CurrFog;
		m_OrigWindForce = m_CurrWindForce;
		
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetSnow", new Param3< float, float, float >( m_CurrSnow, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetRain", new Param3< float, float, float >( m_CurrRain, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetStorm", new Param3< float, float, float >( m_CurrRain * m_CurrRain, 0.8, 4000 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetFog", new Param3< float, float, float >( m_CurrFog, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetOvercast", new Param3< float, float, float >( m_CurrOvercast, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetWindFunctionParams", new Param3< float, float, float >( m_OrigWindForce, m_CurrWindForce, 1 ), true );
	}

	void PresetStorm()
	{
		m_CurrOvercast = 1;
		m_CurrRain = 0.8;
		m_CurrSnow = 0.0;
		m_CurrFog = 0.3;
		m_CurrWindForce = 0.7;

		m_OrigOvercast = m_CurrOvercast;
		m_OrigSnow = m_CurrSnow;
		m_OrigRain = m_CurrRain;
		m_OrigFog = m_CurrFog;
		m_OrigWindForce = m_CurrWindForce;
		
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetSnow", new Param3< float, float, float >( m_CurrSnow, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetRain", new Param3< float, float, float >( m_CurrRain, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetStorm", new Param3< float, float, float >( m_CurrRain * m_CurrRain, 0.8, 4000 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetFog", new Param3< float, float, float >( m_CurrFog, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetOvercast", new Param3< float, float, float >( m_CurrOvercast, 0, 0 ), true );
		GetRPCManager().SendRPC( "COT_Weather", "Weather_SetWindFunctionParams", new Param3< float, float, float >( m_OrigWindForce*0.8, m_CurrWindForce, 1 ), true );
	}

	void RefreshFields()
	{
		GetGame().GetWorld().GetDate( m_OrigYear, m_OrigMonth, m_OrigDay, m_OrigHour, m_OrigMinute );

		Weather weather = GetGame().GetWeather();

		m_OrigOvercast = weather.GetOvercast().GetActual();
		m_OrigRain = weather.GetRain().GetActual();
		m_OrigSnow = weather.GetSnowfall().GetActual();
		m_OrigFog = weather.GetFog().GetActual();
		m_OrigWindForce = weather.GetWindSpeed();

		if ( GetGame().GetMission().GetWorldData() )
			m_OrigTemperature = GetGame().GetMission().GetWorldData().GetBaseEnvTemperature();

		m_CurrYear = m_OrigYear;
		m_CurrMonth = m_OrigMonth;
		m_CurrDay = m_OrigDay;
		m_CurrHour = m_OrigHour;
		m_CurrMinute = m_OrigMinute;
		m_CurrOvercast = m_OrigOvercast;
		m_CurrRain = m_OrigRain;
		m_CurrSnow = m_OrigSnow;
		m_CurrFog = m_OrigFog;
		m_CurrWindForce = m_OrigWindForce;
		m_CurrTemperature = m_OrigTemperature;

		ResetSliders();
	}

	void ResetSliders()
	{
		int year, month, day, hour, minute;

		GetGame().GetWorld().GetDate( year, month, day, hour, minute );

		m_SldStartTime.SetCurrent( ((hour * 60) + minute) / 14.39 );

		UpdateSliderStartTime( hour, minute );

		float start_day = day;

		int month_tmp = month;

		while ( month_tmp > 1 )
		{
			month_tmp--;
			start_day += m_DaysInMonth[month];
		}

		m_SldStartDay.SetCurrent( start_day / 3.64 );

		UpdateSliderStartDay( month, day );

		Weather weather = GetGame().GetWeather();

		m_SldOvercast.SetCurrent(weather.GetOvercast().GetActual() * 100);
		UpdateSliderOvercast();

		m_SldRain.SetCurrent(weather.GetRain().GetActual() * 100);
		UpdateSliderRain();

		m_SldSnow.SetCurrent(weather.GetSnowfall().GetActual() * 100);
		UpdateSliderSnow();

		m_SldFog.SetCurrent(weather.GetFog().GetActual() * 100);
		UpdateSliderFog();

		float slider_wind_value = ( weather.GetWindSpeed() / weather.GetWindMaximumSpeed() ) * 100;

		if( weather.GetWindSpeed() < 0.01 )
		{
			slider_wind_value = 0.0;
		}

		m_SldWindForce.SetCurrent( slider_wind_value );

		UpdateSliderWindForce();

		//m_SldTemperature.SetCurrent();

		UpdateSliderTemperature();
	}

	void UpdateSliderStartTime( int hour, int minute )
	{
		string label_text = hour.ToStringLen(2) + ":" + minute.ToStringLen(2);
		m_TxtStartTimeValue.SetText( label_text );
	}

	void UpdateSliderStartDay( int month, int day )
	{
		string label_text = day.ToString() + "." + month.ToString() + ". " + m_CurrYear.ToString();
		m_TxtStartDayValue.SetText( label_text );
	}

	void UpdateSliderOvercast()
	{
		string label_text = m_SldOvercast.GetCurrent().ToString() + "%";
		m_TxtOvercastValue.SetText( label_text );
	}

	void UpdateSliderRain()
	{
		string label_text = m_SldRain.GetCurrent().ToString() + "%";
		m_TxtRainValue.SetText( label_text );
	}

	void UpdateSliderSnow()
	{
		string label_text = m_SldSnow.GetCurrent().ToString() + "%";
		m_TxtSnowValue.SetText( label_text );
	}

	void UpdateSliderFog()
	{
		string label_text = m_SldFog.GetCurrent().ToString() + "%";
		m_TxtFogValue.SetText( label_text );
	}

	void UpdateSliderWindForce()
	{
		string label_text = m_SldWindForce.GetCurrent().ToString() + "%";
		m_TxtWindForceValue.SetText( label_text );
	}

	void UpdateSliderTemperature()
	{
		string label_text = m_CurrTemperature.ToString() + "°C";
		m_TxtTemperatureValue.SetText( label_text );
	}
};
