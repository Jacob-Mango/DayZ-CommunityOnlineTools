class JMPlayerRowWidget: COT_ScriptedWidgetEventHandler 
{
	static const float TEXT_OFFSET_PLAYER = 38.0;
	static const string PERMISSION_ADMIN = "COT";

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

	private ImageWidget m_BadgeFlagged;
	private ImageWidget m_BadgeRole;
	private ImageWidget m_BadgeDead;
	private ImageWidget m_BadgeUncon;
	private ImageWidget m_BadgeBrokenLeg;
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

		m_BadgeFlagged = ImageWidget.Cast(layoutRoot.FindAnyWidget("badge_flagged"));
		m_BadgeRole = ImageWidget.Cast(layoutRoot.FindAnyWidget("badge_role"));
		m_BadgeDead = ImageWidget.Cast(layoutRoot.FindAnyWidget("badge_dead"));
		m_BadgeUncon = ImageWidget.Cast(layoutRoot.FindAnyWidget("badge_uncon"));
		m_BadgeBrokenLeg = ImageWidget.Cast(layoutRoot.FindAnyWidget("badge_broken_leg"));
		m_BadgeSick = ImageWidget.Cast(layoutRoot.FindAnyWidget("badge_sick"));

		if ( m_ChevronDown ) m_ChevronDown.LoadImageFile( 0, JMConstants.Lucide( "chevron-down" ) );
		if ( m_ChevronRight ) m_ChevronRight.LoadImageFile( 0, JMConstants.Lucide( "chevron-right" ) );
		if ( m_CheckMark ) m_CheckMark.LoadImageFile( 0, JMConstants.Lucide( "check" ) );
		if ( m_BadgeRole ) m_BadgeRole.LoadImageFile( 0, JMConstants.Lucide( "shield" ) );
		if ( m_BadgeFlagged ) m_BadgeFlagged.LoadImageFile( 0, JMConstants.Lucide( "flag" ) );
		if ( m_BadgeDead ) m_BadgeDead.Show( false );
		if ( m_BadgeUncon ) m_BadgeUncon.Show( false );
		if ( m_BadgeBrokenLeg ) m_BadgeBrokenLeg.Show( false );
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

				array< string > roles = player.GetRoles();
				
				if ( roles && roles.Count() > 1 )
					Name.SetText( "[" + roles[1] + "] " + player.GetName() );
				else
					Name.SetText( player.GetName() );

				if ( GetPermissionsManager().GetClientGUID() == m_GUID )
					Name.SetColor( 0xFF2ECC71 );
				else if ( player.HasPermission( "COT" ) )
					Name.SetColor( 0xFFA85A32 );
				else
					Name.SetColor( 0xFFFFFFFF );

				SetChecked( JM_GetSelected().IsSelected( m_GUID ) );

				PlayerBase pb = player.PlayerObject;
				if ( pb )
				{
					if ( m_BadgeDead ) m_BadgeDead.Show( !pb.IsAlive() );
					if ( m_BadgeUncon ) m_BadgeUncon.Show( pb.IsUnconscious() );
				}
				else
				{
					if ( m_BadgeDead ) m_BadgeDead.Show( false );
					if ( m_BadgeUncon ) m_BadgeUncon.Show( false );
				}
				if ( m_BadgeFlagged ) m_BadgeFlagged.Show( false );
				if ( m_BadgeBrokenLeg ) m_BadgeBrokenLeg.Show( false );
				if ( m_BadgeSick ) m_BadgeSick.Show( false );
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

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
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
