class JMFormBase extends ScriptedWidgetEventHandler 
{
	protected Widget layoutRoot;
	
	protected JMWindowBase window;

	protected JMRenderableModuleBase module;

	void JMFormBase() 
	{
	}

	void ~JMFormBase() 
	{
	}

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );
	}

	void Init( JMWindowBase wdw, JMRenderableModuleBase mdl )
	{
		window = wdw;
		module = mdl;

		OnInit();
	}

	void OnInit()
	{
		
	}

	bool IsVisible()
	{
		return window.IsVisible();
	}

	void SetSize( float w, float h )
	{
		window.SetSize( w, h );
	}

	void OnShow()
	{
	}

	void OnHide() 
	{
	}

	override bool OnUpdate( Widget w )
	{
		Update();

		super.OnUpdate( w );

		return true;
	}

	void Update() 
	{
		
	}

	Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}
}