class JMRoleRowWidget: COT_ScriptedWidgetEventHandler 
{
#ifdef DIAG
	static int s_JMRoleRowWidgetCount;
#endif

	string Name;
	int Type;

	private ButtonWidget m_Button;
	protected TextWidget Text;
	protected CheckBoxWidget  Checkbox;

	protected Widget layoutRoot;

	void JMRoleRowWidget()
	{
	#ifdef DIAG
		s_JMRoleRowWidgetCount++;
	#ifdef COT_DEBUGLOGS
		CF_Log.Info("JMRoleRowWidget count: " + s_JMRoleRowWidgetCount);
	#endif
	#endif
	}

	void ~JMRoleRowWidget()
	{
		if (!g_Game)
			return;

	#ifdef COT_DEBUGLOGS
		auto trace = CF_Trace_0(this);
	#endif

	#ifdef DAYZ_1_28
		DestroyWidget(layoutRoot);
	#endif

	#ifdef DIAG
		s_JMRoleRowWidgetCount--;
		if (s_JMRoleRowWidgetCount <= 0)
			CF_Log.Info("JMRoleRowWidget count: " + s_JMRoleRowWidgetCount);
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
		Text = TextWidget.Cast(layoutRoot.FindAnyWidget("role_name"));
		Class.CastTo( m_Button, layoutRoot.FindAnyWidget( "button" ) );
		Checkbox = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("role_setting"));
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

	bool IsChecked()
	{
		return Checkbox.IsChecked();
	}

	void SetChecked( bool checked )
	{
		Checkbox.SetChecked( checked );
	}

	void InitRole( string role )
	{
		Name = role;

		Text.SetText( Name );
	}

	void Enable()
	{
		OnEnable();
	}

	void Disable()
	{
		OnDisable();
	}

	void OnEnable()
	{
		Checkbox.Enable( true );
		Checkbox.Show( true );
	}

	void OnDisable()
	{
		Checkbox.Enable( false );
		Checkbox.Show( false );
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_Button)
			SetChecked(!IsChecked());

		return super.OnClick(w, x, y, button);
	}
}
