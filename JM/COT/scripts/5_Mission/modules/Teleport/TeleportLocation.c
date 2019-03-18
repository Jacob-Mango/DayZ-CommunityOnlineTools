class TeleportLocation
{
	string Name;
	vector Position;
	float Radius;

	void TeleportLocation( string name, vector position, float radius )
	{
		Name = name;
		Position = position;
		Radius = radius;
	}
}