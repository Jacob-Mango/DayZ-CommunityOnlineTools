class JMLockItemT<Class T>
{
	static T Get( Object target, int doorIndex = -1)
	{
		T lockItem;

		if ( Class.CastTo( lockItem, JMLockItem.Get( target, doorIndex ) ) )
			return lockItem;

		return null;
	}
}
