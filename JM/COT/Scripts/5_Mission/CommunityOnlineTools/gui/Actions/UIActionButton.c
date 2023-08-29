class UIActionButton: UIActionBase 
{
	protected TextWidget m_Text;
	protected ButtonWidget m_Button;

	override void OnInit() 
	{
		super.OnInit();
		
		Class.CastTo( m_Button, layoutRoot.FindAnyWidget( "action_button" ) );
		Class.CastTo( m_Text, layoutRoot.FindAnyWidget( "action" ) );
	}

	override void OnShow()
	{
	}

	override void OnHide() 
	{
	}

	override void SetButton( string text )
	{
		text = Widget.TranslateString( text );
		
		m_Button.SetName( text );
		m_Text.SetText( text );
	}

	override string GetButton()
	{
		return m_Button.GetName();
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		bool ret = false;

		if ( w == m_Button )
		{
			ret = CallEvent( UIEvent.CLICK );
		}

		return ret;
	}

	void SetColor(int color)
	{
		m_Button.SetColor(color);
	}

	override bool CallEvent( UIEvent eid )
	{
		if ( !m_HasCallback )
			return false;

		GetGame().GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, new Param2< UIEvent, ref UIActionBase >( eid, this ) );

		return false;
	}
};
