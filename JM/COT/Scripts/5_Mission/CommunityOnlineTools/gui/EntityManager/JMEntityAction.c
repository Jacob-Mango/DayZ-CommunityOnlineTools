// =============================================================================
//  JMEntityAction
//
//  Describes one button the UI will render in the entity options panel.
//  Adapter returns a list of these; the form wires them up.
// =============================================================================
class JMEntityAction
{
	string  m_Id;            // stable id passed back to adapter.OnAction()
	string  m_Label;          // button label
	string  m_Permission;     // required permission (empty = no check)
	bool    m_IsDestructive;  // true -> ConfirmInline button instead of plain
	bool    m_IsBulk;         // true -> shown in list-level bulk actions
	string  m_Tooltip;        // hover tooltip text; empty = no tooltip

	void JMEntityAction( string id, string label, string permission = "", bool destructive = false, bool bulk = false, string tooltip = "" )
	{
		m_Id             = id;
		m_Label          = label;
		m_Permission     = permission;
		m_IsDestructive  = destructive;
		m_IsBulk         = bulk;
		m_Tooltip        = tooltip;
	}
}
