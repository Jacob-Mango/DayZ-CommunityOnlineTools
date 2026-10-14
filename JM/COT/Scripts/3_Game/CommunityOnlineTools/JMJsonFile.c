//! The one way COT reads and writes a settings / data file as JSON. Load() and Save() always
//! make sure the folder exists (else a first save silently writes nothing), call the loader, and
//! say why on failure. Load() fills the object it is handed: JsonFileLoader does NOT run the
//! constructor for a field the file omits, so constructor defaults must already be set.
class JMJsonFile<Class T>
{
	//! True when the file existed and was read into `data`. False for a missing file (not an error:
	//! the caller applies its defaults) and for one that did not parse (which is logged).
	static bool Load( string path, T data )
	{
		if ( !FileExist( path ) )
			return false;

		string error;
		if ( !JsonFileLoader<T>.LoadFile( path, data, error ) )
		{
			CF_Log.Error( "[COT] could not read %1: %2", path, error );
			return false;
		}

		return true;
	}

	//! Creates every missing folder above `path`, then writes it. False (and logged) on failure.
	static bool Save( string path, T data )
	{
		JMFileUtils.EnsureDirectoryFor( path );

		string error;
		if ( !JsonFileLoader<T>.SaveFile( path, data, error ) )
		{
			CF_Log.Error( "[COT] could not write %1: %2", path, error );
			return false;
		}

		return true;
	}
}
