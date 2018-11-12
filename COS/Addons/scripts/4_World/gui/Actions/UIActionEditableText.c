class UIActionEditableText extends UIActionBase 
{
    protected ref TextWidget m_Label;
    protected ref EditBoxWidget m_Text;
    protected ref ButtonWidget m_Button;

    protected Class m_Instance;
    protected string m_FuncName;

    override void OnInit() 
    {
        m_Label = TextWidget.Cast(layoutRoot.FindAnyWidget("action_label"));
        m_Text = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("action_editable_text"));
        m_Button = ButtonWidget.Cast(layoutRoot.FindAnyWidget("action_button"));
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

    string GetText()
    {
        return m_Text.GetText();
    }

    void SetButton( string text )
    {
        m_Button.SetText( text );
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
            GetGame().GameScript.CallFunction( m_Instance, m_FuncName, NULL, new Param1< ref UIActionEditableText >( this ) );
        }

        return true;
    }
}