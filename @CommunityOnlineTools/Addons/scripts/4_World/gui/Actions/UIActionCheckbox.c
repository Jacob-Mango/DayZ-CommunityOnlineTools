class UIActionCheckbox extends UIActionBase 
{
    protected ref TextWidget m_Label;
    protected ref CheckBoxWidget m_Checkbox;
    
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

	override bool OnClick(Widget w, int x, int y, int button)
	{
        bool ret = false;

        if ( w == m_Checkbox )
        {
            ret = CallEvent( UIEvent.CLICK );
        }

        return ret;
    }
}