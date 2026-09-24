//! The preset editor: pick, add, edit, apply and delete presets, and shape the
//! phases and odds that move the dynamic weather between them.
//!
//! A PRESET here is what used to be a separate preset AND a state: one named thing
//! made of phases, where each phase carries the weather it puts on. Picking one in
//! the selector shows its structure; the Edit button additionally puts the
//! Sky / Precipitation / Wind tabs into preset edit mode, where they show - and
//! edit - the weather of the phase selected here, every value a min/max span.
//!
//! What is edited is a WORKING COPY (m_Working). Nothing reaches the server until
//! Save, which sends the whole preset as one payload - the server validates it and
//! keeps its own copy, so a half-finished edit can never leave the machine with a
//! half-applied one. The controls read from and write to the working copy through
//! CommitState() / CommitPhase() / the Load* methods, which are the only places
//! the two are synchronised, so the rule for every handler is "commit before you
//! switch what the controls are showing".
//!
//! Every action is an icon in a card's title bar, and a name is changed through a
//! small popup behind the pencil beside its selector - a name is set once and read
//! many times, so it does not get a permanent text box.
//!
//! Cards, top to bottom - each below the first appears once a preset is picked:
//!   Presets       selector + rename; title bar: Add / Edit / Apply Now / Delete / Save
//!                 (Save glows while there is something to save)
//!   Next Presets  where the machine goes when this preset ends
//!   Phase         selector + rename, hold, fade; title bar: Add / Enter Now / Remove
//!   Opens On      which phase the preset starts with (only with more than one)
//!   After Phase   where the machine goes when this phase ends (incl. "leave")
//! A list card with nothing to show and nothing left to add is hidden.
class JMWeatherDynamicEditor
{
	//! Fixed row at the head of the preset selector; stored presets follow it.
	static const int ROW_NONE  = 0;
	static const int ROW_FIRST = 1;

	//! Tops of the two timing ranges. A hold is edited in minutes and a fade in
	//! seconds, which is how each reads best.
	static const int DURATION_MAX_MINUTES   = 120;
	static const int TRANSITION_MAX_SECONDS = 600;

	//! A selector shares its row with the pencil that renames it: a fraction of the
	//! row for the selector, the rest for the 28px button - a fraction, not a
	//! measured split, so it holds at any scale (same reasoning as the chance rows).
	static const float PICK_WIDTH = 0.9;
	static const int   ICON_PX    = 28;

	//! The Save icon repeats its glow this often while there is something to save.
	//! The glow itself lasts SAVE_PULSE_SECONDS, so the gap keeps it continuous
	//! without ever restarting one that has not finished.
	static const float SAVE_PULSE_SECONDS  = 2.0;
	static const float SAVE_PULSE_INTERVAL = 2.1;

	protected JMWeatherForm m_Form;
	protected UIActionScroller m_Scroller;

	// --- Preset --------------------------------------------------------------
	protected ref UIActionCard m_PresetCard;
	protected UIActionDropdown     m_SelectPreset;
	protected UIActionImageButton  m_ButtonRenamePreset;
	protected UIActionToggleSwitch m_ToggleRotation;
	protected UIActionImageButton  m_ButtonAdd;
	protected UIActionImageButton  m_ButtonApply;
	protected UIActionImageButton  m_ButtonDelete;
	protected UIActionImageButton  m_ButtonSave;

	protected ref UIActionCard m_NextCard;
	protected ref JMWeatherChanceList m_ListNext;

	// --- Phase ---------------------------------------------------------------
	protected ref UIActionCard m_PhaseCard;
	protected UIActionDropdown    m_SelectPhase;
	protected UIActionImageButton m_ButtonRenamePhase;
	protected UIActionSliderRange m_RangeDuration;
	protected UIActionSliderRange m_RangeTransition;
	protected UIActionImageButton m_ButtonPhaseAdd;
	protected UIActionImageButton m_ButtonJump;
	protected UIActionImageButton m_ButtonPhaseRemove;

	protected ref UIActionCard m_EntryCard;
	protected ref JMWeatherChanceList m_ListEntry;

	protected ref UIActionCard m_EdgesCard;
	protected ref JMWeatherChanceList m_ListEdges;

	// --- Model ---------------------------------------------------------------
	//! The preset being edited, or NULL with nothing selected.
	protected ref JMWeatherState m_Working;

	//! The name the server knows the working preset by. "" for one not saved yet.
	protected string m_TargetName;

	//! A preset just sent to the server under this name and not yet seen coming
	//! back. Until it does, a refresh must not mistake it for a deleted one.
	protected string m_PendingName;

	//! The phase whose controls are on screen, by its name in m_Working.
	protected string m_PhaseName;

	//! Something other than a chance list or a weather control was changed since
	//! the last load or save (a rename, an added or removed phase, a switch).
	protected bool m_Dirty;

	//! The weather tabs are editing the selected phase's weather - see the header.
	protected bool m_EditMode;

	//! Which cards are on screen, as a bit per card, so the scroller is only
	//! re-measured when one actually appears or goes. -1 before the first pass.
	protected int m_ShownMask = -1;

	//! Save-icon glow bookkeeping.
	protected bool  m_SaveGlow;
	protected float m_SavePulseAt;

	void JMWeatherDynamicEditor( JMWeatherForm form, UIActionScroller scroller, Widget content )
	{
		m_Form     = form;
		m_Scroller = scroller;

		BuildPresetCards( content );
		BuildPhaseCards( content );

		RebuildPresetList();
		SelectRow( ROW_NONE );
	}

	//! The pencil BuildPickRow just made - read straight after the call.
	protected UIActionImageButton m_PickRename;

	//! A selector with the rename pencil to its right, on one row of `parent`.
	protected UIActionDropdown BuildPickRow( Widget parent, string label, string callback, string renameCallback, string renameTooltip )
	{
		Widget row = UIActionManager.CreateWrapSpacerCompact( parent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		UIActionDropdown drop = UIActionManager.CreateDropdown( row, label, m_Form.GetLayoutRoot(), this, callback );
		drop.SetWidth( PICK_WIDTH );
		m_Form.AddOverlay( drop );

		m_PickRename = UIActionManager.CreateIconButton( row, JMConstants.Lucide( "pencil-line" ), this, renameCallback );

		if ( m_PickRename )
		{
			m_PickRename.SetFixedSize( ICON_PX, ICON_PX );
			m_PickRename.SetTooltip( renameTooltip );
		}

		return drop;
	}

	protected void BuildPresetCards( Widget content )
	{
		m_PresetCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_DYN_PRESET_CARD" );

		//! Creation order is left to right in the title bar.
		m_ButtonAdd    = m_PresetCard.AddAddButton( this, "OnClick_Add", "#STR_COT_WEATHER_DYN_BTN_ADD_DESC" );
		m_ButtonApply  = m_PresetCard.AddCardHeaderAction( JMConstants.Lucide( "play" ), this, "OnClick_Apply", "#STR_COT_WEATHER_DYN_BTN_APPLY_DESC" );
		m_ButtonDelete = m_PresetCard.AddDeleteButton( this, "OnClick_Delete", "#STR_COT_WEATHER_DYN_BTN_DELETE" );
		m_ButtonSave   = m_PresetCard.AddSaveButton( this, "OnClick_Save", "#STR_COT_WEATHER_DYN_SAVE_DESC" );

		//! Straight into the card, not into a grid: the rotation switch is hidden with no
		//! preset selected, and a hidden child would keep its cell in a grid.
		Widget presetBody = m_PresetCard.GetContent();

		m_SelectPreset = BuildPickRow( presetBody, "#STR_COT_WEATHER_DYN_PRESET", "OnChange_PresetSelect", "OnClick_RenamePreset", "#STR_COT_WEATHER_DYN_RENAME_PRESET_DESC" );
		m_ButtonRenamePreset = m_PickRename;
		m_SelectPreset.SetTooltip( "#STR_COT_WEATHER_DYN_PRESET_DESC" );

		m_ToggleRotation = UIActionManager.CreateToggleSwitch( presetBody, "#STR_COT_WEATHER_DYNAMIC_ROTATION", this, "OnChange_Dirty" );
		m_ToggleRotation.SetTooltip( "#STR_COT_WEATHER_DYN_ROTATION_DESC" );

		m_NextCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_DYN_NEXT_PRESETS" );
		m_ListNext = new JMWeatherChanceList( m_Form, m_Scroller, m_NextCard.GetContent(), "#STR_COT_WEATHER_DYN_NEXT_ADD" );
	}

	protected void BuildPhaseCards( Widget content )
	{
		m_PhaseCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_DYN_PHASE_CARD" );

		m_ButtonPhaseAdd    = m_PhaseCard.AddAddButton( this, "OnClick_PhaseAdd", "#STR_COT_WEATHER_DYN_PHASE_ADD" );
		m_ButtonJump        = m_PhaseCard.AddCardHeaderAction( JMConstants.Lucide( "zap" ), this, "OnClick_Jump", "#STR_COT_WEATHER_DYN_JUMP_DESC" );
		m_ButtonPhaseRemove = m_PhaseCard.AddDeleteButton( this, "OnClick_PhaseRemove", "#STR_COT_WEATHER_DYN_PHASE_REMOVE" );

		Widget phaseBody = UIActionManager.CreateGridSpacer( m_PhaseCard.GetContent(), 3, 1 );

		m_SelectPhase = BuildPickRow( phaseBody, "#STR_COT_WEATHER_DYN_PHASE", "OnChange_PhaseSelect", "OnClick_RenamePhase", "#STR_COT_WEATHER_DYN_RENAME_PHASE_DESC" );
		m_ButtonRenamePhase = m_PickRename;

		//! Both are ranges, and both are edited in the unit they read best in.
		m_RangeDuration = UIActionManager.CreateSliderRange( phaseBody, "#STR_COT_WEATHER_DYNAMIC_DURATION", 1, DURATION_MAX_MINUTES, this, "OnChange_Dirty" );
		m_RangeDuration.SetFormat( "#STR_COT_FORMAT_MINUTE" );
		m_RangeDuration.SetStep( 1 );
		m_RangeDuration.SetTooltip( "#STR_COT_WEATHER_DYNAMIC_DURATION_DESC" );

		m_RangeTransition = UIActionManager.CreateSliderRange( phaseBody, "#STR_COT_WEATHER_DYNAMIC_TRANSITION", 0, TRANSITION_MAX_SECONDS, this, "OnChange_Dirty" );
		m_RangeTransition.SetFormat( "#STR_COT_FORMAT_SECOND" );
		m_RangeTransition.SetStep( 10 );
		m_RangeTransition.SetTooltip( "#STR_COT_WEATHER_DYNAMIC_TRANSITION_DESC" );

		m_EntryCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_DYN_ENTRY" );
		m_ListEntry = new JMWeatherChanceList( m_Form, m_Scroller, m_EntryCard.GetContent(), "#STR_COT_WEATHER_DYN_ENTRY_ADD" );

		m_EdgesCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_DYN_EDGES" );
		m_ListEdges = new JMWeatherChanceList( m_Form, m_Scroller, m_EdgesCard.GetContent(), "#STR_COT_WEATHER_DYN_EDGES_ADD" );
	}

	// -------------------------------------------------------------------------
	//  What the form and the banner ask
	// -------------------------------------------------------------------------

	bool IsEditing()
	{
		return m_EditMode && m_Working != NULL;
	}

	//! A preset not sent to the server yet.
	bool IsCreating()
	{
		return m_Working != NULL && m_TargetName == "" && m_PendingName == "";
	}

	string GetSelectedName()
	{
		if ( !m_Working )
			return "";

		return m_Working.Name;
	}

	//! The name the preset is being edited under - what Save writes.
	string GetEditedName()
	{
		return GetSelectedName();
	}

	//! The weather the tabs are showing while a preset is edited: the working
	//! phase's own. NULL when nothing is being edited - the tabs show the world.
	JMWeatherPreset EditingWeather()
	{
		if ( !IsEditing() )
			return NULL;

		JMWeatherPhase phase = m_Working.GetPhase( m_PhaseName );

		if ( !phase )
			return NULL;

		return phase.Conditions;
	}

	// -------------------------------------------------------------------------
	//  Reading the stored configuration
	// -------------------------------------------------------------------------

	protected JMWeatherSerialize Settings()
	{
		if ( !m_Form.m_Module || !m_Form.m_Module.HasSettings() )
			return NULL;

		return m_Form.m_Module.GetSettings();
	}

	protected bool IsDirty()
	{
		if ( m_Dirty )
			return true;

		return m_ListNext.IsChanged() || m_ListEntry.IsChanged() || m_ListEdges.IsChanged();
	}

	//! There is something the server does not have: a preset never saved, or an
	//! edit - in the editor's own controls or, in edit mode, on the weather tabs.
	protected bool IsPending()
	{
		if ( !m_Working )
			return false;

		if ( m_TargetName == "" )
			return true;

		if ( IsDirty() )
			return true;

		return m_EditMode && m_Form.HasDirtySections();
	}

	//! Names of every stored preset.
	protected TStringArray PresetNames()
	{
		TStringArray names = new TStringArray;
		JMWeatherSerialize settings = Settings();

		if ( !settings )
			return names;

		for ( int i = 0; i < settings.States.Count(); i++ )
			names.Insert( settings.States[i].Name );

		return names;
	}

	//! A name for a new preset that no stored preset has.
	protected string NewPresetName()
	{
		JMWeatherSerialize settings = Settings();

		int n = 1;
		string candidate = "Preset " + n;

		while ( settings && settings.HasState( candidate ) )
		{
			n++;
			candidate = "Preset " + n;
		}

		return candidate;
	}

	// -------------------------------------------------------------------------
	//  Refresh - the stored configuration changed
	// -------------------------------------------------------------------------

	//! Called whenever the settings are (re)loaded. Keeps the selection by NAME and
	//! reloads the working copy from the stored preset - unless the person has an
	//! unsaved edit on screen, which a refresh (another admin's save, a preset
	//! created elsewhere) must not throw away. Edit mode counts as one: the weather
	//! controls hold values the working copy does not have yet.
	void RefreshFromSettings()
	{
		JMWeatherSerialize settings = Settings();

		if ( !settings || !m_SelectPreset )
			return;

		//! Also puts the selector back on the working preset, however the list moved.
		RebuildPresetList();

		//! Nothing selected: nothing stored to follow.
		if ( !m_Working )
		{
			UpdateStates();
			return;
		}

		if ( m_TargetName != "" && !settings.HasState( m_TargetName ) )
		{
			//! Just saved and not seen back yet: keep what is on screen.
			if ( m_TargetName == m_PendingName )
			{
				UpdateStates();
				return;
			}

			//! Deleted out from under the editor.
			ExitEdit();
			SelectRow( ROW_NONE );
			return;
		}

		if ( m_TargetName == m_PendingName )
			m_PendingName = "";

		//! Nothing unsaved: follow the stored preset, on the phase already showing.
		if ( m_TargetName != "" && !IsPending() )
		{
			string keepPhase = m_PhaseName;

			SelectRow( SelectorRowFor( m_TargetName ) );

			if ( m_Working && keepPhase != m_PhaseName && m_Working.HasPhase( keepPhase ) )
			{
				m_PhaseName = keepPhase;

				RebuildPhaseList();
				LoadPhase();
				LoadEntry();
			}

			return;
		}

		//! Keep the edit, but the lists of what can be picked have to follow the
		//! stored configuration (a preset may have appeared or gone).
		RefreshCandidates();
		UpdateStates();
	}

	protected int SelectorRowFor( string name )
	{
		JMWeatherSerialize settings = Settings();

		if ( !settings || name == "" )
			return ROW_NONE;

		for ( int i = 0; i < settings.States.Count(); i++ )
		{
			if ( settings.States[i].Name == name )
				return i + ROW_FIRST;
		}

		return ROW_NONE;
	}

	//! The working preset has no stored counterpart yet - a new one, or one saved
	//! and not seen back - so the selector needs a row of its own for it.
	protected bool HasExtraRow()
	{
		if ( !m_Working )
			return false;

		if ( m_TargetName == "" )
			return true;

		JMWeatherSerialize settings = Settings();

		if ( !settings )
			return false;

		return !settings.HasState( m_TargetName );
	}

	//! The selector row of whatever is being edited.
	protected int CurrentRow()
	{
		if ( !m_Working )
			return ROW_NONE;

		if ( HasExtraRow() )
			return ROW_FIRST + PresetNames().Count();

		return SelectorRowFor( m_TargetName );
	}

	//! The selector lists stored presets, plus a row for one being created. The
	//! preset being edited shows under its working name, so a rename is visible before
	//! it is saved.
	protected void RebuildPresetList()
	{
		if ( !m_SelectPreset )
			return;

		array<string> options = new array<string>;

		options.Insert( "#STR_COT_WEATHER_DYN_PRESET_NONE" );

		TStringArray names = PresetNames();
		string label;

		for ( int i = 0; i < names.Count(); i++ )
		{
			label = names[i];

			if ( m_Working && names[i] == m_TargetName )
				label = m_Working.Name;

			options.Insert( label );
		}

		if ( HasExtraRow() )
			options.Insert( m_Working.Name );

		m_SelectPreset.SetItems( options );
		m_SelectPreset.SetSelection( CurrentRow(), false );
	}

	// -------------------------------------------------------------------------
	//  Selecting a preset
	// -------------------------------------------------------------------------

	//! Make the selector row current and load what it stands for. Always leaves edit
	//! mode - a caller that wants it kept re-enters it after.
	protected void SelectRow( int row )
	{
		JMWeatherSerialize settings = Settings();

		m_Dirty       = false;
		m_PendingName = "";

		if ( m_EditMode )
			ExitEdit();

		if ( row == ROW_NONE || !settings )
		{
			m_Working    = NULL;
			m_TargetName = "";
			m_PhaseName  = "";

			RebuildPresetList();
			m_Form.UpdateModeBanner();
			ShowEditor( false );
			return;
		}

		int index = row - ROW_FIRST;

		if ( index < 0 || index >= settings.States.Count() )
		{
			SelectRow( ROW_NONE );
			return;
		}

		m_Working    = settings.States[index].Copy();
		m_TargetName = m_Working.Name;

		if ( m_Working.Phases.Count() > 0 )
			m_PhaseName = m_Working.Phases[0].Name;
		else
			m_PhaseName = "";

		//! A rename left on the previous preset's row, or its extra row, goes with it.
		RebuildPresetList();

		LoadState();
		m_Form.UpdateModeBanner();
		ShowEditor( true );

		//! Picking a preset IS editing it - there is no separate mode to switch on.
		EnterEdit();
	}

	//! A fresh preset with one phase, not stored anywhere yet.
	protected void StartNew()
	{
		if ( m_EditMode )
			ExitEdit();

		m_Dirty       = false;
		m_PendingName = "";

		m_Working = new JMWeatherState;
		m_Working.Name = NewPresetName();

		JMWeatherPhase first = new JMWeatherPhase;
		first.Name = "Phase 1";

		//! A phase nobody has wired up yet must not trap the machine in it.
		JMWeatherNextState leave = new JMWeatherNextState;
		leave.To     = "";
		leave.Chance = 100;
		first.Edges.Insert( leave );

		m_Working.Phases.Insert( first );

		m_TargetName = "";
		m_PhaseName  = first.Name;

		RebuildPresetList();

		LoadState();
		m_Form.UpdateModeBanner();
		ShowEditor( true );
	}

	//! Which cards are on screen. Nothing selected shows only the Presets card - so
	//! with no preset at all there is nothing but the way to add one. A list card is
	//! there only while it has rows or something left to add, and the opening-phase
	//! choice only with more than one phase to choose between.
	protected void ShowEditor( bool show )
	{
		if ( !show )
			m_ShownMask = -1;

		UpdateCardVisibility();
		UpdateStates();
	}

	protected void UpdateCardVisibility()
	{
		bool has = ( m_Working != NULL );

		bool multiple = false;

		if ( has )
			multiple = m_Working.Phases.Count() > 1;

		bool showNext  = has && m_ListNext.HasContent();
		bool showPhase = has;
		bool showEntry = multiple && m_ListEntry.HasContent();
		bool showEdges = has && m_ListEdges.HasContent();

		int mask = 0;

		if ( showNext )
			mask = mask | 1;

		if ( showPhase )
			mask = mask | 2;

		if ( showEntry )
			mask = mask | 4;

		if ( showEdges )
			mask = mask | 8;

		if ( mask == m_ShownMask )
			return;

		m_ShownMask = mask;

		m_ToggleRotation.SetVisible( has );
		m_NextCard.SetVisible( showNext );
		m_PhaseCard.SetVisible( showPhase );
		m_EntryCard.SetVisible( showEntry );
		m_EdgesCard.SetVisible( showEdges );

		if ( m_Scroller )
			m_Scroller.UpdateScroller();
	}

	//! Working copy into every control.
	protected void LoadState()
	{
		m_ToggleRotation.SetChecked( m_Working.InRotation );

		RefreshCandidates();

		m_ListNext.Load( m_Working.NextStates );

		RebuildPhaseList();
		LoadPhase();
		LoadEntry();
	}

	//! What each list may offer follows the stored presets and the working phases.
	protected void RefreshCandidates()
	{
		TStringArray names = PresetNames();

		m_ListNext.SetCandidates( names, names );

		RefreshPhaseCandidates();
	}

	protected void RefreshPhaseCandidates()
	{
		if ( !m_Working )
			return;

		TStringArray phaseNames = new TStringArray;

		int i;

		for ( i = 0; i < m_Working.Phases.Count(); i++ )
			phaseNames.Insert( m_Working.Phases[i].Name );

		m_ListEntry.SetCandidates( phaseNames, phaseNames );

		//! Every phase - the current one included, since a phase that lists ITSELF
		//! is how weather persists across several rolls - plus the way out of the
		//! preset. "" is the name a leave edge is stored under, so it is a candidate
		//! like any phase.
		TStringArray edgeNames  = new TStringArray;
		TStringArray edgeLabels = new TStringArray;

		for ( i = 0; i < m_Working.Phases.Count(); i++ )
		{
			edgeNames.Insert( m_Working.Phases[i].Name );
			edgeLabels.Insert( m_Working.Phases[i].Name );
		}

		edgeNames.Insert( "" );
		edgeLabels.Insert( "#STR_COT_WEATHER_DYN_LEAVE" );

		m_ListEdges.SetCandidates( edgeNames, edgeLabels );
	}

	protected void LoadEntry()
	{
		m_ListEntry.Load( m_Working.Entry );

		UpdateCardVisibility();
	}

	protected void RebuildPhaseList()
	{
		array<string> options = new array<string>;

		for ( int i = 0; i < m_Working.Phases.Count(); i++ )
			options.Insert( m_Working.Phases[i].Name );

		m_SelectPhase.SetItems( options );
		m_SelectPhase.SetSelection( PhaseRowFor( m_PhaseName ), false );
	}

	protected int PhaseRowFor( string name )
	{
		for ( int i = 0; i < m_Working.Phases.Count(); i++ )
		{
			if ( m_Working.Phases[i].Name == name )
				return i;
		}

		return 0;
	}

	//! The phase named m_PhaseName into the phase controls - and, while editing, its
	//! weather into the weather tabs.
	protected void LoadPhase()
	{
		JMWeatherPhase phase = m_Working.GetPhase( m_PhaseName );

		if ( !phase )
			return;

		m_RangeDuration.SetRange( SecondsToMinutes( phase.DurationMin ), SecondsToMinutes( phase.DurationMax ) );
		m_RangeTransition.SetRange( phase.TransitionMin, phase.TransitionMax );

		RefreshPhaseCandidates();
		m_ListEdges.Load( phase.Edges );

		if ( m_EditMode )
			m_Form.LoadEditorValues();
	}

	//! Whole minutes, never zero - a hold of "0 minutes" is not a thing the
	//! machine can honour, and the slider starts at 1.
	protected int SecondsToMinutes( int seconds )
	{
		int minutes = seconds / 60;

		if ( minutes < 1 )
			minutes = 1;

		return minutes;
	}

	// -------------------------------------------------------------------------
	//  Controls into the working copy
	// -------------------------------------------------------------------------

	//! The preset-level controls and lists. The name is not among them: it is set
	//! through its popup, straight into the working copy.
	protected void CommitState()
	{
		if ( !m_Working )
			return;

		m_Working.InRotation = m_ToggleRotation.IsChecked();

		m_Working.NextStates = m_ListNext.Read();
		m_Working.Entry      = m_ListEntry.Read();
	}

	//! The phase controls into the phase they are showing.
	protected void CommitPhase()
	{
		if ( !m_Working )
			return;

		JMWeatherPhase phase = m_Working.GetPhase( m_PhaseName );

		if ( !phase )
			return;

		phase.DurationMin   = Math.Round( m_RangeDuration.GetRangeLow() ) * 60;
		phase.DurationMax   = Math.Round( m_RangeDuration.GetRangeHigh() ) * 60;
		phase.TransitionMin = Math.Round( m_RangeTransition.GetRangeLow() );
		phase.TransitionMax = Math.Round( m_RangeTransition.GetRangeHigh() );

		phase.Edges = m_ListEdges.Read();

		CommitWeather( phase );
	}

	//! The weather tabs into the phase's weather - but only the sections that were
	//! actually touched. A tab that was never opened has no controls to read, and a
	//! card that was not touched still shows the WORLD's value where the phase says
	//! "leave this alone"; writing those back would turn every untouched field of
	//! the preset into a setting.
	protected void CommitWeather( JMWeatherPhase phase )
	{
		if ( !m_EditMode )
			return;

		JMWeatherPreset fresh = new JMWeatherPreset;
		m_Form.GetUIActionValues( fresh );

		JMWeatherPreset weather = phase.Conditions;

		if ( m_Form.IsSectionDirty( JMWeatherForm.SECTION_OVERCAST ) )
			weather.POvercast = fresh.POvercast;

		if ( m_Form.IsSectionDirty( JMWeatherForm.SECTION_FOG ) )
		{
			weather.PFog    = fresh.PFog;
			weather.PDynFog = fresh.PDynFog;
		}

		if ( m_Form.IsSectionDirty( JMWeatherForm.SECTION_STORM ) )
			weather.Storm = fresh.Storm;

		if ( m_Form.IsSectionDirty( JMWeatherForm.SECTION_SANDSTORM ) )
			weather.PSandstorm = fresh.PSandstorm;

		if ( m_Form.IsSectionDirty( JMWeatherForm.SECTION_RAIN ) )
		{
			weather.PRain         = fresh.PRain;
			weather.RainThreshold = fresh.RainThreshold;
		}

		if ( m_Form.IsSectionDirty( JMWeatherForm.SECTION_SNOW ) )
		{
			weather.PSnow         = fresh.PSnow;
			weather.SnowThreshold = fresh.SnowThreshold;
		}

		if ( m_Form.IsSectionDirty( JMWeatherForm.SECTION_WIND ) )
		{
			weather.PWindMagnitude = fresh.PWindMagnitude;
			weather.PWindDirection = fresh.PWindDirection;
		}

		if ( m_Form.IsSectionDirty( JMWeatherForm.SECTION_WIND_FUNCTION ) )
			weather.WindFunc = fresh.WindFunc;

		weather.ClearDate();
	}

	//! Both halves. Order matters: state first, so its lists are read while they
	//! still hold the names they were loaded with, then the phase.
	//!
	//! Loading a list or the weather tabs afterwards forgets what was changed, so
	//! whatever was pending is remembered here, before it goes: an edit is only
	//! ever cleared by a save.
	protected void CommitAll()
	{
		if ( IsPending() )
			m_Dirty = true;

		CommitState();
		CommitPhase();
	}

	// -------------------------------------------------------------------------
	//  Edit mode
	// -------------------------------------------------------------------------

	//! Put the weather tabs in front of the selected phase's weather.
	protected void EnterEdit()
	{
		if ( !m_Working )
			return;

		m_EditMode = true;

		//! Straight after the flag is set and before anything is loaded: the form asks
		//! this editor for the weather to show, and for whether it is in preset mode.
		m_Form.LoadEditorValues();
		m_Form.UpdateModeBanner();

		PreviewWeather();

		m_Form.UpdateStates();
	}

	//! Back to the live world. The working copy is thrown away with it - Save is the
	//! only way an edit is kept - so the preset is reloaded from what is stored.
	void ExitEdit()
	{
		if ( !m_EditMode )
			return;

		m_EditMode = false;

		m_Form.SetRangeModeAll( false );
		m_Form.ClearDirty();

		m_Form.LoadEditorValues();
		m_Form.UpdateModeBanner();
		m_Form.UpdateStates();
	}

	//! Show the working phase's weather on THIS CLIENT, immediately.
	//!
	//! Picking a preset to edit is asking what it looks like, and answering that
	//! with a panel of numbers when the sky is right there is the long way round. It
	//! goes on locally, so nobody else's weather moves, with the fades stripped so
	//! it is the END state rather than a slow drift toward it. Weather only: a
	//! preset carries no date. The server owns the weather and syncs it, so this
	//! lasts until the next sync - the right lifetime for a preview.
	protected void PreviewWeather()
	{
		JMWeatherFormTabSky           sky           = m_Form.m_TabSkyCtrl;
		JMWeatherFormTabPrecipitation precipitation = m_Form.m_TabPrecipitationCtrl;
		JMWeatherFormTabWind          wind          = m_Form.m_TabWindCtrl;

		if ( sky )
		{
			sky.ApplyOvercast();
			sky.ApplyFog();
			sky.ApplyStorm();
		}

		if ( precipitation )
		{
			precipitation.ApplyRain();
			precipitation.ApplySnow();
		}

		if ( wind )
		{
			wind.ApplyWind();
			wind.ApplyWindFunction();
		}
	}

	// -------------------------------------------------------------------------
	//  The Save icon's glow
	// -------------------------------------------------------------------------

	//! Per-frame, from the tab. Green and pulsing while there is something to save,
	//! plain once it is saved. The pulse is a fixed-length animation, so it is
	//! started again whenever one has run its course.
	void Poll()
	{
		if ( !m_ButtonSave )
			return;

		bool pending = IsPending();

		if ( pending != m_SaveGlow )
		{
			m_SaveGlow    = pending;
			m_SavePulseAt = 0;

			PaintSave();
		}

		if ( !m_SaveGlow )
			return;

		float now = g_Game.GetTickTime();

		if ( now - m_SavePulseAt < SAVE_PULSE_INTERVAL )
			return;

		m_SavePulseAt = now;

		m_ButtonSave.TriggerPulse( SAVE_PULSE_SECONDS );
	}

	protected void PaintSave()
	{
		if ( m_SaveGlow )
		{
			m_ButtonSave.SetColor( JMTheme.SUCCESS_FILL );
		}
		else
		{
			m_ButtonSave.StopPulse();
			m_ButtonSave.SetColor( JMTheme.BUTTON_FILL );
		}
	}

	// -------------------------------------------------------------------------
	//  Called through the form and the banner
	// -------------------------------------------------------------------------

	//! The banner's Save and the card's Save icon mean the same thing.
	void Save()
	{
		if ( !m_Working || !m_Form.m_Module )
			return;

		CommitAll();

		//! The server refuses a blank name and one another preset already has; caught
		//! here so the editor is not left believing it saved under a name it did not.
		string saveName = m_Working.Name.Trim();
		JMWeatherSerialize settings = Settings();

		if ( saveName == "" )
			return;

		if ( saveName != m_TargetName && settings && settings.HasState( saveName ) )
			return;

		m_Working.Name = saveName;

		JMWeatherDynamicConfig config = new JMWeatherDynamicConfig;

		config.Enabled    = m_Form.m_Module.IsDynamicEnabled();
		config.Op         = JMWeatherDynamicConfig.OP_SAVE;
		config.TargetName = m_TargetName;
		config.State      = m_Working.Copy();

		//! From here the preset is known to the server as its new name. Until that
		//! shows up in the settings, RefreshFromSettings leaves the screen alone.
		//!
		//! Set BEFORE sending: on a listen server the send runs the whole save and
		//! refreshes this editor before it returns, and a rename would look like the
		//! old name having been deleted out from under it.
		m_TargetName  = m_Working.Name;
		m_PendingName = m_Working.Name;
		m_Dirty       = false;

		//! What was on screen is now what is stored, so nothing is pending.
		m_ListNext.ClearChanged();
		m_ListEntry.ClearChanged();
		m_ListEdges.ClearChanged();
		m_Form.ClearDirty();

		m_Form.m_Module.SetDynamicWeather( config );

		m_Form.UpdateModeBanner();
		UpdateStates();
	}

	//! Drop the selection - and with it edit mode. What was edited and not saved is
	//! thrown away.
	void Deselect()
	{
		SelectRow( ROW_NONE );
	}

	//! Selecting from outside - the form's SetSelectedPreset.
	void SelectByName( string name )
	{
		SelectRow( SelectorRowFor( name ) );
	}

	void AddPreset()
	{
		StartNew();
		EnterEdit();
	}

	void RemoveConfirmed()
	{
		if ( !m_Form.m_Module )
			return;

		//! A preset that was never saved has nothing on the server to delete.
		if ( m_TargetName != "" )
		{
			JMWeatherDynamicConfig config = new JMWeatherDynamicConfig;

			config.Enabled    = m_Form.m_Module.IsDynamicEnabled();
			config.Op         = JMWeatherDynamicConfig.OP_REMOVE;
			config.TargetName = m_TargetName;

			m_Form.m_Module.SetDynamicWeather( config );
		}

		SelectRow( ROW_NONE );
	}

	// -------------------------------------------------------------------------
	//  Renaming - the popups' answers, routed here by the form
	// -------------------------------------------------------------------------

	void ApplyPresetName( string name )
	{
		if ( !m_Working )
			return;

		string clean = name.Trim();
		JMWeatherSerialize settings = Settings();

		if ( clean == "" || clean == m_Working.Name )
			return;

		if ( clean != m_TargetName && settings && settings.HasState( clean ) )
		{
			m_Form.ShowNotice( "#STR_COT_WEATHER_DYN_RENAME_PRESET_TITLE", "#STR_COT_WEATHER_DYN_NAME_TAKEN" );
			return;
		}

		m_Working.Name = clean;
		m_Dirty        = true;

		RebuildPresetList();
		m_Form.UpdateModeBanner();
		UpdateStates();
	}

	void ApplyPhaseName( string name )
	{
		if ( !m_Working )
			return;

		string clean = name.Trim();

		if ( clean == "" || clean == m_PhaseName )
			return;

		if ( m_Working.HasPhase( clean ) )
		{
			m_Form.ShowNotice( "#STR_COT_WEATHER_DYN_RENAME_PHASE_TITLE", "#STR_COT_WEATHER_DYN_NAME_TAKEN" );
			return;
		}

		//! What is on screen goes in under the old name first, then the rename
		//! rewrites every edge that named the phase - in this preset and in the lists
		//! about to be reloaded from it.
		CommitAll();

		m_Working.RenamePhase( m_PhaseName, clean );

		m_PhaseName = clean;
		m_Dirty     = true;

		RebuildPhaseList();
		LoadPhase();
		LoadEntry();
		UpdateStates();
	}

	// -------------------------------------------------------------------------
	//  Handlers
	// -------------------------------------------------------------------------

	void OnChange_Dirty( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CHANGE || eid == UIEvent.CLICK )
			m_Dirty = true;
	}

	void OnChange_PresetSelect( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || !m_SelectPreset )
			return;

		int row = m_SelectPreset.GetSelection();

		//! The row of the preset being created is the current one - picking it is not
		//! a change, and selecting "by index" would discard it.
		if ( HasExtraRow() && row == CurrentRow() )
			return;

		SelectRow( row );
	}

	void OnChange_PhaseSelect( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || !m_Working || !m_SelectPhase )
			return;

		int row = m_SelectPhase.GetSelection();

		if ( row < 0 || row >= m_Working.Phases.Count() )
			return;

		//! Leave the phase being edited BEFORE showing another.
		CommitAll();

		m_PhaseName = m_Working.Phases[row].Name;

		RebuildPhaseList();
		LoadPhase();
		LoadEntry();
	}

	void OnClick_Add( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		AddPreset();
	}

	void OnClick_Apply( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Working || !m_Form.m_Module || m_TargetName == "" )
			return;

		action.AnimateFeedback();

		//! Names as the server has them: what is saved, not what is on screen.
		m_Form.m_Module.UsePreset( m_TargetName, m_PhaseName );
	}

	//! A saved preset asks first, since it is gone for everyone; one never saved is
	//! only ever on this screen and just goes.
	void OnClick_Delete( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Working )
			return;

		if ( m_TargetName == "" )
		{
			RemoveConfirmed();
			return;
		}

		m_Form.RemovePreset( m_TargetName );
	}

	void OnClick_Save( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		action.AnimateFeedback();

		Save();
	}

	void OnClick_RenamePreset( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Working )
			return;

		JMConfirmation prompt = m_Form.PromptInput( "#STR_COT_WEATHER_DYN_RENAME_PRESET_TITLE", "#STR_COT_WEATHER_DYN_RENAME_BODY", "RenamePreset_Confirm" );

		if ( prompt )
			prompt.SetEditBoxText( m_Working.Name );
	}

	void OnClick_RenamePhase( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Working )
			return;

		JMConfirmation prompt = m_Form.PromptInput( "#STR_COT_WEATHER_DYN_RENAME_PHASE_TITLE", "#STR_COT_WEATHER_DYN_RENAME_BODY", "RenamePhase_Confirm" );

		if ( prompt )
			prompt.SetEditBoxText( m_PhaseName );
	}

	void OnClick_PhaseAdd( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Working )
			return;

		CommitAll();

		JMWeatherPhase added = new JMWeatherPhase;
		added.Name = m_Working.NextPhaseName();

		//! Comes in leaving the preset, so a phase nobody has wired up yet cannot
		//! trap the machine in it.
		JMWeatherNextState leave = new JMWeatherNextState;
		leave.To     = "";
		leave.Chance = 100;
		added.Edges.Insert( leave );

		m_Working.Phases.Insert( added );

		m_PhaseName = added.Name;
		m_Dirty     = true;

		RebuildPhaseList();
		LoadPhase();
		LoadEntry();
		UpdateStates();
	}

	//! Only the working copy changes - nothing is lost until Save - so no question.
	void OnClick_PhaseRemove( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Working || m_Working.Phases.Count() <= 1 )
			return;

		//! The preset's own lists are read first so nothing on screen is lost, then
		//! the phase goes and takes every edge that named it with it.
		CommitState();

		m_Working.RemovePhase( m_PhaseName );

		m_PhaseName = m_Working.Phases[0].Name;
		m_Dirty     = true;

		LoadState();
		UpdateStates();
	}

	void OnClick_Jump( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Working || !m_Form.m_Module || m_TargetName == "" )
			return;

		action.AnimateFeedback();

		//! Names as the server has them: what is saved, not what is on screen.
		JMWeatherDynamicControl control = new JMWeatherDynamicControl;

		control.Op    = JMWeatherDynamicControl.OP_JUMP;
		control.State = m_TargetName;
		control.Phase = m_PhaseName;

		m_Form.m_Module.SendDynamicControl( control );
	}

	// -------------------------------------------------------------------------
	//  Permissions / enable state
	// -------------------------------------------------------------------------

	void UpdateStates()
	{
		bool none  = ( m_Working == NULL );
		bool saved = ( !none && m_TargetName != "" );

		string editPermission = JMConstants.PERM_WEATHER_PRESET_UPDATE;

		if ( none || m_TargetName == "" )
			editPermission = JMConstants.PERM_WEATHER_PRESET_CREATE;

		m_Form.UpdateActionState( m_SelectPreset, "" );
		m_Form.UpdateActionState( m_ButtonRenamePreset, editPermission, none );
		m_Form.UpdateActionState( m_ToggleRotation, JMConstants.PERM_WEATHER_PRESET_UPDATE, none );

		m_Form.UpdateActionState( m_ButtonAdd, JMConstants.PERM_WEATHER_PRESET_CREATE );
		m_Form.UpdateActionState( m_ButtonApply, JMConstants.PERM_WEATHER_PRESET_USE, !saved );
		m_Form.UpdateActionState( m_ButtonDelete, JMConstants.PERM_WEATHER_PRESET_REMOVE, none );
		m_Form.UpdateActionState( m_ButtonSave, editPermission, none );

		m_Form.UpdateActionState( m_SelectPhase, "", none );
		m_Form.UpdateActionState( m_ButtonRenamePhase, JMConstants.PERM_WEATHER_PRESET_UPDATE, none );
		m_Form.UpdateActionState( m_RangeDuration, JMConstants.PERM_WEATHER_PRESET_UPDATE, none );
		m_Form.UpdateActionState( m_RangeTransition, JMConstants.PERM_WEATHER_PRESET_UPDATE, none );
		m_Form.UpdateActionState( m_ButtonPhaseAdd, JMConstants.PERM_WEATHER_PRESET_UPDATE, none );

		//! A preset's last phase cannot be removed - it could not be entered.
		bool onlyPhase = ( none || m_Working.Phases.Count() <= 1 );
		m_Form.UpdateActionState( m_ButtonPhaseRemove, JMConstants.PERM_WEATHER_PRESET_UPDATE, onlyPhase );

		bool jumpable = ( !saved || !m_Form.m_Module || !m_Form.m_Module.IsDynamicEnabled() );
		m_Form.UpdateActionState( m_ButtonJump, JMConstants.PERM_WEATHER_PRESET_DYNAMIC, jumpable );

		m_ListNext.UpdateStates( JMConstants.PERM_WEATHER_PRESET_UPDATE, none );
		m_ListEntry.UpdateStates( JMConstants.PERM_WEATHER_PRESET_UPDATE, none );
		m_ListEdges.UpdateStates( JMConstants.PERM_WEATHER_PRESET_UPDATE, none );

		//! A list may have just filled up or emptied, which changes which cards show.
		UpdateCardVisibility();
	}
}
