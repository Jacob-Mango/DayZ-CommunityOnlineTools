class JMPlayerSerialize : Managed
{
	[NonSerialized()]
	string m_FileName;
	ref array< string > Roles;

	// Optional per-role name restriction: role name -> required exact in-game name.
	// If an entry exists for a role, the role is only applied when the player's
	// current name matches exactly (case-sensitive).
	ref map< string, string > NameRestrictions;

	//! Session history - playtime, deaths, lifespan. Absent from files written
	//! before this existed; JsonFileLoader leaves a missing member at its
	//! default, so the constructor value below is what an old file loads with
	//! and no migration step is needed.
	ref JMPlayerStats Stats;

	void JMPlayerSerialize()
	{
		Roles = new array< string >;
		NameRestrictions = new map< string, string >;
		Stats = new JMPlayerStats();
	}

	static string FileReadyStripName( string name )
	{
		name.Replace( "\\", "" );
		name.Replace( "/", "" );
		name.Replace( "=", "" );
		name.Replace( "+", "" );

		return name;
	}

	static bool Load( JMPlayerInstance inst, inout JMPlayerSerialize playerFile )
	{
		auto trace = CF_Trace_1("JMPlayerSerialize").Add(inst.GetGUID());

		// steamid files are made by the user, not generated: prioritize them over ours.
		// Kept under their own name, not migrated to a GUID one: GetGUID() is the BattlEye
		// GUID, which a diag/offline client gets anew every launch, losing its role pin.
		// A Steam64 is stable for the life of the account.
		playerFile.m_FileName = JMConstants.DIR_PLAYERS + inst.GetSteam64ID() + JMConstants.EXT_PLAYER;
		if ( FileExist( playerFile.m_FileName ) )
		{
			JsonFileLoader<JMPlayerSerialize>.JsonLoadFile( playerFile.m_FileName, playerFile );
			return true;
		}

		playerFile.m_FileName = JMConstants.DIR_PLAYERS + FileReadyStripName( inst.GetGUID() ) + JMConstants.EXT_PLAYER;
		if ( FileExist( playerFile.m_FileName ) )
		{
			JsonFileLoader<JMPlayerSerialize>.JsonLoadFile( playerFile.m_FileName, playerFile );
			// File already exists in the correct location - no re-save needed.
			return true;
		}

		// "<steam64>.json.txt" - the same user-made file, written by an editor on a
		// Windows box with file extensions hidden. Strip the stray extension, but
		// keep it keyed on the STEAM64 for the reason above; migrating it onto the
		// GUID would throw away the only stable key the file has.
		playerFile.m_FileName = JMConstants.DIR_PLAYERS + inst.GetSteam64ID() + JMConstants.EXT_PLAYER + JMConstants.EXT_WINDOWS_DEFAULT;
		if ( FileExist( playerFile.m_FileName ) )
		{
			JsonFileLoader<JMPlayerSerialize>.JsonLoadFile( playerFile.m_FileName, playerFile );

			DeleteFile( playerFile.m_FileName );
			playerFile.m_FileName = JMConstants.DIR_PLAYERS + inst.GetSteam64ID() + JMConstants.EXT_PLAYER;

			playerFile.Save();
			return true;
		}

		playerFile.m_FileName = JMConstants.DIR_PLAYERS + FileReadyStripName( inst.GetGUID() ) + JMConstants.EXT_PLAYER + JMConstants.EXT_WINDOWS_DEFAULT;
		if ( FileExist( playerFile.m_FileName ) )
		{
			JsonFileLoader<JMPlayerSerialize>.JsonLoadFile( playerFile.m_FileName, playerFile );

			DeleteFile( playerFile.m_FileName );
			playerFile.m_FileName = JMConstants.DIR_PLAYERS + FileReadyStripName( inst.GetGUID() ) + JMConstants.EXT_PLAYER;

			// Migrate: write under the new GUID-based filename unconditionally.
			playerFile.Save();
			return true;
		}

		// No file found - new player with default state.  Set up the filename for future saves but do NOT write anything.
		playerFile.m_FileName = JMConstants.DIR_PLAYERS + FileReadyStripName( inst.GetGUID() ) + JMConstants.EXT_PLAYER;
		playerFile.Roles.Insert( "everyone" );
		return false;
	}

	void Save()
	{
		JsonFileLoader<JMPlayerSerialize>.JsonSaveFile( m_FileName, this );
	}
}
