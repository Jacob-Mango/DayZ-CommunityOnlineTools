class UIActionManager
{
	static GridSpacerWidget CreateGridSpacer( notnull Widget parent, int rows, int columns )
	{
		//! Assemble path outside of call to CreateWidgets to work-around https://feedback.bistudio.com/T183345
		string layout = string.Format("JM/COT/GUI/layouts/uiactions/Wrappers/%1/GridSpacer%2.layout", rows, columns);
		Widget widget = GetGame().GetWorkspace().CreateWidgets( layout, parent );

		if (!widget)
			Error("No widgets created " + layout);

		GridSpacerWidget spacer;
		if (Class.CastTo(spacer, widget))
		{
			return spacer;
		}

		Error(string.Format("Could not cast %1 (name=\"%2\" type=%3 layout=\"%4\") to GridSpacerWidget", widget, widget.GetName(), widget.GetTypeName(), layout));

		return NULL;
	}

	static WrapSpacerWidget CreateWrapSpacer( notnull Widget parent )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIWrapSpacer.layout", parent );

		if (!widget)
			Error("No widgets created");

		WrapSpacerWidget spacer;
		if (Class.CastTo(spacer, widget))
		{
			return spacer;
		}

		Error(string.Format("Could not cast %1 (name=\"%2\" type=%3) to WrapSpacerWidget", widget, widget.GetName(), widget.GetTypeName()));

		return NULL;
	}

	static Widget CreateActionRows( notnull Widget parent )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionContentRows.layout", parent );

		if (!widget)
			Error("No widgets created");

		return widget;
	}
	
	static Widget CreateSpacer( notnull Widget parent )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UISpacer.layout", parent );

		if (!widget)
			Error("No widgets created");

		return widget;
	}

	static Widget CreatePanel( notnull Widget parent, int color = 0x00000000, float height = -1 )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIPanel.layout", parent );

		widget.SetColor( color );

		if ( height != -1 )
		{
			SetSize(widget, -1, height);
		}

		return widget;
	}

	static UIActionButton CreateButton( notnull Widget parent, string button, Class instance, string funcname, float width = 1 )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionButton.layout", parent );

		UIActionButton action;
		widget.GetScript( action );

		if ( width != 1 )
		{
			SetSize(widget, width, -1);
		}

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetButton( button );

			return action;
		}

		Error(string.Format("Couldn't get script for widget %1 (name=\"%2\" type=%3)", widget, widget.GetName(), widget.GetTypeName()));

		return NULL;
	}

	static UIActionNavigateButton CreateNavButton( notnull Widget parent, string button, string imageR, string imageL, Class instance, string funcname, float width = 1 )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionNavigateButton.layout", parent );

		UIActionNavigateButton action;
		widget.GetScript( action );

		if ( width != 1 )
		{
			SetSize(widget, width, -1);
		}

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetButton( button );
			action.SetImages( imageR, imageL );

			return action;
		}

		Error(string.Format("Couldn't get script for widget %1 (name=\"%2\" type=%3)", widget, widget.GetName(), widget.GetTypeName()));

		return NULL;
	}

	static UIActionEditableTextPreview CreateEditableTextPreview( notnull Widget parent, string label, Class instance = NULL, string funcname = "", string text = "", string button = "" )
	{
		bool hasButton;
		if (button != "" && instance != NULL && funcname != "")
			hasButton = true;

		string layoutName;
		if (hasButton)
			layoutName = "UIActionEditableTextPreviewButton";
		else
			layoutName = "UIActionEditableTextPreview";

		//! Assemble path outside of call to CreateWidgets to work-around https://feedback.bistudio.com/T183345
		string layout = string.Format("JM/COT/GUI/layouts/uiactions/%1.layout", layoutName);
		Widget widget = GetGame().GetWorkspace().CreateWidgets( layout, parent );

		UIActionEditableTextPreview action;
		widget.GetScript( action );

		if ( action )
		{
			if ( hasButton )
				action.SetButton( button );

			action.SetCallback( instance, funcname );

			action.SetLabel( label );
			action.SetText( text );

			return action;
		}

		Error(string.Format("Couldn't get script for widget %1 (name=\"%2\" type=%3)", widget, widget.GetName(), widget.GetTypeName()));

		return NULL;
	}

	static UIActionEditableText CreateEditableText( notnull Widget parent, string label, Class instance = NULL, string funcname = "", string text = "", string button = "" )
	{
		bool hasButton;
		if (button != "" && instance != NULL && funcname != "")
			hasButton = true;

		string layoutName;
		if (hasButton)
			layoutName = "UIActionEditableTextButton";
		else
			layoutName = "UIActionEditableText";

		//! Assemble path outside of call to CreateWidgets to work-around https://feedback.bistudio.com/T183345
		string layout = string.Format("JM/COT/GUI/layouts/uiactions/%1.layout", layoutName);
		Widget widget = GetGame().GetWorkspace().CreateWidgets( layout, parent );

		UIActionEditableText action;
		widget.GetScript( action );

		if ( action )
		{
			if ( hasButton )
				action.SetButton( button );

			action.SetCallback( instance, funcname );

			action.SetLabel( label );
			action.SetText( text );

			return action;
		}

		Error(string.Format("Couldn't get script for widget %1 (name=\"%2\" type=%3)", widget, widget.GetName(), widget.GetTypeName()));

		return NULL;
	}

	static UIActionDropdownList CreateDropdownBox( notnull Widget parent, Widget dropDownParent, string label, array< string > values, Class instance = NULL, string funcname = "" )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionDropdownList.layout", parent );

		UIActionDropdownList action;
		widget.GetScript( action );

		if ( action )
		{
			if ( !dropDownParent )
				dropDownParent = parent;
				
			action.InitListWidget( dropDownParent );

			action.SetItems( values );

			action.SetCallback( instance, funcname );

			action.SetLabel( label );

			return action;
		}

		Error(string.Format("Couldn't get script for widget %1 (name=\"%2\" type=%3)", widget, widget.GetName(), widget.GetTypeName()));

		return NULL;
	}

	static UIActionEditableVector CreateEditableVector( notnull Widget parent, string label, Class instance = NULL, string funcname = "", string button = "" )
	{
		bool hasButton;
		if (button != "" && instance != NULL && funcname != "")
			hasButton = true;

		string layoutName;
		if (hasButton)
			layoutName = "UIActionEditableVectorButton";
		else
			layoutName = "UIActionEditableVector";

		//! Assemble path outside of call to CreateWidgets to work-around https://feedback.bistudio.com/T183345
		string layout = string.Format("JM/COT/GUI/layouts/uiactions/%1.layout", layoutName);
		Widget widget = GetGame().GetWorkspace().CreateWidgets( layout, parent );

		UIActionEditableVector action;
		widget.GetScript( action );

		if ( action )
		{
			if ( hasButton )
				action.SetButton( button );

			action.SetCallback( instance, funcname );

			action.SetLabel( label );

			return action;
		}

		Error(string.Format("Couldn't get script for widget %1 (name=\"%2\" type=%3)", widget, widget.GetName(), widget.GetTypeName()));

		return NULL;
	}

	static UIActionCheckbox CreateCheckbox( notnull Widget parent, string label, Class instance = NULL, string funcname = "", bool checked = false, float width = 1 )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionCheckbox.layout", parent );

		UIActionCheckbox action;
		widget.GetScript( action );

		if ( width != 1 )
		{
			SetSize(widget, width, -1);
		}

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			action.SetChecked( checked );

			return action;
		}

		Error(string.Format("Couldn't get script for widget %1 (name=\"%2\" type=%3)", widget, widget.GetName(), widget.GetTypeName()));

		return NULL;
	}

	static UIActionText CreateText( notnull Widget parent, string label, string text = "", Class instance = NULL, string funcname = "", UIActionHAlign lha = UIActionHAlign.LEFT, UIActionHAlign lva = UIActionHAlign.CENTER, UIActionHAlign tha = UIActionHAlign.RIGHT, UIActionHAlign tva = UIActionHAlign.CENTER )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionText.layout", parent );

		UIActionText action;
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

		Error(string.Format("Couldn't get script for widget %1 (name=\"%2\" type=%3)", widget, widget.GetName(), widget.GetTypeName()));

		return NULL;
	}

	static UIActionSelectBox CreateSelectionBox( notnull Widget parent, string label, notnull array< string > options, Class instance = NULL, string funcname = "" )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionSelectBox.layout", parent );

		UIActionSelectBox action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			action.SetSelections( options );

			return action;
		}

		Error(string.Format("Couldn't get script for widget %1 (name=\"%2\" type=%3)", widget, widget.GetName(), widget.GetTypeName()));

		return NULL;
	}

	static UIActionSlider CreateSlider( notnull Widget parent, string label, float min, float max, Class instance = NULL, string funcname = "" )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionSlider.layout", parent );

		UIActionSlider action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			action.SetMin( min );
			action.SetMax( max );
			action.SetStepValue( 0 );
			action.SetFormat( "#STR_COT_FORMAT_NONE" );

			return action;
		}

		Error(string.Format("Couldn't get script for widget %1 (name=\"%2\" type=%3)", widget, widget.GetName(), widget.GetTypeName()));

		return NULL;
	}

	static UIActionScroller CreateScroller( notnull Widget parent )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionScroller.layout", parent );

		UIActionScroller action;
		widget.GetScript( action );

		if ( action )
		{
			return action;
		}

		Error(string.Format("Couldn't get script for widget %1 (name=\"%2\" type=%3)", widget, widget.GetName(), widget.GetTypeName()));

		return NULL;
	}

	static UIActionImage CreateImage( notnull Widget parent, string image, Class instance = NULL, string funcname = "", UIActionHAlign lha = UIActionHAlign.LEFT, UIActionHAlign lva = UIActionHAlign.CENTER, UIActionHAlign tha = UIActionHAlign.RIGHT, UIActionHAlign tva = UIActionHAlign.CENTER )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionImage.layout", parent );

		UIActionImage action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );

			action.SetImage( image );

			action.SetLabelHAlign( lha );
			action.SetLabelVAlign( lva );

			return action;
		}

		Error(string.Format("Couldn't get script for widget %1 (name=\"%2\" type=%3)", widget, widget.GetName(), widget.GetTypeName()));

		return NULL;
	}

	static void SetSize( Widget widget, float width = -1, float height = -1 )
	{
		float w;
		float h;

		widget.SetFlags( WidgetFlags.VEXACTSIZE );
		widget.GetSize( w, h );

		if (width != -1)
			w = width;

		if (height != -1)
			h = height;

		int resX, resY;

		GetScreenSize(resX, resY);

		float scaleY;

		if (resY > 1080)
			scaleY = resY / 1080;
		else
			scaleY= 1.0;

		widget.SetSize( w, h * scaleY );
	}
};
