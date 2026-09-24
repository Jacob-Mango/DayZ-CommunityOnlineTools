//! The banner across the top of the weather form that says which mode the tabs are in
//! (live weather, editing a preset, creating one) and, while a preset is open, offers
//! Save and Close. Raw widgets rather than UIActions, so the form hands its mouse
//! events to OnClick / OnMouseEnter / OnMouseLeave here.
class JMWeatherModeBanner
{
	protected JMWeatherForm m_Form;

	//! Which mode the weather tabs are in, shown across the full width so it is
	//! answered before anything is touched rather than after.
	protected Widget     m_ModeBanner;
	protected TextWidget m_ModeBannerText;

	//! Save and leave, right there in the banner that says you are editing a
	//! preset - the two things you want from that state without going back to
	//! the Presets tab to find them.
	protected ButtonWidget m_BannerSave;
	protected ButtonWidget m_BannerClose;
	protected Widget       m_BannerSaveHover;
	protected Widget       m_BannerCloseHover;

	void JMWeatherModeBanner( JMWeatherForm form )
	{
		m_Form = form;
	}

	Widget GetWidget()
	{
		return m_ModeBanner;
	}

	//! Look the banner's widgets up in the form's layout and paint the two icons.
	void Build()
	{
		Widget root = m_Form.GetLayoutRoot();

		m_ModeBanner     = root.FindAnyWidget( "mode_banner" );
		m_ModeBannerText = TextWidget.Cast( root.FindAnyWidget( "mode_banner_text" ) );

		m_BannerSave       = ButtonWidget.Cast( root.FindAnyWidget( "mode_banner_save" ) );
		m_BannerClose      = ButtonWidget.Cast( root.FindAnyWidget( "mode_banner_close" ) );
		m_BannerSaveHover  = root.FindAnyWidget( "mode_banner_save_hover" );
		m_BannerCloseHover = root.FindAnyWidget( "mode_banner_close_hover" );

		LoadBannerIcon( "mode_banner_save_icon", JMConstants.Lucide( "save" ) );
		LoadBannerIcon( "mode_banner_close_icon", JMConstants.Lucide( "x" ) );
	}

	//! Raw widgets, so their clicks arrive here rather than at a UIAction. Returns
	//! true when the click was the banner's.
	bool OnClick( Widget w, int button )
	{
		if ( button != MouseState.LEFT )
			return false;

		if ( w == m_BannerClose )
		{
			m_Form.m_TabPresetsCtrl.ExitPresetMode();
			return true;
		}

		if ( w == m_BannerSave )
		{
			m_Form.m_TabPresetsCtrl.SavePreset();
			return true;
		}

		return false;
	}

	void OnMouseEnter( Widget w )
	{
		if ( w == m_BannerSave && m_BannerSaveHover )
			m_BannerSaveHover.Show( true );

		if ( w == m_BannerClose && m_BannerCloseHover )
			m_BannerCloseHover.Show( true );
	}

	void OnMouseLeave( Widget w )
	{
		if ( w == m_BannerSave && m_BannerSaveHover )
			m_BannerSaveHover.Show( false );

		if ( w == m_BannerClose && m_BannerCloseHover )
			m_BannerCloseHover.Show( false );
	}

	void LoadBannerIcon( string name, string path )
	{
		ImageWidget icon = ImageWidget.Cast( m_Form.GetLayoutRoot().FindAnyWidget( name ) );

		if ( !icon )
			return;

		icon.LoadImageFile( 0, path );
		icon.SetImage( 0 );
		icon.SetColor( JMTheme.TEXT_ON_ACCENT );
	}

	//! Public: the Presets tab repaints the banner whenever its selection changes.
	void Update()
	{
		if ( !m_ModeBanner || !m_ModeBannerText )
			return;

		//! Both buttons only mean anything while a preset is being edited.
		if ( m_BannerSave )
			m_BannerSave.Show( m_Form.IsPresetMode() );

		if ( m_BannerClose )
			m_BannerClose.Show( m_Form.IsPresetMode() );

		if ( m_Form.m_TabPresetsCtrl && m_Form.m_TabPresetsCtrl.IsCreatingPreset() )
		{
			m_ModeBanner.SetColor( JMTheme.ACCENT );
			m_ModeBannerText.SetText( "#STR_COT_WEATHER_MODE_NEW" );
			return;
		}

		if ( m_Form.m_TabPresetsCtrl && m_Form.m_TabPresetsCtrl.GetSelectedPreset() != "" )
		{
			m_ModeBanner.SetColor( JMTheme.ACCENT );
			m_ModeBannerText.SetText( COT_String.TranslateEx( "#STR_COT_WEATHER_MODE_PRESET", m_Form.m_TabPresetsCtrl.GetSelectedPreset() ) );
			return;
		}

		//! The live state gets a banner too. Reserving the band either way
		//! means no reflow when the mode changes, and "which of these am I
		//! about to change" is worth answering in both directions.
		m_ModeBanner.SetColor( JMTheme.SURFACE_RAISED );
		m_ModeBannerText.SetText( "#STR_COT_WEATHER_MODE_LIVE" );
	}
}
