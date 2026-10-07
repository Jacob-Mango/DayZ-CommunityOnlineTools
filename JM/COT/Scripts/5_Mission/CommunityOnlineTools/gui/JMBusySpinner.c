//! A spinning loader drawn over the middle of a widget while something is being fetched.
//!
//!     m_Busy = new JMBusySpinner( someWidget );
//!     ...every frame:  m_Busy.Update( isWaiting );
//!
//! It is created as the LAST child of `parent`, so it is drawn above whatever the parent
//! already holds, and it takes the pointer while shown, so what is underneath cannot be
//! clicked until it goes away.
class JMBusySpinner
{
	static const float DEGREES_PER_SECOND = 300;

	protected UIActionImage m_Image;
	protected Widget m_Root;
	protected float m_Size;
	protected float m_Top;

	//! `top` is the distance from the top of the parent in pixels, or -1 to centre it vertically.
	void JMBusySpinner( notnull Widget parent, float size = 32, float top = -1 )
	{
		m_Size = size;
		m_Top = top;

		m_Image = UIActionManager.CreateImage( parent, JMConstants.Lucide( "loader-circle" ), NULL, "", UIActionHAlign.CENTER, UIActionHAlign.CENTER );

		if ( !m_Image )
			return;

		m_Root = m_Image.GetLayoutRoot();
		m_Root.SetFlags( WidgetFlags.HEXACTPOS | WidgetFlags.VEXACTPOS | WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE, true );
		m_Root.SetSize( m_Size, m_Size );
		m_Root.Show( false );
	}

	//! Call every frame: shows or hides the loader and turns it.
	void Update( bool busy )
	{
		if ( !m_Root )
			return;

		if ( m_Root.IsVisible() != busy )
			m_Root.Show( busy );

		if ( !busy )
			return;

		float parentW;
		float parentH;
		m_Root.GetParent().GetScreenSize( parentW, parentH );

		float y = m_Top;
		if ( y < 0 )
			y = ( parentH - m_Size ) * 0.5;

		m_Root.SetPos( ( parentW - m_Size ) * 0.5, y );

		ImageWidget spinner;
		if ( Class.CastTo( spinner, m_Root.FindAnyWidget( "action_label" ) ) )
		{
			float angle = g_Game.GetTickTime() * DEGREES_PER_SECOND;
			angle = angle - Math.Floor( angle / 360.0 ) * 360.0;

			spinner.SetRotation( 0, 0, angle );
		}
	}
}
