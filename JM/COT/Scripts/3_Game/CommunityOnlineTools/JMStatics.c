class JMStatics
{
	static Widget ESP_CONTAINER;
	static Widget COT_MENU;

	static Widget WINDOWS_CONTAINER;

	//! Floating overlays: popups that live outside the window they belong to
	//! (the colour picker's ARGB panel, for one) so the window cannot clip them.
	//! COTModule decides "did this click land on COT UI or on the world" by
	//! walking a widget's ancestors looking for a window; an overlay parented
	//! next to the windows instead of inside one fails that test, the click is
	//! read as a world click, and the game grabs the mouse back mid-drag.
	//! Registering the overlay's root makes that walk recognise it.
	static ref array<Widget> OVERLAY_WIDGETS;

	static void RegisterOverlay( Widget w )
	{
		if ( !w )
			return;

		if ( !OVERLAY_WIDGETS )
			OVERLAY_WIDGETS = new array<Widget>;

		if ( OVERLAY_WIDGETS.Find( w ) == -1 )
			OVERLAY_WIDGETS.Insert( w );
	}

	static void UnregisterOverlay( Widget w )
	{
		if ( !w || !OVERLAY_WIDGETS )
			return;

		int idx = OVERLAY_WIDGETS.Find( w );
		if ( idx != -1 )
			OVERLAY_WIDGETS.RemoveOrdered( idx );
	}

	static bool IsOverlay( Widget w )
	{
		if ( !w || !OVERLAY_WIDGETS )
			return false;

		return OVERLAY_WIDGETS.Find( w ) != -1;
	}

	static void SortStringArray( out array< string > arr )
	{
		int count = arr.Count();
		string strs[ 1000 ];

		for ( int i = 0; i < count; i++ )
		{
			strs[ i ] = arr[ i ];
		}

		Sort( strs, count );

		arr.Clear();

		for ( i = 0; i < count; i++ )
		{
			arr.Insert( strs[i] );
		}
	}

	static void SortStringArrayKVPair( out array< string > arrKey, out array< string > arrValue )
	{
		if ( arrKey.Count() != arrValue.Count() )
			return;

		int count = arrKey.Count();

		string strs[ 1000 ];
		int pIndices[ 1000 ];

		for ( int i = 0; i < count; i++ )
		{
			strs[ i ] = arrKey[ i ];
		}

		Sort( strs, count );

		for ( i = 0; i < arrKey.Count(); i++ )
		{
			for ( int j = 0; j < arrKey.Count(); j++ )
			{
				if ( strs[j] == arrKey[i] )
				{
					pIndices[i] = j;
				}
			}
		}

		array< string > narrKey = new array< string >;
		array< string > narrValue = new array< string >;

		narrKey.Resize( count );
		narrValue.Resize( count );

		for ( i = 0; i < count; i++ )
		{
			narrKey.Set( pIndices[i], arrKey[i] );
			narrValue.Set( pIndices[i], arrValue[i] );
		}

		arrKey.Clear();
		arrValue.Clear();

		arrKey.Copy( narrKey );
		arrValue.Copy( narrValue );
	}

	//! @note joinked from ExpansionString
	static int StrCmp(string a, string b)
	{
		for (int i = 0; i < Math.Min(a.Length(), b.Length()); i++)
		{
			if (a[i] < b[i])
				return -1;
			else if (a[i] > b[i])
				return 1;
		}

		return a.Length() - b.Length();
	}

	//! @note joinked from ExpansionString
	static int StrCaseCmp(string a, string b)
	{
		a.ToLower();
		b.ToLower();

		return StrCmp(a, b);
	}

	//! @note joinked from ExpansionString
	static string CamelCaseToWords(string str, string sep = " ")
	{
		string output;
		string c;
		string cUpper;
		string cLower;
		bool wasLower;

		for (int j = 0; j < str.Length(); j++)
		{
			c = str[j];
			cUpper = c;
			cUpper.ToUpper();
			cLower = c;
			cLower.ToLower();
			if (wasLower && c == cUpper && c != cLower)
				output += sep;
			output += c;
			//! https://feedback.bistudio.com/T173348
			if (c != cUpper && c == cLower)
				wasLower = true;
			else
				wasLower = false;
		}

		return output;
	}

	/**
	 * @brief Set value to enum value if it exists
	 * 
	 * @param e typename/enum
	 * @param enumName name
	 * @param [out] value (if found)
	 * 
	 * @return true if found, false if not
	 * 
	 * @note use this instead of typename.StringToEnum if you need to know if value exists or not
	 * @note joinked from ExpansionStatic
	 */
	static bool StringToEnumEx(typename e, string enumName, out int value)
	{
		int count = e.GetVariableCount();
	   
		for (int i = 0; i < count; i++)
		{
			if (e.GetVariableType(i) == int && e.GetVariableName(i) == enumName && e.GetVariableValue(null, i, value))
				return true;
		}
		
		return false;
	}
}

static bool g_COT_ThreadESP;
static bool g_COT_ThreadESP_Running;
