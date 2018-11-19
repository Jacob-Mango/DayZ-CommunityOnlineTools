class MapEditorMenu 
{
    protected Widget layoutRoot;

    void MapEditorMenu()
    {
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
        layoutRoot = GetGame().GetWorkspace().CreateWidgets( "COT\\gui\\layouts\\Map\\MapMenu.layout" );
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

        OnShow();
    }

    void Hide()
    {
        layoutRoot.Show( false );

        OnHide();
    }

    void OnShow()
    {
        
    }

    void OnHide()
    {
        
    }
}