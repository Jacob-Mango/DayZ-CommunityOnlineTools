class JMVehicleSpawnerForm extends JMFormBase
{
	private UIActionScroller m_sclr_MainActions;
	private Widget m_ActionsWrapper;

	private ref array< ref UIActionButton > m_VehicleButtons;

	private JMVehicleSpawnerModule m_Module;

	void JMVehicleSpawnerForm()
	{
		m_VehicleButtons = new array< ref UIActionButton >;
	}

	void ~JMVehicleSpawnerForm()
	{
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_sclr_MainActions = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
		m_ActionsWrapper = m_sclr_MainActions.GetContentWidget();

		m_sclr_MainActions.UpdateScroller();
	}

	override void OnShow()
	{
		array< string > vehicles = new array< string >;
		array< string > vehiclesDisplay = new array< string >;
		vehicles.Copy( m_Module.GetVehicles() );

		for ( int i = 0; i < vehicles.Count(); i++ )
		{
			string displayName;
			GetGame().ConfigGetText( "cfgVehicles " + vehicles[i] + " displayName", displayName );
			string text = Widget.TranslateString( "#" + displayName );

			if ( text == "" || text.Get( 0 ) == " " )
			{
				vehiclesDisplay.Insert( displayName + " (" + vehicles[i] + ")" );
			} else
			{
				vehiclesDisplay.Insert( text + " (" + vehicles[i] + ")" );
			}
		}

		JMStatics.SortStringArrayKVPair( vehiclesDisplay, vehicles );

		for ( i = 0; i < vehicles.Count(); i++ )
		{
			if ( vehicles[i] == "" )
				continue;
				
			Widget wrapper = UIActionManager.CreatePanel( m_ActionsWrapper, 0x00000000, 30 );
			
			UIActionBase name = UIActionManager.CreateText( wrapper, vehiclesDisplay[i] );

			name.SetWidth(0.8);

			UIActionButton button = UIActionManager.CreateButton( wrapper, "#STR_COT_VEHICLE_MODULE_CURSOR", this, "SpawnVehicle" );
			button.SetData( new JMVehicleSpawnerButtonData( vehicles[i] ) );
			button.SetWidth(0.2);
			button.SetPosition(0.8);

			m_VehicleButtons.Insert( button );
		}

		m_sclr_MainActions.UpdateScroller();
	}

	override void OnHide() 
	{
		//m_VehicleButtons.Clear();
		//
		//m_sclr_MainActions.UpdateScroller();
	}

	void SpawnVehicle( UIEvent eid, ref UIActionBase action ) 
	{
		JMVehicleSpawnerButtonData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_Module.SpawnPosition( data.ClassName, GetCursorPos() );
	}
}