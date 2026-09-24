//! Floating "edit exact values" popup for a tracked object's Position and
//! Orientation - reached from JMESPActionMenu's Transform page. The menu
//! itself is a plain string-item list (UIActionContextMenu.AddItem) with no
//! room for input fields, so free-form entry lives here instead, following
//! the same floating-panel-over-a-stub-widget shape UIActionContextMenu uses
//! (see that class's header comment) rather than trying to embed a text box
//! in a menu row.
//!
//! Every small icon button here (close, refresh, copy, paste, apply) is
//! built and hover-handled the same way JMWindowBase's own close/minimize/
//! pin title-bar buttons are: a plain ButtonWidget with a translucent
//! "*_hover" backdrop panel shown on OnMouseEnter and hidden on OnMouseLeave,
//! not a UIActionImageButton - see JMWindowBase.OnMouseEnter/OnMouseLeave/
//! GetTitleButtonHover for the pattern this mirrors.
class JMESPTransformPopup: UIActionBase
{
	protected Widget m_Anchor;
	protected Widget m_Panel;
	protected Widget m_Content;
	protected Widget m_TitleButtons;

	//! Parallel arrays: button widget, its hover backdrop, which field-action
	//! it fires. Index-matched rather than eight named field pairs so
	//! OnMouseEnter/OnMouseLeave/OnClick can each be one small loop, the same
	//! shape GetTitleButtonHover uses for the three title-bar buttons.
	protected ref array<Widget> m_Buttons = new array<Widget>;
	protected ref array<Widget> m_ButtonHovers = new array<Widget>;
	protected ref array<int> m_ButtonActions = new array<int>;

	//! Click feedback, index-matched to the arrays above. Copy/paste/apply
	//! swap their icon for a checkmark for FEEDBACK_HOLD seconds; refresh
	//! spins instead - see TriggerFeedback/TriggerSpin.
	protected ref array<ImageWidget> m_ButtonIcons = new array<ImageWidget>;
	protected ref array<string> m_ButtonRestIcon = new array<string>;
	protected ref array<bool> m_ButtonIsSpin = new array<bool>;
	protected ref array<float> m_ButtonFeedbackTimer = new array<float>;
	protected ref array<float> m_ButtonSpinAngle = new array<float>;
	static const float FEEDBACK_HOLD = 0.6;
	static const float SPIN_DURATION = 0.5;

	//! Double click to arm a spin button (refresh) to fire itself once a
	//! second, double click again to stop - the same feature
	//! UIActionImageButton's CreateRefreshButton gets, reimplemented here
	//! because these are plain ButtonWidgets (see the class note on why).
	//! Index-matched to m_Buttons like every other per-button array; only
	//! ever true for the one spin (refresh) button, but kept general rather
	//! than a single field so a second auto-repeatable action needs no new
	//! plumbing.
	static const int   AUTO_REPEAT_DOUBLE_CLICK_MS = 400;
	static const float AUTO_REPEAT_INTERVAL        = 1.0;
	static const int   ICON_REST_COLOR             = 0xBFD4DBE6;
	protected ref array<bool>  m_ButtonAutoRepeatEnabled = new array<bool>;
	protected ref array<bool>  m_ButtonAutoRepeatActive = new array<bool>;
	protected ref array<float> m_ButtonAutoRepeatTimer = new array<float>;
	protected ref array<int>   m_ButtonLastClickTime = new array<int>;
	protected JMESPVectorInputRow m_Position;
	protected JMESPVectorInputRow m_Orientation;
	protected JMESPModule m_Module;
	protected Object m_Target;
	protected bool m_Open;
	protected float m_PendingX;
	protected float m_PendingY;
	static const int ACTION_CLOSE          = 0;
	static const int ACTION_REFRESH_ALL    = 1;
	static const int ACTION_COPY_POSITION  = 2;
	static const int ACTION_PASTE_POSITION = 3;
	static const int ACTION_APPLY_POSITION = 4;
	static const int ACTION_COPY_ORI       = 5;
	static const int ACTION_PASTE_ORI      = 6;
	static const int ACTION_APPLY_ORI      = 7;
	static const int HOVER_NEUTRAL = 0x2EFFFFFF;
	static const int HOVER_CLOSE   = 0x59FF627D;
	static const float PANEL_WIDTH = 640;

	//! Gap below the last field before the panel's bottom edge, matching
	//! the 10px popup_root insets the content sits in horizontally.
	static const float CONTENT_BOTTOM_PAD = 10;

	override bool IsOpen()
	{
		return m_Open;
	}

	protected void SetButtonAutoRepeatActive( int idx, bool active )
	{
		m_ButtonAutoRepeatActive[idx] = active;
		m_ButtonAutoRepeatTimer[idx] = 0;

		ImageWidget icon = m_ButtonIcons[idx];
		if ( !icon )
			return;

		if ( active )
			icon.SetColor( JMTheme.ACCENT );
		else
			icon.SetColor( ICON_REST_COLOR );
	}

	static JMESPTransformPopup Create( notnull Widget parent, notnull Widget anchor )
	{
		Widget widget = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/esp/JMESPTransformPopup.layout", parent );

		if ( !widget )
			return null;

		JMESPTransformPopup popup;
		widget.GetScript( popup );

		if ( !popup )
			return null;

		popup.InitPopup( anchor );

		return popup;
	}

	void InitPopup( notnull Widget anchor )
	{
		m_Anchor = anchor;
		m_Panel = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/esp/JMESPTransformPopup_Panel.layout", m_Anchor );

		if ( !m_Panel )
			return;

		Class.CastTo( m_Content, m_Panel.FindAnyWidget( "popup_content" ) );

		//! Packed to the END of a container authored the same 96px wide as
		//! the field rows' own button box, rather than from 0 in a box just
		//! wide enough for two. Both boxes then right-align identically and
		//! the close button lands on exactly the same column as each row's
		//! apply button - matching by construction instead of by an offset
		//! tuned against whatever right_ref does with differing widths.
		m_TitleButtons = m_Panel.FindAnyWidget( "popup_titlebar_buttons" );
		if ( m_TitleButtons )
		{
			Widget refreshBtn = AddIconButton( m_TitleButtons, "refresh-cw", ACTION_REFRESH_ALL, true );
			refreshBtn.SetPos( 34, 0 );

			Widget closeBtn = AddIconButton( m_TitleButtons, "x", ACTION_CLOSE );
			closeBtn.SetPos( 68, 0 );
		}

		if ( m_Content )
		{
			m_Position = BuildFieldSection( "#STR_COT_GENERIC_POSITION", ACTION_COPY_POSITION, ACTION_PASTE_POSITION, ACTION_APPLY_POSITION );
			m_Orientation = BuildFieldSection( "#STR_COT_ESP_MODULE_LABEL_ORIENTATION", ACTION_COPY_ORI, ACTION_PASTE_ORI, ACTION_APPLY_ORI );
		}

		m_Panel.Show( false );
		m_Panel.SetHandler( this );

		JMStatics.AddOverlay( m_Panel );
	}

	//! One field: a header line with the label on the left and the copy/
	//! paste/apply icon buttons on the right (JMESPFieldHeader.layout), then
	//! the X/Y/Z inputs alone on their own full-width line below
	//! (JMESPVectorInputRow).
	protected JMESPVectorInputRow BuildFieldSection( string labelKey, int copyAction, int pasteAction, int applyAction )
	{
		Widget header = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/esp/JMESPFieldHeader.layout", m_Content );

		if ( header )
		{
			TextWidget label;
			if ( Class.CastTo( label, header.FindAnyWidget( "field_label" ) ) )
				label.SetText( labelKey );

			//! field_buttons is deliberately an exact structural twin of
			//! popup_titlebar_buttons - same width-fits-its-buttons box,
			//! same halign/-10 offset, sitting on a row that spans the
			//! panel's full width just as the titlebar does. That is the
			//! one arrangement in this popup known to land flush, so the
			//! copy/paste/apply group lines up with the close button by
			//! construction rather than by an offset tuned against it.
			Widget buttons = header.FindAnyWidget( "field_buttons" );
			if ( buttons )
			{
				Widget copyBtn = AddIconButton( buttons, "copy", copyAction );
				copyBtn.SetPos( 0, 0 );

				Widget pasteBtn = AddIconButton( buttons, "clipboard-paste", pasteAction );
				pasteBtn.SetPos( 34, 0 );

				Widget applyBtn = AddIconButton( buttons, "check", applyAction );
				applyBtn.SetPos( 68, 0 );
			}
		}

		return JMESPVectorInputRow.Create( m_Content );
	}

	//! Build one JMESPPopupIconButton, register it (with its hover backdrop
	//! and the action it fires) so OnMouseEnter/OnMouseLeave/OnClick can find
	//! it, and hand back the raw widget so the caller can position it.
	protected Widget AddIconButton( notnull Widget parent, string lucideIcon, int action, bool spin = false )
	{
		Widget btn = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/esp/JMESPPopupIconButton.layout", parent );

		if ( !btn )
			return null;

		btn.SetFlags( WidgetFlags.HEXACTPOS | WidgetFlags.VEXACTPOS, true );
		btn.SetHandler( this );

		ImageWidget icon;
		string iconPath = JMConstants.Lucide( lucideIcon );

		if ( Class.CastTo( icon, btn.FindAnyWidget( "btn_icon" ) ) )
		{
			icon.LoadImageFile( 0, iconPath );
			icon.SetImage( 0 );
		}

		m_Buttons.Insert( btn );
		m_ButtonHovers.Insert( btn.FindAnyWidget( "btn_hover" ) );
		m_ButtonActions.Insert( action );
		m_ButtonIcons.Insert( icon );
		m_ButtonRestIcon.Insert( iconPath );
		m_ButtonIsSpin.Insert( spin );
		m_ButtonFeedbackTimer.Insert( 0 );
		m_ButtonSpinAngle.Insert( 0 );

		//! Only a spin button is safe to leave firing unattended - copy/
		//! paste/apply mutate the object and must never be armable by a
		//! second click.
		m_ButtonAutoRepeatEnabled.Insert( spin );
		m_ButtonAutoRepeatActive.Insert( false );
		m_ButtonAutoRepeatTimer.Insert( 0 );
		m_ButtonLastClickTime.Insert( 0 );

		return btn;
	}

	protected int IndexOfButton( Widget w )
	{
		for ( int i = 0; i < m_Buttons.Count(); i++ )
		{
			if ( m_Buttons[i] == w )
				return i;
		}

		return -1;
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		super.OnMouseEnter( w, x, y );

		int idx = IndexOfButton( w );
		if ( idx < 0 )
			return false;

		Widget hover = m_ButtonHovers[idx];
		if ( !hover )
			return false;

		int color = HOVER_NEUTRAL;
		if ( m_ButtonActions[idx] == ACTION_CLOSE )
			color = HOVER_CLOSE;

		hover.SetColor( color );
		hover.Show( true );

		return true;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		super.OnMouseLeave( w, enterW, x, y );

		int idx = IndexOfButton( w );
		if ( idx < 0 )
			return false;

		Widget hover = m_ButtonHovers[idx];
		if ( hover )
			hover.Show( false );

		return true;
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		int idx = IndexOfButton( w );
		if ( idx < 0 )
			return false;

		DoAction( m_ButtonActions[idx] );

		//! Close is a navigation, not a data action - it needs no receipt,
		//! and the popup (and the icon with it) is gone by the time one
		//! would show anyway.
		if ( m_ButtonActions[idx] != ACTION_CLOSE )
		{
			if ( m_ButtonIsSpin[idx] )
				m_ButtonSpinAngle[idx] = 0.01;
			else
				m_ButtonFeedbackTimer[idx] = FEEDBACK_HOLD;
		}

		if ( m_ButtonAutoRepeatEnabled[idx] )
			ToggleButtonAutoRepeat( idx );

		return true;
	}

	//! A second click on an auto-repeatable button inside the double-click
	//! window arms or disarms it. The click above already fired the action
	//! once, same as UIActionImageButton's own version of this.
	//! Arming needs a double click; stopping does not - see
	//! UIActionImageButton.OnClick's identical note, this button gets the
	//! same behaviour manually since it is a plain ButtonWidget.
	protected void ToggleButtonAutoRepeat( int idx )
	{
		if ( m_ButtonAutoRepeatActive[idx] )
		{
			SetButtonAutoRepeatActive( idx, false );
			m_ButtonLastClickTime[idx] = 0;
			return;
		}

		int now = g_Game.GetTime();

		if ( now - m_ButtonLastClickTime[idx] > AUTO_REPEAT_DOUBLE_CLICK_MS )
		{
			m_ButtonLastClickTime[idx] = now;
			return;
		}

		m_ButtonLastClickTime[idx] = 0;
		SetButtonAutoRepeatActive( idx, true );
	}

	//! Stops every armed button, e.g. when the popup closes - it must not
	//! keep firing at whatever it was last pointed at.
	protected void StopAllButtonAutoRepeat()
	{
		for ( int i = 0; i < m_Buttons.Count(); i++ )
		{
			if ( !m_ButtonAutoRepeatActive[i] )
				continue;

			m_ButtonAutoRepeatActive[i] = false;
			m_ButtonAutoRepeatTimer[i] = 0;

			ImageWidget icon = m_ButtonIcons[i];
			if ( icon )
				icon.SetColor( ICON_REST_COLOR );
		}
	}

	//! Swaps a button's icon to a checkmark for FEEDBACK_HOLD seconds, or
	//! spins it for SPIN_DURATION - the same two confirmations
	//! UIActionImageButton gives every OTHER icon button in COT, reimplemented
	//! here because these are plain ButtonWidgets (see the class note on why).
	protected void UpdateButtonFeedback( float timeSlice )
	{
		for ( int i = 0; i < m_Buttons.Count(); i++ )
		{
			ImageWidget icon = m_ButtonIcons[i];
			if ( !icon )
				continue;

			if ( m_ButtonIsSpin[i] )
			{
				if ( m_ButtonAutoRepeatActive[i] )
				{
					m_ButtonAutoRepeatTimer[i] = m_ButtonAutoRepeatTimer[i] + timeSlice;

					if ( m_ButtonAutoRepeatTimer[i] >= AUTO_REPEAT_INTERVAL )
					{
						m_ButtonAutoRepeatTimer[i] = 0;
						DoAction( m_ButtonActions[i] );
						m_ButtonSpinAngle[i] = 0.01;
					}
				}

				if ( m_ButtonSpinAngle[i] <= 0 )
					continue;

				m_ButtonSpinAngle[i] = m_ButtonSpinAngle[i] + ( timeSlice / SPIN_DURATION ) * 360;

				if ( m_ButtonSpinAngle[i] >= 360 )
				{
					m_ButtonSpinAngle[i] = 0;
					icon.SetRotation( 0, 0, 0 );
					continue;
				}

				icon.SetRotation( 0, 0, m_ButtonSpinAngle[i] );
				continue;
			}

			if ( m_ButtonFeedbackTimer[i] <= 0 )
				continue;

			if ( m_ButtonFeedbackTimer[i] == FEEDBACK_HOLD )
			{
				icon.LoadImageFile( 0, JMConstants.Lucide( "check" ) );
				icon.SetImage( 0 );
				icon.SetColor( ARGB( 191, 62, 224, 156 ) );
			}

			m_ButtonFeedbackTimer[i] = m_ButtonFeedbackTimer[i] - timeSlice;

			if ( m_ButtonFeedbackTimer[i] <= 0 )
			{
				m_ButtonFeedbackTimer[i] = 0;
				icon.LoadImageFile( 0, m_ButtonRestIcon[i] );
				icon.SetImage( 0 );
				icon.SetColor( ARGB( 191, 212, 219, 230 ) );
			}
		}
	}

	protected void DoAction( int action )
	{
		switch ( action )
		{
			case ACTION_CLOSE:
				Close();
				break;
			case ACTION_REFRESH_ALL:
				DoRefreshAll();
				break;
			case ACTION_COPY_POSITION:
				DoCopyPosition();
				break;
			case ACTION_PASTE_POSITION:
				DoPastePosition();
				break;
			case ACTION_APPLY_POSITION:
				DoApplyPosition();
				break;
			case ACTION_COPY_ORI:
				DoCopyOrientation();
				break;
			case ACTION_PASTE_ORI:
				DoPasteOrientation();
				break;
			case ACTION_APPLY_ORI:
				DoApplyOrientation();
				break;
		}
	}

	//! `module`/`target` drive both the live refresh and where Apply sends its
	//! result - the same JMESPModule.SetPosition/SetOrientation the copy/paste
	//! actions already use (see JMESPActionMenu.DoPastePosition).
	void Open( JMESPModule module, notnull Object target, float screenX, float screenY )
	{
		if ( !m_Panel )
			return;

		m_Module = module;
		m_Target = target;
		m_PendingX = screenX;
		m_PendingY = screenY;
		m_Open = true;

		if ( m_Position )
			m_Position.SetEdited( false );

		if ( m_Orientation )
			m_Orientation.SetEdited( false );

		m_Panel.Show( true );
		m_Panel.SetSort( 9999, true );

		UpdatePlacement();
		RefreshValues();
	}

	override void Close()
	{
		m_Open = false;
		m_Target = null;

		StopAllButtonAutoRepeat();

		if ( m_Panel )
			m_Panel.Show( false );
	}

	override void Hide()
	{
		Close();
	}

	override bool IsVisible()
	{
		return m_Open;
	}

	protected void UpdatePlacement()
	{
		if ( !m_Panel || !m_Anchor )
			return;

		float ax, ay;
		m_Anchor.GetScreenPos( ax, ay );

		float screenW, screenH;
		g_Game.GetWorkspace().GetScreenSize( screenW, screenH );

		//! Height is measured, not derived from the titlebar height plus
		//! whatever popup_root's Padding/Margin are currently set to. Those
		//! two do not contribute the way a constant can track - Padding
		//! spaces vertically, Margin horizontally - and every hand-tuned
		//! constant here has eventually clipped the last row. The content's
		//! own offset below the panel top covers all of it at once, and
		//! both values move with the panel, so the difference stays stable
		//! and cannot feed back into the flip-up placement below.
		float panelHeight = 150;
		if ( m_Content )
		{
			float cx, cy, cw, ch;
			m_Content.GetScreenPos( cx, cy );
			m_Content.GetScreenSize( cw, ch );

			float panelX, panelY;
			m_Panel.GetScreenPos( panelX, panelY );

			float measured = ( cy - panelY ) + ch + CONTENT_BOTTOM_PAD;
			if ( measured > panelHeight )
				panelHeight = measured;
		}

		float px = m_PendingX - ax;
		float py = m_PendingY - ay;

		if ( m_PendingX + PANEL_WIDTH > screenW )
			px = m_PendingX - ax - PANEL_WIDTH;
		if ( m_PendingY + panelHeight > screenH )
			py = m_PendingY - ay - panelHeight;

		if ( px < 0 )
			px = 0;
		if ( py < 0 )
			py = 0;

		m_Panel.SetFlags( WidgetFlags.HEXACTSIZE );
		m_Panel.SetFlags( WidgetFlags.VEXACTSIZE );
		m_Panel.SetSize( PANEL_WIDTH, panelHeight );
		m_Panel.SetPos( px, py );
	}

	//! Reads the target's current Position/Orientation into the fields. Runs
	//! when the popup opens and when the refresh button fires (a click, or the
	//! auto-repeat armed by double clicking it) - never on a per-frame timer, so
	//! the fields hold still while the object moves and only change when asked.
	//! See UIActionEditableVector's class note on why SetValue no-ops while the
	//! user is mid-edit; DoRefreshAll drops that flag first.
	protected void RefreshValues()
	{
		if ( !m_Target )
			return;

		if ( m_Position )
			m_Position.SetValue( m_Target.GetPosition() );

		if ( m_Orientation )
			m_Orientation.SetValue( m_Target.GetOrientation() );
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		if ( !m_Open )
			return;

		UpdateButtonFeedback( timeSlice );

		if ( !m_Target )
		{
			Close();
			return;
		}

		UpdatePlacement();

		bool leftDown = ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK ) != 0;
		bool rightDown = ( GetMouseState( MouseState.RIGHT ) & MB_PRESSED_MASK ) != 0;

		if ( !leftDown && !rightDown )
			return;

		Widget under = GetWidgetUnderCursor();

		if ( IsFocusWidget( under ) )
			return;

		Close();
	}

	override bool IsFocusWidget( Widget widget )
	{
		if ( !widget )
			return false;

		Widget w = widget;

		while ( w )
		{
			if ( w == m_Panel )
				return true;

			w = w.GetParent();
		}

		return false;
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

	//! Refresh (click or auto-repeat): discard unapplied edits and reload both
	//! fields from the object.
	protected void DoRefreshAll()
	{
		if ( m_Position )
			m_Position.SetEdited( false );

		if ( m_Orientation )
			m_Orientation.SetEdited( false );

		RefreshValues();
	}

	protected void DoCopyPosition()
	{
		if ( !m_Position )
			return;

		COTFeedback.Copy( m_Position.GetValue().ToString() );
	}

	protected void DoPastePosition()
	{
		if ( !m_Position )
			return;

		string clipboard;
		g_Game.CopyFromClipboard( clipboard );

		vector pos = clipboard.BeautifiedToVector();

		if ( pos == vector.Zero )
			return;

		//! Loads the field, does not send it - Apply still owns that step, so
		//! a bad paste is easy to see and back out of before it goes anywhere.
		m_Position.SetEdited( false );
		m_Position.SetValue( pos );
		m_Position.SetEdited( true );
	}

	protected void DoApplyPosition()
	{
		if ( !m_Module || !m_Target || !m_Position )
			return;

		JMTeleportHistory.PushObject( m_Target );

		m_Module.SetPosition( m_Position.GetValue(), m_Target );
		m_Position.SetEdited( false );
	}

	protected void DoCopyOrientation()
	{
		if ( !m_Orientation )
			return;

		COTFeedback.Copy( m_Orientation.GetValue().ToString() );
	}

	protected void DoPasteOrientation()
	{
		if ( !m_Orientation )
			return;

		string clipboard;
		g_Game.CopyFromClipboard( clipboard );

		vector ori = clipboard.BeautifiedToVector();

		if ( ori == vector.Zero )
			return;

		m_Orientation.SetEdited( false );
		m_Orientation.SetValue( ori );
		m_Orientation.SetEdited( true );
	}

	protected void DoApplyOrientation()
	{
		if ( !m_Module || !m_Target || !m_Orientation )
			return;

		m_Module.SetOrientation( m_Orientation.GetValue(), m_Target );
		m_Orientation.SetEdited( false );
	}
}
