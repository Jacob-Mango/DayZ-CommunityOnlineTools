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

	//! Whether this control is currently shown - used by the Escape priority
	//! chain (JMFormBase.HasOpenOverlay/CloseOpenOverlays) to tell an open
	//! popup from a closed-but-still-registered one. Same reasoning as
	//! Close(): declared here so a form's overlay registry, typed to this
	//! common ancestor, can ask any registered control without knowing its
	//! concrete type. No-op/false on the base.
	bool IsVisible()
	{
		return false;
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
