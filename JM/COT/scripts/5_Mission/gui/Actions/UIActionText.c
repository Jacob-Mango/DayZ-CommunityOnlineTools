class UIActionText extends UIActionBase 
{
    protected string m_ActualText;

    protected ref TextWidget m_Label;
    protected ref TextWidget m_Text;

    override void OnInit() 
    {
        m_Label = TextWidget.Cast(layoutRoot.FindAnyWidget("action_label"));
        m_Text = TextWidget.Cast(layoutRoot.FindAnyWidget("action"));
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
        m_ActualText = text;
        m_Text.SetText( text );
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {    
        if ( !m_HasCallback )
        {
            if ( w == m_Text )
            {
                GetGame().CopyToClipboard( m_ActualText );
            }
            return false;
        }

        bool ret = false;

        if ( w == m_Text )
        {
            ret = CallEvent( UIEvent.CLICK );
        }

        return ret;
    }

    override bool CallEvent( UIEvent eid )
    {
        GetGame().GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, new Param2< UIEvent, ref UIActionText >( eid, this ) );

        return false;
    }
}