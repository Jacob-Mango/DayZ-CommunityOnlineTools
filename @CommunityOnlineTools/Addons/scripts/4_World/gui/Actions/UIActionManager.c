class UIActionManager
{
    static ref UIActionButton CreateButton( Widget parent, string button, Class instance, string funcname )
    {
        ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "COT/gui/layouts/actions/UIActionButton.layout", parent );

        ref UIActionButton action;
        widget.GetScript( action );

        if ( action )
        {
            action.SetCallback( instance, funcname );
            action.SetButton( button );

            return action;
        }

        return NULL;
    }

    static ref UIActionEditableText CreateEditableText( Widget parent, string label, string text, string button, Class instance, string funcname )
    {
        ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "COT/gui/layouts/actions/UIActionEditableText.layout", parent );

        ref UIActionEditableText action;
        widget.GetScript( action );

        if ( action )
        {
            action.SetCallback( instance, funcname );
            action.SetButton( button );
            action.SetLabel( label );
            action.SetText( text );

            return action;
        }

        return NULL;
    }

    static ref UIActionCheckbox CreateCheckbox( Widget parent, string label, bool checked, Class instance, string funcname )
    {
        ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "COT/gui/layouts/actions/UIActionCheckbox.layout", parent );

        ref UIActionCheckbox action;
        widget.GetScript( action );

        if ( action )
        {
            action.SetCallback( instance, funcname );
            action.SetLabel( label );
            action.SetChecked( checked );

            return action;
        }

        return NULL;
    }

    static ref UIActionText CreateText( Widget parent, string label, string text )
    {
        ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "COT/gui/layouts/actions/UIActionText.layout", parent );

        ref UIActionText action;
        widget.GetScript( action );

        if ( action )
        {
            action.SetLabel( label );
            action.SetText( text );

            return action;
        }

        return NULL;
    }
}