class JMTeleportLocation
{
	string Name;
	string Permission;
	vector Position;
	float Radius;

	void JMTeleportLocation( string name, string permission, vector position, float radius )
	{
		Name = name;
		Permission = permission;
		Position = position;
		Radius = radius;
	}
};