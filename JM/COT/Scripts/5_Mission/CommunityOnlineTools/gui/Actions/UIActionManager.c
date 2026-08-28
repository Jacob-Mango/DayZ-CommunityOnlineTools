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

	// ---------------------------------------------------------------------------
	//  CreateFlexRow - a WrapSpacer wrapped in a UIActionFlexRow so children can
	//  use SetFlex(grow, minPx, maxPx) for CSS-flexbox-style "min width + grow to
	//  fill" layout the raw WrapSpacer cannot express. Add children with
	//  row.Add(action) and call row.Layout() from the form's OnResize.
	//  See UIActionFlexRow.c for the full model.
	// ---------------------------------------------------------------------------
	static ref UIActionFlexRow CreateFlexRow( notnull Widget parent, WidgetAlignment halign = WidgetAlignment.WA_LEFT, WidgetAlignment valign = WidgetAlignment.WA_CENTER )
	{
		WrapSpacerWidget row = CreateWrapSpacer( parent, halign, valign );
		if ( !row )
			return null;

		return new UIActionFlexRow( row );
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
			SetFixedHeight(widget, height);
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
			SetWidthFraction(widget, width);
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

	// ---------------------------------------------------------------------------
	//  CreateFeedbackButton - standard pill button that briefly cross-fades its
	//  own label and icon to a confirmation ("Copy Steam ID" -> "Copied!") after
	//  a click, then fades back. For actions with no other visible result.
	//
	//  Pass feedbackIcon "" to keep the default check mark. Tune with
	//  action.SetFeedbackDuration() / SetFeedbackColor(), or take manual control
	//  with action.SetAutoFeedback( false ) + action.ShowFeedback().
	// ---------------------------------------------------------------------------
	static UIActionFeedbackButton CreateFeedbackButton( notnull Widget parent, string button, string feedback, string feedbackIcon, Class instance, string funcname, float width = 1 )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionFeedbackButton.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionFeedbackButton" ) )
			return null;

		UIActionFeedbackButton action;
		widget.GetScript( action );

		if ( width != 1 )
		{
			SetWidthFraction( widget, width );
		}

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetButton( button );
			action.SetFeedback( feedback, feedbackIcon );

			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );

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
			SetWidthFraction(widget, width);
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
			SetWidthFraction(widget, width);
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
				dropDownParent = FindPopupAnchor( parent );

			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			action.InitListWidget( dropDownParent );
			if ( values )
				action.SetItems( values );

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
			SetWidthFraction(widget, width);
		}

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			action.SetCheckedSilent( checked );

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

	// Walk up from `start` until we reach a widget that isn't a GridSpacer
	// (or its wrap / frame variants). That widget makes a suitable anchor for
	// a floating popup: its children aren't forced into a grid layout.
	protected static Widget FindPopupAnchor( notnull Widget start )
	{
		Widget w = start;
		while ( w )
		{
			GridSpacerWidget grid = GridSpacerWidget.Cast( w );
			WrapSpacerWidget wrap = WrapSpacerWidget.Cast( w );
			if ( !grid && !wrap )
				return w;
			Widget p = w.GetParent();
			if ( !p )
				return w;
			w = p;
		}
		return start;
	}

	static UIActionSlider CreateSlider( notnull Widget parent, string label, float min, float max, Class instance = NULL, string funcname = "", float sliderWidth = -1.0 )
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

			if ( sliderWidth > 0.0 )
				action.SetSliderWidth( sliderWidth );

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
			SetWidthFraction(widget, width);
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
	// iconPath - full icon path constant from JMIcons.c, e.g. JMIcon_Add, JMIcon_Trash
	static UIActionImageButton CreateIconButton( notnull Widget parent, string iconPath, Class instance, string funcname, float width = 1 )
	{
		return CreateImageButton( parent, iconPath, instance, funcname, width );
	}

	// ---------------------------------------------------------------------------
	//  Premade Standardized Action Buttons (Refresh, Delete, Save, Apply, Copy, Paste)
	// ---------------------------------------------------------------------------
	static UIActionImageButton CreateRefreshButton( notnull Widget parent, Class instance, string funcname, string tooltip = "#STR_COT_GENERIC_REFRESH" )
	{
		UIActionImageButton btn = CreateIconButton( parent, JMConstants.Lucide( "refresh-cw" ), instance, funcname );
		if ( btn )
		{
			btn.SetFixedSize( 28, 28 );
			if ( tooltip != "" ) btn.SetTooltip( tooltip );
		}
		return btn;
	}

	static UIActionImageButton CreateDeleteButton( notnull Widget parent, Class instance, string funcname, string tooltip = "#STR_COT_GENERIC_DELETE" )
	{
		UIActionImageButton btn = CreateIconButton( parent, JMConstants.Lucide( "trash-2" ), instance, funcname );
		if ( btn )
		{
			btn.SetFixedSize( 28, 28 );
			btn.SetColor( JMTheme.DANGER );
			btn.SetFeedbackColor( JMTheme.DANGER );
			if ( tooltip != "" ) btn.SetTooltip( tooltip );
		}
		return btn;
	}

	static UIActionImageButton CreateSaveButton( notnull Widget parent, Class instance, string funcname, string tooltip = "#STR_COT_GENERIC_SAVE" )
	{
		UIActionImageButton btn = CreateIconButton( parent, JMConstants.Lucide( "save" ), instance, funcname );
		if ( btn )
		{
			btn.SetFixedSize( 28, 28 );
			if ( tooltip != "" ) btn.SetTooltip( tooltip );
		}
		return btn;
	}

	static UIActionImageButton CreateApplyButton( notnull Widget parent, Class instance, string funcname, string tooltip = "#STR_COT_GENERIC_APPLY" )
	{
		UIActionImageButton btn = CreateIconButton( parent, JMConstants.Lucide( "check" ), instance, funcname );
		if ( btn )
		{
			btn.SetFixedSize( 28, 28 );
			if ( tooltip != "" ) btn.SetTooltip( tooltip );
		}
		return btn;
	}

	static UIActionImageButton CreateCopyButton( notnull Widget parent, Class instance, string funcname, string tooltip = "#STR_COT_GENERIC_COPY" )
	{
		UIActionImageButton btn = CreateIconButton( parent, JMConstants.Lucide( "copy" ), instance, funcname );
		if ( btn )
		{
			btn.SetFixedSize( 28, 28 );
			if ( tooltip != "" ) btn.SetTooltip( tooltip );
		}
		return btn;
	}

	static UIActionImageButton CreatePasteButton( notnull Widget parent, Class instance, string funcname, string tooltip = "#STR_COT_GENERIC_PASTE" )
	{
		UIActionImageButton btn = CreateIconButton( parent, JMConstants.Lucide( "clipboard-paste" ), instance, funcname );
		if ( btn )
		{
			btn.SetFixedSize( 28, 28 );
			if ( tooltip != "" ) btn.SetTooltip( tooltip );
		}
		return btn;
	}

	static UIActionImageButton CreateAddButton( notnull Widget parent, Class instance, string funcname, string tooltip = "#STR_COT_GENERIC_ADD" )
	{
		UIActionImageButton btn = CreateIconButton( parent, JMConstants.Lucide( "plus" ), instance, funcname );
		if ( btn )
		{
			btn.SetFixedSize( 28, 28 );
			if ( tooltip != "" ) btn.SetTooltip( tooltip );
		}
		return btn;
	}

	static UIActionImageButton CreateEditButton( notnull Widget parent, Class instance, string funcname, string tooltip = "#STR_COT_GENERIC_EDIT" )
	{
		UIActionImageButton btn = CreateIconButton( parent, JMConstants.Lucide( "pencil" ), instance, funcname );
		if ( btn )
		{
			btn.SetFixedSize( 28, 28 );
			if ( tooltip != "" ) btn.SetTooltip( tooltip );
		}
		return btn;
	}

	static UIActionImageButton CreateSearchButton( notnull Widget parent, Class instance, string funcname, string tooltip = "#STR_COT_GENERIC_SEARCH" )
	{
		UIActionImageButton btn = CreateIconButton( parent, JMConstants.Lucide( "search" ), instance, funcname );
		if ( btn )
		{
			btn.SetFixedSize( 28, 28 );
			if ( tooltip != "" ) btn.SetTooltip( tooltip );
		}
		return btn;
	}

	static UIActionImageButton CreateLockButton( notnull Widget parent, Class instance, string funcname, string tooltip = "#STR_COT_GENERIC_LOCK" )
	{
		UIActionImageButton btn = CreateIconButton( parent, JMConstants.Lucide( "lock" ), instance, funcname );
		if ( btn )
		{
			btn.SetFixedSize( 28, 28 );
			if ( tooltip != "" ) btn.SetTooltip( tooltip );
		}
		return btn;
	}

	static UIActionImageButton CreateVisibilityButton( notnull Widget parent, Class instance, string funcname, string tooltip = "#STR_COT_GENERIC_VIEW" )
	{
		UIActionImageButton btn = CreateIconButton( parent, JMConstants.Lucide( "eye" ), instance, funcname );
		if ( btn )
		{
			btn.SetFixedSize( 28, 28 );
			if ( tooltip != "" ) btn.SetTooltip( tooltip );
		}
		return btn;
	}

	// ---------------------------------------------------------------------------
	//  CreateImageButtonToggle - one pill flipping between two images. Read the
	//  new value with action.IsToggled() from the callback. Give it a tooltip:
	//  an icon-only control says nothing until it is hovered.
	// ---------------------------------------------------------------------------
	static UIActionImageButtonToggle CreateImageButtonToggle( notnull Widget parent, string imageOff, string imageOn, Class instance, string funcname, float width = 1 )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionImageButtonToggle.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionImageButtonToggle" ) )
			return null;

		UIActionImageButtonToggle action;
		widget.GetScript( action );

		if ( width != 1 )
		{
			SetWidthFraction( widget, width );
		}

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetImageToggle( imageOff, imageOn );

			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );

		return NULL;
	}

	static void SetWidthFraction( Widget widget, float width )
	{
		float w;
		float h;

		widget.GetSize( w, h );
		widget.ClearFlags( WidgetFlags.HEXACTSIZE );
		widget.SetSize( width, h );
	}

	static void SetFixedHeight( Widget widget, float height )
	{
		float w;
		float h;

		widget.GetSize( w, h );
		widget.SetFlags( WidgetFlags.VEXACTSIZE );
		widget.SetSize( w, height );
	}

	static void SetFixedSize( Widget widget, float width, float height )
	{
		widget.SetFlags( WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE );
		widget.SetSize( width, height );
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

	// Bulk creation helpers
	static array<ref UIActionButton> CreateButtonRow( notnull Widget parent, array<string> labels, Class instance, array<string> callbacks )
	{
		if ( !labels || !callbacks || labels.Count() != callbacks.Count() )
			return null;

		Widget grid = CreateGridSpacer( parent, 1, labels.Count() );
		array<ref UIActionButton> buttons = new array<ref UIActionButton>;

		for ( int i = 0; i < labels.Count(); i++ )
		{
			buttons.Insert( CreateButton( grid, labels[i], instance, callbacks[i] ) );
		}

		return buttons;
	}

	static UIActionText CreateLabeledValue( notnull Widget parent, string label, string value = "" )
	{
		return CreateText( parent, label, value );
	}

	static Widget CreateButtonGrid( notnull Widget parent, int rows, int cols, array<string> labels, Class instance, array<string> callbacks )
	{
		if ( !labels || !callbacks || labels.Count() != callbacks.Count() )
			return null;

		Widget grid = CreateGridSpacer( parent, rows, cols );

		for ( int i = 0; i < labels.Count(); i++ )
		{
			CreateButton( grid, labels[i], instance, callbacks[i] );
		}

		return grid;
	}

	static UIActionEditableText CreateLabeledInput( notnull Widget parent, string label, Class instance, string callback )
	{
		Widget row = CreateGridSpacer( parent, 1, 2 );
		CreateText( row, label );
		return CreateEditableText( row, "", instance, callback );
	}

	// ---------------------------------------------------------------------------
	//  CreateScrollableSection - scroller + inner content grid in one call.
	//  Returns a JMScrollableSection whose GetContent() is ready for children.
	//  Call section.UpdateScroller() from OnResize() to keep the scroller live.
	//
	//  innerRows controls how many rows the inner GridSpacer has (default 1).
	//
	//  Example:
	//      m_Scroller = UIActionManager.CreateScrollableSection( layoutRoot.FindAnyWidget("panel") );
	//      Widget content = m_Scroller.GetContent();
	//      UIActionManager.CreateSectionHeader( content, "General" );
	//      m_MyButton = UIActionManager.CreateButton( content, "Do thing", this, "OnClick_DoThing" );
	//
	//  OnResize:
	//      override void OnResize( float w, float h ) { m_Scroller.UpdateScroller(); }
	// ---------------------------------------------------------------------------
	static JMScrollableSection CreateScrollableSection( notnull Widget parent, int innerRows = 1 )
	{
		UIActionScroller scroller = CreateScroller( parent );
		if ( !scroller )
			return null;

		Widget content = CreateGridSpacer( scroller.GetContentWidget(), innerRows, 1 );
		return new JMScrollableSection( scroller, content );
	}

	// ---------------------------------------------------------------------------
	//  ClearChildren - detach all immediate children of a widget without
	//  deleting them.  Use this to rebuild dynamic lists safely instead of
	//  calling `delete widget` which can leave dangling references.
	//
	//  Example:
	//      UIActionManager.ClearChildren( m_ListContainer );
	//      foreach ( MyData d : m_Data ) BuildRow( m_ListContainer, d );
	// ---------------------------------------------------------------------------
	static void ClearChildren( notnull Widget parent )
	{
		Widget child = parent.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}
	}

	// ---------------------------------------------------------------------------
	//  CreateSectionHeader - title text row + thin divider line.
	//  Returns the container widget (2-row grid).
	//
	//  dividerColor defaults to JMUIStyle.DIVIDER_MEDIUM.
	//
	//  Example:
	//      UIActionManager.CreateSectionHeader( parent, "Teleport" );
	//      UIActionManager.CreateSectionHeader( parent, "Danger Zone", JMUIStyle.DIVIDER_LIGHT );
	// ---------------------------------------------------------------------------
	static Widget CreateSectionHeader( notnull Widget parent, string title, int dividerColor = JMTheme.DIVIDER_MEDIUM )
	{
		Widget container = CreateGridSpacer( parent, 2, 1 );
		CreateText( container, title );
		CreatePanel( container, dividerColor, 2 );
		return container;
	}

	// ---------------------------------------------------------------------------
	//  CreateCard - a bordered container with an optional title bar. This is the
	//  "category" chrome; put the section's rows into card.GetContent().
	//
	//  Prefer this over CreateSectionHeader for anything that is a group of
	//  related rows. A rule under a title does not group anything - with several
	//  sections stacked in one scroller there is nothing to say where one ends
	//  and the next begins. Pass "" for a card with no header.
	//
	//  Example:
	//      UIActionCard card = UIActionManager.CreateCard( parent, "#STR_..._VITALS" );
	//      UIActionManager.CreateSlider( card.GetContent(), ... );
	// ---------------------------------------------------------------------------
	static UIActionCard CreateCard( notnull Widget parent, string title = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionCard.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionCard" ) )
			return null;

		UIActionCard action;
		widget.GetScript( action );

		if ( action )
			action.SetLabel( title );

		return action;
	}

	// ---------------------------------------------------------------------------
	//  CreateSectionHeaderAction - a section header with a single icon button on
	//  the right of the title. Returns the button.
	//
	//  The title row is ONE fixed-height panel with both children placed by
	//  fraction, not a grid: a grid row sizes to its content and the content
	//  sizes to the row, which resolves circularly and blows the row out.
	//
	//  Example:
	//      m_Refresh = UIActionManager.CreateSectionHeaderAction( parent, "#STR_...", JMConstants.Lucide( "refresh-cw" ), this, "Click_Refresh" );
	// ---------------------------------------------------------------------------
	static const int SECTION_HEADER_ROW_HEIGHT = 30;

	static UIActionImageButton CreateSectionHeaderAction( notnull Widget parent, string title, string icon, Class instance, string funcname, int dividerColor = JMTheme.DIVIDER_MEDIUM )
	{
		Widget container = CreateGridSpacer( parent, 2, 1 );

		Widget titleRow = CreatePanel( container, 0x00000000, SECTION_HEADER_ROW_HEIGHT );
		CreateText( titleRow, title );

		UIActionImageButton button = CreateIconButton( titleRow, icon, instance, funcname );
		if ( button )
		{
			button.SetWidth( 0.12 );
			button.SetPosition( 0.88 );
		}

		CreatePanel( container, dividerColor, 2 );
		return button;
	}

	// ---------------------------------------------------------------------------
	//  CreateLabeledRow - a 1x2 grid with a text label on the left and an empty
	//  panel cell on the right.  Returns the right-hand cell so the caller can
	//  place any widget inside it.
	//
	//  labelFraction controls how wide the label column is (0..1, default 0.30).
	//
	//  Example:
	//      Widget cell = UIActionManager.CreateLabeledRow( parent, "Duration (s):" );
	//      m_DurationInput = UIActionManager.CreateEditableText( cell, "", this, "OnChange_Duration" );
	//
	//      Widget cell2 = UIActionManager.CreateLabeledRow( parent, "Mode:", 0.4 );
	//      m_ModeDropdown = UIActionManager.CreateDropdownBox( cell2, layoutRoot, "", options, this, "OnChange_Mode" );
	// ---------------------------------------------------------------------------
	static Widget CreateLabeledRow( notnull Widget parent, string label, float labelFraction = 0.30 )
	{
		Widget row = CreateGridSpacer( parent, 1, 2 );

		UIActionText lbl = CreateText( row, label );
		if ( lbl )
			lbl.SetWidth( labelFraction );

		Widget inputCell = CreatePanel( row, 0x00000000 );
		return inputCell;
	}

	// ---------------------------------------------------------------------------
	//  CreateButtonPair - two equally-spaced buttons side by side inside a
	//  1x2 grid.  Both out-params are optional; pass null variables if you don't
	//  need one of the buttons back.
	//
	//  Example:
	//      UIActionButton btnApply, btnCancel;
	//      UIActionManager.CreateButtonPair( parent,
	//          "Apply",  this, "OnClick_Apply",  btnApply,
	//          "Cancel", this, "OnClick_Cancel", btnCancel );
	// ---------------------------------------------------------------------------
	static void CreateButtonPair( notnull Widget parent,
		string labelA, Class instA, string cbA, out UIActionButton btnA,
		string labelB, Class instB, string cbB, out UIActionButton btnB )
	{
		Widget row = CreateGridSpacer( parent, 1, 2 );
		btnA = CreateButton( row, labelA, instA, cbA );
		btnB = CreateButton( row, labelB, instB, cbB );
	}

	// ---------------------------------------------------------------------------
	//  CreateButtonPair (colored) - two side-by-side buttons with explicit
	//  widths and optional colors (pass 0 for no color change).
	//  widthA + widthB should sum to 1.0.
	//
	//  Example:
	//      UIActionButton btnBan, btnUnban;
	//      UIActionManager.CreateButtonPair( parent,
	//          "Ban",   this, "OnClick_Ban",   COLOR_RED, JMUILayout.BTN_PAIR_WIDE,
	//          "Unban", this, "OnClick_Unban", 0,         JMUILayout.BTN_PAIR_NARROW,
	//          btnBan, btnUnban );
	// ---------------------------------------------------------------------------
	static void CreateButtonPair( notnull Widget parent,
		string labelA, Class instA, string cbA, int colorA, float widthA,
		string labelB, Class instB, string cbB, int colorB, float widthB,
		out UIActionButton btnA, out UIActionButton btnB )
	{
		Widget row = CreateGridSpacer( parent, 1, 2 );
		btnA = CreateButton( row, labelA, instA, cbA );
		if ( btnA )
		{
			if ( colorA != 0 ) btnA.SetColor( colorA );
			btnA.SetWidth( widthA );
		}
		btnB = CreateButton( row, labelB, instB, cbB );
		if ( btnB )
		{
			if ( colorB != 0 ) btnB.SetColor( colorB );
			btnB.SetWidth( widthB );
			btnB.SetPosition( widthA );
		}
	}

	// ---------------------------------------------------------------------------
	//  CreateDivider - thin horizontal rule with no label.
	//  Shorthand for CreatePanel with sensible defaults so callers don't need
	//  to remember magic color values or heights.
	//
	//  Example:
	//      UIActionManager.CreateDivider( parent );
	//      UIActionManager.CreateDivider( parent, JMUIStyle.DIVIDER_LIGHT, 1 );
	// ---------------------------------------------------------------------------
	static void CreateDivider( notnull Widget parent,
		int color = JMTheme.DIVIDER_MEDIUM, int height = 2 )
	{
		CreatePanel( parent, color, height );
	}

	// ===========================================================================
	//  New component factory methods
	// ===========================================================================

	// ---------------------------------------------------------------------------
	//  CreateProgressBar - horizontal fill bar with optional label and percentage.
	//  value is 0.0..1.0.  Call SetValue() to update.
	// ---------------------------------------------------------------------------
	static UIActionProgressBar CreateProgressBar( notnull Widget parent, string label = "", float value = 0.0 )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionProgressBar.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionProgressBar" ) )
			return null;

		UIActionProgressBar action;
		widget.GetScript( action );

		if ( action )
		{
			if ( label.Length() > 0 )
				action.SetLabel( label );
			action.SetProgress( value );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateCollapsibleSection - header button + collapsible content panel.
	//  GetContent() returns the inner Widget to add children to.
	// ---------------------------------------------------------------------------
	static UIActionCollapsibleSection CreateCollapsibleSection( notnull Widget parent, string title, bool startExpanded = true )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionCollapsibleSection.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionCollapsibleSection" ) )
			return null;

		UIActionCollapsibleSection action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetLabel( title );
			action.SetExpanded( startExpanded );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateSearchBox - text input with integrated clear button.
	//  Fires CHANGE on keystroke, CLICK when cleared.
	// ---------------------------------------------------------------------------
	static UIActionSearchBox CreateSearchBox( notnull Widget parent, Class instance = null, string funcname = "", string placeholder = "", string initialText = "", string label = "", Widget listAnchor = null, array<string> suggestions = null )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionSearchBox.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionSearchBox" ) )
			return null;

		UIActionSearchBox action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			if ( label.Length() > 0 )
				action.SetLabel( label );
			if ( placeholder.Length() > 0 )
				action.SetPlaceholder( placeholder );
			if ( initialText.Length() > 0 )
				action.SetText( initialText );
			if ( listAnchor )
				action.InitSuggestionList( listAnchor );
			if ( suggestions )
				action.SetSuggestions( suggestions );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateSpinner - label + value edit box + ?/? buttons for stepping.
	//  Fires CHANGE when value changes (button click, edit, or mouse wheel).
	// ---------------------------------------------------------------------------
	static UIActionSpinner CreateSpinner( notnull Widget parent, string label, float min, float max, float step = 1.0, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionSpinner.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionSpinner" ) )
			return null;

		UIActionSpinner action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			action.SetMinMax( min, max );
			action.SetStep( step );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateBadge - label + colored status pill.  Pure display, no interaction.
	// ---------------------------------------------------------------------------
	static UIActionBadge CreateBadge( notnull Widget parent, string label, string status = "", int color = JMTheme.SUCCESS_DIM )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionBadge.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionBadge" ) )
			return null;

		UIActionBadge action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetLabel( label );
			if ( status.Length() > 0 )
				action.SetStatus( status, color );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateStagedIcon - image that cycles through a list of (icon, color) stages.
	//  Call action.AddStage(path, color) to populate, SetStage(n) to set initial.
	// ---------------------------------------------------------------------------
	static UIActionStagedIcon CreateStagedIcon( notnull Widget parent, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionStagedIcon.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionStagedIcon" ) )
			return null;

		UIActionStagedIcon action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateDropdown - styled dropdown with per-entry icon + text.
	//  Pass the overlay widget (e.g. a FrameWidget above form content) as
	//  listAnchor so the open list floats over other widgets.
	//  Call action.AddEntry(text, iconPath, iconColor) to populate.
	// ---------------------------------------------------------------------------
	static UIActionDropdown CreateDropdown( notnull Widget parent, string label, notnull Widget listAnchor, Class instance = null, string funcname = "", array<string> items = null )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionDropdown.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionDropdown" ) )
			return null;

		UIActionDropdown action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			action.InitList( listAnchor );
			if ( items )
			{
				foreach ( string s: items )
					action.AddEntry( s );
			}
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateContextMenu - right-click popup menu.
	//  `parent` only holds the 1px stub that keeps the menu ticking; pass the
	//  window root as `anchor` so the popup floats above the form instead of
	//  being clipped by whatever container it was created in.
	//  Call action.AddItem(id, label, icon, color) then action.ShowAt(x, y).
	// ---------------------------------------------------------------------------
	static UIActionContextMenu CreateContextMenu( notnull Widget parent, notnull Widget anchor, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionContextMenu.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionContextMenu" ) )
			return null;

		UIActionContextMenu action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.InitMenu( anchor );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateValuePrompt - modal "pick one value" dialog.
	//  `parent` only holds the 1px stub that keeps the prompt ticking; pass the
	//  window root as `anchor` so the panel floats over the form.
	//  Call action.ShowSlider(...) or action.ShowOptions(...) to raise it; the
	//  callback fires on Confirm only.
	// ---------------------------------------------------------------------------
	static UIActionValuePrompt CreateValuePrompt( notnull Widget parent, notnull Widget anchor, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionValuePrompt.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionValuePrompt" ) )
			return null;

		UIActionValuePrompt action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.InitPrompt( anchor );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateToggle - custom checkbox / radio button.
	//  SetRound(true) for radio (single-choice) style.
	//  SetChecked(bool) to set state programmatically.
	// ---------------------------------------------------------------------------
	static UIActionToggle CreateToggle( notnull Widget parent, string label, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionToggle.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionToggle" ) )
			return null;

		UIActionToggle action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateTabs - row of tab buttons that show/hide registered content panels.
	//  Call action.AddContent(panel) for each tab after creation, then SetSelection(0).
	// ---------------------------------------------------------------------------
	static UIActionTabs CreateTabs( notnull Widget parent, notnull array<string> labels, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionTabs.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionTabs" ) )
			return null;

		UIActionTabs action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetTabs( labels );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateTabs - same, with one icon path per tab. Pass "" for a text-only tab.
	// ---------------------------------------------------------------------------
	static UIActionTabs CreateTabs( notnull Widget parent, notnull array<string> labels, notnull array<string> icons, Class instance = null, string funcname = "" )
	{
		UIActionTabs action = CreateTabs( parent, labels, instance, funcname );
		if ( !action )
			return null;

		action.SetTabIcons( icons );
		return action;
	}

	// ---------------------------------------------------------------------------
	//  CreateMultiSelectList - scrollable checkbox list.
	//  Fires CHANGE on any checkbox toggle.  Use GetSelectedItems() to read result.
	// ---------------------------------------------------------------------------
	static UIActionMultiSelectList CreateMultiSelectList( notnull Widget parent, string label, array<string> items = null, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionMultiSelectList.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionMultiSelectList" ) )
			return null;

		UIActionMultiSelectList action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			if ( label.Length() > 0 )
				action.SetLabel( label );
			if ( items )
				action.SetItems( items );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateColorPicker - label + hex input + live color swatch.
	//  Fires CHANGE when a valid hex color is entered.  GetColor() returns ARGB.
	// ---------------------------------------------------------------------------
	static UIActionColorPicker CreateColorPicker( notnull Widget parent, string label, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionColorPicker.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionColorPicker" ) )
			return null;

		UIActionColorPicker action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateToggleSwitch - label + visual on/off switch with sliding thumb.
	//  Fires CLICK on toggle.  IsChecked() returns state.
	// ---------------------------------------------------------------------------
	static UIActionToggleSwitch CreateToggleSwitch( notnull Widget parent, string label, Class instance = null, string funcname = "", bool initialState = false )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionToggleSwitch.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionToggleSwitch" ) )
			return null;

		UIActionToggleSwitch action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			action.SetChecked( initialState );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateDataTable - multi-column table with header + scrollable rows.
	//  Call SetColumns() then AddRow() to populate.  Fires CLICK on row select.
	// ---------------------------------------------------------------------------
	static UIActionDataTable CreateDataTable( notnull Widget parent, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionDataTable.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionDataTable" ) )
			return null;

		UIActionDataTable action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateKeyValueList - dynamic label+value info panel.
	//  Call SetValue("key", "value") to add/update rows in place.
	// ---------------------------------------------------------------------------
	static UIActionKeyValueList CreateKeyValueList( notnull Widget parent )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionKeyValueList.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionKeyValueList" ) )
			return null;

		UIActionKeyValueList action;
		widget.GetScript( action );

		if ( action )
			return action;

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreatePaginator - prev/next buttons + "X / N" page label.
	//  Fires CHANGE on page change.  Use GetCurrentPage() (0-based) in handler.
	// ---------------------------------------------------------------------------
	static UIActionPaginator CreatePaginator( notnull Widget parent, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionPaginator.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionPaginator" ) )
			return null;

		UIActionPaginator action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateFilterBar - row of toggle filter buttons.
	//  Exclusive by default; call SetMultiSelect(true) for multi-select mode.
	//  Fires CHANGE on any toggle.  Use GetActiveFilters() / GetActiveIndices().
	// ---------------------------------------------------------------------------
	static UIActionFilterBar CreateFilterBar( notnull Widget parent, notnull array<string> filters, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionFilterBar.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionFilterBar" ) )
			return null;

		UIActionFilterBar action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetFilters( filters );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateSliderRange - dual-handle [low, high] range slider.
	//  Fires CHANGE on either handle move.  Use GetRangeLow/High().
	// ---------------------------------------------------------------------------
	static UIActionSliderRange CreateSliderRange( notnull Widget parent, string label, float min, float max, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionSliderRange.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionSliderRange" ) )
			return null;

		UIActionSliderRange action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			action.SetMinMax( min, max );
			action.SetRange( min, max );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateIconGrid - wrapping grid of icon buttons.
	//  Call AddIcon(id, imagePath, label) after creation.  Fires CLICK on press.
	// ---------------------------------------------------------------------------
	static UIActionIconGrid CreateIconGrid( notnull Widget parent, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionIconGrid.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionIconGrid" ) )
			return null;

		UIActionIconGrid action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateIconGridHorizontal - variant of CreateIconGrid where the root
	//  GridSpacer has "Size To Content H" 1 so the icon row grows
	//  horizontally instead of wrapping. Pair with UseHorizontalLayout() on
	//  the returned grid and an outer horizontal scroller.
	// ---------------------------------------------------------------------------
	static UIActionIconGrid CreateIconGridHorizontal( notnull Widget parent, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionIconGridH.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionIconGrid" ) )
			return null;

		UIActionIconGrid action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateTimePicker - HH:MM:SS duration / time input.
	//  Fires CHANGE on field commit.  Use GetTotalSeconds() / SetTotalSeconds().
	// ---------------------------------------------------------------------------
	static UIActionTimePicker CreateTimePicker( notnull Widget parent, string label, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionTimePicker.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionTimePicker" ) )
			return null;

		UIActionTimePicker action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetLabel( label );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateConfirmInline - button that expands to Confirm/Cancel on first click.
	//  Fires CLICK on initial press, CHANGE on confirm, nothing on cancel/timeout.
	// ---------------------------------------------------------------------------
	static UIActionConfirmInline CreateConfirmInline( notnull Widget parent, string label, Class instance = null, string funcname = "", float timeout = 4.0 )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionConfirmInline.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionConfirmInline" ) )
			return null;

		UIActionConfirmInline action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			action.SetButton( label );
			action.SetTimeout( timeout );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateStepList - ordered list with Up/Down/Remove buttons per row.
	//  Call AddItem(label) to populate.  Fires CHANGE on every edit.
	// ---------------------------------------------------------------------------
	static UIActionStepList CreateStepList( notnull Widget parent, string label = "", Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionStepList.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionStepList" ) )
			return null;

		UIActionStepList action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			if ( label.Length() > 0 )
				action.SetLabel( label );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateLogView - scrollable read-only text log with per-line colors.
	//  Call AppendLine(text, color) to add entries.  Oldest lines drop when
	//  maxLines is exceeded.
	// ---------------------------------------------------------------------------
	static UIActionLogView CreateLogView( notnull Widget parent, int maxLines = 200 )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionLogView.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionLogView" ) )
			return null;

		UIActionLogView action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetMaxLines( maxLines );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	// ---------------------------------------------------------------------------
	//  CreateTooltip - create the global tooltip overlay for a form.
	//  Call once per form in OnInit, passing the form's layoutRoot.
	//  Any UIActionBase with SetTooltip("text") will then show a hover tooltip.
	// ---------------------------------------------------------------------------
	static UIActionTooltip CreateTooltip( notnull Widget anchor )
	{
		return new UIActionTooltip( anchor );
	}
}
