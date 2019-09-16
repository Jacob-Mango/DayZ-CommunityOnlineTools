class UIActionCheckbox extends UIActionBase 
{
	protected ref TextWidget m_Label;
	protected ref CheckBoxWidget m_Checkbox;
	
	override void OnInit() 
	{
		super.OnInit();
		
		m_Label = TextWidget.Cast(layoutRoot.FindAnyWidget("action_label"));
		m_Checkbox = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("action"));
	}

	override void OnShow()
	{
	}

	override void OnHide() 
	{
	}

	override void SetLabel( string text )
	{
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
}