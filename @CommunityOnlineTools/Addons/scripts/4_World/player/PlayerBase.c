modded class PlayerBase
{
    ref AuthPlayer m_AuthPlayer;

    void SetAuthPlayer( ref AuthPlayer player )
    {
        m_AuthPlayer = player;
    }

    ref AuthPlayer GetAuthPlayer()
    {
        return m_AuthPlayer;
    }

    bool CanBeDeleted()
    {
        return IsAlive() && !IsRestrained() && !IsUnconscious();
    }

    override void OnSelectPlayer()
    {
        super.OnSelectPlayer();

        if ( m_AuthPlayer )
        {
            m_AuthPlayer.UpdatePlayerData( this );
        }
    }
}