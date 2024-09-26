class UIActionDropdownList: UIActionBase 
{
	protected TextWidget m_Label;
	protected EditBoxWidget m_Text;
	protected ButtonWidget m_Toggle;
	protected ImageWidget m_ToggleImage;

	protected Widget m_ListParent;
	protected TextListboxWidget m_List;

	protected ref array< string > m_Items;

	protected string m_PreviousText;

	protected int m_SelectedIndex;

	void UIActionDropdownList()
	{
		m_Items = new array< string >();
	}
	
	override void OnInit() 
	{
		super.OnInit();
		
		Class.CastTo( m_Label, layoutRoot.FindAnyWidget( "action_label" ) );
		Class.CastTo( m_Text, layoutRoot.FindAnyWidget( "action" ) );
		Class.CastTo( m_Toggle, layoutRoot.FindAnyWidget( "action_toggle" ) );

		if ( Class.CastTo( m_ToggleImage, m_Toggle.FindAnyWidget( "action_toggle_image" ) ) )
		{
			m_ToggleImage.LoadImageFile( 0, "set:dayz_gui image:icon_expand" );
			m_ToggleImage.LoadImageFile( 1, "set:dayz_gui image:icon_collapse" );
		}
	}

	void InitListWidget( Widget parent )
	{
		m_ListParent = parent;

		Class.CastTo( m_List, GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionDropdownList_List.layout", m_ListParent ) );
		m_List.SetFlags( m_List.GetFlags() | WidgetFlags.EXACTPOS );
		
		ToggleList( false );
	}

	override void OnHide() 
	{
	}

	TextWidget GetLabelWidget()
	{
		return m_Label;
	}

	EditBoxWidget GetEditBoxWidget()
	{
		return m_Text;
	}

	TextListboxWidget GetList()
	{
		return m_List;
	}

	override void SetLabel( string text )
	{
		text = Widget.TranslateString( text );

		m_Label.SetText( text );
	}

	override int GetSelection()
	{
		return m_SelectedIndex;
	}

	override void SetSelection( int i, bool sendEvent = true )
	{
		if (i >= m_List.GetNumItems())
			i = m_List.GetNumItems() - 1;

		m_SelectedIndex = i;
		m_List.SelectRow( m_SelectedIndex );
		
		string result;
		if (m_SelectedIndex > -1)
			m_List.GetItemText( m_SelectedIndex, 0, result );

		m_Text.SetText( result );

		if (sendEvent)
			CallEvent( UIEvent.CHANGE );
	}

	void SetText( float num )
	{
		SetText( "" + num );
	}

	override bool IsFocusWidget( Widget widget )
	{
		if ( widget == m_Text )
			return true;
		if ( widget == m_Text )
			return true;
		if ( widget == m_Toggle )
			return true;

		return false;
	}

	void ToggleList( bool open )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::ToggleList" );
		#endif

		m_List.Show( open );

		if ( open )
		{
			m_ToggleImage.SetImage( 0 );
		} else
		{
			m_ToggleImage.SetImage( 1 );
		}

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::ToggleList" );
		#endif
	}

	void SetItems( array< string > items )
	{
		//if ( m_PreviousText != "" )
		//	return;

		m_Items.Clear();
		m_Items.Copy( items );

		m_List.ClearItems();
		foreach ( string item: m_Items )
		{
			m_List.AddItem( item, NULL, 0 );
		}

		m_SelectedIndex = -1;

		m_PreviousText = "";

		UpdateText();
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		float xPos;
		float yPos;
		float xRootPos;
		float yRootPos;
		float width;
		float height;
		float lWidth;
		float lHeight;

		m_ListParent.GetScreenPos( xRootPos, yRootPos );
		m_Text.GetScreenPos( xPos, yPos );

		xPos -= xRootPos;
		yPos -= yRootPos;

		m_Text.GetScreenSize( width, height );
		m_List.GetScreenSize( lWidth, lHeight );

		m_List.SetPos( xPos, yPos + 21, true );
		m_List.SetSize( width, lHeight, true );

		if ( !m_WasFocused )
		{
			OnSelected();
		}
	}

	override void SetText( string text )
	{
		if ( IsFocused() )
			return;

		m_Text.SetText( text );

		UpdateText();
	}

	private bool UpdateText()
	{
		bool success = false;

		string text = m_Text.GetText();

		string item;

		string nText = text;
		nText.ToLower();

		for ( int i = 0; i < m_Items.Count(); ++i )
		{
			item = m_Items[i];
			item.ToLower();

			if ( item.IndexOf( nText ) == 0 )
			{
				m_PreviousText = text;
				success = true;
				m_SelectedIndex = i;
				break;
			}
		}

		m_Text.SetText( m_PreviousText );

		m_List.SelectRow(m_SelectedIndex);

		return success;
	}

	private void OnSelected()
	{
		int selectedRow = m_List.GetSelectedRow();
		if ( m_List.GetNumItems() > 0 && selectedRow != -1 && selectedRow != m_SelectedIndex)
		{
			string result;
			m_SelectedIndex = selectedRow;
			m_List.GetItemText( m_SelectedIndex, 0, result );
			m_Text.SetText( result );

			CallEvent( UIEvent.CHANGE );
		}

		ToggleList( false );
	}

	override string GetText()
	{
		return m_Text.GetText();
	}

	override bool OnKeyPress( Widget w, int x, int y, int key )
	{
		//if ( w == m_Text || w == m_List )
		{
			if ( m_List.GetNumItems() != 0 )
			{
				int selectedIndex = m_SelectedIndex;

				if ( key == KeyCode.KC_UP )
				{
					//if ( m_SelectedIndex == -1 )
					//	m_SelectedIndex = 0;
					//else
						m_SelectedIndex--;
				} else if ( key == KeyCode.KC_DOWN )
				{
					//if ( m_SelectedIndex == -1 )
					//	m_SelectedIndex = 0;
					//else
						m_SelectedIndex++;
				}

				if ( m_SelectedIndex >= m_List.GetNumItems() )
					m_SelectedIndex = m_List.GetNumItems() - 1;
				else if ( m_SelectedIndex < 0 )
					m_SelectedIndex = 0;

				if (m_SelectedIndex != selectedIndex)
				{
					m_List.SelectRow( m_SelectedIndex );

					string result;
					m_List.GetItemText( m_SelectedIndex, 0, result );
					m_Text.SetText( result );
				}
			}
		}

		return super.OnKeyPress( w, x, y, key );
	}

	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		if ( w == m_Text )
		{
			if ( m_PreviousText == m_Text.GetText() && !finished )
				return true;

			if ( !finished )
				ToggleList( true );
			
			bool success = UpdateText();

			if ( finished )
				OnSelected();

			return true;
		}
		
		return false;
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == m_Text )
		{
			ToggleList( true );

			return true;
		} else if ( w == m_Toggle )
		{
			ToggleList( !m_List.IsVisible() );

			return true;
		} else if ( w == m_List )
		{
			if ( m_List.GetSelectedRow() != -1 )
			{
				OnSelected();
			}

			return true;
		}

		return false;
	}

	override bool CallEvent( UIEvent eid )
	{
		if ( !m_HasCallback )
			return false;

		GetGame().GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, new Param2< UIEvent, ref UIActionBase >( eid, this ) );

		return false;
	}

	void SetEditBoxWidth( float width )
	{
		float w;
		float h;
		m_Text.GetSize( w, h );
		m_Text.SetSize( width, h );
		m_Text.Update();
	}
};
