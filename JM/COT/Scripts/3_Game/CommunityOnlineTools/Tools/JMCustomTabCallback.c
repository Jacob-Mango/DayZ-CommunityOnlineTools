//! One entry in a form's dynamic-tab registry (JMPlayerForm.AddTab and any
//! other form that adopts the same hook). Carries the callback TARGET
//! alongside its function name, so AddTab can dispatch to a per-tab class
//! object instead of only ever calling back on the form itself.
class JMCustomTabCallback
{
	Class  m_Instance;
	string m_FuncName;

	void JMCustomTabCallback( Class instance, string funcName )
	{
		m_Instance = instance;
		m_FuncName = funcName;
	}
}
