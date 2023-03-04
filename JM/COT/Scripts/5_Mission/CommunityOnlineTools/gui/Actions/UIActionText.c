class UIActionText: UIActionBase 
{
	protected string m_ActualText;

	protected TextWidget m_Label;
	protected TextWidget m_Text;

	override void OnInit() 
	{
		super.OnInit();
		
		Class.CastTo( m_Label, layoutRoot.FindAnyWidget( "action_label" ) );
		Class.CastTo( m_Text, layoutRoot.FindAnyWidget( "action" ) );
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

	override void SetText( string text )
	{
		m_ActualText = Widget.TranslateString( text );

		m_Text.SetText( m_ActualText );
	}

	void SetLabelHAlign( UIActionHAlign type )
	{
		switch ( type )
		{
		case UIActionHAlign.CENTER:
			m_Label.SetFlags( m_Label.GetFlags() | WidgetFlags.CENTER );
			break;
		case UIActionHAlign.LEFT:
			// m_Label.SetFlags( m_Label.GetFlags() );
			break;
		case UIActionHAlign.RIGHT:
			m_Label.SetFlags( m_Label.GetFlags() | WidgetFlags.RALIGN );
			break;
		}
	}
	
	void SetLabelVAlign( UIActionVAlign type )
	{
		switch ( type )
		{
		case UIActionVAlign.CENTER:
			m_Label.SetFlags( m_Label.GetFlags() | WidgetFlags.VCENTER );
			break;
		case UIActionVAlign.TOP:
			// m_Label.SetFlags( m_Label.GetFlags() );
			break;
		case UIActionVAlign.BOTTOM:
			// m_Label.SetFlags( m_Label.GetFlags() | WidgetFlags.RALIGN );
			break;
		}
	}
	
	void SetTextHAlign( UIActionHAlign type )
	{
		switch ( type )
		{
		case UIActionHAlign.CENTER:
			m_Text.SetFlags( m_Text.GetFlags() | WidgetFlags.CENTER );
			break;
		case UIActionHAlign.LEFT:
			// m_Text.SetFlags( m_Text.GetFlags() );
			break;
		case UIActionHAlign.RIGHT:
			m_Text.SetFlags( m_Text.GetFlags() | WidgetFlags.RALIGN );
			break;
		}
	}
	
	void SetTextVAlign( UIActionVAlign type )
	{
		switch ( type )
		{
		case UIActionVAlign.CENTER:
			m_Text.SetFlags( m_Text.GetFlags() | WidgetFlags.VCENTER );
			break;
		case UIActionVAlign.TOP:
			// m_Text.SetFlags( m_Text.GetFlags() );
			break;
		case UIActionVAlign.BOTTOM:
			// m_Text.SetFlags( m_Text.GetFlags() | WidgetFlags.RALIGN );
			break;
		}
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{	
		if ( !m_HasCallback )
		{
			if ( w == m_Text )
			{
				GetGame().CopyToClipboard( m_ActualText );
			}
			return false;
		}

		bool ret = false;

		if ( w == m_Text )
		{
			ret = CallEvent( UIEvent.CLICK );
		}

		return ret;
	}

	override bool CallEvent( UIEvent eid )
	{
		GetGame().GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, new Param2< UIEvent, ref UIActionBase >( eid, this ) );

		return false;
	}
};
