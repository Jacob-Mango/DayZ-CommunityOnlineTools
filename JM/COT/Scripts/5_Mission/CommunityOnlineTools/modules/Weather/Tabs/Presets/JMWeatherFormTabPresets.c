//! "Dynamic Presets" tab of JMWeatherForm - the live status of the dynamic weather
//! machine and the preset editor. Back-reference to the owning form, same shape as
//! JMPlayerRowWidget.Menu.
//!
//! ---------------------------------------------------------------------------
//!  Preset edit mode
//!
//!  With a preset selected here, every other tab edits
//!  THAT preset's weather - the phase selected in the editor - instead of the live
//!  world: the controls are loaded from it, the live poll stops writing over them,
//!  every value becomes a min/max span, the Time tab is disabled and Apply becomes
//!  a local preview. Selecting a preset IS entering this mode; the banner's Close
//!  un-selects it. The selection and the mode belong to JMWeatherDynamicEditor;
//!  this tab only hosts it and answers the form's questions through it, and until
//!  the tab has been opened there is no selection and the answer is "live world".
//! ---------------------------------------------------------------------------
class JMWeatherFormTabPresets: JMFormTab
{
	protected JMWeatherForm m_Form;
	protected UIActionScroller m_ScrollerPresets;

	protected ref JMWeatherDynamicStatusCard m_StatusCard;
	protected ref JMWeatherDynamicEditor     m_Editor;

	protected string m_RemovePreset;

	void JMWeatherFormTabPresets( JMWeatherForm form )
	{
		m_Form = form;
	}

	// -------------------------------------------------------------------------
	//  Selection state - read by the form, the banner and the other tabs
	// -------------------------------------------------------------------------

	//! What the name field currently says - the name a save writes under.
	string GetEditedName()
	{
		if ( m_Editor )
			return m_Editor.GetEditedName();

		return "";
	}

	string GetSelectedPreset()
	{
		if ( m_Editor )
			return m_Editor.GetSelectedName();

		return "";
	}

	bool IsCreatingPreset()
	{
		return m_Editor && m_Editor.IsCreating();
	}

	bool IsPresetMode()
	{
		return m_Editor && m_Editor.IsEditing();
	}

	//! The weather the tabs are editing: the selected phase's own. NULL when no
	//! preset is being edited - the tabs then show the live world.
	JMWeatherPreset EditingPreset()
	{
		if ( !m_Editor )
			return NULL;

		return m_Editor.EditingWeather();
	}

	void SetSelectedPreset( string preset )
	{
		if ( m_Editor )
			m_Editor.SelectByName( preset );
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		m_ScrollerPresets = UIActionManager.CreateScroller( panel );
		Widget content = m_ScrollerPresets.GetContentWidget();

		//! The live position comes first - it is what this tab is for, and what an
		//! admin opening it wants to see before touching a preset.
		m_StatusCard = new JMWeatherDynamicStatusCard( m_Form, content, m_ScrollerPresets );

		m_Editor = new JMWeatherDynamicEditor( m_Form, m_ScrollerPresets, content );

		RefreshDynamic();

		m_ScrollerPresets.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_ScrollerPresets )
			m_ScrollerPresets.UpdateScroller();
	}

	// -------------------------------------------------------------------------
	//  Preset edit mode transitions - what the form and the banner call
	// -------------------------------------------------------------------------

	//! Back to the live world: a preset is edited for as long as it is selected, so
	//! leaving edit mode is un-selecting it.
	void ExitPresetMode()
	{
		if ( m_Editor )
			m_Editor.Deselect();
	}

	void CreateNew()
	{
		if ( m_Editor )
			m_Editor.AddPreset();
	}

	//! Send the edited preset to the server. Shared by the editor's button and the
	//! one in the banner, which have to mean the same thing.
	void SavePreset()
	{
		if ( m_Editor )
			m_Editor.Save();
	}

	//! Asks for confirmation; the answer arrives on the form's RemovePreset_Yes
	//! (JMConfirmation callbacks are looked up on the form), which forwards to
	//! RemovePresetConfirmed.
	void RemovePreset( string preset )
	{
		m_RemovePreset = preset;

		m_Form.ConfirmAction( "#STR_COT_GENERIC_CONFIRM", "#STR_COT_WEATHER_PRESET_REMOVE_CONFIRM", "RemovePreset_Yes", "#STR_COT_GENERIC_YES", "#STR_COT_GENERIC_NO" );
	}

	void RemovePresetConfirmed()
	{
		if ( !m_Editor )
			return;

		m_Editor.SelectByName( m_RemovePreset );
		m_Editor.RemoveConfirmed();

		m_Form.UpdateStates();
	}

	//! The rename popups' answers, reaching the editor through the form.
	void RenamePresetConfirmed( string name )
	{
		if ( m_Editor )
			m_Editor.ApplyPresetName( name );
	}

	void RenamePhaseConfirmed( string name )
	{
		if ( m_Editor )
			m_Editor.ApplyPhaseName( name );
	}

	//! The stored presets changed: the selector, both editors and the status card
	//! follow them.
	void UpdatePresetList()
	{
		RefreshDynamic();

		if ( m_ScrollerPresets )
			m_ScrollerPresets.UpdateScroller();
	}

	// -------------------------------------------------------------------------
	//  Dynamic components - status card and preset editor
	// -------------------------------------------------------------------------

	//! Both read the stored configuration, so they follow every settings update.
	protected void RefreshDynamic()
	{
		if ( m_StatusCard )
			m_StatusCard.RefreshFromSettings();

		if ( m_Editor )
			m_Editor.RefreshFromSettings();
	}

	//! Per-frame, from JMWeatherForm.Update().
	void Poll()
	{
		if ( m_StatusCard )
			m_StatusCard.Poll();

		if ( m_Editor )
			m_Editor.Poll();
	}

	// -------------------------------------------------------------------------
	//  Permissions / enable state
	// -------------------------------------------------------------------------

	void UpdateStates()
	{
		if ( m_StatusCard )
			m_StatusCard.UpdateStates();

		if ( m_Editor )
			m_Editor.UpdateStates();
	}
}
