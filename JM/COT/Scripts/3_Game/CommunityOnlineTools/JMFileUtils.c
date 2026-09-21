//! File-system helpers the engine does not provide.
class JMFileUtils
{
	//! MakeDirectory is not recursive: it fails when the parent is missing. This creates each
	//! missing folder above `filePath` in turn, so a save into $profile:COT\Some\Nested\Dir\
	//! works on a profile where none of it exists yet. `filePath` is a file, not a folder: its
	//! last segment is left alone.
	static void EnsureDirectoryFor( string filePath )
	{
		string normalized = filePath;
		normalized.Replace( "/", "\\" );

		TStringArray parts = new TStringArray;
		normalized.Split( "\\", parts );

		string current = "";
		for ( int i = 0; i < parts.Count() - 1; i++ )
		{
			if ( parts[i] == "" )
				continue;

			if ( current != "" )
				current += "\\";

			current += parts[i];

			if ( !FileExist( current ) )
				MakeDirectory( current );
		}
	}
}
