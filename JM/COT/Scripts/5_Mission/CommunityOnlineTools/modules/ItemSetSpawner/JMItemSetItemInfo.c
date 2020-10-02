class JMItemSetItemInfo
{
	string ItemName;
	int NumberOfStacks;
	int StackSize;

	void JMItemSetItemInfo( string item, int numStacks, int stackSize )
	{
		ItemName = item;
		NumberOfStacks = numStacks;
		StackSize = stackSize;
	}
}