class UIActionEditableText extends UIActionBase 
{
    protected ref TextWidget m_Label;
    protected ref EditBoxWidget m_Text;
    protected ref ButtonWidget m_Button;

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

            m_Button = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "action_button" ) );
        } else
        {
            layoutRoot.FindAnyWidget( "action_button_yes" ).Show( false );

            layoutRoot = layoutRoot.FindAnyWidget( "action_button_no" );
            layoutRoot.Show( true );
        }

        m_Label = TextWidget.Cast( layoutRoot.FindAnyWidget( "action_label" ) );
        m_Text = EditBoxWidget.Cast( layoutRoot.FindAnyWidget( "action_editable_text" ) );
    }

    void SetLabel( string text )
    {
        m_Label.SetText( text );
    }

    void SetText( string text )
    {
        if ( m_Text == GetFocus() ) return;

        m_Text.SetText( text );
    }

    string GetText()
    {
        Print( "Text is: " + m_Text.GetText() );
        return m_Text.GetText();
    }

    void SetButton( string text )
    {
        TextWidget.Cast( layoutRoot.FindAnyWidget("action_button_text") ).SetText( text );
    }

    override bool OnChange( Widget w, int x, int y, bool finished )
    {
        if ( !m_HasCallback ) return false;

        bool ret = false;

        if ( w == m_Text && finished )
        {
            ret = CallEvent( UIEvent.CHANGE );
        }

        return ret;
    }

	override bool OnClick(Widget w, int x, int y, int button)
	{    
        if ( !m_HasCallback ) return false;

        bool ret = false;

        if ( w == m_Button )
        {
            ret = CallEvent( UIEvent.CLICK );
        }

        return ret;
    }

    override bool CallEvent( UIEvent eid )
    {
        if ( !m_HasCallback ) return false;

        GetGame().GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, new Param2< UIEvent, ref UIActionEditableText >( eid, this ) );

        return false;
    }
}