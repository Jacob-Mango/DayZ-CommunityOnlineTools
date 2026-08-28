// =============================================================================
//  JMPreviewLabMenu
//
//  Tentative H3 of the Preview Lab: the same ItemPreviewWidget and MapWidget,
//  hosted inside a real UIScriptedMenu entered through the UIManager.
//
//  WHY THIS EXISTS
//
//  Every other tentative - ten of them - draws nothing, including the two that
//  parent a copy straight onto the workspace root. But the workspace root is
//  the SAME context COT's own windows live in: HUD level, no menu. So none of
//  those tests ever moved the one variable that separates COT from every
//  vanilla usage of these two widget types.
//
//  In vanilla there is no exception: MapMenu, the inventory, InspectMenuNew and
//  the radial quickbar are all UIScriptedMenus entered through
//  UIManager.EnterScriptedMenu. Not one map or item preview in the base game
//  lives outside a scripted menu.
//
//  If the widgets draw here and nowhere else, the host IS the answer, and no
//  layout attribute was ever going to fix it - COT would have to host its
//  windows (or at least these widgets) in a scripted menu.
//
//  If they are blank here too, the host is eliminated as well, and what is left
//  is the entity and the engine's own map data - not the UI layer at all.
// =============================================================================
class JMPreviewLabMenu extends UIScriptedMenu
{
	//! Well clear of vanilla's MenuID enum and of Expansion's range.
	static const int MENU_ID = 8123;

	//! Same class the in-form tentatives use, for a like-for-like comparison.
	static const string LAB_ITEM = "Mich2001Helmet";

	protected ItemPreviewWidget m_Preview;
	protected MapWidget         m_LabMap;
	protected EntityAI          m_Item;
	protected ButtonWidget      m_CloseButton;

	override Widget Init()
	{
		layoutRoot = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/previewlab/menu_host.layout" );

		if ( !layoutRoot )
		{
			Print( "[COT-LAB] H3 menu layout failed to load" );
			return NULL;
		}

		Class.CastTo( m_Preview,     layoutRoot.FindAnyWidget( "lab_preview" ) );
		Class.CastTo( m_LabMap,      layoutRoot.FindAnyWidget( "lab_map"     ) );
		Class.CastTo( m_CloseButton, layoutRoot.FindAnyWidget( "lab_close"   ) );

		SetupPreview();
		SetupMap();

		return layoutRoot;
	}

	protected void SetupPreview()
	{
		if ( !m_Preview )
		{
			Print( "[COT-LAB] H3: no lab_preview in the menu layout" );
			return;
		}

		m_Item = EntityAI.Cast( g_Game.CreateObject( LAB_ITEM, vector.Zero, true, false, false ) );

		if ( !m_Item )
		{
			Print( "[COT-LAB] H3: could not spawn " + LAB_ITEM );
			return;
		}

		dBodyActive( m_Item, ActiveState.INACTIVE );
		dBodyDynamic( m_Item, false );
		m_Item.DisableSimulation( true );

		m_Preview.SetItem( m_Item );
		m_Preview.SetView( m_Item.GetViewIndex() );
		m_Preview.SetModelPosition( Vector( 0, 0, 1 ) );
		m_Preview.Show( true );

		float pw, ph;
		m_Preview.GetScreenSize( pw, ph );
		Print( "[COT-LAB] H3 preview: size=" + pw + "x" + ph + " item=" + m_Item.GetType() );
	}

	protected void SetupMap()
	{
		if ( !m_LabMap )
		{
			Print( "[COT-LAB] H3: no lab_map in the menu layout" );
			return;
		}

		vector center = "7500 0 7500";

		Man player = g_Game.GetPlayer();
		if ( player )
			center = player.GetPosition();

		m_LabMap.SetScale( 0.4 );
		m_LabMap.SetMapPos( center );

		float mw, mh;
		m_LabMap.GetScreenSize( mw, mh );
		Print( "[COT-LAB] H3 map: size=" + mw + "x" + mh + " mapPos=" + m_LabMap.GetMapPos() );
	}

	override void OnShow()
	{
		super.OnShow();

		g_Game.GetInput().ChangeGameFocus( 1 );
		g_Game.GetUIManager().ShowUICursor( true );
	}

	override void OnHide()
	{
		super.OnHide();

		// The COT window that opened this is still up and still wants a cursor,
		// so the focus is handed back rather than dropped.
		g_Game.GetInput().ChangeGameFocus( -1 );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == m_CloseButton )
		{
			Close();
			return true;
		}

		return super.OnClick( w, x, y, button );
	}

	void ~JMPreviewLabMenu()
	{
		if ( g_Game && m_Item )
			g_Game.ObjectDelete( m_Item );
	}
}
