//! Base class for a tab's controller - the object that owns one tab's widgets and state.
//!
//! A form registers one per tab index and its lifecycle is driven for it:
//!
//!     m_TabFlagged = new JMAntiCheatFormTabFlagged( this );
//!     RegisterTab( m_TabIdFlagged, m_TabFlagged );   // m_TabIdFlagged = m_Tabs.AddTab( label, icon, panel )
//!
//! Members here are prefixed m_Tab* on purpose: subclasses already keep their own
//! typed `m_Form`, and a same-named member in a derived class is a redefinition.
//!
//! JMFormBase then calls, in this order, as the user moves around:
//!
//!     OnCreate( panel )   once, the first time the tab is selected - build widgets
//!     OnFocus()           the tab became the selected one
//!     OnUpdate()          after create/focus and whenever the form calls UpdateTab / UpdateActiveTab
//!                         (only ever while focused, so a timer never repaints a hidden tab)
//!     OnUnfocus()         another tab was selected - hide hover panels, stop timers
//!     OnResize( w, h )   the form was resized (created tabs only)
//!     OnDestruct()        the form is going away - release anything the widgets do not own
//!
//! A tab with no controller class (a few inline widgets) overrides the matching
//! JMFormBase.OnTab* hooks with its index instead; both forms of the same grammar.
class JMFormTab
{
	protected JMFormBase m_TabForm;
	protected int m_TabId = -1;
	protected Widget m_TabPanel;
	protected bool m_TabFocused;
	protected bool m_TabEnabled = true;

	JMFormBase GetForm()
	{
		return m_TabForm;
	}

	Widget GetPanel()
	{
		return m_TabPanel;
	}

	//! This tab's index in its form's tab strip. The form assigns it when the tab is
	//! registered (RegisterTab, or AddTab( ..., controller )) - a tab never picks its own number, so
	//! two mods adding tabs to the same form can never collide. -1 until registered.
	int GetTabId()
	{
		return m_TabId;
	}

	bool IsEnabled()
	{
		return m_TabEnabled;
	}

	bool IsFocused()
	{
		return m_TabFocused;
	}

	void SetEnabled( bool enabled )
	{
		m_TabEnabled = enabled;
	}

	void SetFocused( bool focused )
	{
		if ( m_TabFocused == focused )
			return;

		m_TabFocused = focused;

		if ( focused )
			OnFocus();
		else
			OnUnfocus();
	}

	//! Called by JMFormBase.RegisterTab; not needed in a subclass constructor.
	void AttachToForm( JMFormBase form, int tabId )
	{
		m_TabForm = form;
		m_TabId = tabId;
	}

	//! Build this tab's widgets into `panel`. Called once.
	void OnCreate( Widget panel )
	{
		m_TabPanel = panel;
	}

	//! Repaint from current state. Only called while the tab is focused.
	void OnUpdate()
	{
	}

	void OnFocus()
	{
	}

	void OnUnfocus()
	{
	}

	void OnResize( float w, float h )
	{
	}

	void OnDestruct()
	{
	}
}
