class JMLoadoutItem
{
	string m_Classname;
	vector m_LocalPosition;
	vector m_LocalRotation;
	ref JMLoadoutItemData m_Data
	autoptr TStringArray m_ConstructionParts;
	ref array< ref JMLoadoutSubItem > m_Attachments;
};
