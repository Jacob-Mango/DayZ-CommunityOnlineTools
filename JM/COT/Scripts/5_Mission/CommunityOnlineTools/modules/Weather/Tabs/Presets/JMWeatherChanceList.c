//! An editable list of weighted targets that always adds up to 100 - one slider
//! per target, a delete button beside each, and a dropdown that adds another the
//! moment it is picked.
//!
//! The dynamic weather editor has three of these (a phase's edges, a state's next
//! states, a state's entry phases), and each is an instance of this class: an
//! instance is its own callback target, so the three never see each other's
//! events. It is the chance-row code that used to be hard-wired into the Presets
//! tab, lifted out so it can be used more than once.
//!
//! A target name of "" is allowed and shown under a label of its own - that is
//! how a phase's "leave this state" edge appears in the same list as the phases.
class JMWeatherChanceList
{
	//! The chance row reserves a tenth of its width for the 28px delete button beside
	//! the slider - a fraction, not a measured split, so it holds at any scale.
	static const float CHANCE_SLIDER_W = 0.9;

	protected JMWeatherForm m_Form;
	protected UIActionScroller m_Scroller;

	//! Adding is a pick, not a pick-then-click: choosing a name in this dropdown adds
	//! it and puts the dropdown back on its prompt. It used to be a dropdown with a
	//! separate Add button, which had to be sized and lined up beside it and never
	//! quite was; with one control there is nothing to line up.
	protected UIActionDropdown m_DropAdd;

	//! The chance rows are per selection and change at runtime, so they are torn
	//! down and rebuilt rather than pre-allocated - a hidden child still owns its
	//! cell in a GridSpacer. m_Host stays put so the rows keep their place in the
	//! card; m_Grid is the part that gets replaced.
	protected Widget m_Host;
	protected Widget m_Grid;

	//! The rows are held as data, not read back off the widgets: they are
	//! destroyed and rebuilt whenever a row is added or removed, and a rebalance
	//! has to survive that.
	protected autoptr TStringArray m_Targets;
	protected autoptr array<int>   m_Values;
	protected autoptr array< ref UIActionSlider >      m_Sliders;
	protected autoptr array< ref UIActionImageButton > m_Deletes;

	//! What may be added: raw names and the text shown for each. Row 0 of the add
	//! dropdown is the prompt, so option n is candidate n - 1.
	protected autoptr TStringArray m_CandidateNames;
	protected autoptr TStringArray m_CandidateLabels;
	protected autoptr TStringArray m_Options;

	protected string m_AddHint;

	//! Whether the add dropdown is on screen. It starts out so.
	protected bool m_AddShown = true;

	//! Set by a change the person made, cleared by Load() and Clear(). The editor
	//! reads it so that refreshing the settings never discards an unsaved edit.
	protected bool m_Changed;

	void JMWeatherChanceList( JMWeatherForm form, UIActionScroller scroller, Widget parent, string addLabel )
	{
		m_Form     = form;
		m_Scroller = scroller;
		m_AddHint  = "#STR_COT_WEATHER_DYN_CHOOSE";

		m_Targets = new TStringArray;
		m_Values  = new array<int>;
		m_Sliders = new array< ref UIActionSlider >;
		m_Deletes = new array< ref UIActionImageButton >;

		m_CandidateNames  = new TStringArray;
		m_CandidateLabels = new TStringArray;
		m_Options         = new TStringArray;

		m_DropAdd = UIActionManager.CreateDropdown( parent, addLabel, m_Form.GetLayoutRoot(), this, "OnChange_Add" );
		m_Form.AddOverlay( m_DropAdd );

		m_Host = UIActionManager.CreateGridSpacer( parent, 1, 1 );
	}

	bool IsChanged()
	{
		return m_Changed;
	}

	int Count()
	{
		return m_Targets.Count();
	}

	//! What is on screen is now what is stored: the editor calls this after a save so
	//! the list stops counting as an unsaved edit.
	void ClearChanged()
	{
		m_Changed = false;
	}

	//! Something is left to pick from. With nothing left the add dropdown is hidden
	//! outright - a disabled control that can never be enabled is only clutter.
	bool CanAdd()
	{
		return m_Options.Count() > 0;
	}

	//! Rows to show or something to add. A list with neither has nothing to say, and
	//! the editor hides the whole card around it.
	bool HasContent()
	{
		return m_Targets.Count() > 0 || m_Options.Count() > 0;
	}

	//! What can be added. `names` and `labels` run in step; a name already in the
	//! list is left out of the dropdown.
	void SetCandidates( TStringArray names, TStringArray labels )
	{
		m_CandidateNames.Clear();
		m_CandidateLabels.Clear();

		for ( int i = 0; i < names.Count(); i++ )
		{
			m_CandidateNames.Insert( names[i] );
			m_CandidateLabels.Insert( labels[i] );
		}

		RebuildOptions();
		Refresh();
	}

	//! Take the model from a stored list. A target that is not a candidate has
	//! nothing to be picked from and is dropped, so a stale name never shows a row.
	void Load( array< ref JMWeatherNextState > edges )
	{
		m_Targets.Clear();
		m_Values.Clear();
		m_Changed = false;

		if ( edges )
		{
			for ( int i = 0; i < edges.Count(); i++ )
			{
				if ( m_CandidateNames.Find( edges[i].To ) == -1 )
					continue;

				m_Targets.Insert( edges[i].To );
				m_Values.Insert( edges[i].Chance );
			}
		}

		Rebuild();
	}

	//! Write the model out as a fresh list. A zero chance is the absence of a
	//! candidate; storing it would only grow the file and slow every roll down.
	array< ref JMWeatherNextState > Read()
	{
		array< ref JMWeatherNextState > edges = new array< ref JMWeatherNextState >;

		JMWeatherNextState entry;

		for ( int i = 0; i < m_Targets.Count(); i++ )
		{
			if ( m_Values[i] <= 0 )
				continue;

			entry = new JMWeatherNextState;
			entry.To     = m_Targets[i];
			entry.Chance = m_Values[i];

			edges.Insert( entry );
		}

		return edges;
	}

	void Clear()
	{
		m_Targets.Clear();
		m_Values.Clear();
		m_Changed = false;

		Rebuild();
	}

	//! Permission and enable state for every control in the list.
	void UpdateStates( string permission, bool forceDisabled )
	{
		//! Only on a change: showing a widget that is already showing is not free.
		if ( CanAdd() != m_AddShown )
		{
			m_AddShown = CanAdd();
			m_DropAdd.SetVisible( m_AddShown );
		}

		m_Form.UpdateActionState( m_DropAdd, permission, forceDisabled );

		int d;

		for ( d = 0; d < m_Deletes.Count(); d++ )
			m_Form.UpdateActionState( m_Deletes[d], permission, forceDisabled );

		for ( d = 0; d < m_Sliders.Count(); d++ )
			m_Form.UpdateActionState( m_Sliders[d], permission, forceDisabled );
	}

	protected string LabelFor( string name )
	{
		int index = m_CandidateNames.Find( name );

		if ( index >= 0 )
			return m_CandidateLabels[index];

		return name;
	}

	//! One slider plus a delete button per row.
	protected void Rebuild()
	{
		if ( !m_Host )
			return;

		if ( m_Grid )
		{
			m_Grid.Unlink();
			m_Grid = NULL;
		}

		m_Sliders.Clear();
		m_Deletes.Clear();

		int count = m_Targets.Count();

		if ( count > 0 )
		{
			m_Grid = UIActionManager.CreateGridSpacer( m_Host, count, 1 );

			UIActionSlider slider;
			UIActionImageButton remove;
			Widget row;

			for ( int i = 0; i < count; i++ )
			{
				//! Compact: margin and padding are both zero, so the children add up
				//! to exactly their own widths and nothing has to be held back for
				//! spacing between them.
				row = UIActionManager.CreateWrapSpacerCompact( m_Grid, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

				//! Delete first, so it sits ahead of the name it removes - a wrap
				//! spacer places children in creation order.
				remove = UIActionManager.CreateDeleteButton( row, this, "" );
				if ( remove ) remove.SetOnClick( this, "OnClick_Remove" );

				slider = UIActionManager.CreateSlider( row, LabelFor( m_Targets[i] ), 0, 100, this, "OnChange_Chance" );
				slider.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
				slider.SetStepValue( 1 );
				slider.SetCurrent( m_Values[i] );

				//! See the note on the add row: a fraction that leaves room for the
				//! fixed button beside it, rather than a measured split.
				slider.SetWidth( CHANCE_SLIDER_W );

				m_Sliders.Insert( slider );
				m_Deletes.Insert( remove );
			}
		}

		RebuildOptions();
		Refresh();
	}

	//! The add dropdown only offers candidates that are not already listed.
	protected void RebuildOptions()
	{
		m_Options.Clear();

		array<string> items = new array<string>;
		items.Insert( Widget.TranslateString( m_AddHint ) );

		for ( int i = 0; i < m_CandidateNames.Count(); i++ )
		{
			if ( m_Targets.Find( m_CandidateNames[i] ) != -1 )
				continue;

			m_Options.Insert( m_CandidateNames[i] );
			items.Insert( m_CandidateLabels[i] );
		}

		if ( m_DropAdd )
		{
			m_DropAdd.SetItems( items );
			m_DropAdd.SetSelection( 0, false );
		}
	}

	protected void Refresh()
	{
		m_Form.UpdateStates();

		if ( m_Scroller )
			m_Scroller.UpdateScroller();
	}

	//! Repaint the sliders from the model after a rebalance. SetCurrent does not
	//! raise CHANGE, so this cannot re-enter the handler that called it.
	protected void Paint()
	{
		for ( int i = 0; i < m_Sliders.Count(); i++ )
			m_Sliders[i].SetCurrent( m_Values[i] );
	}

	//! The chances always add up to 100, so what is on screen is the real odds
	//! rather than a weight that only means something next to a total the admin has
	//! to add up themselves.
	//!
	//! The row that was just moved keeps its value and every other row is scaled
	//! into what is left. Rounding drift is pushed onto the last scaled row, so the
	//! column reads exactly 100 however the shares divide.
	protected void Balance( int keep )
	{
		int count = m_Values.Count();

		if ( count == 0 )
			return;

		if ( count == 1 )
		{
			m_Values[0] = 100;
			return;
		}

		if ( keep < 0 || keep >= count )
		{
			Normalize();
			return;
		}

		int i;
		int share;

		int kept = Math.Clamp( m_Values[keep], 0, 100 );
		m_Values[keep] = kept;

		int remainder   = 100 - kept;
		int othersTotal = 0;

		for ( i = 0; i < count; i++ )
		{
			if ( i != keep )
				othersTotal += m_Values[i];
		}

		int running = 0;
		int last    = -1;

		for ( i = 0; i < count; i++ )
		{
			if ( i == keep )
				continue;

			//! Multiplied out in floats first - an int division would round every
			//! share down and leave the column short of 100.
			if ( othersTotal > 0 )
				share = Math.Round( ( m_Values[i] * remainder * 1.0 ) / othersTotal );
			else
				share = Math.Round( ( remainder * 1.0 ) / ( count - 1 ) );

			m_Values[i] = share;
			running += share;
			last = i;
		}

		if ( last != -1 )
			m_Values[last] = Math.Clamp( m_Values[last] + ( remainder - running ), 0, 100 );
	}

	//! Scale every row to add up to 100, or split evenly when they are all at zero
	//! - which is what a freshly added row looks like.
	protected void Normalize()
	{
		int count = m_Values.Count();

		if ( count == 0 )
			return;

		int i;
		int share;
		int total = 0;

		for ( i = 0; i < count; i++ )
			total += m_Values[i];

		int running = 0;

		for ( i = 0; i < count; i++ )
		{
			if ( total > 0 )
				share = Math.Round( ( m_Values[i] * 100.0 ) / total );
			else
				share = Math.Round( 100.0 / count );

			m_Values[i] = share;
			running += share;
		}

		m_Values[count - 1] = Math.Clamp( m_Values[count - 1] + ( 100 - running ), 0, 100 );
	}

	void OnChange_Chance( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UIActionSlider moved = UIActionSlider.Cast( action );

		if ( !moved )
			return;

		int index = m_Sliders.Find( moved );

		if ( index < 0 )
			return;

		m_Values[index] = Math.Round( moved.GetCurrent() );
		m_Changed = true;

		Balance( index );
		Paint();
	}

	void OnChange_Add( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || !m_DropAdd )
			return;

		//! Row 0 is the prompt, so the option list is offset by one.
		int index = m_DropAdd.GetSelection() - 1;

		if ( index < 0 || index >= m_Options.Count() )
			return;

		m_Targets.Insert( m_Options[index] );
		m_Values.Insert( 0 );

		//! A new row comes in at zero, so the whole column is renormalised rather
		//! than balanced around it - otherwise it would keep the zero and never be
		//! worth adding.
		Normalize();
		Rebuild();

		//! Rebuild() is not a Load, so the flag it leaves alone is set here.
		m_Changed = true;
	}

	void OnClick_Remove( UIActionBase action )
	{
		UIActionImageButton clicked = UIActionImageButton.Cast( action );

		if ( !clicked )
			return;

		int index = m_Deletes.Find( clicked );

		if ( index < 0 )
			return;

		m_Targets.Remove( index );
		m_Values.Remove( index );

		Normalize();
		Rebuild();

		m_Changed = true;
	}
}
