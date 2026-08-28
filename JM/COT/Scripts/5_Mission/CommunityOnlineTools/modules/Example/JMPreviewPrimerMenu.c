// =============================================================================
//  JMPreviewPrimerMenu
//
//  Tentative T13: why did pressing T10/T11 make everything else render?
//
//  THE OBSERVATION
//
//  Attaching a preview and a map to the render layer made every other tentative
//  in the Example window start drawing - M2, M3 and T1-T9, all plain workspace
//  widgets that had not been touched. So something about having those two
//  widgets alive inside an open menu switches the engine's map/preview render
//  pass on globally.
//
//  The first attempt at exploiting that was a 4x4 pixel menu holding one of
//  each widget. It did nothing. So mere existence inside an open menu is not
//  the trigger either - the widgets that worked differed in more than one way:
//
//    - they were large (roughly 733x347), not 4x4
//    - the preview had an entity, a view index, a model position and Show(true)
//    - the map had a scale and a world position
//    - both were created at runtime into the menu, not declared in its layout
//
//  This menu removes every difference except size. It builds the SAME host
//  layouts the render layer builds, drives them with the SAME calls, and only
//  the dimensions change. T13 cycles the size on each press and logs it, so the
//  point where the pass stops being switched on is directly observable.
//
//  If 256 works and 16 does not, the engine is culling render targets below
//  some size and the fix is a primer big enough to survive that. If every size
//  works, the 4x4 attempt failed for a different reason - most likely that its
//  widgets were declared in the layout and never driven by script.
// =============================================================================
class JMPreviewPrimerMenu extends UIScriptedMenu
{
	//! Clear of vanilla's MenuID enum, of Expansion's range and of the lab ids.
	static const int MENU_ID = 8126;

	//! Same item the rest of the lab uses, for a like-for-like comparison.
	static const string PRIMER_ITEM = "Mich2001Helmet";

	//! Edge length in pixels of the primer widgets. T13 steps through these.
	static const ref array<int> SIZE_STEPS = { 512, 256, 64, 16, 4 };

	static int s_SizeStep = 0;

	static JMPreviewPrimerMenu s_Instance;

	protected ItemPreviewWidget m_Preview;
	protected MapWidget         m_Map;
	protected EntityAI          m_Item;

	override Widget Init()
	{
		layoutRoot = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/previewlab/probe_host.layout" );

		if ( !layoutRoot )
		{
			Print( "[COT-LAB] T13 primer: layout failed to load" );
			return NULL;
		}

		int size = SIZE_STEPS[s_SizeStep];

		BuildPreview( size );
		BuildMap( size );

		s_Instance = this;

		Print( "[COT-LAB] T13 primer open at " + size.ToString() + "px, preview=" + ( m_Preview != NULL ).ToString() + " map=" + ( m_Map != NULL ).ToString() );

		return layoutRoot;
	}

	//! Built exactly the way the render layer builds its entries, because the
	//! render layer is the thing that demonstrably works.
	protected void BuildPreview( int size )
	{
		Widget host = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/previewlab/layer_preview_host.layout", layoutRoot );

		if ( !host )
			return;

		host.SetPos( 0, 0, true );
		host.SetSize( size, size, true );

		Class.CastTo( m_Preview, host.FindAnyWidget( "layer_preview" ) );

		if ( !m_Preview )
			return;

		vector spawnAt = vector.Zero;

		Man self = g_Game.GetPlayer();
		if ( self )
			spawnAt = self.GetPosition();

		m_Item = EntityAI.Cast( g_Game.CreateObject( PRIMER_ITEM, spawnAt, true, false, false ) );

		if ( !m_Item )
			return;

		m_Item.DisableSimulation( true );

		m_Preview.SetItem( m_Item );
		m_Preview.SetView( m_Item.GetViewIndex() );
		m_Preview.SetModelPosition( Vector( 0, 0, 1 ) );
		m_Preview.SetModelOrientation( vector.Zero );
		m_Preview.Show( true );
	}

	protected void BuildMap( int size )
	{
		Widget host = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/previewlab/layer_map_host.layout", layoutRoot );

		if ( !host )
			return;

		host.SetPos( size + 8, 0, true );
		host.SetSize( size, size, true );

		Class.CastTo( m_Map, host.FindAnyWidget( "layer_map" ) );

		if ( !m_Map )
			return;

		vector center = "7500 0 7500";

		Man player = g_Game.GetPlayer();
		if ( player )
			center = player.GetPosition();

		m_Map.SetScale( 0.4 );
		m_Map.SetMapPos( center );
	}

	//! The primer never takes focus or the cursor - COT owns both already.
	override void OnShow()
	{
	}

	override void OnHide()
	{
		s_Instance = NULL;
	}

	static void Open()
	{
		if ( s_Instance )
			return;

		g_Game.GetUIManager().EnterScriptedMenu( MENU_ID, NULL );
	}

	static void CloseIfOpen()
	{
		if ( !s_Instance )
			return;

		s_Instance.Close();
	}

	static bool IsOpen()
	{
		return s_Instance != NULL;
	}

	void ~JMPreviewPrimerMenu()
	{
		if ( g_Game && m_Item )
			g_Game.ObjectDelete( m_Item );
	}
}
