// UIActionCollapsibleSection: a header button that shows/hides a content panel below it.
// Add children to the plain Widget returned by GetContent().
// Usage: UIActionManager.CreateCollapsibleSection( parent, "Title" ).
// Starts expanded; call SetExpanded(false) to start collapsed.
class UIActionCollapsibleSection: UIActionBase
{
	protected TextWidget  m_Label;
	protected ButtonWidget m_Button;
	protected ImageWidget  m_Arrow;
	protected Widget       m_Content;
	protected bool m_Expanded;

	//! Returns the content panel. Add child widgets here.
	Widget GetContent()
	{
		return m_Content;
	}

	bool IsExpanded()
	{
		return m_Expanded;
	}

	void SetExpanded( bool expanded )
	{
		m_Expanded = expanded;
		if ( m_Content )
			m_Content.Show( m_Expanded );
		UpdateArrow();
	}

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Label,   layoutRoot.FindAnyWidget( "action_label"   ) );
		Class.CastTo( m_Button,  layoutRoot.FindAnyWidget( "action_button"  ) );
		Class.CastTo( m_Arrow,   layoutRoot.FindAnyWidget( "action_arrow"   ) );
		Class.CastTo( m_Content, layoutRoot.FindAnyWidget( "action_content" ) );

		m_Expanded = true;
		UpdateArrow();
	}

	override void SetLabel( string text )
	{
		if ( m_Label )
			m_Label.SetText( text );
	}

	void Toggle()
	{
		SetExpanded( !m_Expanded );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == m_Button )
		{
			Toggle();
			CallEvent( UIEvent.CLICK );
			return true;
		}
		return false;
	}

	protected void UpdateArrow()
	{
		if ( !m_Arrow )
			return;

		// Pointing right = collapsed, pointing down = expanded
		if ( m_Expanded )
			m_Arrow.SetRotation( 0, 0, 0 );
		else
			m_Arrow.SetRotation( 0, 0, -90 );
	}
}
