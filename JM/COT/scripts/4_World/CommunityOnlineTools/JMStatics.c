class JMStatics
{
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
};