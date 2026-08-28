// =============================================================================
//  UIActionImageButtonToggle
//
//  What UIActionButtonToggle is to UIActionButton, this is to
//  UIActionImageButton: one pill that flips between two states, showing an
//  image for each instead of a label.
//
//  Meant for a control whose two states have an obvious glyph and no room for
//  a word - a sort direction, a filter on/off, a lock. Give it a tooltip: an
//  icon-only button says nothing until it is hovered.
//
//  Fires UIEvent.CLICK after the state has flipped, so the callback can read
//  IsToggled() and get the new value.
//
//  Usage:
//      m_Sort = UIActionManager.CreateImageButtonToggle( row,
//          JMConstants.Lucide( "arrow-down-a-z" ),
//          JMConstants.Lucide( "arrow-down-z-a" ),
//          this, "Event_UpdatePlayerList", 0.2 );
// =============================================================================
class UIActionImageButtonToggle: UIActionImageButton
{
	protected bool   m_IsToggled;
	protected string m_ImageOff;
	protected string m_ImageOn;

	void SetImageToggle( string imageOff, string imageOn )
	{
		m_ImageOff = imageOff;
		m_ImageOn  = imageOn;

		ApplyImage();
	}

	bool IsToggled()
	{
		return m_IsToggled;
	}

	//! Set the state without firing the callback - for restoring a remembered
	//! value while the form is being built.
	void SetToggle( bool state )
	{
		m_IsToggled = state;
		ApplyImage();
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		bool ret = false;

		if ( w == m_Button )
		{
			m_IsToggled = !m_IsToggled;
			ApplyImage();

			ret = CallEvent( UIEvent.CLICK );
		}

		return ret;
	}

	protected void ApplyImage()
	{
		if ( m_IsToggled )
			SetImage( m_ImageOn );
		else
			SetImage( m_ImageOff );
	}
}
