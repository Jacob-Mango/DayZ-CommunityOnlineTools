class UIActionManager
{
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

	// ---------------------------------------------------------------------------
	//  Layout primitives. Prefer these to a bare CreatePanel: the name says what
	//  the panel is for, and the colours / sizes live here instead of in every form.
	//
	//    CreateRowDivider  - the 1px hairline between rows of a list
	//    CreateSpacerPx    - an empty gap of `height` pixels
	//    CreateRow         - a transparent fixed-height host for one row of controls
	//    CreateEmptyState  - a fixed-height "nothing here" message; returns its host
	// ---------------------------------------------------------------------------
	static const int ROW_DIVIDER_COLOR = 0x22FFFFFF;
	static const float EMPTY_STATE_HEIGHT = 60;

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

	static void SetWidthFraction( Widget widget, float width )
	{
		float w;
		float h;

		widget.GetSize( w, h );
		widget.ClearFlags( WidgetFlags.HEXACTSIZE );
		widget.SetSize( width, h );
	}

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

	static UIActionButton CreateButton( notnull Widget parent, string button, Class instance, string funcname, float width = 1, string permission = "" )
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

			if ( permission != "" && instance && instance.IsInherited( JMFormBase ) )
			{
				JMFormBase.Cast( instance ).BindPermission( action, permission );
			}

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

	static UIActionCheckbox CreateCheckbox( notnull Widget parent, string label, Class instance = NULL, string funcname = "", bool checked = false, float width = 1, string icon = "" )
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
			if ( icon != "" )
				action.SetIcon( icon );

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

	// ---------------------------------------------------------------------------
	//  CreateSyncedSlider - a slider with a numeric box beside it, so the admin
	//  can drag OR type an exact value. Drop-in for CreateSlider: it returns the
	//  same UIActionSlider, the ( UIEvent, UIActionBase ) callback fires for both
	//  input paths, and SetCurrent keeps the box in step. The slider owns the
	//  UIActionSliderSync, and hiding / disabling the slider does the same to the
	//  box. Set step and format on the returned slider exactly as before.
	//
	//  editWidth is the box's share of the row (0-1); the slider takes the rest.
	// ---------------------------------------------------------------------------
	static UIActionSlider CreateSyncedSlider( notnull Widget parent, string label, float min, float max, Class instance = NULL, string funcname = "", float editWidth = 0.16 )
	{
		Widget row = CreateWrapSpacerCompact( parent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		if ( !row )
			return null;

		UIActionSlider slider = CreateSlider( row, label, min, max, instance, funcname );
		if ( !slider )
			return null;

		UIActionEditableText box = CreateEditableText( row, "" );
		if ( !box )
			return slider;

		box.SetEditBoxWidth( 1.0 );

		UIActionSliderSync sync = new UIActionSliderSync( slider, box, min, max, slider.GetStepValue() );
		sync.SetRow( row );
		sync.SetEditShare( editWidth );
		sync.ApplyWidth( 1.0 );

		return slider;
	}

	// ---------------------------------------------------------------------------
	//  CreateScrollCard - a scroller holding one titled card, for the common
	//  "whole tab is one card" layout. `scroller` receives the scroller (keep it
	//  to call UpdateScroller after a rebuild); put rows in the returned card's
	//  GetContent().
	// ---------------------------------------------------------------------------
	static UIActionCard CreateScrollCard( notnull Widget parent, string title, out UIActionScroller scroller )
	{
		scroller = CreateScroller( parent );
		if ( !scroller )
			return null;

		return CreateCard( scroller.GetContentWidget(), title );
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
			btn.SetFixedSize( 30, 30 );

			//! Refreshing is the one standard action that is safe to leave
			//! running: it re-reads state and changes nothing. Double click
			//! arms it at one call a second, double click again stops it,
			//! and closing the owner stops it too.
			btn.EnableAutoRepeat( true );

			//! The icon spins itself on every click, so a refresh handler is
			//! just the request - it never animates its own button.
			btn.SetSpinOnClick( true );

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
	static void ClearChildren( Widget parent )
	{
		if ( !parent )
			return;

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
	//  CreateSection - a titled card, returned as its content widget: the one
	//  call a tab builder needs instead of CreateCard + null check + GetContent
	//  + null check. Use CreateCard when the card itself is needed (header
	//  buttons, collapse).
	//
	//  Example:
	//      Widget body = UIActionManager.CreateSection( parent, "Settings" );
	//      if ( body )
	//          UIActionManager.CreateButton( body, "Go", this, "OnClick_Go" );
	// ---------------------------------------------------------------------------
	static Widget CreateSection( notnull Widget parent, string title = "" )
	{
		UIActionCard card = CreateCard( parent, title );

		if ( !card )
			return null;

		return card.GetContent();
	}

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
	static void CreateButtonPair( notnull Widget parent, string labelA, Class instA, string cbA, out UIActionButton btnA, string labelB, Class instB, string cbB, out UIActionButton btnB )
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
	static void CreateButtonPair( notnull Widget parent, string labelA, Class instA, string cbA, int colorA, float widthA, string labelB, Class instB, string cbB, int colorB, float widthB, out UIActionButton btnA, out UIActionButton btnB )
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
	static Widget CreateDivider( notnull Widget parent, int color = JMTheme.DIVIDER_MEDIUM, int height = 2 )
	{
		return CreatePanel( parent, color, height );
	}

	static Widget CreateRowDivider( notnull Widget parent )
	{
		return CreatePanel( parent, ROW_DIVIDER_COLOR, 1 );
	}

	static Widget CreateSpacerPx( notnull Widget parent, float height )
	{
		return CreatePanel( parent, 0x00000000, height );
	}

	static Widget CreateRow( notnull Widget parent, float height )
	{
		return CreatePanel( parent, 0x00000000, height );
	}

	static Widget CreateEmptyState( notnull Widget parent, string title, string hint = "", float height = EMPTY_STATE_HEIGHT )
	{
		Widget host = CreatePanel( parent, 0x00000000, height );
		if ( host )
			CreateText( host, title, hint );

		return host;
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
	//  CreateMap - a world map with markers the caller can add, move and remove
	//  one at a time. `height` is in layout pixels; the map is not fractional
	//  because a MapWidget reports no content height and a size-to-content host
	//  would collapse it to nothing.
	//  Fires CLICK on a press and DOUBLE_CLICK on a double press; read what was
	//  hit off the action. See UIActionMap.
	// ---------------------------------------------------------------------------
	static UIActionMap CreateMap( notnull Widget parent, Class instance = null, string funcname = "", int height = 220 )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionMap.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionMap" ) )
			return null;

		UIActionMap action;
		widget.GetScript( action );

		if ( !action )
		{
			UIAMError( "Couldn't get script", widget, parent );
			return NULL;
		}

		action.SetCallback( instance, funcname );

		widget.SetFlags( WidgetFlags.VEXACTSIZE, true );

		float w, h;
		widget.GetSize( w, h );
		widget.SetSize( w, height );

		return action;
	}

	// ---------------------------------------------------------------------------
	//  CreateMapFill - a world map that fills whatever parent it is given,
	//  rather than the fixed pixel band CreateMap pins a card's map to. For a
	//  map that IS the whole tab, like Vehicle Manager's, not one dropped into
	//  a card alongside other content.
	// ---------------------------------------------------------------------------
	static UIActionMap CreateMapFill( notnull Widget parent, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionMap.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionMap" ) )
			return null;

		UIActionMap action;
		widget.GetScript( action );

		if ( !action )
		{
			UIAMError( "Couldn't get script", widget, parent );
			return NULL;
		}

		action.SetCallback( instance, funcname );

		//! Overrides the layout's own exact-220px sizing (authored for
		//! CreateMap's card use) with a fraction that fills the parent instead.
		//! SetFlags only ever ADDS a flag - VEXACTSIZE has to come off through
		//! ClearFlags, or this SetSize(1, 1) is read as an exact 1x1 PIXEL size
		//! instead of "100% of parent" and the map collapses to a sliver.
		widget.ClearFlags( WidgetFlags.VEXACTSIZE );
		widget.SetSize( 1, 1 );

		return action;
	}

	// ---------------------------------------------------------------------------
	//  CreateFoldPanel - headerless container that slides open and shut.
	//  Rows go into action.GetContent(); the panel measures them itself. Unlike
	//  CreateCollapsibleSection it brings no header of its own, so the control
	//  that opens it can live wherever the host's layout wants it.
	//  Fires CHANGE while the height is moving.
	// ---------------------------------------------------------------------------
	static UIActionFoldPanel CreateFoldPanel( notnull Widget parent, Class instance = null, string funcname = "", bool startExpanded = false )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionFoldPanel.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionFoldPanel" ) )
			return null;

		UIActionFoldPanel action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );

			// Never animated: this is the fold's starting state, not a fold the
			// user just asked for.
			action.SetExpanded( startExpanded, false );

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

			//! Unconditional: the pill's layout ships with a "{STATUS}"
			//! placeholder, so skipping this on an empty status left the
			//! literal token on screen and dropped the caller's colour too.
			//! An empty status is a deliberate blank pill, not "leave as-is".
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
	//  Call action.AddItem(id, label, icon, color) then action.OpenAt(x, y).
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
	//  CreateFilterMenu - a filter button's dropdown, as a stack of named
	//  pages (root list, drill down into a checkbox list, back). See
	//  UIActionFilterMenu.c's class header for why this exists instead of
	//  the menu-plus-submenu shape it replaces. `parent`/`anchor` are the
	//  same pair CreateContextMenu takes and mean the same thing.
	// ---------------------------------------------------------------------------
	static UIActionFilterMenu CreateFilterMenu( notnull Widget parent, notnull Widget anchor )
	{
		UIActionFilterMenu menu = new UIActionFilterMenu();
		menu.InitFilterMenu( parent, anchor );
		return menu;
	}

	// ---------------------------------------------------------------------------
	//  CreateValuePrompt - modal "pick one value" dialog.
	//  `parent` only holds the 1px stub that keeps the prompt ticking; pass the
	//  window root as `anchor` so the panel floats over the form.
	//  Call action.OpenSlider(...) or action.OpenOptions(...) to raise it; the
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
	//  CreateSearchRow - the toolbar above a list: [refresh] [search box] [filter],
	//  in one compact wrap row. Pass "" for refreshFn / filterFn to leave that
	//  button out. Widths are fractions of the row (0 keeps the control's own
	//  width); the search callback is the usual (UIEvent, UIActionBase) one.
	//
	//  Example:
	//      JMSearchRow bar = UIActionManager.CreateSearchRow( parent, "#STR_..._SEARCH", this, "OnSearch", "OnFilterClick", "#STR_..._FILTER_TOOLTIP" );
	//      m_Search = bar.Search;
	//      m_FilterButton = bar.Filter;
	// ---------------------------------------------------------------------------
	static JMSearchRow CreateSearchRow( notnull Widget parent, string placeholder, Class instance, string searchFn, string filterFn = "", string filterTooltip = "", string refreshFn = "", float searchWidth = 0, float filterWidth = 0, float refreshWidth = 0 )
	{
		//! A WrapSpacer packs left to right in creation order, so this IS the
		//! order on screen.
		Widget row = CreateWrapSpacerCompact( parent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		JMSearchRow bar = new JMSearchRow();

		if ( refreshFn != "" )
		{
			bar.Refresh = CreateIconButton( row, JMConstants.Lucide( "refresh-cw" ), instance, refreshFn );
			bar.Refresh.EnableAutoRepeat( true );
			bar.Refresh.SetTooltip( "#STR_COT_GENERIC_REFRESH" );

			if ( refreshWidth > 0 )
				bar.Refresh.SetWidth( refreshWidth );
		}

		bar.Search = CreateSearchBox( row, instance, searchFn, placeholder );

		if ( searchWidth > 0 )
			bar.Search.SetWidth( searchWidth );

		if ( filterFn != "" )
		{
			bar.Filter = CreateIconButton( row, JMConstants.Lucide( "list-filter" ), instance, filterFn );
			bar.Filter.SetTooltip( filterTooltip );

			if ( filterWidth > 0 )
				bar.Filter.SetWidth( filterWidth );
		}

		return bar;
	}

	// ---------------------------------------------------------------------------
	//  CreateSearchFlexRow - the same [refresh] [search box] toolbar as CreateSearchRow,
	//  but as a flex row: the refresh button keeps a fixed pixel size and the search box
	//  takes whatever width is left. Use it when the search box has to end on the same right
	//  edge as full-width controls under it - a fraction width cannot do that, because it is a
	//  fraction of the whole block, so a fixed-size button in front of it always leaves the
	//  field short of the edge.
	//
	//  Pass "" for refreshFn to leave the button out; refreshSize fixes it to a square of that
	//  many pixels (0 keeps the layout's own size). bar.Row is the flex row, for adding more
	//  controls and calling SetGap() again. The result's Filter is always null.
	//
	//  Example:
	//      JMSearchRow bar = UIActionManager.CreateSearchFlexRow( panel, "#STR_COT_GENERIC_SEARCH", this, "OnChange_Search", "OnClick_Refresh", "#STR_COT_GENERIC_REFRESH", 30 );
	//      m_SearchRow = bar.Row;
	//      m_SearchBar = bar.Search;
	// ---------------------------------------------------------------------------
	static JMSearchRow CreateSearchFlexRow( notnull Widget parent, string placeholder, Class instance, string searchFn, string refreshFn = "", string refreshTooltip = "#STR_COT_GENERIC_REFRESH", int refreshSize = 0, int gap = 14 )
	{
		JMSearchRow bar = new JMSearchRow();

		bar.Row = CreateFlexRow( parent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		Widget content = bar.Row.GetContent();

		if ( refreshFn != "" )
		{
			bar.Refresh = CreateRefreshButton( content, instance, refreshFn, refreshTooltip );
			if ( bar.Refresh )
			{
				if ( refreshSize > 0 )
					bar.Refresh.SetFixedSize( refreshSize, refreshSize );

				bar.Row.Add( bar.Refresh );
			}
		}

		bar.Search = CreateSearchBox( content, instance, searchFn, placeholder );
		if ( bar.Search )
		{
			bar.Search.SetFlex( 1.0, 60 );
			bar.Row.Add( bar.Search );
		}

		bar.Row.SetGap( gap );

		return bar;
	}

	// ---------------------------------------------------------------------------
	//  CreateOverlayMenu / CreateOverlayPrompt - a context menu or value prompt
	//  for a form, anchored to the form's window and registered as one of its
	//  overlays in a single call. A popup anchors to the window root, not to the
	//  tab it was opened from, so an unregistered one floats over the next tab;
	//  registering is what makes tab changes and hiding the form dismiss it.
	//
	//  CreateValuePrompt already builds the prompt's panel - never call
	//  InitPrompt() on the result.
	//
	//  Example:
	//      m_Menu = UIActionManager.CreateOverlayMenu( this, this, "OnPick" );
	//      m_Menu.AddItem( "heal", "Heal", JMConstants.Lucide( "heart-pulse" ) );
	//      m_Menu.OpenAtMouse();
	// ---------------------------------------------------------------------------
	static UIActionContextMenu CreateOverlayMenu( notnull JMFormBase form, Class instance = null, string funcname = "" )
	{
		Widget root = form.GetLayoutRoot();
		CF_Window wnd = form.GetWindow();

		if ( !root || !wnd )
			return null;

		UIActionContextMenu menu = CreateContextMenu( root, wnd.GetWidgetRoot(), instance, funcname );
		form.AddOverlay( menu );

		return menu;
	}

	// ---------------------------------------------------------------------------
	//  CreateOverlayFilterMenu - the same for a UIActionFilterMenu: created under the form,
	//  anchored to its window, registered as an overlay, opening under `owner` (the filter
	//  button's layout root) and serving the JMFilterRegistry `registryScope` ("" = none). `anchor`
	//  overrides what the panel hangs off (default: the window root).
	//  Add pages, then call ToggleAt( owner ) from the button's handler.
	// ---------------------------------------------------------------------------
	static UIActionFilterMenu CreateOverlayFilterMenu( notnull JMFormBase form, notnull Widget owner, string registryScope = "", Widget anchor = null )
	{
		Widget root = form.GetLayoutRoot();
		CF_Window wnd = form.GetWindow();

		if ( !root || !wnd )
			return null;

		//! The window root unless the caller has a better place for the panel to hang off.
		Widget panelAnchor = anchor;
		if ( !panelAnchor )
			panelAnchor = wnd.GetWidgetRoot();

		UIActionFilterMenu menu = CreateFilterMenu( root, panelAnchor );
		if ( !menu )
			return null;

		form.AddOverlay( menu.GetInnerMenu() );
		menu.SetOwnerWidget( owner );
		menu.SetRegistryScope( registryScope );

		return menu;
	}

	static UIActionValuePrompt CreateOverlayPrompt( notnull JMFormBase form, Class instance = null, string funcname = "" )
	{
		Widget root = form.GetLayoutRoot();
		CF_Window wnd = form.GetWindow();

		if ( !root || !wnd )
			return null;

		UIActionValuePrompt prompt = CreateValuePrompt( root, wnd.GetWidgetRoot(), instance, funcname );
		form.AddOverlay( prompt );

		return prompt;
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
	//  CreateTabStrip - an empty row of tab buttons. Add tabs one at a time with
	//  action.AddTab( label, icon, panel ) and keep the id it returns; the panels
	//  are shown and hidden with their tab. See UIActionTabs.c.
	// ---------------------------------------------------------------------------
	static UIActionTabs CreateTabStrip( notnull Widget parent, Class instance = null, string funcname = "" )
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
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return null;
	}

	//! DEPRECATED - use CreateTabStrip and AddTab( label, icon, panel ) for each tab.
	static UIActionTabs CreateTabs( notnull Widget parent, notnull array<string> labels, Class instance = null, string funcname = "" )
	{
		JMDeprecated.WarnOnce( null, "UIActionManager.CreateTabs() is deprecated. Please use CreateTabStrip() and AddTab( label, icon, panel )." );

		UIActionTabs action = CreateTabStrip( parent, instance, funcname );
		if ( !action )
			return null;

		foreach ( string label : labels )
			action.AddTab( label );

		return action;
	}

	//! DEPRECATED - use CreateTabStrip and AddTab( label, icon, panel ) for each tab.
	static UIActionTabs CreateTabs( notnull Widget parent, notnull array<string> labels, notnull array<string> icons, Class instance = null, string funcname = "" )
	{
		JMDeprecated.WarnOnce( null, "UIActionManager.CreateTabs() is deprecated. Please use CreateTabStrip() and AddTab( label, icon, panel )." );

		UIActionTabs action = CreateTabStrip( parent, instance, funcname );
		if ( !action )
			return null;

		foreach ( int i, string label : labels )
		{
			string icon = "";
			if ( i < icons.Count() )
				icon = icons[i];

			action.AddTab( label, icon );
		}

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
	// ---------------------------------------------------------------------------
	//  CreateItemList - themed, virtualised single-column list.
	//
	//  Only the rows that fit on screen exist; scrolling moves the data through
	//  them. Use it wherever a set is too large to give a widget each - the
	//  object spawner's ~14000 classes, for one.
	//
	//  The list cannot measure itself on the frame it is built, so tell it how
	//  tall its viewport is with SetViewportHeight() from wherever that height
	//  is decided.
	// ---------------------------------------------------------------------------
	static UIActionItemList CreateItemList( notnull Widget parent, Class instance = null, string funcname = "" )
	{
		string layout = "JM/COT/GUI/layouts/uiactions/UIActionItemList.layout";
		Widget widget = g_Game.GetWorkspace().CreateWidgets( layout, parent );

		if ( !CheckWidget( widget, parent, layout, "UIActionItemList" ) )
			return null;

		UIActionItemList action;
		widget.GetScript( action );

		if ( action )
		{
			action.SetCallback( instance, funcname );
			return action;
		}

		UIAMError( "Couldn't get script", widget, parent );
		return NULL;
	}

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
	// ---------------------------------------------------------------------------
	//  CreateDeleteConfirmIcon - the icon-only "trash, click twice" button used in
	//  row lists: trash glyph, danger fill, compact O / X confirm labels (the
	//  full words clip in a square button), ICON_BUTTON_PX square. Set the
	//  tooltip and SetData on the result as usual.
	// ---------------------------------------------------------------------------
	static UIActionConfirmInline CreateDeleteConfirmIcon( notnull Widget parent, Class instance, string funcname )
	{
		UIActionConfirmInline btn = CreateConfirmInline( parent, "", instance, funcname );
		if ( !btn )
			return null;

		UIActionIconGrid.ApplyDeletePreset( btn );
		btn.SetButton( "" );
		btn.SetFixedSize( JMFormBase.ICON_BUTTON_PX, JMFormBase.ICON_BUTTON_PX );
		btn.CenterIcon( JMFormBase.ICON_BUTTON_PX, 16 );
		btn.SetConfirmLabel( "O" );
		btn.SetCancelLabel( "X" );

		return btn;
	}

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

	// ---------------------------------------------------------------------------
	//  CreateHeaderButton - compact header/toolbar icon button helper
	// ---------------------------------------------------------------------------
	static UIActionImageButton CreateHeaderButton( notnull Widget parent, string icon, string tooltip = "", Class target = null, string callback = "", string permission = "" )
	{
		UIActionImageButton btn = CreateImageButton( parent, icon, target, callback );
		if ( btn )
		{
			if ( tooltip != "" )
				btn.SetTooltip( tooltip );

			if ( permission != "" )
				btn.UpdatePermission( permission );
		}
		return btn;
	}

	// ---------------------------------------------------------------------------
	//  COT Framework Components (Items 2, 3, 4, 5)
	// ---------------------------------------------------------------------------

	static COTFilteredListController CreateFilteredListController( Class callbackInstance = null, string callbackFunc = "" )
	{
		COTFilteredListController controller = new COTFilteredListController();
		if ( callbackInstance && callbackFunc != "" )
			controller.SetCallback( callbackInstance, callbackFunc );
		return controller;
	}

	static UIActionSliderSync CreateSliderSync( UIActionSlider slider, UIActionEditableText editBox, float min = 0.0, float max = 1.0, float step = 0.01, Class callbackInstance = null, string callbackFunc = "" )
	{
		UIActionSliderSync sync = new UIActionSliderSync( slider, editBox, min, max, step );
		if ( callbackInstance && callbackFunc != "" )
			sync.SetCallback( callbackInstance, callbackFunc );
		return sync;
	}

	static UIActionPlayerPicker CreatePlayerPicker( Class callbackInstance = null, string callbackFunc = "" )
	{
		UIActionPlayerPicker picker = new UIActionPlayerPicker();
		if ( callbackInstance && callbackFunc != "" )
			picker.SetCallback( callbackInstance, callbackFunc );
		return picker;
	}

	static UIActionMapController CreateMapController( UIActionMap actionMap, Class callbackInstance = null, string callbackFunc = "" )
	{
		UIActionMapController controller = new UIActionMapController( actionMap );
		if ( callbackInstance && callbackFunc != "" )
			controller.SetCallback( callbackInstance, callbackFunc );
		return controller;
	}
}


