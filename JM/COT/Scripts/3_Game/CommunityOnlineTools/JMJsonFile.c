//! The one way COT reads and writes a settings / data file as JSON.
//!
//! Modules used to each carry their own FileExist + MakeDirectory + JsonFileLoader trio, and they
//! disagreed: some created the directory before saving and some did not (so the first save of a
//! fresh profile silently wrote nothing), some reported a parse error and some swallowed it.
//! Load() and Save() here do the same three things every time: make sure the folder exists,
//! call the loader, and say why on failure.
//!
//!   ref MySettings settings = new MySettings();
//!   if ( !JMJsonFile<MySettings>.Load( path, settings ) )
//!       settings.Defaults();
//!
//!   JMJsonFile<MySettings>.Save( path, settings );
//!
//! Load() fills the object it is handed rather than returning a new one, so a class keeps its
//! own constructor defaults: JsonFileLoader does NOT run the constructor for a field the file
//! does not mention, and this way the caller has already built the object the way it wants.
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
