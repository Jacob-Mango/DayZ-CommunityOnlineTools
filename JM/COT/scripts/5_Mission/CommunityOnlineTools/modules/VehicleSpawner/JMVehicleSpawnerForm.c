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
		vehicles.Copy( m_Module.GetVehicles() );

		JMStatics.SortStringArray( vehicles );

		for ( int i = 0; i < vehicles.Count(); i++ )
		{
			Widget wrapper = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 1, 2 );
			
			string name = vehicles[i];

			UIActionManager.CreateText( wrapper, name );

			UIActionButton button = UIActionManager.CreateButton( wrapper, "Cursor", this, "SpawnVehicle" );
			button.SetData( new JMVehicleSpawnerButtonData( name ) );

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