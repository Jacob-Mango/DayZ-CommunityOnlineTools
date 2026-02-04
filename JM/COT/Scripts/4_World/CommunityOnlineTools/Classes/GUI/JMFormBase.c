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
		if (!g_Game)
			return;

	#ifdef DIAG
		auto trace = CF_Trace_0(this);
	#endif

		OnHide();

		//! @note this should not be necessary since if the JMWindow handling this JMForm is destroyed,
		//! it'll unlink its own layoutRoot and all its children with it. This is just here as a safety.
	#ifdef DAYZ_1_28
		DestroyWidget(layoutRoot);
	#endif

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

	/**
	 * @brief Create confirmation asking whether to act on multiple selected players, single (first) selected player or self
	 *
	 * @param callbackSelectedPlayersMulti
	 * @param callbackSelectedPlayerSingle
	 * @param callbackSelf
	 * @param confirmSelf  If the selected player is the client player (aka self), whether to show confirmation or not
	 * @param executeCallbackOnNoConfirmation  If the selected player is the client player (aka self) and confirmSelf is false, whether to execute the respective callback w/o confirmation
	 * 
	 * @return confirmation if created, else null
	 */
	JMConfirmation CreateAdvancedPlayerConfirm(string title, string callbackSelectedPlayersMulti, string callbackSelectedPlayerSingle, string callbackSelf, bool confirmSelf = true, bool executeCallbackOnNoConfirmation = true)
	{
		auto selected = JM_GetSelected();

		if (!selected)
			return null;

		auto players = selected.GetPlayers();

		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( players[0] );

		if (!inst)
			return null;

		int count = players.Count();
		if (count > 1)
		{
			if (inst != GetPermissionsManager().GetClientPlayer() && callbackSelectedPlayerSingle)
				return CreateConfirmation_Three( JMConfirmationType.INFO, title, string.Format(Widget.TranslateString("#STR_COT_WARNING_PLAYERS_MESSAGE_BODY"), count.ToString()), "#STR_COT_GENERIC_CANCEL", "", inst.GetName(), callbackSelectedPlayerSingle, "#STR_COT_GENERIC_CONFIRM", callbackSelectedPlayersMulti );
			else if (callbackSelf)
				return CreateConfirmation_Three( JMConfirmationType.INFO, title, string.Format(Widget.TranslateString("#STR_COT_WARNING_PLAYERS_MESSAGE_BODY"), count.ToString()), "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_SELF", callbackSelf, "#STR_COT_GENERIC_CONFIRM", callbackSelectedPlayersMulti );
			else
				return CreateConfirmation_Two( JMConfirmationType.INFO, title, "#STR_COT_WARNING_PLAYERS_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", callbackSelectedPlayersMulti );
		}
		else
		{
			if (inst != GetPermissionsManager().GetClientPlayer() && callbackSelectedPlayerSingle)
			{
				return CreateConfirmation_Three( JMConfirmationType.INFO, title, string.Format(Widget.TranslateString("#STR_COT_WARNING_SELECTEDPLAYER_MESSAGE_BODY"), inst.GetName()), "#STR_COT_GENERIC_CANCEL", "", inst.GetName(), callbackSelectedPlayerSingle, "#STR_COT_GENERIC_SELF", callbackSelf );
			}
			else if (callbackSelf)
			{
				if (confirmSelf)
					return CreateConfirmation_Two( JMConfirmationType.INFO, title, "#STR_COT_WARNING_SELECTEDSELF_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_SELF", callbackSelf );
				else if (executeCallbackOnNoConfirmation)
					g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallByName( this, callbackSelf, new Param1<JMConfirmation>( NULL ) );
			}
		}

		return null;
	}

	/**
	 * @brief Create confirmation if acting on multiple selected objects or no confirmation if single selected object
	 *
	 * @param callbackSelectedObjectsMulti
	 * @param callbackSelectedObjectSingle
	 * @param executeCallbackOnNoConfirmation  If single selected object, whether to execute the respective callback w/o confirmation
	 * 
	 * @return confirmation if created, else null
	 */
	JMConfirmation CreateAdvancedObjectConfirm(string callbackSelectedObjectsMulti, string callbackSelectedObjectSingle, bool executeCallbackOnNoConfirmation = true)
	{
		auto selected = JM_GetSelected();

		if (!selected)
			return null;

		auto objects = selected.GetObjects();

		int count = objects.Count();
		if (count > 1)
		{
			return CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_WARNING_OBJECTS_MESSAGE_HEADER", string.Format(Widget.TranslateString("#STR_COT_WARNING_OBJECTS_MESSAGE_BODY"), count.ToString()), "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", callbackSelectedObjectsMulti );
		}
		
		if ( executeCallbackOnNoConfirmation && callbackSelectedObjectSingle != string.Empty )
			g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallByName( this, callbackSelectedObjectSingle, new Param1<JMConfirmation>( NULL ) );

		return null;
	}
	
	bool CreateAdvancedPlayerConfirm(string title, string funcName, bool confirmSelf = true, bool callbackOnNoConfirmation = true)
	{
		ErrorEx("DEPRECATED, use different callbacks for the different options", ErrorExSeverity.WARNING);
		if (CreateAdvancedPlayerConfirm(title, funcName, funcName, funcName, confirmSelf, callbackOnNoConfirmation))
			return true;
		return false;
	}
	
	bool CreateAdvancedObjectConfirm(string funcName, bool callbackOnNoConfirmation = true)
	{
		ErrorEx("DEPRECATED, use different callbacks for the different options", ErrorExSeverity.WARNING);
		if (CreateAdvancedObjectConfirm(funcName, funcName, callbackOnNoConfirmation))
			return true;
		return false;
	}
}
