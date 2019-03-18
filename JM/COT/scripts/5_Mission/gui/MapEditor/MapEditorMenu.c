/*
class MapEditorMenu 
{
    protected Widget layoutRoot;

    protected EntityAI m_SelectedObject;
    protected float m_Distance = 200.0;

    protected ref MapEditorModule m_Module;

    protected bool m_MouseButtonPressed;
    protected int m_PreviousTime;

    void MapEditorMenu( ref MapEditorModule module )
    {
        m_Module = module;
        m_MouseButtonPressed = false;
    }
    
    void ~MapEditorMenu()
    {
        Hide();
    }

    bool IsVisible()
    {
        return layoutRoot.IsVisible();
    }
    
    Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets( "JM\\COT\\gui\\layouts\\Map\\MapMenu.layout" );
        layoutRoot.Show( false );

        ref Widget objectInfoWrapper = layoutRoot.FindAnyWidget( "object_info_wrapper" );

        ref Widget objectInfoGrid = UIActionManager.CreateGridSpacer( objectInfoWrapper, 1, 2 );

        UIActionManager.CreateEditableVector( objectInfoGrid, "Position: " );
        UIActionManager.CreateEditableVector( objectInfoGrid, "Rotation: " );

        ref Widget objectControlsWrapper = layoutRoot.FindAnyWidget( "object_controls" );

        ref Widget objectControlsGrid = UIActionManager.CreateGridSpacer( objectControlsWrapper, 1, 2 );
        UIActionManager.CreateText( objectControlsGrid, "Select: ", "Left Mouse" );
        UIActionManager.CreateText( objectControlsGrid, "Delete: ", "Delete" );

        return layoutRoot;
    }

    void Show()
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;

        layoutRoot.Show( true );

        m_PreviousTime = GetGame().GetTime();

        GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Insert( this.Update );

        OnShow();
    }

    void Hide()
    {
        if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;

        layoutRoot.Show( false );

        GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove( this.Update );

        OnHide();
    }

    void Update()
    {
        int currentTime = GetGame().GetTime();
        OnUpdate( ( m_PreviousTime - currentTime ) / 1000.0);
        m_PreviousTime = currentTime;
    }

    void OnShow()
    {
    }

    void OnHide()
    {
    }

    void CheckForSelection()
    {
    }

    void OnUpdate( float timeslice )
    {
        Input input = GetGame().GetInput();

        if ( GetWidgetUnderCursor().GetName() != "Windows" && GetWidgetUnderCursor().GetName() != "map_editor_menu" )
        {
            return;
        }

        if ( input.GetActionUp( UADefaultAction, false ) )
        {
            GetRPCManager().SendRPC( "COT_MapEditor", "SetPosition", new Param1<vector>( m_SelectedObject.GetPosition() ), false, NULL, m_SelectedObject );

            m_SelectedObject = NULL;

            CameraTool.SetTarget( NULL );
        }

        if ( input.GetActionDown( UADefaultAction, false ) )
        {
            m_SelectedObject = GetPointerObject( m_Distance );

            CameraTool.SetTarget( m_SelectedObject );
        }

        if ( m_SelectedObject )
        {
            vector position = m_SelectedObject.GetPosition();

            float forward = input.GetAction( UAMoveForward ) - input.GetAction( UAMoveBack );
            float strafe = input.GetAction( UATurnRight ) - input.GetAction( UATurnLeft );
            float altitude = input.GetAction( UALeanLeft ) - input.GetAction( UALeanRight );

            position[0] = position[0] + strafe;
            position[1] = position[1] + altitude;
            position[2] = position[2] + forward;

            m_SelectedObject.SetPosition( position );
        }

    }
}
*/