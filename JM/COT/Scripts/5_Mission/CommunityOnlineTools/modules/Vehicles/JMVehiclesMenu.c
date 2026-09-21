//! DEPRECATED - the Vehicles window is JMVehiclesForm now, split into tabs.
//!
//! JMVehiclesMenu no longer exists as a form and nothing in COT creates one. This empty class is
//! kept only so a third-party `modded class JMVehiclesMenu` (COT_VehicleManager) still COMPILES:
//! an unknown type there fails the whole Mission script module and takes every other mod down
//! with it. Whatever such a mod adds to the menu does not appear in the new window, and its
//! buttons and handlers never run. Port it to `modded class JMVehiclesForm` /
//! `JMVehiclesFormTabActions` / `JMVehiclesFormTabMap`.
//!
//! The members below are the ones the old menu exposed to modded classes. Nothing reads them.
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
