class UIActionEditableVector extends UIActionBase 
{
	protected ref TextWidget m_Label;
	protected ref EditBoxWidget m_TextX;
	protected ref EditBoxWidget m_TextY;
	protected ref EditBoxWidget m_TextZ;
	protected ref ButtonWidget m_Button;

	override void OnInit() 
	{
		super.OnInit();
	}

	override void OnShow()
	{
	}

	override void OnHide() 
	{
	}

	void HasButton( bool enabled )
	{
		if ( enabled )
		{
			layoutRoot.FindAnyWidget( "action_wrapper_input" ).Show( false );

			layoutRoot = layoutRoot.FindAnyWidget( "action_wrapper_check" );
			layoutRoot.Show( true );

			m_Button = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "action_button" ) );
		} else
		{
			layoutRoot.FindAnyWidget( "action_wrapper_check" ).Show( false );

			layoutRoot = layoutRoot.FindAnyWidget( "action_wrapper_input" );
			layoutRoot.Show( true );
		}

		m_Label = TextWidget.Cast( layoutRoot.FindAnyWidget( "action_label" ) );

		m_TextX = EditBoxWidget.Cast( layoutRoot.FindAnyWidget( "action_x" ) );
		m_TextY = EditBoxWidget.Cast( layoutRoot.FindAnyWidget( "action_y" ) );
		m_TextZ = EditBoxWidget.Cast( layoutRoot.FindAnyWidget( "action_z" ) );

		SetValue( vector.Zero );
	}

	override void SetLabel( string text )
	{
		m_Label.SetText( text );
	}

	override void SetValue( vector v )
	{
		m_TextX.SetText( v[0].ToString() );
		m_TextY.SetText( v[1].ToString() );
		m_TextZ.SetText( v[2].ToString() );
	}

	override vector GetValue()
	{
		return Vector( m_TextX.GetText().ToFloat(), m_TextY.GetText().ToFloat(), m_TextZ.GetText().ToFloat() );
	}

	override void SetButton( string text )
	{
		TextWidget.Cast( layoutRoot.FindAnyWidget( "action_button_text" ) ).SetText( text );
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( w == m_TextX )
		{
			SetFocus( m_TextX );
			return true;
		}
		if ( w == m_TextY )
		{
			SetFocus( m_TextY );
			return true;
		}
		if ( w == m_TextZ )
		{
			SetFocus( m_TextZ );
			return true;
		}

		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( w == m_TextX )
		{
			SetFocus( NULL );
			return true;
		}
		if ( w == m_TextY )
		{
			SetFocus( NULL );
			return true;
		}
		if ( w == m_TextZ )
		{
			SetFocus( NULL );
			return true;
		}

		return false;
	}

	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		if ( !m_HasCallback ) return false;

		if ( w == m_TextX || w == m_TextY || w == m_TextZ )
		{
			DISABLE_ALL_INPUT = true;
			CallEvent( UIEvent.CHANGE );
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( this.RemoveDisableInput, 100, false );
			return true;
		}
		
		return false;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{	
		if ( !m_HasCallback ) return false;

		bool ret = false;

		if ( w == m_Button )
		{
			ret = CallEvent( UIEvent.CLICK );
		}

		return ret;
	}

	override bool CallEvent( UIEvent eid )
	{
		if ( !m_HasCallback ) return false;

		GetGame().GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, new Param2< UIEvent, ref UIActionBase >( eid, this ) );

		return false;
	}
}