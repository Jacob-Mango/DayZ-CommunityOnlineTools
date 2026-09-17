// =============================================================================
//  UIActionStagedIcon
//
//  Displays one icon at a time from an ordered list of (imagePath, color) stages.
//  Call SetStage(int) or NextStage() to advance.  Fires UIEvent.CHANGE when the
//  stage changes.  Optionally clickable (fires UIEvent.CLICK on press and
//  advances to the next stage automatically when m_CycleOnClick is true).
//
//  Usage:
//      m_Battery = UIActionManager.CreateStagedIcon( parent, this, "OnChange_Battery" );
//      m_Battery.AddStage( JMConstants.ICON_BATTERY_EMPTY,  0xFFFF4444 );
//      m_Battery.AddStage( JMConstants.ICON_BATTERY_LOW,    0xFFFFAA00 );
//      m_Battery.AddStage( JMConstants.ICON_BATTERY_MEDIUM, 0xFFFFFF44 );
//      m_Battery.AddStage( JMConstants.ICON_BATTERY_FULL,   0xFF44FF44 );
//      m_Battery.SetStage( 3 );
// =============================================================================

class JMStagedIconEntry
{
	string ImagePath;
	int    Color;

	void JMStagedIconEntry( string imagePath, int color )
	{
		ImagePath = imagePath;
		Color     = color;
	}
}

class UIActionStagedIcon: UIActionBase
{
	protected ButtonWidget m_Button;
	protected ImageWidget  m_Image;

	protected ref array<ref JMStagedIconEntry> m_Stages;
	protected int  m_CurrentStage;
	protected bool m_CycleOnClick;

	//! Screen position of the last right-click, for a handler that answers
	//! UIEvent.CLICK_RIGHT by opening a menu there.
	protected int  m_LastRightClickX;
	protected int  m_LastRightClickY;

	protected ref JMAnimColor m_ColorAnim;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Button, layoutRoot.FindAnyWidget( "action_button" ) );
		Class.CastTo( m_Image,  layoutRoot.FindAnyWidget( "action_image"  ) );

		m_Stages       = new array<ref JMStagedIconEntry>;
		m_CurrentStage = -1;
		m_CycleOnClick = false;
		m_ColorAnim    = new JMAnimColor();

		m_LastRightClickX = 0;
		m_LastRightClickY = 0;
	}

	//! Add a stage: icon path + tint colour (0xAARRGGBB).
	void AddStage( string imagePath, int color = 0xFFFFFFFF )
	{
		m_Stages.Insert( new JMStagedIconEntry( imagePath, color ) );

		if ( m_CurrentStage == -1 )
			SetStage( 0 );
	}

	//! Replace all stages at once and reset to stage 0.
	void SetStages( notnull array<string> imagePaths, int color = 0xFFFFFFFF )
	{
		m_Stages.Clear();
		foreach ( string path : imagePaths )
			m_Stages.Insert( new JMStagedIconEntry( path, color ) );

		m_CurrentStage = -1;
		if ( m_Stages.Count() > 0 )
			SetStage( 0 );
	}

	int GetStage()
	{
		return m_CurrentStage;
	}

	int GetStageCount()
	{
		return m_Stages.Count();
	}

	//! Jump to a specific stage index (clamped).
	void SetStage( int stage, bool sendEvent = true )
	{
		if ( m_Stages.Count() == 0 )
			return;

		if ( stage < 0 )
			stage = 0;
		if ( stage >= m_Stages.Count() )
			stage = m_Stages.Count() - 1;

		if ( stage == m_CurrentStage )
			return;

		m_CurrentStage = stage;
		ApplyStage();

		if ( sendEvent )
			CallEvent( UIEvent.CHANGE );
	}

	//! Advance to the next stage, wrapping around.
	void NextStage( bool sendEvent = true )
	{
		if ( m_Stages.Count() == 0 )
			return;

		int next = m_CurrentStage + 1;
		if ( next >= m_Stages.Count() )
			next = 0;

		SetStage( next, sendEvent );
	}

	//! When true, clicking the widget cycles to the next stage automatically.
	void SetCycleOnClick( bool cycle )
	{
		m_CycleOnClick = cycle;
	}

	override void SetIcon( string imagePath )
	{
		if ( m_Image )
		{
			m_Image.LoadImageFile( 0, imagePath );
			m_Image.SetImage( 0 );
		}
	}

	override void SetColor( int color )
	{
		if ( m_Image )
			m_Image.SetColor( color );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w != m_Button )
			return false;

		CallEvent( UIEvent.CLICK );

		if ( m_CycleOnClick )
			NextStage();

		return true;
	}

	//! Right-click fires UIEvent.CLICK_RIGHT and never cycles the stage. The
	//! cursor position is kept so a handler can open a context menu on it -
	//! ButtonWidget raises no click at all for the right button.
	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( button != MouseState.RIGHT || w != m_Button )
			return super.OnMouseButtonDown( w, x, y, button );

		m_LastRightClickX = x;
		m_LastRightClickY = y;

		CallEvent( UIEvent.CLICK_RIGHT );

		return true;
	}

	int GetLastRightClickX()
	{
		return m_LastRightClickX;
	}

	int GetLastRightClickY()
	{
		return m_LastRightClickY;
	}

	override bool IsFocusWidget( Widget widget )
	{
		return widget == m_Button;
	}

	private void ApplyStage()
	{
		if ( !m_Image || m_CurrentStage < 0 || m_CurrentStage >= m_Stages.Count() )
			return;

		JMStagedIconEntry entry = m_Stages[ m_CurrentStage ];
		m_Image.LoadImageFile( 0, entry.ImagePath );
		m_Image.SetImage( 0 );
		m_ColorAnim.SetTarget( entry.Color, 6.0 );
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		if ( m_ColorAnim.Step( timeSlice ) && m_Image )
			m_Image.SetColor( m_ColorAnim.Value );
	}
}
