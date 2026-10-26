class JMConfirmationForm: JMConfirmation 
{
	protected JMWindowBase m_Window;

	override void Init( Managed base )
	{
		Class.CastTo( m_Window, base );

		OnInit();
	}

	protected override void CallCallback( string callback )
	{
		if ( callback != "" )
		{
			auto param = new Param1<JMConfirmation>( this );
			g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallByName( m_Window.GetForm(), callback, param );
		}
	}
}