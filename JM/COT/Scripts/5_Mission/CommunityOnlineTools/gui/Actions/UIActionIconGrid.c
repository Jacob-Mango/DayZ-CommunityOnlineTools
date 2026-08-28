// =============================================================================
//  UIActionIconGrid
//
//  A wrapping grid of 32x32 icon buttons.  Each icon has a string ID, an
//  image path, an optional tooltip label, and an optional integer badge count.
//  Fires UIEvent.CLICK when any icon is pressed; GetLastClickedId() returns
//  the ID of the pressed icon.
//
//  Usage:
//      m_Grid = UIActionManager.CreateIconGrid( parent, this, "OnClick_Icon" );
//      m_Grid.AddIcon( "item_apple",  "set:dayz_inventory image:apple",  "Apple" );
//      m_Grid.AddIcon( "item_bandage","set:dayz_inventory image:bandage","Bandage" );
//      m_Grid.SetBadgeCount( "item_bandage", 3 );
//
//      void OnClick_Icon( UIEvent eid, UIActionBase action )
//      {
//          string id = m_Grid.GetLastClickedId();
//      }
// =============================================================================

// Internal data for one icon slot
class JMIconGridEntry
{
	string       Id;
	string       Image;
	string       Label;
	int          BadgeCount;
	ButtonWidget Button;
	TextWidget   BadgeText;

	void JMIconGridEntry( string id, string image, string label )
	{
		Id         = id;
		Image      = image;
		Label      = label;
		BadgeCount = 0;
	}
}

class UIActionIconGrid: UIActionBase
{
	protected WrapSpacerWidget m_Grid;

	protected ref array<ref JMIconGridEntry>  m_Entries;
	protected ref map<string, JMIconGridEntry> m_ById;
	protected string                          m_LastClickedId;
	protected string                          m_SelectedId;

	//! A cell is the same rounded pill as a button; the selected one wears the
	//! selected-blue, exactly like the current tab or the active filter chip.
	static const int COLOR_SELECTED      = JMTheme.SELECTED_FILL;
	static const int COLOR_IDLE          = JMTheme.BUTTON_FILL;
	static const int COLOR_HOVER         = JMTheme.BUTTON_FILL_HOVER;
	static const int COLOR_OUTLINE       = JMTheme.BUTTON_OUTLINE;
	static const int COLOR_OUTLINE_ON    = JMTheme.SELECTED_OUTLINE;
	static const int COLOR_OUTLINE_HOVER = JMTheme.BUTTON_OUTLINE_HOVER;

	static const int ICON_SIZE   = 40;
	static const int BADGE_COLOR = JMTheme.DANGER;

	// Preset IDs - use these strings to identify which icon was clicked.
	static const string PRESET_CLOSE    = "preset_close";
	static const string PRESET_MINIMIZE = "preset_minimize";
	static const string PRESET_PIN      = "preset_pin";
	static const string PRESET_REFRESH  = "preset_refresh";
	static const string PRESET_INFO     = "preset_info";
	static const string PRESET_COPY     = "preset_copy";
	static const string PRESET_PLAY     = "preset_play";
	static const string PRESET_PAUSE    = "preset_pause";
	static const string PRESET_NEXT     = "preset_next";
	static const string PRESET_PREVIOUS = "preset_previous";
	static const string PRESET_EDIT     = "preset_edit";
	static const string PRESET_DELETE   = "preset_delete";
	static const string PRESET_SAVE     = "preset_save";
	static const string PRESET_LOAD     = "preset_load";
	static const string PRESET_CLOSE_X  = "preset_close_x";

	override void OnInit()
	{
		super.OnInit();

		m_Grid = UIActionManager.CreateWrapSpacer( layoutRoot );

		m_Entries = new array<ref JMIconGridEntry>;
		m_ById    = new map<string, JMIconGridEntry>;
	}

	//! When true, AddIcon uses a wider cell with a visible text label next to
	//! the icon (chip style) instead of the default 40x40 icon-only square.
	//! Must be set BEFORE the first AddIcon call.
	protected bool m_UseLabeledCells = false;

	void UseLabeledCells( bool labeled = true )
	{
		m_UseLabeledCells = labeled;
	}

	//! Switch the inner WrapSpacer to "grow horizontally" mode. Useful when
	//! placing the grid inside a horizontal scroller so it lays out as a single
	//! row that pans, instead of wrapping to multiple rows. Must be called
	//! before the first AddIcon.
	void UseHorizontalLayout()
	{
		// Swap the WrapSpacer for one with Size To Content H = 1 (grows
		// horizontally instead of wrapping). Safe to call before children
		// are added; if called after, those children stay on the old grid.
		if ( m_Grid )
			m_Grid.Unlink();
		m_Grid = UIActionManager.CreateWrapSpacer(
			"JM/COT/GUI/layouts/uiactions/UIWrapSpacerH.layout", layoutRoot );
	}

	//! Force the inner WrapSpacer to a known pixel width and height. Useful
	//! when the Size-To-Content-H chain through nested spacers fails to
	//! propagate - call this AFTER all AddIcon() calls so the parent horizontal
	//! scroller can compute content overflow against this size. The height
	//! must be exact too, otherwise the WrapSpacer wraps chips into multiple
	//! rows when the available vertical space is large.
	void ForceContentWidth( int pixelWidth, int pixelHeight = 40 )
	{
		if ( !m_Grid )
			return;
		// Force the inner WrapSpacer to an exact size. The pixelHeight is what
		// constrains chips to a single row.
		m_Grid.SetFlags( WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE );
		m_Grid.SetSize( pixelWidth, pixelHeight );
		// IconGrid root: only force width (so the outer scroller measures
		// content overflow correctly). Leave height fractional so the
		// horizontal scroller's Content can size it naturally without conflict
		// (forcing exact height here was causing the chip row to clip
		// vertically when its computed size didn't match the parent cell).
		layoutRoot.SetFlags( WidgetFlags.HEXACTSIZE );
		float lrW, lrH;
		layoutRoot.GetSize( lrW, lrH );
		layoutRoot.SetSize( pixelWidth, lrH );
	}

	//! Add an icon button.  Duplicate IDs are ignored.
	void AddIcon( string id, string imagePath, string label = "" )
	{
		if ( m_ById.Contains( id ) )
			return;

		ref JMIconGridEntry entry = new JMIconGridEntry( id, imagePath, label );

		string cellLayout;
		if ( m_UseLabeledCells )
			cellLayout = "JM/COT/GUI/layouts/uiactions/UIActionIconCellLabeled.layout";
		else
			cellLayout = "JM/COT/GUI/layouts/uiactions/UIActionIconCell.layout";

		Widget cell = g_Game.GetWorkspace().CreateWidgets( cellLayout, m_Grid );

		if ( !cell )
			return;

		cell.SetHandler( this );

		ButtonWidget btn;
		Class.CastTo( btn, cell.FindAnyWidget( "action_button" ) );
		entry.Button = btn;

		ImageWidget img;
		Class.CastTo( img, cell.FindAnyWidget( "action_image" ) );
		if ( img )
			img.LoadImageFile( 0, imagePath );

		if ( m_UseLabeledCells )
		{
			TextWidget txt;
			Class.CastTo( txt, cell.FindAnyWidget( "action_label" ) );
			if ( txt )
				txt.SetText( label );
		}

		m_Entries.Insert( entry );
		m_ById.Insert( id, entry );

		PaintCell( entry, false );
	}

	//! Remove an icon by ID.
	void RemoveIcon( string id )
	{
		JMIconGridEntry entry = m_ById.Get( id );
		if ( !entry )
			return;

		if ( entry.Button )
		{
			Widget root = entry.Button.GetParent();
			if ( root ) root.Unlink();
		}

		m_ById.Remove( id );
		m_Entries.RemoveItem( entry );
	}

	//! Set or clear the badge count on an icon (0 = hide badge).
	void SetBadgeCount( string id, int count )
	{
		JMIconGridEntry entry = m_ById.Get( id );
		if ( !entry )
			return;

		entry.BadgeCount = count;

		if ( !entry.BadgeText && count > 0 && entry.Button )
		{
			// Create badge overlay on first use
			Widget badgeWidget = g_Game.GetWorkspace().CreateWidgets(
				"JM/COT/GUI/layouts/uiactions/UIPanel.layout", entry.Button.GetParent() );

			// Fallback: just use a TextWidget directly
			if ( !badgeWidget && entry.Button.GetParent() )
			{
				TextWidget tw = TextWidget.Cast(
					g_Game.GetWorkspace().CreateWidgets( "TextWidget", entry.Button.GetParent() ) );
				entry.BadgeText = tw;
			}
		}

		if ( entry.BadgeText )
		{
			entry.BadgeText.Show( count > 0 );
			if ( count > 0 )
				entry.BadgeText.SetText( "" + count );
		}
	}

	// ---------------------------------------------------------------------------
	//  Presets - one method per common action.  Each is a no-op if the id
	//  already exists, so calling AddDefaultPresets() twice is safe.
	// ---------------------------------------------------------------------------

	void AddPreset_Close()    { AddIcon( PRESET_CLOSE,    JMConstants.ICON_CANCEL,       "Close"    ); }
	void AddPreset_Minimize() { AddIcon( PRESET_MINIMIZE, JMConstants.ICON_EXPAND, 		 "Minimize" ); }
	void AddPreset_Pin()      { AddIcon( PRESET_PIN,      JMConstants.ICON_PIN,          "Pin"      ); }
	void AddPreset_Refresh()  { AddIcon( PRESET_REFRESH,  JMConstants.ICON_CLOCKWISE,    "Refresh"  ); }
	void AddPreset_Info()     { AddIcon( PRESET_INFO,     JMConstants.ICON_INFO,         "Info"     ); }
	void AddPreset_Copy()     { AddIcon( PRESET_COPY,     JMConstants.ICON_STACK, 		 "Copy"     ); }
	void AddPreset_Play()     { AddIcon( PRESET_PLAY,     JMConstants.ICON_PLAY,         "Play"     ); }
	void AddPreset_Pause()    { AddIcon( PRESET_PAUSE,    JMConstants.ICON_PAUSE,        "Pause"    ); }
	void AddPreset_Next()     { AddIcon( PRESET_NEXT,     JMConstants.ICON_FAST_FORWARD, "Next"     ); }
	void AddPreset_Previous() { AddIcon( PRESET_PREVIOUS, JMConstants.ICON_FAST_BACKWARD,"Previous" ); }
	void AddPreset_Edit()     { AddIcon( PRESET_EDIT,     JMConstants.ICON_PENCIL,       "Edit"     ); }
	void AddPreset_Delete()   { AddIcon( PRESET_DELETE,   JMConstants.ICON_TRASH_CAN,    "Delete"   ); }
	void AddPreset_Save()     { AddIcon( PRESET_SAVE,     JMConstants.ICON_SAVE_ARROW,   "Save"     ); }
	void AddPreset_Load()     { AddIcon( PRESET_LOAD,     JMConstants.ICON_OPEN_FOLDER,  "Load"     ); }
	void AddPreset_CloseX()   { AddIcon( PRESET_CLOSE_X,  JMConstants.ICON_CLOSE,        "Close"    ); }

	//! Apply the delete preset style to a confirm-inline or button: trash-can icon + red colour.
	//! For ConfirmInline buttons, also switches the confirm/cancel labels to short
	//! unicode symbols so they fit in narrow icon-sized buttons.
	static void ApplyDeletePreset( UIActionBase btn )
	{
		if ( !btn )
			return;

		btn.SetIcon( JMConstants.ICON_TRASH_CAN );
		btn.SetColor( JMTheme.DANGER_FILL );

		UIActionConfirmInline confirm;
		if ( Class.CastTo( confirm, btn ) )
			confirm.UseIconConfirmLabels();
	}

	//! Add all standard presets in one call.
	void AddDefaultPresets()
	{
		AddPreset_Close();
		AddPreset_CloseX();
		AddPreset_Minimize();
		AddPreset_Pin();
		AddPreset_Refresh();
		AddPreset_Info();
		AddPreset_Copy();
		AddPreset_Play();
		AddPreset_Pause();
		AddPreset_Next();
		AddPreset_Previous();
		AddPreset_Edit();
		AddPreset_Delete();
		AddPreset_Save();
		AddPreset_Load();
	}

	void ClearAll()
	{
		if ( !m_Grid )
			return;

		Widget child = m_Grid.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}
		m_Entries.Clear();
		m_ById.Clear();
		m_LastClickedId = "";
	}

	string GetLastClickedId()
	{
		return m_LastClickedId;
	}

	void SetSelected( string id )
	{
		// Clear old highlight.
		if ( m_SelectedId != "" )
		{
			JMIconGridEntry old = m_ById.Get( m_SelectedId );
			if ( old )
				PaintCell( old, false );
		}

		m_SelectedId = id;

		if ( id != "" )
		{
			JMIconGridEntry entry = m_ById.Get( id );
			if ( entry )
				PaintCell( entry, false );
		}
	}

	//! Repaint one cell's pill. Selection wins over hover, so the chosen icon
	//! keeps its blue while the pointer wanders over its neighbours.
	protected void PaintCell( JMIconGridEntry entry, bool hovered )
	{
		if ( !entry || !entry.Button )
			return;

		int fillColor = COLOR_IDLE;
		int ringColor = COLOR_OUTLINE;

		if ( entry.Id == m_SelectedId && m_SelectedId != "" )
		{
			fillColor = COLOR_SELECTED;
			ringColor = COLOR_OUTLINE_ON;
		}
		else if ( hovered )
		{
			fillColor = COLOR_HOVER;
			ringColor = COLOR_OUTLINE_HOVER;
		}

		Widget fill = entry.Button.FindAnyWidget( "fill" );
		if ( fill )
			fill.SetColor( fillColor );

		Widget outline = entry.Button.FindAnyWidget( "outline" );
		if ( outline )
			outline.SetColor( ringColor );
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		foreach ( JMIconGridEntry entry : m_Entries )
		{
			if ( entry.Button && ( w == entry.Button || entry.Button.FindAnyWidget( w.GetName() ) == w ) )
			{
				PaintCell( entry, true );

				if ( entry.Label != "" )
					UIActionTooltip.Show( entry.Label, "", 0, 0, w );
				return false;   // cell tooltip replaces the action tooltip
			}
		}
		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		foreach ( JMIconGridEntry entry : m_Entries )
		{
			if ( entry.Button && ( w == entry.Button || entry.Button.FindAnyWidget( w.GetName() ) == w ) )
			{
				PaintCell( entry, false );
				break;
			}
		}

		return super.OnMouseLeave( w, enterW, x, y );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		foreach ( JMIconGridEntry entry : m_Entries )
		{
			if ( entry.Button && w == entry.Button )
			{
				m_LastClickedId = entry.Id;
				SetSelected( entry.Id );
				CallEvent( UIEvent.CLICK );
				return true;
			}
		}
		return false;
	}
}
