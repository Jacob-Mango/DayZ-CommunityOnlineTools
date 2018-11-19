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