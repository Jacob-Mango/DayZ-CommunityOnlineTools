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

	void Write( ParamsWriteContext ctx )
	{
		ctx.Write( Name );
		ctx.Write( Position );
		ctx.Write( Radius );
	}

	bool Read( ParamsReadContext ctx )
	{
		if ( !ctx.Read( Name ) )
			return false;
		if ( !ctx.Read( Position ) )
			return false;
		if ( !ctx.Read( Radius ) )
			return false;

		return true;
	}
};