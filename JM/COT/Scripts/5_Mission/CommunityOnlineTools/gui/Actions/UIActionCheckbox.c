class UIActionCheckbox: UIActionBase 
{
	protected TextWidget m_Label;
	protected CheckBoxWidget m_Checkbox;
	
	override void OnInit() 
	{
		super.OnInit();
		
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

		if ( w == m_Checkbox )
		{
			ret = CallEvent( UIEvent.CLICK );
		}

		return ret;
	}

	override bool CallEvent( UIEvent eid )
	{
		if ( !m_HasCallback ) return false;

		GetGame().GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, new Param2< UIEvent, ref UIActionBase >( eid, this ) );

		return false;
	}
};
