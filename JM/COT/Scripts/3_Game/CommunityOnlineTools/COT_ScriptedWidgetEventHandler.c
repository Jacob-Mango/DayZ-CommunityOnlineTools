class COT_ScriptedWidgetEventHandler: ScriptedWidgetEventHandler
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
	void UpdatePermission( string permission )
	{
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
