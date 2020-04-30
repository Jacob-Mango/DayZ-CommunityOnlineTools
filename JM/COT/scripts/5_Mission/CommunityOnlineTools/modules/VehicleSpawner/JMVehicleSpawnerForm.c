class JMVehicleSpawnerForm extends JMFormBase
{
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

	protected override bool SetModule( ref JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );
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
				vehiclesDisplay.Insert( displayName );
			} else
			{
				vehiclesDisplay.Insert( text );
			}
		}

		JMStatics.SortStringArrayKVPair( vehiclesDisplay, vehicles );

		for ( i = 0; i < vehicles.Count(); i++ )
		{
			if ( vehicles[i] == "" )
				continue;
				
			Widget wrapper = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 1, 2 );
			
			UIActionManager.CreateText( wrapper, vehiclesDisplay[i] );

			UIActionButton button = UIActionManager.CreateButton( wrapper, "Cursor", this, "SpawnVehicle" );
			button.SetData( new JMVehicleSpawnerButtonData( vehicles[i] ) );

			m_VehicleButtons.Insert( button );
		}
	}

	override void OnHide() 
	{
		Widget child = m_ActionsWrapper.GetChildren();
		while ( child != NULL )
		{
			child.Unlink();

			child = child.GetSibling();
		}

		m_VehicleButtons.Clear();
	}

	void SpawnVehicle( UIEvent eid, ref UIActionBase action ) 
	{
		JMVehicleSpawnerButtonData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_Module.SpawnPosition( data.ClassName, GetCursorPos() );
	}
}