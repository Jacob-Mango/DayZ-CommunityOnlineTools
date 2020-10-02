enum JMConstructionPartState
{
	UNKNOWN = 0,
	BUILT,
	CAN_BUILD,
	REQUIRED_PART_NOT_BUILT,
	CONFLICTING_PART,
	NOT_ENOUGH_MATERIALS //TODO: link this to a permission
};

class JMConstructionPartData : UIActionData
{
	string m_Name;
	string m_DisplayName;
	JMConstructionPartState m_State;
};