// JMCameraWaypoint is defined in 3_Game/Entities/Cameras/JMCameraWaypoint.c

// ============================================================
//  JMCameraPath
//  A named list of waypoints — one saved path.
// ============================================================
class JMCameraPath
{
	string Name;
	ref array< ref JMCameraWaypoint > Waypoints;

	void JMCameraPath( string name = "" )
	{
		Name      = name;
		Waypoints = new array< ref JMCameraWaypoint >;
	}
}

// ============================================================
//  JMCameraBookmark
//  A named freecam position bookmark.
// ============================================================
class JMCameraBookmark
{
	string Name;
	vector Position;

	void JMCameraBookmark( string name = "", vector pos = vector.Zero )
	{
		Name     = name;
		Position = pos;
	}
}

// ============================================================
//  JMCameraSerialize
//  Root JSON object — holds all saved paths and bookmarks.
//  Stored at $profile:CommunityOnlineTools\CameraPaths.json
// ============================================================
class JMCameraSerialize : Managed
{
	ref array< ref JMCameraPath >     Paths;
	ref array< ref JMCameraBookmark > Bookmarks;

	static string GetFilePath()
	{
		return JMConstants.DIR_COT + "CameraPaths.json";
	}

	private void JMCameraSerialize()
	{
		Paths     = new array< ref JMCameraPath >;
		Bookmarks = new array< ref JMCameraBookmark >;
	}

	// ----------------------------------------------------------
	//  Load from disk (or return a fresh empty instance).
	// ----------------------------------------------------------
	static JMCameraSerialize Load()
	{
		JMCameraSerialize data = new JMCameraSerialize();

		string filepath = GetFilePath();
		if ( FileExist( filepath ) )
			JsonFileLoader< JMCameraSerialize >.JsonLoadFile( filepath, data );

		return data;
	}

	// ----------------------------------------------------------
	//  Persist to disk.
	// ----------------------------------------------------------
	void Save()
	{
		string dir = JMConstants.DIR_COT;
		if ( !FileExist( dir ) )
			MakeDirectory( dir );

		JsonFileLoader< JMCameraSerialize >.JsonSaveFile( GetFilePath(), this );
	}

	// ----------------------------------------------------------
	//  Path helpers
	// ----------------------------------------------------------
	int FindPath( string name )
	{
		for ( int i = 0; i < Paths.Count(); i++ )
		{
			if ( Paths[i].Name == name )
				return i;
		}
		return -1;
	}

	void SaveWaypoints( string name, array< ref JMCameraWaypoint > waypoints )
	{
		int idx = FindPath( name );
		JMCameraPath path;

		if ( idx == -1 )
		{
			path = new JMCameraPath( name );
			Paths.Insert( path );
		}
		else
		{
			path = Paths[idx];
			path.Waypoints.Clear();
		}

		foreach ( JMCameraWaypoint src : waypoints )
		{
			JMCameraWaypoint wp = new JMCameraWaypoint();
			wp.Position            = src.Position;
			wp.Time                = src.Time;
			wp.Speed               = src.Speed;
			wp.Smooth              = src.Smooth;
			wp.Orientation         = src.Orientation;
			wp.OrientationCaptured = src.OrientationCaptured;
			wp.Exposure            = src.Exposure;
			wp.Vignette            = src.Vignette;
			wp.Blur                = src.Blur;
			wp.FOV                 = src.FOV;
			wp.ShakeIntensity      = src.ShakeIntensity;
			wp.ShakeFrequency      = src.ShakeFrequency;
			wp.UseCatmull          = src.UseCatmull;
			wp.m_Easing              = src.m_Easing;
			wp.HoldTime            = src.HoldTime;
			wp.TrackTarget         = src.TrackTarget;
			path.Waypoints.Insert( wp );
		}

		Save();
	}

	void DeletePath( string name )
	{
		int idx = FindPath( name );
		if ( idx != -1 )
		{
			Paths.Remove( idx );
			Save();
		}
	}

	// ----------------------------------------------------------
	//  Bookmark helpers
	// ----------------------------------------------------------
	int FindBookmark( string name )
	{
		for ( int i = 0; i < Bookmarks.Count(); i++ )
		{
			if ( Bookmarks[i].Name == name )
				return i;
		}
		return -1;
	}

	void SaveBookmark( string name, vector position )
	{
		int idx = FindBookmark( name );
		if ( idx == -1 )
			Bookmarks.Insert( new JMCameraBookmark( name, position ) );
		else
			Bookmarks[idx].Position = position;

		Save();
	}

	void DeleteBookmark( string name )
	{
		int idx = FindBookmark( name );
		if ( idx != -1 )
		{
			Bookmarks.Remove( idx );
			Save();
		}
	}

	TStringArray GetPathNames()
	{
		TStringArray names = new TStringArray;
		foreach ( auto p : Paths )
			names.Insert( p.Name );
		return names;
	}

	TStringArray GetBookmarkNames()
	{
		TStringArray names = new TStringArray;
		foreach ( auto b : Bookmarks )
			names.Insert( b.Name );
		return names;
	}
}
