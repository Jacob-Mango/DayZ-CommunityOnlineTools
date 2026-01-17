class UIActionImageButton: UIActionButton 
{
	protected ImageWidget m_Image;

	override void OnInit() 
	{
		Class.CastTo( m_Button, layoutRoot.FindAnyWidget( "action_button" ) );
		Class.CastTo( m_Image, layoutRoot.FindAnyWidget( "action_image" ) );
	}

	void SetImage( string image )
	{
		m_Image.LoadImageFile(0, image);
	}
}
