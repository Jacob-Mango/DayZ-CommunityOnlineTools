class GameBaseBuildingSpawn
{
    string Item;
    int NumberOfStacks;
    int StackSize;

    void GameBaseBuildingSpawn( string item, int numStacks, int stackSize )
    {
        Item = item;
        NumberOfStacks = numStacks;
        StackSize = stackSize;
    }
}