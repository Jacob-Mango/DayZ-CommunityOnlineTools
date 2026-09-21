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
	protected JMESPFormTabFilters m_Tab;

	//! Two independent reasons a row can be off screen, kept apart so clearing
	//! the search does not resurrect a row the admin has no permission for.
	protected bool m_Filtered;
	protected bool m_Permitted;

	void JMESPViewTypeWidget()
	{
		m_Permitted = true;
	}

	Widget GetLayoutRoot()
	{
		return m_Row;
	}

	JMESPViewType GetViewType()
	{
		return m_Type;
	}

	bool IsChecked()
	{
		if ( !m_Switch )
			return false;

		return m_Switch.IsChecked();
	}

	bool IsFiltered()
	{
		return m_Filtered;
	}

	bool IsPermitted()
	{
		return m_Permitted;
	}

	void SetChecked( bool state )
	{
		if ( !m_Switch || !m_Type )
			return;

		m_Switch.SetChecked( state );
		m_Type.View = state;

		JMScriptInvokers.ESP_VIEWTYPE_CHANGED.Invoke( m_Type );
	}

	void SetFiltered( bool filtered )
	{
		m_Filtered = filtered;
		ApplyVisibility();
	}

	void Build( notnull Widget parent, JMESPViewType type, JMESPFormTabFilters tab )
	{
		m_Type = type;
		m_Tab = tab;

		m_Row = UIActionManager.CreateWrapSpacer( parent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		Widget rowInset = UIActionManager.CreateSpacer( m_Row );
		UIActionManager.SetFixedSize( rowInset, ROW_INSET_PX, GUTTER_PX );

		m_PaletteButton = UIActionManager.CreateIconButton( m_Row, JMConstants.Lucide( "palette" ), this, "" );
		if ( m_PaletteButton ) m_PaletteButton.SetOnClick( this, "OnClick_Palette" );
		m_PaletteButton.SetFixedSize( GUTTER_PX, GUTTER_PX );
		m_PaletteButton.SetTooltip( "#STR_COT_ESP_MODULE_CATEGORY_COLOUR" );

		//! Empty label - the switch here is only the track.
		m_Switch = UIActionManager.CreateToggleSwitch( m_Row, "", this, "OnClick_Switch", type.View );
		m_Switch.SetFixedSize( SWITCH_PX, GUTTER_PX );
		m_Switch.SetThumbIcon( IconForViewType( type ) );

		//! The name is its own flat button so clicking it toggles the row too,
		//! and so it lights up under the cursor like the rest of the list.
		m_LabelButton = UIActionManager.CreateButton( m_Row, type.Localisation, this, "" );
		if ( m_LabelButton ) m_LabelButton.SetOnClick( this, "OnClick_Label" );
		m_LabelButton.SetFlat( true );
		m_LabelButton.SetWidth( LABEL_W );

		RefreshSwatch();
		ApplyVisibility();
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

	// -------------------------------------------------------------------------
	//  Handlers
	// -------------------------------------------------------------------------

	void OnClick_Switch( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK_RIGHT )
		{
			if ( m_Tab )
				m_Tab.OnRightClick_CategoryRow( this );

			return;
		}

		if ( eid != UIEvent.CLICK )
			return;

		//! The switch has already flipped itself; this only mirrors the new
		//! state onto the view type.
		m_Type.View = m_Switch.IsChecked();

		JMScriptInvokers.ESP_VIEWTYPE_CHANGED.Invoke( m_Type );

		if ( m_Tab )
			m_Tab.OnChange_CategoryRow();
	}

	void OnClick_Label( UIActionBase action )
	{
		SetChecked( !IsChecked() );

		if ( m_Tab )
			m_Tab.OnChange_CategoryRow();
	}

	//! The form owns one picker for the whole list and hangs its ARGB popup off
	//! whichever palette button asked for it.
	void OnClick_Palette( UIActionBase action )
	{
		OpenColourEditor();
	}

	void OpenColourEditor()
	{
		if ( !m_Tab || !m_PaletteButton )
			return;

		m_Tab.OpenCategoryColourPopup( this, m_PaletteButton.GetLayoutRoot() );
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

	//! Not named UpdatePermission() - see the note on
	//! COT_ScriptedWidgetEventHandler for why the polymorphic one-argument
	//! version carries its own name.
	override void COT_ApplyPermission( string permission )
	{
		m_Permitted = JMPermissions.Has( permission );

		if ( m_Type )
			m_Type.HasPermission = m_Permitted;

		ApplyVisibility();
	}

	//! A category the admin cannot use is hidden outright rather than dimmed:
	//! the list is long enough already, and a disabled row here carries no
	//! information the permission editor does not state better.
	protected void ApplyVisibility()
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

	//! Glyph baked into a category row's switch thumb. Keyed on typename the
	//! same way GroupIndexFor is, and for the same reason: a sub-mod's own view
	//! type falls through to the generic marker instead of rendering blank.
	//!
	//! Most-specific first - every weapon and item leaf also passes the base
	//! test, so the bases have to be asked last.
	static string IconForViewType( JMESPViewType viewType )
	{
		typename t = viewType.Type();

		if ( t == JMESPViewTypePlayer )   return JMConstants.Lucide( "user" );
		if ( t == JMESPViewTypePlayerAI ) return JMConstants.Lucide( "bot" );
		if ( t == JMESPViewTypeInfected ) return JMConstants.Lucide( "skull" );
		if ( t == JMESPViewTypeAnimal )   return JMConstants.Lucide( "rabbit" );

		if ( t == JMESPViewTypeCar )   return JMConstants.Lucide( "car" );
		if ( t == JMESPViewTypeBoat )  return JMConstants.Lucide( "ship" );
		if ( t == JMESPViewTypeTrain ) return JMConstants.Lucide( "train-front" );
	#ifndef DAYZ_1_29
		if ( t == JMESPViewTypeMotorbike ) return JMConstants.Lucide( "bike" );
	#endif

		if ( t == JMESPViewTypeArchery )         return JMConstants.Lucide( "bow-arrow" );
		if ( t == JMESPViewTypePistol )          return JMConstants.Lucide( "target" );
		if ( t == JMESPViewTypeLauncher )        return JMConstants.Lucide( "rocket" );
		if ( t == JMESPViewTypeRifle )           return JMConstants.Lucide( "crosshair" );
		if ( t == JMESPViewTypeBoltRifle )       return JMConstants.Lucide( "crosshair" );
		if ( t == JMESPViewTypeBoltActionRifle ) return JMConstants.Lucide( "crosshair" );

		if ( t == JMESPViewTypeTent )         return JMConstants.Lucide( "tent" );
		if ( t == JMESPViewTypeBaseBuilding ) return JMConstants.Lucide( "blocks" );
		if ( t == JMESPViewTypeFood )         return JMConstants.Lucide( "apple" );
		if ( t == JMESPViewTypeExplosive )    return JMConstants.Lucide( "bomb" );
		if ( t == JMESPViewTypeBook )         return JMConstants.Lucide( "book" );
		if ( t == JMESPViewTypeContainer )    return JMConstants.Lucide( "box" );
		if ( t == JMESPViewTypeTransmitter )  return JMConstants.Lucide( "radio" );
		if ( t == JMESPViewTypeClothing )     return JMConstants.Lucide( "shirt" );
		if ( t == JMESPViewTypeMagazine )     return JMConstants.Lucide( "layers" );
		if ( t == JMESPViewTypeAmmo )         return JMConstants.Lucide( "shell" );
		if ( t == JMESPViewTypeUnknown )      return JMConstants.Lucide( "circle-help" );

		if ( t == JMESPViewTypeBuilding )    return JMConstants.Lucide( "building" );
		if ( t == JMESPViewTypeRock )        return JMConstants.Lucide( "gem" );
		if ( t == JMESPViewTypeTree )        return JMConstants.Lucide( "tree-pine" );
		if ( t == JMESPViewTypeBush )        return JMConstants.Lucide( "shrub" );
		if ( t == JMESPViewTypePlainObject ) return JMConstants.Lucide( "square" );

		if ( t.IsInherited( JMESPViewTypeWeapon ) )    return JMConstants.Lucide( "swords" );
		if ( t.IsInherited( JMESPViewTypeItemBase ) )  return JMConstants.Lucide( "package" );
		if ( t.IsInherited( JMESPViewTypeImmovable ) ) return JMConstants.Lucide( "mountain" );

		return JMConstants.Lucide( "map-pin" );
	}
}
