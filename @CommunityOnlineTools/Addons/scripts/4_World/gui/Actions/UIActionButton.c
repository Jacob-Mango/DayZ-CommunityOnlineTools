class UIActionButton extends UIActionBase 
{
    protected ref ButtonWidget m_Button;

    protected Class m_Instance;
    protected string m_FuncName;

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

    void SetCallback( Class instance, string funcname )
    {
        m_Instance = instance;
        m_FuncName = funcname;
    }

	override bool OnClick(Widget w, int x, int y, int button)
	{    
        if ( w == m_Button )
        {
            GetGame().GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, new Param );
        }

        return true;
    }
}