// =============================================================================
//  UIActionStepList
//
//  An ordered list of string items, each displayed with its 1-based index,
//  label text, and Up / Down / Remove buttons.  Fires UIEvent.CHANGE whenever
//  the list order or content changes.  GetItems() returns the current order.
//
//  Usage:
//      m_Steps = UIActionManager.CreateStepList( parent, "Spawn Sequence",
//          this, "OnChange_Steps" );
//      m_Steps.AddItem( "Give AK-74" );
//      m_Steps.AddItem( "Give Vest"  );
//
//      void OnChange_Steps( UIEvent eid, UIActionBase action )
//      {
//          array<string> steps = m_Steps.GetItems();
//      }
// =============================================================================

class JMStepListRow
{
	string       Label;
	Widget       Root;
	TextWidget   IndexText;
	TextWidget   LabelText;
	ButtonWidget BtnUp;
	ButtonWidget BtnDown;
	ButtonWidget BtnRemove;
}

class UIActionStepList: UIActionBase
{
	protected TextWidget m_HeaderLabel;
	protected Widget     m_Rows;

	protected ref array<ref JMStepListRow> m_Items;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_HeaderLabel, layoutRoot.FindAnyWidget( "action_label" ) );
		Class.CastTo( m_Rows,        layoutRoot.FindAnyWidget( "action_rows"  ) );

		m_Items = new array<ref JMStepListRow>;
	}

	override void SetLabel( string text )
	{
		text = Widget.TranslateString( text );
		if ( m_HeaderLabel )
		{
			m_HeaderLabel.SetText( text );
			m_HeaderLabel.Show( text.Length() > 0 );
		}
	}

	void AddItem( string label )
	{
		ref JMStepListRow row = BuildRow( label );
		if ( row )
		{
			m_Items.Insert( row );
			RefreshIndices();
			CallEvent( UIEvent.CHANGE );
		}
	}

	void RemoveItem( int index )
	{
		if ( index < 0 || index >= m_Items.Count() )
			return;

		JMStepListRow row = m_Items[index];
		if ( row && row.Root )
			row.Root.Unlink();

		m_Items.Remove( index );
		RefreshIndices();
		CallEvent( UIEvent.CHANGE );
	}

	void Clear()
	{
		if ( !m_Rows )
			return;

		Widget child = m_Rows.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}
		m_Items.Clear();
		CallEvent( UIEvent.CHANGE );
	}

	array<string> GetItems()
	{
		array<string> result = new array<string>;
		foreach ( JMStepListRow row : m_Items )
			result.Insert( row.Label );
		return result;
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		foreach ( int i, JMStepListRow row : m_Items )
		{
			if ( row.BtnRemove && w == row.BtnRemove )
			{
				RemoveItem( i );
				return true;
			}

			if ( row.BtnUp && w == row.BtnUp && i > 0 )
			{
				m_Items.SwapItems( i, i - 1 );
				RebuildOrder();
				RefreshIndices();
				CallEvent( UIEvent.CHANGE );
				return true;
			}

			if ( row.BtnDown && w == row.BtnDown && i < m_Items.Count() - 1 )
			{
				m_Items.SwapItems( i, i + 1 );
				RebuildOrder();
				RefreshIndices();
				CallEvent( UIEvent.CHANGE );
				return true;
			}
		}
		return false;
	}

	private ref JMStepListRow BuildRow( string label )
	{
		// Row layout: [index 30px] [label fill] [^ 24px] [v 24px] [x 24px]
		Widget rowRoot = g_Game.GetWorkspace().CreateWidgets(
			"JM/COT/GUI/layouts/uiactions/Wrappers/1/GridSpacer5.layout", m_Rows );

		if ( !rowRoot )
			return null;

		rowRoot.SetHandler( this );

		ref JMStepListRow row = new JMStepListRow();
		row.Label = label;
		row.Root  = rowRoot;

		// Index cell
		Widget idxWidget = g_Game.GetWorkspace().CreateWidgets(
			"JM/COT/GUI/layouts/uiactions/UIActionText.layout", rowRoot );
		if ( idxWidget )
		{
			idxWidget.GetScript( row.IndexText );
			if ( !row.IndexText )
				Class.CastTo( row.IndexText, idxWidget.FindAnyWidget( "action_label" ) );
			SetWidgetWidth( idxWidget, 0.06 );
		}

		// Label cell
		Widget lblWidget = g_Game.GetWorkspace().CreateWidgets(
			"JM/COT/GUI/layouts/uiactions/UIActionText.layout", rowRoot );
		if ( lblWidget )
		{
			UIActionText lblAction;
			lblWidget.GetScript( lblAction );
			if ( lblAction )
				lblAction.SetLabel( label );
			SetWidgetWidth( lblWidget, 0.7 );
		}

		// ^ button
		Widget upWidget = g_Game.GetWorkspace().CreateWidgets(
			"JM/COT/GUI/layouts/uiactions/UIActionButton.layout", rowRoot );
		if ( upWidget )
		{
			upWidget.SetHandler( this );
			Class.CastTo( row.BtnUp, upWidget.FindAnyWidget( "action_button" ) );
			TextWidget upTxt;
			Class.CastTo( upTxt, upWidget.FindAnyWidget( "action" ) );
			if ( upTxt ) upTxt.SetText( "^" );
			SetWidgetWidth( upWidget, 0.08 );
		}

		// v button
		Widget downWidget = g_Game.GetWorkspace().CreateWidgets(
			"JM/COT/GUI/layouts/uiactions/UIActionButton.layout", rowRoot );
		if ( downWidget )
		{
			downWidget.SetHandler( this );
			Class.CastTo( row.BtnDown, downWidget.FindAnyWidget( "action_button" ) );
			TextWidget downTxt;
			Class.CastTo( downTxt, downWidget.FindAnyWidget( "action" ) );
			if ( downTxt ) downTxt.SetText( "v" );
			SetWidgetWidth( downWidget, 0.08 );
		}

		// x button
		Widget removeWidget = g_Game.GetWorkspace().CreateWidgets(
			"JM/COT/GUI/layouts/uiactions/UIActionButton.layout", rowRoot );
		if ( removeWidget )
		{
			removeWidget.SetHandler( this );
			Class.CastTo( row.BtnRemove, removeWidget.FindAnyWidget( "action_button" ) );
			if ( row.BtnRemove )
				row.BtnRemove.SetColor( JMTheme.DANGER_DIM );
			TextWidget remTxt;
			Class.CastTo( remTxt, removeWidget.FindAnyWidget( "action" ) );
			if ( remTxt ) remTxt.SetText( "x" );
			SetWidgetWidth( removeWidget, 0.08 );
		}

		return row;
	}

	private void RefreshIndices()
	{
		foreach ( int i, JMStepListRow row : m_Items )
		{
			if ( row.IndexText )
				row.IndexText.SetText( "" + ( i + 1 ) );
		}
	}

	private void RebuildOrder()
	{
		// Re-sort widgets to match m_Items order using SetSort
		foreach ( int i, JMStepListRow row : m_Items )
			if ( row.Root ) row.Root.SetSort( i );
	}
}
