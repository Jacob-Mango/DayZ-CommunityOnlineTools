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
		//Print( "+" + this + "::Show" );

		layoutRoot.Show( true );
		OnShow();

		//Print( "-" + this + "::Show" );
	}

	void Hide()
	{
		//Print( "+" + this + "::Hide" );

		OnHide();
		layoutRoot.Show( false );

		//Print( "-" + this + "::Hide" );
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