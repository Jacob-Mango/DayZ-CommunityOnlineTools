#ifdef JM_CommunityOnlineTools
// Example: Creating and managing popups (ValuePrompt & ContextMenu Overlays)
modded class JMPlayerForm
{
	protected UIActionValuePrompt m_CustomValuePrompt;

	protected void InitCustomPopups()
	{
		// Create a floating value prompt anchored to the window root
		m_CustomValuePrompt = UIActionManager.CreateValuePrompt( layoutRoot, m_Window.GetWidgetRoot(), this, "OnConfirm_CustomValuePrompt" );

		// Must be built once before either Show, and anchored the same as the panel
		m_CustomValuePrompt.InitPrompt( m_Window.GetWidgetRoot() );

		// Register overlay so OnChange_Tab and OnHide automatically dismiss it
		RegisterOverlay( m_CustomValuePrompt );
	}

	void OpenCustomPrompt()
	{
		if ( !m_CustomValuePrompt )
			return;

		m_CustomValuePrompt.ShowSlider( "submod_value", "Set Custom Value", "Value", 1, 100, 50 );
	}

	void OnConfirm_CustomValuePrompt( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_CustomValuePrompt )
			return;

		if ( m_CustomValuePrompt.GetPromptId() != "submod_value" )
			return;

		float chosenValue = m_CustomValuePrompt.GetSliderValue();
		// Process confirmed value...
	}
}
#endif
