class JMPermissionRowWidget: COT_ScriptedWidgetEventHandler 
{
	// I dont like it, TODO: check if we could do it in a better way
	private ref JMPlayerForm m_PlayerForm;

	ref JMPermission m_Permission;

	private Widget layoutRoot;

	private TextWidget m_txt_PermissionIndent;
	private TextWidget m_txt_PermissionName;
	private CheckBoxWidget m_PermissionCheckbox;

	private bool m_IsEnabled = false;

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );

		Init();
	}

	void ~JMPermissionRowWidget()
	{
		if (!GetGame())
			return;

	#ifdef COT_DEBUGLOGS
		auto trace = CF_Trace_0(this);
	#endif

		DestroyWidget(layoutRoot);
	}

	void Init() 
	{
		Class.CastTo( m_txt_PermissionIndent, layoutRoot.FindAnyWidget( "permission_indent" ) );
		Class.CastTo( m_txt_PermissionName, layoutRoot.FindAnyWidget( "permission_name" ) );
		Class.CastTo( m_PermissionCheckbox, layoutRoot.FindAnyWidget( "permission_setting" ) );

		Disable();
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == m_PermissionCheckbox )
		{
			OnPermissionStateChanged();
			m_PlayerForm.OnPermissionUpdated();
			return true;
		}

		return false;
	}

	void Show()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Show" );
		#endif

		layoutRoot.Show( true );
		OnShow();

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Show" );
		#endif
	}

	void Hide()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Hide" );
		#endif

		OnHide();
		layoutRoot.Show( false );

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Hide" );
		#endif
	}

	void OnShow()
	{
	}

	void OnHide() 
	{
	}

	Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	void OnPermissionStateChanged()
	{
		if (m_Permission)
		{
			int type;
			if (m_PermissionCheckbox.IsChecked())
				type = JMPermissionType.ALLOW;
			else
				type = JMPermissionType.DISALLOW;

			CF_Log.Debug(ToString() + "::OnPermissionStateChanged - %1 %2 -> %3", m_Permission.GetFullName(), typename.EnumToString(JMPermissionType, m_Permission.Type), typename.EnumToString(JMPermissionType, type));

			m_Permission.Type = type;
		}
	}

	void InitPermission(JMPermission permission, JMPlayerForm playerform)
	{
		m_Permission = permission;
		permission.View = layoutRoot;

		m_txt_PermissionIndent.SetText( permission.Indent );
		permission.Indent = "";
		
		m_txt_PermissionName.SetText( permission.Name );
		m_PermissionCheckbox.SetChecked(permission.Type != JMPermissionType.DISALLOW);

		m_PlayerForm = playerform;

		UpdatePermission();
	}

	void UpdatePermission()
	{
		if (ShouldHidePermission(m_Permission.Parent))
		{
			Disable();

			// less confusing to disable childends as well
			m_PermissionCheckbox.SetChecked(false);
			OnPermissionStateChanged();
		}
		else
		{
			Enable();
		}
	}

	bool ShouldHidePermission(JMPermission permission)
	{
		if (!permission)
			return false;
		
		if (permission == permission.Root)
			return false;

		if (permission.Type == JMPermissionType.DISALLOW)
			return true;

		return ShouldHidePermission(permission.Parent);
	}

	bool IsPermissionChecked()
	{
		return m_PermissionCheckbox.IsChecked();
	}

	bool IsEnabled()
	{
		return m_IsEnabled;
	}

	void Enable()
	{
		m_IsEnabled = OnEnable();
	}

	void Disable()
	{
		m_IsEnabled = !OnDisable();
	}

	bool OnEnable()
	{
		Show();
		return true;
	}

	bool OnDisable()
	{
		Hide();
		return true;
	}
};
