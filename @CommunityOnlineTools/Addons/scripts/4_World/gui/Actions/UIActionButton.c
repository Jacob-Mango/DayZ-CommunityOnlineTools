class UIActionButton extends UIActionBase 
{
    protected ref TextWidget m_Text;
    protected ref ButtonWidget m_Button;

    protected string m_String;

    override void OnInit() 
    {
        m_Button = ButtonWidget.Cast( layoutRoot.FindAnyWidget("action_button") );
        m_Text = TextWidget.Cast( m_Button.FindAnyWidget("action_button_text") );
    }

    override void OnShow()
    {
    }

    override void OnHide() 
    {
    }

    void SetButton( string text )
    {
        m_String = text;
        m_Text.SetText( text );
    }

    string GetButton()
    {
        return m_String;
    }

	override bool OnClick(Widget w, int x, int y, int button)
	{
        bool ret = false;

        if ( w == m_Button )
        {
            ret = CallEvent( UIEvent.CLICK );
        }

        return ret;
    }
}