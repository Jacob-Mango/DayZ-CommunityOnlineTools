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

        UIActionManager.CreateButton( m_ActionsWrapper, "Action", this, "Action" );
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