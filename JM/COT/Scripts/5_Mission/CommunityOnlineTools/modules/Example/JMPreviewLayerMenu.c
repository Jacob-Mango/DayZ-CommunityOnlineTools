// =============================================================================
//  JMPreviewLayerMenu
//
//  Tentative T10: the narrow fix.
//
//  The constraint is fixed and not ours to argue with: the engine renders
//  ItemPreviewWidget and MapWidget only inside a UIScriptedMenu's widget tree.
//  H3 draws, H1/H2 at the workspace root do not, and H4 showed that merely
//  having a menu open changes nothing.
//
//  Hosting COT's windows in a menu satisfies that, but it drags the whole
//  window lifecycle - creation, ESC, CloseAll, focus - along with it, which is
//  far too much machinery for two widget types.
//
//  This is the alternative. One shared, invisible, click-through menu acts as a
//  render layer. COT windows stay exactly where they are on the workspace; they
//  keep a plain empty panel as a placeholder, and the actual preview widget
//  lives in this menu and is pinned to that placeholder's screen rectangle
//  every frame. Nothing about windows, focus or ESC changes, and modules keep
//  their existing layouts.
//
//  KNOWN LIMITS OF THE APPROACH
//
//  A widget in this layer is not a child of the window, so it is not clipped by
//  the window and not ordered against other windows. The layer sits above every
//  module window on purpose - below them its contents are simply buried by the
//  window backgrounds - which means a preview inside a scroller will not be cut
//  off at the scroller's edge, and a preview belonging to a window that is
//  behind another window still draws on top. Update() hides an entry whose
//  anchor is hidden or has collapsed to nothing, which covers the common cases;
//  true clipping would need the anchor's visible rectangle intersected against
//  its ancestors.
// =============================================================================
class JMPreviewLayerMenu extends UIScriptedMenu
{
	//! Clear of vanilla's MenuID enum, of Expansion's range and of the lab ids.
	static const int MENU_ID = 8125;

	static JMPreviewLayerMenu s_Instance;

	//! Sort applied to the layer root when it opens, or -1 to leave the sort
	//! alone. Set from the lab before the layer opens; see Init.
	//!
	//! COT's window container sits at SORT_WINDOW (920), so anything below that
	//! leaves the layer buried and anything above should cover it. 970 renders
	//! nothing at all, which is what these steps are for: they separate "any
	//! SetSort on a menu root breaks the render pass" from "only a sort past
	//! some threshold does".
	static int s_SortStep = 0;

	static const ref array<int> SORT_STEPS = { -1, 921, 930, 970 };

	//! Placeholder inside a COT window, and the preview pinned to it. Index i of
	//! one matches index i of the other.
	protected ref array<Widget> m_Anchors;
	protected ref array<Widget> m_Hosts;

	//! One-shot guard so a zero-sized anchor does not spam the log every frame.
	protected bool m_WarnedEmpty;

	//! Counts the first few sync passes, which are the only ones logged.
	protected int m_SyncLogged;

	//! COT's window sort, saved while the layer holds it down.
	protected int  m_SavedWindowSort;
	protected bool m_WindowSortSaved;

	void JMPreviewLayerMenu()
	{
		m_Anchors = new array<Widget>;
		m_Hosts   = new array<Widget>;
	}

	override Widget Init()
	{
		layoutRoot = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/previewlab/probe_host.layout" );

		if ( !layoutRoot )
		{
			Print( "[COT-LAB] T10 render layer: layout failed to load" );
			return NULL;
		}

		//! A menu's layoutRoot is created with no parent, which makes it a
		//! workspace sibling of JMStatics.WINDOWS_CONTAINER - and COT gives that
		//! SORT_WINDOW, so at the default sort of 0 the layer draws underneath
		//! every module window and is hidden by their backgrounds.
		//!
		//! Raising it fixes that ordering but appears to stop the map and the
		//! preview drawing at all, which is why it is behind a switch: the map
		//! rendered fine at the default sort, just buried. Toggle it from the
		//! lab to see both behaviours in one session.
		int sort = SORT_STEPS[s_SortStep];

		if ( sort >= 0 )
			layoutRoot.SetSort( sort );

		//! Both roots are workspace siblings, so their sorts decide who covers
		//! whom. Log the real numbers rather than reasoning about them.
		int layerSort = layoutRoot.GetSort();
		int windowSort = -1;

		if ( JMStatics.WINDOWS_CONTAINER )
			windowSort = JMStatics.WINDOWS_CONTAINER.GetSort();

		Print( "[COT-LAB] render layer open, sort step " + s_SortStep + " = " + sort + " | layer sort=" + layerSort + " windows sort=" + windowSort );

		LowerWindows();


		s_Instance = this;

		return layoutRoot;
	}

	//! The layer never takes focus or the cursor. COT already owns both, and the
	//! panel ignores the pointer, so clicks pass straight through to the window
	//! underneath.
	override void OnShow()
	{
	}

	override void OnHide()
	{
		RestoreWindows();

		s_Instance = NULL;
	}

	//! Raising the layer's own sort past COT's windows stops the map and the
	//! preview rendering altogether, so the stacking is fixed from the other
	//! end: COT's window container is dropped underneath the layer for as long
	//! as the layer is up, and put back exactly as it was afterwards.
	//!
	//! COT set SORT_WINDOW so the in-game HUD would not cover module windows,
	//! and this gives that up while a preview is on screen. That trade only
	//! lasts as long as the layer does.
	protected void LowerWindows()
	{
		if ( !JMStatics.WINDOWS_CONTAINER )
			return;

		if ( m_WindowSortSaved )
			return;

		m_SavedWindowSort = JMStatics.WINDOWS_CONTAINER.GetSort();
		m_WindowSortSaved = true;

		JMStatics.WINDOWS_CONTAINER.SetSort( 0 );
	}

	//! OnHide is the normal path, but a menu torn down by CloseAll or by the
	//! mission ending may not see it. Leaving COT's windows stuck at sort 0
	//! would put the HUD over them for the rest of the session.
	void ~JMPreviewLayerMenu()
	{
		RestoreWindows();
	}

	protected void RestoreWindows()
	{
		if ( !m_WindowSortSaved )
			return;

		m_WindowSortSaved = false;

		if ( JMStatics.WINDOWS_CONTAINER )
			JMStatics.WINDOWS_CONTAINER.SetSort( m_SavedWindowSort );
	}

	//! Give a COT window's placeholder a preview. Opens the layer if it is not
	//! up yet. Returns the ItemPreviewWidget so the caller can drive it exactly
	//! as it would a widget of its own.
	static ItemPreviewWidget Attach( Widget anchor )
	{
		if ( !anchor )
			return NULL;

		if ( !s_Instance )
			g_Game.GetUIManager().EnterScriptedMenu( MENU_ID, NULL );

		if ( !s_Instance )
		{
			Print( "[COT-LAB] T10 render layer: menu would not open" );
			return NULL;
		}

		return s_Instance.AttachInternal( anchor );
	}

	//! Drop everything and shut the layer. While a scripted menu is open the
	//! UIManager refuses to enter another one, so leaving an empty layer up
	//! blocks the vanilla map and inventory from opening at all.
	static void CloseLayer()
	{
		if ( !s_Instance )
			return;

		s_Instance.Close();
	}

	//! Same idea for the map. A MapWidget needs no entity, only a position and a
	//! scale, so the caller gets the widget straight back.
	static MapWidget AttachMap( Widget anchor )
	{
		if ( !anchor )
			return NULL;

		if ( !s_Instance )
			g_Game.GetUIManager().EnterScriptedMenu( MENU_ID, NULL );

		if ( !s_Instance )
		{
			Print( "[COT-LAB] T11 render layer: menu would not open" );
			return NULL;
		}

		return s_Instance.AttachMapInternal( anchor );
	}

	protected MapWidget AttachMapInternal( Widget anchor )
	{
		Widget host = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/previewlab/layer_map_host.layout", layoutRoot );

		if ( !host )
			return NULL;

		m_Anchors.Insert( anchor );
		m_Hosts.Insert( host );

		MapWidget hostedMap;
		Class.CastTo( hostedMap, host.FindAnyWidget( "layer_map" ) );

		return hostedMap;
	}

	protected ItemPreviewWidget AttachInternal( Widget anchor )
	{
		Widget host = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/previewlab/layer_preview_host.layout", layoutRoot );

		if ( !host )
			return NULL;

		m_Anchors.Insert( anchor );
		m_Hosts.Insert( host );

		ItemPreviewWidget preview;
		Class.CastTo( preview, host.FindAnyWidget( "layer_preview" ) );

		return preview;
	}

	//! Drop every entry whose anchor is gone. Callers do not have to unregister.
	override void Update( float timeslice )
	{
		super.Update( timeslice );

		for ( int i = m_Anchors.Count() - 1; i >= 0; i-- )
		{
			Widget anchor = m_Anchors[i];
			Widget host   = m_Hosts[i];

			if ( !anchor || !host )
			{
				if ( host )
					host.Unlink();

				m_Anchors.RemoveOrdered( i );
				m_Hosts.RemoveOrdered( i );
				continue;
			}

			SyncEntry( anchor, host );
		}

		//! Nothing left to draw, so stop standing in the way of other menus.
		if ( m_Anchors.Count() == 0 )
			Close();
	}

	//! Pin one preview to its placeholder's screen rectangle. layoutRoot covers
	//! the whole screen and sits at the origin, so a child's exact position is
	//! the screen position unchanged.
	//!
	//! The host layouts declare hexactpos/vexactpos/hexactsize/vexactsize 1 for
	//! exactly this reason. With the fractional flags the engine reads these
	//! pixel values as fractions of the screen and the widget lands somewhere
	//! else entirely, full width across the top.
	protected void SyncEntry( Widget anchor, Widget host )
	{
		if ( !anchor.IsVisibleHierarchy() )
		{
			host.Show( false );
			return;
		}

		float w, h;
		anchor.GetScreenSize( w, h );

		if ( w < 1 || h < 1 )
		{
			//! An anchor with no measurable size is almost always a caller
			//! mistake - an empty spacer, or a widget asked for before its
			//! parent has laid out - so say so once rather than silently
			//! hiding the preview forever.
			if ( !m_WarnedEmpty )
			{
				m_WarnedEmpty = true;
				Print( "[COT-LAB] render layer: anchor has no size (" + w + "x" + h + "), entry hidden" );
			}

			host.Show( false );
			return;
		}

		float x, y;
		anchor.GetScreenPos( x, y );

		//! immedUpdate matters here. Deferred, the move is applied on the next
		//! layout pass, and this widget's parent never triggers one.
		host.SetPos( x, y, true );
		host.SetSize( w, h, true );
		host.Show( true );

		//! First few frames only: read the position straight back out, so a
		//! mismatch between what was asked for and what the engine stored shows
		//! up in the log instead of having to be guessed at from a screenshot.
		if ( m_SyncLogged < 3 )
		{
			m_SyncLogged++;

			float hx, hy, hw, hh;
			host.GetScreenPos( hx, hy );
			host.GetScreenSize( hw, hh );

			Print( "[COT-LAB] layer sync: anchor " + x + "," + y + " " + w + "x" + h + "  ->  host " + hx + "," + hy + " " + hw + "x" + hh );
		}
	}
}
