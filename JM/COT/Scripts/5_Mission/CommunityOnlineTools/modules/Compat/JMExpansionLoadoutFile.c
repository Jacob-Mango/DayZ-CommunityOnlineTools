#ifdef DZ_Expansion_Core
//! Reads a loadout saved in Expansion's prefab format from DIR_LOADOUTS. JMLoadoutModule
//! tries this first and falls back to COT's JMLoadout format when it answers false
//! (missing, unreadable or non-ExpansionPrefab file all mean "not an Expansion loadout").
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
