//! One tab strip a form has pinned over its content, and what it was pinned with.
//!
//! JMFormBase keeps one per strip so it can pin again when the tab strip wraps or unwraps after
//! the window's own OnResize has come and gone (see JMFormBase.OnTabStripFitted), and so a form
//! can ask how tall a strip really is now (JMFormBase.GetPinnedStripHeight).
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
