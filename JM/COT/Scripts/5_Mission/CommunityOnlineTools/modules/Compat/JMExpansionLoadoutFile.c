#ifdef DZ_Expansion_Core
//! Reads a loadout saved in Expansion's prefab format from DIR_LOADOUTS.
//!
//! The spawn paths of JMLoadoutModule try this first and fall back to COT's own JMLoadout
//! format when it answers false: a missing file, an unreadable one and one that is not an
//! ExpansionPrefab are all "not an Expansion loadout" to the caller.
class JMExpansionLoadoutFile
{
	static string GetPath( string loadoutName )
	{
		return JMConstants.DIR_LOADOUTS + loadoutName + ".json";
	}

	static bool Read( string loadoutName, out ExpansionPrefab prefab )
	{
		string filepath = GetPath( loadoutName );
		if ( !FileExist( filepath ) )
			return false;

		FileHandle file = OpenFile( filepath, FileMode.READ );
		if ( !file )
			return false;

		string content;
		string line;
		while ( FGets( file, line ) > 0 )
			content += line + "\n";

		CloseFile( file );

		ExpansionPrefab parsed = new ExpansionPrefab();
		string errorMsg;
		if ( !JsonFileLoader<ExpansionPrefab>.MakeData( parsed, content, errorMsg ) )
			return false;

		prefab = parsed;
		return true;
	}
}
#endif
