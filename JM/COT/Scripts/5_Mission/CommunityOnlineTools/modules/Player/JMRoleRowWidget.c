class JMRoleRowWidget extends ScriptedWidgetEventHandler 
{
	string Name;
	int Type;

	protected TextWidget Text;
	protected CheckBoxWidget  Checkbox;

	protected Widget layoutRoot;

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );

		Init();
	}


	void Init() 
	{
		Text = TextWidget.Cast(layoutRoot.FindAnyWidget("role_name"));
		Checkbox = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("role_setting"));
	}

	void Show()
	{
		#ifdef COT_DEBUGLOGS
		Print(( "+" + this + "::Show" ));
		#endif

		layoutRoot.Show( true );
		OnShow();

		#ifdef COT_DEBUGLOGS
		Print(( "-" + this + "::Show" ));
		#endif
	}

	void Hide()
	{
		#ifdef COT_DEBUGLOGS
		Print(( "+" + this + "::Hide" ));
		#endif

		OnHide();
		layoutRoot.Show( false );

		#ifdef COT_DEBUGLOGS
		Print(( "-" + this + "::Hide" ));
		#endif
	}

	void OnShow()
	{
	}

	void OnHide() 
	{
	}

	ref Widget GetLayoutRoot() 
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
}