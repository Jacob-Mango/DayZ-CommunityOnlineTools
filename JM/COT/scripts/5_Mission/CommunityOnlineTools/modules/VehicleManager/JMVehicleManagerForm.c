class JMVehicleManagerForm extends JMFormBase
{
	private JMVehicleManagerModule m_Module;

	void JMVehicleManagerForm()
	{
	}

	void ~JMVehicleManagerForm()
	{
	}

	protected override bool SetModule( ref JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{

	}

	override void OnShow()
	{
		ref array<ref ExpansionVehicleMetaData> vehicles = new array<ref ExpansionVehicleMetaData>;
	}

	override void OnHide() 
	{
	}
}