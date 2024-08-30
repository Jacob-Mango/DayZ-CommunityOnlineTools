class JMLoadoutItem
{
	string m_Classname;
	vector m_LocalPosition;
	vector m_LocalRotation;
	float m_Health;
	float m_Quantity;
	int m_LiquidType;
	int m_Temperature;
	ref array< ref JMLoadoutItem > m_Attachments;
};
