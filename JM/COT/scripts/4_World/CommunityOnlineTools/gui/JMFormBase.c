class JMFormBase extends ScriptedWidgetEventHandler 
{
	protected ref Widget layoutRoot;
	
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
		
		window.OnFormLoaded();
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

	void OnFocus()
	{
	}

	void OnUnfocus()
	{
	}

	void OnClientPermissionsUpdated()
	{
	}
	
	void Update() 
	{
		
	}

	Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	void CreateConfirmation_One( string title, string message, string callBackOneName, string callBackOne )
	{
		window.CreateConfirmation_One( title, message, callBackOneName, callBackOne );
	}

	void CreateConfirmation_Two( string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo )
	{
		window.CreateConfirmation_Two( title, message, callBackOneName, callBackOne, callBackTwoName, callBackTwo );
	}

	void CreateConfirmation_Three( string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo, string callBackThreeName, string callBackThree )
	{
		window.CreateConfirmation_Three( title, message, callBackOneName, callBackOne, callBackTwoName, callBackTwo, callBackThreeName, callBackThree );
	}
}