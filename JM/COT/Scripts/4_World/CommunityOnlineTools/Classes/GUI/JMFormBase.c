class JMFormBase extends ScriptedWidgetEventHandler 
{
	protected Widget layoutRoot;
	
	protected CF_Window m_Window;
	
	#ifndef CF_WINDOWS
	protected JMWindowBase window;
	#endif

	bool m_IsShown;

	void JMFormBase() 
	{
	}

	void ~JMFormBase() 
	{
		OnHide();
	}

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );
	}

	void Init( CF_Window wdw, JMRenderableModuleBase mdl )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "Init").Add(wdw.ToString()).Add(mdl.ToString());
		#endif

		m_Window = wdw;
		
		#ifndef CF_WINDOWS
		window = wdw;
		#endif

		if ( SetModule( mdl ) )
		{
			mdl.SetForm(this);

			OnInit();

			OnClientPermissionsUpdated();

			OnShow();
			m_IsShown = true;
		}
	}

	protected bool SetModule( JMRenderableModuleBase mdl )
	{
		return false;
	}

	void OnInit()
	{
		
	}

	bool IsVisible()
	{
		return m_Window != null;
	}

	void SetSize( float w, float h )
	{
		if (!IsVisible()) return;
		
		m_Window.SetSize(w, h);
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

	JMConfirmation CreateConfirmation_One( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return window.CreateConfirmation_One( type, title, message, callBackOneName, callBackOne );
		#endif
	}

	JMConfirmation CreateConfirmation_One( JMConfirmationType type, string title, string message, string callBackOneName )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return CreateConfirmation_One( type, title, message, callBackOneName, "" );
		#endif
	}

	JMConfirmation CreateConfirmation_Two( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return window.CreateConfirmation_Two( type, title, message, callBackOneName, callBackOne, callBackTwoName, callBackTwo );
		#endif
	}

	JMConfirmation CreateConfirmation_Two( JMConfirmationType type, string title, string message, string callBackOneName, string callBackTwoName )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return CreateConfirmation_Two( type, title, message, callBackOneName, "", callBackTwoName, "" );
		#endif
	}

	JMConfirmation CreateConfirmation_Three( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo, string callBackThreeName, string callBackThree )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return window.CreateConfirmation_Three( type, title, message, callBackOneName, callBackOne, callBackTwoName, callBackTwo, callBackThreeName, callBackThree );
		#endif
	}

	JMConfirmation CreateConfirmation_Three( JMConfirmationType type, string title, string message, string callBackOneName, string callBackTwoName, string callBackThreeName )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return CreateConfirmation_Three( type, title, message, callBackOneName, "", callBackTwoName, "", callBackThreeName, "" );
		#endif
	}
}