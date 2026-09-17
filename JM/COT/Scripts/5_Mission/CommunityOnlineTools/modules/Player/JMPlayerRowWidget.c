class JMPlayerRowWidget: COT_ScriptedWidgetEventHandler 
{
	static const float TEXT_OFFSET_PLAYER = 38.0;
	static const string PERMISSION_ADMIN = "COT";

	//! Checkbox fill. The tick alone was the only thing that changed on check,
	//! and a white glyph on the same pale square reads as nothing happening -
	//! the box itself has to take the accent for the state to be visible.
	static const int CHECKBOX_REST    = 0xFFB8C4CA;
	static const int CHECKBOX_CHECKED = JMTheme.ACCENT;

	//! Row name colours, worst state wins. Dead is the darkest because a corpse
	//! is settled - it is not going to get worse and does not want the eye the
	//! way a player still going down does. Hurt is the pair you can walk off:
	//! bleeding and a broken leg. Everything else is plain white.
	static const int NAME_DEAD    = 0xFF8C2438;
	static const int NAME_DANGER  = 0xFFFF627D;
	static const int NAME_HURT    = 0xFFFF9F43;
	static const int NAME_DEFAULT = 0xFFFFFFFF;

	//! Status glyphs stack from the row's right edge in a fixed order, so a row
	//! with one badge puts it where a row with four puts its first. Hard-coded
	//! slots meant a lone "sick" sat four gaps out in empty space.
	static const float BADGE_STACK_ORIGIN = 6.0;
	static const float BADGE_STACK_STEP   = 18.0;

#ifdef DIAG
	static int s_JMPlayerRowWidgetCount;
#endif

	private Widget layoutRoot;

	private string m_GUID;
	private string m_Role;
	private bool m_IsHeader;
	private bool m_IsChecked;

	TextWidget Name;
	ButtonWidget Button;
	CheckBoxWidget Checkbox;

	private Widget m_RowFill;
	private Widget m_RowFocusBar;

	private ButtonWidget m_ChevronButton;
	private ImageWidget m_ChevronDown;
	private ImageWidget m_ChevronRight;

	private ButtonWidget m_CheckboxButton;
	private ImageWidget m_CheckMark;
	private Widget m_CheckBoxFill;

	private ImageWidget m_BadgeFlagged;
	private ImageWidget m_BadgeRole;
	private ImageWidget m_BadgeDead;
	private ImageWidget m_BadgeUncon;
	private ImageWidget m_BadgeBrokenLeg;
	private ImageWidget m_BadgeBleeding;
	private ImageWidget m_BadgeSick;

	JMPlayerForm Menu;

	void JMPlayerRowWidget()
	{
#ifdef DIAG
		s_JMPlayerRowWidgetCount++;
#endif
	}

	void ~JMPlayerRowWidget()
	{
#ifdef DIAG
		s_JMPlayerRowWidgetCount--;
#endif
	}

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );
		Init();
	}

	void Init() 
	{
		Name = TextWidget.Cast(layoutRoot.FindAnyWidget("text_name"));
		Button = ButtonWidget.Cast(layoutRoot.FindAnyWidget("button"));
		Checkbox = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("checkbox"));

		m_RowFill = layoutRoot.FindAnyWidget("row_fill");
		m_RowFocusBar = layoutRoot.FindAnyWidget("row_focus_bar");

		m_ChevronButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("chevron_button"));
		m_ChevronDown = ImageWidget.Cast(layoutRoot.FindAnyWidget("chevron_down"));
		m_ChevronRight = ImageWidget.Cast(layoutRoot.FindAnyWidget("chevron_right"));

		m_CheckboxButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("checkbox_button"));
		m_CheckMark = ImageWidget.Cast(layoutRoot.FindAnyWidget("check_mark"));
		m_CheckBoxFill = layoutRoot.FindAnyWidget("check_box");

		m_BadgeFlagged = ImageWidget.Cast(layoutRoot.FindAnyWidget("badge_flagged"));
		m_BadgeRole = ImageWidget.Cast(layoutRoot.FindAnyWidget("badge_role"));
		m_BadgeDead = ImageWidget.Cast(layoutRoot.FindAnyWidget("badge_dead"));
		m_BadgeUncon = ImageWidget.Cast(layoutRoot.FindAnyWidget("badge_uncon"));
		m_BadgeBrokenLeg = ImageWidget.Cast(layoutRoot.FindAnyWidget("badge_broken_leg"));
		m_BadgeBleeding = ImageWidget.Cast(layoutRoot.FindAnyWidget("badge_bleeding"));
		m_BadgeSick = ImageWidget.Cast(layoutRoot.FindAnyWidget("badge_sick"));

		if ( m_ChevronDown ) m_ChevronDown.LoadImageFile( 0, JMConstants.Lucide( "chevron-down" ) );
		if ( m_ChevronRight ) m_ChevronRight.LoadImageFile( 0, JMConstants.Lucide( "chevron-right" ) );
		if ( m_CheckMark ) m_CheckMark.LoadImageFile( 0, JMConstants.Lucide( "check" ) );
		if ( m_BadgeRole ) m_BadgeRole.LoadImageFile( 0, JMConstants.Lucide( "shield" ) );
		if ( m_BadgeFlagged ) m_BadgeFlagged.LoadImageFile( 0, JMConstants.Lucide( "flag" ) );

		//! These four had a widget and a colour but no image, so every one of
		//! them drew nothing even once its condition was true.
		if ( m_BadgeDead ) m_BadgeDead.LoadImageFile( 0, JMConstants.Lucide( "skull" ) );
		if ( m_BadgeUncon ) m_BadgeUncon.LoadImageFile( 0, JMConstants.Lucide( "bed" ) );
		if ( m_BadgeBrokenLeg ) m_BadgeBrokenLeg.LoadImageFile( 0, JMConstants.Lucide( "bone-fracture" ) );
		if ( m_BadgeBleeding ) m_BadgeBleeding.LoadImageFile( 0, JMConstants.Lucide( "droplet" ) );
		if ( m_BadgeSick ) m_BadgeSick.LoadImageFile( 0, JMConstants.Lucide( "thermometer" ) );

		if ( m_BadgeDead ) m_BadgeDead.Show( false );
		if ( m_BadgeUncon ) m_BadgeUncon.Show( false );
		if ( m_BadgeBrokenLeg ) m_BadgeBrokenLeg.Show( false );
		if ( m_BadgeBleeding ) m_BadgeBleeding.Show( false );
		if ( m_BadgeSick ) m_BadgeSick.Show( false );
	}

	bool IsHeader()
	{
		return m_IsHeader;
	}

	string GetRole()
	{
		return m_Role;
	}

	string GetGUID()
	{
		return m_GUID;
	}

	bool IsChecked()
	{
		return m_IsChecked;
	}

	void SetChecked( bool checked )
	{
		m_IsChecked = checked;
		if ( Checkbox )
			Checkbox.SetChecked( checked );
		if ( m_CheckMark )
			m_CheckMark.Show( checked );

		if ( m_CheckBoxFill )
		{
			if ( checked )
				m_CheckBoxFill.SetColor( CHECKBOX_CHECKED );
			else
				m_CheckBoxFill.SetColor( CHECKBOX_REST );
		}
	}

	//! Pack the visible status glyphs against the row's right edge in a fixed
	//! order, closing the gaps left by the ones that are hidden. Called after
	//! every pass that changes which badges are up.
	//!
	//! badge_role is deliberately not in here: it belongs to a header row, sits
	//! beside the chevron rather than in the player row's status strip, and the
	//! two are never on screen together.
	private void StackBadges()
	{
		float slot = BADGE_STACK_ORIGIN;

		slot = StackBadge( m_BadgeFlagged, slot );
		slot = StackBadge( m_BadgeDead, slot );
		slot = StackBadge( m_BadgeUncon, slot );
		slot = StackBadge( m_BadgeBrokenLeg, slot );
		slot = StackBadge( m_BadgeBleeding, slot );
		slot = StackBadge( m_BadgeSick, slot );
	}

	//! Places one badge and returns the next free slot. A hidden badge costs
	//! nothing and leaves the slot for whoever comes after it.
	private float StackBadge( ImageWidget badge, float slot )
	{
		if ( !badge || !badge.IsVisible() )
			return slot;

		float ox, oy;
		badge.GetPos( ox, oy );
		badge.SetPos( slot, oy );

		return slot + BADGE_STACK_STEP;
	}

	void SetFocused( bool focused )
	{
		if ( m_RowFocusBar )
			m_RowFocusBar.Show( focused );
	}

	void SetRevealProgress( float t )
	{
		if ( !layoutRoot )
			return;

		layoutRoot.SetSize( 1.0, 28.0 * t );
		layoutRoot.SetAlpha( t );
	}

	void SetChevronProgress( float t )
	{
		if ( m_ChevronDown )
			m_ChevronDown.Show( t >= 0.5 );
		if ( m_ChevronRight )
			m_ChevronRight.Show( t < 0.5 );
	}

	void SetRoleHeader( string role, string label, int count, bool expanded, bool checked, bool isAdminRole )
	{
		m_IsHeader = true;
		m_Role = role;
		m_GUID = "";

		Show();

		if ( Name )
		{
			Name.SetText( label + " (" + count + ")" );
			Name.SetColor( 0xFF95A5A6 );
		}

		if ( m_ChevronButton )
			m_ChevronButton.Show( true );

		if ( m_ChevronDown )
			m_ChevronDown.Show( expanded );

		if ( m_ChevronRight )
			m_ChevronRight.Show( !expanded );

		if ( m_BadgeRole )
			m_BadgeRole.Show( isAdminRole );

		if ( m_BadgeFlagged ) m_BadgeFlagged.Show( false );
		if ( m_BadgeDead ) m_BadgeDead.Show( false );
		if ( m_BadgeUncon ) m_BadgeUncon.Show( false );
		if ( m_BadgeBrokenLeg ) m_BadgeBrokenLeg.Show( false );
		if ( m_BadgeBleeding ) m_BadgeBleeding.Show( false );
		if ( m_BadgeSick ) m_BadgeSick.Show( false );

		SetChecked( checked );
	}

	void SetPlayer( string guid )
	{
		m_IsHeader = false;
		m_Role = "";
		m_GUID = guid;

		if ( m_ChevronButton )
			m_ChevronButton.Show( false );

		if ( m_BadgeRole )
			m_BadgeRole.Show( false );

		if ( m_GUID == "" ) 
		{
			Hide();
		}
		else
		{
			JMPlayerInstance player = GetPermissionsManager().GetPlayer( m_GUID );
			if ( player )
			{
				Show();

				//! No role prefix on the name: the row already sits under its
				//! role's header, so "[admin] Name" repeated the category for
				//! every member of it and ate the width the name needed.
				Name.SetText( player.GetName() );

				//! Read off the synced player vars rather than PlayerObject:
				//! the object is null for anyone outside the client's network
				//! bubble, which is most of the roster, and that silently made
				//! every distant player look healthy.
				bool isDead = player.IsDead();
				bool isUncon = player.IsUnconscious();
				bool hasBrokenLeg = player.HasBrokenLegs();
				bool isBleeding = player.IsBleeding();
				bool isSick = player.IsSick();
				bool isFlagged = JMAntiCheatStatus.IsFlagged( m_GUID );

				//! Unconscious or sick is "in danger" - both end with a body if
				//! nobody intervenes. Bleeding and a broken leg are the two a
				//! player walks away from, so they read as hurt, not dying.
				int nameColor;
				if ( isDead )
					nameColor = NAME_DEAD;
				else if ( isUncon || isSick )
					nameColor = NAME_DANGER;
				else if ( isBleeding || hasBrokenLeg )
					nameColor = NAME_HURT;
				else
					nameColor = NAME_DEFAULT;

				Name.SetColor( nameColor );

				SetChecked( JM_GetSelected().IsSelected( m_GUID ) );

				//! Dead outranks the rest: a corpse is not also unconscious or
				//! bleeding in any way worth a second glyph.
				if ( m_BadgeDead ) m_BadgeDead.Show( isDead );
				if ( m_BadgeUncon ) m_BadgeUncon.Show( isUncon && !isDead );
				if ( m_BadgeBrokenLeg ) m_BadgeBrokenLeg.Show( hasBrokenLeg && !isDead );
				if ( m_BadgeBleeding ) m_BadgeBleeding.Show( isBleeding && !isDead );
				if ( m_BadgeSick ) m_BadgeSick.Show( isSick && !isDead );
				if ( m_BadgeFlagged ) m_BadgeFlagged.Show( isFlagged );

				StackBadges();
			}
			else
			{
				Hide();
			}
		}
	}

	void Show()
	{
		if ( layoutRoot ) layoutRoot.Show( true );
		if ( Button ) Button.Show( true );
		if ( Checkbox ) Checkbox.Show( true );
		if ( m_CheckboxButton ) m_CheckboxButton.Show( true );
		if ( Name ) Name.Show( true );
		OnShow();
	}

	void Hide()
	{
		OnHide();
		if ( Name ) Name.Show( false );
		if ( Button ) Button.Show( false );
		if ( Checkbox ) Checkbox.Show( false );
		if ( m_CheckboxButton ) m_CheckboxButton.Show( false );
		if ( m_ChevronButton ) m_ChevronButton.Show( false );
		if ( layoutRoot ) layoutRoot.Show( false );
	}

	void OnShow()
	{
	}

	void OnHide() 
	{
	}

	void Update() 
	{
	}

	Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	//! What each status glyph stands for, for the hover hint. "" for anything
	//! that is not a badge, which is also how OnMouseEnter decides whether the
	//! pointer is over one at all.
	private string BadgeTooltip( Widget w )
	{
		if ( !w )
			return "";

		if ( w == m_BadgeDead )
			return "#STR_COT_PLAYER_MODULE_STATUS_DEAD";
		if ( w == m_BadgeUncon )
			return "#STR_COT_PLAYER_MODULE_STATUS_UNCONSCIOUS";
		if ( w == m_BadgeBrokenLeg )
			return "#STR_COT_PLAYER_MODULE_STATUS_BROKEN_LEGS";
		if ( w == m_BadgeBleeding )
			return "#STR_COT_PLAYER_MODULE_STATUS_BLEEDING";
		if ( w == m_BadgeSick )
			return "#STR_COT_PLAYER_MODULE_STATUS_SICK";
		if ( w == m_BadgeRole )
			return "#STR_COT_PLAYER_MODULE_STATUS_ADMIN";
		if ( w == m_BadgeFlagged )
			return "#STR_COT_PLAYER_MODULE_STATUS_FLAGGED";

		return "";
	}

	//! A glyph is a picture with no label, so the word it stands for has to be
	//! reachable somehow. The badges are pointer-aware in the layout purely so
	//! this can fire.
	override bool OnMouseEnter( Widget w, int x, int y )
	{
		string tooltip = BadgeTooltip( w );

		if ( tooltip == "" )
			return false;

		UIActionTooltip.Show( Widget.TranslateString( tooltip ), "", 0, 0, w );

		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( BadgeTooltip( w ) != "" )
			UIActionTooltip.Hide();

		return false;
	}

	//! Right-click opens the row's action menu instead of selecting. ButtonWidget
	//! raises no click for the right button at all, so this is the only event
	//! that sees it, and it has to cover the badges too - they sit on top of the
	//! row button and would otherwise be a dead strip along the right edge.
	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		bool onBadge = ( BadgeTooltip( w ) != "" );

		if ( button == MouseState.RIGHT )
		{
			if ( m_IsHeader || m_GUID == "" || !Menu )
				return false;

			UIActionTooltip.Hide();
			Menu.OnPlayerRow_RightClick( m_GUID, x, y );

			return true;
		}

		//! A left click that lands on a badge is still a click on the row. The
		//! badges only take the pointer so they can raise a tooltip, and an
		//! ImageWidget does not raise OnClick, so without this the right-hand
		//! strip of every row with a status would be dead to selection.
		if ( button == MouseState.LEFT && onBadge )
		{
			if ( m_IsHeader )
			{
				if ( Menu )
					Menu.OnRoleHeader_Toggled( m_Role );
			}
			else if ( m_GUID != "" )
			{
				JMScriptInvokers.MENU_PLAYER_BUTTON.Invoke( m_GUID, !m_IsChecked );
			}

			return true;
		}

		return false;
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
			return false;

		//! Selection is a left-button gesture only. Without this a right-click
		//! that opened the menu also checked the row it was opened on.
		if ( button != MouseState.LEFT )
			return false;

		if ( m_IsHeader )
		{
			if ( w == Checkbox || w == m_CheckboxButton )
			{
				if ( Menu )
					Menu.OnRoleHeader_Checked( m_Role, !m_IsChecked );
				return true;
			}

			if ( w == m_ChevronButton || w == Button || w == layoutRoot )
			{
				if ( Menu )
					Menu.OnRoleHeader_Toggled( m_Role );
				return true;
			}
		}
		else
		{
			if ( w == Checkbox || w == m_CheckboxButton )
			{
				bool newCheck = !m_IsChecked;
				SetChecked( newCheck );
				JMScriptInvokers.MENU_PLAYER_CHECKBOX.Invoke( m_GUID, newCheck );
				return true;
			}

			if ( w == Button )
			{
				JMScriptInvokers.MENU_PLAYER_BUTTON.Invoke( m_GUID, !m_IsChecked );
				return true;
			}
		}

		return false;
	}
}
