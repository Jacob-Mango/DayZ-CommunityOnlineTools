class UIActionCheckbox: UIActionBase 
{
	protected ButtonWidget m_Button;
	protected TextWidget m_Label;
	protected CheckBoxWidget m_Checkbox;
	
	override void OnInit() 
	{
		super.OnInit();
		
		Class.CastTo( m_Button, layoutRoot.FindAnyWidget( "action_button" ) );
		Class.CastTo( m_Label, layoutRoot.FindAnyWidget( "action_label" ) );
		Class.CastTo( m_Checkbox, layoutRoot.FindAnyWidget( "action" ) );
	}

	override void OnShow()
	{
	}

	override void OnHide() 
	{
	}

	override void SetLabel( string text )
	{
		text = Widget.TranslateString( text );
		
		m_Label.SetText( text );
	}

	override void SetChecked( bool checked )
	{
		m_Checkbox.SetChecked( checked );
	}

	override bool IsChecked()
	{
		return m_Checkbox.IsChecked();
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		bool ret = false;

		if ( w == m_Checkbox || w == m_Button )
		{
			if (w == m_Button)
				SetChecked(!IsChecked());

			ret = CallEvent( UIEvent.CLICK );
		}

		return ret;
	}
}
