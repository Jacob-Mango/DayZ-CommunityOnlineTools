class JMMultiSelectRow
{
	string       Label;
	bool         Checked;
	ButtonWidget Button;       // hit area for the whole row
	Widget       Box;   // styled panel, COTField4Set
	ImageWidget  Check;
	TextWidget   LabelWidget;
	Widget       Root;
}

class UIActionMultiSelectList: UIActionBase
{
	protected TextWidget  m_Label;
	protected Widget      m_ListContainer;

	protected ref array<string>              m_Items;
	protected ref array<ref JMMultiSelectRow> m_Rows;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Label,         layoutRoot.FindAnyWidget( "action_label"          ) );
		Class.CastTo( m_ListContainer, layoutRoot.FindAnyWidget( "action_list_container" ) );

		m_Items = new array<string>;
		m_Rows  = new array<ref JMMultiSelectRow>;
	}

	override void SetLabel( string text )
	{
		text = Widget.TranslateString( text );
		if ( m_Label )
			m_Label.SetText( text );
	}

	void SetItems( notnull array<string> items )
	{
		if ( !m_ListContainer )
			return;

		m_Items.Clear();
		m_Rows.Clear();

		Widget child = m_ListContainer.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}

		foreach ( string item : items )
		{
			m_Items.Insert( item );

			Widget row = g_Game.GetWorkspace().CreateWidgets(
				"JM/COT/GUI/layouts/uiactions/UIActionCheckboxRow.layout", m_ListContainer );

			if ( !row )
				continue;

			row.SetHandler( this );

			ref JMMultiSelectRow r = new JMMultiSelectRow();
			r.Label   = item;
			r.Checked = false;
			r.Root    = row;

			Class.CastTo( r.Button,      row.FindAnyWidget( "action"           ) );
			Class.CastTo( r.Box,         row.FindAnyWidget( "action_box_square" ) );
			Class.CastTo( r.Check,       row.FindAnyWidget( "action_check"      ) );
			Class.CastTo( r.LabelWidget, row.FindAnyWidget( "action_label"      ) );

			// Same box + tick the standalone UIActionToggle draws, so a row in a
			// list and a lone checkbox are the same control.
			if ( r.Check )
			{
				r.Check.LoadImageFile( 0, JMConstants.SURFACE_CHECK );
				r.Check.SetImage( 0 );
			}

			if ( r.LabelWidget )
				r.LabelWidget.SetText( Widget.TranslateString( item ) );

			PaintRow( r );

			m_Rows.Insert( r );
		}
	}

	array<string> GetSelectedItems()
	{
		array<string> result = new array<string>;
		foreach ( int i, JMMultiSelectRow r : m_Rows )
			if ( r && r.Checked ) result.Insert( m_Items[i] );
		return result;
	}

	array<int> GetSelectedIndices()
	{
		array<int> result = new array<int>;
		foreach ( int i, JMMultiSelectRow r : m_Rows )
			if ( r && r.Checked ) result.Insert( i );
		return result;
	}

	void SetChecked( int index, bool checked )
	{
		if ( index < 0 || index >= m_Rows.Count() )
			return;
		JMMultiSelectRow r = m_Rows[index];
		if ( !r ) return;
		r.Checked = checked;
		PaintRow( r );
	}

	protected void PaintRow( JMMultiSelectRow r )
	{
		if ( !r )
			return;

		int boxColor = UIActionToggle.COLOR_BOX_OFF;
		float tick   = 0.0;

		if ( r.Checked )
		{
			boxColor = UIActionToggle.COLOR_BOX_ON;
			tick     = 1.0;
		}

		if ( r.Box )
			r.Box.SetColor( boxColor );

		if ( r.Check )
			r.Check.SetAlpha( tick );
	}

	void SelectAll()
	{
		foreach ( int i, JMMultiSelectRow r : m_Rows )
			SetChecked( i, true );
	}

	void DeselectAll()
	{
		foreach ( int i, JMMultiSelectRow r : m_Rows )
			SetChecked( i, false );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		foreach ( int i, JMMultiSelectRow r : m_Rows )
		{
			if ( r && r.Button && w == r.Button )
			{
				r.Checked = !r.Checked;
				PaintRow( r );
				CallEvent( UIEvent.CHANGE );
				return true;
			}
		}
		return false;
	}
}
