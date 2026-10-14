//  UIActionBadge: status chip, label left and colored pill right; update with SetStatus/SetBadgeColor.
//  Read-only by default; SetClickable( true ) + callback makes it a control that fires UIEvent.CLICK.
//  It holds no state: the callback decides the new value and repaints with SetStatus.
//  Usage: m_Badge = UIActionManager.CreateBadge( parent, "Connection" ); m_Badge.SetStatus( "ONLINE", ARGB(255, 40, 180, 80) );
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

	bool IsClickable()
	{
		return m_Clickable;
	}

	void SetBadgeColor( int color )
	{
		m_PillColor = color;

		ApplyPillColor();
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

	//! Set the badge text and background color in one call.
	void SetStatus( string status, int color = JMTheme.SUCCESS_DIM )
	{
		m_StatusValue = status;
		if ( m_StatusText )
			m_StatusText.SetText( m_StatusValue );

		SetBadgeColor( color );
	}

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

	override void SetLabel( string text )
	{
		text = text;
		if ( m_Label )
			m_Label.SetText( text );
	}

	override void SetText( string text )
	{
		m_StatusValue = text;
		if ( m_StatusText )
			m_StatusText.SetText( m_StatusValue );
	}

	override string GetText()
	{
		return m_StatusValue;
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
