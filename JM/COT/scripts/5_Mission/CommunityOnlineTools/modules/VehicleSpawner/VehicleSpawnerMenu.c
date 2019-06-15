class VehicleSpawnerMenu extends Form
{
	protected Widget m_ActionsWrapper;

	protected ref array< ref UIActionButton > m_VehicleButtons;

	void VehicleSpawnerMenu()
	{
		m_VehicleButtons = new ref array< ref UIActionButton >;
	}

	void ~VehicleSpawnerMenu()
	{
	}

	override string GetTitle()
	{
		return "Vehicle Spawner";
	}
	
	override string GetIconName()
	{
		return "VS";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override void OnInit( bool fromMenu )
	{
		m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );
	}

	override void OnShow()
	{
		VehicleSpawnerModule gm = VehicleSpawnerModule.Cast( module );

		if ( gm == NULL ) return;

		string name;
		ref UIActionButton button;

		for ( int i = 0; i < gm.GetVehicles().Count(); i++ )
		{
			name = gm.GetVehicles()[i];

			button = UIActionManager.CreateButton( m_ActionsWrapper, "Spawn " + name + " at Cursor", this, "SpawnVehicle" );
			button.SetData( new VS_Button_Data( name ) );

			m_VehicleButtons.Insert( button );
		}
	}

	override void OnHide() 
	{
		for ( int j = 0; j < m_VehicleButtons.Count(); j++ )
		{
			m_ActionsWrapper.RemoveChild( m_VehicleButtons[j].GetLayoutRoot() );
			m_VehicleButtons[j].GetLayoutRoot().Unlink();
		}

		m_VehicleButtons.Clear();
	}

	void SpawnVehicle( UIEvent eid, ref UIActionButton action ) 
	{
		ref VS_Button_Data data = VS_Button_Data.Cast( action.GetData() );
		if ( !data ) return;

		GetRPCManager().SendRPC( "COT_VehicleSpawner", "SpawnCursor", new Param2< string, vector >( data.ClassName, GetCursorPos() ) );
	}
}