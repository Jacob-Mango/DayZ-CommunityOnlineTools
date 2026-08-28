// =============================================================================
//  JMEntityMetaData
//
//  Generic descriptor for anything the EntityManager UI can list: a vehicle,
//  a territory, a crashed helicopter, a contaminated zone, an airdrop, etc.
//
//  Adapters fill this out from their source data. The UI only reads these
//  fields - it never touches engine objects directly.
//
//  String map `m_Extra` holds arbitrary adapter-specific fields (owner UID,
//  loot count, etc.) that are rendered in the info panel.
// =============================================================================
class JMEntityMetaData
{
	// Identity - NetworkID for live entities, synthetic int for pure data rows.
	int     m_NetworkIDLow;
	int     m_NetworkIDHigh;
	string  m_Id;

	// Categorization - adapter-specific enum, used for color/icon selection.
	int     m_Category;

	// Display
	string  m_DisplayName;
	string  m_ClassName;
	string  m_StatusText;
	string  m_StatusColor;

	// World placement
	vector  m_Position;
	vector  m_Orientation;

	// Marker appearance
	int     m_MarkerColor;
	string  m_MarkerIcon;

	// Freeform extra info rows - rendered as "key: value" lines in the info
	// panel. Order of insertion is preserved in m_ExtraKeys.
	ref array<string>  m_ExtraKeys;
	ref map<string, string>  m_Extra;

	void JMEntityMetaData()
	{
		m_ExtraKeys = new array<string>;
		m_Extra     = new map<string, string>;
	}

	void SetExtra( string key, string value )
	{
		if ( !m_Extra.Contains( key ) )
			m_ExtraKeys.Insert( key );
		m_Extra.Set( key, value );
	}

	string GetExtra( string key )
	{
		if ( m_Extra.Contains( key ) )
			return m_Extra.Get( key );
		return "";
	}

	void Write( ParamsWriteContext ctx )
	{
		ctx.Write( m_NetworkIDLow );
		ctx.Write( m_NetworkIDHigh );
		ctx.Write( m_Id );
		ctx.Write( m_Category );
		ctx.Write( m_DisplayName );
		ctx.Write( m_ClassName );
		ctx.Write( m_StatusText );
		ctx.Write( m_StatusColor );
		ctx.Write( m_Position );
		ctx.Write( m_Orientation );
		ctx.Write( m_MarkerColor );
		ctx.Write( m_MarkerIcon );

		int n = m_ExtraKeys.Count();
		ctx.Write( n );
		for ( int i = 0; i < n; i++ )
		{
			string k = m_ExtraKeys[i];
			ctx.Write( k );
			string v = m_Extra.Get( k );
			ctx.Write( v );
		}
	}

	bool Read( ParamsReadContext ctx )
	{
		if ( !ctx.Read( m_NetworkIDLow ) )   return false;
		if ( !ctx.Read( m_NetworkIDHigh ) )  return false;
		if ( !ctx.Read( m_Id ) )             return false;
		if ( !ctx.Read( m_Category ) )       return false;
		if ( !ctx.Read( m_DisplayName ) )    return false;
		if ( !ctx.Read( m_ClassName ) )      return false;
		if ( !ctx.Read( m_StatusText ) )     return false;
		if ( !ctx.Read( m_StatusColor ) )    return false;
		if ( !ctx.Read( m_Position ) )       return false;
		if ( !ctx.Read( m_Orientation ) )    return false;
		if ( !ctx.Read( m_MarkerColor ) )    return false;
		if ( !ctx.Read( m_MarkerIcon ) )     return false;

		int n;
		if ( !ctx.Read( n ) ) return false;
		for ( int i = 0; i < n; i++ )
		{
			string k;
			string v;
			if ( !ctx.Read( k ) ) return false;
			if ( !ctx.Read( v ) ) return false;
			SetExtra( k, v );
		}
		return true;
	}
}
