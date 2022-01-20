class JMFormBase extends ScriptedWidgetEventHandler 
{
	protected Widget layoutRoot;
	
	protected CF_Window m_Window;

	protected COTConfirmationModal m_Confirmation;
	
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

		auto confirmationPanel = layoutRoot.FindAnyWidget("confirmation_panel");

		if (confirmationPanel)
		{
			confirmationPanel.GetScript(m_Confirmation);
		}

		if (m_Confirmation)
		{
			m_Confirmation.Init(this);
		}
	}

	void Init( CF_Window wdw, JMRenderableModuleBase mdl )
	{
		m_Window = wdw;
		
		if ( SetModule( mdl ) )
		{
			OnInit();

			OnShow();
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

	COTConfirmationModal CreateConfirmation_One( COTConfirmationModalType type, string title, string message, string callBackOneName, string callBackOne )
	{
		if (!m_Confirmation)
			return null;

		m_Confirmation.CreateConfirmation_One( type, title, message, callBackOneName, callBackOne );
		
		return m_Confirmation;
	}

	COTConfirmationModal CreateConfirmation_One( COTConfirmationModalType type, string title, string message, string callBackOneName )
	{
		return CreateConfirmation_One( type, title, message, callBackOneName, "" );
	}

	COTConfirmationModal CreateConfirmation_Two( COTConfirmationModalType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo )
	{
		if (!m_Confirmation)
			return null;

		m_Confirmation.CreateConfirmation_Two( type, title, message, callBackOneName, callBackOne, callBackTwoName, callBackTwo );
		
		return m_Confirmation;
	}

	COTConfirmationModal CreateConfirmation_Two( COTConfirmationModalType type, string title, string message, string callBackOneName, string callBackTwoName )
	{
		return CreateConfirmation_Two( type, title, message, callBackOneName, "", callBackTwoName, "" );
	}

	COTConfirmationModal CreateConfirmation_Three( COTConfirmationModalType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo, string callBackThreeName, string callBackThree )
	{
		if (!m_Confirmation)
			return null;

		m_Confirmation.CreateConfirmation_Three( type, title, message, callBackOneName, callBackOne, callBackTwoName, callBackTwo, callBackThreeName, callBackThree );
		
		return m_Confirmation;
	}

	COTConfirmationModal CreateConfirmation_Three( COTConfirmationModalType type, string title, string message, string callBackOneName, string callBackTwoName, string callBackThreeName )
	{
		return CreateConfirmation_Three( type, title, message, callBackOneName, "", callBackTwoName, "", callBackThreeName, "" );
	}
};
