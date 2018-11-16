class PositionMenu extends Form
{
    protected TextListboxWidget m_LstPositionList;
    protected Widget m_ActionsWrapper;

    protected UIActionEditableText m_PositionX;
    protected UIActionEditableText m_PositionZ;
    protected UIActionButton m_Teleport;

    protected autoptr map< string, vector > m_oPositions = new map< string, vector >;

    void PositionMenu()
    {
        // TODO: Move to Teleport module and only reference position by array index for security. (Maybe read from file?)
        // Moved to post release...
        m_oPositions.Insert( "Severograd", "8428.0 0.0 12767.1" );
        m_oPositions.Insert( "Krasnostav", "11172.0 0.0 12314.1" );
        m_oPositions.Insert( "Mogilevka", "7583.0 0.0 5159.4" );
        m_oPositions.Insert( "Stary Sobor", "6072.0 0.0 7852.5" );
        m_oPositions.Insert( "Msta", "11207.0 0.0 5380.54" );
        m_oPositions.Insert( "Vybor", "3729.51 0.0 8935.56" );
        m_oPositions.Insert( "Gorka", "9787.1 0.0 8767.19" );
        m_oPositions.Insert( "Solnicni", "13393.1 0.0 6159.8" );
        m_oPositions.Insert( "Airport Vybor", "4828.9 0.0 10219.5" );
        m_oPositions.Insert( "Airport Balota", "4791.53 0.0 2557.47" );
        m_oPositions.Insert( "Airport Krasnostav", "12004.3 0.0 12655.3" );
        m_oPositions.Insert( "Chernogorsk Center", "6893.07 0.0 2618.13" );
        m_oPositions.Insert( "Chernogorsk West", "6546.03 0.0 2317.56" );
        m_oPositions.Insert( "Chernogorsk East", "7105.76 0.0 2699.53" );
        m_oPositions.Insert( "Elektrozavodsk Center", "10459.8 0.0 2322.72" );
        m_oPositions.Insert( "Elektrozavodsk East", "10414.5 0.0 2569.42" );
        m_oPositions.Insert( "Berezino Center", "12296.9 0.0 9470.51" );
        m_oPositions.Insert( "Berezino West", "12035.4 0.0 9143.49" );
        m_oPositions.Insert( "Berezino East", "12932.3 0.0 10172.7" );
        m_oPositions.Insert( "Svetlojarsk Center", "13835.3 0.0 13202.3" );
        m_oPositions.Insert( "Zelenogorsk Center", "2660.99 0.0 5299.28" );
        m_oPositions.Insert( "Zelenogorsk West", "2489.45 0.0 5080.41" );
    }

    void ~PositionMenu()
    {
    }

    override string GetTitle()
    {
        return "Teleport";
    }
    
    override string GetIconName()
    {
        return "T";
    }

    override bool ImageIsIcon()
    {
        return false;
    }

    void Init()
    {
        m_LstPositionList = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("tls_ppp_pm_positions_list") );

        m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );

        m_PositionX = UIActionManager.CreateEditableText( m_ActionsWrapper, "X: " );
        m_PositionZ = UIActionManager.CreateEditableText( m_ActionsWrapper, "Z: " );
        m_Teleport = UIActionManager.CreateButton( m_ActionsWrapper, "Teleport", this, "Click_Teleport" );

        for ( int nPosition = 0; nPosition < m_oPositions.Count(); nPosition++ )
        {
            m_LstPositionList.AddItem( m_oPositions.GetKey( nPosition ), NULL, 0 );
        }
    }

    override void OnShow()
    {
        super.OnShow();
        vector player_pos = GetGame().GetPlayer().GetPosition();

        m_PositionX.SetText( player_pos[0].ToString() );
        m_PositionZ.SetText( player_pos[2].ToString() );
    }

    override void OnHide()
    {
        super.OnHide();
    }

    void Click_Teleport()
    {
        vector vPlayerPos;
        vPlayerPos[0] = m_PositionX.GetText().ToFloat();
        vPlayerPos[2] = m_PositionZ.GetText().ToFloat();

        vPlayerPos = SnapToGround( vPlayerPos );
        GetRPCManager().SendRPC( "COT_Teleport", "Predefined", new Param2< vector, ref array< string > >( vPlayerPos, SerializePlayersGUID( GetSelectedPlayers() ) ), true, NULL );
    }

    override bool OnItemSelected( Widget w, int x, int y, int row, int column, int oldRow, int oldColumn )
    {
        vector position = "0 0 0";

        if( !m_oPositions.Find( GetCurrentPositionName(), position ) )
        {
            position = "0 0 0";
        }

        m_PositionX.SetText( position[0].ToString() );
        m_PositionZ.SetText( position[2].ToString() );

        return true;
    }

    string GetCurrentPositionName()
    {
        if ( m_LstPositionList.GetSelectedRow() != -1 )
        {
            string position_name;
            m_LstPositionList.GetItemText( m_LstPositionList.GetSelectedRow(), 0, position_name );
            return position_name;
        }

        return "";
    }
}