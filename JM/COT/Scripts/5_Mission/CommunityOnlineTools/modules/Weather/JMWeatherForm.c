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
	private UIActionButton m_BtnQuickActionStorm;

	private UIActionButton m_BtnQuickActionNight;
	private UIActionButton m_BtnQuickActionDusk;
	private UIActionButton m_BtnQuickActionDay;
	private UIActionButton m_BtnQuickActionDawn;

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

	//! Rain stuff !
	private UIActionSlider m_SliderRainForecast;
	private UIActionEditableText m_EditRainInterpTime;
	private UIActionEditableText m_EditRainMinDuration;

	private UIActionSlider m_SliderRainOvercastMin;
	private UIActionSlider m_SliderRainOvercastMax;
	private UIActionEditableText m_EditTextRainTransitionTime;

	//! Overcast stuff !
	private UIActionSlider m_SliderOvercastForecast;
	private UIActionEditableText m_EditOvercastInterpTime;
	private UIActionEditableText m_EditOvercastMinDuration;

	//! Wind stuff !
	private UIActionSlider m_SliderWindDirectionX;
	private UIActionSlider m_SliderWindDirectionZ;
	private UIActionSlider m_EditWindSpeed;
	private UIActionEditableText m_EditWindMaxSpeed;

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

		UIActionButton button = UIActionManager.CreateButton( spacer, "Apply", this, "OnClick_Apply" );
		button.SetWidth( 0.24 );

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
		
		UpdateActionState( m_ButtonList, "Weather.Preset", easyModeEnabled );
		
		UpdateActionState( m_ButtonRefresh, "Weather", hasNotSelectedPreset );
		UpdateActionState( m_Checkbox_EasyMode, "Weather", hasNotSelectedPreset );
		UpdateActionState( m_Checkbox_AutoRefresh, "Weather", hasNotSelectedPreset );

		UpdateActionState( m_BtnQuickActionClear, "Weather.QuickAction.Clear", hasNotSelectedPreset );
		UpdateActionState( m_BtnQuickActionCloudy, "Weather.QuickAction.Overcast", hasNotSelectedPreset );
		UpdateActionState( m_BtnQuickActionRainy, "Weather.QuickAction.Rain", hasNotSelectedPreset );		
		UpdateActionState( m_BtnQuickActionStorm, "Weather.QuickAction.Storm", hasNotSelectedPreset );

		UpdateActionState( m_BtnQuickActionNight, "Weather.QuickAction.Date", hasNotSelectedPreset );
		UpdateActionState( m_BtnQuickActionDusk, "Weather.QuickAction.Date", hasNotSelectedPreset );
		UpdateActionState( m_BtnQuickActionDay, "Weather.QuickAction.Date", hasNotSelectedPreset );
		UpdateActionState( m_BtnQuickActionDawn, "Weather.QuickAction.Date", hasNotSelectedPreset );

		UpdateActionState( m_EditTextDateYear, "Weather.Date", hasNotSelectedPreset );
		UpdateActionState( m_SliderDateMonth, "Weather.Date", hasNotSelectedPreset );
		UpdateActionState( m_SliderDateDay, "Weather.Date", hasNotSelectedPreset );
		UpdateActionState( m_SliderDateHour, "Weather.Date", hasNotSelectedPreset );
		UpdateActionState( m_SliderDateMinute, "Weather.Date", hasNotSelectedPreset );
		
		UpdateActionState( m_SliderStormDensity, "Weather.Storm", hasNotSelectedPreset );
		UpdateActionState( m_SliderStormThreshold, "Weather.Storm", hasNotSelectedPreset || easyModeEnabled );
		UpdateActionState( m_SliderMinTimeBetweenLightning, "Weather.Storm", hasNotSelectedPreset || easyModeEnabled );

		UpdateActionState( m_SliderFogForecast, "Weather.Fog", hasNotSelectedPreset );
		UpdateActionState( m_EditFogInterpTime, "Weather.Fog", hasNotSelectedPreset || easyModeEnabled );
		UpdateActionState( m_EditFogMinDuration, "Weather.Fog", hasNotSelectedPreset || easyModeEnabled);

		UpdateActionState( m_SliderRainForecast, "Weather.Rain", hasNotSelectedPreset );
		UpdateActionState( m_EditRainInterpTime, "Weather.Rain", hasNotSelectedPreset || easyModeEnabled );
		UpdateActionState( m_EditRainMinDuration, "Weather.Rain", hasNotSelectedPreset || easyModeEnabled );

		UpdateActionState( m_SliderRainOvercastMin, "Weather.Rain.Thresholds", hasNotSelectedPreset );
		UpdateActionState( m_SliderRainOvercastMax, "Weather.Rain.Thresholds", hasNotSelectedPreset );
		UpdateActionState( m_EditTextRainTransitionTime, "Weather.Rain.Thresholds", hasNotSelectedPreset || easyModeEnabled );

		UpdateActionState( m_SliderOvercastForecast, "Weather.Overcast", hasNotSelectedPreset );
		UpdateActionState( m_EditOvercastInterpTime, "Weather.Overcast", hasNotSelectedPreset || easyModeEnabled );
		UpdateActionState( m_EditOvercastMinDuration, "Weather.Overcast", hasNotSelectedPreset || easyModeEnabled );

		UpdateActionState( m_SliderWindDirectionX, "Weather.Wind", hasNotSelectedPreset );
		UpdateActionState( m_SliderWindDirectionZ, "Weather.Wind", hasNotSelectedPreset );
		UpdateActionState( m_EditWindSpeed, "Weather.Wind", hasNotSelectedPreset );
		UpdateActionState( m_EditWindMaxSpeed, "Weather.Wind", hasNotSelectedPreset );

		UpdateActionState( m_EditWindFuncMin, "Weather.Wind.FunctionParams", hasNotSelectedPreset || easyModeEnabled );
		UpdateActionState( m_EditWindFuncMax, "Weather.Wind.FunctionParams", hasNotSelectedPreset || easyModeEnabled );
		UpdateActionState( m_EditWindFuncChangeFreq, "Weather.Wind.FunctionParams", hasNotSelectedPreset || easyModeEnabled );

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
			m_SliderOvercastForecast.SetCurrent( preset.POvercast.Actual * 100.0 );
		else if ( preset.POvercast.Forecast != -1 )
			m_SliderOvercastForecast.SetCurrent( preset.POvercast.Forecast * 100.0 );

		if ( preset.POvercast.Time != -1 )
			m_EditOvercastInterpTime.SetText( preset.POvercast.Time );

		if ( preset.POvercast.MinDuration != -1 )
			m_EditOvercastMinDuration.SetText( preset.POvercast.MinDuration );

		if ( preset.Wind.Dir != "0 0 0" )
		{
			vector dirWind = preset.Wind.Dir.Normalized();
			m_SliderWindDirectionX.SetCurrent( dirWind[0] );
			m_SliderWindDirectionZ.SetCurrent( dirWind[2] );
		}

		if ( preset.Wind.MaxSpeed != -1 )
		{
			m_EditWindMaxSpeed.SetText( preset.Wind.MaxSpeed );
			UpdateWindMaxSpeed();
		}

		if ( preset.Wind.Speed != -1 )
			m_EditWindSpeed.SetCurrent( preset.Wind.Speed );

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

		preset.PRain.Forecast = m_SliderRainForecast.GetCurrent() * 0.01;
		preset.PRain.Time = ToFloat( m_EditRainInterpTime.GetText() );
		preset.PRain.MinDuration = ToFloat( m_EditRainMinDuration.GetText() );

		preset.RainThreshold.OvercastMin = m_SliderRainOvercastMin.GetCurrent() * 0.01;			
		preset.RainThreshold.OvercastMax = m_SliderRainOvercastMax.GetCurrent() * 0.01;
		preset.RainThreshold.Time = ToFloat( m_EditTextRainTransitionTime.GetText() );

		preset.POvercast.Forecast = m_SliderOvercastForecast.GetCurrent() * 0.01;
		preset.POvercast.Time = ToFloat( m_EditOvercastInterpTime.GetText() );
		preset.POvercast.MinDuration = ToFloat( m_EditOvercastMinDuration.GetText() );

		preset.Wind.Dir[0] = m_SliderWindDirectionX.GetCurrent();
		preset.Wind.Dir[2] = m_SliderWindDirectionZ.GetCurrent();
		preset.Wind.Speed = m_EditWindSpeed.GetCurrent();
		preset.Wind.MaxSpeed = ToFloat( m_EditWindMaxSpeed.GetText() );

		preset.WindFunc.Min = m_EditWindFuncMin.GetCurrent();
		preset.WindFunc.Max = m_EditWindFuncMax.GetCurrent();
		preset.WindFunc.Speed = ToFloat( m_EditWindFuncChangeFreq.GetText() );
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
		InitDateWidgets( m_PanelWeatherActions );

		InitOvercastWidgets( m_PanelWeatherActions );
		InitStormWidgets( m_PanelWeatherActions );
		InitFogWidgets( m_PanelWeatherActions );

		InitRainWidgets( m_PanelWeatherActions );
		InitRainThresholdWidgets( m_PanelWeatherActions );

		InitWindWidgets( m_PanelWeatherActions );
		InitWindFuncWidgets( m_PanelWeatherActions );
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

		float dirX = m_SliderWindDirectionX.GetCurrent();
		float dirZ = m_SliderWindDirectionZ.GetCurrent();

		vector dirWind = Vector( dirX, 0.0, dirZ ).Normalized();
		float speedWind = m_EditWindSpeed.GetCurrent();
		float maxSpeedWind = m_EditWindMaxSpeed.GetText().ToFloat();

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

		if ( m_Checkbox_EasyMode.IsChecked() )
		{
			m_Module.SetDate( year, month, day, hour, minute );
			m_Module.SetFog( forecastFog, 0, 240.0 );
			m_Module.SetWind( dirWind, speedWind, maxSpeedWind );
			m_Module.SetWindFunctionParams( minFunc, maxFunc, speedFunc );
			m_Module.SetOvercast( forecastOvercast, 0, 240.0 );
			m_Module.SetStorm( densityStorm, 0.7, 25 );
			m_Module.SetRain( forecastRain, 0, 240.0 );
			m_Module.SetRainThresholds( 0.0, 1.0, 120.0 );
		}
		else
		{
			m_Module.SetDate( year, month, day, hour, minute );
			m_Module.SetFog( forecastFog, timeFog, minDurationFog );
			m_Module.SetWind( dirWind, speedWind, maxSpeedWind );
			m_Module.SetWindFunctionParams( minFunc, maxFunc, speedFunc );
			m_Module.SetOvercast( forecastOvercast, timeOvercast, minDurationOvercast );
			m_Module.SetStorm( densityStorm, thresholdStorm, minTimeBetweenLightning );
			m_Module.SetRain( forecastRain, timeRain, minDurationRain );
			m_Module.SetRainThresholds( minRainThresh, maxRainThresh, transitionRainThresh );
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
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 2, 1 );

		UIActionManager.CreateText( parent, "Quick Actions: ", "Instantly change the Weather" );
	
		Widget actionsWeather 	= UIActionManager.CreateGridSpacer( parent, 1, 4 );
		m_BtnQuickActionClear 	= UIActionManager.CreateButton( actionsWeather, "Clear", this, "OnClick_PresetClear" );
		m_BtnQuickActionCloudy 	= UIActionManager.CreateButton( actionsWeather, "Cloudy", this, "OnClick_PresetCloudy" );
		m_BtnQuickActionRainy 	= UIActionManager.CreateButton( actionsWeather, "Rainy", this, "OnClick_PresetRainy" );
		m_BtnQuickActionStorm 	= UIActionManager.CreateButton( actionsWeather, "Storm", this, "OnClick_PresetStorm" );

		Widget actionsDate 		= UIActionManager.CreateGridSpacer( parent, 1, 4 );
		m_BtnQuickActionNight 	= UIActionManager.CreateButton( actionsDate, "Night", this, "OnClick_PresetNight" );
		m_BtnQuickActionDusk 	= UIActionManager.CreateButton( actionsDate, "Dusk", this, "OnClick_PresetDusk" );
		m_BtnQuickActionDay 	= UIActionManager.CreateButton( actionsDate, "Day", this, "OnClick_PresetDay" );
		m_BtnQuickActionDawn 	= UIActionManager.CreateButton( actionsDate, "Dawn", this, "OnClick_PresetDawn" );

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
	}

	void OnClick_PresetClear( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_PresetsShown )
			return;

		float duration = 240;
		float transition = 0;

		m_Module.SetWind( Vector( 0, 0, 0 ), 0.1, 20 );
		m_Module.SetWindFunctionParams( 0, 1, 30 );
		m_Module.SetOvercast( 0, transition, duration );
		m_Module.SetRain( 0, transition, duration );
		m_Module.SetFog( 0, transition, duration );

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

		float windMaxSpeed = Math.RandomFloatInclusive(0.1,20);
		m_Module.SetWind( Vector( Math.RandomFloatInclusive(-1,1), 0, Math.RandomFloatInclusive(-1,1) ), Math.RandomFloatInclusive(0.1,windMaxSpeed * 0.5), windMaxSpeed );
		float windMin = Math.RandomFloatInclusive(0,0.8);
		m_Module.SetWindFunctionParams( windMin, Math.RandomFloatInclusive(windMin,1), Math.RandomFloatInclusive(0,30) );
		float thresholdStorm = Math.Max(m_SliderStormThreshold.GetCurrent() * 0.01, 0.7);
		//! Set overcast to below storm threshold
		m_Module.SetOvercast( Math.RandomFloat(0.5, thresholdStorm), transition, duration );
		m_Module.SetRain( 0, transition, duration );
		m_Module.SetFog( 0, transition, duration );
		m_Module.SetStorm( m_SliderStormDensity.GetCurrent() * 0.01, thresholdStorm, m_SliderMinTimeBetweenLightning.GetCurrent() );

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

		float windMaxSpeed = Math.RandomFloatInclusive(0.1,20);
		m_Module.SetWind( Vector( Math.RandomFloatInclusive(-1,1), 0, Math.RandomFloatInclusive(-1,1) ), Math.RandomFloatInclusive(0.1,windMaxSpeed * 0.5), windMaxSpeed );
		float windMin = Math.RandomFloatInclusive(0,0.8);
		m_Module.SetWindFunctionParams( windMin, Math.RandomFloatInclusive(windMin,1), Math.RandomFloatInclusive(0,30) );
		float thresholdStorm = Math.Max(m_SliderStormThreshold.GetCurrent() * 0.01, 0.7);
		//! Set overcast to below storm threshold
		m_Module.SetOvercast( Math.RandomFloat(0.5, thresholdStorm), transition, duration );
		m_Module.SetRainThresholds( 0.5, 1.0, 120.0 );
		m_Module.SetRain( Math.RandomFloatInclusive(0.5,1.0), transition, duration );
		m_Module.SetFog( Math.RandomFloatInclusive(0.0,1.0 - windMin), transition, duration );
		m_Module.SetStorm( m_SliderStormDensity.GetCurrent() * 0.01, thresholdStorm, m_SliderMinTimeBetweenLightning.GetCurrent() );

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

		float windMaxSpeed = Math.RandomFloatInclusive(0.1,20);
		m_Module.SetWind( Vector( Math.RandomFloatInclusive(-1,1), 0, Math.RandomFloatInclusive(-1,1) ), Math.RandomFloatInclusive(0.1,windMaxSpeed), windMaxSpeed );
		float windMin = Math.RandomFloatInclusive(0,1);
		m_Module.SetWindFunctionParams( windMin, Math.RandomFloatInclusive(windMin,1), Math.RandomFloatInclusive(0,30) );
		m_Module.SetOvercast( Math.RandomFloatInclusive(0.7,1.0), transition, duration );
		float minRainThresh = m_SliderRainOvercastMin.GetCurrent() * 0.01;
		m_Module.SetRain( Math.RandomFloatInclusive(minRainThresh,1.0), transition, duration );
		m_Module.SetFog( Math.RandomFloatInclusive(0.0,1.0 - windMin), transition, duration );
		m_Module.SetStorm( 1.0, 0.7, m_SliderMinTimeBetweenLightning.GetCurrent() );

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

	private void InitDateWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( parent, "Date: ", "Sets the in-game date" );
	
		Widget actions = UIActionManager.CreateGridSpacer( parent, 3, 2 );

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

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
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
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( parent, "Storm: ", "Sets the weather storm" );
	
		Widget actions = UIActionManager.CreatePanel( parent, 0x00000000, 50 );

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

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
	}

	private void InitFogWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( parent, "Fog: ", "Sets the weather fog phenomenon" );
	
		Widget actions = UIActionManager.CreatePanel( parent, 0x00000000, 35 );

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

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
	}

	private void InitRainWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( parent, "Rain: ", "Sets the weather rain phenomenon" );
	
		Widget actions = UIActionManager.CreatePanel( parent, 0x00000000, 35 );

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

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
	}

	private void InitRainThresholdWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( parent, "Rain Tresholds: ", "Sets the minimum, maximum overcast" );
	
		Widget actions = UIActionManager.CreatePanel( parent, 0x00000000, 35 );

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

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
	}

	private void InitOvercastWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( parent, "Clouds: ", "Sets the weather overcast phenomenon" );
	
		Widget actions = UIActionManager.CreatePanel( parent, 0x00000000, 35 );

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

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
	}

	private void InitWindDirectionSlider( Widget parent, out UIActionSlider action, string label )
	{
		action = UIActionManager.CreateSlider( parent, label, 0, 1, this, "OnChange_Wind" );
		action.SetCurrent( 0 );
		action.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		action.SetStepValue( 0.01 );
		action.SetMin( -1.0 );
		action.SetMax( 1.0 );
		action.SetWidgetWidth( action.GetLabelWidget(), 0.5 );
		action.SetWidgetWidth( action.GetSliderWidget(), 0.5 );
	}

	private void InitWindWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 4, 1 );

		UIActionManager.CreateText( parent, "Wind: ", "Sets the wind direction, speed and the maximum speed" );
	
		Widget sliders = UIActionManager.CreateGridSpacer( parent, 1, 2 );
		sliders.SetFlags( WidgetFlags.RALIGN );
		SetWidthPos( sliders, 0.95, 0.05 );
		InitWindDirectionSlider( sliders, m_SliderWindDirectionX, "Direction X" );
		InitWindDirectionSlider( sliders, m_SliderWindDirectionZ, "Direction Z" );
		sliders.Update();

		Widget actions = UIActionManager.CreatePanel( parent, 0x00000000, 35 );

		Widget actionsGrid = UIActionManager.CreateGridSpacer( actions, 1, 2 );

		m_EditWindSpeed = UIActionManager.CreateSlider( actionsGrid, "Speed", 0, 1, this );
		m_EditWindSpeed.SetCurrent( 0 );
		m_EditWindSpeed.SetStepValue( 0.05 );
		m_EditWindSpeed.SetMin( 0 );
		m_EditWindSpeed.SetMax( 1.0 );
		m_EditWindSpeed.SetWidgetWidth( m_EditWindSpeed.GetLabelWidget(), 0.6 );
		m_EditWindSpeed.SetWidgetWidth( m_EditWindSpeed.GetSliderWidget(), 0.6 );

		m_EditWindMaxSpeed = UIActionManager.CreateEditableText( actionsGrid, "Max Speed", this, "OnChange_UpdateWindMaxSpeed");
		m_EditWindMaxSpeed.SetOnlyNumbers( true );
		m_EditWindMaxSpeed.SetText( "0" );
		m_EditWindMaxSpeed.SetWidgetWidth( m_EditWindMaxSpeed.GetLabelWidget(), 0.6 );
		m_EditWindMaxSpeed.SetWidgetWidth( m_EditWindMaxSpeed.GetEditBoxWidget(), 0.6 );

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
	}

	void OnChange_UpdateWindMaxSpeed( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
		
		UpdateWindMaxSpeed();
	}

	void UpdateWindMaxSpeed()
	{
		float max = ToFloat(m_EditWindMaxSpeed.GetText());

		m_EditWindSpeed.SetMax( max );
	}

	void OnChange_Wind( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		float dirX = m_SliderWindDirectionX.GetCurrent();
		float dirZ = m_SliderWindDirectionZ.GetCurrent();

		vector dir = Vector( dirX, 0.0, dirZ ).Normalized();
		m_SliderWindDirectionX.SetCurrent( dir[0] );
		m_SliderWindDirectionZ.SetCurrent( dir[2] );
	}
	
	private void InitWindFuncWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( parent, "Wind Function: ", "Sets the wind function parameters" );

		Widget actions = UIActionManager.CreatePanel( parent, 0x00000000, 35 );

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

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
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
