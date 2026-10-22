//! TODO: Remove this and use vanilla EConstructionInfoCategories instead
enum JMConstructionPartState
{
	UNKNOWN = 0,
	BUILT,
	CAN_BUILD,
	REQUIRED_PART_NOT_BUILT,
	CONFLICTING_PART,
	NOT_ENOUGH_MATERIALS //TODO: link this to a permission
}

//! TODO: Remove this and use vanilla ConstructionPart instead
class JMConstructionPartData : UIActionData
{
	string m_Name;
	string m_DisplayName;
	string m_MainPartName;
	JMConstructionPartState m_State;
}