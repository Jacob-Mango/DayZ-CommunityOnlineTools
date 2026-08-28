// =============================================================================
//  JMScrollableSection
//
//  Convenience wrapper around a UIActionScroller + an inner content grid.
//  Avoids the three-step boilerplate (CreateScroller -> GetContentWidget ->
//  CreateGridSpacer) that every form repeats, and provides a single
//  UpdateScroller() call to wire into OnResize().
//
//  Usage
//  -----
//  In your form class:
//
//      private ref JMScrollableSection m_Scroller;
//
//  In OnInit():
//
//      m_Scroller = UIActionManager.CreateScrollableSection( layoutRoot.FindAnyWidget("panel") );
//      Widget content = m_Scroller.GetContent();
//      // add widgets to content ...
//
//  In OnResize():
//
//      override void OnResize( float w, float h ) { m_Scroller.UpdateScroller(); }
//
// =============================================================================
class JMScrollableSection
{
	private UIActionScroller m_Scroller;
	private Widget           m_Content;

	void JMScrollableSection( UIActionScroller scroller, Widget content )
	{
		m_Scroller = scroller;
		m_Content  = content;
	}

	//! The inner grid widget - add child widgets here.
	Widget GetContent()
	{
		return m_Content;
	}

	UIActionScroller GetScroller()
	{
		return m_Scroller;
	}

	//! Call this from your form's OnResize() to keep the scroller in sync.
	void UpdateScroller()
	{
		if ( m_Scroller )
			m_Scroller.UpdateScroller();
	}
}
