// JMScrollableSection: wraps a UIActionScroller + inner content grid, replacing the
// CreateScroller -> GetContentWidget -> CreateGridSpacer boilerplate. Create with
// UIActionManager.CreateScrollableSection( root ), add widgets to GetContent(), and call
// UpdateScroller() from OnResize().
class JMScrollableSection
{
	protected UIActionScroller m_Scroller;
	protected Widget           m_Content;

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
