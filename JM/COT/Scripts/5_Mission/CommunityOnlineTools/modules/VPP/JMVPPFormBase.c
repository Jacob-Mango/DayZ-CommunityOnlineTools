/*#ifdef VPPADMINTOOLS
class JMVPPFormBase : JMFormBase
{
	private ref AdminHudSubMenu m_VPPMenu;
	private JMVPPModuleBase m_VPPModule;

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_VPPModule, mdl );
	}
	
	override void Update() 
	{
		m_VPPMenu.OnUpdate( 1.0 / 40.0 );
	}

	override void Init( JMWindowBase wdw, JMRenderableModuleBase mdl )
	{
		super.Init( wdw, mdl );
		
		Class.CastTo( m_VPPMenu, m_VPPModule.GetMenuClassName().ToType().Spawn() );

		layoutRoot = m_VPPMenu.InitializeCOT( this, wdw );
		layoutRoot.SetHandler( this );
	}
};
#endif*/