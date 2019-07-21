class JMTeleportLocation
{
	string Name;
	vector Position;
	float Radius;

	void JMTeleportLocation( string name, vector position, float radius )
	{
		Name = name;
		Position = position;
		Radius = radius;
	}
}