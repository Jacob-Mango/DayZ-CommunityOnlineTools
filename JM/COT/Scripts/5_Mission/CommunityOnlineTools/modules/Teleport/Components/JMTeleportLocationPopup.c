//! The save / edit location popup of the Teleport form: name, category (existing or new),
//! and the position it captures. The form opens it with ShowPopup() from the coordinates
//! card, the row menu and the map menu.
class JMTeleportLocationPopup
{
	protected JMTeleportForm m_Form;

	//! Picker row that turns the category control into a text box. Matched by
	//! index, not by text, so a category legitimately named after this row's
	//! (localized) label would still pick its own row.
	static const string CATEGORY_NEW_ROW = "#STR_COT_TELEPORT_MODULE_NEW_CATEGORY_ROW";

	// ---- Save / edit popup -------------------------------------------------
	protected Widget                 m_PopupShade;
	protected Widget                 m_PopupBody;
	protected UIActionCard           m_PopupCard;
	protected UIActionEditableText   m_PopupName;
	protected UIActionText           m_PopupPosition;
	protected UIActionButton         m_PopupConfirm;

	//! The category control is two controls, one showing at a time: a picker
	//! over the categories that exist, and a text box for a category that does
	//! not exist yet. m_PopupNewIndex is the picker row that swaps between
	//! them, and m_PopupNewCategory is which of the two is currently showing.
	protected UIActionDropdown       m_PopupCategory;
	protected UIActionEditableText   m_PopupCategoryNew;
	protected UIActionImageButton    m_PopupCategoryBack;
	protected int                    m_PopupNewIndex;
	protected bool                   m_PopupNewCategory;

	//! Position the popup will write. Captured when it opens so that panning
	//! the map behind it cannot move the thing being saved.
	protected vector m_PopupWorldPos;

	//! Name of the location the popup is editing, or "" when it is saving a new
	//! one. This is what makes it one popup rather than two.
	protected string m_PopupEditing;

	void JMTeleportLocationPopup( JMTeleportForm form )
	{
		m_Form = form;
	}

	//! Show one of the two category controls and hide the other.
	protected void SetCategoryMode( bool newCategory )
	{
		m_PopupNewCategory = newCategory;

		if ( m_PopupCategory )
			m_PopupCategory.SetVisible( !newCategory );

		if ( m_PopupCategoryNew )
			m_PopupCategoryNew.SetVisible( newCategory );

		if ( m_PopupCategoryBack )
			m_PopupCategoryBack.SetVisible( newCategory );

		if ( m_PopupBody )
			m_PopupBody.Update();
	}

	// =========================================================================
	//  Save / edit popup
	//
	//  One popup for both jobs. Saving and editing ask for the same two things,
	//  and the only difference is whether there is an existing location to write
	//  back to - which is what m_PopupEditing records.
	// =========================================================================

	protected void InitPopup()
	{
		if ( !m_PopupBody )
			return;

		m_PopupCard = UIActionManager.CreateCard( m_PopupBody, "#STR_COT_TELEPORT_MODULE_SAVE_LOCATION" );

		Widget body = UIActionManager.CreateGridSpacer( m_PopupCard.GetContent(), 4, 1 );

		m_PopupPosition = UIActionManager.CreateLabeledValue( body, "#STR_COT_GENERIC_POSITION", "" );

		//! Both rows are built the same way on purpose. CreateLabeledInput
		//! splits its row 50/50, which puts one box at the far right and the
		//! one below it a quarter in.
		Widget nameRow = UIActionManager.CreateWrapSpacer( body, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		UIActionText nameLabel = UIActionManager.CreateText( nameRow, "#STR_COT_GENERIC_NAME" );
		nameLabel.SetWidth( 0.24 );
		nameLabel.SetLabelVAlign( UIActionVAlign.CENTER );

		m_PopupName = UIActionManager.CreateEditableText( nameRow, "", this );
		m_PopupName.SetWidth( 0.74 );
		m_PopupName.SetWidgetWidth( m_PopupName.GetLabelWidget(), 0.0 );
		m_PopupName.SetWidgetWidth( m_PopupName.GetEditBoxWidget(), 1.0 );

		Widget catRow = UIActionManager.CreateWrapSpacer( body, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		UIActionText catLabel = UIActionManager.CreateText( catRow, "#STR_COT_GENERIC_CATEGORY" );
		catLabel.SetWidth( 0.24 );
		catLabel.SetLabelVAlign( UIActionVAlign.CENTER );

		//! The list is anchored to the shade, not to the window root. The shade
		//! is the popup's own layer and draws above the form; a list anchored
		//! any lower would open behind the card it belongs to.
		m_PopupCategory = UIActionManager.CreateDropdown( catRow, "", m_PopupShade, this, "PopupCategory_OnChange" );
		m_PopupCategory.SetWidth( 0.74 );

		//! The dropdown layout keeps the left 30% for its own label, which this
		//! one does not use - the row already has a label of its own. Pull the
		//! field over that gap so the control fills what it was given.
		Widget catField = m_PopupCategory.GetLayoutRoot().FindAnyWidget( "action_field" );
		if ( catField )
		{
			m_PopupCategory.SetWidgetPosition( catField, 0.0 );
			m_PopupCategory.SetWidgetWidth( catField, 1.0 );
		}

		m_Form.AddOverlay( m_PopupCategory );

		m_PopupCategoryNew = UIActionManager.CreateEditableText( catRow, "", this );
		m_PopupCategoryNew.SetWidth( 0.60 );
		m_PopupCategoryNew.SetWidgetWidth( m_PopupCategoryNew.GetLabelWidget(), 0.0 );
		m_PopupCategoryNew.SetWidgetWidth( m_PopupCategoryNew.GetEditBoxWidget(), 1.0 );
		m_PopupCategoryNew.SetTooltip( "#STR_COT_TELEPORT_MODULE_NEW_CATEGORY_NAME_TOOLTIP" );

		m_PopupCategoryBack = UIActionManager.CreateIconButton( catRow, JMConstants.Lucide( "list" ), this, "" );
		if ( m_PopupCategoryBack ) m_PopupCategoryBack.SetOnClick( this, "Click_PopupCategoryBack" );
		m_PopupCategoryBack.SetFixedSize( JMTeleportForm.HEADER_BTN_PX, JMTeleportForm.HEADER_BTN_PX );
		m_PopupCategoryBack.SetTooltip( "#STR_COT_TELEPORT_MODULE_PICK_EXISTING_CATEGORY_TOOLTIP" );

		SetCategoryMode( false );

		UIActionButton cancel;
		UIActionManager.CreateButtonPair( body, "#STR_COT_GENERIC_CANCEL", this, "Click_PopupCancel", cancel, "#STR_COT_GENERIC_SAVE", this, "Click_PopupConfirm", m_PopupConfirm );

		m_PopupConfirm.SetColor( JMTheme.SUCCESS_FILL );

		HidePopup();
	}

	//! Open the popup over whatever the coordinate fields hold.
	void Click_OpenSavePopup( UIActionBase action )
	{
		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_CREATE ) )
			return;

		ShowPopup( "", m_Form.GetInputCoords().GetValue(), "", "" );
	}

	void ShowPopup( string editing, vector position, string name, string category )
	{
		if ( !m_PopupShade )
			return;

		m_PopupEditing  = editing;
		m_PopupWorldPos = position;

		if ( m_PopupCard )
		{
			//! SetLabel, not SetTitle - the card's header text is its label.
			if ( editing == "" )
				m_PopupCard.SetLabel( "#STR_COT_TELEPORT_MODULE_SAVE_LOCATION" );
			else
				m_PopupCard.SetLabel( COT_String.TranslateEx("#STR_COT_TELEPORT_MODULE_EDIT_LOCATION", editing) );
		}

		//! The position row is read-only text, and greyed while editing: the
		//! edit path renames a location, it does not move it.
		if ( m_PopupPosition )
			m_PopupPosition.SetText( m_Form.FormatVector( position ) );

		m_PopupName.SetText( name );

		RebuildPopupCategories( category );

		m_PopupShade.Show( true );
	}

	void HidePopup()
	{
		if ( m_PopupShade )
			m_PopupShade.Show( false );

		m_PopupEditing = "";
	}

	void Click_PopupCancel( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		HidePopup();
	}

	//! Fill the picker with the categories that exist plus the row that swaps
	//! it for a text box, and select `select`.
	//!
	//! Rebuilt on every open rather than once: a category this popup creates
	//! has to be pickable the next time it opens.
	protected void RebuildPopupCategories( string select )
	{
		int i;
		int added       = 0;
		int selectIndex = -1;

		if ( !m_PopupCategory )
			return;

		m_PopupCategory.ClearEntries();

		if ( m_Form.GetCategories() )
		{
			for ( i = 0; i < m_Form.GetCategories().Count(); i++ )
			{
				if ( m_Form.GetCategories()[i] == JMTeleportForm.CATEGORY_ALL )
					continue;

				if ( m_Form.GetCategories()[i] == select )
					selectIndex = added;

				m_PopupCategory.AddEntry( m_Form.GetCategories()[i] );

				added++;
			}
		}

		m_PopupNewIndex = added;
		m_PopupCategory.AddEntry( CATEGORY_NEW_ROW, JMConstants.Lucide( "plus" ), JMTheme.ACCENT );

		//! Open straight in the text box when there is nothing to pick, or when
		//! the category being edited is not one of the rows - landing on a row
		//! that says the wrong thing would silently recategorise the location
		//! on save.
		if ( added == 0 || ( select != "" && selectIndex < 0 ) )
		{
			SetCategoryMode( true );
			m_PopupCategoryNew.SetText( select );
			return;
		}

		SetCategoryMode( false );
		m_PopupCategoryNew.SetText( "" );

		if ( selectIndex < 0 )
			selectIndex = 0;

		m_PopupCategory.SetSelection( selectIndex, false );
	}

	//! Whichever of the two category controls is showing.
	protected string PopupCategoryText()
	{
		if ( m_PopupNewCategory )
			return m_PopupCategoryNew.GetText();

		if ( !m_PopupCategory )
			return "";

		if ( m_PopupCategory.GetSelection() == m_PopupNewIndex )
			return "";

		return m_PopupCategory.GetSelectedText();
	}

	void Click_PopupCategoryBack( UIActionBase action )
	{
		SetCategoryMode( false );

		//! Step off the "new" row on the way back. SetSelection ignores a
		//! selection that is already current, so leaving it there would make
		//! the second visit to the text box impossible.
		if ( m_PopupCategory && m_PopupNewIndex > 0 )
			m_PopupCategory.SetSelection( 0, false );
	}

	void Click_PopupConfirm( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		string name = m_PopupName.GetText();

		if ( name == "" )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "#STR_COT_TELEPORT_MODULE_TELEPORT_ERROR_NONAME" ) );
			return;
		}

		string category = PopupCategoryText();

		if ( category == "" )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "#STR_COT_TELEPORT_MODULE_PICK_OR_NAME_CATEGORY" ) );
			return;
		}

		if ( m_PopupEditing == "" )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_CREATE ) )
				return;

			m_Form.GetModule().AddLocation( name, category, m_PopupWorldPos );

			COTCreateLocalAdminNotification( new StringLocaliser( COT_String.TranslateEx( "#STR_COT_TELEPORT_MODULE_ADDED_LOCATION", name, category ) ) );
		}
		else
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_EDIT ) )
				return;

			JMTeleportLocation target = m_Form.FindLocation( m_PopupEditing );

			if ( !target )
			{
				HidePopup();
				return;
			}

			m_Form.GetModule().EditLocation( target, name, category );

			COTCreateLocalAdminNotification( new StringLocaliser( COT_String.TranslateEx( "#STR_COT_TELEPORT_MODULE_EDITED_LOCATION", m_PopupEditing ) ) );

			//! Follow the rename, so the row that was selected is still the
			//! selected row once the reload lands.
			m_Form.GetSelectedName() = name;
		}

		HidePopup();

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION ) )
			return;

		m_Form.GetModule().Reload();
	}

	//! The picker only has to notice one row: the one that is not a category.
	void PopupCategory_OnChange( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || !m_PopupCategory )
			return;

		if ( m_PopupCategory.GetSelection() != m_PopupNewIndex )
			return;

		SetCategoryMode( true );

		m_PopupCategoryNew.SetText( "" );
	}

	//! Look the popup's shade and body up in the form's layout and build its card.
	void Build()
	{
		Widget root = m_Form.GetLayoutRoot();

		m_PopupShade = root.FindAnyWidget( "tp_popup_shade" );
		m_PopupBody  = root.FindAnyWidget( "tp_popup_body" );

		InitPopup();
	}
}
