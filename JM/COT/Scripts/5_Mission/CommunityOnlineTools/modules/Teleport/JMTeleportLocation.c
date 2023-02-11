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

	void Write( ParamsWriteContext ctx )
	{
		ctx.Write( Name );
		ctx.Write( Permission );
		ctx.Write( Position );
		ctx.Write( Radius );
	}

	bool Read( ParamsReadContext ctx )
	{
		if ( !ctx.Read( Name ) )
			return false;
		if ( !ctx.Read( Permission ) )
			return false;
		if ( !ctx.Read( Position ) )
			return false;
		if ( !ctx.Read( Radius ) )
			return false;

		return true;
	}
};