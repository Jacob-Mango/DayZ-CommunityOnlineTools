class CameraSettings extends Form 
{
    private SliderWidget m_SldCamBlur;
    private TextWidget m_TxtCamBlur;
    
    private SliderWidget m_SldCamDist;
    private TextWidget m_TxtCamDist;
    
    private SliderWidget m_SldCamFLen;
    private TextWidget m_TxtCamFlen;
    
    private SliderWidget m_SldCamFnear;
    private TextWidget m_TxtCamFnear;

    private SliderWidget m_SldCamExp;
    private TextWidget m_TxtCamExp;

    private ButtonWidget m_btn_rot;
    private ButtonWidget m_btn_phi

    static Widget CAMERA_ROT;
    static Widget CAMERA_PHI;

    private SliderWidget m_SldChromX;
    private SliderWidget m_SldChromY;

    private TextWidget m_TxtChromX;
    private TextWidget m_TxtChromY;

    private ref WidgetStore widgetStore;

    void CameraSettings()
    {
    }    
    
    void ~CameraSettings()
    {
    }

    override string GetTitle()
    {
        return "Camera Tools";
    }
    
    override string GetImageSet()
    {
        return "rover_imageset";
    }

    override string GetIconName()
    {
        return "camera";
    }

    override bool ImageIsIcon()
    {
        return true;
    }
    
    override void OnInit( bool fromMenu )
    {
        widgetStore = new WidgetStore( layoutRoot );

        m_SldCamBlur = SliderWidget.Cast( layoutRoot.FindAnyWidget("camera_slider_blur") );
        m_TxtCamBlur = TextWidget.Cast( layoutRoot.FindAnyWidget("camera_slider_blur_value") );
    
        m_SldCamDist = SliderWidget.Cast( layoutRoot.FindAnyWidget("camera_slider_dist") );
        m_TxtCamDist = TextWidget.Cast( layoutRoot.FindAnyWidget("camera_slider_dist_value") );
        
        m_SldCamFLen = SliderWidget.Cast( layoutRoot.FindAnyWidget("camera_slider_flen" ) );
        m_TxtCamFlen = TextWidget.Cast( layoutRoot.FindAnyWidget("camera_slider_flen_value") );
        
        m_SldCamFnear = SliderWidget.Cast( layoutRoot.FindAnyWidget("camera_slider_fnear") );
        m_TxtCamFnear = TextWidget.Cast( layoutRoot.FindAnyWidget("camera_slider_fnear_value") );
        
        m_SldCamExp = layoutRoot.FindAnyWidget("camera_slider_exp");
        m_TxtCamExp = layoutRoot.FindAnyWidget("camera_slider_exp_value");

        m_btn_rot = ButtonWidget.Cast( layoutRoot.FindAnyWidget("camera_btn_rot"));
        m_btn_phi = ButtonWidget.Cast( layoutRoot.FindAnyWidget("camera_btn_phi"));

        m_TxtChromX = layoutRoot.FindAnyWidget("camera_slider_chrom_value_x");
        m_TxtChromY = layoutRoot.FindAnyWidget("camera_slider_chrom_value_y");

        if ( CAMERA_ROT == NULL )
        {
            CAMERA_ROT = GetGame().GetWorkspace().CreateWidgets( "JM/COT/gui/layouts/Camera/CameraROT.layout" );
        }
        
        if ( CAMERA_PHI == NULL )
        {
            CAMERA_PHI = GetGame().GetWorkspace().CreateWidgets( "JM/COT/gui/layouts/Camera/CameraPHI.layout" );
        }
    }
    
    
    override void OnShow()
    {
        super.OnShow();

        if ( HasBeenInitialized )
        {
            GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Insert( this.Update );
        }

        UpdateEditBox();
    }

    override void OnHide()
    {
        super.OnHide();

        GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove( this.Update );

    }
    
    void Update()
    {
        UpdateSliders();
    }

    void UpdateEditBox() 
    {
        widgetStore.GetEditBoxWidget("camera_input_chrom_x").SetText( CHROMABERX.ToString());
        widgetStore.GetEditBoxWidget("camera_input_chrom_y").SetText( CHROMABERY.ToString());
    }

    override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
    {
        SetFocus( layoutRoot );
        return false;
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if ( w.GetName() == "close_button" ) 
        {
            GetLayoutRoot().Show( false );
            OnHide();
        }

        if ( w.GetName() == "camera_toggle" ) 
        {
            ref CameraTool cmt = GetModuleManager().GetModule( CameraTool );
            GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Call(cmt.ToggleCamera ); // Fix crash
        }

        Widget effectsFrame = widgetStore.GetWidget( "camera_effects_frame" );
        Widget settingsFrame = widgetStore.GetWidget( "camera_settings_frame" );

        if ( w.GetName() == "camera_slider_tab_effects" ) 
        {
            if ( !effectsFrame.IsVisible() ) 
            {
                effectsFrame.Show(!effectsFrame.IsVisible());
                settingsFrame.Show(!effectsFrame.IsVisible());
            }
        }
        if ( w.GetName() == "camera_slider_tab_settings" ) 
        {
            if ( !settingsFrame.IsVisible() ) 
            {
                settingsFrame.Show(!settingsFrame.IsVisible());
                effectsFrame.Show(!settingsFrame.IsVisible());
            }
        }

        if ( w == m_btn_rot ) 
        {
            CAMERA_ROT.Show( !CAMERA_ROT.IsVisible() );
        }

        if ( w == m_btn_phi )
        {
            CAMERA_PHI.Show( !CAMERA_PHI.IsVisible() );
        }

        if ( w.GetName() == "camera_fov_speed_btn_inc" ) 
        {
            CAMERA_FOV_SPEED_MODIFIER += 1;
        }

        if ( w.GetName() == "camera_fov_speed_btn_dec" ) 
        {
            CAMERA_FOV_SPEED_MODIFIER -= 1;
            if ( CAMERA_FOV_SPEED_MODIFIER < 0 ) CAMERA_FOV_SPEED_MODIFIER = 0;
        }

        if ( w.GetName() == "camera_smooth_btn_inc" )
        {
            CAMERA_SMOOTH += 0.025;
            CAMERA_SMOOTH = Math.Clamp(CAMERA_SMOOTH, 0.0, 1.0); // ugh
        }

        if ( w.GetName() == "camera_smooth_btn_dec" )
        {
            CAMERA_SMOOTH -= 0.025;
            CAMERA_SMOOTH = Math.Clamp(CAMERA_SMOOTH, 0.0, 1.0);
        }

        if ( w.GetName() == "camera_msens_btn_inc" ) 
        {
            CAMERA_MSENS += 0.05;
            CAMERA_MSENS = Math.Clamp(CAMERA_MSENS, 0.0, 1.5);
        }

        if ( w.GetName() == "camera_msens_btn_dec" ) 
        {
            CAMERA_MSENS -= 0.05;
            CAMERA_MSENS = Math.Clamp(CAMERA_MSENS, 0.0, 1.5);
        }
        if ( w.GetName() == "camera_msmooth_btn_inc" ) 
        {
            CAMERA_VELDRAG += 0.01; // percent
            CAMERA_VELDRAG = Math.Clamp(CAMERA_VELDRAG, 0.9, 1.0);
        }
        if ( w.GetName() == "camera_msmooth_btn_dec" ) 
        {
            CAMERA_VELDRAG -= 0.01;
            CAMERA_VELDRAG = Math.Clamp(CAMERA_VELDRAG, 0.9, 1.0); // 10 clicks
        }

        return false;
    }

    override bool OnChange( Widget w, int x, int y, bool finished )
    {
        Material chromAber = GetGame().GetWorld().GetMaterial( "Graphics/Materials/postprocess/chromaber" );
        Material matColors = GetGame().GetWorld().GetMaterial( "Graphics/Materials/postprocess/glow" );
        Material rotBlur = GetGame().GetWorld().GetMaterial( "Graphics/Materials/postprocess/rotblur" );
        Material radBlur = GetGame().GetWorld().GetMaterial( "Graphics/Materials/postprocess/radialblur" );

        SliderWidget changeSlider = widgetStore.GetSliderWidget( w.GetName() );
        EditBoxWidget editBox = widgetStore.GetEditBoxWidget( w.GetName() );

        if ( editBox ) 
        {
            string text = editBox.GetText();
            float value = text.ToFloat();

            if ( editBox.GetName() == "camera_input_chrom_x" ) 
            {
                CHROMABERX = value;
                chromAber.SetParam( "PowerX", CHROMABERX );
            }

            if ( editBox.GetName() == "camera_input_chrom_y" ) 
            {
                CHROMABERY = value;
                chromAber.SetParam( "PowerY", CHROMABERY );
            }
            return false;
        }

        if ( w == m_SldCamBlur ) 
        {
            CAMERA_BLUR = 4.0 * (m_SldCamBlur.GetCurrent() * 0.01); // percent
            
            if ( m_SldCamBlur.GetCurrent() == 0 ) 
            {
                CAMERA_DOF = false;
                PPEffects.ResetDOFOverride();
            } 
            else 
            {
                CAMERA_DOF = true;
            }
        } 
        else if ( w == m_SldCamDist ) 
        {
            if ( m_SldCamDist.GetCurrent() == 0 ) 
            {
                CAMERA_AFOCUS = true;
            } 
            else 
            {
                CAMERA_AFOCUS = false;
                CAMERA_FDIST = m_SldCamDist.GetCurrent() * 15;
            }
        }
        else if ( w == m_SldCamFLen ) 
        {
            CAMERA_FLENGTH = (m_SldCamFLen.GetCurrent() * 2.0);
        }
        else if ( w == m_SldCamFnear ) 
        {
            CAMERA_FNEAR = (m_SldCamFnear.GetCurrent() * 2.0);
        }
        else if ( w == m_SldCamExp ) 
        {
            EXPOSURE = (m_SldCamExp.GetCurrent() * 0.1) - 5.0;
            GetGame().SetEVUser( EXPOSURE );
        }
        else if ( changeSlider.GetName() == "camera_slider_hue" ) 
        {
            HUESHIFT = (changeSlider.GetCurrent() * 0.1) - 5.0;
            matColors.SetParam( "Saturation", HUESHIFT );
        }
        else if ( changeSlider.GetName() == "camera_slider_rotblur_power" ) 
        {
            ROTBLUR = 0.5 * (changeSlider.GetCurrent() * 0.01);
            rotBlur.SetParam( "Power", ROTBLUR );
        }
        else if ( changeSlider.GetName() == "camera_slider_rotblur_mindepth" ) 
        {
            MINDEPTH = 2.5 * (changeSlider.GetCurrent() * 0.01);
            rotBlur.SetParam( "MinDepth", MINDEPTH );
        }
        else if ( changeSlider.GetName() == "camera_slider_rotblur_maxdepth" ) 
        {
            MAXDEPTH = (changeSlider.GetCurrent() * 0.1);
            rotBlur.SetParam( "MaxDepth", MAXDEPTH );
        }
        else if ( changeSlider.GetName() == "camera_slider_radblur_x" ) 
        {
            RADBLURX = (changeSlider.GetCurrent() * 0.01);
            radBlur.SetParam( "PowerX", RADBLURX );
        }
        else if ( changeSlider.GetName() == "camera_slider_radblur_y" ) 
        {
            RADBLURY = (changeSlider.GetCurrent() * 0.01);
            radBlur.SetParam( "PowerY", RADBLURY );
        }
        else if ( changeSlider.GetName() == "camera_slider_radblur_offsetx" ) 
        {
            RADBLUROFFX = (changeSlider.GetCurrent() * 0.01);
            radBlur.SetParam( "OffsetX", RADBLUROFFX );
        }
        else if ( changeSlider.GetName() == "camera_slider_radblur_offsety" ) 
        {
            RADBLUROFFY = (changeSlider.GetCurrent() * 0.01);
            radBlur.SetParam( "OffsetY", RADBLUROFFY );
        }
        else if ( changeSlider.GetName() == "camera_slider_vign" ) 
        {
            VIGNETTE = (changeSlider.GetCurrent() * 0.02);
            matColors.SetParam( "Vignette", VIGNETTE );
        }
        else if ( changeSlider.GetName() == "camera_slider_vign_r" ) 
        {
            VARGB[0] = ((changeSlider.GetCurrent() * 0.1) - 5.0);
            matColors.SetParam("VignetteColor", VARGB );
        }
        else if ( changeSlider.GetName() == "camera_slider_vign_g" ) 
        {
            VARGB[1] = ((changeSlider.GetCurrent() * 0.1) - 5.0);
            matColors.SetParam("VignetteColor", VARGB );
        }
        else if ( changeSlider.GetName() == "camera_slider_vign_b" ) 
        {
            VARGB[2] = ((changeSlider.GetCurrent() * 0.1) - 5.0);
            matColors.SetParam("VignetteColor", VARGB );
        }
        else if ( changeSlider.GetName() == "camera_slider_color_r" ) 
        {
            CARGB[0] = ((changeSlider.GetCurrent() * 0.1) - 5.0);
            matColors.SetParam("OverlayColor", CARGB );
        }
        else if ( changeSlider.GetName() == "camera_slider_color_g" ) 
        {
            CARGB[1] = ((changeSlider.GetCurrent() * 0.1) - 5.0);
            matColors.SetParam("OverlayColor", CARGB );
        }
        else if ( changeSlider.GetName() == "camera_slider_color_b" ) 
        {
            CARGB[2] = ((changeSlider.GetCurrent() * 0.1) - 5.0);
            matColors.SetParam("OverlayColor", CARGB );
        }
        else if ( changeSlider.GetName() == "camera_slider_color_a" ) 
        {
            CARGB[3] = ((changeSlider.GetCurrent() * 0.05) - 4.0);
            matColors.SetParam("OverlayFactor", CARGB[3] );
        }
        else if ( changeSlider.GetName() == "camera_slider_view" )
        {
            VIEWDISTANCE = 100*changeSlider.GetCurrent();
            GetGame().GetWorld().SetPreferredViewDistance(VIEWDISTANCE);
        }
        return false;
    }

    override bool OnMouseWheel( Widget w, int x, int y, int wheel )
    {
        SliderWidget slider = widgetStore.GetSliderWidget( w.GetName() );

        if ( slider ) 
        {
            bool up = wheel < 0;
            int value = 1;

            if ( up ) value = -1;

            float current = slider.GetCurrent();
            slider.SetCurrent( current + value );

            OnChange( w, x, y, false );
        }
        return false;
    }

    void UpdateSliders() 
    {
        if ( widgetStore == NULL ) return;

        string cameraTarget = "None";

        CameraTool cameraTool = CameraTool.Cast( GetModuleManager().GetModule(CameraTool) );

        Object targetObject = NULL;
        
        if ( cameraTool )
        {
            targetObject = cameraTool.GetTargetObject();
            
            if ( targetObject ) 
            {
                cameraTarget = targetObject.GetType();
            }
        }

        vector targetPos = CameraTool.Cast(GetModuleManager().GetModule(CameraTool)).GetTargetPos();
        
        if ( targetPos != vector.Zero ) 
        {
            cameraTarget = VectorToString( targetPos, 1 );
        }

        widgetStore.GetTextWidget("camera_target_txt").SetText("Target: " + cameraTarget );

        m_TxtCamBlur.SetText(((CAMERA_BLUR / 4.0) * 100.0).ToString() + "%");

        string autoF = "";
        if ( CAMERA_AFOCUS ) 
        {
            autoF = " (AUTO)";
        }
        m_TxtCamDist.SetText(CAMERA_FDIST.ToString()+"m" + autoF);
        m_TxtCamFlen.SetText(CAMERA_FLENGTH.ToString());
        m_TxtCamFnear.SetText(CAMERA_FNEAR.ToString());
        m_TxtCamExp.SetText( EXPOSURE.ToString() );

        widgetStore.GetTextWidget("camera_slider_hue_value").SetText( HUESHIFT.ToString() ); // make script param that updates child textwidget of slider based on vvariable?
        widgetStore.GetTextWidget("camera_slider_rotblur_power_value").SetText( ROTBLUR.ToString() );
        widgetStore.GetTextWidget("camera_slider_rotblur_mindepth_value").SetText( MINDEPTH.ToString() );
        widgetStore.GetTextWidget("camera_slider_rotblur_maxdepth_value").SetText( MAXDEPTH.ToString() );
        widgetStore.GetTextWidget("camera_slider_radblur_value_x").SetText( RADBLURX.ToString() );
        widgetStore.GetTextWidget("camera_slider_radblur_value_y").SetText( RADBLURY.ToString() );
        widgetStore.GetTextWidget("camera_slider_radblur_value_offsetx").SetText( RADBLUROFFX.ToString() );
        widgetStore.GetTextWidget("camera_slider_radblur_value_offsety").SetText( RADBLUROFFY.ToString() );
        widgetStore.GetTextWidget("camera_slider_vign_value").SetText( VIGNETTE.ToString() );
        widgetStore.GetTextWidget("camera_slider_vign_value_r").SetText( VARGB[0].ToString() );
        widgetStore.GetTextWidget("camera_slider_vign_value_g").SetText( VARGB[1].ToString() );
        widgetStore.GetTextWidget("camera_slider_vign_value_b").SetText( VARGB[2].ToString() );
        widgetStore.GetTextWidget("camera_slider_color_value_r").SetText( CARGB[0].ToString() );
        widgetStore.GetTextWidget("camera_slider_color_value_g").SetText( CARGB[1].ToString() );
        widgetStore.GetTextWidget("camera_slider_color_value_b").SetText( CARGB[2].ToString() );
        widgetStore.GetTextWidget("camera_slider_color_value_a").SetText( CARGB[3].ToString() );
        widgetStore.GetTextWidget("camera_slider_view_value").SetText( VIEWDISTANCE.ToString() );

        // get child and update text?

        // Set slider current to value
        widgetStore.GetSliderWidget("camera_slider_hue").SetCurrent( (HUESHIFT + 5.0) / 0.1); // this could probably be moved to a proper system
        widgetStore.GetSliderWidget("camera_slider_rotblur_power").SetCurrent( (ROTBLUR / 0.5) / 0.01);
        widgetStore.GetSliderWidget("camera_slider_rotblur_mindepth").SetCurrent( (MINDEPTH / 0.01) / 2.5);
        widgetStore.GetSliderWidget("camera_slider_rotblur_maxdepth").SetCurrent( MAXDEPTH / 0.1);
        widgetStore.GetSliderWidget("camera_slider_radblur_x").SetCurrent( RADBLURX / 0.01);
        widgetStore.GetSliderWidget("camera_slider_radblur_y").SetCurrent( RADBLURY / 0.01);
        widgetStore.GetSliderWidget("camera_slider_radblur_offsetx").SetCurrent( RADBLUROFFX / 0.01);
        widgetStore.GetSliderWidget("camera_slider_radblur_offsety").SetCurrent( RADBLUROFFY / 0.01);
        widgetStore.GetSliderWidget("camera_slider_view").SetCurrent( VIEWDISTANCE / 100.0);

        // fk me ok im lazy. cbf doing this for all the sliders.

        TextWidget speedTxt = layoutRoot.FindAnyWidget( "camera_fov_speed_text" );
        speedTxt.SetText("FOV Smooth: " + CAMERA_FOV_SPEED_MODIFIER );

        TextWidget smoothTxt = layoutRoot.FindAnyWidget( "camera_smooth_text" );
        smoothTxt.SetText("Cam Smooth: " + CAMERA_SMOOTH );

        TextWidget sensTxt = layoutRoot.FindAnyWidget( "camera_msens_text" );
        sensTxt.SetText("Cam Sens: " + CAMERA_MSENS );

        TextWidget flySpeed = layoutRoot.FindAnyWidget("camera_msmooth_text");
        flySpeed.SetText("Fly smooth: " + (CAMERA_VELDRAG - 0.9) / 0.1);
    }    
}