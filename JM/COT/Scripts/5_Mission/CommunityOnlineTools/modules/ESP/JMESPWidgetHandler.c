//! #define scope in Enforce is per-file, NOT per compiled module - see COTModule.c.
#define COT_DEBUGLOGS

//! The world-space tag for one tracked object.
//!
//! One line - dot, state markers, name, distance - and a right-click. The tag
//! used to unfold a panel of controls under itself; every action lives in
//! JMESPActionMenu now, which the ESP form's tracked list shares.
class JMESPWidgetHandler: ScriptedWidgetEventHandler
{
	static JMESPForm espMenu;
	static JMESPModule espModule;

	static bool UseClassName = false;

	//! Distance is the one part of the label an admin may not want: a wall of
	//! tags reads faster without it.
	static bool ShowDistance = true;

	//! Header geometry, in pixels. The row is packed by script rather than by
	//! the layout: the markers sit tight against the name, and the name's width
	//! is only known once it has been measured.
	static const float ROW_PAD_LEFT  = 9;
	static const float ROW_PAD_RIGHT = 8;
	static const float ROW_DOT_W     = 10;
	static const float ROW_ICON_W    = 14;
	static const float ROW_GAP       = 6;

	//! Below this perceived luminance a category colour cannot be read on the
	//! dark tag ground, and the row flips to a light one instead.
	static const float ROW_DARK_LUMA = 0.38;

	static const int ROW_BG_LIGHT   = 0xFBECF9FF;
	static const int ROW_FG_ONLIGHT = 0xFF161B22;

	//! A right-press has to travel before it counts as a drag, otherwise every
	//! attempt at the context menu would nudge the object.
	static const float DRAG_ARM_PX = 4;

	//! How far the cursor ray is allowed to reach before it counts as a miss.
	static const float DRAG_RAY_LENGTH = 2000;

	//! World metres of lift per pixel of mouse travel, per metre between the
	//! camera and the object. A near crate and one across a field both rise at
	//! the speed the hand suggests rather than the far one crawling.
	static const float DRAG_VERTICAL_SCALE = 0.0016;

	//! Smallest change worth an RPC, and the gap between them. A gesture that
	//! sent every frame would flood the server for a movement nobody can see at
	//! that resolution anyway; the release always sends a final exact transform.
	static const float DRAG_MIN_DELTA = 0.05;
	static const float DRAG_MIN_ANGLE = 1.0;
	static const int   DRAG_SEND_MS   = 100;

	private Widget layoutRoot;

	private Widget m_pnl_Header;
	private Widget m_pnl_Accent;

	private ImageWidget m_img_HealthLevel;
	private TextWidget m_txt_ObjectName;

	//! Status strip, packed left to right ahead of the name so a column of tags
	//! reads down its left edge.
	private ImageWidget m_img_IconSelected;
	private ImageWidget m_img_IconState;
	private ImageWidget m_img_IconDrag;

	private bool m_Selected;

	//! True while the row is painted light because its category colour is too
	//! dark to read on the normal ground. The markers pick their palette from
	//! it.
	private bool m_LightRow;

	//! One menu for every tag on screen - there can be hundreds, and only one
	//! of them can be open at a time. It is the same instance a right-click on
	//! the object itself raises, and it lives on JMESPActionMenu.

	//! Which axis lock the move is under: none, Shift, or Ctrl. Kept because a
	//! modifier pressed part way through a drag has to re-baseline, not measure
	//! from where the gesture began.
	static const int DRAG_MODE_FREE       = 0;
	static const int DRAG_MODE_HORIZONTAL = 1;
	static const int DRAG_MODE_VERTICAL   = 2;

	private int m_DragMode;

	private bool m_RightHeld;
	private bool m_Dragging;
	private bool m_Rotating;

	//! Whether the gesture ever actually changed anything, so the release only
	//! commits what it touched.
	private bool m_DidMove;
	private bool m_DidRotate;

	private int m_DragMouseX;
	private int m_DragMouseY;
	private vector m_DragOrigin;
	private vector m_DragPosition;
	private vector m_DragOrientation;

	//! Transform at the moment of the press, held untouched for the whole
	//! gesture so Ctrl+Z has something to go back to - m_DragOrigin/
	//! m_DragOrientation above get overwritten mid-gesture (mode switches,
	//! per-frame rotation updates) and can't serve that purpose.
	private vector m_GestureStartPosition;
	private vector m_GestureStartOrientation;

	//! Last transform actually sent, so the throttle measures change since the
	//! last RPC rather than since the start of the gesture.
	private vector m_DragSent;
	private vector m_OriSent;

	//! Camera distance at the moment of the grab, held so a ray into open sky
	//! still has somewhere to put the object.
	private float m_DragDistance;
	private int m_DragLastSend;

	private bool m_IsShown;
	private bool m_DidUnlink;

	private vector m_LastPosition;
	private string m_TargetType;

	bool ShowOnScreen;

	int Width;
	int Height;
	float FOV;
	vector ScreenPos;

	JMESPMeta Info;

	void OnWidgetScriptInit( Widget w )
	{
		m_DidUnlink = false;

		layoutRoot = w;
		layoutRoot.SetHandler( this );

		Init();
	}

	void ~JMESPWidgetHandler()
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPWidgetHandler::~JMESPWidgetHandler();" );
		#endif
		#endif

		OnHide();

		JMScriptInvokers.ON_DELETE_ALL.Remove( OnDeleteAll );

		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "-JMESPWidgetHandler::~JMESPWidgetHandler();" );
		#endif
		#endif
	}

	void Init()
	{
		Class.CastTo( m_pnl_Header, layoutRoot.FindAnyWidget( "esp_object_header" ) );

		Widget header = m_pnl_Header;

		m_pnl_Accent = header.FindAnyWidget( "esp_object_accent" );

		Class.CastTo( m_img_HealthLevel, header.FindAnyWidget( "esp_health_level_icon" ) );
		Class.CastTo( m_txt_ObjectName, header.FindAnyWidget( "esp_object_name" ) );

		InitStatusIcon( m_img_IconSelected, "esp_icon_selected", "check" );
		InitStatusIcon( m_img_IconState, "esp_icon_state", "skull" );
		InitStatusIcon( m_img_IconDrag, "esp_icon_drag", "move" );

		ApplyRowColours();

		JMScriptInvokers.ON_DELETE_ALL.Insert( OnDeleteAll );
	}

	private void InitStatusIcon( out ImageWidget icon, string name, string lucide )
	{
		if ( !Class.CastTo( icon, m_pnl_Header.FindAnyWidget( name ) ) )
			return;

		icon.LoadImageFile( 0, JMConstants.Lucide( lucide ) );
		icon.Show( false );
	}

	//! Marker colours come in pairs: the bright set reads on the dark tag
	//! ground, the deep set on the light ground a dark category flips it to.
	private void ApplyRowColours()
	{
		if ( m_LightRow )
		{
			if ( m_img_IconSelected ) m_img_IconSelected.SetColor( JMTheme.SUCCESS_FILL );
			if ( m_img_IconState ) m_img_IconState.SetColor( JMTheme.DANGER_FILL );
			if ( m_img_IconDrag ) m_img_IconDrag.SetColor( JMTheme.WARNING_DIM );

			return;
		}

		if ( m_img_IconSelected ) m_img_IconSelected.SetColor( JMTheme.SUCCESS );
		if ( m_img_IconState ) m_img_IconState.SetColor( JMTheme.DANGER );
		if ( m_img_IconDrag ) m_img_IconDrag.SetColor( JMTheme.WARNING );
	}

	void Show()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPWidgetHandler::Show" );
		#endif

		layoutRoot.Show( true );
		OnShow();

		#ifdef COT_DEBUGLOGS
		Print( "-JMESPWidgetHandler::Show" );
		#endif
	}

	void Hide()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPWidgetHandler::Hide" );
		#endif

		OnHide();
		layoutRoot.Show( false );

		#ifdef COT_DEBUGLOGS
		Print( "-JMESPWidgetHandler::Hide" );
		#endif
	}

	void OnShow()
	{
		if (m_IsShown)
			return;

		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Insert( this.Update );

		m_IsShown = true;
	}

	void OnHide()
	{
		ReleaseRight();

		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Remove( this.Update );

		Deselect();

		m_IsShown = false;
	}

	void OnDeleteAll()
	{
		if (Info)
			Info.m_TargetDeleted = true;
	}

	float ATan( float a )
	{
		return Math.Asin( a ) / Math.Acos( a );
	}

	vector Player_GetTopOfHeadPosition()
	{
		Human man;
		if ( !Class.CastTo( man, Info.target ) )
		{
			return Info.target.GetPosition();
		}

		if ( man.IsInTransport() )
		{
			return man.GetPosition() + "0 1.1 0";
		}

		vector position = man.GetPosition() + "0 1.8 0";

		int bone = man.GetBoneIndexByName( "Head" );

		if ( bone != -1 )
		{
			position = man.GetBonePositionWS( bone ) + "0 0.2 0";
		}

		return position;
	}

	vector GetPosition()
	{
		if ( Info.target )
		{
			return Info.target.GetPosition();
		}

		return m_LastPosition;
	}

	void Update()
	{
		if ( Info == NULL )
		{
			ReleaseRight();
			ShowOnScreen = false;
			Hide();
			return;
		}

		//! Outside the on-screen branch on purpose: a tag that scrolls off the
		//! edge mid-gesture still has to let go of the mouse and of the inputs.
		UpdateRightHold();

		m_LastPosition = GetPosition();

		ScreenPos = g_Game.GetScreenPos( m_LastPosition );

		float distance = vector.Distance(GetCurrentPosition(), m_LastPosition);

		//! Nearer tags draw over further ones; the one being handled draws over
		//! all of them, so it cannot vanish behind whatever it is dragged past.
		int zIndex;

		if (m_RightHeld)
			zIndex = 0;
		else
			zIndex = -distance * 100;

		layoutRoot.SetSort(zIndex);

		distance = Math.Round(distance * 10.0) / 10.0;

		GetScreenSize( Width, Height );

		if (ScreenPos[0] <= 0 || ScreenPos[1] <= 0 || ScreenPos[0] >= Width || ScreenPos[1] >= Height || ScreenPos[2] < 0)
		{
			ShowOnScreen = false;
		}
		else if (g_Game.GetUIManager().GetMenu())
		{
			ShowOnScreen = false;
		}
	#ifdef DZ_Expansion_Core
		else if (GetDayZExpansion().GetExpansionUIManager().GetMenu())
		{
			ShowOnScreen = false;
		}
	#endif
		else
		{
			ShowOnScreen = true;
		}

		if ( ShowOnScreen && Info )
		{
			layoutRoot.SetPos( ScreenPos[0], ScreenPos[1], true );

			bool isHealthVisible = false;

			if (Info.target && !Info.target.IsPlainObject() && Info.target.GetNumberOfHealthLevels() > 0)
			{
				isHealthVisible = true;

				switch (Info.target.GetHealthLevel())
				{
					case GameConstants.STATE_WORN:
						m_img_HealthLevel.SetColor(Colors.COLOR_WORN | 0xFF000000);
						break;

					case GameConstants.STATE_DAMAGED:
						m_img_HealthLevel.SetColor(Colors.COLOR_DAMAGED | 0xFF000000);
						break;

					case GameConstants.STATE_BADLY_DAMAGED:
						m_img_HealthLevel.SetColor(Colors.COLOR_BADLY_DAMAGED | 0xFF000000);
						break;

					case GameConstants.STATE_RUINED:
						m_img_HealthLevel.SetColor(Colors.COLOR_RUINED | 0xFF000000);
						break;

					default:
						m_img_HealthLevel.SetColor(Colors.COLOR_PRISTINE | 0xFF000000);
						break;
				}
			}

			string text;

			if ( Info.type.IsInherited( JMESPViewTypePlayer ) || !UseClassName )
				text += Info.name;
			else
				text += m_TargetType;

			if ( ShowDistance )
				text += " (" + distance + " m)";

			m_txt_ObjectName.SetText( text );

			float tw, th;
			m_txt_ObjectName.GetScreenSize( tw, th );

			//! Measured every frame rather than grown with Math.Max, so the tag
			//! shrinks back when the name gets shorter - switching to classnames
			//! used to leave it stuck at its widest.
			float w, h;
			layoutRoot.GetScreenSize( w, h );
			layoutRoot.SetScreenSize( LayoutRow( isHealthVisible, tw ), h );

			layoutRoot.Show( true );
		} else
		{
			layoutRoot.Show( false );
		}
	}

	//! Pack the row left to right and return the width it came to.
	private float LayoutRow( bool healthVisible, float textWidth )
	{
		float cursor = ROW_PAD_LEFT;

		m_img_HealthLevel.Show( healthVisible );

		if ( healthVisible )
		{
			m_img_HealthLevel.SetPos( cursor, 0 );
			cursor += ROW_DOT_W + ROW_GAP;
		}

		bool destroyed = false;
		if ( Info.target )
			destroyed = Info.target.IsDamageDestroyed();

		cursor = PlaceStatusIcon( m_img_IconSelected, m_Selected, cursor );
		cursor = PlaceStatusIcon( m_img_IconState, destroyed, cursor );
		cursor = PlaceStatusIcon( m_img_IconDrag, m_Dragging, cursor );

		m_txt_ObjectName.SetPos( cursor, 0 );

		return cursor + textWidth + ROW_PAD_RIGHT;
	}

	private float PlaceStatusIcon( ImageWidget icon, bool show, float x )
	{
		if ( !icon )
			return x;

		icon.Show( show );

		if ( !show )
			return x;

		icon.SetPos( x, 0 );

		return x + ROW_ICON_W + ROW_GAP;
	}

	Widget GetLayoutRoot()
	{
		return layoutRoot;
	}

	void SetInfo( JMESPMeta info )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPWidgetHandler::SetInfo" );
		#endif

		Info = info;

		if ( Info == NULL || Info.target == NULL )
		{
			ShowOnScreen = false;
			Hide();
			return;
		}

		m_Selected = JM_GetSelected().IsObjectSelected( Info.target );

		m_TargetType = Info.GetType();

		m_txt_ObjectName.SetColor( Info.colour );

		ApplyCategoryColour();

		ShowOnScreen = true;

		Show();

		#ifdef COT_DEBUGLOGS
		Print( "-JMESPWidgetHandler::SetInfo" );
		#endif
	}

	//! Paint the tag from its category: a solid accent strip down the left edge
	//! and a ground the category colour can actually be read against.
	//!
	//! A bright category keeps the dark ground with a weak wash of its own hue
	//! - a full-strength fill would take the contrast with it. A dark one has
	//! nowhere to go on a dark ground, so the row flips to a light ground and
	//! the colour is left to carry the text instead of fighting it.
	void ApplyCategoryColour()
	{
		if ( !Info || !Info.type )
			return;

		int rgb = Info.type.Colour & 0x00FFFFFF;

		if ( m_pnl_Accent )
			m_pnl_Accent.SetColor( rgb | 0xFF000000 );

		m_LightRow = Luminance( rgb ) < ROW_DARK_LUMA;

		if ( m_LightRow )
		{
			if ( m_pnl_Header )
				m_pnl_Header.SetColor( ROW_BG_LIGHT );

			if ( m_txt_ObjectName )
				m_txt_ObjectName.SetColor( rgb | 0xFF000000 );
		}
		else
		{
			if ( m_pnl_Header )
				m_pnl_Header.SetColor( rgb | 0x2E000000 );

			if ( m_txt_ObjectName )
				m_txt_ObjectName.SetColor( Info.colour );
		}

		ApplyRowColours();
	}

	//! Perceived luminance, 0..1. Weighted rather than a flat average because
	//! the eye reads green as far brighter than blue at the same value, and a
	//! flat average calls a saturated blue "light" when it is not.
	private float Luminance( int rgb )
	{
		//! Unpacked into ints and only then widened. Enforce will not mix an
		//! int bit expression with float arithmetic in one statement, whatever
		//! the error it prints says.
		int ri = ( rgb >> 16 ) & 0xFF;
		int gi = ( rgb >> 8 ) & 0xFF;
		int bi = rgb & 0xFF;

		float r = ri;
		float g = gi;
		float b = bi;

		float lum = ( 0.299 * r ) + ( 0.587 * g ) + ( 0.114 * b );

		return lum / 255.0;
	}

	void Select()
	{
		m_Selected = true;

		SetSelected( true );
	}

	void Deselect()
	{
		m_Selected = false;

		SetSelected( false );
	}

	bool IsSelected()
	{
		return m_Selected;
	}

	//! Left-click selects. The row used to be an expand button; there is
	//! nothing left to expand, and selection is what an admin does to a tag
	//! most often.
	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w != m_pnl_Header || button != MouseState.LEFT )
			return false;

		if ( m_Selected )
			Deselect();
		else
			Select();

		return true;
	}

	// =========================================================================
	//  Context menu
	// =========================================================================

	//! One menu for every tag on screen. It is anchored to the ESP container
	//! rather than to this tag: the tag is repositioned every frame from the
	//! object's projected screen position, and a menu parented to it would ride
	//! along and slide out from under the cursor.
	void OpenContextMenu()
	{
		if ( !Info || !Info.target )
			return;

		JMESPActionMenu menu = JMESPActionMenu.Shared();

		if ( !menu )
			return;

		int mx;
		int my;
		GetMousePos( mx, my );

		menu.Open( Info, mx, my );
	}

	// =========================================================================
	//  Right-press gestures
	// =========================================================================

	//! Right-press arms every gesture at once: let go without moving and it is
	//! a context menu, drag and it is a move, add the middle button and it is a
	//! turn. Nothing is decided here, because the press cannot yet know which
	//! one it will turn out to be.
	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( w != m_pnl_Header || button != MouseState.RIGHT )
			return false;

		if ( !Info || !Info.target )
			return false;

		m_RightHeld       = true;
		m_Dragging        = false;
		m_Rotating        = false;
		m_DragMode        = DRAG_MODE_FREE;
		m_DidMove         = false;
		m_DidRotate       = false;
		m_DragOrigin      = Info.target.GetPosition();
		m_DragPosition    = m_DragOrigin;
		m_DragSent        = m_DragOrigin;
		m_DragOrientation = Info.target.GetOrientation();
		m_OriSent         = m_DragOrientation;
		m_DragLastSend    = 0;

		m_GestureStartPosition    = m_DragOrigin;
		m_GestureStartOrientation = m_DragOrientation;
		m_DragDistance    = vector.Distance( g_Game.GetCurrentCameraPosition(), m_DragOrigin );

		GetMousePos( m_DragMouseX, m_DragMouseY );

		//! Held from the press rather than from the drag: the world must not
		//! read a right-press over a tag as aiming down sights.
		CommunityOnlineToolsBase.ForceDisableInputs( true );

		return true;
	}

	override bool OnMouseButtonUp( Widget w, int x, int y, int button )
	{
		if ( !m_RightHeld )
			return false;

		ReleaseRight();

		return true;
	}

	//! A press that never travelled is a click - raise the menu. One that did
	//! is a gesture - commit whatever it changed.
	private void ReleaseRight()
	{
		if ( !m_RightHeld )
			return;

		m_RightHeld = false;

		CommunityOnlineToolsBase.ForceDisableInputs( false );

		if ( m_Dragging )
		{
			EndDrag();
			return;
		}

		OpenContextMenu();
	}

	private void EndDrag()
	{
		if ( !m_Dragging )
			return;

		m_Dragging = false;
		m_Rotating = false;

		if ( !Info || !Info.target )
			return;


		//! One undo step per gesture, not per throttled RPC during the drag -
		//! this is the one point a move/rotate gesture actually commits, so
		//! it's the only safe place to record it without spamming the stack.
		//! Routed through the module (a server-side RPC, same as SetPosition/
		//! SetOrientation below) rather than pushed to JMActionHistory
		//! directly - this code runs on the dragging admin's client, and the
		//! shared undo stack only exists on the server.
		if ( m_DidMove || m_DidRotate )
			Info.module.RecordTransformHistory( Info.target, m_GestureStartPosition, m_GestureStartOrientation );

		//! Always commit the exact final transform, whatever the throttle did
		//! or did not send during the gesture.
		if ( m_DidMove )
			Info.module.SetPosition( m_DragPosition, Info.target );

		if ( m_DidRotate )
			Info.module.SetOrientation( m_DragOrientation, Info.target );
	}

	//! Where the cursor is pointing in the world: a ray from the camera through
	//! the pointer.
	//!
	//! Three things have to be invisible to it - the object being handled, the
	//! admin's own body, and whatever they are sitting in - or the gesture
	//! lands on one of those instead of on the ground behind. RaycastRVParams
	//! carries a single ignore, which is why this goes through the proxy call
	//! and its exclusion list rather than the plain RaycastRV.
	//!
	//! A ray into open sky hits nothing, and an object cannot be placed at
	//! infinity - so it holds the camera distance it was grabbed at and slides
	//! along the ray instead of snapping away.
	private vector CursorWorldPosition()
	{
		vector from = g_Game.GetCurrentCameraPosition();
		vector dir = g_Game.GetPointerDirection();

		vector to = from + ( dir * DRAG_RAY_LENGTH );

		RaycastRVParams rayParams = new RaycastRVParams( from, to, Info.target );
		rayParams.sorted = true;

		array<Object> excluded = new array<Object>;
		excluded.Insert( Info.target );

		PlayerBase self = PlayerBase.Cast( g_Game.GetPlayer() );

		if ( self )
		{
			excluded.Insert( self );

			HumanCommandVehicle vehicleCommand = self.GetCommand_Vehicle();

			if ( vehicleCommand && vehicleCommand.GetTransport() )
				excluded.Insert( vehicleCommand.GetTransport() );
		}

		array<ref RaycastRVResult> results = new array<ref RaycastRVResult>;

		if ( DayZPhysics.RaycastRVProxy( rayParams, results, excluded ) && results.Count() )
			return results[0].pos;

		return from + ( dir * m_DragDistance );
	}

	//! Once the press has travelled, the object follows the cursor - moved, or
	//! turned if the middle button is down as well.
	private void UpdateRightHold()
	{
		if ( !m_RightHeld )
			return;

		if ( !Info || !Info.target )
		{
			ReleaseRight();
			return;
		}

		//! Losing the button anywhere - including off the tag - ends it.
		if ( ( GetMouseState( MouseState.RIGHT ) & MB_PRESSED_MASK ) == 0 )
		{
			ReleaseRight();
			return;
		}

		bool rotating = ( GetMouseState( MouseState.MIDDLE ) & MB_PRESSED_MASK ) != 0;

		int mx;
		int my;
		GetMousePos( mx, my );

		float px = mx - m_DragMouseX;
		float py = my - m_DragMouseY;

		if ( !m_Dragging )
		{
			//! Adding the middle button is itself a deliberate act, so a turn
			//! does not have to travel first the way a move does.
			if ( !rotating && Math.AbsFloat( px ) < DRAG_ARM_PX && Math.AbsFloat( py ) < DRAG_ARM_PX )
				return;

			m_Dragging = true;
		}

		if ( rotating != m_Rotating )
		{
			m_Rotating = rotating;

			if ( m_img_IconDrag )
			{
				if ( rotating )
					m_img_IconDrag.LoadImageFile( 0, JMConstants.Lucide( "rotate-cw" ) );
				else
					m_img_IconDrag.LoadImageFile( 0, JMConstants.Lucide( "move" ) );
			}
		}

		vector cursor = CursorWorldPosition();

		if ( rotating )
			UpdateRotate( cursor );
		else
			UpdateMove( cursor, mx, my );
	}

	private void UpdateMove( vector cursor, int mx, int my )
	{
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.SetPosition" ) )
			return;

		//! Shift pins the height and Ctrl pins the ground position. One axis at
		//! a time is what makes a placement repeatable: without it, sliding
		//! something along a wall also walks it up or down that wall.
		int mode = DRAG_MODE_FREE;

		if ( KeyState( KeyCode.KC_LCONTROL ) || KeyState( KeyCode.KC_RCONTROL ) )
			mode = DRAG_MODE_VERTICAL;
		else if ( KeyState( KeyCode.KC_LSHIFT ) || KeyState( KeyCode.KC_RSHIFT ) )
			mode = DRAG_MODE_HORIZONTAL;

		//! Every change of lock starts a fresh gesture from where the object and
		//! the cursor are NOW. Measuring from the original press instead would
		//! snap the object back across everything the drag had already done the
		//! instant a modifier went down - and again when it came up.
		if ( mode != m_DragMode )
		{
			m_DragMode = mode;

			m_DragOrigin = Info.target.GetPosition();
			m_DragDistance = vector.Distance( g_Game.GetCurrentCameraPosition(), m_DragOrigin );
			m_DragMouseX = mx;
			m_DragMouseY = my;
		}

		vector pos = cursor;

		if ( mode == DRAG_MODE_VERTICAL )
		{
			//! Height comes off the mouse, NOT off the cursor ray. The ray
			//! answers "what is under the pointer", and pointing at the sky to
			//! raise something has nothing under it at all - which is what made
			//! this jump. Travel is measured from where the lock began and
			//! scaled by the distance to the object, so the lift tracks the
			//! hand at any range.
			float lift = ( my - m_DragMouseY ) * m_DragDistance * DRAG_VERTICAL_SCALE;

			pos = m_DragOrigin;
			pos[1] = m_DragOrigin[1] - lift;
		}
		else if ( mode == DRAG_MODE_HORIZONTAL )
		{
			pos[1] = m_DragOrigin[1];
		}

		m_DragPosition = pos;
		m_DidMove = true;

		//! Moved locally every frame so the gesture looks live; the server only
		//! hears about it on the throttle below, and gets the exact final
		//! transform from EndDrag() either way.
		PreviewTransform( m_DragPosition, Info.target.GetOrientation() );

		if ( !ShouldSend() )
			return;

		if ( vector.Distance( m_DragPosition, m_DragSent ) < DRAG_MIN_DELTA )
			return;

		m_DragLastSend = g_Game.GetTime();
		m_DragSent = m_DragPosition;

		Info.module.SetPosition( m_DragPosition, Info.target );
	}

	//! Turn the object to face where the cursor landed. Yaw only - pitch and
	//! roll are left alone, because a mouse pointing at the ground says nothing
	//! about how far something should lean.
	private void UpdateRotate( vector cursor )
	{
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.SetOrientation" ) )
			return;

		vector dir = cursor - Info.target.GetPosition();
		dir[1] = 0;

		if ( dir.Length() < 0.05 )
			return;

		vector angles = dir.VectorToAngles();

		vector ori = Info.target.GetOrientation();
		ori[0] = angles[0];

		m_DragOrientation = ori;
		m_DidRotate = true;

		//! Turned locally every frame, same as UpdateMove above.
		PreviewTransform( Info.target.GetPosition(), ori );

		if ( !ShouldSend() )
			return;

		if ( Math.AbsFloat( m_DragOrientation[0] - m_OriSent[0] ) < DRAG_MIN_ANGLE )
			return;

		m_DragLastSend = g_Game.GetTime();
		m_OriSent = m_DragOrientation;

		Info.module.SetOrientation( m_DragOrientation, Info.target );
	}

	//! Local, cosmetic only - it just makes the gesture track the cursor while
	//! the button is held. The server owns the real transform and gets it from
	//! the throttled sends plus EndDrag's final commit, so anything wrong here
	//! is corrected the moment the drag ends.
	//!
	//! A vehicle needs its velocities cleared on the way through. Its rigid
	//! body keeps simulating locally between our updates, so a bare
	//! SetPosition leaves gravity and whatever momentum the body already had
	//! to drag it back out of place before the next frame re-places it, which
	//! reads as jitter. Zeroing both each frame pins it under the cursor.
	private void PreviewTransform( vector position, vector orientation )
	{
		Transport transport;
		if ( !Class.CastTo( transport, Info.target ) )
		{
			Info.target.SetPosition( position );
			Info.target.SetOrientation( orientation );
			return;
		}

		transport.SetPosition( position );
		transport.SetOrientation( orientation );

		SetVelocity( transport, vector.Zero );
		dBodySetAngularVelocity( transport, vector.Zero );
	}

	private bool ShouldSend()
	{
		return g_Game.GetTime() - m_DragLastSend >= DRAG_SEND_MS;
	}

	void SetSelected( bool selected )
	{
		if (!Info)
			return;

		if ( Info.type.IsInherited( JMESPViewTypePlayer ) )
		{
			if (Info.player)
				JMScriptInvokers.MENU_PLAYER_CHECKBOX.Invoke( Info.player.GetGUID(), selected );
		}

		if ( selected )
		{
			if ( Info.target )
				JMScriptInvokers.ADD_OBJECT.Invoke( Info.target );
		}
		else
		{
			JMScriptInvokers.REMOVE_OBJECT.Invoke( Info.target, Info.networkLow, Info.networkHigh );
		}
	}
}
