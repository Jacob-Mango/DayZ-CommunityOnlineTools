class UIActionImage: UIActionBase 
{
	protected ImageWidget m_Label;

	//! LoadImageFile only fills slot 0; SetImage picks it. Without the second
	//! call the widget can keep drawing nothing at all, which is invisible
	//! rather than obviously broken.
	void SetImage( string image )
	{
		if ( !m_Label )
			return;

		m_Label.LoadImageFile( 0, image );
		m_Label.SetImage( 0 );
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
}
