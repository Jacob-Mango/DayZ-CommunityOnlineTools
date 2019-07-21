class UIActionManager
{
	static ref GridSpacerWidget CreateGridSpacer( Widget parent, int rows, int columns )
	{
		ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/Wrappers/" + rows.ToString() + "/GridSpacer" + columns.ToString() + ".layout", parent );

		if ( widget )
		{
			return GridSpacerWidget.Cast( widget );
		}

		return NULL;
	}
	
	static ref Widget CreateSpacer( Widget parent )
	{
		ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UISpacer.layout", parent );

		return widget;
	}

	static ref UIActionButton CreateButton( Widget parent, string button, Class instance, string funcname )
	{
		ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionButton.layout", parent );

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

	static ref UIActionEditableText CreateEditableText( Widget parent, string label, Class instance = NULL, string funcname = "", string text = "", string button = "" )
	{
		ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionEditableText.layout", parent );

		ref UIActionEditableText action;
		widget.GetScript( action );

		if ( action )
		{
			if ( button == "" || instance == NULL || funcname == "" )
			{
				action.HasButton( false );
			} else 
			{
				action.HasButton( true );

				action.SetButton( button );
			}

			action.SetCallback( instance, funcname );

			action.SetLabel( label );
			action.SetText( text );

			return action;
		}

		return NULL;
	}

	static ref UIActionEditableVector CreateEditableVector( Widget parent, string label, Class instance = NULL, string funcname = "", string button = "" )
	{
		ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionEditableVector.layout", parent );

		ref UIActionEditableVector action;
		widget.GetScript( action );

		if ( action )
		{
			if ( button == "" || instance == NULL || funcname == "" )
			{
				action.HasButton( false );
			} else 
			{
				action.HasButton( true );

				action.SetButton( button );
			}

			action.SetCallback( instance, funcname );

			action.SetLabel( label );

			return action;
		}

		return NULL;
	}

	static ref UIActionCheckbox CreateCheckbox( Widget parent, string label, Class instance = NULL, string funcname = "", bool checked = false )
	{
		ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionCheckbox.layout", parent );

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

	static ref UIActionText CreateText( Widget parent, string label, string text = "", Class instance = NULL, string funcname = "", UIActionHAlign lha = UIActionHAlign.LEFT, UIActionHAlign lva = UIActionHAlign.CENTER, UIActionHAlign tha = UIActionHAlign.RIGHT, UIActionHAlign tva = UIActionHAlign.CENTER )
	{
		ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionText.layout", parent );

		ref UIActionText action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			action.SetText( text );

			action.SetLabelHAlign( lha );
            action.SetLabelVAlign( lva );

            action.SetTextHAlign( tha );
            action.SetTextVAlign( tva );

			return action;
		}

		return NULL;
	}

	static ref UIActionSelectBox CreateSelectionBox( Widget parent, string label, notnull ref array<string> options, Class instance = NULL, string funcname = "" )
	{
		ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionSelectBox.layout", parent );

		ref UIActionSelectBox action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			action.SetSelections( options );

			return action;
		}

		return NULL;
	}

	static ref UIActionSlider CreateSlider( Widget parent, string label, float min, float max, Class instance = NULL, string funcname = "" )
	{
		ref Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionSlider.layout", parent );

		ref UIActionSlider action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			action.SetMin( min );
			action.SetMax( max );
			action.SetStepValue( 0 );

			return action;
		}

		return NULL;
	}
}