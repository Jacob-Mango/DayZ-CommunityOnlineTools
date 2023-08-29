class JMConfirmationForm: JMConfirmation 
{
	#ifndef CF_WINDOWS
	private JMWindowBase m_Window;

	override void Init( Managed base )
	{
		Class.CastTo( m_Window, base );

		OnInit();
	}

	protected override void CallCallback( string callback )
	{
		if ( callback != "" )
		{
			GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallByName( m_Window.GetForm(), callback, new Param1<JMConfirmation>( this ) );
		}
	}
	#endif
}