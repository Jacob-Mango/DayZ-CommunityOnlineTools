class PlayerModule: EditorModule
{
    ref array< Man > m_Players;

    void PlayerModule()
    {
        m_Players = new ref array< Man >;

        GetRPCManager().AddRPC( "COS_Player", "ReloadList", this, SingeplayerExecutionType.Server );
    }

    override string GetLayoutRoot()
    {
        return "COS/gui/layouts/player/PlayerMenu.layout";
    }

    override void onUpdate( float timeslice )
    {
            
    }

    void ReloadList( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Print("PlayerModule::ReloadList");
        if ( !GetPermissionsManager().HasPermission( sender, "Player.ReloadList" ) )
            return;
        
        if ( type == CallType.Server )
        {
            m_Players.Clear();

            ref array<Man> players = new ref array<Man>;

            GetGame().GetPlayers( players );

            for ( int i = 0; i < players.Count(); i++ )
            {
                m_Players.Insert( players[i] );
            }

            Print( "Players: " + m_Players );

            GetRPCManager().SendRPC( "COS_Player", "ReloadList", new Param1< ref array< Man > >( m_Players ), true, sender );
        }

        if ( type == CallType.Client )
        {
            Param1< ref array< Man > > data;
            if ( !ctx.Read( data ) ) return;
            
            m_Players.Clear();
            m_Players.Copy( data.param1 );

            Print( "Players: " + m_Players );
        }
    }
}