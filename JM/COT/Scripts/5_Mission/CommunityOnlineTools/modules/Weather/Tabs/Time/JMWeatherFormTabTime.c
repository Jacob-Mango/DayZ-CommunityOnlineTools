//! "Time" tab of JMWeatherForm - the in-game date/time-of-day card.
//! Back-reference to the owning form, same shape as JMPlayerRowWidget.Menu.
//! See JMWeatherFormTabSky.c's header for why Apply/dirty-tracking/preview
//! mechanics stay on the form.
class JMWeatherFormTabTime: JMFormTab
{
	protected JMWeatherForm m_Form;
	protected UIActionScroller m_ScrollerTime;
	protected UIActionSpinner    m_SpinnerYear;
	protected UIActionSpinner    m_SpinnerMonth;
	protected UIActionSpinner    m_SpinnerDay;
	protected UIActionTimePicker m_TimeOfDay;

	//! Days in each month of a non-leap year, for the day spinner's upper bound.
	static const int m_DaysInMonth [ 12 ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	void JMWeatherFormTabTime( JMWeatherForm form )
	{
		m_Form = form;
	}

	void SetTimeValues( JMWeatherPreset preset )
	{
		if ( preset.PDate.Year == -1 )
			return;

		if ( m_SpinnerYear )
			m_SpinnerYear.SetValue( preset.PDate.Year );

		if ( m_SpinnerMonth )
			m_SpinnerMonth.SetValue( preset.PDate.Month );

		if ( m_SpinnerDay )
		{
			ClampDayToMonth( preset.PDate.Month );
			m_SpinnerDay.SetValue( preset.PDate.Day );
		}

		if ( m_TimeOfDay )
			m_TimeOfDay.SetTotalSeconds( ( preset.PDate.Hour * 3600 ) + ( preset.PDate.Minute * 60 ) );
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		m_ScrollerTime = UIActionManager.CreateScroller( panel );
		Widget content = m_ScrollerTime.GetContentWidget();

		UIActionCard dateCard = UIActionManager.CreateCard( content, "#STR_COT_GENERIC_DATE" );
		dateCard.AddApplyButton( this, "OnClick_ApplyTime" );
		Widget dateBody = UIActionManager.CreateGridSpacer( dateCard.GetContent(), 2, 1 );

		//! Year, month and day are one date, so they read as one row.
		Widget dateRow = UIActionManager.CreateGridSpacer( dateBody, 1, 3 );

		m_SpinnerYear = UIActionManager.CreateSpinner( dateRow, "#STR_COT_GENERIC_DATE_YEAR", 1900, 2999, 1, this, "OnChange_Time" );
		m_SpinnerYear.SetIntegerOnly( true );
		m_SpinnerYear.SetValue( 1985 );

		m_SpinnerMonth = UIActionManager.CreateSpinner( dateRow, "#STR_COT_GENERIC_DATE_MONTH", 1, 12, 1, this, "OnChange_Month" );
		m_SpinnerMonth.SetIntegerOnly( true );
		m_SpinnerMonth.SetValue( 1 );

		m_SpinnerDay = UIActionManager.CreateSpinner( dateRow, "#STR_COT_GENERIC_DATE_DAY", 1, 31, 1, this, "OnChange_Time" );
		m_SpinnerDay.SetIntegerOnly( true );
		m_SpinnerDay.SetValue( 1 );

		//! Hour and minute as one field rather than two sliders - the pair is a
		//! single time of day, and a slider is a poor way to hit 08:42.
		m_TimeOfDay = UIActionManager.CreateTimePicker( dateBody, "#STR_COT_WEATHER_MODULE_START_TIME", this, "OnChange_Time" );
		m_TimeOfDay.SetMaxHours( 23 );

		m_ScrollerTime.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_ScrollerTime )
			m_ScrollerTime.UpdateScroller();
	}

	void ReadInto( JMWeatherPreset preset )
	{
		if ( m_SpinnerYear )  preset.PDate.Year  = m_SpinnerYear.GetValueInt();
		if ( m_SpinnerMonth ) preset.PDate.Month = m_SpinnerMonth.GetValueInt();
		if ( m_SpinnerDay )   preset.PDate.Day   = m_SpinnerDay.GetValueInt();

		if ( m_TimeOfDay )
		{
			preset.PDate.Hour   = m_TimeOfDay.GetHours();
			preset.PDate.Minute = m_TimeOfDay.GetMinutes();
		}
	}

	protected void ClampDayToMonth( int month )
	{
		if ( !m_SpinnerDay )
			return;

		if ( month < 1 || month > 12 )
			return;

		m_SpinnerDay.SetMinMax( 1, m_DaysInMonth[month - 1] );
	}

	void OnChange_Time( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_TIME );
	}

	void OnChange_Month( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_TIME );

		if ( m_SpinnerMonth )
			ClampDayToMonth( m_SpinnerMonth.GetValueInt() );
	}

	void OnClick_ApplyTime( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module )
			return;
		if ( !JMPermissions.Has( JMConstants.PERM_WEATHER_DATE ) )
			return;

		action.AnimateFeedback();
		ApplyTime();
	}

	void ApplyTime()
	{
		if ( m_Form.PreviewSection( JMWeatherForm.SECTION_TIME ) )
			return;

		if ( !m_SpinnerYear || !m_SpinnerMonth || !m_SpinnerDay || !m_TimeOfDay )
			return;

		ClampDayToMonth( m_SpinnerMonth.GetValueInt() );

		m_Form.m_Module.SetDate( m_SpinnerYear.GetValueInt(), m_SpinnerMonth.GetValueInt(), m_SpinnerDay.GetValueInt(), m_TimeOfDay.GetHours(), m_TimeOfDay.GetMinutes() );

		m_Form.SetSectionDirty( JMWeatherForm.SECTION_TIME, false );
	}

	void UpdateStates()
	{
		m_Form.UpdateActionState( m_SpinnerYear, JMConstants.PERM_WEATHER_DATE );
		m_Form.UpdateActionState( m_SpinnerMonth, JMConstants.PERM_WEATHER_DATE );
		m_Form.UpdateActionState( m_SpinnerDay, JMConstants.PERM_WEATHER_DATE );
		m_Form.UpdateActionState( m_TimeOfDay, JMConstants.PERM_WEATHER_DATE );
	}

	//! HH:MM, shared by the Overview readout and the preset preview.
	static string FormatClock( int hour, int minute )
	{
		string h = hour.ToString();
		if ( hour < 10 )
			h = "0" + h;

		string m = minute.ToString();
		if ( minute < 10 )
			m = "0" + m;

		return h + ":" + m;
	}
}
