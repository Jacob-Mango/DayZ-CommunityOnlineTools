class JMFormBase extends ScriptedWidgetEventHandler 
{
	protected Widget layoutRoot;
	
	protected JMWindowBase window;

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

	void Init( ref JMWindowBase wdw, ref JMRenderableModuleBase mdl )
	{
		window = wdw;

		if ( SetModule( mdl ) )
		{
			OnInit();
			
			window.OnFormLoaded();
		}
	}

	protected bool SetModule( ref JMRenderableModuleBase mdl )
	{
		return false;
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

	void OnSettingsUpdated()
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

	ref JMConfirmation CreateConfirmation_One( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne )
	{
		return window.CreateConfirmation_One( type, title, message, callBackOneName, callBackOne );
	}

	ref JMConfirmation CreateConfirmation_One( JMConfirmationType type, string title, string message, string callBackOneName )
	{
		return CreateConfirmation_One( type, title, message, callBackOneName, "" );
	}

	ref JMConfirmation CreateConfirmation_Two( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo )
	{
		return window.CreateConfirmation_Two( type, title, message, callBackOneName, callBackOne, callBackTwoName, callBackTwo );
	}

	ref JMConfirmation CreateConfirmation_Two( JMConfirmationType type, string title, string message, string callBackOneName, string callBackTwoName )
	{
		return CreateConfirmation_Two( type, title, message, callBackOneName, "", callBackTwoName, "" );
	}

	ref JMConfirmation CreateConfirmation_Three( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo, string callBackThreeName, string callBackThree )
	{
		return window.CreateConfirmation_Three( type, title, message, callBackOneName, callBackOne, callBackTwoName, callBackTwo, callBackThreeName, callBackThree );
	}

	ref JMConfirmation CreateConfirmation_Three( JMConfirmationType type, string title, string message, string callBackOneName, string callBackTwoName, string callBackThreeName )
	{
		return CreateConfirmation_Three( type, title, message, callBackOneName, "", callBackTwoName, "", callBackThreeName, "" );
	}
}