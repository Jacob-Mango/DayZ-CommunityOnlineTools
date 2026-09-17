//! One row of the ESP category filter list.
//!
//!     [palette] (=O) Players
//!
//! Laid out in a WrapSpacer, not a GridSpacer: a GridSpacer splits its width
//! into equal columns and ignores a child's own SetWidth, which is what threw
//! the palette button out to fill half the row. A WrapSpacer honours both
//! SetFixedSize on the icon button and a SetWidth fraction on the label, the
//! same combination this module's original toolbar rows used.
//!
//! The row is built from stock UIAction controls rather than from a layout of
//! its own. The hand-rolled esp_type_widget.layout it replaces put a checkbox,
//! a label and a swatch in a Size-To-Content-H grid whose fractional middle
//! column ate the whole width, so the swatch column collapsed to nothing and
//! could never be clicked. UIActionToggleSwitch and UIActionImageButton already
//! own their hover and press feedback, which the raw CheckBoxWidget did not.
//!
//! The category glyph is baked into the switch thumb and tinted with that
//! category's overlay colour, so the switch carries the on/off state and the
//! colour at a glance.
//!
//! Derives from COT_ScriptedWidgetEventHandler so JMFormBase.RegisterPermission
//! can hold it - the registry is typed to that class, and ESP registers one
//! "ESP.View.<Permission>" key per view type.
class JMESPViewTypeWidget: COT_ScriptedWidgetEventHandler
{
	//! Side of the square icon button in the row's left gutter. Group headers
	//! use the same figure for their chevron so the two line up down the list.
	static const int GUTTER_PX = 26;

	//! Breathing room before the gutter. Without it the palette buttons and the
	//! switch column sit hard against the scroller's left edge, which reads as
	//! a clipped list rather than an indented one. Group headers pad by the
	//! same figure so both stay in one column.
	static const int ROW_INSET_PX = 10;

	//! Just the switch track plus its inset - the name lives in its own label
	//! beside it, so the switch needs no room for one of its own.
	static const int SWITCH_PX = 52;

	//! Fraction of the row left for the name. Deliberately short of 1.0: the
	//! two fixed cells ahead of it are already spent, and a WrapSpacer wraps
	//! rather than shrinks when a line overflows.
	static const float LABEL_W = 0.72;

	protected Widget m_Row;

	protected UIActionImageButton m_PaletteButton;
	protected UIActionToggleSwitch m_Switch;
	protected UIActionButton m_LabelButton;

	protected JMESPViewType m_Type;
	protected JMESPForm m_Form;

	//! Two independent reasons a row can be off screen, kept apart so clearing
	//! the search does not resurrect a row the admin has no permission for.
	protected bool m_Filtered;
	protected bool m_Permitted;

	void JMESPViewTypeWidget()
	{
		m_Permitted = true;
	}

	void Build( notnull Widget parent, JMESPViewType type, JMESPForm form )
	{
		m_Type = type;
		m_Form = form;

		m_Row = UIActionManager.CreateWrapSpacer( parent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		Widget rowInset = UIActionManager.CreateSpacer( m_Row );
		UIActionManager.SetFixedSize( rowInset, ROW_INSET_PX, GUTTER_PX );

		m_PaletteButton = UIActionManager.CreateIconButton( m_Row, JMConstants.Lucide( "palette" ), this, "OnClick_Palette" );
		m_PaletteButton.SetFixedSize( GUTTER_PX, GUTTER_PX );
		m_PaletteButton.SetTooltip( "#STR_COT_ESP_MODULE_CATEGORY_COLOUR" );

		//! Empty label - the switch here is only the track.
		m_Switch = UIActionManager.CreateToggleSwitch( m_Row, "", this, "OnClick_Switch", type.View );
		m_Switch.SetFixedSize( SWITCH_PX, GUTTER_PX );
		m_Switch.SetThumbIcon( JMESPForm.IconForViewType( type ) );

		//! The name is its own flat button so clicking it toggles the row too,
		//! and so it lights up under the cursor like the rest of the list.
		m_LabelButton = UIActionManager.CreateButton( m_Row, type.Localisation, this, "OnClick_Label" );
		m_LabelButton.SetFlat( true );
		m_LabelButton.SetWidth( LABEL_W );

		RefreshSwatch();
		ApplyVisibility();
	}

	Widget GetLayoutRoot()
	{
		return m_Row;
	}

	JMESPViewType GetViewType()
	{
		return m_Type;
	}

	//! Repaint the switch thumb from the view type's current colour. The thumb
	//! only shows the colour while the row is ON - an off category draws
	//! nothing, so advertising its colour would be a lie.
	void RefreshSwatch()
	{
		if ( !m_Switch || !m_Type )
			return;

		m_Switch.SetThumbColors( m_Type.Colour, UIActionToggleSwitch.COLOR_THUMB_OFF );
	}

	void SetChecked( bool state )
	{
		if ( !m_Switch || !m_Type )
			return;

		m_Switch.SetChecked( state );
		m_Type.View = state;

		JMScriptInvokers.ESP_VIEWTYPE_CHANGED.Invoke( m_Type );
	}

	bool IsChecked()
	{
		if ( !m_Switch )
			return false;

		return m_Switch.IsChecked();
	}

	// -------------------------------------------------------------------------
	//  Handlers
	// -------------------------------------------------------------------------

	void OnClick_Switch( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK_RIGHT )
		{
			if ( m_Form )
				m_Form.OnRightClick_CategoryRow( this );

			return;
		}

		if ( eid != UIEvent.CLICK )
			return;

		//! The switch has already flipped itself; this only mirrors the new
		//! state onto the view type.
		m_Type.View = m_Switch.IsChecked();

		JMScriptInvokers.ESP_VIEWTYPE_CHANGED.Invoke( m_Type );

		if ( m_Form )
			m_Form.OnChange_CategoryRow();
	}

	void OnClick_Label( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		SetChecked( !IsChecked() );

		if ( m_Form )
			m_Form.OnChange_CategoryRow();
	}

	//! The form owns one picker for the whole list and hangs its ARGB popup off
	//! whichever palette button asked for it.
	void OnClick_Palette( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		OpenColourEditor();
	}

	void OpenColourEditor()
	{
		if ( !m_Form || !m_PaletteButton )
			return;

		m_Form.OpenCategoryColourPopup( this, m_PaletteButton.GetLayoutRoot() );
	}

	// -------------------------------------------------------------------------
	//  Search filtering & permission
	// -------------------------------------------------------------------------

	//! `needle` is already lower-cased by the caller - it is the same string for
	//! every row, so it is folded once per keystroke rather than once per row.
	bool MatchesFilter( string needle )
	{
		if ( needle == "" )
			return true;

		if ( !m_Type )
			return false;

		string label = Widget.TranslateString( m_Type.Localisation );
		label.ToLower();

		if ( label.IndexOf( needle ) >= 0 )
			return true;

		string permission = m_Type.Permission;
		permission.ToLower();

		return permission.IndexOf( needle ) >= 0;
	}

	void SetFiltered( bool filtered )
	{
		m_Filtered = filtered;
		ApplyVisibility();
	}

	bool IsFiltered()
	{
		return m_Filtered;
	}

	bool IsPermitted()
	{
		return m_Permitted;
	}

	//! Not named UpdatePermission() - see the note on
	//! COT_ScriptedWidgetEventHandler for why the polymorphic one-argument
	//! version carries its own name.
	override void COT_ApplyPermission( string permission )
	{
		m_Permitted = GetPermissionsManager().HasPermission( permission );

		if ( m_Type )
			m_Type.HasPermission = m_Permitted;

		ApplyVisibility();
	}

	//! A category the admin cannot use is hidden outright rather than dimmed:
	//! the list is long enough already, and a disabled row here carries no
	//! information the permission editor does not state better.
	private void ApplyVisibility()
	{
		bool visible = m_Type && !m_Filtered && m_Permitted;

		if ( m_PaletteButton )
			m_PaletteButton.SetVisible( visible );

		if ( m_Switch )
			m_Switch.SetVisible( visible );

		if ( m_LabelButton )
			m_LabelButton.SetVisible( visible );

		if ( m_Row )
			m_Row.Show( visible );
	}
}
