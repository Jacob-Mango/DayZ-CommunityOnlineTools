class JMStatics
{
	static Widget ESP_CONTAINER;
	static Widget COT_MENU;

	static Widget WINDOWS_CONTAINER;

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
};
