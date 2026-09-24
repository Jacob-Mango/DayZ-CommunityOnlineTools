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

	//! Optional per-value colouring for the slider fill, e.g. health or
	//! temperature - matches the object spawner's own coloured sliders instead
	//! of leaving every prompt the same flat fill regardless of what the number
	//! means. NONE leaves the slider's default colour alone.
	protected int m_SliderColorMode;
	static const int SLIDER_COLOR_NONE        = 0;
	static const int SLIDER_COLOR_HEALTH      = 1;
	static const int SLIDER_COLOR_TEMPERATURE = 2;

	//! Measured off the content grid once it has been laid out; 0 until then.
	protected float m_FullHeight;

	//! 340 put the slider's value readout and the right edge of the Confirm
	//! button hard against the panel's own edge - a long item name on the slider
	//! label pushed both past it. The row this has to fit is label + track +
	//! value, and two buttons side by side under it.
	static const float PANEL_WIDTH  = 420;
	static const float PANEL_PAD    = 12;

	void ~UIActionValuePrompt()
	{
		if (!g_Game)
			return;

		if ( m_Open )
			COT.ForceDisableInputs( false );

		if ( m_Panel && m_Panel.ToString() != "INVALID" )
		{
			JMStatics.RemoveOverlay( m_Panel );
			m_Panel.Unlink();
		}
	}

	string GetPromptId()
	{
		return m_PromptId;
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

	//! Slider prompts: the number the admin left the handle on. Not GetValue -
	//! UIActionBase already has one of those, and it answers a vector.
	float GetSliderValue()
	{
		if ( !m_Slider )
			return 0;

		return m_Slider.GetCurrent();
	}

	override bool IsOpen()
	{
		return m_Open;
	}

	override void OnInit()
	{
		super.OnInit();

		m_OptionValues    = new array<int>;
		m_PromptId        = "";
		m_Open            = false;
		m_IsSlider        = true;
		m_FullHeight      = 0;
		m_SliderColorMode = SLIDER_COLOR_NONE;
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
		JMStatics.AddOverlay( m_Panel );

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
		m_Confirm = UIActionManager.CreateButton( buttons, "#STR_COT_GENERIC_CONFIRM", this, "" );
		if ( m_Confirm ) m_Confirm.SetOnClick( this, "OnClick_Confirm" );
	}

	//! layoutRoot is a 1px stub here too (see class note) - m_Open is the real
	//! open/closed state the Escape priority chain needs.
	override bool IsVisible()
	{
		return m_Open;
	}

	//! Ask for a number. `colorMode` is one of the SLIDER_COLOR_* constants -
	//! leave it NONE for a plain slider.
	//! DEPRECATED - use OpenSlider
	void ShowSlider( string id, string title, string label, float min, float max, float current, float step = 1, string format = "%1", int colorMode = 0 )
	{
		JMDeprecated.WarnOnce( this, "UIActionValuePrompt.ShowSlider() is deprecated. Please use OpenSlider()." );

		OpenSlider( id, title, label, min, max, current, step, format, colorMode );
	}

	//! DEPRECATED - use OpenOptions
	void ShowOptions( string id, string title, string label, notnull array<string> labels, notnull array<int> values, int current )
	{
		JMDeprecated.WarnOnce( this, "UIActionValuePrompt.ShowOptions() is deprecated. Please use OpenOptions()." );

		OpenOptions( id, title, label, labels, values, current );
	}

	void OpenSlider( string id, string title, string label, float min, float max, float current, float step = 1, string format = "%1", int colorMode = 0 )
	{
		if ( !m_Panel || !m_Slider )
			return;

		m_PromptId = id;
		m_IsSlider = true;
		m_SliderColorMode = colorMode;

		m_TitleAction.SetLabel( title );

		m_Slider.SetLabel( label );
		m_Slider.SetMinMax( min, max );
		m_Slider.SetStepValue( step );
		m_Slider.SetFormat( format );
		m_Slider.SetCurrent( Math.Clamp( current, min, max ) );

		m_Slider.GetLayoutRoot().Show( true );
		m_Dropdown.GetLayoutRoot().Show( false );
		m_Dropdown.Close();

		UpdateSliderColor();

		Open();
	}

	//! Repaints the slider fill for the current handle position. Called on
	//! open and on every drag - a health slider left green at 5 health would
	//! tell the admin nothing they could not already read off the number.
	protected void UpdateSliderColor()
	{
		if ( !m_Slider || m_SliderColorMode == SLIDER_COLOR_NONE )
			return;

		float value = m_Slider.GetCurrent();

		if ( m_SliderColorMode == SLIDER_COLOR_HEALTH )
		{
			float max = m_Slider.GetMax();
			float percent = 100;
			if ( max > 0 )
				percent = ( value / max ) * 100;

			m_Slider.SetColor( HealthSliderColor( percent ) | 0xFF000000 );
			return;
		}

		if ( m_SliderColorMode == SLIDER_COLOR_TEMPERATURE )
		{
			int intValue = value;

			m_Slider.SetColor( ObjectTemperatureState.GetStateData( intValue ).m_Color | 0xFF000000 );

			if ( ObjectTemperatureState.GetStateData( intValue ).m_State != GameConstants.STATE_NEUTRAL_TEMP )
				m_Slider.SetFormat( "#STR_COT_FORMAT_DEGREE " + ObjectTemperatureState.GetStateData( intValue ).m_LocalizedName );
			else
				m_Slider.SetFormat( "#STR_COT_FORMAT_DEGREE" );
		}
	}

	//! Same bands vanilla's own health colouring uses - see
	//! ItemManager.GetItemHealthColor - so a 0..100 admin slider reads the same
	//! as the condition dot next to it.
	protected int HealthSliderColor( float health )
	{
		if ( health > 70 )
			return Colors.COLOR_PRISTINE;

		if ( health > 50 )
			return Colors.COLOR_WORN;

		if ( health > 30 )
			return Colors.COLOR_DAMAGED;

		if ( health > 0 )
			return Colors.COLOR_BADLY_DAMAGED;

		return Colors.COLOR_RUINED;
	}

	//! Ask for one of a fixed list. `labels` and `values` are parallel; the row
	//! whose value matches `current` is preselected, falling back to the first.
	void OpenOptions( string id, string title, string label, notnull array<string> labels, notnull array<int> values, int current )
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

		COT.ForceDisableInputs( false );
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
		COT.ForceDisableInputs( true );

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

		// Never wider than what there is room to draw. Past the anchor's edge
		// the panel is clipped, and what gets cut is the right-hand end of the
		// row - the Confirm button and the slider's value readout.
		float width = PANEL_WIDTH;
		if ( aw >= 1 && width > aw )
			width = aw;

		m_Panel.SetFlags( WidgetFlags.HEXACTSIZE );
		m_Panel.SetFlags( WidgetFlags.VEXACTSIZE );
		m_Panel.SetSize( width, m_FullHeight );

		// The layout hardcodes the grid's width, so a panel that is any size
		// other than the layout's own leaves the content sized for the old one.
		// Drive it from the panel instead and the two can never disagree.
		//
		// Only once the grid has a real measured height: passing the 0 it
		// answers before it is laid out would collapse it.
		if ( gh >= 1 )
		{
			m_Grid.SetFlags( WidgetFlags.HEXACTSIZE );
			m_Grid.SetSize( width - ( PANEL_PAD * 2 ), gh );
		}

		// Clamped, not just centred: a negative origin puts the left edge of the
		// panel outside the anchor, and that end is clipped just as hard.
		float px = ( aw - width ) * 0.5;
		float py = ( ah - m_FullHeight ) * 0.5;

		if ( px < 0 )
			px = 0;
		if ( py < 0 )
			py = 0;

		m_Panel.SetPos( px, py );
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
		// The value itself is read on confirm; the callback still has to exist
		// or the slider never fires CHANGE and stops repainting mid-drag. The
		// colour, if this prompt has one, has to keep up with the drag though.
		UpdateSliderColor();
	}

	void OnClick_Cancel( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		Close();
	}

	void OnClick_Confirm( UIActionBase action )
	{
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
