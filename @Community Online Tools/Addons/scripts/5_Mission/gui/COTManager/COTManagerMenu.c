class COTManagerMenu extends Form
{
    protected Widget m_ActionsWrapper;

    void COTManagerMenu()
    {
    }

    void ~COTManagerMenu()
    {
    }

    override string GetTitle()
    {
        return "COT Manager";
    }
    
    override string GetIconName()
    {
        return "CM";
    }

    override bool ImageIsIcon()
    {
        return false;
    }

    override void OnInit( bool fromMenu )
    {
        m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );

        UIActionManager.CreateCheckbox( m_ActionsWrapper, "ESP", this, "Click_ESP" );
    }

    void Click_ESP( UIEvent eid, ref UIActionCheckbox action )
    {
        if ( eid != UIEvent.CLICK ) return;
        
        COT_ESP_Toggled = !COT_ESP_Toggled;
    }

    override void OnShow()
    {
        super.OnShow();
    }

    override void OnHide()
    {
        super.OnHide();
    }
}