class PositionMenu extends Form
{
    protected TextListboxWidget m_LstPositionList;
    protected Widget m_ActionsWrapper;

    protected UIActionText m_PositionX;
    protected UIActionText m_PositionZ;
    protected UIActionButton m_Teleport;

    void PositionMenu()
    {
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

    override void OnInit( bool fromMenu )
    {
        m_LstPositionList = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("tls_ppp_pm_positions_list") );

        m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );

        ref Widget rows = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 2, 1 );

        ref Widget coords = UIActionManager.CreateGridSpacer( rows, 1, 2 );

        m_PositionX = UIActionManager.CreateText( coords, "X: " );
        m_PositionZ = UIActionManager.CreateText( coords, "Z: " );

        m_Teleport = UIActionManager.CreateButton( rows, "Teleport Selected Player(s)", this, "Click_Teleport" );
    }

    override void OnShow()
    {
        super.OnShow();
        vector player_pos = GetGame().GetPlayer().GetPosition();

        m_PositionX.SetText( player_pos[0].ToString() );
        m_PositionZ.SetText( player_pos[2].ToString() );
        
        TeleportModule tm = TeleportModule.Cast( module );

        if ( tm == NULL ) return;

        m_LstPositionList.ClearItems();

        for ( int nPosition = 0; nPosition < tm.GetPositions().Count(); nPosition++ )
        {
            m_LstPositionList.AddItem( tm.GetPositions().GetKey( nPosition ), NULL, 0 );
        }
    }

    override void OnHide()
    {
        super.OnHide();
    }

    void Click_Teleport( UIEvent eid, ref UIActionButton action )
    {
        GetRPCManager().SendRPC( "COT_Teleport", "Predefined", new Param2< string, ref array< string > >( GetCurrentPositionName(), SerializePlayersID( GetSelectedPlayers() ) ), true, NULL );
    }

    override bool OnItemSelected( Widget w, int x, int y, int row, int column, int oldRow, int oldColumn )
    {
        vector position = "0 0 0";

        TeleportModule tm = TeleportModule.Cast( module );

        if ( tm == NULL ) return false;

        if ( !tm.GetPositions().Find( GetCurrentPositionName(), position ) )
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