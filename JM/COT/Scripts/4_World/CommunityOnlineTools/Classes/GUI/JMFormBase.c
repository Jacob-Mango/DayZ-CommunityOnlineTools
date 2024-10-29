class JMFormBase: COT_ScriptedWidgetEventHandler 
{
#ifdef DIAG
	static int s_JMFormBaseCount;
#endif

	protected Widget layoutRoot;
	
	protected CF_Window m_Window;
	
	#ifndef CF_WINDOWS
	protected JMWindowBase window;
	#endif

	bool m_IsShown;

	void JMFormBase() 
	{
	#ifdef DIAG
		s_JMFormBaseCount++;
		CF_Log.Info("JMFormBase count: " + s_JMFormBaseCount);
	#endif
	}

	void ~JMFormBase()
	{
		if (!GetGame())
			return;

	#ifdef DIAG
		auto trace = CF_Trace_0(this);
	#endif

		OnHide();

		//! @note this should not be necessary since if the JMWindow handling this JMForm is destroyed,
		//! it'll unlink its own layoutRoot and all its children with it. This is just here as a safety.
		DestroyWidget(layoutRoot);

	#ifdef DIAG
		s_JMFormBaseCount--;
		if (s_JMFormBaseCount <= 0)
			CF_Log.Info("JMFormBase count: " + s_JMFormBaseCount);
	#endif
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

	JMConfirmation CreateConfirmation_One( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, int btnIdOffset = -1 )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return window.CreateConfirmation_One( type, title, message, callBackOneName, callBackOne, btnIdOffset );
		#endif
	}

	JMConfirmation CreateConfirmation_One( JMConfirmationType type, string title, string message, string callBackOneName, int btnIdOffset = -1 )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return CreateConfirmation_One( type, title, message, callBackOneName, "", btnIdOffset );
		#endif
	}

	JMConfirmation CreateConfirmation_Two( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo, int btnIdOffset = -1 )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return window.CreateConfirmation_Two( type, title, message, callBackOneName, callBackOne, callBackTwoName, callBackTwo, btnIdOffset );
		#endif
	}

	JMConfirmation CreateConfirmation_Two( JMConfirmationType type, string title, string message, string callBackOneName, string callBackTwoName, int btnIdOffset = -1 )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return CreateConfirmation_Two( type, title, message, callBackOneName, "", callBackTwoName, "", btnIdOffset );
		#endif
	}

	JMConfirmation CreateConfirmation_Three( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo, string callBackThreeName, string callBackThree, int btnIdOffset = -1 )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return window.CreateConfirmation_Three( type, title, message, callBackOneName, callBackOne, callBackTwoName, callBackTwo, callBackThreeName, callBackThree, btnIdOffset );
		#endif
	}

	JMConfirmation CreateConfirmation_Three( JMConfirmationType type, string title, string message, string callBackOneName, string callBackTwoName, string callBackThreeName, int btnIdOffset = -1 )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return CreateConfirmation_Three( type, title, message, callBackOneName, "", callBackTwoName, "", callBackThreeName, "", btnIdOffset );
		#endif
	}
	
	bool CreateAdvancedPlayerConfirm(string title, string funcName, bool confirmSelf = true, bool callbackOnNoConfirmation = true)
	{
		auto selected = JM_GetSelected();

		if (!selected)
			return false;

		auto players = selected.GetPlayers();

		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( players[0] );

		if (!inst)
			return false;

		int count = players.Count();
		if (count > 1)
		{
			CreateConfirmation_Three( JMConfirmationType.INFO, title, string.Format(Widget.TranslateString("#STR_COT_WARNING_PLAYERS_MESSAGE_BODY"), count.ToString()), "#STR_COT_GENERIC_CANCEL", "", inst.GetName(), funcName, "#STR_COT_GENERIC_CONFIRM", funcName );
			return true;
		}
		else
		{
			if (inst != GetPermissionsManager().GetClientPlayer() )
			{
				CreateConfirmation_Three( JMConfirmationType.INFO, title, string.Format(Widget.TranslateString("#STR_COT_WARNING_SELECTEDPLAYER_MESSAGE_BODY"), inst.GetName()), "#STR_COT_GENERIC_CANCEL", "", inst.GetName(), funcName, "#STR_COT_GENERIC_SELF", funcName, 3 );
				return true;
			}
			else if (confirmSelf)
			{
				CreateConfirmation_Two( JMConfirmationType.INFO, title, "#STR_COT_WARNING_SELECTEDSELF_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", funcName );
				return true;
			}
		}
		
		if ( callbackOnNoConfirmation && funcName != string.Empty )
			GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallByName( this, funcName, new Param1<JMConfirmation>( NULL ) );

		return false;
	}

	bool CreateAdvancedObjectConfirm(string funcName, bool callbackOnNoConfirmation = true)
	{
		auto selected = JM_GetSelected();

		if (!selected)
			return false;

		auto objects = selected.GetObjects();

		int count = objects.Count();
		if (count > 1)
		{
			CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_WARNING_OBJECTS_MESSAGE_HEADER", string.Format(Widget.TranslateString("#STR_COT_WARNING_OBJECTS_MESSAGE_BODY"), count.ToString()), "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", funcName );
			return true;
		}
		
		if ( callbackOnNoConfirmation && funcName != string.Empty )
			GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallByName( this, funcName, new Param1<JMConfirmation>( NULL ) );

		return false;
	}
};
