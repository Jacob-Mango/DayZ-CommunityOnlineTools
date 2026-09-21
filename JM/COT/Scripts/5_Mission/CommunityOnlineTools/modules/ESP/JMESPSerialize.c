//! Client-side overrides for the per-category ESP overlay colours.
//!
//! JMESPViewType.Colour is set in each subclass constructor, so an edit made in
//! the filter list would die on the next map load. This stores only the
//! categories that were actually changed, keyed by the view type's Permission
//! string rather than its typename - a typename is not stable across a mod
//! update, and Permission is already the identity ESP uses for its keys.
//!
//! Modelled on JMTeleportSerialize: static Load()/Create(), instance Save().
class JMESPSerialize : Managed
{
	ref map<string, int> Colours;
	static string m_FileName;

	protected void JMESPSerialize()
	{
		Colours = new map<string, int>;
		m_FileName = JMConstants.FILE_ESP;
	}

	void SetColour( string permission, int colour )
	{
		Colours.Set( permission, colour );
		Save();
	}

	static JMESPSerialize Load()
	{
		JMESPSerialize settings = new JMESPSerialize();

		JMJsonFile<JMESPSerialize>.Load( settings.m_FileName, settings );

		//! A file written by an older build can deserialize with a null map.
		if ( !settings.Colours )
			settings.Colours = new map<string, int>;

		return settings;
	}

	static JMESPSerialize Create()
	{
		return new JMESPSerialize();
	}

	void Save()
	{
		JMJsonFile<JMESPSerialize>.Save( m_FileName, this );
	}

	//! Push every stored override onto the live view types. A category with no
	//! entry keeps whatever its constructor set, so a reset is a delete.
	void Apply( notnull array< ref JMESPViewType > viewTypes )
	{
		foreach ( JMESPViewType viewType : viewTypes )
		{
			if ( Colours.Contains( viewType.Permission ) )
				viewType.Colour = Colours.Get( viewType.Permission );
		}
	}

	//! Deleting the entry is the reset: a category with no entry falls back to
	//! whatever its constructor set.
	void ClearColour( string permission )
	{
		if ( !Colours.Contains( permission ) )
			return;

		Colours.Remove( permission );
		Save();
	}

	void ClearColours()
	{
		Colours.Clear();
		Save();
	}
}
