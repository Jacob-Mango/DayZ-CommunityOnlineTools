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

	bool Equals(JMTeleportLocation location, float tolerance = 0.1, bool ignoreCase = true)
	{
		string name = Name;
		string type = Type;
		string otherName = location.Name;
		string otherType = location.Type;

		if (ignoreCase)
		{
			name.ToLower();
			type.ToLower();
			otherName.ToLower();
			otherType.ToLower();
		}

		if (otherName != name)
			return false;

		if (otherType != type)
			return false;

		if (!Math.IsPointInCircle(location.Position, tolerance, Position))
			return false;

		if (Math.AbsFloat(location.Radius - Radius) > tolerance)
			return false;

		return true;
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
}