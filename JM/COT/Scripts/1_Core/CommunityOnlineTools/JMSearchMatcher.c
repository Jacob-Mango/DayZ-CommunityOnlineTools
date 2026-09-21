//! One search box's needle, prepared once, matched against many rows.
//!
//! Wraps COT_String's keyword search (all words must match, or any of them when
//! the needle uses `|` / `OR`) so a list builder no longer carries the
//! needle / keywords / requireAll / closestMatch quartet around:
//!
//!     JMSearchMatcher matcher = new JMSearchMatcher( m_SearchBox.GetText() );
//!
//!     foreach ( Row row: rows )
//!     {
//!         if ( !matcher.Matches( row.Name ) )
//!             continue;
//!         ...
//!     }
//!
//!     m_SearchBox.SetTextPreview( matcher.GetClosestMatch() );
//!
//! Matching is case-insensitive; an empty needle matches everything.
class JMSearchMatcher
{
	protected string m_Search;
	protected ref TStringArray m_Keywords;
	protected bool m_RequireAll;
	protected string m_ClosestMatch;

	void JMSearchMatcher( string search )
	{
		//! KeywordSearch_Prepare lowercases the string it is called on, in place.
		COT_String prepared = search;
		m_Keywords = prepared.KeywordSearch_Prepare( m_RequireAll );
		m_Search = prepared;
	}

	//! Shortest matching string that starts with the needle, for the search
	//! box's autocomplete preview. Empty until something matched.
	string GetClosestMatch()
	{
		return m_ClosestMatch;
	}

	//! The prepared (lowercased) needle.
	string GetSearch()
	{
		return m_Search;
	}

	//! True when there is nothing to filter by.
	bool IsEmpty()
	{
		return m_Search == "";
	}

	//! Does `text` satisfy the search? Always true for an empty needle.
	bool Matches( string text )
	{
		if ( m_Search == "" )
			return true;

		COT_String haystack = text;
		haystack.ToLower();

		return haystack.KeywordSearchImplEx( m_Search, m_Keywords, m_RequireAll, m_ClosestMatch );
	}

	//! Does any of `texts` satisfy the search (a name and a GUID, say)?
	bool MatchesAny( array< string > texts )
	{
		if ( m_Search == "" )
			return true;

		foreach ( string text: texts )
		{
			if ( Matches( text ) )
				return true;
		}

		return false;
	}
}
