//! "Info" tab of JMVehiclesForm - the read-only detail card for whichever
//! vehicle is currently selected (from either the roster or a map marker).
//! Back-reference to the owning form, same shape as JMPlayerRowWidget.Menu.
//!
//! Sibling tabs: JMVehiclesFormTabMap and JMVehiclesFormTabActions. This one is
//! fed one vehicle at a time through PopulateInfo() and touches nothing else
//! on the form.
class JMVehiclesFormTabInfo: JMFormTab
{
	protected JMVehiclesForm m_Form;
	protected UIActionScroller m_VehicleInfoScroller;

	// Map from copy-button widget -> UIActionText it belongs to
	protected ref map<Widget, ref UIActionText> m_CopyButtonMap = new map<Widget, ref UIActionText>;
	protected UIActionText m_VehicleName;
	protected UIActionText m_VehicleClassName;
	protected UIActionText m_VehicleStatus;
	protected UIActionText m_VehicleType;
	protected UIActionText m_VehicleID;
	protected UIActionText m_VehiclePersistentIDAB;
	protected UIActionText m_VehiclePersistentIDCD;
	protected UIActionText m_VehiclePosition;
	protected UIActionText m_VehicleRotation;
	protected UIActionText m_VehicleCoolant;
	protected UIActionText m_VehicleKeys;
	protected UIActionText m_VehicleInfoOwner;
	protected UIActionText m_VehicleLastDriverUID;
	protected UIActionText m_VehicleLastDriverSteam;
	protected UIActionText m_VehicleLastDriverGUID;
	protected UIActionText m_VehicleCovered;

	void JMVehiclesFormTabInfo( JMVehiclesForm form )
	{
		m_Form = form;
	}

	// Per-info row: [icon 22] [label ----  value ----]
	// Compact spacer keeps the icon tight against the text without a gap.
	protected UIActionText CreateCopyableText( Widget parent, string label, string value = "" )
	{
		Widget row = UIActionManager.CreateWrapSpacerCompact( parent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		UIActionImageButton copyBtn = UIActionManager.CreateIconButton( row, JMConstants.ICON_STACK, this, "OnClick_CopyInfo" );
		copyBtn.SetFixedSize( 22, 22 );
		copyBtn.SetTooltip( "#STR_COT_VEHICLES_COPY_TO_CLIPBOARD" );

		UIActionText txt = UIActionManager.CreateText( row, label, value );
		txt.SetWidth( 0.9 );

		if ( copyBtn )
			m_CopyButtonMap.Insert( copyBtn.GetLayoutRoot(), txt );

		return txt;
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		m_VehicleInfoScroller = UIActionManager.CreateScroller( panel );
		Widget infoContent = m_VehicleInfoScroller.GetContentWidget();

		UIActionCard infoCard = UIActionManager.CreateCard( infoContent, "#STR_COT_VEHICLE_INFORMATION_TITLE" );
		Widget gridInfoA = UIActionManager.CreateGridSpacer( infoCard.GetContent(), 10, 1 );
			m_VehicleName           = CreateCopyableText( gridInfoA, "#STR_COT_VEHICLE_NAME", "Value" );
			m_VehicleClassName      = CreateCopyableText( gridInfoA, "#STR_COT_VEHICLE_CLASSNAME", "Value" );
			m_VehicleStatus         = CreateCopyableText( gridInfoA, "#STR_COT_VEHICLE_STATUS", "Value" );
			m_VehicleType           = CreateCopyableText( gridInfoA, "#STR_COT_VEHICLE_TYPE", "Value" );
			m_VehicleID             = CreateCopyableText( gridInfoA, "#STR_COT_VEHICLE_ID", "Value" );
			m_VehiclePersistentIDAB = CreateCopyableText( gridInfoA, "ID AB:", "Value" );
			m_VehiclePersistentIDCD = CreateCopyableText( gridInfoA, "ID CD:", "Value" );
			m_VehiclePosition       = CreateCopyableText( gridInfoA, "#STR_COT_VEHICLE_POSITION", "Value" );
			m_VehicleRotation       = CreateCopyableText( gridInfoA, "#STR_COT_VEHICLE_ROTATION", "Value" );
			m_VehicleCoolant        = CreateCopyableText( gridInfoA, "#STR_COT_VEHICLE_COOLANT", "Value" );

		Widget gridInfoB = UIActionManager.CreateGridSpacer( infoCard.GetContent(), 7, 1 );
			m_VehicleKeys            = CreateCopyableText( gridInfoB, "#STR_COT_VEHICLE_KEYS", "Value" );
			m_VehicleInfoOwner       = CreateCopyableText( gridInfoB, "#STR_COT_VEHICLE_OWNER", "Value" );
			m_VehicleLastDriverUID   = CreateCopyableText( gridInfoB, "#STR_COT_VEHICLE_DRIVER_UID", "#STR_COT_GENERIC_NA" );
			m_VehicleLastDriverSteam = CreateCopyableText( gridInfoB, "#STR_COT_VEHICLE_DRIVER_STEAM", "#STR_COT_GENERIC_NA" );
			m_VehicleLastDriverGUID  = CreateCopyableText( gridInfoB, "#STR_COT_VEHICLE_DRIVER_GUID", "#STR_COT_GENERIC_NA" );
			m_VehicleCovered         = CreateCopyableText( gridInfoB, "#STR_COT_VEHICLE_COVERED", "Value" );
			UIActionManager.CreateText( gridInfoB, "" );

		m_VehicleInfoScroller.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_VehicleInfoScroller )
			m_VehicleInfoScroller.UpdateScroller();
	}

	void PopulateInfo( JMVehicleMetaData vehicle )
	{
		if ( !vehicle || !m_VehicleName )
			return;

		m_VehicleName.SetText( vehicle.m_DisplayName );
		m_VehicleClassName.SetText( vehicle.m_ClassName );
		string statusText = vehicle.GetVehicleDestructionState();
		if ( statusText == "None" )
			statusText = "OK";
		m_VehicleStatus.SetText( statusText );
		m_VehicleType.SetText( vehicle.GetVehicleType() );
		m_VehicleID.SetText( vehicle.m_NetworkIDHigh.ToString() + " " + vehicle.m_NetworkIDLow.ToString() );
		m_VehiclePersistentIDAB.SetText( vehicle.m_PersistentIDA.ToString() + " " + vehicle.m_PersistentIDB.ToString() );
		m_VehiclePersistentIDCD.SetText( vehicle.m_PersistentIDC.ToString() + " " + vehicle.m_PersistentIDD.ToString() );
		m_VehiclePosition.SetText( vehicle.m_Position.ToString() );
		m_VehicleRotation.SetText( vehicle.m_Orientation.ToString() );

		string coolantText = "#STR_COT_GENERIC_NA";
		if ( vehicle.m_CoolantPct >= 0 )
			coolantText = Math.Round( vehicle.m_CoolantPct * 100 ).ToString() + "%";
		if ( m_VehicleCoolant )
			m_VehicleCoolant.SetText( coolantText );

		string hasKeys;
		if ( vehicle.m_HasKeys )
			hasKeys = "#STR_COT_GENERIC_YES";
		else
			hasKeys = "#STR_COT_GENERIC_NO";
		m_VehicleKeys.SetText( hasKeys );
		if ( vehicle.m_OwnerName != "" )
			m_VehicleInfoOwner.SetText( vehicle.m_OwnerName + " (" + vehicle.m_OwnerUID + ")" );
		else if ( vehicle.m_LastDriverSteam != "" )
			m_VehicleInfoOwner.SetText( vehicle.m_LastDriverSteam + " (" + "#STR_COT_VEHICLE_LAST_DRIVER" + ")" );
		else if ( vehicle.m_LastDriverUID != "" )
			m_VehicleInfoOwner.SetText( vehicle.m_LastDriverUID + " (" + "#STR_COT_VEHICLE_LAST_DRIVER" + ")" );
		else
			m_VehicleInfoOwner.SetText( "#STR_COT_GENERIC_UNKNOWN" );

		if ( vehicle.m_LastDriverUID != "" )
			m_VehicleLastDriverUID.SetText( vehicle.m_LastDriverUID );
		else
			m_VehicleLastDriverUID.SetText( "#STR_COT_GENERIC_NA" );

		if ( vehicle.m_LastDriverSteam != "" )
			m_VehicleLastDriverSteam.SetText( vehicle.m_LastDriverSteam );
		else
			m_VehicleLastDriverSteam.SetText( "#STR_COT_GENERIC_NA" );

		if ( vehicle.m_LastDriverGUID != "" )
			m_VehicleLastDriverGUID.SetText( vehicle.m_LastDriverGUID );
		else
			m_VehicleLastDriverGUID.SetText( "#STR_COT_GENERIC_NA" );

		string isCovered;
		if ( vehicle.m_IsCover )
			isCovered = "Yes";
		else
			isCovered = "No";
		m_VehicleCovered.SetText( isCovered );

		m_VehicleInfoScroller.UpdateScroller();
	}

	void OnClick_CopyInfo( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !action )
			return;

		UIActionText txt = m_CopyButtonMap.Get( action.GetLayoutRoot() );
		if ( txt )
			COTFeedback.Copy( txt.GetText() );
	}
}
