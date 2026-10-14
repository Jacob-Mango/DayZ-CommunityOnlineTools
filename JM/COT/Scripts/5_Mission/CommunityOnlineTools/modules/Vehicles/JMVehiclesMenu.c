//! DEPRECATED - now JMVehiclesForm with tabs. Kept only so a third-party `modded
//! class JMVehiclesMenu` (COT_VehicleManager) still COMPILES (an unknown type fails
//! the Mission module); its additions never run. Port to `modded class
//! JMVehiclesForm` / `...TabActions` / `...TabMap`. Members below are unread.
class JMVehiclesMenu: JMFormBase
{
	protected ButtonWidget m_DeleteUnclaimedButton;
	protected TextWidget m_DeleteUnclaimedButtonLabel;
	protected ButtonWidget m_CancleVehicleEdit;
	protected MapWidget m_MapWidget;
	protected JMVehicleMetaData m_CurrentVehicle;
	protected ref array< ref JMVehiclesMapMarker > m_MapMarkers = new array< ref JMVehiclesMapMarker >;

	void JMVehiclesMenu()
	{
		JMDeprecated.WarnOnce( this, "JMVehiclesMenu is deprecated and is never shown. Please port to JMVehiclesForm / JMVehiclesFormTabActions / JMVehiclesFormTabMap." );
	}

	void SetVehicleInfo( JMVehicleMetaData vehicle )
	{
	}
}
