class JMWeatherForm extends JMFormBase
{
	private static const int m_DaysInMonth [ 12 ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	private TextWidget m_TextPresetCount;
	private autoptr array< JMWeatherPresetWidget > m_WidgetsPreset;

	private Widget m_PanelLeft;
	private Widget m_PanelRight;

	private TextWidget m_TextList;
	private ButtonWidget m_ButtonList;

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
	}

	override void OnHide() 
	{
		super.OnHide();

		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).Remove( UpdatePresetList );
	}

	void CreateNew()
	{
		m_TextList.SetText( "Creating New " );
	}

	void SetSelectedPreset( string preset )
	{
		m_SelectedPreset = preset;

		m_TextList.SetText( "Selected: " + m_SelectedPreset );
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
		// m_RemovePreset
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
	}

	void HideList()
	{
		m_PanelLeft.Show( false );
		m_PanelRight.Show( true );

		SetWidthPos( m_PanelLeft, 0.0, 0.0 );
		SetWidthPos( m_PanelRight, 1.0, 0.0 );

		m_TextList.SetText( "Use Presets" );

		m_PresetsShown = false;
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
		m_PanelWeatherActions = UIActionManager.CreateGridSpacer( parent.FindAnyWidget( "actions_wrapper" ), 4, 1 );
		
		InitStormWidgets( m_PanelWeatherActions );
		InitFogWidgets( m_PanelWeatherActions );
		InitRainWidgets( m_PanelWeatherActions );
		InitOvercastWidgets( m_PanelWeatherActions );
		InitWindWidgets( m_PanelWeatherActions );
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

		if ( m_PresetsShown )
			return;

		float dirX = m_SliderWindDirectionX.GetCurrent();
		float dirY = m_SliderWindDirectionY.GetCurrent();
		float dirZ = m_SliderWindDirectionZ.GetCurrent();

		vector dir = Vector( dirX, dirY, dirZ ).Normalized();
		m_SliderWindDirectionX.SetCurrent( dir[0] );
		m_SliderWindDirectionY.SetCurrent( dir[1] );
		m_SliderWindDirectionZ.SetCurrent( dir[2] );

		float speed = m_EditWindSpeed.GetText().ToFloat();
		float maxSpeed = m_EditWindMaxSpeed.GetText().ToFloat();

		JMWeatherModule.Cast( module ).SetWind( dir, speed, maxSpeed );
	}
}