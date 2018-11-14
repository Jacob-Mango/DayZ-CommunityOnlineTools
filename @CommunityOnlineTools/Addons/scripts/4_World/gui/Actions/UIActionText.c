class UIActionText extends UIActionBase 
{
    protected ref TextWidget m_Label;
    protected ref TextWidget m_Text;

    protected Class m_Instance;
    protected string m_FuncName;

    override void OnInit() 
    {
        m_Label = TextWidget.Cast(layoutRoot.FindAnyWidget("action_label"));
        m_Text = TextWidget.Cast(layoutRoot.FindAnyWidget("action_text"));
    }

    override void OnShow()
    {
    }

    override void OnHide() 
    {
    }

    void SetLabel( string text )
    {
        m_Label.SetText( text );
    }

    void SetText( string text )
    {
        m_Text.SetText( text );
    }
}