//! One tab strip a form has pinned over its content, and what it was pinned with.
//! JMFormBase keeps one per strip to re-pin when the strip wraps after OnResize has passed
//! (see JMFormBase.OnTabStripFitted) and to report its height (JMFormBase.GetPinnedStripHeight).
class JMPinnedStrip
{
	Widget Strip;

	//! The band between the strip and the content, e.g. the Weather form's mode banner. Null for none.
	Widget Band;
	Widget Content;

	//! What PinStripGeometry was last asked.
	float ContentHeight;
	int StripHeight;
	float BandHeight;

	//! The height the strip was actually pinned at, extra tab rows included.
	float PinnedStripHeight;
}
