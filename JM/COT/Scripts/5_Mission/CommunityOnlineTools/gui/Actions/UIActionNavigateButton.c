class UIActionNavigateButton: UIActionBase
{
	protected TextWidget m_Text;
	
	protected ImageWidget m_Image_Left;
	protected ImageWidget m_Image_Right;

	protected ButtonWidget m_Button_Left;
	protected ButtonWidget m_Button_Right;

	override void OnInit() 
	{
		super.OnInit();

		Class.CastTo( m_Text, layoutRoot.FindAnyWidget( "action" ) );

		Class.CastTo( m_Image_Left, layoutRoot.FindAnyWidget( "image_left" ) );
		Class.CastTo( m_Image_Right, layoutRoot.FindAnyWidget( "image_right" ) );
		
		Class.CastTo( m_Button_Left, layoutRoot.FindAnyWidget( "action_button_left" ) );
		Class.CastTo( m_Button_Right, layoutRoot.FindAnyWidget( "action_button_right" ) );
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
		
		m_Text.SetText( text );
	}

	void SetImages( string imageL, string imageR )
	{
		m_Button_Left.SetText("");
		m_Button_Right.SetText("");

		//m_Image_Left.LoadImageFile( 0, imageL );
		//m_Image_Right.LoadImageFile( 0, imageR );

	}

	override string GetButton()
	{
		return "";
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		bool ret = false;

		if ( w == m_Button_Left )
		{
			ret = CallEvent( UIEvent.CLICK_LEFTSIDE );
		}
		else if ( w == m_Button_Right )
		{
			ret = CallEvent( UIEvent.CLICK_RIGHTSIDE );
		}

		return ret;
	}

	override bool CallEvent( UIEvent eid )
	{
		if ( !m_HasCallback )
			return false;

		GetGame().GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, new Param2< UIEvent, ref UIActionBase >( eid, this ) );

		return false;
	}
};
