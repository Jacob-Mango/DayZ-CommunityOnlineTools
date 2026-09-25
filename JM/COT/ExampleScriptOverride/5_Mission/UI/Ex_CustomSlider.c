#ifdef JM_CommunityOnlineTools
// Example: creating numeric slider controls with live feedback labels.
modded class JMCustomExampleForm
{
	protected ref UIActionSlider m_ExSlider;

	override protected void BuildSliderSection( Widget parent )
	{
		if ( !parent )
			return;

		m_ExSlider = UIActionManager.CreateSlider( parent, "Intensity Scale", 0, 100, this, "OnExSliderChange" );
	}

	void OnExSliderChange( UIActionBase action )
	{
		if ( m_ExSlider )
		{
			float val = m_ExSlider.GetValue();
			COTCreateLocalAdminNotification( new StringLocaliser( "Slider set to " + val.ToString() ), JMConstants.Lucide( "sliders" ) );
		}
	}
}
#endif
