//! DEPRECATED - vehicle map markers are UIActionMap markers now (AddMarker / SetMarkerColor).
//!
//! Kept empty so a third-party `modded class JMVehiclesMapMarker` still compiles; see JMVehiclesMenu.
//! COT never creates one.
class JMVehiclesMapMarker: ScriptedWidgetEventHandler
{
	protected int m_MarkerColor;
	protected vector m_MarkerPos;
	protected ImageWidget m_Icon;
	protected TextWidget m_Name;

	void JMVehiclesMapMarker()
	{
		JMDeprecated.WarnOnce( this, "JMVehiclesMapMarker is deprecated and is never created. Please use UIActionMap.AddMarker / SetMarkerColor." );
	}

	void ChangeColor( int color )
	{
		m_MarkerColor = color;
	}
}
