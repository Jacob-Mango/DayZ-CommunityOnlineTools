class UIActionEditableVector extends UIActionBase 
{
    protected ref TextWidget m_Label;
    protected ref EditBoxWidget m_TextX;
    protected ref EditBoxWidget m_TextY;
    protected ref EditBoxWidget m_TextZ;
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
            layoutRoot.FindAnyWidget( "action_wrapper_input" ).Show( false );

            layoutRoot = layoutRoot.FindAnyWidget( "action_wrapper_check" );
            layoutRoot.Show( true );

            m_Button = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "action_button" ) );
        } else
        {
            layoutRoot.FindAnyWidget( "action_wrapper_check" ).Show( false );

            layoutRoot = layoutRoot.FindAnyWidget( "action_wrapper_input" );
            layoutRoot.Show( true );
        }

        m_Label = TextWidget.Cast( layoutRoot.FindAnyWidget( "action_label" ) );

        m_TextX = EditBoxWidget.Cast( layoutRoot.FindAnyWidget( "action_x" ) );
        m_TextY = EditBoxWidget.Cast( layoutRoot.FindAnyWidget( "action_y" ) );
        m_TextZ = EditBoxWidget.Cast( layoutRoot.FindAnyWidget( "action_z" ) );

        SetVector( vector.Zero );
    }

    void SetLabel( string text )
    {
        m_Label.SetText( text );
    }

    void SetVector( vector v )
    {
        m_TextX.SetText( v[0].ToString() );
        m_TextY.SetText( v[1].ToString() );
        m_TextZ.SetText( v[2].ToString() );
    }

    vector GetVector()
    {
        return Vector( m_TextX.GetText().ToFloat(), m_TextY.GetText().ToFloat(), m_TextZ.GetText().ToFloat() );
    }

    void SetButton( string text )
    {
        TextWidget.Cast( layoutRoot.FindAnyWidget( "action_button_text" ) ).SetText( text );
    }

    void RemoveDisableInput()
    {
        DISABLE_ALL_INPUT = false;
    }

    override bool OnChange( Widget w, int x, int y, bool finished )
    {
        if ( !m_HasCallback ) return false;

        if ( w == m_TextX || w == m_TextY || w == m_TextZ )
        {
            DISABLE_ALL_INPUT = true;
            CallEvent( UIEvent.CHANGE );
            GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( this.RemoveDisableInput, 100, false );
            return true;
        }
        
        return false;
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

        GetGame().GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, new Param2< UIEvent, ref UIActionEditableVector >( eid, this ) );

        return false;
    }
}