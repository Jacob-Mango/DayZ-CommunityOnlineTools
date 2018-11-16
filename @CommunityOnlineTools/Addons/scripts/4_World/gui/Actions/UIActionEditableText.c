class UIActionEditableText extends UIActionBase 
{
    protected ref TextWidget m_Label;
    protected ref EditBoxWidget m_Text;
    protected ref ButtonWidget m_Button;

    protected Class m_Instance;
    protected string m_FuncName;

    override void OnInit() 
    {
    }

    override void OnShow()
    {
    }

    override void OnHide() 
    {
    }

    void HasButton( bool enabled )
    {
        if ( enabled )
        {
            layoutRoot.FindAnyWidget( "action_button_no" ).Show( false );

            layoutRoot = layoutRoot.FindAnyWidget( "action_button_yes" );
            layoutRoot.Show( true );
        } else
        {
            layoutRoot.FindAnyWidget( "action_button_yes" ).Show( false );

            layoutRoot = layoutRoot.FindAnyWidget( "action_button_no" );
            layoutRoot.Show( true );
        }

        m_Label = TextWidget.Cast( layoutRoot.FindAnyWidget( "action_label" ) );
        m_Text = EditBoxWidget.Cast( layoutRoot.FindAnyWidget( "action_editable_text" ) );
        m_Button = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "action_button" ) );

        WidgetHandler.GetInstance().RegisterOnClick( m_Button, this, "OnClick" );
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
            GetGame().GameScript.CallFunction( m_Instance, m_FuncName, NULL, new Param1< ref UIActionEditableText >( this ) );
        }

        return true;
    }
}