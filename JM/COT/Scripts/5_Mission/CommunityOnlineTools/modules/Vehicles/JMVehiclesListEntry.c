class JMVehiclesListEntry: ScriptedWidgetEventHandler
{
	protected Widget m_Root;
	protected TextWidget m_VehicleName;
	protected TextWidget m_VehicleID;
	protected int m_Sort;
	protected string m_Label;
	protected ButtonWidget m_EditButton;
	protected ImageWidget m_VehicleStatusIcon;
	
	protected ref JMVehicleMetaData m_Vehicle;
	protected ref JMVehiclesMenu m_COTVehicleMenu;

	void JMVehiclesListEntry(Widget parent, JMVehiclesMenu menu, JMVehicleMetaData vehicle)
	{
		m_Root					= GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/vehicles/Vehicles_List_Entry_New.layout", parent );
		m_VehicleName			= TextWidget.Cast( m_Root.FindAnyWidget( "vehicle_name" ) );
		m_VehicleID				= TextWidget.Cast( m_Root.FindAnyWidget( "vehicle_id" ) );
		m_EditButton			= ButtonWidget.Cast( m_Root.FindAnyWidget( "edit_button" ) );
		m_VehicleStatusIcon		= ImageWidget.Cast( m_Root.FindAnyWidget( "vehicle_status_icon" ) );
		
		m_COTVehicleMenu		= menu;
		m_Vehicle				= vehicle;
		
		m_Root.SetHandler( this );
		
		SetEntry();
	}

	void ~JMVehiclesListEntry()
	{
		if (g_Game && m_Root)
			m_Root.Unlink();
	}

	void SetSort(int sort, bool immedUpdate = true)
	{
		m_Sort = sort;
		m_Root.SetSort(sort, immedUpdate);
		SetEntry();
	}

	string GetLabel()
	{
		return m_Label;
	}

	void SetEntry()
	{
		if ( m_Vehicle )
		{
			m_VehicleName.SetText( (m_Sort + 1).ToString() + " - " + m_Vehicle.m_DisplayName );

			string id = " - ID: " + m_Vehicle.m_NetworkIDHigh.ToString() + " " + m_Vehicle.m_NetworkIDLow.ToString();
			m_VehicleID.SetText( id );

			m_Label = m_Vehicle.m_DisplayName + id;
			
			if ( m_Vehicle.m_DestructionType != JMDT_NONE )
			{
				m_VehicleStatusIcon.SetColor( ARGB( 255,255,0,0 ) );
			} else
			{
				m_VehicleStatusIcon.SetColor( ARGB( 255,0,255,0 ) );
			}
		}
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if ( m_EditButton && w == m_EditButton )
		{
			if ( m_COTVehicleMenu && m_Vehicle )
			{
				m_COTVehicleMenu.SetVehicleInfo( m_Vehicle );
				return true;
			}
		}
		
		return false;
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if ( m_EditButton && w == m_EditButton )
		{
			m_VehicleName.SetColor( ARGB( 255, 0, 0, 0 ) );
			m_VehicleID.SetColor( ARGB( 255, 0, 0, 0 ) );
		}

		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if ( m_EditButton && w == m_EditButton )
		{
			m_VehicleName.SetColor( ARGB( 255, 255, 255, 255 ) );
			m_VehicleID.SetColor( ARGB( 255, 255, 255, 255 ) );
		}

		return false;
	}
};
