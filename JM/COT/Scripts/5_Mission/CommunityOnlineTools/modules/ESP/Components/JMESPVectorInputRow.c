//! X/Y/Z input row without UIActionEditableVector's label column (same action_x/y/z chrome), so
//! JMESPTransformPopup gets three full-width fields. Rings are NOT the inherited OnRegisterChrome/
//! ApplyChrome path (one hover flag for the whole control would light Y and Z when hovering X);
//! each ring follows its own field's hover/focus state.
class JMESPVectorInputRow: UIActionBase
{
	protected EditBoxWidget m_TextX;
	protected EditBoxWidget m_TextY;
	protected EditBoxWidget m_TextZ;
	protected Widget m_RingX;
	protected Widget m_RingY;
	protected Widget m_RingZ;
	protected int m_RingRestColor;
	protected bool m_HoverX;
	protected bool m_HoverY;
	protected bool m_HoverZ;

	//! See UIActionEditableVector's identical field for why this exists -
	//! a live SetValue() has to skip a box the user is mid-edit on.
	protected bool m_Edited;

	bool IsEdited()
	{
		return m_Edited;
	}

	void SetEdited( bool edited )
	{
		m_Edited = edited;
	}

	static JMESPVectorInputRow Create( notnull Widget parent )
	{
		Widget widget = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/esp/JMESPVectorInputRow.layout", parent );

		if ( !widget )
			return null;

		JMESPVectorInputRow row;
		widget.GetScript( row );

		return row;
	}

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_TextX, layoutRoot.FindAnyWidget( "action_x" ) );
		Class.CastTo( m_TextY, layoutRoot.FindAnyWidget( "action_y" ) );
		Class.CastTo( m_TextZ, layoutRoot.FindAnyWidget( "action_z" ) );

		m_RingX = layoutRoot.FindAnyWidget( "action_x_ring" );
		m_RingY = layoutRoot.FindAnyWidget( "action_y_ring" );
		m_RingZ = layoutRoot.FindAnyWidget( "action_z_ring" );

		if ( m_RingX )
			m_RingRestColor = m_RingX.GetColor();

		SetValue( vector.Zero );
	}

	override bool IsFocusWidget( Widget widget )
	{
		if ( widget == m_TextX )
			return true;
		if ( widget == m_TextY )
			return true;
		if ( widget == m_TextZ )
			return true;

		return false;
	}

	override void SetValue( vector v )
	{
		if ( IsFocused() || m_Edited )
			return;

		m_TextX.SetText( v[0].ToString() );
		m_TextY.SetText( v[1].ToString() );
		m_TextZ.SetText( v[2].ToString() );
	}

	override vector GetValue()
	{
		return Vector( m_TextX.GetText().ToFloat(), m_TextY.GetText().ToFloat(), m_TextZ.GetText().ToFloat() );
	}

	override bool OnMouseWheel( Widget w, int x, int y, int wheel )
	{
		if ( ( w == m_TextX || w == m_TextY || w == m_TextZ ) && IsFocusWidget( GetFocus() ) )
		{
			EditBoxWidget editw = EditBoxWidget.Cast( w );
			IncrementValue( editw, wheel );

			return true;
		}

		return super.OnMouseWheel( w, x, y, wheel );
	}

	void IncrementValue( EditBoxWidget w, int wheel )
	{
		float multiplier = 0.1;
		if ( SHIFT() )
			multiplier = multiplier * 10;

		float currValue = w.GetText().ToFloat();
		currValue = currValue + ( wheel * multiplier );
		w.SetText( currValue.ToString() );

		m_Edited = true;
	}

	override bool OnKeyPress( Widget w, int x, int y, int key )
	{
		m_Edited = true;

		return super.OnKeyPress( w, x, y, key );
	}

	//! Per-field hover, not the shared m_ChromeHovered flag - deliberately
	//! does not call super, so the inherited all-rings-together path never
	//! engages.
	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( w == m_TextX )
			m_HoverX = true;
		else if ( w == m_TextY )
			m_HoverY = true;
		else if ( w == m_TextZ )
			m_HoverZ = true;
		else
			return false;

		return true;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( w == m_TextX )
			m_HoverX = false;
		else if ( w == m_TextY )
			m_HoverY = false;
		else if ( w == m_TextZ )
			m_HoverZ = false;
		else
			return false;

		return true;
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		Widget focused = GetFocus();

		ApplyFieldRing( m_RingX, focused == m_TextX, m_HoverX );
		ApplyFieldRing( m_RingY, focused == m_TextY, m_HoverY );
		ApplyFieldRing( m_RingZ, focused == m_TextZ, m_HoverZ );
	}

	protected void ApplyFieldRing( Widget ring, bool focused, bool hovered )
	{
		if ( !ring )
			return;

		int color = m_RingRestColor;

		if ( focused )
			color = JMTheme.BORDER_RING;
		else if ( hovered )
			color = JMTheme.BORDER_RING_HOVER;

		ring.SetColor( color );
	}
}
