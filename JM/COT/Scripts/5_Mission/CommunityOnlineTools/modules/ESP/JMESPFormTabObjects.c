//! "Visible" (Objects) tab of JMESPForm - the tracked-object list, bulk
//! select/deselect/delete/move/export actions, and the row context menu.
//! Back-reference to the owning form, same shape as JMPlayerRowWidget.Menu.
class JMESPFormTabObjects
{
	protected JMESPForm m_Form;

	//! The object list is sized to its CONTENT between these two bounds, not to
	//! whatever height is left over.
	static const int OBJECT_LIST_MIN_ROWS = 4;
	static const int OBJECT_LIST_MAX_ROWS = 12;

	//! Vertical space the rest of the tab needs, so a short window shrinks the
	//! list rather than hiding the card under it.
	static const int OBJECT_LIST_CHROME = 260;

	protected UIActionScroller m_ESPSelectedObjects;
	protected UIActionItemList m_ObjectList;
	protected UIActionSearchBox m_ObjectSearch;
	protected UIActionCard m_ObjectsCard;
	protected UIActionImageButton m_ObjectListLockButton;

	//! Manual hold, set from the lock button. Independent of the automatic hold
	//! that focus applies, so releasing the mouse does not silently unpin a
	//! list the admin pinned on purpose.
	protected bool m_ObjectListPinned;

	//! What the lock button is currently drawing, so the icon is only swapped
	//! when the state actually changes rather than twice a second.
	protected bool m_ObjectListHeldShown;

	//! The same menu the world tags use, so a row in this list and the tag over
	//! the object it names offer the identical set of actions.
	protected ref JMESPActionMenu m_ObjectMenu;
	//! Parallel to the list's rows - the list itself only holds strings.
	protected ref array<JMESPMeta> m_ObjectRows = new array<JMESPMeta>;

	protected UIActionButton m_ExportButton;
	protected UIActionDropdown m_ExportTypeList;
	protected UIActionButton m_MoveToCursorButton;
	protected UIActionConfirmInline m_DeleteButton;

	//! Bulk selection. Feedback buttons because what they change is a list
	//! somewhere else on screen - see the note where they are built.
	protected UIActionFeedbackButton m_SelectAll;
	protected UIActionFeedbackButton m_DeselectAll;

	void JMESPFormTabObjects( JMESPForm form )
	{
		m_Form = form;
	}

	void Build( Widget parent )
	{
		m_ESPSelectedObjects = UIActionManager.CreateScroller( parent );
		Widget container = m_ESPSelectedObjects.GetContentWidget();

		//! The list and the things you do to a selection are two different
		//! jobs, so they get a card each rather than a loose search row over a
		//! bare list. The count lives in the card's own title, which is where
		//! the eye already is and costs no row of its own.
		//!
		//! Selection first, tracked objects under it: the actions are a fixed
		//! three rows while the list grows to whatever the scan returned, so
		//! putting the list first pushes the buttons off the bottom of the tab.
		UIActionCard card = UIActionManager.CreateCard( container, "#STR_COT_ESP_MODULE_SELECTION_HEADER" );
		Widget cardContent = card.GetContent();

		//! Three even two-up rows, so the card reads as a block of paired
		//! actions instead of a ragged mix of full and half width buttons.
		Widget rowSelectors = UIActionManager.CreateGridSpacer( cardContent, 1, 2 );
		m_SelectAll = UIActionManager.CreateFeedbackButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_SELECT_ALL", "#STR_COT_SELECTED", JMConstants.ICON_CHECK_MARK, this, "OnClick_SelectAll" );
		m_SelectAll.SetIcon( JMConstants.ICON_SELECT_ALL );
		m_SelectAll.SetTooltip( "#STR_COT_ESP_MODULE_TT_OBJECT_SELECT_ALL" );
		m_DeselectAll = UIActionManager.CreateFeedbackButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_DESELECT_ALL", "#STR_COT_CLEARED", JMConstants.ICON_CHECK_MARK, this, "OnClick_DeselectAll" );
		m_DeselectAll.SetIcon( JMConstants.ICON_DESELECT_ALL );
		m_DeselectAll.SetTooltip( "#STR_COT_ESP_MODULE_TT_OBJECT_DESELECT_ALL" );

		Widget rowMisc = UIActionManager.CreateGridSpacer( cardContent, 1, 2 );
		m_MoveToCursorButton = UIActionManager.CreateButton( rowMisc, "#STR_COT_ESP_MODULE_ACTION_MOVE_TO_CURSOR", this, "OnClick_MoveToCursor" );
		m_MoveToCursorButton.SetIcon( JMConstants.Lucide( "mouse-pointer-click" ) );
		m_MoveToCursorButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_MOVE_TO_CURSOR" );
		m_Form.BindPermission( m_MoveToCursorButton, JMConstants.PERM_ESP_OBJECT_MOVETOCURSOR );

		m_DeleteButton = UIActionManager.CreateConfirmInline( rowMisc, "#STR_COT_GENERIC_DELETE", this, "OnClick_DeleteSelected" );
		UIActionIconGrid.ApplyDeletePreset( m_DeleteButton );
		m_DeleteButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_DELETE_SELECTED" );
		m_Form.BindPermission( m_DeleteButton, JMConstants.PERM_ESP_OBJECT_DELETEALL );

		//! Format first, then the button that acts on it - the row reads in the
		//! order it is used.
		Widget rowExports = UIActionManager.CreateGridSpacer( cardContent, 1, 2 );

		TStringArray exportChoices = {
			"#STR_COT_ESP_MODULE_EXPORT_RAW",
			"#STR_COT_ESP_MODULE_EXPORT_SPAWNABLETYPES",
		#ifdef DZ_Expansion_Market
			"#STR_COT_ESP_MODULE_EXPORT_EXP_MARKET",
		#endif
		#ifdef DZ_Expansion_Core
			"#STR_COT_ESP_MODULE_EXPORT_EXP_LOADOUT",
		#endif
			"#STR_COT_LOADOUT_MODULE_NAME"
		};
		m_ExportTypeList = UIActionManager.CreateDropdown( rowExports, "", parent, this, "OnClick_ExportType", exportChoices );
		m_Form.RegisterOverlay( m_ExportTypeList );

		//! Without this the dropdown opens blank, which reads as "no format
		//! chosen" even though Raw is what the button would actually do.
		m_ExportTypeList.SetSelection( COT_ESPMode.COPYLISTRAW, false );

		m_ExportButton = UIActionManager.CreateButton( rowExports, "#STR_COT_TO_CLIPBOARD", this, "OnClick_CopyToClipboard" );
		m_ExportButton.SetIcon( JMConstants.ICON_STACK );
		m_ExportButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_EXPORT_SELECTED" );

		m_ObjectsCard = UIActionManager.CreateCard( container, "#STR_COT_ESP_MODULE_OBJECTS_CARD" );
		Widget objectsContent = m_ObjectsCard.GetContent();

		m_ObjectsCard.AddRefreshButton( this, "OnClick_RefreshObjects" );

		m_ObjectListLockButton = m_ObjectsCard.AddCardHeaderAction( JMConstants.Lucide( "lock-open" ), this, "OnClick_ToggleListLock", "#STR_COT_ESP_MODULE_LIST_LOCK" );

		m_ObjectSearch = UIActionManager.CreateSearchBox( objectsContent, this, "OnChange_ObjectFilter", "#STR_COT_ESP_MODULE_OBJECT_FILTER" );
		m_ObjectSearch.SetTooltip( "#STR_COT_ESP_MODULE_TT_OBJECT_FILTER" );

		//! Virtualized: only the visible rows exist as widgets, so a scan that
		//! comes back with a few thousand objects costs a few thousand strings
		//! rather than a few thousand layouts.
		m_ObjectList = UIActionManager.CreateItemList( objectsContent, this, "OnClick_ObjectRow" );
		ApplyObjectListHeight();

		RefreshList();

		m_ESPSelectedObjects.UpdateScroller();
	}

	void OnResize()
	{
		ApplyObjectListHeight();

		if ( m_ESPSelectedObjects )
			m_ESPSelectedObjects.UpdateScroller();
	}

	//! Built from the module's tracked list rather than from a walk of
	//! JMESPMeta.s_JM_All: the linked list also holds metas queued for creation
	//! and destruction, which are not on screen.
	void RefreshList( bool force = false )
	{
		if ( !m_ObjectList )
			return;

		RefreshListLockIndicator();

		if ( !force && IsObjectListHeld() )
			return;

		array< ref JMESPMeta > active = m_Form.m_Module.GetActiveObjects();

		string needle = "";
		if ( m_ObjectSearch )
			needle = m_ObjectSearch.GetText();
		needle.ToLower();

		m_ObjectRows.Clear();

		TStringArray names = new TStringArray;
		TStringArray subs  = new TStringArray;

		vector camera = g_Game.GetCurrentCameraPosition();

		foreach ( JMESPMeta meta : active )
		{
			if ( !meta || !meta.target )
				continue;

			string label = meta.GetName();
			string classname = meta.GetType();

			if ( needle != "" )
			{
				string labelLower = label;
				labelLower.ToLower();

				string classLower = classname;
				classLower.ToLower();

				if ( labelLower.IndexOf( needle ) < 0 && classLower.IndexOf( needle ) < 0 )
					continue;
			}

			int distance = Math.Round( vector.Distance( camera, meta.target.GetPosition() ) );

			string category = classname;
			if ( meta.type )
				category = Widget.TranslateString( meta.type.Localisation );

			names.Insert( label );
			subs.Insert( string.Format( "%1  -  %2m", category, distance ) );

			m_ObjectRows.Insert( meta );
		}

		m_ObjectList.SetItems( names, subs );

		ApplyObjectListHeight();

		RefreshObjectCardTitle( names.Count() );
	}

	//! Card title carries the count, and says so when the list is being held.
	private void RefreshObjectCardTitle( int count )
	{
		if ( !m_ObjectsCard )
			return;

		string title = string.Format( "%1 (%2)", Widget.TranslateString( "#STR_COT_ESP_MODULE_OBJECTS_CARD" ), count );

		if ( m_ObjectListHeldShown )
			title = string.Format( "%1  -  %2", title, Widget.TranslateString( "#STR_COT_ESP_MODULE_LIST_HELD" ) );

		m_ObjectsCard.SetLabel( title );
	}

	void OnClick_RefreshObjects( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		action.AnimateSpin();

		//! An explicit refresh beats the hold - that is what it is for.
		RefreshList( true );
	}

	//! True while the list must not be rebuilt under the admin.
	//!
	//! A 500ms rebuild re-sorts and re-indexes every row, which makes the list
	//! impossible to work: the entry under the cursor is gone by the time it is
	//! clicked. Touching the list holds it automatically, and the lock button
	//! holds it indefinitely.
	bool IsObjectListHeld()
	{
		if ( m_ObjectListPinned )
			return true;

		return m_ObjectList && m_ObjectList.IsFocused();
	}

	void OnClick_ToggleListLock( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_ObjectListPinned = !m_ObjectListPinned;

		RefreshListLockIndicator();

		//! Unpinning is a request to see current data now, not in half a
		//! second.
		if ( !m_ObjectListPinned )
			RefreshList( true );
	}

	//! Icon and title marker follow the EFFECTIVE hold, so a list held by the
	//! cursor says so just as plainly as one held by the button.
	private void RefreshListLockIndicator()
	{
		bool held = IsObjectListHeld();

		if ( held == m_ObjectListHeldShown )
			return;

		m_ObjectListHeldShown = held;

		if ( !m_ObjectListLockButton )
			return;

		if ( held )
		{
			m_ObjectListLockButton.SetImage( JMConstants.Lucide( "lock" ) );
			m_ObjectListLockButton.SetColor( JMTheme.WARNING );
		}
		else
		{
			m_ObjectListLockButton.SetImage( JMConstants.Lucide( "lock-open" ) );
			m_ObjectListLockButton.SetColor( JMTheme.TEXT_SECONDARY );
		}

		RefreshObjectCardTitle( m_ObjectRows.Count() );
	}

	//! Height of the object list: as many rows as it actually holds, clamped to
	//! [MIN_ROWS, MAX_ROWS] and then to whatever the window can spare. An empty
	//! list is four rows tall, not half the form.
	private void ApplyObjectListHeight()
	{
		if ( !m_ObjectList )
			return;

		int rows = Math.Clamp( m_ObjectList.GetItemCount(), OBJECT_LIST_MIN_ROWS, OBJECT_LIST_MAX_ROWS );

		float listHeight = rows * UIActionItemList.ROW_H;

		float available = m_Form.m_ContentHeight - JMESPForm.TOOLBAR_HEIGHT - JMFormBase.TAB_STRIP_HEIGHT - OBJECT_LIST_CHROME;
		float floorPx   = OBJECT_LIST_MIN_ROWS * UIActionItemList.ROW_H;

		if ( available < floorPx )
			available = floorPx;

		if ( listHeight > available )
			listHeight = available;

		m_ObjectList.SetViewportHeight( listHeight );
	}

	void OnChange_ObjectFilter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		RefreshList( true );
	}

	void OnClick_ObjectRow( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		int idx = m_ObjectList.GetSelectedIndex();

		if ( idx < 0 || idx >= m_ObjectRows.Count() )
			return;

		JMESPMeta meta = m_ObjectRows[idx];

		if ( !meta || !meta.widgetHandler )
			return;

		if ( m_ObjectList.GetLastClickButton() == MouseState.RIGHT )
		{
			ShowObjectMenu( meta );
			return;
		}

		meta.widgetHandler.Select();
	}

	private void ShowObjectMenu( JMESPMeta meta )
	{
		if ( !m_ObjectMenu )
		{
			CF_Window window = m_Form.GetWindow();
			if ( !window )
				return;

			m_ObjectMenu = new JMESPActionMenu();

			//! Anchored to the window root, not to the list: a menu parented to
			//! the list would be clipped by it on the bottom rows.
			UIActionContextMenu menu = m_ObjectMenu.Init( m_Form.GetLayoutRoot(), window.GetWidgetRoot() );

			if ( !menu )
			{
				m_ObjectMenu = NULL;
				return;
			}

			m_Form.RegisterOverlay( menu );
		}

		int mx;
		int my;
		GetMousePos( mx, my );

		m_ObjectMenu.Open( meta, mx, my );
	}

	void OnClick_ExportType( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_ExportTypeList.GetSelection() == COT_ESPMode.CREATELOADOUT )
			m_ExportButton.SetButton( "#STR_COT_SAVE_AS" );
		else
			m_ExportButton.SetButton( "#STR_COT_TO_CLIPBOARD" );
	}

	void OnClick_DeleteSelected( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_ESP_OBJECT_DELETEALL ) )
			return;

		m_Form.CreateAdvancedObjectConfirm( "DeleteSelected", "DeleteSelected" );
	}

	void OnClick_SelectAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		auto node = JMESPMeta.s_JM_All.m_Head;
		while ( node )
		{
			if ( node.m_Value.widgetRoot.IsVisible() )
				node.m_Value.widgetHandler.Select();

			node = node.m_Next;
		}
	}

	void OnClick_DeselectAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		auto node = JMESPMeta.s_JM_All.m_Head;
		while ( node )
		{
			node.m_Value.widgetHandler.Deselect();
			node = node.m_Next;
		}

		JM_GetSelected().ClearObjects();
	}

	void OnClick_MoveToCursor( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_ESP_OBJECT_MOVETOCURSOR ) )
			return;

		m_Form.CreateAdvancedObjectConfirm( "MoveToCursor", "MoveToCursor" );
	}

	void OnClick_CopyToClipboard( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		switch ( m_ExportTypeList.GetSelection() )
		{
			case COT_ESPMode.COPYLISTRAW:
				m_Form.m_Module.CopyToClipboardRaw();
			break;
			case COT_ESPMode.COPYLISTSPAWNABLETYPES:
				m_Form.m_Module.CopyToClipboardSpawnableTypes();
			break;
		#ifdef DZ_Expansion_Market
			case COT_ESPMode.COPYLISTEXPMARKET:
				m_Form.m_Module.CopyToClipboardMarket();
			break;
		#endif
		#ifdef DZ_Expansion_Core
			case COT_ESPMode.COPYLISTEXPLOADOUT:
				m_Form.m_Module.CopyToClipboardExpLoadout( JMSelectedObject );
			break;
		#endif
			case COT_ESPMode.CREATELOADOUT:
				m_Form.CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_ESP_MODULE_LOADOUT_MESSAGE_HEADER", "#STR_COT_ESP_MODULE_LOADOUT_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CREATE", "CreateLoadout_Confirm" );
			break;
		}
	}
}
