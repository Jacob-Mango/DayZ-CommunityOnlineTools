class UIActionManager
{
	static GridSpacerWidget CreateGridSpacer( notnull Widget parent, int rows, int columns )
	{
		//! Assemble path outside of call to CreateWidgets to work-around https://feedback.bistudio.com/T183345
		string layout = string.Format("JM/COT/GUI/layouts/uiactions/Wrappers/%1/GridSpacer%2.layout", rows, columns);
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "spacer"))
			return null;

		GridSpacerWidget spacer;
		if (Class.CastTo(spacer, widget))
		{
			return spacer;
		}

		UIAMError("Could not cast to GridSpacerWidget", widget, parent, layout);

		return NULL;
	}

	static WrapSpacerWidget CreateWrapSpacer( string layout, notnull Widget parent, WidgetAlignment halign = WidgetAlignment.WA_LEFT, WidgetAlignment valign = WidgetAlignment.WA_TOP)
	{
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "UIWrapSpacer"))
			return null;

		WrapSpacerWidget spacer;
		if (Class.CastTo(spacer, widget))
		{
			spacer.SetContentAlignmentH(halign);
			spacer.SetContentAlignmentV(valign);
			return spacer;
		}

		UIAMError("Could not cast to WrapSpacerWidget", widget, parent);

		return NULL;
	}

	static WrapSpacerWidget CreateWrapSpacer( notnull Widget parent, WidgetAlignment halign = WidgetAlignment.WA_LEFT, WidgetAlignment valign = WidgetAlignment.WA_TOP)
	{
		return CreateWrapSpacer("JM/COT/GUI/layouts/uiactions/UIWrapSpacer.layout", parent, halign, valign);
	}

	static WrapSpacerWidget CreateWrapSpacerCompact( notnull Widget parent, WidgetAlignment halign = WidgetAlignment.WA_LEFT, WidgetAlignment valign = WidgetAlignment.WA_TOP)
	{
		return CreateWrapSpacer("JM/COT/GUI/layouts/uiactions/UIWrapSpacerCompact.layout", parent, halign, valign);
	}

	static WrapSpacerWidget CreateWrapSpacerFit( notnull Widget parent, WidgetAlignment halign = WidgetAlignment.WA_LEFT, WidgetAlignment valign = WidgetAlignment.WA_TOP)
	{
		return CreateWrapSpacer("JM/COT/GUI/layouts/uiactions/UIWrapSpacerFit.layout", parent, halign, valign);
	}

	static Widget CreateActionRows( notnull Widget parent )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionContentRows.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "UIActionContentRows"))
			return null;

		return widget;
	}
	
	static Widget CreateSpacer( notnull Widget parent )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UISpacer.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "UISpacer"))
			return null;

		return widget;
	}

	static Widget CreatePanel( notnull Widget parent, int color = 0x00000000, float height = -1 )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIPanel.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "UIPanel"))
			return null;

		widget.SetColor( color );

		if ( height != -1 )
		{
			SetSize(widget, -1, height);
		}

		return widget;
	}

	static UIActionButton CreateButton( notnull Widget parent, string button, Class instance, string funcname, float width = 1 )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionButton.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "UIActionButton"))
			return null;

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
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionButtonToggle.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "UIActionButtonToggle"))
			return null;

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
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionNavigateButton.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "UIActionNavigateButton"))
			return null;

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

		string widgetName;
		if (hasButton)
			widgetName = "UIActionEditableTextPreviewButton";
		else
			widgetName = "UIActionEditableTextPreview";

		//! Assemble path outside of call to CreateWidgets to work-around https://feedback.bistudio.com/T183345
		string layout = string.Format("JM/COT/GUI/layouts/uiactions/%1.layout", widgetName);
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, widgetName))
			return null;

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

		string widgetName;
		if (hasButton)
			widgetName = "UIActionEditableTextButton";
		else
			widgetName = "UIActionEditableText";

		//! Assemble path outside of call to CreateWidgets to work-around https://feedback.bistudio.com/T183345
		string layout = string.Format("JM/COT/GUI/layouts/uiactions/%1.layout", widgetName);
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, widgetName))
			return null;

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

		string widgetName;
		if (hasButton)
			widgetName = "UIActionEditableRichTextButton";
		else
			widgetName = "UIActionEditableRichText";

		//! Assemble path outside of call to CreateWidgets to work-around https://feedback.bistudio.com/T183345
		string layout = string.Format("JM/COT/GUI/layouts/uiactions/%1.layout", widgetName);
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, widgetName))
			return null;

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
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionDropdownList.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "UIActionDropdownList"))
			return null;

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

		string widgetName;
		if (hasButton)
			widgetName = "UIActionEditableVectorButton";
		else
			widgetName = "UIActionEditableVector";

		//! Assemble path outside of call to CreateWidgets to work-around https://feedback.bistudio.com/T183345
		string layout = string.Format("JM/COT/GUI/layouts/uiactions/%1.layout", widgetName);
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, widgetName))
			return null;

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
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionCheckbox.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "UIActionCheckbox"))
			return null;

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
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionText.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "UIActionText"))
			return null;

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
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionSelectBox.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "UIActionSelectBox"))
			return null;

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
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionSlider.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "UIActionSlider"))
			return null;

		UIActionSlider action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			action.SetMinMax( min, max );
			action.SetFormat( "#STR_COT_FORMAT_NONE" );

			return action;
		}

		UIAMError("Couldn't get script", widget, parent);

		return NULL;
	}

	static UIActionScroller CreateScroller( notnull Widget parent )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionScroller.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "UIActionScroller"))
			return null;

		UIActionScroller action;
		widget.GetScript( action );

		if ( action )
			return action;

		UIAMError("Couldn't get script", widget, parent);

		return NULL;
	}

	static UIActionImage CreateImage( notnull Widget parent, string image, Class instance = NULL, string funcname = "", UIActionHAlign lha = UIActionHAlign.LEFT, UIActionHAlign lva = UIActionHAlign.CENTER, UIActionHAlign tha = UIActionHAlign.RIGHT, UIActionHAlign tva = UIActionHAlign.CENTER )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionImage.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "UIActionImage"))
			return null;

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

	static UIActionImageButton CreateImageButton( notnull Widget parent, string image, Class instance, string funcname, float width = 1 )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionImageButton.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if (!CheckWidget(widget, parent, layout, "UIActionImageButton"))
			return null;

		UIActionImageButton action;
		widget.GetScript( action );

		if ( width != 1 )
		{
			SetSize(widget, width, -1);
		}

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetImage( image );

			return action;
		}

		UIAMError("Couldn't get script", widget, parent);

		return NULL;
	}

	// Shorthand for creating a button with one of the built-in COT icon textures.
	// iconPath — full icon path constant from JMIcons.c, e.g. JMIcon_Add, JMIcon_Trash
	static UIActionImageButton CreateIconButton( notnull Widget parent, string iconPath, Class instance, string funcname, float width = 1 )
	{
		return CreateImageButton( parent, iconPath, instance, funcname, width );
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

	static bool CheckWidget(Widget widget, Widget parent, string layout = string.Empty, string widgetName = string.Empty)
	{
		if (!widget)
		{
			UIAMError("No widget created", widget, parent, layout);
			return false;
		}
		else if (widget.ToString() == "INVALID")
		{
			UIAMError("MEMORY CORRUPTION widget == INVALID", widget, parent, layout);
			return false;
		}
		else if (widget == parent)
		{
			UIAMError("MEMORY CORRUPTION widget == parent", widget, parent, layout);
			return false;
		}
		else if (widgetName && widget.GetName() != widgetName)
		{
			UIAMError(string.Format("MEMORY CORRUPTION unexpected widget name \"%1\" != \"%2\"", widget.GetName(), widgetName), widget, parent, layout);
			return false;
		}

		return true;
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
}
