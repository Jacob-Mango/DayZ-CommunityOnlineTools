class JMObjectSpawnerForm: JMFormBase
{
	private UIActionIconGrid m_TypeFilter;

	private Widget m_TypesActionsWrapper;
	private Widget m_SpawnerActionsWrapper;

	private UIActionSlider m_QuantityItem;
	private UIActionSlider m_TemperatureItem;
	private UIActionSlider m_HealthItem;

	private UIActionSearchBox m_SearchBox;
	private UIActionDropdown m_SpawnMode;

	private UIActionDropdown m_ObjSetupMode;

	private TextListboxWidget m_ClassList;

	private ItemPreviewWidget m_ItemPreview;
	//! The preview's wrapper panel. The preview itself is no longer the layout
	//! root: it now hangs inside a plainly-styled panel, the way vanilla's own
	//! previews do, so it never inherits whatever style the engine has
	//! registered as the default for a styleless widget - which is exactly what
	//! COT's own style table changes. Geometry is applied to the WRAPPER; the
	//! preview fills it fractionally.
	private Widget m_ItemPreviewPanel;
	private EntityAI m_PreviewItem;
	//! Setup mode the current preview entity was built with. The attachments a
	//! spawn produces depend on it, so a mode change has to rebuild the preview
	//! exactly like a class change does - an entity that already has its debug
	//! attachments cannot be walked back to a bare one.
	private int m_PreviewSetupMode;
	private vector m_Orientation;
	private float m_Distance;

	private int m_MouseX;
	private int m_MouseY;

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMObjectSpawnerModule m_Module;

	private UIActionButton m_SpawnButton;
	private UIActionImageButton m_RefreshListButton;
	private UIActionScrollerH m_CategoryScroller;
	private ref UIActionFlexRow m_Footer;
	private static int s_ObjSpawnMode = COT_ObjectSpawnerMode.CURSOR;
	private ref array< string > m_ObjSpawnModeText =
	{
		"#STR_COT_OBJECT_MODULE_EXPORT_RAW",
		"#STR_COT_OBJECT_MODULE_EXPORT_TYPES",
	#ifdef DZ_Expansion_Market
		"#STR_COT_OBJECT_MODULE_EXPORT_MARKET",
	#endif

		"#STR_COT_OBJECT_MODULE_CROSSHAIR",
		"#STR_COT_OBJECT_MODULE_SELF",
		"#STR_COT_OBJECT_MODULE_TARGET"
	};

	void JMObjectSpawnerForm()
	{
		m_PreviewSetupMode = -1;
	}

	void ~JMObjectSpawnerForm()
	{
		if (m_PreviewItem)
			g_Game.ObjectDelete(m_PreviewItem);
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnResize( float w, float h )
	{
		super.OnResize( w, h );

		// Fixed top (search 40 + categories 52 = 92) and a bottom-anchored footer
		// (actions 160, valign bottom_ref self-pins). The browse panel is the ONE
		// fill panel: it grows to occupy whatever is left between them. This is the
		// legitimate single-fill case - only browse is computed, the footer anchors
		// itself. h is correct content-height layout pixels from JMWindowBase.
		Widget browseWrap = layoutRoot.FindAnyWidget( "object_browse_wrapper" );
		if ( browseWrap && h > 1 )
		{
			int topH    = 92;
			int actionH = 160;

			float browseH = h - topH - actionH;
			if ( browseH < 1 ) browseH = 1;

			browseWrap.SetFlags( WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE, true );
			browseWrap.SetPos( 0, topH );
			browseWrap.SetSize( 1, browseH );

			// List + preview each fill half the browse panel. Their layout files use
			// fractional height (size 0.5 1) AND the list has `lines 20`, which gives
			// the TextListbox a 20-row intrinsic height that overflows the panel and
			// bleeds into the sliders below. Pin both to EXACT pixels filling the
			// panel so neither the fractional re-resolution nor the line-count
			// intrinsic height can push them past browseWrap's bottom edge.
			float halfW = w * 0.5;
			if ( m_ClassList )
			{
				m_ClassList.SetFlags( WidgetFlags.HEXACTPOS | WidgetFlags.VEXACTPOS | WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE, true );
				m_ClassList.SetPos( 0, 0 );
				m_ClassList.SetSize( halfW, browseH );
			}
			if ( m_ItemPreviewPanel )
			{
				m_ItemPreviewPanel.SetFlags( WidgetFlags.HEXACTPOS | WidgetFlags.VEXACTPOS | WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE, true );
				m_ItemPreviewPanel.SetPos( halfW, 0 );
				m_ItemPreviewPanel.SetSize( halfW, browseH );
			}
		}

		// Category scroller recomputes its thumb/overflow against the new width.
		if ( m_CategoryScroller )
			m_CategoryScroller.UpdateScroller();

		// Re-flow the footer flex row (Mode / Setup / Spawn min-width + grow).
		if ( m_Footer )
			m_Footer.Layout();
	}

	override void OnShow()
	{
		super.OnShow();

		if ( m_ItemPreview )
			m_ItemPreview.Show( true );
		// _UpdateScroller fires 34ms after OnInit via CallLater, but the form
		// is not rendered yet then so GetScreenSize returns 0 -> chips get 0px
		// content height. Re-trigger once the form is actually on screen.
		if ( m_CategoryScroller )
			m_CategoryScroller.UpdateScroller();

		// Same story for the footer flex row: OnResize fired from SetModule before
		// render, where GetScreenSize returns 0 and Layout() no-ops. Defer one tick
		// so it re-flows against real on-screen widths. Bind to a this-method (the
		// proven CallLater pattern here) rather than m_Footer.Layout directly.
		if ( m_Footer )
			g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( _LayoutFooter, 34 );
	}

	//! this-bound trampoline for the deferred footer flex layout (CallLater target).
	void _LayoutFooter()
	{
		if ( m_Footer )
			m_Footer.Layout();
	}

	override void OnHide()
	{
		super.OnHide();
		if ( m_ItemPreview )
			m_ItemPreview.Show( false );
	}

	override void OnInit()
	{
		// ----------------------------------------------------------------------
		// Search-first layout (June 2026 redesign).
		//
		// Fixed-top / fill-body / bottom-anchored-footer model:
		//   object_search_wrapper      (40px  fixed, top_ref)    - search + checkboxes
		//   object_categories_wrapper  (52px  fixed, top_ref)    - horizontal chip scroller
		//   object_browse_wrapper      (fills, top_ref)          - list + preview
		//   object_actions_wrapper     (130px fixed, bottom_ref) - sliders + footer
		// The footer self-anchors to the bottom (valign bottom_ref) and the top
		// two panels are fixed height, so only the browse panel needs computing:
		// it fills the gap (h - 92 top - 130 footer). Done in OnResize/UpdateBrowseHeight.
		// ----------------------------------------------------------------------

		Widget searchWrap = layoutRoot.FindAnyWidget( "object_search_wrapper" );
		Widget catsWrap   = layoutRoot.FindAnyWidget( "object_categories_wrapper" );
		Widget browseWrap = layoutRoot.FindAnyWidget( "object_browse_wrapper" );
		Widget actionsWrap = layoutRoot.FindAnyWidget( "object_actions_wrapper" );

		// --- Row 1: Search anchor -------------------------------------------
		// Icon-button first, then fractional widgets summing to <= 0.99 to keep
		// all siblings on the same row (engine wraps at exactly 1.0).
		Widget searchRow = UIActionManager.CreateWrapSpacer( searchWrap, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_SearchBox = UIActionManager.CreateSearchBox( searchRow, this, "SearchInput_OnChange", "#STR_COT_OBJECT_MODULE_SEARCH" );
		m_SearchBox.SetWidth( 0.40 );

		UIActionCheckbox chkDisplayName = UIActionManager.CreateCheckbox( searchRow, "#STR_COT_OBJECT_MODULE_SPAWN_DISPLAYNAME", this, "Click_OnFilterDisplayName", m_Module.m_FilterWithDisplayName );
		chkDisplayName.SetWidth( 0.27 );
		chkDisplayName.SetTooltip( "Match search against in-game display names instead of class names" );

		UIActionCheckbox chkUnsafe = UIActionManager.CreateCheckbox( searchRow, "#STR_COT_OBJECT_MODULE_SHOWUNSAFE", this, "Click_OnSafetyToogle", m_Module.m_AllowRestrictedClassNames );
		chkUnsafe.SetWidth( 0.27 );
		chkUnsafe.SetTooltip( "Show classes blacklisted by JMObjectSpawnerModule (proxy_, *Source, etc.)" );

		// --- Row 2: Category strip (horizontal scroller) --------------------
		// Layout provides catsWrap as a fixed-height (~10% of form) panel with
		// clipchildren=1. Inside it we put a horizontal scroller that pans
		// through the labeled chip row.
		Widget hScrollerWidget = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionScrollerH.layout", catsWrap );
		Widget catsContent = catsWrap;
		if ( hScrollerWidget )
		{
			hScrollerWidget.GetScript( m_CategoryScroller );
			if ( m_CategoryScroller )
				catsContent = m_CategoryScroller.GetContentWidget();
		}
		m_TypesActionsWrapper = catsContent;
		m_TypeFilter = UIActionManager.CreateIconGridHorizontal( m_TypesActionsWrapper, this, "OnClick_TypeFilter" );
		m_TypeFilter.UseLabeledCells( true );
		m_TypeFilter.UseHorizontalLayout();
		m_TypeFilter.AddIcon( "",              JMConstants.Lucide( "layers" ),   "All"            );
		m_TypeFilter.AddIcon( "edible_base",   JMConstants.ICON_MEAT,            "Food"           );
		m_TypeFilter.AddIcon( "bottle_base",   JMConstants.ICON_BEER_BOTTLE,     "Drinks"         );
		m_TypeFilter.AddIcon( "transport",     JMConstants.ICON_JEEP,            "Vehicles"       );
		m_TypeFilter.AddIcon( "weapon_base",   JMConstants.ICON_FAMAS,           "Firearms"       );
		m_TypeFilter.AddIcon( "meleeweapon",   JMConstants.ICON_GLADIUS,         "Melee"          );
		m_TypeFilter.AddIcon( "magazine_base", JMConstants.ICON_MACHINE_GUN_MAG, "Ammo/Magazines" );
		m_TypeFilter.AddIcon( "clothing_base", JMConstants.ICON_CLOTHES,         "Clothing"       );
		m_TypeFilter.AddIcon( "headgear_base", JMConstants.ICON_STAHLHELM,       "Headgear"       );
		m_TypeFilter.AddIcon( "mask_base",     JMConstants.ICON_BALACLAVA,       "Masks"          );
		m_TypeFilter.AddIcon( "glasses_base",  JMConstants.ICON_SUNGLASSES,      "Glasses"        );
		m_TypeFilter.AddIcon( "top_base",      JMConstants.ICON_T_SHIRT,         "Tops"           );
		m_TypeFilter.AddIcon( "pants_base",    JMConstants.ICON_TROUSERS,        "Pants"          );
		m_TypeFilter.AddIcon( "vest_base",     JMConstants.ICON_BELT,            "Vests"          );
		m_TypeFilter.AddIcon( "gloves_base",   JMConstants.ICON_GLOVES,          "Gloves"         );
		m_TypeFilter.AddIcon( "shoes_base",    JMConstants.ICON_TROUSERS,        "Shoes"          );
		m_TypeFilter.AddIcon( "backpack_base", JMConstants.ICON_LIGHT_BACKPACK,  "Backpacks"      );
		m_TypeFilter.AddIcon( "container_base",JMConstants.ICON_KNAPSACK,        "Containers"     );
		m_TypeFilter.AddIcon( "inventory_base",JMConstants.ICON_FULL_FOLDER,     "Items"          );
		m_TypeFilter.AddIcon( "itemmedical",   JMConstants.ICON_MEDICINES,       "Medical"        );
		m_TypeFilter.AddIcon( "tool_base",     JMConstants.ICON_SHARP_AXE,       "Tools"          );
		m_TypeFilter.AddIcon( "trapbase",      JMConstants.ICON_TINKER,          "Traps"          );
		m_TypeFilter.AddIcon( "electricdevice",JMConstants.ICON_ELECTRIC,        "Electronics"    );
		m_TypeFilter.AddIcon( "tentbase",      JMConstants.ICON_CAMPING_TENT,    "Tents"          );
		m_TypeFilter.AddIcon( "grenade_base",  JMConstants.ICON_UNLIT_BOMB,      "Explosives"     );
		m_TypeFilter.AddIcon( "house",         JMConstants.ICON_HOME_GARAGE,     "Buildings"      );
		m_TypeFilter.AddIcon( "dz_lightai",    JMConstants.ICON_SHAMBLING_ZOMBIE,"AI"             );
		m_TypeFilter.SetSelected( "" );

		// Force the inner WrapSpacer to a known pixel width that fits all 27
		// chips (110px each + gaps) and a 30px height so chips never wrap to
		// a 2nd row. Size-To-Content-H wouldn't propagate through the nested
		// spacer chain otherwise.
		m_TypeFilter.ForceContentWidth( 27 * 115, 36 );

		// --- Row 3: Browse pane - list + preview ----------------------------
		// browseWrap is a 53%-of-form-height panel from the layout file. List
		// and preview are added directly as children with size=0.5 1 each so
		// they fill the panel without a GridSpacer (which would shrink to
		// content via Size-To-Content-V).
		m_ClassList = TextListboxWidget.Cast( g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/objectspawner_classlist.layout", browseWrap ) );
		m_ItemPreviewPanel = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/objectspawner_preview.layout", browseWrap );

		if ( m_ItemPreviewPanel )
			Class.CastTo( m_ItemPreview, m_ItemPreviewPanel.FindAnyWidget( "object_preview" ) );


		if ( !g_Game.IsMultiplayer() )
			m_ObjSpawnModeText.Insert("#STR_COT_OBJECT_MODULE_INVENTORY");
		else
			m_ObjSpawnModeText.Insert("#STR_COT_OBJECT_MODULE_SELECTED_PLAYERS");

		m_ObjSpawnModeText.Insert("#STR_COT_OBJECT_MODULE_SELECTED_OBJECTS");

		// ----------------------------------------------------------------------
		// BOTTOM AREA - inside actionsWrap (136px bottom-anchored).
		//
		// GridSpacer(4 rows, 1 col) with "Size To Content V": stacks its 4
		// children vertically, each row taking its child's natural height, and
		// the grid grows to fit (it does NOT divide height equally - proven by
		// JMExampleForm which stacks 16 varying-height widgets this way).
		//   rows 1-3 = sliders (30px each)
		//   row 4    = footer flex row (32px, horizontal: delete + dropdowns + spawn)
		// Slider cells fill the column width automatically, so no SetWidth(1.0) is
		// needed. The footer uses UIActionFlexRow for min-width + grow layout.
		// ----------------------------------------------------------------------
		m_SpawnerActionsWrapper = UIActionManager.CreateGridSpacer( actionsWrap, 4, 1 );

		// 3 sliders (one per row).
		m_QuantityItem = UIActionManager.CreateSlider( m_SpawnerActionsWrapper, "#STR_COT_OBJECT_MODULE_QUANTITY", 0, 100);
		m_QuantityItem.SetCurrent( 100 );

		m_HealthItem = UIActionManager.CreateSlider( m_SpawnerActionsWrapper, "#STR_COT_OBJECT_MODULE_HEALTH", 0, 100, this, "Click_SetHealth");
		m_HealthItem.SetStepValue( 1 );
		m_HealthItem.SetCurrent( 100 );

		m_TemperatureItem = UIActionManager.CreateSlider( m_SpawnerActionsWrapper, "#STR_COT_OBJECT_MODULE_TEMPERATURE", GameConstants.STATE_COLD_LVL_FOUR, GameConstants.STATE_HOT_LVL_FOUR, this, "Click_SetTemperature");
		m_TemperatureItem.SetSliderWidth(0.6);
		m_TemperatureItem.SetStepValue( 1 );
		m_TemperatureItem.SetFormat( "#STR_COT_FORMAT_DEGREE" );
		m_TemperatureItem.SetCurrent( GameConstants.STATE_NEUTRAL_TEMP );

		// Footer row - Refresh + delete icon + Mode + Setup + Spawn, laid out with the flex
		// system (UIActionFlexRow). Refresh + Delete are fixed 32px icons; Mode/Setup have a
		// pixel floor and small grow; Spawn has the biggest grow so it soaks up
		// leftover width but never shrinks below its min. m_Footer.Layout() is
		// called from OnResize to re-flow on window resize.
		m_Footer = UIActionManager.CreateFlexRow( m_SpawnerActionsWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		Widget footer = m_Footer.GetContent();

		m_RefreshListButton = UIActionManager.CreateRefreshButton( footer, this, "OnClick_RefreshList", "#STR_COT_GENERIC_REFRESH" );
		m_RefreshListButton.SetFixedSize( ICON_BUTTON_PX, ICON_BUTTON_PX );
		m_Footer.Add( m_RefreshListButton );

		UIActionConfirmInline deleteBtn = UIActionManager.CreateConfirmInline( footer, "", this, "DeleteCursor" );
		UIActionIconGrid.ApplyDeletePreset( deleteBtn );
		deleteBtn.SetButton( "" );
		deleteBtn.SetFixedSize( ICON_BUTTON_PX, ICON_BUTTON_PX );
		deleteBtn.CenterIcon( ICON_BUTTON_PX, 16 );
		deleteBtn.SetTooltip( "Delete the object under the cursor" );
		m_Footer.Add( deleteBtn );

		RegisterPermission( deleteBtn, "Entity.Delete" );

		m_SpawnMode = UIActionManager.CreateDropdown( footer, "Mode", layoutRoot, this, "ChangeSpawnMode", m_ObjSpawnModeText );
		m_SpawnMode.SetFlex( 1.0, 120 );   // min 120px, grows modestly
		m_SpawnMode.SetSelection(s_ObjSpawnMode, false);
		RegisterOverlay( m_SpawnMode );
		m_Footer.Add( m_SpawnMode );

		array<string> setupOptions = {"#STR_COT_OBJECT_MODULE_SPAWN_WITH_DEBUG", "#STR_COT_OBJECT_MODULE_SPAWN_WITH_CE", "#STR_COT_GENERIC_NONE"};
		m_ObjSetupMode = UIActionManager.CreateDropdown( footer, "Setup", layoutRoot, this, "Click_ObjSetupMode", setupOptions );
		m_ObjSetupMode.SetFlex( 1.0, 120 );   // min 120px, grows modestly
		m_ObjSetupMode.SetSelection(m_Module.m_ObjSetupMode, false);
		m_ObjSetupMode.SetTooltip( "Setup mode for spawning with attachments" );
		RegisterOverlay( m_ObjSetupMode );
		m_Footer.Add( m_ObjSetupMode );

		// Spawn button: min 160px so the label never truncates, biggest grow weight
		// so it absorbs most of the leftover width as the form widens - true
		// "min size + stretch to fill", which the raw WrapSpacer fraction couldn't do.
		m_SpawnButton = UIActionManager.CreateButton( footer, "#STR_COT_OBJECT_MODULE_SPAWN_ON", this, "Click_SpawnObject" );
		m_SpawnButton.SetFlex( 3.0, 160 );
		m_SpawnButton.SetColor( JMTheme.SUCCESS_FILL );
		m_SpawnButton.SetTooltip( "Spawn the selected item according to the chosen spawn mode" );
		m_Footer.Add( m_SpawnButton );

		// Sync the Spawn button label to whatever mode was last persisted -
		// ChangeSpawnMode flips the label to "Copy to clipboard" for clipboard
		// modes and back to "Spawn on ..." for spawn modes.
		SyncSpawnButtonLabel();

		m_SearchBox.SetText(m_Module.m_SearchText);
		UpdateList();

		UpdateItemPreview();
	}

	void Click_SetHealth( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateHealthItemColor();
		UpdateItemPreview();
	}

	void Click_SetTemperature( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateTemperatureItemColor();
		UpdateItemPreview();
	}

	void UpdateHealthControls(string type)
	{
		float maxHealth = MiscGameplayFunctions.GetTypeMaxGlobalHealth(type);
		if (maxHealth > 0)
		{
			m_HealthItem.Enable();

			float oldMax = m_HealthItem.GetMax();
			float pct;
			if (oldMax > 0)
				pct = m_HealthItem.GetCurrent() / oldMax;
			else
				pct = 1.0;

			m_HealthItem.SetMinMax(0, maxHealth);
			m_HealthItem.SetCurrent(pct * maxHealth);
		}
		else
		{
			m_HealthItem.Disable();
		}
	}

	void UpdateHealthItemColor()
	{
		if (!m_HealthItem.IsEnabled())
		{
			m_HealthItem.SetColor( JMTheme.VALUE_OK );
			return;
		}

		float health = m_HealthItem.GetCurrent();
		float health01;
		float sliderMax = m_HealthItem.GetMax();

		if (sliderMax > 0)
			health01 = health / sliderMax;

		if ( health01 >= 0.7 )			m_HealthItem.SetColor( Colors.COLOR_PRISTINE );
		else if ( health01 >= 0.5 )		m_HealthItem.SetColor( Colors.COLOR_WORN );
		else if ( health01 >= 0.3 )		m_HealthItem.SetColor( Colors.COLOR_DAMAGED );
		else if ( health01 > 0 )		m_HealthItem.SetColor( Colors.COLOR_BADLY_DAMAGED );
		else							m_HealthItem.SetColor( Colors.COLOR_RUINED );

		m_HealthItem.SetAlpha( 1.0 );
	}
	

	void UpdateTemperatureItemColor()
	{
		int value = m_TemperatureItem.GetCurrent();

		m_TemperatureItem.SetColor( ObjectTemperatureState.GetStateData(value).m_Color );
		if (ObjectTemperatureState.GetStateData(value).m_State != GameConstants.STATE_NEUTRAL_TEMP)
			m_TemperatureItem.SetFormat( "#STR_COT_FORMAT_DEGREE " + ObjectTemperatureState.GetStateData(value).m_LocalizedName );
		else
			m_TemperatureItem.SetFormat("#STR_COT_FORMAT_DEGREE");

		m_TemperatureItem.SetAlpha( 1.0 );
	}

	void Click_OnSafetyToogle( UIEvent eid, UIActionBase action )	
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.m_AllowRestrictedClassNames = action.IsChecked();
		UpdateList();
	}

	void Click_OnFilterDisplayName( UIEvent eid, UIActionBase action )	
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.m_FilterWithDisplayName = action.IsChecked();
		UpdateList();
	}

	void Click_ObjSetupMode( UIEvent eid, UIActionBase action )	
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Module.m_ObjSetupMode = action.GetSelection();

		UpdateItemPreview();
	}

	void Click_SpawnObject( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		SpawnObject(s_ObjSpawnMode);
	}


	void UpdateRotation( int mouse_x, int mouse_y, bool is_dragging )
	{
		m_Orientation[0] = m_Orientation[0] + ( ( m_MouseY - mouse_y ) * 0.01 );
		m_Orientation[1] = m_Orientation[1] - ( ( m_MouseX - mouse_x ) * 0.01 );
			
		m_ItemPreview.SetModelOrientation( m_Orientation );
	}

	void UpdateDistance( float wheel )
	{
		vector minMax[2];
		float radius = m_PreviewItem.ClippingInfo(minMax);

		m_Distance = m_Distance - (wheel * radius / 10.0);
		
		m_ItemPreview.SetModelPosition( Vector( m_Distance, 0, 0.5 + m_Distance ) );
	}

	void UpdateItemPreview()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::UpdateItemPreview" );
		#endif
		
		string strSelection = GetCurrentSelection();

		bool modeChanged = m_PreviewSetupMode != m_Module.m_ObjSetupMode;

		if (m_PreviewItem && (m_PreviewItem.GetType() != strSelection || modeChanged)) 
		{
			g_Game.ObjectDelete( m_PreviewItem );
			m_PreviewItem = null;
		}

		if (!m_PreviewItem)
		{
			m_PreviewItem = EntityAI.Cast( g_Game.CreateObject( strSelection, vector.Zero, true, false, false ) );
			m_PreviewSetupMode = m_Module.m_ObjSetupMode;

			if (m_PreviewItem)
			{
				dBodyActive(m_PreviewItem, ActiveState.INACTIVE);
				dBodyDynamic(m_PreviewItem, false);
				m_PreviewItem.DisableSimulation(true);

				// Give the preview entity the same attachment pass the spawn
				// itself would run, so a weapon shows its optic and magazine and
				// a vehicle its parts instead of a stripped model that never
				// matches what actually lands in the world.
				PlayerBase previewOwner = PlayerBase.Cast( g_Game.GetPlayer() );
				if ( previewOwner )
					m_Module.SetupEntityForMode( m_PreviewItem, previewOwner, m_Module.m_ObjSetupMode );

				m_ItemPreview.SetItem( m_PreviewItem );

				m_Distance = 0;
				m_Orientation = vector.Zero;
				m_ItemPreview.SetModelPosition( Vector( m_Distance, 0, 0.5 + m_Distance ) );
				m_ItemPreview.SetModelOrientation( vector.Zero );
				m_ItemPreview.SetView( m_ItemPreview.GetItem().GetViewIndex() );
				m_ItemPreview.Show( true );
			}
		}

		m_QuantityItem.Disable();
		UpdateHealthControls(strSelection);
		UpdateHealthItemColor();
		m_TemperatureItem.Disable();

		if ( m_PreviewItem )
		{
			// Same pass the spawn runs: the value lands on the item, on its
			// damage zones, and on every attachment at the same PERCENTAGE of
			// that attachment's own max health. Without the cascade a ruined
			// rifle previews with a pristine optic and magazine hanging off it.
			// temp stays -1 - the temperature slider is a spawn-time value and
			// has nothing to show in the preview.
			if (m_HealthItem.IsEnabled() && !m_PreviewItem.IsTransport())
				m_Module.SetupEntityHealth( m_PreviewItem, m_HealthItem.GetCurrent(), -1 );

			if (m_PreviewItem.IsInherited(ItemBase))
			{
				ItemBase item = ItemBase.Cast(m_PreviewItem);

				if ( item.IsLiquidContainer() || ( item.HasFoodStage() && item.CanBeCooked() ) )
					m_TemperatureItem.Enable();

				// Snapshot quantity percentage before range changes so switching items scales the value.
				float qOldMin = m_QuantityItem.GetMin();
				float qOldMax = m_QuantityItem.GetMax();
				float qOldRange = qOldMax - qOldMin;
				float qPct;
				if (qOldRange > 0)
					qPct = (m_QuantityItem.GetCurrent() - qOldMin) / qOldRange;
				else
					qPct = 1.0;

				Magazine mag;
				if (Class.CastTo(mag, item))
				{
					if (mag.GetAmmoMax() > 0)
					{
						float min;

						if ( mag.IsAmmoPile() && mag.GetAmmoMax() > 1 )
							min = 1.0;
						else
							min = 0.0;

						m_QuantityItem.SetMinMax(min, mag.GetAmmoMax());
						m_QuantityItem.SetStepValue(1);
						m_QuantityItem.SetCurrent(min + qPct * (mag.GetAmmoMax() - min));
						m_QuantityItem.Enable();
					}
				}
				else if (item.GetQuantityMax() - item.GetQuantityMin() > 0)
				{
					if ( item.IsSplitable() )
						m_QuantityItem.SetStepValue(1);
					else
						m_QuantityItem.SetStepValue(0.1);

					float qNewMin = item.GetQuantityMin();
					float qNewMax = item.GetQuantityMax();
					m_QuantityItem.SetMinMax(qNewMin, qNewMax);
					m_QuantityItem.SetCurrent(qNewMin + qPct * (qNewMax - qNewMin));
					m_QuantityItem.Enable();
				}
			}
		}
		else
		{
			m_ItemPreview.Show( false );
		}

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::UpdateItemPreview" );
		#endif
	}

	override void OnFocus()
	{
		super.OnFocus();

		m_ItemPreview.Show(true);
	}

	override void OnUnfocus()
	{
		super.OnUnfocus();

		m_ItemPreview.Show(false);
	}

	override bool OnItemSelected( Widget w, int x, int y, int row, int column, int oldRow, int oldColumn )
	{
		if ( w == m_ClassList )
		{
			UpdateItemPreview();

			return true;
		}

		return false;
	}

	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		super.OnMouseButtonDown( w, x, y, button );

		if ( w == m_ItemPreview && button == MouseState.LEFT )
		{
			g_Game.GetDragQueue().Call( this, "UpdateRotation" );
			GetMousePos( m_MouseX, m_MouseY );

			return true;
		}

		return false;
	}

	override bool OnMouseWheel(Widget  w, int  x, int  y, int wheel)
	{
		if ( w == m_ItemPreview && m_PreviewItem )
		{
			UpdateDistance(wheel);

			return true;
		}

		return super.OnMouseWheel( w, x, y, wheel );
	}

	override bool OnDoubleClick( Widget w, int x, int y, int button )
	{
		super.OnDoubleClick( w, x, y, button );

		if ( w == m_ClassList && button == MouseState.LEFT )
		{
			SpawnObject(s_ObjSpawnMode);
			
			return true;
		}

		return false;
	}

	void OnClick_TypeFilter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.m_CurrentType = m_TypeFilter.GetLastClickedId();

		UpdateList();
	}

	void OnClick_RefreshList( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_RefreshListButton )
			m_RefreshListButton.TriggerSpin( 2 );

		UpdateList();
	}

	// Reflect the current s_ObjSpawnMode on the Spawn button label + enable/disable
	// the Setup dropdown (clipboard modes don't use setup options).
	void SyncSpawnButtonLabel()
	{
		string label;
		switch (s_ObjSpawnMode)
		{
			case COT_ObjectSpawnerMode.CURSOR:
				label = "Spawn at Crosshair";
				m_ObjSetupMode.Enable();
				break;
			case COT_ObjectSpawnerMode.PLAYER_POSITION:
				label = "Spawn at Player";
				m_ObjSetupMode.Enable();
				break;
			case COT_ObjectSpawnerMode.TARGET_INVENTORY:
				label = "Spawn at Target";
				m_ObjSetupMode.Enable();
				break;
			case COT_ObjectSpawnerMode.PLAYER_INVENTORY:
				if (g_Game.IsMultiplayer())
					label = "Spawn in Selected Player(s)";
				else
					label = "Spawn in Own Inventory";
				m_ObjSetupMode.Enable();
				break;
			case COT_ObjectSpawnerMode.OBJECT_INVENTORY:
				label = "Spawn in Selected Object(s)";
				m_ObjSetupMode.Enable();
				break;
			case COT_ObjectSpawnerMode.COPYLISTRAW:
				label = "Copy Class List";
				m_ObjSetupMode.Disable();
				break;
			case COT_ObjectSpawnerMode.COPYLISTTYPES:
				label = "Copy as types.xml";
				m_ObjSetupMode.Disable();
				break;
		#ifdef DZ_Expansion_Market
			case COT_ObjectSpawnerMode.COPYLISTEXPMARKET:
				label = "Copy as Market Category";
				m_ObjSetupMode.Disable();
				break;
		#endif
			default:
				label = "Spawn";
				m_ObjSetupMode.Enable();
				break;
		}
		m_SpawnButton.SetButton( label );
	}

	void ChangeSpawnMode( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		s_ObjSpawnMode = action.GetSelection();
		SyncSpawnButtonLabel();
	}

	void SpawnObject(int mode = COT_ObjectSpawnerMode.CURSOR)
	{
		string clipboardOutput = "";
		string result;

		string selection = GetCurrentSelection();
		#ifdef DZ_Expansion_Market
		if ( selection == "" && mode != COT_ObjectSpawnerMode.COPYLISTRAW && mode != COT_ObjectSpawnerMode.COPYLISTTYPES && mode != COT_ObjectSpawnerMode.COPYLISTEXPMARKET)
		#else
		if ( selection == "" && mode != COT_ObjectSpawnerMode.COPYLISTRAW && mode != COT_ObjectSpawnerMode.COPYLISTTYPES)
		#endif
			
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "No class selected. Use the search bar to find a class, then select it from the list." ) );
			return;
		}

		float health = m_HealthItem.GetCurrent();
		float temp = m_TemperatureItem.GetCurrent();
		float quantity = m_QuantityItem.GetCurrent();

		switch (mode)
		{
			default:
			case COT_ObjectSpawnerMode.PLAYER_POSITION:
				m_Module.SpawnEntity_Position(GetCurrentSelection(), g_Game.GetPlayer().GetPosition(), quantity, health, temp, 0);
				break;

			case COT_ObjectSpawnerMode.CURSOR:
				m_Module.SpawnEntity_Position(GetCurrentSelection(), GetCursorPos(), quantity, health, temp, 0);
				break;

			case COT_ObjectSpawnerMode.TARGET_INVENTORY:
				m_Module.SpawnEntity_Position(GetCurrentSelection(), GetCursorPos(), quantity, health, temp, 0, true);
				break;

			case COT_ObjectSpawnerMode.PLAYER_INVENTORY:
				m_Module.SpawnEntity_Inventory(GetCurrentSelection(), JM_GetSelected().GetPlayers(), quantity, health, temp, 0);
				break;

			case COT_ObjectSpawnerMode.OBJECT_INVENTORY:
				m_Module.SpawnEntity_Inventory(GetCurrentSelection(), JM_GetSelected().GetObjects(), quantity, health, temp, 0);
				break;

			case COT_ObjectSpawnerMode.COPYLISTRAW:
				for (int i = 0; i < m_ClassList.GetNumItems(); i++)
				{
					m_ClassList.GetItemText(i, 0, result);
					clipboardOutput += result + "\n";
				}
				g_Game.CopyToClipboard(clipboardOutput);
				break;

			case COT_ObjectSpawnerMode.COPYLISTTYPES:
				clipboardOutput = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
				clipboardOutput += "<types>\n";
				for (int j = 0; j < m_ClassList.GetNumItems(); j++)
				{
					m_ClassList.GetItemText(j, 0, result);
					clipboardOutput += "	<type name=\"" + result + "\">\n";
					clipboardOutput += "		<nominal>0</nominal>\n";
					clipboardOutput += "		<lifetime>3888000</lifetime>\n";
					clipboardOutput += "		<restock>0</restock>\n";
					clipboardOutput += "		<min>0</min>\n";
					clipboardOutput += "		<quantmin>-1</quantmin>\n";
					clipboardOutput += "		<quantmax>-1</quantmax>\n";
					clipboardOutput += "		<cost>100</cost>\n";
					clipboardOutput += "		<flags count_in_cargo=\"0\" count_in_hoarder=\"0\" count_in_map=\"1\" count_in_player=\"0\" crafted=\"0\" deloot=\"0\"/>\n";
					clipboardOutput += "	</type>\n";
				}
				clipboardOutput += "</types>";
				g_Game.CopyToClipboard(clipboardOutput);
				break;

		#ifdef DZ_Expansion_Market
			case COT_ObjectSpawnerMode.COPYLISTEXPMARKET:
				string categoryJSON;

				auto category = new ExpansionMarketCategory();
				category.Defaults();
				category.DisplayName = m_SearchBox.GetText();

				for (int k = 0; k < m_ClassList.GetNumItems(); k++)
				{
					m_ClassList.GetItemText(k, 0, result);
					auto item = new ExpansionMarketItem(-1, result, 100, 100, 1, 1);
					category.Items.Insert(item);
				}

				string errorMsg;
				if (JsonFileLoader<ExpansionMarketCategory>.MakeData(category, categoryJSON, errorMsg))
					g_Game.CopyToClipboard(categoryJSON);
				else
					COTCreateLocalAdminNotification(new StringLocaliser(errorMsg));
				break;
		#endif
		}
	}

	void DeleteCursor( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		DeleteCursor( m_Module.GetObjectAtCursor() );
	}

	void DeleteCursor( Object obj )
	{
		if ( !obj )
			return;

		m_Module.DeleteEntity( obj );

		if ( m_Module.m_AutoShow )
			g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).Call( m_Module.Hide );
	}

	void SearchInput_OnChange( UIEvent eid, UIActionBase action )
	{
		m_Module.m_SearchText = m_SearchBox.GetText();
		UpdateList();
	}

	protected void ReportExactClassRejection( string searchText, TStringArray configs )
	{
		string searchLower = searchText;
		searchLower.ToLower();

		foreach ( string cfgPath: configs )
		{
			int count = g_Game.ConfigGetChildrenCount( cfgPath );
			for ( int i = 0; i < count; i++ )
			{
				string name;
				g_Game.ConfigGetChildName( cfgPath, i, name );

				string nameLower = name;
				nameLower.ToLower();

				if ( nameLower != searchLower )
					continue;

				int scope = g_Game.ConfigGetInt( cfgPath + " " + name + " scope" );
				if ( scope == 0 || (scope == 1 && !m_Module.m_AllowRestrictedClassNames) )
				{
					COTCreateLocalAdminNotification( new StringLocaliser( name + " exists but has scope < 2 (private/modder-only). Enable 'Show Unsafe' to reveal it." ) );
					return;
				}

				string model;
				if ( !g_Game.ConfigGetText( cfgPath + " " + name + " model", model ) || model == string.Empty || model == "bmp" )
				{
					COTCreateLocalAdminNotification( new StringLocaliser( name + " exists but has no valid model and cannot be spawned." ) );
					return;
				}

				if ( m_Module.IsExcludedClassName( nameLower ) )
				{
					COTCreateLocalAdminNotification( new StringLocaliser( name + " is on the spawner blacklist. Enable 'Show Unsafe' to bypass it." ) );
					return;
				}

				// Class exists and passed all filters - no rejection to report.
				return;
			}
		}
	}

	void UpdateList()
	{
	#ifdef DIAG
		int ticks = TickCount(0);
	#endif

		m_ClassList.ClearItems();
		string closestMatch;

		TStringArray configs = new TStringArray;
		configs.Insert( CFG_VEHICLESPATH );
		configs.Insert( CFG_WEAPONSPATH );
		configs.Insert( CFG_MAGAZINESPATH );

		COT_String strSearch = m_Module.m_SearchText;
		bool requireAllKeywords;
		TStringArray keywords = strSearch.KeywordSearch_Prepare(requireAllKeywords);

		for ( int nConfig; nConfig < configs.Count(); nConfig++ )
		{
			string strConfigPath = configs.Get( nConfig );

			int nClasses = g_Game.ConfigGetChildrenCount( strConfigPath );

			for ( int nClass = 0; nClass < nClasses; nClass++ )
			{
				string strName;

				g_Game.ConfigGetChildName( strConfigPath, nClass, strName );

				string strNameLower = strName;
				strNameLower.ToLower();

				int scope = g_Game.ConfigGetInt( strConfigPath + " " + strName + " scope" );

				if ( scope == 0 || (scope == 1 && !m_Module.m_AllowRestrictedClassNames) )
					continue;

				string model;
				if (!g_Game.ConfigGetText(strConfigPath + " " + strName + " model", model) || model == string.Empty || model == "bmp")
					continue;

				if (m_Module.m_CurrentType == "" || g_Game.IsKindOf( strNameLower, m_Module.m_CurrentType ) )
				{
					if ( m_Module.IsExcludedClassName( strNameLower ) )
						continue;

					COT_String strNameSearch = strNameLower;
					if (m_Module.m_FilterWithDisplayName)
					{
						if (!g_Game.ConfigGetText(strConfigPath + " " + strName + " displayName", strNameSearch))
							continue;

						strNameSearch.ToLower();
					}

					if ( strSearch != "" && !strNameSearch.KeywordSearchImplEx(strSearch, keywords, requireAllKeywords, closestMatch) )
						continue;

					m_ClassList.AddItem( strName, NULL, 0 );
				}
			}
		}

		//if ( strSearch != "" && m_ClassList.GetNumItems() == 0 )
			ReportExactClassRejection( strSearch, configs );

	#ifdef DIAG
		float elapsed = TickCount(ticks) * 0.0001;
		PrintFormat("UpdateList %1 %2 ms", m_Module.m_SearchText, elapsed);
	#endif

		m_SearchBox.SetTextPreview(closestMatch);
	}

	string GetCurrentSelection()
	{
		if ( m_ClassList.GetSelectedRow() != -1 )
		{
			string result;
			m_ClassList.GetItemText( m_ClassList.GetSelectedRow(), 0, result );
			return result;
		}

		return "";
	}
}
