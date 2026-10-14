//  UIActionImageButtonToggle - UIActionButtonToggle for UIActionImageButton: a
//  two-state pill showing an image per state (give it a tooltip). Fires
//  UIEvent.CLICK after the flip, so IsToggled() is the new value.
//  CreateImageButtonToggle( row, imgA, imgB, this, "Event_X", 0.2 );
class UIActionImageButtonToggle: UIActionImageButton
{
	protected bool   m_IsToggled;
	protected string m_ImageOff;
	protected string m_ImageOn;

	bool IsToggled()
	{
		return m_IsToggled;
	}

	void SetImageToggle( string imageOff, string imageOn )
	{
		m_ImageOff = imageOff;
		m_ImageOn  = imageOn;

		ApplyImage();
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
