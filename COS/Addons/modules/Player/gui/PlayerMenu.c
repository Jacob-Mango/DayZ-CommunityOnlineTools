class PlayerMenu extends PopupMenu
{
    TextListboxWidget   m_playerScriptList;
    ButtonWidget        m_reloadScriptButton;

    void PlayerMenu()
    {
    }

    void ~PlayerMenu()
    {
    }

    override string GetTitle()
    {
        return "Players";
    }
    
    override string GetIconName()
    {
        return "P";
    }

    override bool ImageIsIcon()
    {
        return false;
    }

    override void Init()
    {
        m_playerScriptList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("game_list_box"));
        m_reloadScriptButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("reload_scripts_button"));
    }

    override void OnShow()
    {
    }

    override void OnHide() 
    {
    }

    override bool OnClick( Widget w, int x, int y, int button )
    {  
        if ( w == m_playerScriptList )
        {
            GetRPCManager().SendRPC( "COS_Player", "ReloadList", new Param, true );
        }

        return false;
    }
}