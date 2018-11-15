class UIActionCheckbox extends UIActionBase 
{
    protected ref TextWidget m_Label;
    protected ref CheckBoxWidget m_Checkbox;

    protected Class m_Instance;
    protected string m_FuncName;

    override void OnInit() 
    {
        m_Label = TextWidget.Cast(layoutRoot.FindAnyWidget("action_label"));
        m_Checkbox = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("action_checkbox"));

        WidgetHandler.GetInstance().RegisterOnClick( m_Checkbox, this, "OnClick" );
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

    void SetChecked( bool checked )
    {
        m_Checkbox.SetChecked( checked );
    }

    bool IsChecked()
    {
        return m_Checkbox.IsChecked();
    }

    void SetCallback( Class instance, string funcname )
    {
        m_Instance = instance;
        m_FuncName = funcname;
    }

	override bool OnClick(Widget w, int x, int y, int button)
	{    
        if ( w == m_Checkbox )
        {
            GetGame().GameScript.CallFunction( m_Instance, m_FuncName, NULL, new Param1< ref UIActionCheckbox >( this ) );
        }

        return true;
    }
}