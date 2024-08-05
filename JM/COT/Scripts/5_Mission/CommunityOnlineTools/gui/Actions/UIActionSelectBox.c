class UIActionSelectBox: UIActionBase 
{
	protected Widget m_Frame;
	protected TextWidget m_Label;

	protected ref OptionSelectorMultistate m_Selection;

	void ~UIActionSelectBox()
	{
		if (!GetGame())
			return;

		delete m_Selection;
	}

	override void OnInit() 
	{
		super.OnInit();
				
		Class.CastTo( m_Frame, layoutRoot.FindAnyWidget( "action" ) );
		Class.CastTo( m_Label, layoutRoot.FindAnyWidget( "action_label" ) );
	}

	override void OnShow()
	{
	}

	override void OnHide() 
	{
	}

	void SetSelections( notnull array< string > options )
	{
		m_Selection = new OptionSelectorMultistate( m_Frame, 0, this, true, options );
		m_Selection.m_OptionChanged.Insert( OnSelectionChange );
		m_Selection.Enable();
	}

	override void Disable()
	{
		m_Selection.Disable();
	}

	override void Enable()
	{
		m_Selection.Enable();
	}

	override void SetLabel( string text )
	{
		text = Widget.TranslateString( text );
		
		m_Label.SetText( text );
	}

	override void SetSelection( int i, bool sendEvent = true )
	{
		m_Selection.SetValue( i, sendEvent );
	}

	override int GetSelection()
	{
		return m_Selection.GetValue();
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

	void SetSelectorWidth( float width )
	{
		float w;
		float h;
		m_Frame.GetSize( w, h );
		m_Frame.SetSize( width, h );
		m_Frame.Update();
	}
};

