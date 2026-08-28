enum UIEvent 
{
	CLICK = 0,
	CLICK_RIGHTSIDE,
	CLICK_LEFTSIDE,
	CHANGE,
	MOUSEWHEEL,
	//! Right mouse button pressed on the control. Distinct from
	//! CLICK_RIGHTSIDE, which means "the right half of the widget was clicked"
	//! with the LEFT button. Appended, never inserted - the numeric values of
	//! the entries above are compared across the client/server boundary.
	CLICK_RIGHT,
	//! Pointer entered / left a control. Used by UIActionDataTable to drive a
	//! per-row hover preview; the row under the cursor is read back with
	//! GetHoveredRow().
	MOUSE_ENTER,
	MOUSE_LEAVE
}
