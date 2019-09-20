class JMWeatherForm extends JMFormBase
{
	private static const int m_DaysInMonth [ 12 ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	private TextWidget m_TextPresetCount;
	private autoptr array< JMWeatherPresetWidget > m_WidgetsPreset;

	private Widget m_PanelLeft;
	private Widget m_PanelRight;

	private TextWidget m_TextList;
	private ButtonWidget m_ButtonList;

	private Widget m_PanelPresetManageActions;

	private UIActionEditableText m_EditTextPresetName;
	private UIActionEditableText m_EditTextPresetPermission;

	private UIActionButton m_ButtonPresetUpdate;
	private UIActionButton m_ButtonPresetRemove;
	private UIActionButton m_ButtonPresetUse;

	private Widget m_PanelWeatherActions;

	private UIActionSlider m_SliderStormDensity;
	private UIActionSlider m_SliderStormThreshold;
	private UIActionEditableText m_EditTextStormTimeout;

	private UIActionSlider m_SliderFogForecast;
	private UIActionEditableText m_EditFogInterpTime;
	private UIActionEditableText m_EditFogMinDuration;

	private UIActionSlider m_SliderRainForecast;
	private UIActionEditableText m_EditRainInterpTime;
	private UIActionEditableText m_EditRainMinDuration;

	private UIActionSlider m_SliderOvercastForecast;
	private UIActionEditableText m_EditOvercastInterpTime;
	private UIActionEditableText m_EditOvercastMinDuration;

	private UIActionSlider m_SliderWindDirectionX;
	private UIActionSlider m_SliderWindDirectionY;
	private UIActionSlider m_SliderWindDirectionZ;
	private UIActionEditableText m_EditWindSpeed;
	private UIActionEditableText m_EditWindMaxSpeed;

	private UIActionEditableText m_EditWindFuncMin;
	private UIActionEditableText m_EditWindFuncMax;
	private UIActionEditableText m_EditWindFuncSpeed;

	private bool m_PresetsShown;
	private bool m_IsCreatingPreset;
	private string m_SelectedPreset;
	private string m_RemovePreset;

	void JMWeatherForm()
	{
		m_WidgetsPreset = new array< JMWeatherPresetWidget >;
	}

	void ~JMWeatherForm()
	{
	}

	override void OnInit()
	{
		m_PanelLeft = layoutRoot.FindAnyWidget( "panel_left" );
		m_PanelRight = layoutRoot.FindAnyWidget( "panel_right" );

		m_TextList = TextWidget.Cast( m_PanelRight.FindAnyWidget( "actions_list_text" ) );
		m_ButtonList = ButtonWidget.Cast( m_PanelRight.FindAnyWidget( "actions_list_button" ) );

		InitLeftPanel( m_PanelLeft );
		InitRightPanel( m_PanelRight );

		HideList();
	}

	override void OnShow()
	{
		super.OnShow();

		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).CallLater( UpdatePresetList, 1500, true );

		UpdatePresetList();

		UpdateStates();
	}

	override void OnHide() 
	{
		super.OnHide();

		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).Remove( UpdatePresetList );
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

		CreateConfirmation_Two( "Confirm", "Do you wish to remove the preset " + preset, "No", "RemovePreset_No", "Yes", "RemovePreset_Yes" );
	}

	void RemovePreset_No()
	{
		// do nothing
	}

	void RemovePreset_Yes()
	{
		if ( m_SelectedPreset == m_RemovePreset )
		{
			m_SelectedPreset = "";
		}

		JMWeatherModule.Cast( module ).RemovePreset( m_RemovePreset );

		UpdateStates();
	}

	void UpdatePresetList()
	{
		JMWeatherModule mod;
		if ( !Class.CastTo( mod, module ) )
			return;

		if ( !mod.HasSettings() )
			return;

		array< ref JMWeatherPreset > presets = mod.GetPresets();

		for ( int i = 0; i < m_WidgetsPreset.Count(); i++ )
		{
			if ( i < presets.Count() )
			{
				m_WidgetsPreset[i].SetPreset( presets[i].Name, presets[i].Permission );
			} else if ( i == presets.Count() )
			{
				m_WidgetsPreset[i].SetCreateNew();
			} else
			{
				m_WidgetsPreset[i].SetPreset( "", "" );
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
		m_PanelLeft.Show( true );
		m_PanelRight.Show( true );

		SetWidthPos( m_PanelLeft, 0.25, 0 );
		SetWidthPos( m_PanelRight, 0.75, 0.25 );

		m_TextList.SetText( "Don't Use Presets" );

		m_PresetsShown = true;

		UpdateStates();
	}

	void HideList()
	{
		m_PanelLeft.Show( false );
		m_PanelRight.Show( true );

		SetWidthPos( m_PanelLeft, 0.0, 0.0 );
		SetWidthPos( m_PanelRight, 1.0, 0.0 );

		m_TextList.SetText( "Use Presets" );

		m_PresetsShown = false;

		UpdateStates();
	}

	private void UpdateActionState( UIActionBase action, string permission, bool shouldDisable = false, bool shouldHide = false )
	{
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

		if ( m_PresetsShown )
		{
			hasNotSelectedPreset = m_SelectedPreset == "";
		}

		UpdateActionState( m_SliderStormDensity, "Weather.Storm", hasNotSelectedPreset );
		UpdateActionState( m_SliderStormThreshold, "Weather.Storm", hasNotSelectedPreset );
		UpdateActionState( m_EditTextStormTimeout, "Weather.Storm", hasNotSelectedPreset );

		UpdateActionState( m_SliderFogForecast, "Weather.Fog", hasNotSelectedPreset );
		UpdateActionState( m_EditFogInterpTime, "Weather.Fog", hasNotSelectedPreset );
		UpdateActionState( m_EditFogMinDuration, "Weather.Fog", hasNotSelectedPreset );

		UpdateActionState( m_SliderRainForecast, "Weather.Rain", hasNotSelectedPreset );
		UpdateActionState( m_EditRainInterpTime, "Weather.Rain", hasNotSelectedPreset );
		UpdateActionState( m_EditRainMinDuration, "Weather.Rain", hasNotSelectedPreset );

		UpdateActionState( m_SliderOvercastForecast, "Weather.Overcast", hasNotSelectedPreset );
		UpdateActionState( m_EditOvercastInterpTime, "Weather.Overcast", hasNotSelectedPreset );
		UpdateActionState( m_EditOvercastMinDuration, "Weather.Overcast", hasNotSelectedPreset );

		UpdateActionState( m_SliderWindDirectionX, "Weather.Wind", hasNotSelectedPreset );
		UpdateActionState( m_SliderWindDirectionY, "Weather.Wind", hasNotSelectedPreset );
		UpdateActionState( m_SliderWindDirectionZ, "Weather.Wind", hasNotSelectedPreset );
		UpdateActionState( m_EditWindSpeed, "Weather.Wind", hasNotSelectedPreset );
		UpdateActionState( m_EditWindMaxSpeed, "Weather.Wind", hasNotSelectedPreset );

		UpdateActionState( m_EditWindFuncMin, "Weather.Wind.FunctionParams", hasNotSelectedPreset );
		UpdateActionState( m_EditWindFuncMax, "Weather.Wind.FunctionParams", hasNotSelectedPreset );
		UpdateActionState( m_EditWindFuncSpeed, "Weather.Wind.FunctionParams", hasNotSelectedPreset );

		bool hasNotSelectedPresetAndNotCreating = false;
		if ( !m_IsCreatingPreset )
		{
			hasNotSelectedPresetAndNotCreating = hasNotSelectedPreset;
		}

		UpdateActionState( m_EditTextPresetName, "Weather.Preset.Create", hasNotSelectedPresetAndNotCreating, !m_PresetsShown );
		UpdateActionState( m_EditTextPresetPermission, "Weather.Preset.Create", hasNotSelectedPresetAndNotCreating, !m_PresetsShown );
		UpdateActionState( m_ButtonPresetUpdate, "Weather.Preset.Update", false, !m_PresetsShown );
		UpdateActionState( m_ButtonPresetRemove, "Weather.Preset.Remove", m_IsCreatingPreset, !m_PresetsShown );
		UpdateActionState( m_ButtonPresetUse, "Weather.Preset", m_IsCreatingPreset, !m_PresetsShown );

		if ( m_IsCreatingPreset )
		{
			m_ButtonPresetUpdate.SetButton( "Create Preset" );
		} else
		{
			m_ButtonPresetUpdate.SetButton( "Update Preset" );

			array< ref JMWeatherPreset > presets = JMWeatherModule.Cast( module ).GetPresets();

			JMWeatherPreset preset;
			for ( int i = 0; i < presets.Count(); i++ )
			{
				if ( presets[i].Permission == m_SelectedPreset )
				{
					preset = presets[i];
					break;
				}
			}

			if ( preset )
			{
				SetUIActionValues( preset );
			}
		}
	}

	void SetUIActionValues( JMWeatherPreset preset )
	{
		m_EditTextPresetName.SetText( preset.Name );
		m_EditTextPresetPermission.SetText( preset.Permission );
		m_SliderStormDensity.SetCurrent( preset.Storm.Density * 100.0 );
		m_SliderStormThreshold.SetCurrent( preset.Storm.Threshold * 100.0 );
		m_EditTextStormTimeout.SetText( preset.Storm.TimeOut );
		m_SliderFogForecast.SetCurrent( preset.PFog.Forecast * 100.0 );
		m_EditFogInterpTime.SetText( preset.PFog.Time );
		m_EditFogMinDuration.SetText( preset.PFog.MinDuration );
		m_SliderRainForecast.SetCurrent( preset.PRain.Forecast * 100.0 );
		m_EditRainInterpTime.SetText( preset.PRain.Time );
		m_EditRainMinDuration.SetText( preset.PRain.MinDuration );
		m_SliderOvercastForecast.SetCurrent( preset.POvercast.Forecast * 100.0 );
		m_EditOvercastInterpTime.SetText( preset.POvercast.Time );
		m_EditOvercastMinDuration.SetText( preset.POvercast.MinDuration );
		m_SliderWindDirectionX.SetCurrent( preset.Wind.Dir[0] );
		m_SliderWindDirectionY.SetCurrent( preset.Wind.Dir[1] );
		m_SliderWindDirectionZ.SetCurrent( preset.Wind.Dir[2] );
		m_EditWindSpeed.SetText( preset.Wind.Speed );
		m_EditWindMaxSpeed.SetText( preset.Wind.MaxSpeed );
		m_EditWindFuncMin.SetText( preset.WindFunc.Min );
		m_EditWindFuncMax.SetText( preset.WindFunc.Max );
		m_EditWindFuncSpeed.SetText( preset.WindFunc.Speed );
	}

	void GetUIActionValues( out JMWeatherPreset preset )
	{
		preset.Name = m_EditTextPresetName.GetText();
		preset.Permission = m_EditTextPresetPermission.GetText();
		preset.Storm.Density = m_SliderStormDensity.GetCurrent() * 0.01;
		preset.Storm.Threshold = m_SliderStormThreshold.GetCurrent() * 0.01;
		preset.Storm.TimeOut = ToFloat( m_EditTextStormTimeout.GetText() );
		preset.PFog.Forecast = m_SliderFogForecast.GetCurrent() * 0.01;
		preset.PFog.Time = ToFloat( m_EditFogInterpTime.GetText() );
		preset.PFog.MinDuration = ToFloat( m_EditFogMinDuration.GetText() );
		preset.PRain.Forecast = m_SliderRainForecast.GetCurrent() * 0.01;
		preset.PRain.Time = ToFloat( m_EditRainInterpTime.GetText() );
		preset.PRain.MinDuration = ToFloat( m_EditRainMinDuration.GetText() );
		preset.POvercast.Forecast = m_SliderOvercastForecast.GetCurrent() * 0.01;
		preset.POvercast.Time = ToFloat( m_EditOvercastInterpTime.GetText() );
		preset.POvercast.MinDuration = ToFloat( m_EditOvercastMinDuration.GetText() );
		preset.Wind.Dir[0] = m_SliderWindDirectionX.GetCurrent();
		preset.Wind.Dir[1] = m_SliderWindDirectionY.GetCurrent();
		preset.Wind.Dir[2] = m_SliderWindDirectionZ.GetCurrent();
		preset.Wind.Speed = ToFloat( m_EditWindSpeed.GetText() );
		preset.Wind.MaxSpeed = ToFloat( m_EditWindMaxSpeed.GetText() );
		preset.WindFunc.Min = ToFloat( m_EditWindFuncMin.GetText() );
		preset.WindFunc.Max = ToFloat( m_EditWindFuncMax.GetText() );
		preset.WindFunc.Speed = ToFloat( m_EditWindFuncSpeed.GetText() );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
		{
			return false;
		}

		if ( w == m_ButtonList )
		{
			if ( m_PresetsShown )
			{
				HideList();
			} else
			{
				ShowList();
			}
			return true;
		}

		return false;
	}

	void InitLeftPanel( Widget parent )
	{
		m_TextPresetCount = TextWidget.Cast( layoutRoot.FindAnyWidget( "count" ) );

		ref Widget rwWidget = NULL;
		ref JMWeatherPresetWidget rwScript = NULL;

		for ( int i = 0; i < 10; i++ )
		{
			GridSpacerWidget gsw = GridSpacerWidget.Cast( parent.FindAnyWidget( "list_0" + i ) );
			
			if ( !gsw )
				continue;

			for ( int j = 0; j < 100; j++ )
			{
				rwWidget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/weather_preset_widget.layout", gsw );
				
				if ( rwWidget == NULL )
				{
					continue;
				}

				rwWidget.GetScript( rwScript );

				if ( rwScript == NULL )
				{
					continue;
				}

				rwScript.Init( this );

				rwScript.SetPreset( "", "" );

				m_WidgetsPreset.Insert( rwScript );
			}
		}
	}

	void InitRightPanel( Widget parent )
	{
		Widget tParent = parent.FindAnyWidget( "actions_wrapper" );

		m_PanelPresetManageActions = UIActionManager.CreateGridSpacer( tParent, 2, 1 );
		InitPresetDetails( m_PanelPresetManageActions );
		InitPresetRun( m_PanelPresetManageActions );

		m_PanelWeatherActions = UIActionManager.CreateGridSpacer( tParent, 5, 1 );
		
		InitStormWidgets( m_PanelWeatherActions );
		InitFogWidgets( m_PanelWeatherActions );
		InitRainWidgets( m_PanelWeatherActions );
		InitOvercastWidgets( m_PanelWeatherActions );
		InitWindWidgets( m_PanelWeatherActions );
	}

	private void InitPresetDetails( Widget actionsParent )
	{
		Widget actions = UIActionManager.CreateGridSpacer( actionsParent, 1, 2 );

		m_EditTextPresetName = UIActionManager.CreateEditableText( actions, "Name", this, "OnChange_PresetDetails" );
		m_EditTextPresetName.SetOnlyNumbers( false );
		m_EditTextPresetName.SetText( "" );
		m_EditTextPresetName.SetPosition( 0 );
		m_EditTextPresetName.SetWidth( 0.5 );
		m_EditTextPresetName.SetWidgetWidth( m_EditTextPresetName.GetLabelWidget(), 0.6 );
		m_EditTextPresetName.SetWidgetWidth( m_EditTextPresetName.GetEditBoxWidget(), 0.4 );
		m_EditTextPresetName.SetWidgetPosition( m_EditTextPresetName.GetEditBoxWidget(), 0.6 );

		m_EditTextPresetPermission = UIActionManager.CreateEditableText( actions, "Permission", this, "OnChange_PresetDetails" );
		m_EditTextPresetPermission.SetOnlyNumbers( false );
		m_EditTextPresetPermission.SetText( "" );
		m_EditTextPresetPermission.SetPosition( 0.5 );
		m_EditTextPresetPermission.SetWidth( 0.5 );
		m_EditTextPresetPermission.SetWidgetWidth( m_EditTextPresetPermission.GetLabelWidget(), 0.7 );
		m_EditTextPresetPermission.SetWidgetWidth( m_EditTextPresetPermission.GetEditBoxWidget(), 0.3 );
		m_EditTextPresetPermission.SetWidgetPosition( m_EditTextPresetPermission.GetEditBoxWidget(), 0.7 );
	}

	void OnChange_PresetDetails( UIEvent eid, ref UIActionBase action )
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

	void OnClick_PresetUpdate( UIEvent eid, ref UIActionBase action )
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
			preset.Permission = m_EditTextPresetPermission.GetText();

			if ( preset.Permission == "" )
			{
				return;
			}
			if ( preset.Name == "" )
			{
				return;
			}
		} else
		{
			array< ref JMWeatherPreset > presets = JMWeatherModule.Cast( module ).GetPresets();

			for ( int i = 0; i < presets.Count(); i++ )
			{
				if ( presets[i].Permission == m_SelectedPreset )
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
			JMWeatherModule.Cast( module ).CreatePreset( preset );

			m_IsCreatingPreset = false;

			m_SelectedPreset = preset.Permission;
		} else
		{
			JMWeatherModule.Cast( module ).UpdatePreset( preset );
		}

		UpdateStates();
	}

	void OnClick_PresetRun( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_PresetsShown )
			return;

		JMWeatherModule.Cast( module ).UsePreset( m_SelectedPreset );
	}

	void OnClick_PresetRemove( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_PresetsShown )
			return;

		RemovePreset( m_SelectedPreset );
	}

	private void InitStormWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( parent, "Storm: ", "Modify the weather storm" );
	
		Widget actions = UIActionManager.CreatePanel( parent, 0x00000000, 35 );

		m_SliderStormDensity = UIActionManager.CreateSlider( actions, "Density", 0, 1, this, "OnChange_Storm" );
		m_SliderStormDensity.SetCurrent( 0 );
		m_SliderStormDensity.SetAppend( "%" );
		m_SliderStormDensity.SetStepValue( 0.1 );
		m_SliderStormDensity.SetMin( 0 );
		m_SliderStormDensity.SetMax( 100 );
		m_SliderStormDensity.SetPosition( 0.05 );
		m_SliderStormDensity.SetWidth( 0.3 );
		m_SliderStormDensity.SetWidgetWidth( m_SliderStormDensity.GetLabelWidget(), 0.57 );
		m_SliderStormDensity.SetWidgetWidth( m_SliderStormDensity.GetSliderWidget(), 0.43 );
		m_SliderStormDensity.SetWidgetPosition( m_SliderStormDensity.GetSliderWidget(), 0.6 );

		m_SliderStormThreshold = UIActionManager.CreateSlider( actions, "Min Overcast", 0, 1, this, "OnChange_Storm" );
		m_SliderStormThreshold.SetCurrent( 0 );
		m_SliderStormThreshold.SetAppend( "%" );
		m_SliderStormThreshold.SetStepValue( 0.1 );
		m_SliderStormThreshold.SetMin( 0 );
		m_SliderStormThreshold.SetMax( 100 );
		m_SliderStormThreshold.SetPosition( 0.35 );
		m_SliderStormThreshold.SetWidth( 0.4 );
		m_SliderStormThreshold.SetWidgetWidth( m_SliderStormThreshold.GetLabelWidget(), 0.6 );
		m_SliderStormThreshold.SetWidgetWidth( m_SliderStormThreshold.GetSliderWidget(), 0.4 );
		m_SliderStormThreshold.SetWidgetPosition( m_SliderStormThreshold.GetSliderWidget(), 0.6 );

		m_EditTextStormTimeout = UIActionManager.CreateEditableText( actions, "Timeout", this, "OnChange_Storm" );
		m_EditTextStormTimeout.SetOnlyNumbers( true );
		m_EditTextStormTimeout.SetText( "60" );
		m_EditTextStormTimeout.SetPosition( 0.75 );
		m_EditTextStormTimeout.SetWidth( 0.25 );
		m_EditTextStormTimeout.SetWidgetWidth( m_EditTextStormTimeout.GetLabelWidget(), 0.6 );
		m_EditTextStormTimeout.SetWidgetWidth( m_EditTextStormTimeout.GetEditBoxWidget(), 0.4 );
		m_EditTextStormTimeout.SetWidgetPosition( m_EditTextStormTimeout.GetEditBoxWidget(), 0.6 );

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
	}

	void OnChange_Storm( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_PresetsShown )
			return;

		float density = m_SliderStormDensity.GetCurrent() * 0.01;
		float threshold = m_SliderStormThreshold.GetCurrent() * 0.01;
		float timeout = m_EditTextStormTimeout.GetText().ToFloat();

		JMWeatherModule.Cast( module ).SetStorm( density, threshold, timeout );
	}

	private void InitFogWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( parent, "Fog: ", "Modify the weather fog phenomenon" );
	
		Widget actions = UIActionManager.CreatePanel( parent, 0x00000000, 35 );

		m_SliderFogForecast = UIActionManager.CreateSlider( actions, "Forecast", 0, 1, this, "OnChange_Fog" );
		m_SliderFogForecast.SetCurrent( 0 );
		m_SliderFogForecast.SetAppend( "%" );
		m_SliderFogForecast.SetStepValue( 0.1 );
		m_SliderFogForecast.SetMin( 0 );
		m_SliderFogForecast.SetMax( 100 );
		m_SliderFogForecast.SetPosition( 0.05 );
		m_SliderFogForecast.SetWidth( 0.35 );
		m_SliderFogForecast.SetWidgetWidth( m_SliderFogForecast.GetLabelWidget(), 0.5 );
		m_SliderFogForecast.SetWidgetWidth( m_SliderFogForecast.GetSliderWidget(), 0.5 );

		m_EditFogInterpTime = UIActionManager.CreateEditableText( actions, "Time", this, "OnChange_Fog" );
		m_EditFogInterpTime.SetOnlyNumbers( true );
		m_EditFogInterpTime.SetText( "0" );
		m_EditFogInterpTime.SetPosition( 0.4 );
		m_EditFogInterpTime.SetWidth( 0.2 );
		m_EditFogInterpTime.SetWidgetWidth( m_EditFogInterpTime.GetLabelWidget(), 0.6 );
		m_EditFogInterpTime.SetWidgetWidth( m_EditFogInterpTime.GetEditBoxWidget(), 0.4 );
		m_EditFogInterpTime.SetWidgetPosition( m_EditFogInterpTime.GetEditBoxWidget(), 0.6 );

		m_EditFogMinDuration = UIActionManager.CreateEditableText( actions, "Min Duration", this, "OnChange_Fog" );
		m_EditFogMinDuration.SetOnlyNumbers( true );
		m_EditFogMinDuration.SetText( "0" );
		m_EditFogMinDuration.SetPosition( 0.6 );
		m_EditFogMinDuration.SetWidth( 0.4 );
		m_EditFogMinDuration.SetWidgetWidth( m_EditFogMinDuration.GetLabelWidget(), 0.7 );
		m_EditFogMinDuration.SetWidgetWidth( m_EditFogMinDuration.GetEditBoxWidget(), 0.3 );
		m_EditFogMinDuration.SetWidgetPosition( m_EditFogMinDuration.GetEditBoxWidget(), 0.7 );

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
	}

	void OnChange_Fog( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_PresetsShown )
			return;

		float forecast = m_SliderFogForecast.GetCurrent() * 0.01;
		float time = m_EditFogInterpTime.GetCurrent();
		float minDuration = m_EditFogMinDuration.GetText().ToFloat();

		JMWeatherModule.Cast( module ).SetFog( forecast, time, minDuration );
	}

	private void InitRainWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( parent, "Rain: ", "Modify the weather rain phenomenon" );
	
		Widget actions = UIActionManager.CreatePanel( parent, 0x00000000, 35 );

		m_SliderRainForecast = UIActionManager.CreateSlider( actions, "Forecast", 0, 1, this, "OnChange_Rain" );
		m_SliderRainForecast.SetCurrent( 0 );
		m_SliderRainForecast.SetAppend( "%" );
		m_SliderRainForecast.SetStepValue( 0.1 );
		m_SliderRainForecast.SetMin( 0 );
		m_SliderRainForecast.SetMax( 100 );
		m_SliderRainForecast.SetPosition( 0.05 );
		m_SliderRainForecast.SetWidth( 0.35 );
		m_SliderRainForecast.SetWidgetWidth( m_SliderRainForecast.GetLabelWidget(), 0.5 );
		m_SliderRainForecast.SetWidgetWidth( m_SliderRainForecast.GetSliderWidget(), 0.5 );

		m_EditRainInterpTime = UIActionManager.CreateEditableText( actions, "Time", this, "OnChange_Rain" );
		m_EditRainInterpTime.SetOnlyNumbers( true );
		m_EditRainInterpTime.SetText( "0" );
		m_EditRainInterpTime.SetPosition( 0.4 );
		m_EditRainInterpTime.SetWidth( 0.2 );
		m_EditRainInterpTime.SetWidgetWidth( m_EditRainInterpTime.GetLabelWidget(), 0.6 );
		m_EditRainInterpTime.SetWidgetWidth( m_EditRainInterpTime.GetEditBoxWidget(), 0.4 );
		m_EditRainInterpTime.SetWidgetPosition( m_EditRainInterpTime.GetEditBoxWidget(), 0.6 );

		m_EditRainMinDuration = UIActionManager.CreateEditableText( actions, "Min Duration", this, "OnChange_Rain" );
		m_EditRainMinDuration.SetOnlyNumbers( true );
		m_EditRainMinDuration.SetText( "0" );
		m_EditRainMinDuration.SetPosition( 0.6 );
		m_EditRainMinDuration.SetWidth( 0.4 );
		m_EditRainMinDuration.SetWidgetWidth( m_EditRainMinDuration.GetLabelWidget(), 0.7 );
		m_EditRainMinDuration.SetWidgetWidth( m_EditRainMinDuration.GetEditBoxWidget(), 0.3 );
		m_EditRainMinDuration.SetWidgetPosition( m_EditRainMinDuration.GetEditBoxWidget(), 0.7 );

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
	}

	void OnChange_Rain( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_PresetsShown )
			return;

		float forecast = m_SliderRainForecast.GetCurrent() * 0.01;
		float time = m_EditRainInterpTime.GetCurrent();
		float minDuration = m_EditRainMinDuration.GetText().ToFloat();

		JMWeatherModule.Cast( module ).SetRain( forecast, time, minDuration );
	}

	private void InitOvercastWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( parent, "Overcast: ", "Modify the weather overcast phenomenon" );
	
		Widget actions = UIActionManager.CreatePanel( parent, 0x00000000, 35 );

		m_SliderOvercastForecast = UIActionManager.CreateSlider( actions, "Forecast", 0, 1, this, "OnChange_Overcast" );
		m_SliderOvercastForecast.SetCurrent( 0 );
		m_SliderOvercastForecast.SetAppend( "%" );
		m_SliderOvercastForecast.SetStepValue( 0.1 );
		m_SliderOvercastForecast.SetMin( 0 );
		m_SliderOvercastForecast.SetMax( 100 );
		m_SliderOvercastForecast.SetPosition( 0.05 );
		m_SliderOvercastForecast.SetWidth( 0.35 );
		m_SliderOvercastForecast.SetWidgetWidth( m_SliderOvercastForecast.GetLabelWidget(), 0.5 );
		m_SliderOvercastForecast.SetWidgetWidth( m_SliderOvercastForecast.GetSliderWidget(), 0.5 );

		m_EditOvercastInterpTime = UIActionManager.CreateEditableText( actions, "Time", this, "OnChange_Overcast" );
		m_EditOvercastInterpTime.SetOnlyNumbers( true );
		m_EditOvercastInterpTime.SetText( "0" );
		m_EditOvercastInterpTime.SetPosition( 0.4 );
		m_EditOvercastInterpTime.SetWidth( 0.2 );
		m_EditOvercastInterpTime.SetWidgetWidth( m_EditOvercastInterpTime.GetLabelWidget(), 0.6 );
		m_EditOvercastInterpTime.SetWidgetWidth( m_EditOvercastInterpTime.GetEditBoxWidget(), 0.4 );
		m_EditOvercastInterpTime.SetWidgetPosition( m_EditOvercastInterpTime.GetEditBoxWidget(), 0.6 );

		m_EditOvercastMinDuration = UIActionManager.CreateEditableText( actions, "Min Duration", this, "OnChange_Overcast" );
		m_EditOvercastMinDuration.SetOnlyNumbers( true );
		m_EditOvercastMinDuration.SetText( "0" );
		m_EditOvercastMinDuration.SetPosition( 0.6 );
		m_EditOvercastMinDuration.SetWidth( 0.4 );
		m_EditOvercastMinDuration.SetWidgetWidth( m_EditOvercastMinDuration.GetLabelWidget(), 0.7 );
		m_EditOvercastMinDuration.SetWidgetWidth( m_EditOvercastMinDuration.GetEditBoxWidget(), 0.3 );
		m_EditOvercastMinDuration.SetWidgetPosition( m_EditOvercastMinDuration.GetEditBoxWidget(), 0.7 );

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
	}

	void OnChange_Overcast( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_PresetsShown )
			return;

		float forecast = m_SliderOvercastForecast.GetCurrent() * 0.01;
		float time = m_EditOvercastInterpTime.GetCurrent();
		float minDuration = m_EditOvercastMinDuration.GetText().ToFloat();

		JMWeatherModule.Cast( module ).SetOvercast( forecast, time, minDuration );
	}

	private void InitWindDirectionSlider( Widget parent, out UIActionSlider action, string label )
	{
		action = UIActionManager.CreateSlider( parent, label, 0, 1, this, "OnChange_Wind" );
		action.SetCurrent( 0 );
		action.SetAppend( "%" );
		action.SetStepValue( 0.01 );
		action.SetMin( -1.0 );
		action.SetMax( 1.0 );
		action.SetWidgetWidth( action.GetLabelWidget(), 0.4 );
		action.SetWidgetWidth( action.GetSliderWidget(), 0.6 );
		action.SetWidgetPosition( action.GetSliderWidget(), 0.4 );
	}

	private void InitWindWidgets( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 4, 1 );

		UIActionManager.CreateText( parent, "Wind: ", "Modify the wind direction, speed and the maximum speed" );
	
		Widget sliders = UIActionManager.CreateGridSpacer( parent, 1, 3 );
		InitWindDirectionSlider( sliders, m_SliderWindDirectionX, "Dir X" );
		InitWindDirectionSlider( sliders, m_SliderWindDirectionY, "Dir Y" );
		InitWindDirectionSlider( sliders, m_SliderWindDirectionZ, "Dir Z" );

		Widget actions = UIActionManager.CreatePanel( parent, 0x00000000, 35 );

		m_EditWindSpeed = UIActionManager.CreateEditableText( actions, "Speed", this, "OnChange_Wind" );
		m_EditWindSpeed.SetOnlyNumbers( true );
		m_EditWindSpeed.SetText( "0" );
		m_EditWindSpeed.SetPosition( 0.0 );
		m_EditWindSpeed.SetWidth( 0.5 );
		m_EditWindSpeed.SetWidgetWidth( m_EditWindSpeed.GetLabelWidget(), 0.7 );
		m_EditWindSpeed.SetWidgetWidth( m_EditWindSpeed.GetEditBoxWidget(), 0.3 );
		m_EditWindSpeed.SetWidgetPosition( m_EditWindSpeed.GetEditBoxWidget(), 0.7 );

		m_EditWindMaxSpeed = UIActionManager.CreateEditableText( actions, "Maximum Speed", this, "OnChange_Wind" );
		m_EditWindMaxSpeed.SetOnlyNumbers( true );
		m_EditWindMaxSpeed.SetText( "0" );
		m_EditWindMaxSpeed.SetPosition( 0.5 );
		m_EditWindMaxSpeed.SetWidth( 0.5 );
		m_EditWindMaxSpeed.SetWidgetWidth( m_EditWindMaxSpeed.GetLabelWidget(), 0.7 );
		m_EditWindMaxSpeed.SetWidgetWidth( m_EditWindMaxSpeed.GetEditBoxWidget(), 0.3 );
		m_EditWindMaxSpeed.SetWidgetPosition( m_EditWindMaxSpeed.GetEditBoxWidget(), 0.7 );

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
	}

	void OnChange_Wind( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		float dirX = m_SliderWindDirectionX.GetCurrent();
		float dirY = m_SliderWindDirectionY.GetCurrent();
		float dirZ = m_SliderWindDirectionZ.GetCurrent();

		vector dir = Vector( dirX, dirY, dirZ ).Normalized();
		m_SliderWindDirectionX.SetCurrent( dir[0] );
		m_SliderWindDirectionY.SetCurrent( dir[1] );
		m_SliderWindDirectionZ.SetCurrent( dir[2] );

		if ( m_PresetsShown )
			return;

		float speed = m_EditWindSpeed.GetText().ToFloat();
		float maxSpeed = m_EditWindMaxSpeed.GetText().ToFloat();

		JMWeatherModule.Cast( module ).SetWind( dir, speed, maxSpeed );
	}
}