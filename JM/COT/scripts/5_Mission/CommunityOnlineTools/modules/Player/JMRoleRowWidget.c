class JMRoleRowWidget extends ScriptedWidgetEventHandler 
{
	string Name;
	int Type;

	protected ref TextWidget Text;
	protected ref CheckBoxWidget  Checkbox;

	protected ref Widget layoutRoot;

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
		layoutRoot.Show( true );
		OnShow();
	}

	void Hide()
	{
		OnHide();
		layoutRoot.Show( false );
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
	}

	void OnDisable()
	{
	}

}