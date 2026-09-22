class COT_ScriptedWidgetEventHandler: ScriptedWidgetEventHandler
{
	static ref CF_DoublyLinkedNodes_WeakRef<COT_ScriptedWidgetEventHandler> s_COT_All = new CF_DoublyLinkedNodes_WeakRef<COT_ScriptedWidgetEventHandler>();
	ref CF_DoublyLinkedNode_WeakRef<COT_ScriptedWidgetEventHandler> m_COT_Node;

	void COT_ScriptedWidgetEventHandler()
	{
		m_COT_Node = s_COT_All.Add(this);
	}

	void ~COT_ScriptedWidgetEventHandler()
	{
		if (!g_Game)
			return;

		if (s_COT_All)
			s_COT_All.Remove(m_COT_Node);

	#ifdef DIAG_DEVELOPER
	#ifdef DZ_Expansion_Core
		EXError.Info(this, "~");
	#endif
	#endif
	}

	//! Is the (root) layout element visible?
	bool IsVisible()
	{
		return false;
	}

	//! Is the control considered "open"? (i.e. a popup menu on a button)
	//! @note not functionally the same as IsVisible
	//! IsVisible can be true on controls registered as overlays while IsOpen can depend on control state
	//! (i.e. search box with suggestion list)
	bool IsOpen()
	{
		return false;
	}

	void DestroyLater()
	{
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(Destroy);
	}

	void Destroy()
	{
		delete this;
	}

	void DestroyWidget(Widget w)
	{
		if (w && w.ToString() != "INVALID")
		{
		#ifdef DIAG
			CF_Log.Info("Unlinking %1 of %2", w.ToString(), ToString());
		#endif
			w.Unlink();
		}
	}

	//! Dismiss a floating overlay - a dropdown list, a context menu, a value
	//! prompt. Declared here rather than on UIActionBase because JMFormBase
	//! lives in 4_World and cannot see 5_Mission types: this class is the only
	//! common ancestor a form can hold a registry of. No-op for controls that
	//! do not float.
	void Close()
	{
	}

	//! Enable/disable this control from a permission key. Same reasoning as
	//! Close() - the real implementation is UIActionBase's. No-op on the base.
	//!
	//! NOT named UpdatePermission(). Enforce has no method overloading, and
	//! JMFormBase - which inherits this class - has to keep upstream COT's
	//! two-argument UpdatePermission( control, permission ) helper, because
	//! third-party forms call it (DayZ-Expansion's PersonalStorage, AI and
	//! Hardline JMPlayerForm modules all do). Two same-named methods on one
	//! class are indistinguishable to the call resolver and crash the script
	//! compiler, so the polymorphic one-argument version carries its own name.
	void COT_ApplyPermission( string permission )
	{
	}

	//! Tab-strip contract, for the same reason as Close(): JMFormBase (4_World)
	//! holds its tab strip as this type and drives it through these. The real
	//! implementations are UIActionTabs'. No-ops / -1 / null on the base.

	//! Index of the selected tab, -1 when there is none.
	int COT_TabSelection()
	{
		return -1;
	}

	//! Append a tab whose panel is created inside `host`. Returns the tab's
	//! index, or -1 if the strip could not add it.
	int COT_AddTab( string label, string icon, Widget host )
	{
		return -1;
	}

	//! Attach an existing panel to tab `tab`. Returns the panel's id, or -1.
	int COT_AddPanel( int tab, Widget panel )
	{
		return -1;
	}

	//! Remove tab `tab` (and its panels). The other tabs keep their ids.
	void COT_RemoveTabById( int tab )
	{
	}

	void COT_RemovePanelById( int panelId )
	{
	}

	void COT_RemovePanel( Widget panel )
	{
	}

	//! The first panel of tab `idx`, or null.
	Widget COT_TabContent( int idx )
	{
		return null;
	}

	void COT_SetTabEnabled( int idx, bool enabled )
	{
	}

	bool COT_IsTabEnabled( int idx )
	{
		return false;
	}

	void COT_SetTabVisible( int idx, bool visible )
	{
	}

	//! true selects the tab; false on the selected tab selects the next visible one.
	void COT_SetTabFocused( int idx, bool focused )
	{
	}

	//! Fit a tab strip to `width` pixels, wrapping the tabs onto more rows when they do not fit on
	//! one. Returns the height the strip gained over a single row, so the form that pins the strip
	//! can make room; 0 when it did not wrap, and for anything that is not a tab strip.
	float COT_FitTabs( float width )
	{
		return 0;
	}
}

class COT_WidgetHolder
{
	void DestroyLater()
	{
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(Destroy);
	}

	void Destroy()
	{
		delete this;
	}

	void DestroyWidget(Widget w)
	{
		if (w && w.ToString() != "INVALID")
		{
		#ifdef DIAG
			CF_Log.Info("Unlinking %1 of %2", w.ToString(), ToString());
		#endif
			w.Unlink();
		}
	}
}
