//! Archetype B: a fixed toolbar over a tabbed pane.
//!
//! Filters/Objects/Settings tabs each live in their own file
//! (JMESPFormTabFilters.c / JMESPFormTabObjects.c / JMESPFormTabSettings.c).
//! What stays here:
//!   - The shared toolbar (ESP on/off + classname filter), above the tab strip.
//!   - `IconForViewType` - a deprecated static forwarder to JMESPViewTypeWidget, for sub-mods
//!     that still call it on this class.
//!   - DeleteSelected / MoveToCursor / CreateLoadout_Confirm /
//!     OnSendMessage_Confirm / PromptSendMessage - JMConfirmation dispatches
//!     its named callbacks against whatever object its window was Init()'d
//!     with, not against whichever tab raised the popup, so these have to
//!     stay on the form. PromptSendMessage is also called externally
//!     (JMESPActionMenu.c, via espModule.GetForm()), which is a second,
//!     independent reason it cannot move.
class JMESPForm: JMFormBase
{
	//! Tab indices - what the strip's AddTab() returned for each tab, never written as numbers.
	protected int m_TabIdFilters;
	protected int m_TabIdObjects;
	protected int m_TabIdSettings;

	//! Height of panel_top in esp_form.layout. The tab strip below is pinned
	//! against it in OnResize; keep the two in step.
	static const int TOOLBAR_HEIGHT = 44;

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

	//! One class per tab, in its own file - only this form constructs/
	//! dispatches to them. Not private: the toolbar/lifecycle forwarders
	//! below read them, and so do the JMESPViewTypeWidget forwarders.
	ref JMESPFormTabFilters  m_TabFilters;
	ref JMESPFormTabObjects  m_TabObjects;
	ref JMESPFormTabSettings m_TabSettings;

	//! Last content height OnResize reported. Public: JMESPFormTabObjects's
	//! ApplyObjectListHeight() needs it to size the object list.
	float m_ContentHeight;
	protected string m_PendingMsgPlayerGUID;

	void JMESPForm()
	{
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

	override void OnCreate()
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

		m_ToggleButton = UIActionManager.CreateButton( row, "#STR_COT_ESP_MODULE_TOGGLE", this, "" );
		if ( m_ToggleButton ) m_ToggleButton.SetOnClick( this, "OnClick_UpdateESP" );
		m_ToggleButton.SetFlex( 0, 150, 150 );
		m_ToggleButton.SetIcon( JMConstants.Lucide( "eye" ) );
		m_ToggleButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_TOGGLE_ESP" );
		m_ToolbarRow.Add( m_ToggleButton );
		BindPermission( m_ToggleButton, JMConstants.PERM_ESP_VIEW );

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
			AddOverlay( m_SearchBox );
		}

		m_ToolbarRow.SetGap( 10 );

		//! Paint it now rather than waiting for the first UpdateUI tick, which
		//! is up to 500ms away - long enough to see the placeholder label.
		ApplyToggleFace();
	}

	protected void InitWidgetsBottom()
	{
		Widget panelFilters  = layoutRoot.FindAnyWidget( "esp_filters_panel" );
		Widget panelObjects  = layoutRoot.FindAnyWidget( "esp_objects_panel" );
		Widget panelSettings = layoutRoot.FindAnyWidget( "esp_settings_panel" );


		m_BottomTabStrip = layoutRoot.FindAnyWidget( "panel_bottom_tabs" );
		m_BottomContent  = layoutRoot.FindAnyWidget( "panel_bottom_content" );

		m_Tabs = UIActionManager.CreateTabStrip( m_BottomTabStrip, this, "OnChange_Tab" );

		m_TabIdFilters = m_Tabs.AddTab( "#STR_COT_ESP_TAB_FILTERS", JMConstants.Lucide( "filter" ), panelFilters );
		m_TabIdObjects = m_Tabs.AddTab( "#STR_COT_ESP_TAB_VISIBLE", JMConstants.Lucide( "eye" ), panelObjects );
		m_TabIdSettings = m_Tabs.AddTab( "#STR_COT_ESP_TAB_SETTINGS", JMConstants.Lucide( "settings" ), panelSettings );

		DeclareTabs( 3 );

		m_Tabs.SetSelection( m_TabIdFilters, false );

		InitTabFocus( m_TabIdFilters );
	}

	// =========================================================================
	//  Tabs & lifecycle
	// =========================================================================

	override protected void OnTabCreate( int tab, Widget panel )
	{
		if ( tab == m_TabIdFilters )
		{
			m_TabFilters = new JMESPFormTabFilters( this );
			RegisterTab( m_TabIdFilters, m_TabFilters );
			m_TabFilters.OnCreate( panel );
		}
		else if ( tab == m_TabIdObjects )
		{
			m_TabObjects = new JMESPFormTabObjects( this );
			RegisterTab( m_TabIdObjects, m_TabObjects );
			m_TabObjects.OnCreate( panel );
		}
		else if ( tab == m_TabIdSettings )
		{
			m_TabSettings = new JMESPFormTabSettings( this );
			RegisterTab( m_TabIdSettings, m_TabSettings );
			m_TabSettings.OnCreate( panel );
		}

		UpdateUI();
	}

	protected override COT_ScriptedWidgetEventHandler GetTabStrip()
	{
		return m_Tabs;
	}

	void OnChange_Tab( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CHANGE )
			HandleTabChange();
	}

	override void OnResize( float w, float h )
	{
		super.OnResize( w, h );

		PinBottomPanelGeometry( h - TOOLBAR_HEIGHT );

		if ( m_ToolbarRow )
			m_ToolbarRow.Layout();

		m_ContentHeight = h;

		ResizeTabs( w, h );
	}

	override void OnShow()
	{
		super.OnShow();

		DeferCall( "UpdateUI", 500, true );

		UpdateUI();
	}

	override void OnHide()
	{
		super.OnHide();

		CancelDeferredCall( "UpdateUI" );
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

		UpdateActiveTab();

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

	void OnClick_UpdateESP( UIActionBase action )
	{
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
		vector contactPos;

		if ( COT_CameraRaycast( 1000, contactPos ) )
			m_Module.MoveToCursor( contactPos );
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

	void PromptSendMessage( string playerGUID, string playerName )
	{
		m_PendingMsgPlayerGUID = playerGUID;
		PromptInput( "#STR_COT_PLAYER_MODULE_MESSAGE_HEADER", Widget.TranslateString( "#STR_COT_ESP_MODULE_MESSAGE_PROMPT" ) + " " + playerName + ":", "OnSendMessage_Confirm" );
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

	//! DEPRECATED - moved to JMESPViewTypeWidget.IconForViewType().
	static string IconForViewType( JMESPViewType viewType )
	{
		JMDeprecated.WarnOnce( null, "JMESPForm.IconForViewType() is deprecated. It moved to JMESPViewTypeWidget.IconForViewType()." );

		return JMESPViewTypeWidget.IconForViewType( viewType );
	}
}
