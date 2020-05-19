class JMCameraForm extends JMFormBase 
{
	private Widget m_ActionsWrapper;

	private UIActionSlider m_SliderBlurStrength;
	private UIActionSlider m_SliderFocusDistance;
	private UIActionSlider m_SliderFocalLength;
	private UIActionSlider m_SliderFocalNear;
	private UIActionSlider m_SliderExposure;

	private JMCameraModule m_Module;

	void JMCameraForm()
	{
	}	
	
	void ~JMCameraForm()
	{
	}

	protected override bool SetModule( ref JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}
	
	override void OnInit()
	{
		m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );

		Widget actions = m_ActionsWrapper;// UIActionManager.CreateGridSpacer( m_ActionsWrapper, 5, 1 );

		m_SliderBlurStrength = UIActionManager.CreateSlider( actions, "Blur: ", 0, 1, this, "OnChange_Blur" );
		m_SliderBlurStrength.SetCurrent( 0 );
		m_SliderBlurStrength.SetAppend( "%" );
		m_SliderBlurStrength.SetStepValue( 0.1 );
		m_SliderBlurStrength.SetMin( 0.0 );
		m_SliderBlurStrength.SetMax( 100.0 );
		m_SliderBlurStrength.SetPosition( 0.0 );
		m_SliderBlurStrength.SetWidth( 1.0 );
		m_SliderBlurStrength.SetWidgetWidth( m_SliderBlurStrength.GetLabelWidget(), 0.3 );
		m_SliderBlurStrength.SetWidgetWidth( m_SliderBlurStrength.GetSliderWidget(), 0.7 );

		m_SliderFocusDistance = UIActionManager.CreateSlider( actions, "Focus: ", 0, 1, this, "OnChange_Focus" );
		m_SliderFocusDistance.SetCurrent( 0 );
		m_SliderFocusDistance.SetAppend( "m" );
		m_SliderFocusDistance.SetStepValue( 0.1 );
		m_SliderFocusDistance.SetMin( 0.0 );
		m_SliderFocusDistance.SetMax( 1000.0 );
		m_SliderFocusDistance.SetPosition( 0.0 );
		m_SliderFocusDistance.SetWidth( 1.0 );
		m_SliderFocusDistance.SetWidgetWidth( m_SliderFocusDistance.GetLabelWidget(), 0.3 );
		m_SliderFocusDistance.SetWidgetWidth( m_SliderFocusDistance.GetSliderWidget(), 0.7 );

		m_SliderFocalLength = UIActionManager.CreateSlider( actions, "Focal (L): ", 0, 1, this, "OnChange_FocalLength" );
		m_SliderFocalLength.SetCurrent( 0 );
		m_SliderFocalLength.SetAppend( "m" );
		m_SliderFocalLength.SetStepValue( 0.1 );
		m_SliderFocalLength.SetMin( 0.0 );
		m_SliderFocalLength.SetMax( 1000.0 );
		m_SliderFocalLength.SetPosition( 0.0 );
		m_SliderFocalLength.SetWidth( 1.0 );
		m_SliderFocalLength.SetWidgetWidth( m_SliderFocalLength.GetLabelWidget(), 0.3 );
		m_SliderFocalLength.SetWidgetWidth( m_SliderFocalLength.GetSliderWidget(), 0.7 );

		m_SliderFocalNear = UIActionManager.CreateSlider( actions, "Focal (N): ", 0, 1, this, "OnChange_FocalNear" );
		m_SliderFocalNear.SetCurrent( 0 );
		m_SliderFocalNear.SetAppend( "m" );
		m_SliderFocalNear.SetStepValue( 0.1 );
		m_SliderFocalNear.SetMin( 0.0 );
		m_SliderFocalNear.SetMax( 1000.0 );
		m_SliderFocalNear.SetPosition( 0.0 );
		m_SliderFocalNear.SetWidth( 1.0 );
		m_SliderFocalNear.SetWidgetWidth( m_SliderFocalNear.GetLabelWidget(), 0.3 );
		m_SliderFocalNear.SetWidgetWidth( m_SliderFocalNear.GetSliderWidget(), 0.7 );

		m_SliderExposure = UIActionManager.CreateSlider( actions, "Exposure: ", 0, 1, this, "OnChange_Exposure" );
		m_SliderExposure.SetCurrent( 0.5 );
		m_SliderExposure.SetAppend( "" );
		m_SliderExposure.SetStepValue( 0.1 );
		m_SliderExposure.SetMin( -5.0 );
		m_SliderExposure.SetMax( 5.0 );
		m_SliderExposure.SetPosition( 0.0 );
		m_SliderExposure.SetWidth( 1.0 );
		m_SliderExposure.SetWidgetWidth( m_SliderExposure.GetLabelWidget(), 0.3 );
		m_SliderExposure.SetWidgetWidth( m_SliderExposure.GetSliderWidget(), 0.7 );
	}

	void OnSliderUpdate()
	{
		if ( CAMERA_BLUR == 0 )
		{
			CAMERA_DOF = false;
			PPEffects.ResetDOFOverride();
		} else
		{
			CAMERA_DOF = true;
		}

		if ( CAMERA_FDIST == 0 )
		{
			CAMERA_AFOCUS = true;
		} else
		{
			CAMERA_AFOCUS = false;
		}
	}

	void OnChange_Blur( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CAMERA_BLUR = action.GetCurrent();

		OnSliderUpdate();
	}

	void OnChange_Focus( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CAMERA_FDIST = action.GetCurrent();

		OnSliderUpdate();
	}

	void OnChange_FocalLength( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CAMERA_FLENGTH = action.GetCurrent();

		OnSliderUpdate();
	}

	void OnChange_FocalNear( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CAMERA_FNEAR = action.GetCurrent();

		OnSliderUpdate();
	}

	void OnChange_Exposure( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		EXPOSURE = action.GetCurrent();

		OnSliderUpdate();
	}
}