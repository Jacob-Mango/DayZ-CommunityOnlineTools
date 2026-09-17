//! Archetype B: a fixed toolbar over a tabbed pane.
//!
//! Filters/Objects/Settings tabs each live in their own file
//! (JMESPFormTabFilters.c / JMESPFormTabObjects.c / JMESPFormTabSettings.c).
//! What stays here:
//!   - The shared toolbar (ESP on/off + classname filter), above the tab strip.
//!   - `IconForViewType` - JMESPViewTypeWidget.c calls it class-qualified
//!     (JMESPForm.IconForViewType), so it has to stay a static on this exact
//!     class.
//!   - OnRightClick_CategoryRow / OnChange_CategoryRow / OpenCategoryColourPopup
//!     - thin forwarders into the Filters tab. JMESPViewTypeWidget.c holds a
//!     JMESPForm-typed back-reference and calls these three directly, so
//!     they cannot move without also editing that file; forwarding keeps
//!     that file untouched.
//!   - DeleteSelected / MoveToCursor / CreateLoadout_Confirm /
//!     OnSendMessage_Confirm / PromptSendMessage - JMConfirmation dispatches
//!     its named callbacks against whatever object its window was Init()'d
//!     with, not against whichever tab raised the popup, so these have to
//!     stay on the form. PromptSendMessage is also called externally
//!     (JMESPActionMenu.c, via espModule.GetForm()), which is a second,
//!     independent reason it cannot move.
class JMESPForm: JMFormBase
{
	//! Height of panel_top in esp_form.layout. The tab strip below is pinned
	//! against it in OnResize; keep the two in step.
	static const int TOOLBAR_HEIGHT = 44;

	static const int TAB_FILTERS  = 0;
	static const int TAB_OBJECTS  = 1;
	static const int TAB_SETTINGS = 2;

	protected autoptr array< ref JMESPViewTypeWidget > m_ESPTypeList;
	protected ref map<typename, JMESPViewTypeWidget> m_ESPTypeWidgetsByType;

	// -- Toolbar (panel_top) --------------------------------------------------
	protected ref UIActionFlexRow m_ToolbarRow;
	protected UIActionButton m_ToggleButton;
	protected UIActionSearchBox m_SearchBox;

	//! Every spawnable classname, scanned once and handed to the search box as
	//! its suggestion set. Null until the first keystroke - the walk covers
	//! four config trees and is far too slow to pay for at form open.
	protected ref TStringArray m_ClassnameSuggestions;

	//! protected, not private: sub-mods reach for the module through the form.
	//! Also public enough (no modifier) for the three tab classes to reach it
	//! through their back-reference.
	JMESPModule m_Module;
	JMLoadoutModule m_LoadoutModule;

	protected UIActionTabs m_Tabs;
	protected Widget m_TabFiltersPanel;
	protected Widget m_TabObjectsPanel;
	protected Widget m_TabSettingsPanel;

	//! One class per tab, in its own file - only this form constructs/
	//! dispatches to them. Not private: the toolbar/lifecycle forwarders
	//! below read them, and so do the JMESPViewTypeWidget forwarders.
	ref JMESPFormTabFilters  m_TabFilters;
	ref JMESPFormTabObjects  m_TabObjects;
	ref JMESPFormTabSettings m_TabSettings;

	//! Last content height OnResize reported. Public: JMESPFormTabObjects's
	//! ApplyObjectListHeight() needs it to size the object list.
	float m_ContentHeight;

	void JMESPForm()
	{
		m_ESPTypeList = new array< ref JMESPViewTypeWidget >;
		m_ESPTypeWidgetsByType = new map<typename, JMESPViewTypeWidget>;

		JMScriptInvokers.ESP_VIEWTYPE_CHANGED.Insert( OnESPViewTypeChanged );
	}

	void ~JMESPForm()
	{
		JMScriptInvokers.ESP_VIEWTYPE_CHANGED.Remove( OnESPViewTypeChanged );
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		JMESPWidgetHandler.espMenu = this;

		InitWidgetsTop();
		InitWidgetsBottom();

		Class.CastTo( m_LoadoutModule, GetModuleManager().GetModule( JMLoadoutModule ) );
	}

	// =========================================================================
	//  Toolbar
	// =========================================================================

	protected void InitWidgetsTop()
	{
		Widget top = layoutRoot.FindAnyWidget( "panel_top" );

		m_ToolbarRow = UIActionManager.CreateFlexRow( top, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		Widget row = m_ToolbarRow.GetContent();

		m_ToggleButton = UIActionManager.CreateButton( row, "#STR_COT_ESP_MODULE_TOGGLE", this, "OnClick_UpdateESP" );
		m_ToggleButton.SetFlex( 0, 150, 150 );
		m_ToggleButton.SetIcon( JMConstants.Lucide( "eye" ) );
		m_ToggleButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_TOGGLE_ESP" );
		m_ToolbarRow.Add( m_ToggleButton );
		RegisterPermission( m_ToggleButton, JMConstants.PERM_ESP_VIEW );

		//! UIActionSearchBox carries its own inline clear, so there is no
		//! separate 'x' button beside it any more.
		//! Placeholder names what it does, not what it is. "Filter:" said
		//! nothing about WHAT was being filtered, in a form that has two other
		//! search boxes filtering two other things.
		m_SearchBox = UIActionManager.CreateSearchBox( row, this, "OnChange_Filter", "#STR_COT_ESP_MODULE_CLASS_FILTER", m_Module.Filter );
		m_SearchBox.SetFlex( 1.0, 120 );
		m_SearchBox.SetTooltip( "#STR_COT_ESP_MODULE_TT_CLASS_FILTER" );
		m_ToolbarRow.Add( m_SearchBox );

		//! Anchored to the window root so the suggestion list floats over the
		//! tabs below instead of being clipped by the toolbar strip.
		if ( GetWindow() )
		{
			m_SearchBox.InitSuggestionList( GetWindow().GetWidgetRoot() );
			m_SearchBox.SetMaxVisibleSuggestions( 10 );
			RegisterOverlay( m_SearchBox );
		}

		m_ToolbarRow.SetGap( 10 );

		//! Paint it now rather than waiting for the first UpdateUI tick, which
		//! is up to 500ms away - long enough to see the placeholder label.
		ApplyToggleFace();
	}

	protected void InitWidgetsBottom()
	{
		m_TabFiltersPanel  = layoutRoot.FindAnyWidget( "esp_filters_panel" );
		m_TabObjectsPanel  = layoutRoot.FindAnyWidget( "esp_objects_panel" );
		m_TabSettingsPanel = layoutRoot.FindAnyWidget( "esp_settings_panel" );

		ref array<string> tabLabels = { "#STR_COT_ESP_TAB_FILTERS", "#STR_COT_ESP_TAB_VISIBLE", "#STR_COT_ESP_TAB_SETTINGS" };
		ref array<string> tabIcons  = { JMConstants.Lucide( "filter" ), JMConstants.Lucide( "eye" ), JMConstants.Lucide( "settings" ) };

		m_Tabs = UIActionManager.CreateTabs( layoutRoot.FindAnyWidget( "panel_bottom_tabs" ), tabLabels, tabIcons, this, "OnChange_Tab" );

		m_Tabs.AddContent( m_TabFiltersPanel );
		m_Tabs.AddContent( m_TabObjectsPanel );
		m_Tabs.AddContent( m_TabSettingsPanel );

		InitTabState( 3 );

		m_Tabs.SetSelection( TAB_FILTERS, false );

		BuildTabIfNeeded( TAB_FILTERS );
	}

	// =========================================================================
	//  JMESPViewTypeWidget forwarders - see file header.
	// =========================================================================

	void OnRightClick_CategoryRow( JMESPViewTypeWidget row )
	{
		if ( m_TabFilters )
			m_TabFilters.OnRightClick_CategoryRow( row );
	}

	void OnChange_CategoryRow()
	{
		if ( m_TabFilters )
			m_TabFilters.OnChange_CategoryRow();
	}

	void OpenCategoryColourPopup( JMESPViewTypeWidget row, Widget anchor )
	{
		if ( m_TabFilters )
			m_TabFilters.OpenCategoryColourPopup( row, anchor );
	}

	//! Glyph baked into a category row's switch thumb. Keyed on typename the
	//! same way GroupIndexFor is, and for the same reason: a sub-mod's own view
	//! type falls through to the generic marker instead of rendering blank.
	//!
	//! Most-specific first - every weapon and item leaf also passes the base
	//! test, so the bases have to be asked last. Referenced class-qualified
	//! from JMESPViewTypeWidget.c - must stay on JMESPForm.
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

	// =========================================================================
	//  Tabs & lifecycle
	// =========================================================================

	private void BuildTabIfNeeded( int tabIdx )
	{
		if ( !ShouldBuildTab( tabIdx ) )
			return;

		switch ( tabIdx )
		{
			case TAB_FILTERS:
				m_TabFilters = new JMESPFormTabFilters( this );
				m_TabFilters.Build( m_TabFiltersPanel );
				break;

			case TAB_OBJECTS:
				m_TabObjects = new JMESPFormTabObjects( this );
				m_TabObjects.Build( m_TabObjectsPanel );
				break;

			case TAB_SETTINGS:
				m_TabSettings = new JMESPFormTabSettings( this );
				m_TabSettings.Build( m_TabSettingsPanel );
				break;
		}

		UpdateUI();
	}

	override int GetActiveTabIndex()
	{
		if ( !m_Tabs )
			return -1;

		return m_Tabs.GetSelection();
	}

	void OnChange_Tab( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CloseAllOverlays();

		BuildTabIfNeeded( GetActiveTabIndex() );

		if ( IsTabActive( TAB_OBJECTS ) && m_TabObjects )
			m_TabObjects.RefreshList();
	}

	override void OnResize( float w, float h )
	{
		super.OnResize( w, h );

		PinStripGeometry( layoutRoot.FindAnyWidget( "panel_bottom_tabs" ), layoutRoot.FindAnyWidget( "panel_bottom_content" ), h - TOOLBAR_HEIGHT, TAB_STRIP_HEIGHT );

		if ( m_ToolbarRow )
			m_ToolbarRow.Layout();

		m_ContentHeight = h;

		if ( m_TabFilters )
			m_TabFilters.OnResize();

		if ( m_TabObjects )
			m_TabObjects.OnResize();
	}

	override void OnShow()
	{
		super.OnShow();

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateUI, 500, true );

		UpdateUI();
	}

	override void OnHide()
	{
		super.OnHide();

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( UpdateUI );
	}

	//! Kept because JMESPModule.OnClientPermissionsUpdated calls it. The
	//! per-control work it used to stand in for is now done by the base's
	//! registered-permission walk.
	void DisableToggleableOptions()
	{
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();

		if ( m_TabFilters )
			m_TabFilters.RefreshGroupHeaders();

		UpdateUI();
	}

	void OnESPViewTypeChanged( JMESPViewType viewType )
	{
		if ( m_TabSettings )
			m_TabSettings.UpdateMaxRange();
	}

	//! Paint the toolbar toggle to say, in one glance, whether ESP is currently
	//! drawing and what clicking will do about it.
	protected void ApplyToggleFace()
	{
		if ( !m_ToggleButton )
			return;

		if ( m_Module.GetState() != JMESPState.Remove )
		{
			m_ToggleButton.SetButton( "#STR_COT_ESP_MODULE_STATE_ON" );
			m_ToggleButton.SetIcon( JMConstants.Lucide( "eye" ) );
			m_ToggleButton.SetColor( JMTheme.SUCCESS_FILL );
			m_ToggleButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_STATE_ON" );
		}
		else
		{
			m_ToggleButton.SetButton( "#STR_COT_ESP_MODULE_STATE_OFF" );
			m_ToggleButton.SetIcon( JMConstants.Lucide( "eye-off" ) );
			m_ToggleButton.SetColor( JMTheme.BUTTON_FILL );
			m_ToggleButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_STATE_OFF" );
		}
	}

	//! Runs on a 500ms timer while the form is open, so every widget it touches
	//! can belong to a tab that has never been built.
	void UpdateUI()
	{
		if ( m_TabSettings )
			m_TabSettings.RefreshSliders();

		if ( IsTabActive( TAB_OBJECTS ) && m_TabObjects )
			m_TabObjects.RefreshList();

		ApplyToggleFace();
	}

	//! Feed the search box its suggestion set, building it on the first call.
	void UpdateList()
	{
		if ( !m_SearchBox )
			return;

		if ( m_ClassnameSuggestions )
			return;

		m_ClassnameSuggestions = new TStringArray;

		TStringArray configs = new TStringArray;
		configs.Insert( CFG_VEHICLESPATH );
		configs.Insert( CFG_WEAPONSPATH );
		configs.Insert( CFG_MAGAZINESPATH );
		configs.Insert( CFG_NONAI_VEHICLES );

		for ( int nConfig = 0; nConfig < configs.Count(); nConfig++ )
		{
			string strConfigPath = configs.Get( nConfig );

			int nClasses = g_Game.ConfigGetChildrenCount( strConfigPath );

			//! The first twenty entries of the vehicles tree are the abstract
			//! bases, which are not spawnable and never worth suggesting.
			int nClassStart = 0;
			if ( nConfig == 0 ) nClassStart = 20;

			for ( int nClass = nClassStart; nClass < nClasses; nClass++ )
			{
				string strName;

				g_Game.ConfigGetChildName( strConfigPath, nClass, strName );

				int scope = g_Game.ConfigGetInt( strConfigPath + " " + strName + " scope" );

				if ( scope == 0 )
					continue;

				if ( !g_Game.ConfigIsExisting( strConfigPath + " " + strName + " model" ) )
					continue;

				m_ClassnameSuggestions.Insert( strName );
			}
		}

		m_SearchBox.SetSuggestions( m_ClassnameSuggestions );
	}

	// =========================================================================
	//  Toolbar handlers
	// =========================================================================

	void OnClick_UpdateESP( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_ESP_VIEW ) )
			return;

		if ( m_Module.GetState() != JMESPState.Remove )
		{
			m_Module.UpdateState( JMESPState.Remove );
			m_Module.Log( "Clearing ESP" );
		} else
		{
			bool autoRefresh = m_TabSettings && m_TabSettings.IsAutoRefreshChecked();

			if ( autoRefresh )
			{
				m_Module.UpdateState( JMESPState.Update );
				m_Module.Log( "ESP updating" );
			}
			else
			{
				m_Module.UpdateState( JMESPState.View );
				m_Module.Log( "Viewing ESP" );
			}
		}

		UpdateUI();
	}

	void OnChange_Filter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateList();

		m_Module.Filter = action.GetText();
	}

	// =========================================================================
	//  Confirmation-callback chain and external API - stay here, see file
	//  header.
	// =========================================================================

	void OnClick_DuplicateSelected( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_ESP_OBJECT_DUPLICATEALL ) )
			return;

		m_Module.DuplicateSelected();
	}

	void DeleteSelected()
	{
		m_Module.DeleteSelected();

		if ( m_TabObjects )
			m_TabObjects.RefreshList();
	}

	void MoveToCursor()
	{
		vector dir = g_Game.GetCurrentCameraDirection();
		vector from = g_Game.GetCurrentCameraPosition();
		vector to = from + ( dir * 1000 );
		vector contact_pos;
		vector contact_dir;
		int contact_component;

		if ( DayZPhysics.RaycastRV( from, to, contact_pos, contact_dir, contact_component, NULL, NULL, NULL, false, true ) )
			m_Module.MoveToCursor( contact_pos );
	}

	void CreateLoadout_Confirm( JMConfirmation confirmation )
	{
		string name = confirmation.GetEditBoxValue();
		if ( name == string.Empty )
			return;

		if ( !m_LoadoutModule )
			Class.CastTo( m_LoadoutModule, GetModuleManager().GetModule( JMLoadoutModule ) );

		m_LoadoutModule.Create( name );
	}

	protected string m_PendingMsgPlayerGUID;

	void PromptSendMessage( string playerGUID, string playerName )
	{
		m_PendingMsgPlayerGUID = playerGUID;
		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_MESSAGE_HEADER", Widget.TranslateString( "#STR_COT_ESP_MODULE_MESSAGE_PROMPT" ) + " " + playerName + ":", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "OnSendMessage_Confirm" );
	}

	void OnSendMessage_Confirm( JMConfirmation confirmation )
	{
		string msgText = confirmation.GetEditBoxValue();
		if ( msgText == "" || m_PendingMsgPlayerGUID == "" )
			return;

		JMPlayerModule playerModule = JMPlayerModule.Cast( GetModuleManager().GetModule( JMPlayerModule ) );
		if ( playerModule )
		{
			playerModule.DoMessage( { m_PendingMsgPlayerGUID }, msgText );
		}
		m_PendingMsgPlayerGUID = "";
	}
}
