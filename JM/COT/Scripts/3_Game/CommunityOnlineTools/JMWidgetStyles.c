// =============================================================================
//  JMWidgetStyles
//
//  Registers COT's imageset and its widget-style table, ONCE per game process,
//  BEFORE any mission exists.
//
//  ---------------------------------------------------------------------------
//  THIS IS NOT WHAT BREAKS MAPS AND ITEM PREVIEWS. Tested, twice.
//
//  The blank MapWidget / ItemPreviewWidget hunt spent three rounds here on the
//  theory that swapping the active imageset invalidates the render resources
//  those two widget types use. It does not:
//
//    * moving the whole registration to OnAfterCreate, before any mission
//      exists, changed nothing;
//    * a build with JM_COT_NO_CUSTOM_STYLES - registering NEITHER the imageset
//      NOR the style table, i.e. byte-for-byte upstream behaviour - changed
//      nothing either.
//
//  Do not re-open that theory without new evidence. The remaining difference
//  from every vanilla map / preview is the HOST: vanilla only ever puts those
//  widgets inside a UIScriptedMenu, while COT builds its windows straight onto
//  the workspace at HUD level.
//
//  It still runs from OnAfterCreate rather than the mission: registering once,
//  before any mission, is simply the right place for a process-global call.
//  ---------------------------------------------------------------------------
//
//  ORDER MATTERS IN ALL THREE STEPS. Read this before touching it.
//
//  LoadWidgetImageSet does not add to a registry: after it runs,
//  `set:dayz_gui image:...` stops resolving game-wide. Styles bind their slices
//  eagerly, at load time, so anything already bound keeps working - but
//  anything resolved LATER, when a layout is created at runtime, does not.
//  So: make cot_gui active, bind COT's styles against it while it is, then hand
//  the active slot back to vanilla.
// =============================================================================
class JMWidgetStyles
{
	static protected bool s_Loaded;

	//! True once cot_gui registered. A style whose ImageSet cannot be resolved
	//! is an engine-level CTD the moment a widget wearing it is rendered - the
	//! renderer dereferences the missing imageset (access violation reading a
	//! field offset off a null pointer, no script trace). So any layout using a
	//! cot_gui-backed style must check this first and fall back if it is false.
	static protected bool s_ImageSetLoaded;

	static bool IsImageSetLoaded()
	{
		return s_ImageSetLoaded;
	}

	//! Idempotent: safe to call from more than one entry point, and the later
	//! callers are no-ops. The mission still calls it as a backstop in case a
	//! game path reaches a mission without having gone through OnAfterCreate.
	static void Load()
	{
		if ( s_Loaded )
			return;

		// A dedicated server has no workspace to register any of this against.
		if ( g_Game && g_Game.IsDedicatedServer() )
			return;

		// 1. cot_gui becomes the active set.
		//    Required - a COT style whose imageset never resolves at all makes
		//    the renderer dereference a null set, and the client dies the moment
		//    a sidebar draws.
		s_ImageSetLoaded = LoadWidgetImageSet( "JM/COT/GUI/imagesets/cot_gui.imageset" );
		Print( "[COT] LoadWidgetImageSet cot_gui -> " + s_ImageSetLoaded.ToString() );

		// 2. COT's styles bind their nine-slice Items against cot_gui, now,
		//    while it is the active set.
		//
		//    COT's OWN file, deliberately - not a merged copy of vanilla's
		//    table. Re-registering vanilla's styles here would re-bind them too,
		//    at a point where dayz_gui is not active, which is exactly how
		//    notifications ended up as untextured white boxes. Vanilla's styles
		//    are already bound correctly; leave them alone.
		LoadWidgetStyles( "JM/COT/GUI/styles/cot.styles" );

		// 3. Give the active slot back to vanilla, so every layout built from
		//    here on - maps, tooltips, notifications - resolves `set:dayz_gui`
		//    again. COT's styles bound in step 2 and do not need cot_gui to
		//    still be active.
		Print( "[COT] LoadWidgetImageSet dayz_gui -> " + LoadWidgetImageSet( "gui/imagesets/dayz_gui.imageset" ).ToString() );

		s_Loaded = true;
	}
}
