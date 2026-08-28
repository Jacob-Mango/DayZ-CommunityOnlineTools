// =============================================================================
//  JMPreviewLabProbeMenu
//
//  H3 answered the big question: an ItemPreviewWidget and a MapWidget draw
//  correctly inside a UIScriptedMenu and draw nothing anywhere else, including
//  at the workspace root. So the host is what matters.
//
//  What the host test did NOT separate is which half of "inside a menu" is the
//  requirement:
//
//    H4  A menu merely has to be OPEN. If the engine only runs the 3D-into-UI
//        render pass while the menu stack is non-empty, then COT's existing
//        workspace widgets are fine as they are and the fix is to keep a menu
//        open while the COT UI is up.
//
//    H5  The widget has to LIVE inside the menu's widget tree. If so, keeping a
//        menu open changes nothing and COT's windows have to be re-parented
//        under a menu - which is the real architectural fix.
//
//  H4 is what remains, and it is answered: no. A single fully transparent,
//  pointer-ignoring panel opened as a menu changes nothing about the widgets
//  already on the workspace - they stay blank while it is up. The render pass
//  is not gated on the menu stack; the widget has to be inside the menu's own
//  widget tree, which is exactly what H3 shows.
//
//  H5 used to live here too: it re-parented JMStatics.WINDOWS_CONTAINER into
//  this menu as a prototype of the production fix. It has been removed. The
//  engine destroys a menu's child widgets along with the menu, so closing it
//  destroyed COT's window container and the UI could not be reopened until the
//  client restarted. H3 proves the same point with none of that risk.
// =============================================================================
class JMPreviewLabProbeMenu extends UIScriptedMenu
{
	//! Clear of vanilla's MenuID enum, of Expansion's range and of H3's id.
	static const int MENU_ID = 8124;

	override Widget Init()
	{
		layoutRoot = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/previewlab/probe_host.layout" );

		if ( !layoutRoot )
		{
			Print( "[COT-LAB] probe menu layout failed to load" );
			return NULL;
		}

		Print( "[COT-LAB] probe menu open" );

		return layoutRoot;
	}

	override void OnHide()
	{
		super.OnHide();
	}
}
