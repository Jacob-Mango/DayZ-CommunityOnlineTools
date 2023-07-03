class UIActionImage: UIActionBase 
{
	protected ImageWidget m_Label;

	override void OnInit() 
	{
		super.OnInit();
		
		Class.CastTo( m_Label, layoutRoot.FindAnyWidget( "action_label" ) );
	}

	override void OnShow()
	{
	}

	override void OnHide() 
	{
	}

	void SetImage( string image )
	{
		m_Label.LoadImageFile(0, image);
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

	override bool CallEvent( UIEvent eid )
	{
		GetGame().GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, new Param2< UIEvent, ref UIActionBase >( eid, this ) );

		return false;
	}
};
