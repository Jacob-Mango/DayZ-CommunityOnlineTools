class COT_ScriptedWidgetEventHandler: ScriptedWidgetEventHandler
{
	void DestroyLater()
	{
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Destroy);
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

class COT_WidgetHolder
{
	void DestroyLater()
	{
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Destroy);
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
