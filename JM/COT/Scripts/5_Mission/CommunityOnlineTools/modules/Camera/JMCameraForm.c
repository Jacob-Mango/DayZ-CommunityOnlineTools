class JMCameraForm: JMFormBase 
{
	private UIActionScroller m_sclr_MainActions;

	private UIActionSlider m_SliderBlurStrength;
	private UIActionSlider m_SliderFocusDistance;
	private UIActionSlider m_SliderFocalLength;
	private UIActionSlider m_SliderFocalNear;
	private UIActionSlider m_SliderExposure;
	private UIActionSlider m_SliderChromAbb;
	private UIActionSlider m_SliderVignette;
	private UIActionSlider m_SliderSpeed;
	private UIActionCheckbox m_EnableFullmapCamera;
	private UIActionCheckbox m_1stPersonADS_HideScope;
	private UIActionCheckbox m_HideGrass;

	private UIActionCheckbox m_ToggleSmooth;
	private UIActionEditableText m_TravelTime;
	private UIActionEditableVector m_Positon;
	private UIActionNavigateButton m_PositionList;

	private int m_PositionID;
	private ref TFloatArray m_Times;
	private ref TBoolArray m_IsSmooth;
	private ref TVectorArray m_Positions;

	private JMCameraModule m_Module;

	void JMCameraForm()
	{
		m_Times 	= new TFloatArray;
		m_IsSmooth 	= new TBoolArray;
		m_Positions = new TVectorArray;
	}

	override void OnHide()
	{
		m_Module.m_Times 	 = m_Times;
		m_Module.m_IsSmooth  = m_IsSmooth;
		m_Module.m_Positions = m_Positions;
		if (m_EnableFullmapCamera)
			m_Module.m_EnableFullmapCamera = m_EnableFullmapCamera.IsChecked();
		if (m_HideGrass)
			m_Module.m_HideGrass = m_HideGrass.IsChecked();
		if (m_SliderBlurStrength)
			m_Module.m_BlurStrength = m_SliderBlurStrength.GetCurrent();
		if (m_SliderFocusDistance)
			m_Module.m_FocusDistance = m_SliderFocusDistance.GetCurrent();
		if (m_SliderFocalLength)
			m_Module.m_FocalLength = m_SliderFocalLength.GetCurrent();
		if (m_SliderFocalNear)
			m_Module.m_FocalNear = m_SliderFocalNear.GetCurrent();
		if (m_SliderExposure)
			m_Module.m_Exposure = m_SliderExposure.GetCurrent();
		if (m_SliderChromAbb)
			m_Module.m_ChromAbb = m_SliderChromAbb.GetCurrent();
		if (m_SliderVignette)
			m_Module.m_Vignette = m_SliderVignette.GetCurrent();

		super.OnHide();
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}
	
	override void OnInit()
	{
		m_sclr_MainActions = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
		Widget actions = m_sclr_MainActions.GetContentWidget();
		
		Widget gridCategories = UIActionManager.CreateGridSpacer( actions, 1, 2 );

		Widget gridSliders  = UIActionManager.CreateGridSpacer( gridCategories, 7, 1 );
		Widget gridActions = UIActionManager.CreateGridSpacer( gridCategories, 5, 1 );
		
		m_EnableFullmapCamera = UIActionManager.CreateCheckbox( gridActions, "Enable fullmap freecam update", this, "OnClick_EnableFullmap", m_Module.m_EnableFullmapCamera );

		m_1stPersonADS_HideScope = UIActionManager.CreateCheckbox( gridActions, "Hide scope in 1st person spectate ADS", this, "OnClick_1stPersonADS_HideScope", GetCurrentCamera1stPersonADSHideScope() );
		
		m_HideGrass = UIActionManager.CreateCheckbox( gridActions, "Hide grass in freecam", this, "OnClick_HideGrass", m_Module.m_HideGrass );
		
		UIActionManager.CreateButton( gridActions, "Look at Object", this, "OnClick_LookAtSelection" );
		
		//UIActionManager.CreateCheckbox( gridActions, "Lock Rotation", this, "OnClick_LockRotation" );
		//UIActionManager.CreateCheckbox( gridActions, "Lock Movement", this, "OnClick_LockMovement" );
		
		Widget gridTime = UIActionManager.CreateGridSpacer( gridActions, 1, 2 );
		m_TravelTime = UIActionManager.CreateEditableText( gridTime, "Time", this );
		m_TravelTime.SetOnlyNumbers( true );
		m_TravelTime.SetText( "5" );
		m_TravelTime.SetWidgetWidth( m_TravelTime.GetLabelWidget(), 0.6 );
		m_TravelTime.SetWidgetWidth( m_TravelTime.GetEditBoxWidget(), 0.4 );

		m_ToggleSmooth = UIActionManager.CreateCheckbox( gridTime, "Smooth", this );

		m_Positon = UIActionManager.CreateEditableVector( gridActions, "Position", this, "OnClick_SetPosition", "Set" );
		Widget gridPosActions = UIActionManager.CreateGridSpacer( gridActions, 1, 3 );
		UIActionManager.CreateButton( gridPosActions, "Copy", this, "OnClick_CopyPos" );
		m_PositionList = UIActionManager.CreateNavButton( gridPosActions, "0", JM_COT_ICON_ARROW_LEFT, JM_COT_ICON_ARROW_RIGHT, this, "OnClick_SwitchPosition" );
		UIActionManager.CreateButton( gridPosActions, "Paste", this, "OnClick_PastePos" );
		
		UIActionManager.CreateButton( gridActions, "Travel through Positions", this, "OnClick_GoToPositions" );

		m_SliderBlurStrength = UIActionManager.CreateSlider( gridSliders, "#STR_COT_CAMERA_MODULE_BLUR", 0, 1, this, "OnChange_Blur" );
		m_SliderBlurStrength.SetCurrent( 0 );
		m_SliderBlurStrength.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderBlurStrength.SetStepValue( 0.1 );
		m_SliderBlurStrength.SetMin( 0.0 );
		m_SliderBlurStrength.SetMax( 100.0 );
		m_SliderBlurStrength.SetPosition( 0.0 );
		m_SliderBlurStrength.SetWidth( 1.0 );
		m_SliderBlurStrength.SetWidgetWidth( m_SliderBlurStrength.GetLabelWidget(), 0.3 );
		m_SliderBlurStrength.SetWidgetWidth( m_SliderBlurStrength.GetSliderWidget(), 0.7 );

		m_SliderFocusDistance = UIActionManager.CreateSlider( gridSliders, "#STR_COT_CAMERA_MODULE_FOCUS", 0, 1, this, "OnChange_Focus" );
		m_SliderFocusDistance.SetCurrent( 0 );
		m_SliderFocusDistance.SetFormat( "#STR_COT_FORMAT_METRE" );
		m_SliderFocusDistance.SetStepValue( 0.1 );
		m_SliderFocusDistance.SetMin( 0.0 );
		m_SliderFocusDistance.SetMax( 1000.0 );
		m_SliderFocusDistance.SetPosition( 0.0 );
		m_SliderFocusDistance.SetWidth( 1.0 );
		m_SliderFocusDistance.SetWidgetWidth( m_SliderFocusDistance.GetLabelWidget(), 0.3 );
		m_SliderFocusDistance.SetWidgetWidth( m_SliderFocusDistance.GetSliderWidget(), 0.7 );

		m_SliderFocalLength = UIActionManager.CreateSlider( gridSliders, "#STR_COT_CAMERA_MODULE_FOCAL_LENGTH", 0, 1, this, "OnChange_FocalLength" );
		m_SliderFocalLength.SetCurrent( 0 );
		m_SliderFocalLength.SetFormat( "#STR_COT_FORMAT_METRE" );
		m_SliderFocalLength.SetStepValue( 0.1 );
		m_SliderFocalLength.SetMin( 0.0 );
		m_SliderFocalLength.SetMax( 1000.0 );
		m_SliderFocalLength.SetPosition( 0.0 );
		m_SliderFocalLength.SetWidth( 1.0 );
		m_SliderFocalLength.SetWidgetWidth( m_SliderFocalLength.GetLabelWidget(), 0.3 );
		m_SliderFocalLength.SetWidgetWidth( m_SliderFocalLength.GetSliderWidget(), 0.7 );

		m_SliderFocalNear = UIActionManager.CreateSlider( gridSliders, "#STR_COT_CAMERA_MODULE_FOCAL_NEAR", 0, 1, this, "OnChange_FocalNear" );
		m_SliderFocalNear.SetCurrent( 0 );
		m_SliderFocalNear.SetFormat( "#STR_COT_FORMAT_METRE" );
		m_SliderFocalNear.SetStepValue( 0.1 );
		m_SliderFocalNear.SetMin( 0.0 );
		m_SliderFocalNear.SetMax( 1000.0 );
		m_SliderFocalNear.SetPosition( 0.0 );
		m_SliderFocalNear.SetWidth( 1.0 );
		m_SliderFocalNear.SetWidgetWidth( m_SliderFocalNear.GetLabelWidget(), 0.3 );
		m_SliderFocalNear.SetWidgetWidth( m_SliderFocalNear.GetSliderWidget(), 0.7 );

		m_SliderExposure = UIActionManager.CreateSlider( gridSliders, "#STR_COT_CAMERA_MODULE_EXPOSURE", 0, 1, this, "OnChange_Exposure" );
		m_SliderExposure.SetCurrent( 0 );
		m_SliderExposure.SetFormat( "#STR_COT_FORMAT_NONE" );
		m_SliderExposure.SetStepValue( 0.01 );
		m_SliderExposure.SetMin( 0.0 );
		m_SliderExposure.SetMax( 1.0 );
		m_SliderExposure.SetPosition( 0.0 );
		m_SliderExposure.SetWidth( 1.0 );
		m_SliderExposure.SetWidgetWidth( m_SliderExposure.GetLabelWidget(), 0.3 );
		m_SliderExposure.SetWidgetWidth( m_SliderExposure.GetSliderWidget(), 0.7 );

		m_SliderChromAbb = UIActionManager.CreateSlider( gridSliders, "Chrom Abb", 0, 1, this, "OnChange_ChromAbb" );
		m_SliderChromAbb.SetCurrent( 0 );
		m_SliderChromAbb.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderChromAbb.SetStepValue( 0.01 );
		m_SliderChromAbb.SetMin( 0.0 );
		m_SliderChromAbb.SetMax( 1.0 );
		m_SliderChromAbb.SetPosition( 0.0 );
		m_SliderChromAbb.SetWidth( 1.0 );
		m_SliderChromAbb.SetWidgetWidth( m_SliderChromAbb.GetLabelWidget(), 0.3 );
		m_SliderChromAbb.SetWidgetWidth( m_SliderChromAbb.GetSliderWidget(), 0.7 );

		m_SliderVignette = UIActionManager.CreateSlider( gridSliders, "Vignette", 0, 1, this, "OnChange_Vignette" );
		m_SliderVignette.SetCurrent( 0 );
		m_SliderVignette.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		m_SliderVignette.SetStepValue( 0.01 );
		m_SliderVignette.SetMin( 0.0 );
		m_SliderVignette.SetMax( 1.0 );
		m_SliderVignette.SetPosition( 0.0 );
		m_SliderVignette.SetWidth( 1.0 );
		m_SliderVignette.SetWidgetWidth( m_SliderVignette.GetLabelWidget(), 0.3 );
		m_SliderVignette.SetWidgetWidth( m_SliderVignette.GetSliderWidget(), 0.7 );

		m_SliderSpeed = UIActionManager.CreateSlider( gridSliders, "Speed", 0, 1, this, "OnChange_Speed" );
		m_SliderSpeed.SetCurrent( CAMERA_SPEED );
		m_SliderSpeed.SetStepValue( 0.001 );
		m_SliderSpeed.SetMin( 0.001 );
		m_SliderSpeed.SetMax( 10.0 );
		m_SliderSpeed.SetPosition( 0.0 );
		m_SliderSpeed.SetWidth( 1.0 );
		m_SliderSpeed.SetWidgetWidth( m_SliderVignette.GetLabelWidget(), 0.3 );
		m_SliderSpeed.SetWidgetWidth( m_SliderSpeed.GetSliderWidget(), 0.7 );

		if ( m_Module )
		{
			m_Positions = m_Module.m_Positions;
			if ( m_Positions )
				m_PositionList.SetValue(m_Positions[0]);

			m_Times = m_Module.m_Times;
			if ( m_Times && m_Times[0] > 0.0 )
				m_TravelTime.SetText(m_Times[0]);

			m_IsSmooth = m_Module.m_IsSmooth;
			if ( m_IsSmooth )
				m_ToggleSmooth.SetChecked(m_IsSmooth[0]);

			m_SliderBlurStrength.SetCurrent(m_Module.m_BlurStrength);
			m_SliderFocusDistance.SetCurrent(m_Module.m_FocusDistance);
			m_SliderFocalLength.SetCurrent(m_Module.m_FocalLength);
			m_SliderFocalNear.SetCurrent(m_Module.m_FocalNear);
			m_SliderExposure.SetCurrent(m_Module.m_Exposure);
			m_SliderChromAbb.SetCurrent(m_Module.m_ChromAbb);
			m_SliderVignette.SetCurrent(m_Module.m_Vignette);
			m_EnableFullmapCamera.SetChecked(m_Module.m_EnableFullmapCamera);
			m_HideGrass.SetChecked(m_Module.m_HideGrass);
		}

		UpdateUIPosition();
		m_sclr_MainActions.UpdateScroller();
	}

	override void Update() 
	{
		OnSliderUpdate();
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

		m_SliderSpeed.SetCurrent(CAMERA_SPEED);
	}

	void OnChange_Blur( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CAMERA_BLUR = action.GetCurrent();

		OnSliderUpdate();
	}

	void OnChange_Focus( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CAMERA_FDIST = action.GetCurrent();

		OnSliderUpdate();
	}

	void OnChange_FocalLength( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CAMERA_FLENGTH = action.GetCurrent();

		OnSliderUpdate();
	}

	void OnChange_FocalNear( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CAMERA_FNEAR = action.GetCurrent();

		OnSliderUpdate();
	}

	void OnChange_Exposure( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		EXPOSURE = action.GetCurrent();

		OnSliderUpdate();
	}

	void OnChange_ChromAbb( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CHROMABERX = action.GetCurrent();

		OnSliderUpdate();
	}

	void OnChange_Vignette( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		VIGNETTE = action.GetCurrent();

		OnSliderUpdate();
	}

	void OnChange_Speed( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CAMERA_SPEED = action.GetCurrent();

		OnSliderUpdate();
	}

	void OnClick_LockRotation( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		//m_Module.m_LockCameraRotation = !m_Module.m_LockCameraRotation;
	}
	
	void OnClick_LockMovement( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		//m_Module.m_LockCameraMovement = !m_Module.m_LockCameraMovement;
	}

	void OnClick_LookAtSelection( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.LookAtSelection();
	}

	void OnClick_CopyPos( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		GetGame().CopyToClipboard(m_Positon.GetValue().ToString());
	}

	void OnClick_PastePos( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		string clipboard;
		GetGame().CopyFromClipboard(clipboard);

		vector pos = clipboard.BeautifiedToVector();

		if ( pos != vector.Zero )
		{
			m_Positions[m_PositionID] = pos;
			m_Positon.SetValue(pos);
		}

		SavePosition();
	}

	void OnClick_SetPosition( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_Positon.GetValue() == "0 0 0" )
			OnClick_SwitchPosition(UIEvent.CLICK_RIGHTSIDE, NULL);
		else
			SavePosition();
	}

	void OnClick_SwitchPosition( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK_RIGHTSIDE )
		{
			SavePosition();
			m_PositionID++;
		}
		else if ( eid == UIEvent.CLICK_LEFTSIDE )
		{
			SavePosition();
			m_PositionID--;
		}
		else
		{
			return;
		}

		UpdateUIPosition();
	}

	void OnClick_GoToPositions( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.GoToSelection(m_Positions, m_Times, m_IsSmooth);
	}

	void OnClick_EnableFullmap( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.m_EnableFullmapCamera = action.IsChecked();
	}

	void OnClick_1stPersonADS_HideScope( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if (CurrentActiveCamera)
			CurrentActiveCamera.m_JM_1stPersonADS_HideScope = action.IsChecked();
	}

	void OnClick_HideGrass( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.m_HideGrass = action.IsChecked();
	}

	bool GetCurrentCamera3rdPerson()
	{
		if (CurrentActiveCamera)
			return CurrentActiveCamera.m_JM_3rdPerson;

		return false;
	}

	bool GetCurrentCamera1stPersonADSHideScope()
	{
		if (CurrentActiveCamera)
			return CurrentActiveCamera.m_JM_1stPersonADS_HideScope;

		return false;
	}

	void SetEnableFullmapCamera(bool enable)
	{
		m_EnableFullmapCamera.SetChecked(enable);
	}

	void SavePosition()
	{
		vector pos = m_Positon.GetValue();
		if ( pos == "0 0 0" )
			pos = GetGame().GetCurrentCameraPosition();
		
		float time = m_TravelTime.GetText().ToFloat();
		if ( time == 0.0 )
		{
			if ( m_Times[m_PositionID - 1] > 0.0 )
				time = m_Times[m_PositionID - 1];
			else
				time = 5;
		}

		if ( m_Positions.Count() < m_PositionID + 1 )
		{
			m_Positions.Insert(pos);
			m_Times.Insert(time);
			m_IsSmooth.Insert(m_ToggleSmooth.IsChecked());
		}
		else
		{
			m_Positions[m_PositionID] = pos;
			m_Times[m_PositionID] = time;
			m_IsSmooth[m_PositionID] = m_ToggleSmooth.IsChecked();
		}
	}

	void UpdateUIPosition()
	{
		if ( m_Positions.Count() < m_PositionID )
			m_PositionID = m_Positions.Count() + 1;
		else if ( m_PositionID < 0 )
			m_PositionID = 0;
		
		m_TravelTime.SetText(m_Times[m_PositionID]);
		m_ToggleSmooth.SetChecked(m_IsSmooth[m_PositionID]);
		m_Positon.SetValue(m_Positions[m_PositionID]);
		m_PositionList.SetButton(m_PositionID.ToString());
	}
};
