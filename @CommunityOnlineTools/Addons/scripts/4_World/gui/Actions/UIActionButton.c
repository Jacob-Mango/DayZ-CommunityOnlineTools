class UIActionButton extends UIActionBase 
{
    protected ref ButtonWidget m_Button;

    override void OnInit() 
    {
        m_Button = ButtonWidget.Cast( layoutRoot.FindAnyWidget("action_button") );

        WidgetHandler.GetInstance().RegisterOnClick( m_Button, this, "OnClick" );
    }

    override void OnShow()
    {
    }

    override void OnHide() 
    {
    }

    void SetButton( string text )
    {
        TextWidget.Cast( layoutRoot.FindAnyWidget("action_button_text") ).SetText( text );
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