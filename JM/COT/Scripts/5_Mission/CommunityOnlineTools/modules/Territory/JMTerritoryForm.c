#ifdef EXPANSIONMODBASEBUILDING
class JMTerritoryForm: JMFormBase
{
	//! protected, not private: sub-mods reach for the module through the form.
	protected JMTerritoryModule m_Module;

	protected UIActionScroller m_sclr_Territories;
	protected Widget m_ContentWrapper;
	protected Widget m_TerritoriesWrapper;

	protected UIActionSearchBox m_SearchBar;
	protected string m_SearchFilter;

	override void OnInit()
	{
		InitWidgetsTop();
		InitWidgetsBottom();
	}

	//! Archetype B: a fixed filter/toolbar header over the list. The toolbar is
	//! built once, so a settings refresh cannot pull the search box out from
	//! under the caret mid-typing.
	protected void InitWidgetsTop()
	{
		Widget toolbar = UIActionManager.CreateWrapSpacer( layoutRoot.FindAnyWidget( "panel_top" ), WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		UIActionManager.CreateRefreshButton( toolbar, this, "OnClick_Refresh", "#STR_COT_TERRITORY_TT_REFRESH" );

		m_SearchBar = UIActionManager.CreateSearchBox( toolbar, this, "OnChange_Search", "#STR_COT_GENERIC_SEARCH" );
		m_SearchBar.SetWidth( 0.85 );
		m_SearchBar.SetFixedHeight( HEADER_CONTROL_HEIGHT );
	}

	protected void InitWidgetsBottom()
	{
		m_sclr_Territories = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel_bottom" ) );
		m_ContentWrapper   = m_sclr_Territories.GetContentWidget();

		m_sclr_Territories.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_sclr_Territories )
			m_sclr_Territories.UpdateScroller();
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnShow()
	{
		super.OnShow();

		m_Module.RequestTerritories();
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();

		// The per-row buttons are created and destroyed with the list, so they
		// cannot be bound once - the list is rebuilt against the new rights.
		RebuildTerritoryList();
	}

	override void OnSettingsUpdated()
	{
		RebuildTerritoryList();
	}

	protected void RebuildTerritoryList()
	{
		if ( !m_ContentWrapper )
			return;

		if ( m_TerritoriesWrapper )
			delete m_TerritoriesWrapper;

		m_TerritoriesWrapper = UIActionManager.CreateGridSpacer( m_ContentWrapper, 1, 1 );

		bool canSetLevel = GetPermissionsManager().HasPermission( "Expansion.Territory.SetLevel" );

		if ( !m_Module.m_Territories || m_Module.m_Territories.Count() < 1 )
		{
			UIActionManager.CreateText( m_TerritoriesWrapper, "#STR_COT_TERRITORY_NONE_FOUND" );
			m_sclr_Territories.UpdateScroller();
			return;
		}

		UIActionCard card = UIActionManager.CreateCard( m_TerritoriesWrapper, "#STR_COT_TERRITORY_SECTION_LIST" );
		Widget listBody = card.GetContent();

		string filter = m_SearchFilter;
		filter.ToLower();

		string nameLower;

		foreach ( JMTerritoryData data: m_Module.m_Territories )
		{
			if ( filter != "" )
			{
				nameLower = data.TerritoryName;
				nameLower.ToLower();

				if ( nameLower.IndexOf( filter ) == -1 )
					continue;
			}

			Widget row = UIActionManager.CreateWrapSpacer( listBody, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

			string label = data.TerritoryName + " [" + data.CurrentLevel + "/" + data.MaxLevel + "]";
			UIActionText nameField = UIActionManager.CreateText( row, "", label );
			nameField.SetWidth( 0.70 );
			nameField.SetTextVAlign( UIActionVAlign.CENTER );

			// Lucide glyphs rather than the literal arrow characters these rows
			// used to carry: a non-ASCII byte anywhere in a .c poisons the
			// Enforce type-pool for the whole module.
			if ( canSetLevel && data.CurrentLevel > 1 )
			{
				UIActionImageButton downBtn = UIActionManager.CreateIconButton( row, JMConstants.Lucide( "chevron-down" ), this, "DowngradeTerritory" );
				downBtn.SetFixedSize( HEADER_ACTION_PX, HEADER_ACTION_PX );
				downBtn.SetData( data );
				downBtn.SetTooltip( "#STR_COT_TERRITORY_TT_DOWNGRADE" );
			}

			if ( canSetLevel && data.CurrentLevel < data.MaxLevel )
			{
				UIActionImageButton upBtn = UIActionManager.CreateIconButton( row, JMConstants.Lucide( "chevron-up" ), this, "UpgradeTerritory" );
				upBtn.SetFixedSize( HEADER_ACTION_PX, HEADER_ACTION_PX );
				upBtn.SetData( data );
				upBtn.SetTooltip( "#STR_COT_TERRITORY_TT_UPGRADE" );
			}
		}

		m_sclr_Territories.UpdateScroller();
	}

	void OnChange_Search( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_SearchBar )
			m_SearchFilter = m_SearchBar.GetText();
		else
			m_SearchFilter = "";

		RebuildTerritoryList();
	}

	void OnClick_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		action.AnimateSpin( 2 );

		m_Module.RequestTerritories();
	}

	void UpgradeTerritory( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !GetPermissionsManager().HasPermission( "Expansion.Territory.SetLevel" ) )
			return;

		JMTerritoryData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		action.AnimateFeedback();

		m_Module.SendSetLevel( data.TerritoryID, data.CurrentLevel + 1 );
		data.CurrentLevel++;
		RebuildTerritoryList();
	}

	void DowngradeTerritory( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !GetPermissionsManager().HasPermission( "Expansion.Territory.SetLevel" ) )
			return;

		JMTerritoryData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		action.AnimateFeedback();

		m_Module.SendSetLevel( data.TerritoryID, data.CurrentLevel - 1 );
		data.CurrentLevel--;
		RebuildTerritoryList();
	}
}
#endif
