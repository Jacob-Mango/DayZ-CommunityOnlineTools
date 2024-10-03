class UIActionManager
{
	static GridSpacerWidget CreateGridSpacer( notnull Widget parent, int rows, int columns )
	{
		//! Assemble path outside of call to CreateWidgets to work-around https://feedback.bistudio.com/T183345
		string layout = string.Format("JM/COT/GUI/layouts/uiactions/Wrappers/%1/GridSpacer%2.layout", rows, columns);
		Widget widget = GetGame().GetWorkspace().CreateWidgets( layout, parent );

		CheckWidget(widget, parent, layout);

		GridSpacerWidget spacer;
		if (Class.CastTo(spacer, widget))
		{
			return spacer;
		}

		UIAMError("Could not cast to GridSpacerWidget", widget, parent, layout);

		return NULL;
	}

	static WrapSpacerWidget CreateWrapSpacer( notnull Widget parent )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIWrapSpacer.layout", parent );

		CheckWidget(widget, parent);

		WrapSpacerWidget spacer;
		if (Class.CastTo(spacer, widget))
		{
			return spacer;
		}

		UIAMError("Could not cast to WrapSpacerWidget", widget, parent);

		return NULL;
	}

	static Widget CreateActionRows( notnull Widget parent )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionContentRows.layout", parent );

		CheckWidget(widget, parent);

		return widget;
	}
	
	static Widget CreateSpacer( notnull Widget parent )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UISpacer.layout", parent );

		CheckWidget(widget, parent);

		return widget;
	}

	static Widget CreatePanel( notnull Widget parent, int color = 0x00000000, float height = -1 )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIPanel.layout", parent );

		CheckWidget(widget, parent);

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

		CheckWidget(widget, parent);

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

		UIAMError("Couldn't get script", widget, parent);

		return NULL;
	}

	static UIActionButtonToggle CreateButtonToggle( notnull Widget parent, string buttonOff, string buttonOn, Class instance, string funcname, float width = 1 )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionButtonToggle.layout", parent );

		CheckWidget(widget, parent);

		UIActionButtonToggle action;
		widget.GetScript( action );

		if ( width != 1 )
		{
			SetSize(widget, width, -1);
		}

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetButtonToggle( buttonOff, buttonOn );

			return action;
		}

		UIAMError("Couldn't get script", widget, parent);

		return NULL;
	}

	static UIActionNavigateButton CreateNavButton( notnull Widget parent, string button, string imageR, string imageL, Class instance, string funcname, float width = 1 )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionNavigateButton.layout", parent );

		CheckWidget(widget, parent);

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

		UIAMError("Couldn't get script", widget, parent);

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

		CheckWidget(widget, parent, layout);

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

		UIAMError("Couldn't get script", widget, parent, layout);

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

		CheckWidget(widget, parent, layout);

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

		UIAMError("Couldn't get script", widget, parent, layout);

		return NULL;
	}

	static UIActionEditableRichText CreateEditableRichText( notnull Widget parent, string label, Class instance = NULL, string funcname = "", string text = "", string button = "" )
	{
		bool hasButton;
		if (button != "" && instance != NULL && funcname != "")
			hasButton = true;

		string layoutName;
		if (hasButton)
			layoutName = "UIActionEditableRichTextButton";
		else
			layoutName = "UIActionEditableRichText";

		//! Assemble path outside of call to CreateWidgets to work-around https://feedback.bistudio.com/T183345
		string layout = string.Format("JM/COT/GUI/layouts/uiactions/%1.layout", layoutName);
		Widget widget = GetGame().GetWorkspace().CreateWidgets( layout, parent );

		CheckWidget(widget, parent, layout);

		UIActionEditableRichText action;
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

		UIAMError("Couldn't get script", widget, parent, layout);

		return NULL;
	}

	static UIActionDropdownList CreateDropdownBox( notnull Widget parent, Widget dropDownParent, string label, array< string > values, Class instance = NULL, string funcname = "" )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionDropdownList.layout", parent );

		CheckWidget(widget, parent);

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

		UIAMError("Couldn't get script", widget, parent);

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

		CheckWidget(widget, parent, layout);

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

		UIAMError("Couldn't get script", widget, parent, layout);

		return NULL;
	}

	static UIActionCheckbox CreateCheckbox( notnull Widget parent, string label, Class instance = NULL, string funcname = "", bool checked = false, float width = 1 )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionCheckbox.layout", parent );

		CheckWidget(widget, parent);

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

		UIAMError("Couldn't get script", widget, parent);

		return NULL;
	}

	static UIActionText CreateText( notnull Widget parent, string label, string text = "", Class instance = NULL, string funcname = "", UIActionHAlign lha = UIActionHAlign.LEFT, UIActionHAlign lva = UIActionHAlign.CENTER, UIActionHAlign tha = UIActionHAlign.RIGHT, UIActionHAlign tva = UIActionHAlign.CENTER )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionText.layout", parent );

		CheckWidget(widget, parent);

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

		UIAMError("Couldn't get script", widget, parent);

		return NULL;
	}

	static UIActionSelectBox CreateSelectionBox( notnull Widget parent, string label, notnull array< string > options, Class instance = NULL, string funcname = "" )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionSelectBox.layout", parent );

		CheckWidget(widget, parent);

		UIActionSelectBox action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			action.SetSelections( options );

			return action;
		}

		UIAMError("Couldn't get script", widget, parent);

		return NULL;
	}

	static UIActionSlider CreateSlider( notnull Widget parent, string label, float min, float max, Class instance = NULL, string funcname = "" )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionSlider.layout", parent );

		CheckWidget(widget, parent);

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

		UIAMError("Couldn't get script", widget, parent);

		return NULL;
	}

	static UIActionScroller CreateScroller( notnull Widget parent )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionScroller.layout", parent );

		CheckWidget(widget, parent);

		UIActionScroller action;
		widget.GetScript( action );

		if ( action )
		{
			return action;
		}

		UIAMError("Couldn't get script", widget, parent);

		return NULL;
	}

	static UIActionImage CreateImage( notnull Widget parent, string image, Class instance = NULL, string funcname = "", UIActionHAlign lha = UIActionHAlign.LEFT, UIActionHAlign lva = UIActionHAlign.CENTER, UIActionHAlign tha = UIActionHAlign.RIGHT, UIActionHAlign tva = UIActionHAlign.CENTER )
	{
		Widget widget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionImage.layout", parent );

		CheckWidget(widget, parent);

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

		UIAMError("Couldn't get script", widget, parent);

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

	static void CheckWidget(Widget widget, Widget parent, string layout = string.Empty)
	{
		if (!widget)
			UIAMError("No widget created", widget, parent, layout);
		else if (widget == parent)
			UIAMError("widget == parent", widget, parent, layout);
	}

	static void UIAMError(string err, Widget widget, Widget parent, string layout = string.Empty)
	{
		string widgetStr;
		string widgetName;
		string widgetTypeName;

		string parentStr;
		string parentName;
		string parentTypeName;

		if (widget)
		{
			widgetStr = widget.ToString();
			if (widgetStr != "INVALID")
			{
				widgetName = widget.GetName();
				widgetTypeName = widget.GetTypeName();
			}
		}
		else
		{
			widgetStr = "NULL";
		}

		if (parent)
		{
			parentStr = parent.ToString();
			if (parentStr != "INVALID")
			{
				parentName = parent.GetName();
				parentTypeName = parent.GetTypeName();
			}
		}
		else
		{
			parentStr = "NULL";
		}

		Error(string.Format("%1 %2 (name=\"%3\" type=%4 layout=\"%5\"), parent %6 (name=\"%7\" type=%8)", err, widgetStr, widgetName, widgetTypeName, layout, parentStr, parentName, parentTypeName));
	}
};
