class JMTeleportLocation
{
	string Type;
	string Name;
	vector Position;
	float Radius;

	void JMTeleportLocation( string type, string name, vector position, float radius )
	{
		Type = type;
		Name = name;
		Position = position;
		Radius = radius;
	}

	void Write( ParamsWriteContext ctx )
	{
		ctx.Write( Type );
		ctx.Write( Name );
		ctx.Write( Position );
		ctx.Write( Radius );
	}

	bool Read( ParamsReadContext ctx )
	{
		if ( !ctx.Read( Type ) )
			return false;
			
		if ( !ctx.Read( Name ) )
			return false;

		if ( !ctx.Read( Position ) )
			return false;

		if ( !ctx.Read( Radius ) )
			return false;

		return true;
	}
};