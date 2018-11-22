modded class PlayerBase
{
    bool CanBeDeleted()
    {
        return IsAlive() && !IsRestrained() && !IsUnconscious();
    }
}