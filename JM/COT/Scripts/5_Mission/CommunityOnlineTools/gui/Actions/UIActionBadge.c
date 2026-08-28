// =============================================================================
//  UIActionBadge
//
//  Status chip: a label on the left and a colored pill on the right.
//  Use SetStatus/SetBadgeColor to update the display.
//
//  Read-only by default. SetClickable( true ) plus a callback turns the chip
//  into its own control - the whole badge becomes a hit area, the pill lights
//  on hover, and a click fires UIEvent.CLICK - so a flag an admin can change is
//  toggled where it is READ instead of only from a checkbox on another tab.
//  The badge holds no state of its own: the callback decides what the new value
//  is and repaints it with SetStatus.
//
//  Usage:
//      m_StatusBadge = UIActionManager.CreateBadge( parent, "Connection" );
//      m_StatusBadge.SetStatus( "ONLINE", ARGB(255,  40, 180, 80) );
//      m_StatusBadge.SetStatus( "BANNED", ARGB(255, 200,  40, 40) );
// =============================================================================
class UIActionBadge: UIActionBase
{
	protected TextWidget   m_Label;
	protected Widget       m_Pill;
	protected TextWidget   m_StatusText;
	protected string       m_StatusValue;
	protected ButtonWidget m_Button;

	//! Off means the hit area is hidden and the chip is pure display.
	protected bool m_Clickable;
	protected bool m_Hovered;

	//! Resting pill colour, kept so hover can lift it and put it back.
	protected int m_PillColor;

	//! How far hover lifts the pill. Enough to read as "this answers a click"
	//! without turning into a second status colour.
	static const float HOVER_LIFT = 0.35;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Label,      layoutRoot.FindAnyWidget( "action_label"  ) );
		Class.CastTo( m_Pill,       layoutRoot.FindAnyWidget( "action_pill"   ) );
		Class.CastTo( m_StatusText, layoutRoot.FindAnyWidget( "action"        ) );
		Class.CastTo( m_Button,     layoutRoot.FindAnyWidget( "action_button" ) );

		m_Clickable = false;
		m_Hovered   = false;
		m_PillColor = JMTheme.SUCCESS_DIM;

		if ( m_Button )
			m_Button.Show( false );
	}

	//! Turn the chip into a control. The hit area covers the whole badge, so
	//! the label answers the click as well as the pill.
	void SetClickable( bool enabled )
	{
		m_Clickable = enabled;

		if ( m_Button )
			m_Button.Show( enabled );

		if ( !enabled )
			m_Hovered = false;

		ApplyPillColor();
	}

	bool IsClickable()
	{
		return m_Clickable;
	}

	override void SetLabel( string text )
	{
		text = Widget.TranslateString( text );
		if ( m_Label )
			m_Label.SetText( text );
	}

	//! Set the badge text and background color in one call.
	void SetStatus( string status, int color = JMTheme.SUCCESS_DIM )
	{
		m_StatusValue = Widget.TranslateString( status );
		if ( m_StatusText )
			m_StatusText.SetText( m_StatusValue );

		SetBadgeColor( color );
	}

	override void SetText( string text )
	{
		m_StatusValue = Widget.TranslateString( text );
		if ( m_StatusText )
			m_StatusText.SetText( m_StatusValue );
	}

	override string GetText()
	{
		return m_StatusValue;
	}

	void SetBadgeColor( int color )
	{
		m_PillColor = color;

		ApplyPillColor();
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( !m_Clickable || w != m_Button )
			return false;

		return CallEvent( UIEvent.CLICK );
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		super.OnMouseEnter( w, x, y );

		if ( !m_Clickable )
			return false;

		m_Hovered = true;
		ApplyPillColor();

		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		super.OnMouseLeave( w, enterW, x, y );

		m_Hovered = false;
		ApplyPillColor();

		return false;
	}

	//! Only a clickable chip lifts: a read-only badge under the pointer must
	//! keep saying exactly what it said before.
	protected void ApplyPillColor()
	{
		if ( !m_Pill )
			return;

		if ( m_Clickable && m_Hovered )
			m_Pill.SetColor( JMTheme.Lighten( m_PillColor, HOVER_LIFT ) );
		else
			m_Pill.SetColor( m_PillColor );
	}
}
