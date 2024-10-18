class JMWeatherForm: JMFormBase
{
	private static const int m_DaysInMonth [ 12 ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	private TextWidget m_TextPresetCount;
	private autoptr array< JMWeatherPresetWidget > m_WidgetsPreset;

	private Widget m_PanelLeft;
	private Widget m_PanelRight;

	private UIActionButton m_ButtonList;
	private UIActionButton m_ButtonRefresh;
	private UIActionButton m_ButtonApply;
	private UIActionCheckbox m_Checkbox_AutoRefresh;
	private UIActionCheckbox m_Checkbox_EasyMode;	

	private Widget m_PanelPresetManageActions;

	private UIActionEditableText m_EditTextPresetName;
	private UIActionEditableText m_EditTextPresetPermission;

	private UIActionButton m_ButtonPresetUpdate;
	private UIActionButton m_ButtonPresetRemove;
	private UIActionButton m_ButtonPresetUse;

	private Widget m_PanelWeatherActions;

	//! Quick Actions stuff !
	private UIActionButton m_BtnQuickActionClear;
	private UIActionButton m_BtnQuickActionCloudy;
	private UIActionButton m_BtnQuickActionRainy;
	private UIActionButton m_BtnQuickActionSnowy;
	private UIActionButton m_BtnQuickActionStorm;

	private UIActionButton m_BtnQuickActionNight;
	private UIActionButton m_BtnQuickActionDusk;
	private UIActionButton m_BtnQuickActionDay;
	private UIActionButton m_BtnQuickActionDawn;

	private GridSpacerWidget m_PanelQuickActions;
	private GridSpacerWidget m_PanelToggles;
	private GridSpacerWidget m_PanelDate;
	private GridSpacerWidget m_PanelStorm;
	private GridSpacerWidget m_PanelOvercast;
	private GridSpacerWidget m_PanelRain;
	private GridSpacerWidget m_PanelRainThreshold;
	private GridSpacerWidget m_PanelSnow;
	private GridSpacerWidget m_PanelSnowThreshold;
	private GridSpacerWidget m_PanelFog;
	private GridSpacerWidget m_PanelFogDynamic;
	private GridSpacerWidget m_PanelWindMagnitude;
	private GridSpacerWidget m_PanelWindDirection;
	private GridSpacerWidget m_PanelWind;
	private GridSpacerWidget m_PanelWindFunc;

	private UIActionButtonToggle m_ToggleQuickActions;
	private UIActionButtonToggle m_ToggleDate;
	private UIActionButtonToggle m_ToggleStorm;
	private UIActionButtonToggle m_ToggleOvercast;
	private UIActionButtonToggle m_ToggleRain;
	private UIActionButtonToggle m_ToggleSnow;
	private UIActionButtonToggle m_ToggleFog;
	private UIActionButtonToggle m_ToggleWind;

	//! Date stuff !
	private UIActionEditableText m_EditTextDateYear;
	private UIActionSlider m_SliderDateMonth;
	private UIActionSlider m_SliderDateDay;
	private UIActionSlider m_SliderDateHour;
	private UIActionSlider m_SliderDateMinute;

	//! Storm stuff !
	private UIActionSlider m_SliderStormDensity;
	private UIActionSlider m_SliderStormThreshold;
	private UIActionSlider m_SliderMinTimeBetweenLightning;

	//! Fog stuff !
	private UIActionSlider m_SliderFogForecast;
	private UIActionEditableText m_EditFogInterpTime;
	private UIActionEditableText m_EditFogMinDuration;

	//! Dynamic Fog stuff !
	private UIActionSlider m_SliderDyanmicFogDistance;
	private UIActionSlider m_SliderDyanmicFogHeight;
	private UIActionSlider m_SliderDyanmicFogBias;
	private UIActionEditableText m_EditDynamicFogInterpTime

	//! Rain stuff !
	private UIActionSlider m_SliderRainForecast;
	private UIActionEditableText m_EditRainInterpTime;
	private UIActionEditableText m_EditRainMinDuration;

	private UIActionSlider m_SliderRainOvercastMin;
	private UIActionSlider m_SliderRainOvercastMax;
	private UIActionEditableText m_EditTextRainTransitionTime;

	//! Snow stuff !
	private UIActionSlider m_SliderSnowForecast;
	private UIActionEditableText m_EditSnowInterpTime;
	private UIActionEditableText m_EditSnowMinDuration;

	private UIActionSlider m_SliderSnowOvercastMin;
	private UIActionSlider m_SliderSnowOvercastMax;
	private UIActionEditableText m_EditTextSnowTransitionTime;

	//! Overcast stuff !
	private UIActionSlider m_SliderOvercastForecast;
	private UIActionEditableText m_EditOvercastInterpTime;
	private UIActionEditableText m_EditOvercastMinDuration;

	//! Wind stuff !
	private UIActionSlider m_SliderWindMagnitudeForecast;
	private UIActionEditableText m_EditWindMagnitudeInterpTime;
	private UIActionEditableText m_EditWindMagnitudeMinDuration;
	
	private UIActionSlider m_SliderWindDirectionForecast;
	private UIActionEditableText m_EditWindDirectionInterpTime;
	private UIActionEditableText m_EditWindDirectionMinDuration;

	private UIActionSlider m_EditWindFuncMin;
	private UIActionSlider m_EditWindFuncMax;
	private UIActionEditableText m_EditWindFuncChangeFreq;

	//! Preset stuff !
	private bool m_PresetsShown;
	private bool m_IsCreatingPreset;
	private string m_SelectedPreset;
	private string m_RemovePreset;

	private JMWeatherModule m_Module;

	void JMWeatherForm()
	{
		m_WidgetsPreset = new array< JMWeatherPresetWidget >;
	}

	void ~JMWeatherForm()
	{
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_PanelLeft = layoutRoot.FindAnyWidget( "panel_left" );
		m_PanelRight = layoutRoot.FindAnyWidget( "panel_right" );

		Widget tParent = layoutRoot.FindAnyWidget( "actions_wrapper" );
		Widget spacer = UIActionManager.CreateGridSpacer( tParent, 1, 3 );
		m_ButtonList = UIActionManager.CreateButton( spacer, "Don't Use Presets", this, "OnClick_List" );
		m_ButtonList.SetWidth( 0.24 );

		m_ButtonApply = UIActionManager.CreateButton( spacer, "Apply", this, "OnClick_Apply" );
		m_ButtonApply.SetWidth( 0.24 );

		m_ButtonRefresh = UIActionManager.CreateButton( spacer, "Refresh Data", this, "OnClick_Refresh" );
		m_ButtonRefresh.SetWidth( 0.24 );

		spacer = UIActionManager.CreateGridSpacer( tParent, 1, 3 );
		m_Checkbox_EasyMode = UIActionManager.CreateCheckbox( spacer, "Easy Mode", this, "OnClick_EasyMode" );
		m_Checkbox_AutoRefresh = UIActionManager.CreateCheckbox( spacer, "Auto Refresh" );

		InitLeftPanel( m_PanelLeft );
		InitRightPanel( m_PanelRight );

		HideList();
	}

	override void Update() 
	{
		if (m_Checkbox_AutoRefresh.IsChecked())
			RefreshValues(true);
	}

	override void OnShow()
	{
		super.OnShow();

		if (m_Module.IsLoaded())
			OnSettingsUpdated();
		else
			m_Module.Load();

		UpdateStates();
	}

	override void OnSettingsUpdated()
	{
		CF_Trace_0(this);

		if (!m_Module.IsLoaded())
			return;

		UpdatePresetList();
	}

	void CreateNew()
	{
		m_IsCreatingPreset = true;
		
		UpdateStates();
	}

	void SetSelectedPreset( string preset )
	{
		m_SelectedPreset = preset;

		UpdateStates();
	}

	void RemovePreset( string preset )
	{
		m_RemovePreset = preset;

		CreateConfirmation_Two( JMConfirmationType.INFO, "Confirm", "Do you wish to remove the preset " + preset, "No", "", "Yes", "RemovePreset_Yes" );
	}

	void RemovePreset_Yes( JMConfirmation confirmation )
	{
		if ( m_SelectedPreset == m_RemovePreset )
		{
			m_SelectedPreset = "";
		}

		m_Module.RemovePreset( m_RemovePreset );

		UpdateStates();
	}

	void UpdatePresetList()
	{
		if ( !m_Module.HasSettings() )
			return;

		array< ref JMWeatherPreset > presets = m_Module.GetPresets();

		for ( int i = 0; i < m_WidgetsPreset.Count(); i++ )
		{
			if ( i < presets.Count() )
			{
				m_WidgetsPreset[i].SetPreset( presets[i].Name );
			} else if ( i == presets.Count() )
			{
				m_WidgetsPreset[i].SetCreateNew();
			} else
			{
				m_WidgetsPreset[i].SetPreset( "" );
			}
		}

		m_TextPresetCount.SetText( "" + presets.Count() + " Total" );
	}

	private void SetWidthPos( Widget widget, float width, float xPos )
	{
		float w;
		float h;
		widget.GetSize( w, h );
		widget.SetSize( width, h );

		float x;
		float y;
		widget.GetPos( x, y );
		widget.SetPos( xPos, y );
	}

	void ShowList()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::ShowList" );
		#endif

		m_PanelLeft.Show( true );
		m_PanelRight.Show( true );

		SetWidthPos( m_PanelLeft, 0.25, 0 );
		SetWidthPos( m_PanelRight, 0.75, 0.25 );

		m_ButtonList.SetButton( "Don't Use Presets" );

		m_PresetsShown = true;

		UpdateStates();

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::ShowList" );
		#endif
	}

	void HideList()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::HideList" );
		#endif

		m_PanelLeft.Show( false );
		m_PanelRight.Show( true );

		SetWidthPos( m_PanelLeft, 0.0, 0.0 );
		SetWidthPos( m_PanelRight, 1.0, 0.0 );

		m_ButtonList.SetButton( "Use Presets" );

		m_PresetsShown = false;

		UpdateStates();

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::HideList" );
		#endif
	}

	private void UpdateActionState( UIActionBase action, string permission, bool shouldDisable = false, bool shouldHide = false )
	{
		if (action == NULL)
			return;
		
		bool disable = false;
		if ( permission != "" )
		{
			disable = !GetPermissionsManager().HasPermission( permission );
		}
		
		if ( disable || shouldDisable )
		{
			action.Disable();
		} else
		{
			action.Enable();
		}

		if ( shouldHide )
		{
			action.Hide();
		} else
		{
			action.Show();
		}
	}

	void UpdateStates()
	{
		bool hasNotSelectedPreset = false;
		bool easyModeEnabled = m_Checkbox_EasyMode.IsChecked();

		if ( m_PresetsShown )
		{
			hasNotSelectedPreset = m_SelectedPreset == "";
		}

		m_PanelToggles.Show(!m_PresetsShown);
		m_PanelQuickActions.Show(!m_PresetsShown);
		
		ToggleMenus();
		
		UpdateActionState( m_ButtonList, "Weather.Preset", easyModeEnabled );
		
		UpdateActionState( m_ButtonRefresh, "Weather", hasNotSelectedPreset );
		UpdateActionState( m_Checkbox_EasyMode, "Weather", hasNotSelectedPreset );
		UpdateActionState( m_Checkbox_AutoRefresh, "Weather", hasNotSelectedPreset );

		UpdateActionState( m_BtnQuickActionClear, "Weather.QuickAction.Clear", hasNotSelectedPreset );
		UpdateActionState( m_BtnQuickActionCloudy, "Weather.QuickAction.Overcast", hasNotSelectedPreset );
		UpdateActionState( m_BtnQuickActionRainy, "Weather.QuickAction.Rain", hasNotSelectedPreset );
		UpdateActionState( m_BtnQuickActionSnowy, "Weather.QuickAction.Snow", hasNotSelectedPreset );
		UpdateActionState( m_BtnQuickActionStorm, "Weather.QuickAction.Storm", hasNotSelectedPreset );

		UpdateActionState( m_BtnQuickActionNight, "Weather.QuickAction.Date", hasNotSelectedPreset );
		UpdateActionState( m_BtnQuickActionDusk, "Weather.QuickAction.Date", hasNotSelectedPreset );
		UpdateActionState( m_BtnQuickActionDay, "Weather.QuickAction.Date", hasNotSelectedPreset );
		UpdateActionState( m_BtnQuickActionDawn, "Weather.QuickAction.Date", hasNotSelectedPreset );

		UpdateActionState( m_EditTextDateYear, "Weather.Date", hasNotSelectedPreset, easyModeEnabled );
		UpdateActionState( m_SliderDateMonth, "Weather.Date", hasNotSelectedPreset, easyModeEnabled );
		UpdateActionState( m_SliderDateDay, "Weather.Date", hasNotSelectedPreset, easyModeEnabled );
		UpdateActionState( m_SliderDateHour, "Weather.Date", hasNotSelectedPreset );
		UpdateActionState( m_SliderDateMinute, "Weather.Date", hasNotSelectedPreset );
		
		UpdateActionState( m_SliderStormDensity, "Weather.Storm", hasNotSelectedPreset );
		UpdateActionState( m_SliderStormThreshold, "Weather.Storm", hasNotSelectedPreset, easyModeEnabled );
		UpdateActionState( m_SliderMinTimeBetweenLightning, "Weather.Storm", hasNotSelectedPreset, easyModeEnabled );

		UpdateActionState( m_SliderFogForecast, "Weather.Fog", hasNotSelectedPreset );
		UpdateActionState( m_EditFogInterpTime, "Weather.Fog", hasNotSelectedPreset, easyModeEnabled );
		UpdateActionState( m_EditFogMinDuration, "Weather.Fog", hasNotSelectedPreset, easyModeEnabled);
		UpdateActionState( m_SliderDyanmicFogBias, "Weather.Fog.Dynamic", hasNotSelectedPreset);
		UpdateActionState( m_SliderDyanmicFogDistance, "Weather.Fog.Dynamic", hasNotSelectedPreset, easyModeEnabled );
		UpdateActionState( m_SliderDyanmicFogHeight, "Weather.Fog.Dynamic", hasNotSelectedPreset, easyModeEnabled );
		UpdateActionState( m_EditDynamicFogInterpTime, "Weather.Fog.Dynamic", hasNotSelectedPreset, easyModeEnabled );
		UpdateActionState( m_SliderRainForecast, "Weather.Rain", hasNotSelectedPreset );
		UpdateActionState( m_EditRainInterpTime, "Weather.Rain", hasNotSelectedPreset, easyModeEnabled );
		UpdateActionState( m_EditRainMinDuration, "Weather.Rain", hasNotSelectedPreset, easyModeEnabled );

		UpdateActionState( m_SliderRainOvercastMin, "Weather.Rain.Thresholds", hasNotSelectedPreset );
		UpdateActionState( m_SliderRainOvercastMax, "Weather.Rain.Thresholds", hasNotSelectedPreset );
		UpdateActionState( m_EditTextRainTransitionTime, "Weather.Rain.Thresholds", hasNotSelectedPreset, easyModeEnabled );

		UpdateActionState( m_SliderSnowForecast, "Weather.Snow", hasNotSelectedPreset );
		UpdateActionState( m_EditSnowInterpTime, "Weather.Snow", hasNotSelectedPreset, easyModeEnabled );
		UpdateActionState( m_EditSnowMinDuration, "Weather.Snow", hasNotSelectedPreset, easyModeEnabled );

		UpdateActionState( m_SliderSnowOvercastMin, "Weather.Snow.Thresholds", hasNotSelectedPreset );
		UpdateActionState( m_SliderSnowOvercastMax, "Weather.Snow.Thresholds", hasNotSelectedPreset );
		UpdateActionState( m_EditTextSnowTransitionTime, "Weather.Snow.Thresholds", hasNotSelectedPreset, easyModeEnabled );

		UpdateActionState( m_SliderOvercastForecast, "Weather.Overcast", hasNotSelectedPreset );
		UpdateActionState( m_EditOvercastInterpTime, "Weather.Overcast", hasNotSelectedPreset, easyModeEnabled );
		UpdateActionState( m_EditOvercastMinDuration, "Weather.Overcast", hasNotSelectedPreset, easyModeEnabled );

		UpdateActionState( m_SliderWindMagnitudeForecast, "Weather.Wind", hasNotSelectedPreset );
		UpdateActionState( m_EditWindMagnitudeInterpTime, "Weather.Wind", hasNotSelectedPreset, easyModeEnabled );
		UpdateActionState( m_EditWindMagnitudeMinDuration, "Weather.Wind", hasNotSelectedPreset, easyModeEnabled );
		
		UpdateActionState( m_SliderWindDirectionForecast, "Weather.Wind", hasNotSelectedPreset );
		UpdateActionState( m_EditWindDirectionInterpTime, "Weather.Wind", hasNotSelectedPreset, easyModeEnabled );
		UpdateActionState( m_EditWindDirectionMinDuration, "Weather.Wind", hasNotSelectedPreset, easyModeEnabled );

		UpdateActionState( m_EditWindFuncMin, "Weather.Wind.FunctionParams", hasNotSelectedPreset, easyModeEnabled );
		UpdateActionState( m_EditWindFuncMax, "Weather.Wind.FunctionParams", hasNotSelectedPreset, easyModeEnabled );
		UpdateActionState( m_EditWindFuncChangeFreq, "Weather.Wind.FunctionParams", hasNotSelectedPreset, easyModeEnabled );

		if ( hasNotSelectedPreset )
		{
			UpdateActionState( m_EditTextPresetName, "Weather.Preset.Create", !m_IsCreatingPreset, !m_PresetsShown );
			UpdateActionState( m_EditTextPresetPermission, "Weather.Preset.Create", !m_IsCreatingPreset, !m_PresetsShown );
			UpdateActionState( m_ButtonPresetUpdate, "Weather.Preset.Create", !m_IsCreatingPreset, !m_PresetsShown );

			if ( m_IsCreatingPreset )
			{
				UpdateActionState( m_ButtonPresetRemove, "Weather.Preset.Remove", hasNotSelectedPreset, true );
				UpdateActionState( m_ButtonPresetUse, "Weather.Preset", hasNotSelectedPreset, true );
			} else
			{
				UpdateActionState( m_ButtonPresetRemove, "Weather.Preset.Remove", hasNotSelectedPreset, !m_PresetsShown );
				UpdateActionState( m_ButtonPresetUse, "Weather.Preset", hasNotSelectedPreset, !m_PresetsShown );
			}
		} else
		{
			UpdateActionState( m_EditTextPresetName, "Weather.Preset.Create", hasNotSelectedPreset, !m_PresetsShown );
			UpdateActionState( m_EditTextPresetPermission, "Weather.Preset.Create", hasNotSelectedPreset, !m_PresetsShown );
			UpdateActionState( m_ButtonPresetUpdate, "Weather.Preset.Update", hasNotSelectedPreset, !m_PresetsShown );

			UpdateActionState( m_ButtonPresetRemove, "Weather.Preset.Remove", hasNotSelectedPreset, !m_PresetsShown );
			UpdateActionState( m_ButtonPresetUse, "Weather.Preset", hasNotSelectedPreset, !m_PresetsShown );
		}

		JMWeatherPreset preset;

		if ( m_PresetsShown )
		{
			if ( hasNotSelectedPreset && !m_IsCreatingPreset )
			{
				m_ButtonPresetUpdate.SetButton( "Update Preset" );

				RefreshValues();
			}
			else if ( m_IsCreatingPreset )
			{
				m_ButtonPresetUpdate.SetButton( "Create Preset" );
			}
			else
			{
				m_ButtonPresetUpdate.SetButton( "Update Preset" );

				array< ref JMWeatherPreset > presets = m_Module.GetPresets();

				for ( int i = 0; i < presets.Count(); i++ )
				{
					if ( presets[i].Name == m_SelectedPreset )
					{
						preset = presets[i];
						break;
					}
				}

				if ( preset )
				{
					m_EditTextPresetName.SetText( preset.Name );
					SetUIActionValues( preset );
				}
				else
				{
					m_SelectedPreset = "";
					UpdateStates();
				}
			}
		}
		else
		{
			RefreshValues();
		}
	}

	void RefreshValues(bool actual = false)
	{
		auto preset = new JMWeatherPreset;
		preset.SetFromWorld();
		SetUIActionValues( preset, actual );
	}

	void SetUIActionValues( JMWeatherPreset preset, bool actual = false )
	{
		if ( preset.PDate.Year != -1 )
			m_EditTextDateYear.SetText( preset.PDate.Year );

		if ( preset.PDate.Month != -1 )
			m_SliderDateMonth.SetCurrent( preset.PDate.Month );

		if ( preset.PDate.Day != -1 )
			m_SliderDateDay.SetCurrent( preset.PDate.Day );

		if ( preset.PDate.Hour != -1 )
			m_SliderDateHour.SetCurrent( preset.PDate.Hour );

		if ( preset.PDate.Minute != -1 )
			m_SliderDateMinute.SetCurrent( preset.PDate.Minute );

		if ( preset.Storm.Density != -1 )
			m_SliderStormDensity.SetCurrent( preset.Storm.Density * 100.0 );

		if ( preset.Storm.Threshold != -1 )
			m_SliderStormThreshold.SetCurrent( preset.Storm.Threshold * 100.0 );

		if ( preset.Storm.MinTimeBetweenLightning != -1 )
			m_SliderMinTimeBetweenLightning.SetCurrent( preset.Storm.MinTimeBetweenLightning );

		if (actual)
			m_SliderFogForecast.SetCurrent( preset.PFog.Actual * 100.0 );
		else if ( preset.PFog.Forecast != -1 )
			m_SliderFogForecast.SetCurrent( preset.PFog.Forecast * 100.0 );

		if ( preset.PFog.Time != -1 )
			m_EditFogInterpTime.SetText( preset.PFog.Time );

		if ( preset.PFog.MinDuration != -1 )
			m_EditFogMinDuration.SetText( preset.PFog.MinDuration );

		if ( preset.PDynFog.Distance != -1 )
			m_SliderDyanmicFogDistance.SetCurrent( preset.PDynFog.Distance );
		if ( preset.PDynFog.Height != -1 )
			m_SliderDyanmicFogHeight.SetCurrent( preset.PDynFog.Height );
		if ( preset.PDynFog.Bias != -1 )
			m_SliderDyanmicFogBias.SetCurrent( preset.PDynFog.Bias );
		if ( preset.PDynFog.Time != -1 )
			m_EditDynamicFogInterpTime.SetText( preset.PDynFog.Time );

		if (actual)
			m_SliderRainForecast.SetCurrent( preset.PRain.Actual * 100.0 );
		else if ( preset.PRain.Forecast != -1 )
			m_SliderRainForecast.SetCurrent( preset.PRain.Forecast * 100.0 );

		if ( preset.PRain.Time != -1 )
			m_EditRainInterpTime.SetText( preset.PRain.Time );

		if ( preset.PRain.MinDuration != -1 )
			m_EditRainMinDuration.SetText( preset.PRain.MinDuration );

		if ( preset.RainThreshold.OvercastMin != -1 )
			m_SliderRainOvercastMin.SetCurrent( preset.RainThreshold.OvercastMin * 100.0 );

		if ( preset.RainThreshold.OvercastMax != -1 )
			m_SliderRainOvercastMax.SetCurrent( preset.RainThreshold.OvercastMax * 100.0 );

		if ( preset.RainThreshold.Time != -1 )
			m_EditTextRainTransitionTime.SetText( preset.RainThreshold.Time );

		if (actual)
			m_SliderSnowForecast.SetCurrent( preset.PSnow.Actual * 100.0 );
		else if ( preset.PSnow.Forecast != -1 )
			m_SliderSnowForecast.SetCurrent( preset.PSnow.Forecast * 100.0 );

		if ( preset.PSnow.Time != -1 )
			m_EditSnowInterpTime.SetText( preset.PSnow.Time );

		if ( preset.PSnow.MinDuration != -1 )
			m_EditSnowMinDuration.SetText( preset.PSnow.MinDuration );

		if ( preset.SnowThreshold.OvercastMin != -1 )
			m_SliderSnowOvercastMin.SetCurrent( preset.SnowThreshold.OvercastMin * 100.0 );

		if ( preset.SnowThreshold.OvercastMax != -1 )
			m_SliderSnowOvercastMax.SetCurrent( preset.SnowThreshold.OvercastMax * 100.0 );

		if ( preset.SnowThreshold.Time != -1 )
			m_EditTextSnowTransitionTime.SetText( preset.SnowThreshold.Time );

		if (actual)
			m_SliderOvercastForecast.SetCurrent( preset.POvercast.Actual * 100.0 );
		else if ( preset.POvercast.Forecast != -1 )
			m_SliderOvercastForecast.SetCurrent( preset.POvercast.Forecast * 100.0 );

		if ( preset.POvercast.Time != -1 )
			m_EditOvercastInterpTime.SetText( preset.POvercast.Time );

		if ( preset.POvercast.MinDuration != -1 )
			m_EditOvercastMinDuration.SetText( preset.POvercast.MinDuration );
		
		if (actual)
			m_SliderWindMagnitudeForecast.SetCurrent( preset.PWindMagnitude.Actual );
		else if ( preset.PWindMagnitude.Forecast != -1 )
			m_SliderWindMagnitudeForecast.SetCurrent( preset.PWindMagnitude.Forecast );

		if ( preset.PWindMagnitude.Time != -1 )
			m_EditWindMagnitudeInterpTime.SetText( preset.PWindMagnitude.Time );

		if ( preset.PWindMagnitude.MinDuration != -1 )
			m_EditWindMagnitudeMinDuration.SetText( preset.PWindMagnitude.MinDuration );
		
		float DirRad;
		if (actual)
			DirRad = preset.PWindDirection.Actual;
		else if ( preset.PWindDirection.Forecast != -1 )
			DirRad = preset.PWindDirection.Forecast;
		
		m_SliderWindDirectionForecast.SetCurrent( PI2DEG(DirRad) );
		UpdateWindDirectionSlider();

		if ( preset.PWindDirection.Time != -1 )
			m_EditWindDirectionInterpTime.SetText( preset.PWindDirection.Time );

		if ( preset.PWindDirection.MinDuration != -1 )
			m_EditWindDirectionMinDuration.SetText( preset.PWindDirection.MinDuration );

		if ( preset.WindFunc.Min != -1 )
			m_EditWindFuncMin.SetCurrent( preset.WindFunc.Min );

		if ( preset.WindFunc.Max != -1 )
			m_EditWindFuncMax.SetCurrent( preset.WindFunc.Max );

		if ( preset.WindFunc.Speed != -1 )
			m_EditWindFuncChangeFreq.SetText( preset.WindFunc.Speed );
	}

	void GetUIActionValues( out JMWeatherPreset preset )
	{
		preset.Name = m_EditTextPresetName.GetText();

		preset.PDate.Year = ToFloat( m_EditTextDateYear.GetText() );
		preset.PDate.Month = m_SliderDateMonth.GetCurrent();
		preset.PDate.Day = m_SliderDateDay.GetCurrent();
		preset.PDate.Hour = m_SliderDateHour.GetCurrent();
		preset.PDate.Minute = m_SliderDateMinute.GetCurrent();

		preset.Storm.Density = m_SliderStormDensity.GetCurrent() * 0.01;
		preset.Storm.Threshold = m_SliderStormThreshold.GetCurrent() * 0.01;
		preset.Storm.MinTimeBetweenLightning = m_SliderMinTimeBetweenLightning.GetCurrent();

		preset.PFog.Forecast = m_SliderFogForecast.GetCurrent() * 0.01;
		preset.PFog.Time = ToFloat( m_EditFogInterpTime.GetText() );
		preset.PFog.MinDuration = ToFloat( m_EditFogMinDuration.GetText() );

		preset.PDynFog.Distance = m_SliderDyanmicFogDistance.GetCurrent();
		preset.PDynFog.Height = m_SliderDyanmicFogHeight.GetCurrent();
		preset.PDynFog.Bias = m_SliderDyanmicFogBias.GetCurrent();
		preset.PDynFog.Time = ToFloat(  m_EditDynamicFogInterpTime.GetText() );

		preset.PRain.Forecast = m_SliderRainForecast.GetCurrent() * 0.01;
		preset.PRain.Time = ToFloat( m_EditRainInterpTime.GetText() );
		preset.PRain.MinDuration = ToFloat( m_EditRainMinDuration.GetText() );

		preset.RainThreshold.OvercastMin = m_SliderRainOvercastMin.GetCurrent() * 0.01;
		preset.RainThreshold.OvercastMax = m_SliderRainOvercastMax.GetCurrent() * 0.01;
		preset.RainThreshold.Time = ToFloat( m_EditTextRainTransitionTime.GetText() );

		preset.PSnow.Forecast = m_SliderSnowForecast.GetCurrent() * 0.01;
		preset.PSnow.Time = ToFloat( m_EditSnowInterpTime.GetText() );
		preset.PSnow.MinDuration = ToFloat( m_EditSnowMinDuration.GetText() );

		preset.SnowThreshold.OvercastMin = m_SliderSnowOvercastMin.GetCurrent() * 0.01;
		preset.SnowThreshold.OvercastMax = m_SliderSnowOvercastMax.GetCurrent() * 0.01;
		preset.SnowThreshold.Time = ToFloat( m_EditTextSnowTransitionTime.GetText() );

		preset.POvercast.Forecast = m_SliderOvercastForecast.GetCurrent() * 0.01;
		preset.POvercast.Time = ToFloat( m_EditOvercastInterpTime.GetText() );
		preset.POvercast.MinDuration = ToFloat( m_EditOvercastMinDuration.GetText() );

		preset.PWindMagnitude.Forecast = m_SliderWindMagnitudeForecast.GetCurrent();
		preset.PWindMagnitude.Time = ToFloat( m_EditWindMagnitudeInterpTime.GetText() );
		preset.PWindMagnitude.MinDuration = ToFloat( m_EditWindMagnitudeMinDuration.GetText() );

		preset.PWindDirection.Forecast = DEG2PI(m_SliderWindDirectionForecast.GetCurrent());
		preset.PWindDirection.Time = ToFloat( m_EditWindDirectionInterpTime.GetText() );
		preset.PWindDirection.MinDuration = ToFloat( m_EditWindDirectionMinDuration.GetText() );

		preset.WindFunc.Min = m_EditWindFuncMin.GetCurrent();
		preset.WindFunc.Max = m_EditWindFuncMax.GetCurrent();
		preset.WindFunc.Speed = ToFloat( m_EditWindFuncChangeFreq.GetText() );
	}

	static float PI2DEG(float value)
	{
		return (value * Math.RAD2DEG) + 180;
	}

	static float DEG2PI(float value)
	{
		return (value * Math.DEG2RAD) - Math.PI;
	}

	void InitLeftPanel( Widget parent )
	{
		m_TextPresetCount = TextWidget.Cast( layoutRoot.FindAnyWidget( "count" ) );

		Widget rwWidget = NULL;
		JMWeatherPresetWidget rwScript = NULL;

		//for ( int i = 0; i < 10; i++ )
		{
			//GridSpacerWidget gsw = GridSpacerWidget.Cast( parent.FindAnyWidget( "list_0" + i ) );
			GridSpacerWidget gsw = GridSpacerWidget.Cast( parent.FindAnyWidget( "list_01" ) );
			
			//if ( !gsw )
				//break;

			for ( int j = 0; j < 100; j++ )
			{
				rwWidget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/weather_preset_widget.layout", gsw );
				
				if ( rwWidget == NULL )
				{
					break;
				}

				rwWidget.GetScript( rwScript );

				if ( rwScript == NULL )
				{
					break;
				}

				rwScript.Init( this );

				rwScript.SetPreset( "" );

				m_WidgetsPreset.Insert( rwScript );
			}
		}
	}

	void InitRightPanel( Widget parent )
	{
		Widget tParent = parent.FindAnyWidget( "actions_wrapper" );

		m_PanelPresetManageActions = UIActionManager.CreateGridSpacer( tParent, 3, 1 );
		InitPresetDetails( m_PanelPresetManageActions );
		InitPresetRun( m_PanelPresetManageActions );
		UIActionManager.CreatePanel( m_PanelPresetManageActions, 0xFF000000, 1 );

		m_PanelWeatherActions = UIActionManager.CreateGridSpacer( tParent, 9, 1 );
		
		InitQuickActionsWidgets( m_PanelWeatherActions );
		InitTogglesWidgets( m_PanelWeatherActions );

		InitDateWidgets( m_PanelWeatherActions );

		InitOvercastWidgets( m_PanelWeatherActions );
		InitStormWidgets( m_PanelWeatherActions );

		InitFogWidgets( m_PanelWeatherActions );
		InitDynFogWidgets( m_PanelWeatherActions );

		InitRainWidgets( m_PanelWeatherActions );
		InitRainThresholdWidgets( m_PanelWeatherActions );

		InitSnowWidgets( m_PanelWeatherActions );
		InitSnowThresholdWidgets( m_PanelWeatherActions );

		InitWindMagnitudeWidgets( m_PanelWeatherActions );
		InitWindDirectionWidgets( m_PanelWeatherActions );
		InitWindFuncWidgets( m_PanelWeatherActions );
		
		ToggleMenus();
	}

	private void InitPresetDetails( Widget actionsParent )
	{
		Widget actions = UIActionManager.CreateGridSpacer( actionsParent, 1, 2 );

		m_EditTextPresetName = UIActionManager.CreateEditableText( actions, "Name", this, "OnChange_PresetDetails" );
		m_EditTextPresetName.SetOnlyNumbers( false );
		m_EditTextPresetName.SetText( "" );
		m_EditTextPresetName.SetWidgetWidth( m_EditTextPresetName.GetLabelWidget(), 0.6 );
		m_EditTextPresetName.SetWidgetWidth( m_EditTextPresetName.GetEditBoxWidget(), 0.5 );
	}

	void OnChange_PresetDetails( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_PresetsShown )
			return;
	}

	private void InitPresetRun( Widget actionsParent )
	{
		m_ButtonPresetUse = UIActionManager.CreateButton( actionsParent, "Run Preset", this, "OnClick_PresetRun" );
		m_ButtonPresetUpdate = UIActionManager.CreateButton( actionsParent, "Update Preset", this, "OnClick_PresetUpdate" );
		m_ButtonPresetRemove = UIActionManager.CreateButton( actionsParent, "Remove Preset", this, "OnClick_PresetRemove" );
	}

	void OnClick_List( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		if ( m_PresetsShown )
		{
			HideList();
		} else
		{
			ShowList();
		}
	}
	
	void OnClick_EasyMode( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		UpdateStates();
	}

	void OnClick_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		RefreshValues(true);
	}

	void OnClick_Apply( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_PresetsShown )
			return;
		
		int year = m_EditTextDateYear.GetText().ToInt();
		int month = m_SliderDateMonth.GetCurrent();
		m_SliderDateDay.SetMax( m_DaysInMonth[month - 1] );

		int day = m_SliderDateDay.GetCurrent();
		int hour = m_SliderDateHour.GetCurrent();
		int minute = m_SliderDateMinute.GetCurrent();

		float forecastFog = m_SliderFogForecast.GetCurrent() * 0.01;
		float timeFog = m_EditFogInterpTime.GetText().ToFloat();
		float minDurationFog = m_EditFogMinDuration.GetText().ToFloat();

		float distance = m_SliderDyanmicFogDistance.GetCurrent();
		float height = m_SliderDyanmicFogHeight.GetCurrent();
		float bias = m_SliderDyanmicFogBias.GetCurrent();
		float timeDynFog = m_EditDynamicFogInterpTime.GetText().ToFloat();

		float forecastWindMag = m_SliderWindMagnitudeForecast.GetCurrent();
		float timeWindMag = m_EditWindMagnitudeInterpTime.GetText().ToFloat();
		float minDurationWindMag = m_EditWindMagnitudeMinDuration.GetText().ToFloat();
		
		float forecastWindDir = DEG2PI(m_SliderWindDirectionForecast.GetCurrent());
		float timeWindDir = m_EditWindDirectionInterpTime.GetText().ToFloat();
		float minDurationWindDir = m_EditWindDirectionMinDuration.GetText().ToFloat();

		float minFunc = m_EditWindFuncMin.GetCurrent();
		float maxFunc = m_EditWindFuncMax.GetCurrent();

		float speedFunc = m_EditWindFuncChangeFreq.GetText().ToFloat();

		float forecastOvercast = m_SliderOvercastForecast.GetCurrent() * 0.01;
		float timeOvercast = m_EditOvercastInterpTime.GetText().ToFloat();
		float minDurationOvercast = m_EditOvercastMinDuration.GetText().ToFloat();

		float densityStorm = m_SliderStormDensity.GetCurrent() * 0.01;
		float thresholdStorm = m_SliderStormThreshold.GetCurrent() * 0.01;
		float minTimeBetweenLightning = m_SliderMinTimeBetweenLightning.GetCurrent();

		float forecastRain = m_SliderRainForecast.GetCurrent() * 0.01;
		float timeRain = m_EditRainInterpTime.GetText().ToFloat();
		float minDurationRain = m_EditRainMinDuration.GetText().ToFloat();

		float minRainThresh = m_SliderRainOvercastMin.GetCurrent() * 0.01;
		float maxRainThresh = m_SliderRainOvercastMax.GetCurrent() * 0.01;
		float transitionRainThresh = m_EditTextRainTransitionTime.GetText().ToFloat();

		float forecastSnow = m_SliderSnowForecast.GetCurrent() * 0.01;
		float timeSnow = m_EditSnowInterpTime.GetText().ToFloat();
		float minDurationSnow = m_EditSnowMinDuration.GetText().ToFloat();

		float minSnowThresh = m_SliderSnowOvercastMin.GetCurrent() * 0.01;
		float maxSnowThresh = m_SliderSnowOvercastMax.GetCurrent() * 0.01;
		float transitionSnowThresh = m_EditTextSnowTransitionTime.GetText().ToFloat();

		if ( m_Checkbox_EasyMode.IsChecked() )
		{
			timeFog = 0;
			minDurationFog = 240;

			distance = 0.25;
			height = 0.25;
			timeWindMag = 0;
			minDurationWindMag = 240;
			timeWindDir = 0;
			minDurationWindDir = 240;

			timeOvercast = 0;
			minDurationOvercast = 240;
			thresholdStorm = 0.7;
			minTimeBetweenLightning = 25;
			timeRain = 0;
			minDurationRain = 240;
			minRainThresh = 0;
			maxRainThresh = 1;
			transitionRainThresh = 120;

			timeSnow = 0;
			minDurationSnow = 240;
			minSnowThresh = 0;
			maxSnowThresh = 1;
			transitionSnowThresh = 120;
		}

		if ( m_PanelDate.IsVisible() )
			m_Module.SetDate( year, month, day, hour, minute );
		
		if ( m_PanelFog.IsVisible() )
		{
			m_Module.SetFog( forecastFog, timeFog, minDurationFog );
			m_Module.SetDynamicFog( distance, height, bias, timeDynFog );
		}
		
		if ( m_PanelWindMagnitude.IsVisible() )
		{
			m_Module.SetWindMagnitude( forecastWindMag, timeWindMag, minDurationWindMag );
			m_Module.SetWindDirection( forecastWindDir, timeWindDir, minDurationWindDir );
			m_Module.SetWindFunctionParams( minFunc, maxFunc, speedFunc );
		}
		
		if ( m_PanelOvercast.IsVisible() )
			m_Module.SetOvercast( forecastOvercast, timeOvercast, minDurationOvercast );
		
		if ( m_PanelStorm.IsVisible() )
			m_Module.SetStorm( densityStorm, thresholdStorm, minTimeBetweenLightning );
		
		if ( m_PanelRain.IsVisible() )
		{
			m_Module.SetRain( forecastRain, timeRain, minDurationRain );
			m_Module.SetRainThresholds( minRainThresh, maxRainThresh, transitionRainThresh );
		}
		
		if ( m_PanelSnow.IsVisible() )
		{
			m_Module.SetSnow( forecastSnow, timeSnow, minDurationSnow );
			m_Module.SetSnowThresholds( minSnowThresh, maxSnowThresh, transitionSnowThresh );
		}
	}

	void OnClick_PresetUpdate( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_PresetsShown )
			return;

		JMWeatherPreset preset;

		if ( m_IsCreatingPreset )
		{
			preset = new JMWeatherPreset;

			preset.Name = m_EditTextPresetName.GetText();

			if ( preset.Name == "" )
			{
				CreateConfirmation_One( JMConfirmationType.INFO, "Error", "Name can't be empty", "Ok" );
				return;
			}
		} else
		{
			array< ref JMWeatherPreset > presets = m_Module.GetPresets();

			for ( int i = 0; i < presets.Count(); i++ )
			{
				if ( presets[i].Name == m_SelectedPreset )
				{
					preset = presets[i];
					break;
				}
			}
		}

		if ( preset == NULL )
			return;

		GetUIActionValues( preset );

		if ( m_IsCreatingPreset )
		{
			m_Module.CreatePreset( preset );

			m_IsCreatingPreset = false;

			m_SelectedPreset = preset.Name;
		} else
		{
			m_Module.UpdatePreset( preset );
		}

		UpdateStates();
	}

	void OnClick_PresetRun( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_PresetsShown )
			return;

		m_Module.UsePreset( m_SelectedPreset );
	}

	void OnClick_PresetRemove( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_PresetsShown )
			return;

		RemovePreset( m_SelectedPreset );
	}
	
	private void InitQuickActionsWidgets( Widget actionsParent )
	{
		m_PanelQuickActions = UIActionManager.CreateGridSpacer( actionsParent, 2, 1 );

		UIActionManager.CreateText( m_PanelQuickActions, "Quick Actions: ", "Instantly change the Weather" );
	
		Widget actionsWeather 	= UIActionManager.CreateGridSpacer( m_PanelQuickActions, 1, 5 );

		m_BtnQuickActionClear 	= UIActionManager.CreateButton( actionsWeather, "Clear", this, "OnClick_PresetClear" );
		m_BtnQuickActionCloudy 	= UIActionManager.CreateButton( actionsWeather, "Cloudy", this, "OnClick_PresetCloudy" );
		m_BtnQuickActionRainy 	= UIActionManager.CreateButton( actionsWeather, "Rainy", this, "OnClick_PresetRainy" );
		m_BtnQuickActionSnowy 	= UIActionManager.CreateButton( actionsWeather, "Snowy", this, "OnClick_PresetSnowy" );
		m_BtnQuickActionStorm 	= UIActionManager.CreateButton( actionsWeather, "Storm", this, "OnClick_PresetStorm" );

		Widget actionsDate 		= UIActionManager.CreateGridSpacer( m_PanelQuickActions, 1, 4 );
		m_BtnQuickActionNight 	= UIActionManager.CreateButton( actionsDate, "Night", this, "OnClick_PresetNight" );
		m_BtnQuickActionDusk 	= UIActionManager.CreateButton( actionsDate, "Dusk", this, "OnClick_PresetDusk" );
		m_BtnQuickActionDay 	= UIActionManager.CreateButton( actionsDate, "Day", this, "OnClick_PresetDay" );
		m_BtnQuickActionDawn 	= UIActionManager.CreateButton( actionsDate, "Dawn", this, "OnClick_PresetDawn" );

		UIActionManager.CreatePanel( m_PanelQuickActions, 0xFF000000, 1 );
	}

	void OnClick_PresetClear( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_PresetsShown )
			return;

		float duration = 240;
		float transition = 0;

		m_Module.SetOvercast( 0, transition, duration );
		m_Module.SetRain( 0, transition, duration );
		m_Module.SetSnow( 0, transition, duration );

		m_Module.SetFog( 0, transition, duration );
		m_Module.SetDynamicFog( 0, 0, 0 );

		m_Module.SetWindMagnitude( 0, transition, duration );
		m_Module.SetWindDirection( 0, transition, duration );
		m_Module.SetWindFunctionParams( 0, 1, 30 );

		RefreshValues();
	}

	void OnClick_PresetCloudy( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_PresetsShown )
			return;

		float duration = 240;
		float transition = 0;
		float thresholdStorm = Math.Max(m_SliderStormThreshold.GetCurrent() * 0.01, 0.7);
		float windMin = Math.RandomFloatInclusive(0,0.8);

		//! Set overcast to below storm threshold
		m_Module.SetStorm( m_SliderStormDensity.GetCurrent() * 0.01, thresholdStorm, m_SliderMinTimeBetweenLightning.GetCurrent() );
		m_Module.SetOvercast( Math.RandomFloat(0.5, thresholdStorm), transition, duration );

		m_Module.SetRain( 0, transition, duration );
		m_Module.SetSnow( 0, transition, duration );

		m_Module.SetFog( 0, transition, duration );
		m_Module.SetDynamicFog( 0, 0, 0 );

		m_Module.SetWindMagnitude( Math.RandomFloatInclusive(0,20), transition, duration );
		float windMaxSpeed = Math.RandomFloatInclusive(0.1,20);
		m_Module.SetWindDirection( Math.RandomFloatInclusive(0.1,windMaxSpeed * 0.5), transition, duration );
		m_Module.SetWindFunctionParams( windMin, Math.RandomFloatInclusive(windMin,1), Math.RandomFloatInclusive(0,30) );

		RefreshValues();
	}

	void OnClick_PresetRainy( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_PresetsShown )
			return;

		float duration = 240;
		float transition = 0;
		float thresholdStorm = Math.Max(m_SliderStormThreshold.GetCurrent() * 0.01, 0.7);
		float windMin = Math.RandomFloatInclusive(0,0.8);

		//! Set overcast to below storm threshold
		m_Module.SetStorm( m_SliderStormDensity.GetCurrent() * 0.01, thresholdStorm, m_SliderMinTimeBetweenLightning.GetCurrent() );
		m_Module.SetOvercast( Math.RandomFloat(0.5, thresholdStorm), transition, duration );

		m_Module.SetRainThresholds( 0.5, 1.0, 120.0 );
		m_Module.SetRain( Math.RandomFloatInclusive(0.5,1.0), transition, duration );
		m_Module.SetSnow( 0, transition, duration );

		m_Module.SetFog( Math.RandomFloatInclusive(0.0,1.0 - windMin), transition, duration );
		m_Module.SetDynamicFog( 0, 0, 0 );

		m_Module.SetWindMagnitude( Math.RandomFloatInclusive(0,20), transition, duration );
		float windMaxSpeed = Math.RandomFloatInclusive(0.1,20);
		m_Module.SetWindDirection( Math.RandomFloatInclusive(0.1,windMaxSpeed * 0.5), transition, duration );
		m_Module.SetWindFunctionParams( windMin, Math.RandomFloatInclusive(windMin,1), Math.RandomFloatInclusive(0,30) );

		RefreshValues();
	}

	void OnClick_PresetSnowy( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_PresetsShown )
			return;

		float duration = 240;
		float transition = 0;
		float thresholdStorm = Math.Max(m_SliderStormThreshold.GetCurrent() * 0.01, 0.7);
		float windMin = Math.RandomFloatInclusive(0,0.8);

		//! Set overcast to below storm threshold
		m_Module.SetStorm( m_SliderStormDensity.GetCurrent() * 0.01, thresholdStorm, m_SliderMinTimeBetweenLightning.GetCurrent() );
		m_Module.SetOvercast( Math.RandomFloat(0.5, thresholdStorm), transition, duration );

		m_Module.SetSnowThresholds( 0.5, 1.0, 120.0 );
		m_Module.SetRain( 0, transition, duration );
		m_Module.SetSnow( Math.RandomFloatInclusive(0.5,1.0), transition, duration );
		m_Module.SetSnowThresholds( 0.5, 1.0, 120.0 );

		m_Module.SetFog( Math.RandomFloatInclusive(0.0,1.0 - windMin), transition, duration );
		m_Module.SetDynamicFog( 0, 0, 0 );

		m_Module.SetWindMagnitude( Math.RandomFloatInclusive(0,20), transition, duration );
		float windMaxSpeed = Math.RandomFloatInclusive(0.1,20);
		m_Module.SetWindDirection( Math.RandomFloatInclusive(0.1,windMaxSpeed * 0.5), transition, duration );
		m_Module.SetWindFunctionParams( windMin, Math.RandomFloatInclusive(windMin,1), Math.RandomFloatInclusive(0,30) );

		RefreshValues();
	}

	void OnClick_PresetStorm( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_PresetsShown )
			return;
			
		float duration = 240;
		float transition = 0;
		float windMin = Math.RandomFloatInclusive(0,1);
		float minRainThresh = m_SliderRainOvercastMin.GetCurrent() * 0.01;

		m_Module.SetStorm( 1.0, 0.7, m_SliderMinTimeBetweenLightning.GetCurrent() );
		m_Module.SetOvercast( Math.RandomFloatInclusive(0.7,1.0), transition, duration );

		m_Module.SetRain( Math.RandomFloatInclusive(minRainThresh,1.0), transition, duration );
		m_Module.SetSnow( 0, transition, duration );

		m_Module.SetFog( Math.RandomFloatInclusive(0.0,1.0 - windMin), transition, duration );
		m_Module.SetDynamicFog( 0, 0, 0 );

		m_Module.SetWindMagnitude( Math.RandomFloatInclusive(0,20), transition, duration );
		float windMaxSpeed = Math.RandomFloatInclusive(0.1,20);
		m_Module.SetWindDirection( Math.RandomFloatInclusive(0.1,windMaxSpeed * 0.5), transition, duration );
		m_Module.SetWindFunctionParams( windMin, Math.RandomFloatInclusive(windMin,1), Math.RandomFloatInclusive(0,30) );

		RefreshValues();
	}

	void OnClick_PresetNight( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_PresetsShown )
			return;

		int year = m_EditTextDateYear.GetText().ToInt();
		int month = m_SliderDateMonth.GetCurrent();
		m_SliderDateDay.SetMax( m_DaysInMonth[month - 1] );
		int day = m_SliderDateDay.GetCurrent();
		int hour = 0;
		int minute = 0;

		m_Module.SetDate( year, month, day, hour, minute );
		
		RefreshValues();
	}

	void OnClick_PresetDusk( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_PresetsShown )
			return;

		int year = m_EditTextDateYear.GetText().ToInt();
		int month = m_SliderDateMonth.GetCurrent();
		m_SliderDateDay.SetMax( m_DaysInMonth[month - 1] );
		int day = m_SliderDateDay.GetCurrent();
		int hour = 6;
		int minute = 0;

		m_Module.SetDate( year, month, day, hour, minute );
		
		RefreshValues();
	}

	void OnClick_PresetDay( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_PresetsShown )
			return;

		int year = m_EditTextDateYear.GetText().ToInt();
		int month = m_SliderDateMonth.GetCurrent();
		m_SliderDateDay.SetMax( m_DaysInMonth[month - 1] );
		int day = m_SliderDateDay.GetCurrent();
		int hour = 12;
		int minute = 0;

		m_Module.SetDate( year, month, day, hour, minute );
		
		RefreshValues();
	}

	void OnClick_PresetDawn( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_PresetsShown )
			return;

		int year = m_EditTextDateYear.GetText().ToInt();
		int month = m_SliderDateMonth.GetCurrent();
		m_SliderDateDay.SetMax( m_DaysInMonth[month - 1] );
		int day = m_SliderDateDay.GetCurrent();
		int hour = 18;
		int minute = 0;

		m_Module.SetDate( year, month, day, hour, minute );
		
		RefreshValues();
	}
	
	private void InitTogglesWidgets( Widget actionsParent )
	{
		m_PanelToggles = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( m_PanelToggles, "Toggles: ", "Show or Hide different elements" );
	
		Widget actions = UIActionManager.CreateGridSpacer( m_PanelToggles, 2, 4 );

		m_ToggleQuickActions= UIActionManager.CreateButtonToggle( actions, "[  ] Quick", "[×] Quick", this, "OnClick_Toggle" );
		m_ToggleDate 		= UIActionManager.CreateButtonToggle( actions, "[  ] Date", "[×] Date", this, "OnClick_Toggle" );
		m_ToggleStorm 		= UIActionManager.CreateButtonToggle( actions, "[  ] Storm", "[×] Storm", this, "OnClick_Toggle" );
		m_ToggleOvercast 	= UIActionManager.CreateButtonToggle( actions, "[  ] Overcast", "[×] Overcast", this, "OnClick_Toggle" );
		
		m_ToggleRain 		= UIActionManager.CreateButtonToggle( actions, "[  ] Rain", "[×] Rain", this, "OnClick_Toggle" );
		m_ToggleSnow 		= UIActionManager.CreateButtonToggle( actions, "[  ] Snow", "[×] Snow", this, "OnClick_Toggle" );
		m_ToggleFog 		= UIActionManager.CreateButtonToggle( actions, "[  ] Fog", "[×] Fog", this, "OnClick_Toggle" );
		m_ToggleWind 		= UIActionManager.CreateButtonToggle( actions, "[  ] Wind", "[×] Wind", this, "OnClick_Toggle" );

		m_ToggleQuickActions.SetToggle(true);

		UIActionManager.CreatePanel( m_PanelToggles, 0xFF000000, 1 );
	}

	void OnClick_Toggle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_PresetsShown )
			return;

		ToggleMenus();
	}
	
	void ToggleMenus()
	{
		// This is aids, will redo this bit one day
		if (m_ToggleQuickActions.IsToggled() && !m_PresetsShown)
			m_PanelQuickActions.Show(true);
		else
			m_PanelQuickActions.Show(false);

		if (m_ToggleDate.IsToggled() || m_PresetsShown)
			m_PanelDate.Show(true);
		else
			m_PanelDate.Show(false);
		
		if (m_ToggleStorm.IsToggled() || m_PresetsShown)
			m_PanelStorm.Show(true);
		else
			m_PanelStorm.Show(false);
		
		if (m_ToggleOvercast.IsToggled() || m_PresetsShown)
			m_PanelOvercast.Show(true);
		else
			m_PanelOvercast.Show(false);
		
		if (m_ToggleRain.IsToggled() || m_PresetsShown)
		{
			m_PanelRain.Show(true);
			m_PanelRainThreshold.Show(true);
		}
		else
		{
			m_PanelRain.Show(false);
			m_PanelRainThreshold.Show(false);
		}
		
		if (m_ToggleSnow.IsToggled() || m_PresetsShown)
		{
			m_PanelSnow.Show(true);
			m_PanelSnowThreshold.Show(true);
		}
		else
		{
			m_PanelSnow.Show(false);
			m_PanelSnowThreshold.Show(false);
		}
		
		if (m_ToggleFog.IsToggled() || m_PresetsShown)
		{
			m_PanelFog.Show(true);
			m_PanelFogDynamic.Show(true);
		}
		else
		{
			m_PanelFog.Show(false);
			m_PanelFogDynamic.Show(false);
		}
		
		if (m_ToggleWind.IsToggled() || m_PresetsShown)
		{
			m_PanelWindMagnitude.Show(true);
			m_PanelWindDirection.Show(true);

			if (m_PresetsShown)
				m_PanelWindFunc.Show(true);
			else
				m_PanelWindFunc.Show(!m_Checkbox_EasyMode.IsChecked());
		}
		else
		{
			m_PanelWindMagnitude.Show(false);
			m_PanelWindDirection.Show(false);
			m_PanelWindFunc.Show(false);
		}
	}

	private void InitDateWidgets( Widget actionsParent )
	{
		m_PanelDate = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( m_PanelDate, "Date: ", "Sets the in-game date" );
	
		Widget actions = UIActionManager.CreateGridSpacer( m_PanelDate, 3, 2 );

		m_EditTextDateYear = UIActionManager.CreateEditableText( actions, "Year", this );
		m_EditTextDateYear.SetOnlyNumbers( true, true );
		m_EditTextDateYear.SetText( "Year" );

		m_SliderDateMonth = UIActionManager.CreateSlider( actions, "Month", 0, 1, this, "OnChange_Date" );
		m_SliderDateMonth.SetCurrent( 0 );
		m_SliderDateMonth.SetStepValue( 1 );
		m_SliderDateMonth.SetMin( 1 );
		m_SliderDateMonth.SetMax( 12 );

		m_SliderDateDay = UIActionManager.CreateSlider( actions, "Day", 0, 1, this );
		m_SliderDateDay.SetCurrent( 0 );
		m_SliderDateDay.SetStepValue( 1 );
		m_SliderDateDay.SetMin( 1 );
		m_SliderDateDay.SetMax( 31 );

		m_SliderDateHour = UIActionManager.CreateSlider( actions, "Hour", 0, 1, this );
		m_SliderDateHour.SetCurrent( 0 );
		m_SliderDateHour.SetStepValue( 1 );
		m_SliderDateHour.SetMin( 0 );
		m_SliderDateHour.SetMax( 23 );

		m_SliderDateMinute = UIActionManager.CreateSlider( actions, "Minute", 0, 1, this );
		m_SliderDateMinute.SetCurrent( 0 );
		m_SliderDateMinute.SetStepValue( 1 );
		m_SliderDateMinute.SetMin( 0 );
		m_SliderDateMinute.SetMax( 59 );

		UIActionManager.CreatePanel( m_PanelDate, 0xFF000000, 1 );
	}

	void OnChange_Date( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_PresetsShown )
			return;
	
		int year = m_EditTextDateYear.GetText().ToInt();
		int month = m_SliderDateMonth.GetCurrent();
		m_SliderDateDay.SetMax( m_DaysInMonth[month - 1] );
	}

	private void InitStormWidgets( Widget actionsParent )
	{
		m_PanelStorm = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( m_PanelStorm, "Storm: ", "Sets the weather storm" );
	
		Widget actions = UIActionManager.CreatePanel( m_PanelStorm, 0x00000000, 50 );

		Widget actionsGrid = UIActionManager.CreateGridSpacer( actions, 1, 3 );

		m_SliderStormDensity = UIActionManager.CreateSlider( actionsGrid, "Density", 0, 1, this );
		m_SliderStormDensity.SetCurrent( 0 );
		m_SliderStormDensity.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderStormDensity.SetStepValue( 0.1 );
		m_SliderStormDensity.SetMin( 0 );
		m_SliderStormDensity.SetMax( 100 );
		m_SliderStormDensity.SetWidgetWidth( m_SliderStormDensity.GetLabelWidget(), 0.6 );
		m_SliderStormDensity.SetWidgetWidth( m_SliderStormDensity.GetSliderWidget(), 0.6 );

		m_SliderStormThreshold = UIActionManager.CreateSlider( actionsGrid, "Threshold", 0, 1, this );
		m_SliderStormThreshold.SetCurrent( 0 );
		m_SliderStormThreshold.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderStormThreshold.SetStepValue( 0.1 );
		m_SliderStormThreshold.SetMin( 0 );
		m_SliderStormThreshold.SetMax( 100 );
		m_SliderStormThreshold.SetWidgetWidth( m_SliderStormThreshold.GetLabelWidget(), 0.6 );
		m_SliderStormThreshold.SetWidgetWidth( m_SliderStormThreshold.GetSliderWidget(), 0.6 );

		m_SliderMinTimeBetweenLightning = UIActionManager.CreateSlider( actionsGrid, "Lightning", 0, 1, this );
		m_SliderMinTimeBetweenLightning.SetCurrent( 0 );
		m_SliderMinTimeBetweenLightning.SetStepValue( 1 );
		m_SliderMinTimeBetweenLightning.SetMin( 0 );
		m_SliderMinTimeBetweenLightning.SetMax( 120 );
		m_SliderMinTimeBetweenLightning.SetWidgetWidth( m_SliderMinTimeBetweenLightning.GetLabelWidget(), 0.6 );
		m_SliderMinTimeBetweenLightning.SetWidgetWidth( m_SliderMinTimeBetweenLightning.GetSliderWidget(), 0.6 );

		UIActionManager.CreatePanel( m_PanelStorm, 0xFF000000, 1 );
	}

	private void InitFogWidgets( Widget actionsParent )
	{
		m_PanelFog = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( m_PanelFog, "Fog: ", "Sets the weather fog phenomenon" );
	
		Widget actions = UIActionManager.CreatePanel( m_PanelFog, 0x00000000, 35 );

		Widget actionsGrid = UIActionManager.CreateGridSpacer( actions, 1, 3 );

		m_SliderFogForecast = UIActionManager.CreateSlider( actionsGrid, "Amount", 0, 1, this);
		m_SliderFogForecast.SetCurrent( 0 );
		m_SliderFogForecast.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderFogForecast.SetStepValue( 0.1 );
		m_SliderFogForecast.SetMin( 0 );
		m_SliderFogForecast.SetMax( 100 );
		m_SliderFogForecast.SetWidgetWidth( m_SliderFogForecast.GetLabelWidget(), 0.6 );
		m_SliderFogForecast.SetWidgetWidth( m_SliderFogForecast.GetSliderWidget(), 0.6 );

		m_EditFogMinDuration = UIActionManager.CreateEditableText( actionsGrid, "Duration", this);
		m_EditFogMinDuration.SetOnlyNumbers( true );
		m_EditFogMinDuration.SetText( "0" );
		m_EditFogMinDuration.SetWidgetWidth( m_EditFogMinDuration.GetLabelWidget(), 0.6 );
		m_EditFogMinDuration.SetWidgetWidth( m_EditFogMinDuration.GetEditBoxWidget(), 0.6 );

		m_EditFogInterpTime = UIActionManager.CreateEditableText( actionsGrid, "Transition", this);
		m_EditFogInterpTime.SetOnlyNumbers( true );
		m_EditFogInterpTime.SetText( "0" );
		m_EditFogInterpTime.SetWidgetWidth( m_EditFogInterpTime.GetLabelWidget(), 0.6 );
		m_EditFogInterpTime.SetWidgetWidth( m_EditFogInterpTime.GetEditBoxWidget(), 0.6 );

		UIActionManager.CreatePanel( m_PanelFog, 0xFF000000, 1 );
	}

	private void InitDynFogWidgets( Widget actionsParent )
	{
		m_PanelFogDynamic = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( m_PanelFogDynamic, "Dynamic Fog: ", "Sets the weather dynamic fog phenomenon" );
	
		Widget actions = UIActionManager.CreatePanel( m_PanelFogDynamic, 0x00000000, 35 );

		Widget actionsGrid = UIActionManager.CreateGridSpacer( actions, 1, 4 );

		m_SliderDyanmicFogBias = UIActionManager.CreateSlider( actionsGrid, "Bias", 0, 1, this);
		m_SliderDyanmicFogBias.SetCurrent( 0 );
		m_SliderDyanmicFogBias.SetStepValue( 10 );
		m_SliderDyanmicFogBias.SetMin( -500 );
		m_SliderDyanmicFogBias.SetMax( 500 );
		m_SliderDyanmicFogBias.SetWidgetWidth( m_SliderDyanmicFogBias.GetLabelWidget(), 0.6 );
		m_SliderDyanmicFogBias.SetWidgetWidth( m_SliderDyanmicFogBias.GetSliderWidget(), 0.6 );

		m_SliderDyanmicFogDistance = UIActionManager.CreateSlider( actionsGrid, "Distance", 0, 1, this);
		m_SliderDyanmicFogDistance.SetCurrent( 0 );
		m_SliderDyanmicFogDistance.SetStepValue( 0.1 );
		m_SliderDyanmicFogDistance.SetMin( 0 );
		m_SliderDyanmicFogDistance.SetMax( 1 );
		m_SliderDyanmicFogDistance.SetWidgetWidth( m_SliderDyanmicFogDistance.GetLabelWidget(), 0.6 );
		m_SliderDyanmicFogDistance.SetWidgetWidth( m_SliderDyanmicFogDistance.GetSliderWidget(), 0.6 );
		
		m_SliderDyanmicFogHeight = UIActionManager.CreateSlider( actionsGrid, "Height", 0, 1, this);
		m_SliderDyanmicFogHeight.SetCurrent( 0 );
		m_SliderDyanmicFogHeight.SetStepValue( 0.1 );
		m_SliderDyanmicFogHeight.SetMin( 0 );
		m_SliderDyanmicFogHeight.SetMax( 1 );
		m_SliderDyanmicFogHeight.SetWidgetWidth( m_SliderDyanmicFogHeight.GetLabelWidget(), 0.6 );
		m_SliderDyanmicFogHeight.SetWidgetWidth( m_SliderDyanmicFogHeight.GetSliderWidget(), 0.6 );

		m_EditDynamicFogInterpTime = UIActionManager.CreateEditableText( actionsGrid, "Transition", this);
		m_EditDynamicFogInterpTime.SetOnlyNumbers( true );
		m_EditDynamicFogInterpTime.SetText( "0" );
		m_EditDynamicFogInterpTime.SetWidgetWidth( m_EditDynamicFogInterpTime.GetLabelWidget(), 0.6 );
		m_EditDynamicFogInterpTime.SetWidgetWidth( m_EditDynamicFogInterpTime.GetEditBoxWidget(), 0.6 );

		UIActionManager.CreatePanel( m_PanelFogDynamic, 0xFF000000, 1 );
	}

	private void InitRainWidgets( Widget actionsParent )
	{
		m_PanelRain = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( m_PanelRain, "Rain: ", "Sets the weather rain phenomenon" );
	
		Widget actions = UIActionManager.CreatePanel( m_PanelRain, 0x00000000, 35 );

		Widget actionsGrid = UIActionManager.CreateGridSpacer( actions, 1, 3 );

		m_SliderRainForecast = UIActionManager.CreateSlider( actionsGrid, "Amount", 0, 1, this );
		m_SliderRainForecast.SetCurrent( 0 );
		m_SliderRainForecast.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderRainForecast.SetStepValue( 0.1 );
		m_SliderRainForecast.SetMin( 0 );
		m_SliderRainForecast.SetMax( 100 );
		m_SliderRainForecast.SetWidgetWidth( m_SliderRainForecast.GetLabelWidget(), 0.6 );
		m_SliderRainForecast.SetWidgetWidth( m_SliderRainForecast.GetSliderWidget(), 0.6 );

		m_EditRainMinDuration = UIActionManager.CreateEditableText( actionsGrid, "Duration", this );
		m_EditRainMinDuration.SetOnlyNumbers( true );
		m_EditRainMinDuration.SetText( "0" );
		m_EditRainMinDuration.SetWidgetWidth( m_EditRainMinDuration.GetLabelWidget(), 0.6 );
		m_EditRainMinDuration.SetWidgetWidth( m_EditRainMinDuration.GetEditBoxWidget(), 0.6 );

		m_EditRainInterpTime = UIActionManager.CreateEditableText( actionsGrid, "Transition", this );
		m_EditRainInterpTime.SetOnlyNumbers( true );
		m_EditRainInterpTime.SetText( "0" );
		m_EditRainInterpTime.SetWidgetWidth( m_EditRainInterpTime.GetLabelWidget(), 0.6 );
		m_EditRainInterpTime.SetWidgetWidth( m_EditRainInterpTime.GetEditBoxWidget(), 0.6 );

		UIActionManager.CreatePanel( m_PanelRain, 0xFF000000, 1 );
	}

	private void InitRainThresholdWidgets( Widget actionsParent )
	{
		m_PanelRainThreshold = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( m_PanelRainThreshold, "Rain Tresholds: ", "Sets the minimum, maximum overcast" );
	
		Widget actions = UIActionManager.CreatePanel( m_PanelRainThreshold, 0x00000000, 35 );

		Widget actionsGrid = UIActionManager.CreateGridSpacer( actions, 1, 3 );

		m_SliderRainOvercastMin = UIActionManager.CreateSlider( actionsGrid, "Min", 0, 1, this );
		m_SliderRainOvercastMin.SetCurrent( 0 );
		m_SliderRainOvercastMin.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderRainOvercastMin.SetStepValue( 0.1 );
		m_SliderRainOvercastMin.SetMin( 0 );
		m_SliderRainOvercastMin.SetMax( 100 );
		m_SliderRainOvercastMin.SetWidgetWidth( m_SliderRainOvercastMin.GetLabelWidget(), 0.6 );
		m_SliderRainOvercastMin.SetWidgetWidth( m_SliderRainOvercastMin.GetSliderWidget(), 0.6 );

		m_SliderRainOvercastMax = UIActionManager.CreateSlider( actionsGrid, "Max", 0, 1, this );
		m_SliderRainOvercastMax.SetCurrent( 0 );
		m_SliderRainOvercastMax.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderRainOvercastMax.SetStepValue( 0.1 );
		m_SliderRainOvercastMax.SetMin( 0 );
		m_SliderRainOvercastMax.SetMax( 100 );
		m_SliderRainOvercastMax.SetWidgetWidth( m_SliderRainOvercastMax.GetLabelWidget(), 0.6 );
		m_SliderRainOvercastMax.SetWidgetWidth( m_SliderRainOvercastMax.GetSliderWidget(), 0.6 );

		m_EditTextRainTransitionTime = UIActionManager.CreateEditableText( actionsGrid, "Transition", this );
		m_EditTextRainTransitionTime.SetOnlyNumbers( true );
		m_EditTextRainTransitionTime.SetText( "0" );
		m_EditTextRainTransitionTime.SetWidgetWidth( m_EditTextRainTransitionTime.GetLabelWidget(), 0.6 );
		m_EditTextRainTransitionTime.SetWidgetWidth( m_EditTextRainTransitionTime.GetEditBoxWidget(), 0.6 );

		UIActionManager.CreatePanel( m_PanelRainThreshold, 0xFF000000, 1 );
	}

	private void InitSnowWidgets( Widget actionsParent )
	{
		m_PanelSnow = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( m_PanelSnow, "Snow: ", "Sets the weather Snow phenomenon" );
	
		Widget actions = UIActionManager.CreatePanel( m_PanelSnow, 0x00000000, 35 );

		Widget actionsGrid = UIActionManager.CreateGridSpacer( actions, 1, 3 );

		m_SliderSnowForecast = UIActionManager.CreateSlider( actionsGrid, "Amount", 0, 1, this );
		m_SliderSnowForecast.SetCurrent( 0 );
		m_SliderSnowForecast.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderSnowForecast.SetStepValue( 0.1 );
		m_SliderSnowForecast.SetMin( 0 );
		m_SliderSnowForecast.SetMax( 100 );
		m_SliderSnowForecast.SetWidgetWidth( m_SliderSnowForecast.GetLabelWidget(), 0.6 );
		m_SliderSnowForecast.SetWidgetWidth( m_SliderSnowForecast.GetSliderWidget(), 0.6 );

		m_EditSnowMinDuration = UIActionManager.CreateEditableText( actionsGrid, "Duration", this );
		m_EditSnowMinDuration.SetOnlyNumbers( true );
		m_EditSnowMinDuration.SetText( "0" );
		m_EditSnowMinDuration.SetWidgetWidth( m_EditSnowMinDuration.GetLabelWidget(), 0.6 );
		m_EditSnowMinDuration.SetWidgetWidth( m_EditSnowMinDuration.GetEditBoxWidget(), 0.6 );

		m_EditSnowInterpTime = UIActionManager.CreateEditableText( actionsGrid, "Transition", this );
		m_EditSnowInterpTime.SetOnlyNumbers( true );
		m_EditSnowInterpTime.SetText( "0" );
		m_EditSnowInterpTime.SetWidgetWidth( m_EditSnowInterpTime.GetLabelWidget(), 0.6 );
		m_EditSnowInterpTime.SetWidgetWidth( m_EditSnowInterpTime.GetEditBoxWidget(), 0.6 );

		UIActionManager.CreatePanel( m_PanelSnow, 0xFF000000, 1 );
	}

	private void InitSnowThresholdWidgets( Widget actionsParent )
	{
		m_PanelSnowThreshold = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( m_PanelSnowThreshold, "Snow Tresholds: ", "Sets the minimum, maximum overcast" );
	
		Widget actions = UIActionManager.CreatePanel( m_PanelSnowThreshold, 0x00000000, 35 );

		Widget actionsGrid = UIActionManager.CreateGridSpacer( actions, 1, 3 );

		m_SliderSnowOvercastMin = UIActionManager.CreateSlider( actionsGrid, "Min", 0, 1, this );
		m_SliderSnowOvercastMin.SetCurrent( 0 );
		m_SliderSnowOvercastMin.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderSnowOvercastMin.SetStepValue( 0.1 );
		m_SliderSnowOvercastMin.SetMin( 0 );
		m_SliderSnowOvercastMin.SetMax( 100 );
		m_SliderSnowOvercastMin.SetWidgetWidth( m_SliderSnowOvercastMin.GetLabelWidget(), 0.6 );
		m_SliderSnowOvercastMin.SetWidgetWidth( m_SliderSnowOvercastMin.GetSliderWidget(), 0.6 );

		m_SliderSnowOvercastMax = UIActionManager.CreateSlider( actionsGrid, "Max", 0, 1, this );
		m_SliderSnowOvercastMax.SetCurrent( 0 );
		m_SliderSnowOvercastMax.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderSnowOvercastMax.SetStepValue( 0.1 );
		m_SliderSnowOvercastMax.SetMin( 0 );
		m_SliderSnowOvercastMax.SetMax( 100 );
		m_SliderSnowOvercastMax.SetWidgetWidth( m_SliderSnowOvercastMax.GetLabelWidget(), 0.6 );
		m_SliderSnowOvercastMax.SetWidgetWidth( m_SliderSnowOvercastMax.GetSliderWidget(), 0.6 );

		m_EditTextSnowTransitionTime = UIActionManager.CreateEditableText( actionsGrid, "Transition", this );
		m_EditTextSnowTransitionTime.SetOnlyNumbers( true );
		m_EditTextSnowTransitionTime.SetText( "0" );
		m_EditTextSnowTransitionTime.SetWidgetWidth( m_EditTextSnowTransitionTime.GetLabelWidget(), 0.6 );
		m_EditTextSnowTransitionTime.SetWidgetWidth( m_EditTextSnowTransitionTime.GetEditBoxWidget(), 0.6 );

		UIActionManager.CreatePanel( m_PanelSnowThreshold, 0xFF000000, 1 );
	}

	private void InitOvercastWidgets( Widget actionsParent )
	{
		m_PanelOvercast = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( m_PanelOvercast, "Clouds: ", "Sets the weather overcast phenomenon" );
	
		Widget actions = UIActionManager.CreatePanel( m_PanelOvercast, 0x00000000, 35 );

		Widget actionsGrid = UIActionManager.CreateGridSpacer( actions, 1, 3 );

		m_SliderOvercastForecast = UIActionManager.CreateSlider( actionsGrid, "Amount", 0, 1, this );
		m_SliderOvercastForecast.SetCurrent( 0 );
		m_SliderOvercastForecast.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderOvercastForecast.SetStepValue( 0.1 );
		m_SliderOvercastForecast.SetMin( 0 );
		m_SliderOvercastForecast.SetMax( 100 );
		m_SliderOvercastForecast.SetWidgetWidth( m_SliderOvercastForecast.GetLabelWidget(), 0.6 );
		m_SliderOvercastForecast.SetWidgetWidth( m_SliderOvercastForecast.GetSliderWidget(), 0.6 );

		m_EditOvercastMinDuration = UIActionManager.CreateEditableText( actionsGrid, "Duration", this );
		m_EditOvercastMinDuration.SetOnlyNumbers( true );
		m_EditOvercastMinDuration.SetText( "0" );
		m_EditOvercastMinDuration.SetWidgetWidth( m_EditOvercastMinDuration.GetLabelWidget(), 0.6 );
		m_EditOvercastMinDuration.SetWidgetWidth( m_EditOvercastMinDuration.GetEditBoxWidget(), 0.6 );

		m_EditOvercastInterpTime = UIActionManager.CreateEditableText( actionsGrid, "Transition", this );
		m_EditOvercastInterpTime.SetOnlyNumbers( true );
		m_EditOvercastInterpTime.SetText( "0" );
		m_EditOvercastInterpTime.SetWidgetWidth( m_EditOvercastInterpTime.GetLabelWidget(), 0.6 );
		m_EditOvercastInterpTime.SetWidgetWidth( m_EditOvercastInterpTime.GetEditBoxWidget(), 0.6 );

		UIActionManager.CreatePanel( m_PanelOvercast, 0xFF000000, 1 );
	}

	private void InitWindMagnitudeWidgets( Widget actionsParent )
	{
		m_PanelWindMagnitude = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( m_PanelWindMagnitude, "Wind Magnitude: ", "Sets the Weather Wind Magnitude phenomenon" );
	
		Widget actions = UIActionManager.CreatePanel( m_PanelWindMagnitude, 0x00000000, 35 );

		Widget actionsGrid = UIActionManager.CreateGridSpacer( actions, 1, 3 );

		m_SliderWindMagnitudeForecast = UIActionManager.CreateSlider( actionsGrid, "Amount", 0, 1, this);
		m_SliderWindMagnitudeForecast.SetCurrent( 0 );
		m_SliderWindMagnitudeForecast.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderWindMagnitudeForecast.SetStepValue( 0.1 );
		m_SliderWindMagnitudeForecast.SetMin( 0 );
		m_SliderWindMagnitudeForecast.SetMax( 20 );
		m_SliderWindMagnitudeForecast.SetWidgetWidth( m_SliderWindMagnitudeForecast.GetLabelWidget(), 0.6 );
		m_SliderWindMagnitudeForecast.SetWidgetWidth( m_SliderWindMagnitudeForecast.GetSliderWidget(), 0.6 );

		m_EditWindMagnitudeMinDuration = UIActionManager.CreateEditableText( actionsGrid, "Duration", this);
		m_EditWindMagnitudeMinDuration.SetOnlyNumbers( true );
		m_EditWindMagnitudeMinDuration.SetText( "0" );
		m_EditWindMagnitudeMinDuration.SetWidgetWidth( m_EditWindMagnitudeMinDuration.GetLabelWidget(), 0.6 );
		m_EditWindMagnitudeMinDuration.SetWidgetWidth( m_EditWindMagnitudeMinDuration.GetEditBoxWidget(), 0.6 );

		m_EditWindMagnitudeInterpTime = UIActionManager.CreateEditableText( actionsGrid, "Transition", this);
		m_EditWindMagnitudeInterpTime.SetOnlyNumbers( true );
		m_EditWindMagnitudeInterpTime.SetText( "0" );
		m_EditWindMagnitudeInterpTime.SetWidgetWidth( m_EditWindMagnitudeInterpTime.GetLabelWidget(), 0.6 );
		m_EditWindMagnitudeInterpTime.SetWidgetWidth( m_EditWindMagnitudeInterpTime.GetEditBoxWidget(), 0.6 );

		UIActionManager.CreatePanel( m_PanelWindMagnitude, 0xFF000000, 1 );
	}
	
	private void InitWindDirectionWidgets( Widget actionsParent )
	{
		m_PanelWindDirection = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( m_PanelWindDirection, "Wind Direction: ", "Sets the Weather Wind Direction phenomenon" );
	
		Widget actions = UIActionManager.CreatePanel( m_PanelWindDirection, 0x00000000, 35 );

		Widget actionsGrid = UIActionManager.CreateGridSpacer( actions, 1, 3 );

		m_SliderWindDirectionForecast = UIActionManager.CreateSlider( actionsGrid, "Direction", 0, 1, this, "OnChange_Wind");
		m_SliderWindDirectionForecast.SetCurrent( 0 );
		m_SliderWindDirectionForecast.SetFormat("");
		m_SliderWindDirectionForecast.SetStepValue( 1 );
		m_SliderWindDirectionForecast.SetMin( 0 );
		m_SliderWindDirectionForecast.SetMax( 360 );
		m_SliderWindDirectionForecast.SetWidgetWidth( m_SliderWindDirectionForecast.GetLabelWidget(), 0.6 );
		m_SliderWindDirectionForecast.SetWidgetWidth( m_SliderWindDirectionForecast.GetSliderWidget(), 0.6 );

		m_EditWindDirectionMinDuration = UIActionManager.CreateEditableText( actionsGrid, "Duration", this);
		m_EditWindDirectionMinDuration.SetOnlyNumbers( true );
		m_EditWindDirectionMinDuration.SetText( "0" );
		m_EditWindDirectionMinDuration.SetWidgetWidth( m_EditWindDirectionMinDuration.GetLabelWidget(), 0.6 );
		m_EditWindDirectionMinDuration.SetWidgetWidth( m_EditWindDirectionMinDuration.GetEditBoxWidget(), 0.6 );

		m_EditWindDirectionInterpTime = UIActionManager.CreateEditableText( actionsGrid, "Transition", this);
		m_EditWindDirectionInterpTime.SetOnlyNumbers( true );
		m_EditWindDirectionInterpTime.SetText( "0" );
		m_EditWindDirectionInterpTime.SetWidgetWidth( m_EditWindDirectionInterpTime.GetLabelWidget(), 0.6 );
		m_EditWindDirectionInterpTime.SetWidgetWidth( m_EditWindDirectionInterpTime.GetEditBoxWidget(), 0.6 );

		UIActionManager.CreatePanel( m_PanelWindDirection, 0xFF000000, 1 );
	}

	void OnChange_Wind( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
		
		UpdateWindDirectionSlider();
	}

	static autoptr TStringArray CARDINAL_DIRECTIONS = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
	void UpdateWindDirectionSlider()
	{
		float direction = m_SliderWindDirectionForecast.GetCurrent();
		int index = Math.Floor(((direction + 22.5) / 45) );

		// prevents out of bound stuff - Shouldnt be needed
		if ( index > CARDINAL_DIRECTIONS.Count() - 1 )
			index -= CARDINAL_DIRECTIONS.Count();
		else if ( index < 0 )
			index = 0;

		string cardinal = CARDINAL_DIRECTIONS[index];
		m_SliderWindDirectionForecast.SetText( string.Format("%1 %2°", cardinal, direction) );
	}
	
	private void InitWindFuncWidgets( Widget actionsParent )
	{
		m_PanelWindFunc = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( m_PanelWindFunc, "Wind Function: ", "Sets the wind function parameters" );

		Widget actions = UIActionManager.CreatePanel( m_PanelWindFunc, 0x00000000, 35 );

		Widget actionsGrid = UIActionManager.CreateGridSpacer( actions, 1, 3 );

		m_EditWindFuncMin = UIActionManager.CreateSlider( actionsGrid, "Min", 0, 1, this, "OnClick_UpdateWindMinMax");
		m_EditWindFuncMin.SetCurrent( 0 );
		m_EditWindFuncMin.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_EditWindFuncMin.SetStepValue( 0.05 );
		m_EditWindFuncMin.SetMin( 0 );
		m_EditWindFuncMin.SetMax( 1.0 );
		m_EditWindFuncMin.SetWidgetWidth( m_EditWindFuncMin.GetLabelWidget(), 0.6 );
		m_EditWindFuncMin.SetWidgetWidth( m_EditWindFuncMin.GetSliderWidget(), 0.6 );

		m_EditWindFuncMax = UIActionManager.CreateSlider( actionsGrid, "Max", 0, 1, this, "OnClick_UpdateWindMinMax");
		m_EditWindFuncMax.SetCurrent( 0 );
		m_EditWindFuncMax.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_EditWindFuncMax.SetStepValue( 0.05 );
		m_EditWindFuncMax.SetMin( 0 );
		m_EditWindFuncMax.SetMax( 1.0 );
		m_EditWindFuncMax.SetWidgetWidth( m_EditWindFuncMax.GetLabelWidget(), 0.6 );
		m_EditWindFuncMax.SetWidgetWidth( m_EditWindFuncMax.GetSliderWidget(), 0.6 );

		m_EditWindFuncChangeFreq = UIActionManager.CreateEditableText( actionsGrid, "Frequency", this );
		m_EditWindFuncChangeFreq.SetOnlyNumbers( true );
		m_EditWindFuncChangeFreq.SetText( "0" );
		m_EditWindFuncChangeFreq.SetWidgetWidth( m_EditWindFuncChangeFreq.GetLabelWidget(), 0.6 );
		m_EditWindFuncChangeFreq.SetWidgetWidth( m_EditWindFuncChangeFreq.GetEditBoxWidget(), 0.6 );

		UIActionManager.CreatePanel( m_PanelWindFunc, 0xFF000000, 1 );
	}

	void OnClick_UpdateWindMinMax( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
		
		float min = m_EditWindFuncMin.GetCurrent();
		float max = m_EditWindFuncMax.GetCurrent();

		m_EditWindFuncMin.SetMax( max );
		m_EditWindFuncMax.SetMin( min );
	}
};
