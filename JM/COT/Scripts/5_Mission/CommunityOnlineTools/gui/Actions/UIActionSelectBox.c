// =============================================================================
//  UIActionSelectBox
//
//  Inline cycle-arrow selector backed by OptionSelectorMultistate. Good for
//  short fixed sets (modes, levels) where a full dropdown popup is overkill.
//  For larger lists use UIActionDropdown / CreateDropdownBox instead.
// =============================================================================
class UIActionSelectBox: UIActionBase
{
	protected Widget m_Frame;
	protected TextWidget m_Label;

	protected ref OptionSelectorMultistate m_Selection;

	void ~UIActionSelectBox()
	{
		if (!g_Game)
			return;

		delete m_Selection;
	}

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Frame, layoutRoot.FindAnyWidget( "action" ) );
		Class.CastTo( m_Label, layoutRoot.FindAnyWidget( "action_label" ) );
	}

	override void OnShow()  {}
	override void OnHide()  {}

	void SetSelections( notnull array<string> options )
	{
		// Guard: OptionSelectorMultistate VME's ("Invalid options") if the
		// options array is empty. Seed a single placeholder so the selector
		// has at least one valid entry; callers with a populated list pass
		// through unchanged.
		if ( options.Count() == 0 )
			options.Insert( "(empty)" );

		m_Selection = new OptionSelectorMultistate( m_Frame, 0, this, true, options );
		m_Selection.m_OptionChanged.Insert( OnSelectionChange );
		m_Selection.Enable();
	}

	// Backwards-compat alias for callers written against the (brief) Dropdown-style API.
	void SetItems( notnull array<string> options )
	{
		SetSelections( options );
	}

	override void Disable()
	{
		if ( m_Selection )
			m_Selection.Disable();
	}

	override void Enable()
	{
		if ( m_Selection )
			m_Selection.Enable();
	}

	override void SetLabel( string text )
	{
		text = Widget.TranslateString( text );
		if ( m_Label )
			m_Label.SetText( text );
	}

	override void SetSelection( int i, bool sendEvent = true )
	{
		if ( m_Selection )
			m_Selection.SetValue( i, sendEvent );
	}

	override int GetSelection()
	{
		if ( m_Selection )
			return m_Selection.GetValue();
		return -1;
	}

	bool OnSelectionChange()
	{
		if ( !m_HasCallback )
			return false;
		return CallEvent( UIEvent.CHANGE );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( !m_HasCallback )
			return false;
		bool ret = false;
		if ( w == m_Selection )
			ret = CallEvent( UIEvent.CLICK );
		return ret;
	}

	void SetSelectorWidth( float width )
	{
		if ( !m_Frame )
			return;
		float w, h;
		m_Frame.GetSize( w, h );
		m_Frame.SetSize( width, h );
		m_Frame.Update();
	}
}
