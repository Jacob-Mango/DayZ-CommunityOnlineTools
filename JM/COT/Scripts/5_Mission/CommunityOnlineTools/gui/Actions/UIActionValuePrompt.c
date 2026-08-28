// =============================================================================
//  UIActionValuePrompt
//
//  A small modal that asks for ONE value and answers with Confirm or Cancel.
//  Two shapes, one widget: a slider for a number, a dropdown for a choice out
//  of a fixed list. Both carry a caller-supplied id, so a single host callback
//  can serve every prompt a form raises.
//
//  Like the context menu, the script's own layoutRoot is a 1px transparent stub
//  that exists only to keep Update() ticking; the visible panel is a separate
//  widget parented to the anchor passed to InitPrompt(), so it floats above the
//  host instead of being clipped by it. Anchor it to the window root.
//
//  It is MODAL: while it is up the game's inputs are held down and a click
//  outside it does nothing. That is the difference between this and the popup
//  on a colour picker - a click landing elsewhere on a form that is halfway
//  through answering a question is a misclick, not a decision, and dismissing
//  a prompt on one loses whatever was dialled in.
//
//  Usage:
//      m_Prompt = UIActionManager.CreateValuePrompt( layoutRoot, m_Window.GetWidgetRoot(), this, "OnConfirm_Prompt" );
//
//      m_Prompt.ShowSlider( "quantity", "Set Quantity", "Rounds", 0, 30, 12, 1, "%1" );
//
//      void OnConfirm_Prompt( UIEvent eid, UIActionBase action )
//      {
//          if ( eid != UIEvent.CLICK )
//              return;
//          if ( m_Prompt.GetPromptId() == "quantity" )
//              Apply( m_Prompt.GetSliderValue() );
//      }
// =============================================================================

class UIActionValuePrompt: UIActionBase
{
	protected Widget           m_Anchor;
	protected Widget           m_Panel;
	protected GridSpacerWidget m_Grid;

	protected UIActionText     m_TitleAction;
	protected UIActionSlider   m_Slider;
	protected UIActionDropdown m_Dropdown;
	protected UIActionButton   m_Cancel;
	protected UIActionButton   m_Confirm;

	//! Dropdown row -> the value the caller actually cares about. The rows are
	//! whatever list the caller passed, and those are rarely 0..n-1: a liquid
	//! type is a bit flag and a food stage starts at 1.
	protected ref array<int> m_OptionValues;

	protected string m_PromptId;
	protected bool   m_Open;
	protected bool   m_IsSlider;

	//! Measured off the content grid once it has been laid out; 0 until then.
	protected float m_FullHeight;

	static const float PANEL_WIDTH  = 340;
	static const float PANEL_PAD    = 12;

	override void OnInit()
	{
		super.OnInit();

		m_OptionValues = new array<int>;
		m_PromptId     = "";
		m_Open         = false;
		m_IsSlider     = true;
		m_FullHeight   = 0;
	}

	void ~UIActionValuePrompt()
	{
		if ( m_Open )
			CommunityOnlineToolsBase.ForceDisableInputs( false );

		if ( m_Panel )
		{
			JMStatics.UnregisterOverlay( m_Panel );
			m_Panel.Unlink();
		}
	}

	//! Build the panel under `anchor`. Must be called before either Show.
	void InitPrompt( notnull Widget anchor )
	{
		m_Anchor = anchor;

		m_Panel = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionValuePrompt_Panel.layout", m_Anchor );
		if ( !m_Panel )
			return;

		m_Panel.Show( false );
		m_Panel.SetHandler( this );

		// The panel sits outside the window, so COT's "was that click on our
		// UI?" ancestor walk cannot find a window above it. Without this the
		// click reads as a world click and the game takes the mouse back -
		// which turns the camera in the middle of a slider drag.
		JMStatics.RegisterOverlay( m_Panel );

		Class.CastTo( m_Grid, m_Panel.FindAnyWidget( "prompt_grid" ) );
		if ( !m_Grid )
			return;

		m_TitleAction = UIActionManager.CreateText( m_Grid, "" );

		// Both inputs are built once and the unused one is hidden: spacers skip
		// hidden children, so the row it would have taken collapses and the
		// panel measures the same as if it had never been created.
		m_Slider = UIActionManager.CreateSlider( m_Grid, "", 0, 1, this, "OnChange_Slider" );
		m_Slider.SetSliderWidth( 0.55 );

		m_Dropdown = UIActionManager.CreateDropdown( m_Grid, "", m_Anchor, this, "" );

		Widget buttons = UIActionManager.CreateGridSpacer( m_Grid, 1, 2 );
		m_Cancel  = UIActionManager.CreateButton( buttons, "#STR_COT_GENERIC_CANCEL",  this, "OnClick_Cancel"  );
		m_Confirm = UIActionManager.CreateButton( buttons, "#STR_COT_GENERIC_CONFIRM", this, "OnClick_Confirm" );
	}

	bool IsOpen()
	{
		return m_Open;
	}

	string GetPromptId()
	{
		return m_PromptId;
	}

	//! Slider prompts: the number the admin left the handle on. Not GetValue -
	//! UIActionBase already has one of those, and it answers a vector.
	float GetSliderValue()
	{
		if ( !m_Slider )
			return 0;

		return m_Slider.GetCurrent();
	}

	//! Option prompts: the caller's own value for the selected row, not the row
	//! index. Returns 0 when nothing is selected.
	int GetSelectedValue()
	{
		if ( !m_Dropdown || !m_OptionValues )
			return 0;

		int row = m_Dropdown.GetSelection();
		if ( row < 0 || row >= m_OptionValues.Count() )
			return 0;

		return m_OptionValues[row];
	}

	//! Ask for a number.
	void ShowSlider( string id, string title, string label, float min, float max, float current, float step = 1, string format = "%1" )
	{
		if ( !m_Panel || !m_Slider )
			return;

		m_PromptId = id;
		m_IsSlider = true;

		m_TitleAction.SetLabel( title );

		m_Slider.SetLabel( label );
		m_Slider.SetMinMax( min, max );
		m_Slider.SetStepValue( step );
		m_Slider.SetFormat( format );
		m_Slider.SetCurrent( Math.Clamp( current, min, max ) );

		m_Slider.GetLayoutRoot().Show( true );
		m_Dropdown.GetLayoutRoot().Show( false );
		m_Dropdown.Close();

		Open();
	}

	//! Ask for one of a fixed list. `labels` and `values` are parallel; the row
	//! whose value matches `current` is preselected, falling back to the first.
	void ShowOptions( string id, string title, string label, notnull array<string> labels, notnull array<int> values, int current )
	{
		if ( !m_Panel || !m_Dropdown )
			return;

		if ( labels.Count() == 0 || labels.Count() != values.Count() )
			return;

		m_PromptId = id;
		m_IsSlider = false;

		m_TitleAction.SetLabel( title );

		m_OptionValues.Clear();
		m_Dropdown.ClearEntries();
		m_Dropdown.SetLabel( label );

		int selected = 0;
		int i;

		for ( i = 0; i < labels.Count(); i++ )
		{
			m_Dropdown.AddEntry( labels[i] );
			m_OptionValues.Insert( values[i] );

			if ( values[i] == current )
				selected = i;
		}

		m_Dropdown.SetSelection( selected, false );

		m_Dropdown.GetLayoutRoot().Show( true );
		m_Slider.GetLayoutRoot().Show( false );

		Open();
	}

	override void Close()
	{
		if ( !m_Open )
			return;

		m_Open = false;

		if ( m_Dropdown )
			m_Dropdown.Close();

		if ( m_Panel )
			m_Panel.Show( false );

		CommunityOnlineToolsBase.ForceDisableInputs( false );
	}

	//! UIActionBase.Hide() tears down the update loop this control needs to keep
	//! itself placed; closing the prompt is what a caller means by "hide".
	override void Hide()
	{
		Close();
	}

	override void OnHide()
	{
		Close();
	}

	protected void Open()
	{
		m_Open       = true;
		m_FullHeight = 0;

		m_Panel.Show( true );
		m_Panel.SetSort( 9999, true );

		// Nothing inside takes keyboard focus, so UIActionBase's focus-driven
		// input guard never trips on its own and a drag would reach the world.
		CommunityOnlineToolsBase.ForceDisableInputs( true );

		UpdatePlacement();
	}

	//! Centred on the anchor, sized to whatever the content grid measured. The
	//! height is only known a frame after the panel is shown, which is why this
	//! runs from Update rather than once at open.
	protected void UpdatePlacement()
	{
		if ( !m_Panel || !m_Grid || !m_Anchor )
			return;

		float aw, ah;
		m_Anchor.GetScreenSize( aw, ah );

		float gw, gh;
		m_Grid.GetScreenSize( gw, gh );
		if ( gh >= 1 )
			m_FullHeight = gh + ( PANEL_PAD * 2 );

		if ( m_FullHeight <= 0 )
			return;

		m_Panel.SetFlags( WidgetFlags.HEXACTSIZE );
		m_Panel.SetFlags( WidgetFlags.VEXACTSIZE );
		m_Panel.SetSize( PANEL_WIDTH, m_FullHeight );
		m_Panel.SetPos( ( aw - PANEL_WIDTH ) * 0.5, ( ah - m_FullHeight ) * 0.5 );
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		if ( !m_Open )
			return;

		UpdatePlacement();
	}

	override bool OnKeyPress( Widget w, int x, int y, int key )
	{
		if ( m_Open && key == KeyCode.KC_ESCAPE )
		{
			Close();
			return true;
		}

		return super.OnKeyPress( w, x, y, key );
	}

	void OnChange_Slider( UIEvent eid, UIActionBase action )
	{
		// The value is read on confirm, so a drag needs no handler of its own -
		// but the slider must have a callback target or it never fires CHANGE
		// and stops repainting mid-drag.
	}

	void OnClick_Cancel( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		Close();
	}

	void OnClick_Confirm( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		// Closed BEFORE the callback: the host usually answers by opening a
		// confirmation or rebuilding the panel this prompt is drawn over, and
		// a prompt still up over that reads as one that failed to take.
		Close();

		CallEvent( UIEvent.CLICK );
	}

	override bool IsFocusWidget( Widget widget )
	{
		if ( !widget )
			return false;

		if ( widget == m_Panel )
			return true;

		Widget cur = widget;
		while ( cur )
		{
			if ( cur == m_Panel )
				return true;

			cur = cur.GetParent();
		}

		// The dropdown's open list is parented to the anchor, not to the panel,
		// so it is outside the walk above and has to answer for itself.
		if ( m_Dropdown && m_Dropdown.IsFocusWidget( widget ) )
			return true;

		return false;
	}
}
