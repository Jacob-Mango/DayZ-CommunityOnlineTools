enum JMConfirmationType
{
	INFO = 0,
	EDIT,
	SELECTION
}

class JMConfirmation: COT_ScriptedWidgetEventHandler 
{
#ifdef DIAG
	static int s_JMConfirmationCount;
#endif

	private Widget layoutRoot;
	
	private Managed m_Base;
	
	private TextWidget m_TextTitle;
	private TextWidget m_TextMessage;
	private EditBoxWidget m_EditBox;
	//! The input drew its own box through vanilla's Editor style, which is the
	//! olive menuButton nine-slice. It now uses the same fill/ring chrome as
	//! every other COT input, so the two panels track the box's visibility.
	private Widget m_EditBoxFill;
	private Widget m_EditBoxRing;

	private Widget m_Buttons1Panel;
	private Widget m_Buttons2Panel;
	private Widget m_Buttons3Panel;

	private ButtonWidget m_Button1;
	private ButtonWidget m_Button2;
	private ButtonWidget m_Button3;

	private TextWidget m_ButtonText1;
	private TextWidget m_ButtonText2;
	private TextWidget m_ButtonText3;

	private string m_Callback1;
	private string m_Callback2;
	private string m_Callback3;

	private string m_EditBoxValue;

	string m_SelectedCallback;

	void JMConfirmation() 
	{
	#ifdef DIAG
		s_JMConfirmationCount++;
		CF_Log.Info("JMConfirmation count: " + s_JMConfirmationCount);
	#endif
	}

	void ~JMConfirmation() 
	{
		if (!g_Game)
			return;

	#ifdef COT_DEBUGLOGS
		auto trace = CF_Trace_0(this);
	#endif

	#ifdef DAYZ_1_28
		DestroyWidget(layoutRoot);
	#endif

		OnHide();

	#ifdef DIAG
		s_JMConfirmationCount--;
		if (s_JMConfirmationCount <= 0)
			CF_Log.Info("JMConfirmation count: " + s_JMConfirmationCount);
	#endif
	}

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );
	}

	void Init( Managed base )
	{
		m_Base = base;

		OnInit();
	}

	void OnInit()
	{
		g_Game.GetWorkspace().CreateWidgets( "JM/COT/gui/layouts/confirmation.layout", layoutRoot );
		layoutRoot.SetSort(65536);

		Class.CastTo( m_TextTitle, layoutRoot.FindAnyWidget( "confirmation_title_text" ) );
		Class.CastTo( m_TextMessage, layoutRoot.FindAnyWidget( "confirmation_message_text" ) );
		Class.CastTo( m_EditBox, layoutRoot.FindAnyWidget( "confirmation_message_input" ) );
		m_EditBoxFill = layoutRoot.FindAnyWidget( "confirmation_input_fill" );
		m_EditBoxRing = layoutRoot.FindAnyWidget( "confirmation_input_ring" );

		Class.CastTo( m_Buttons1Panel, layoutRoot.FindAnyWidget( "confirmation_buttons_1" ) );
		Class.CastTo( m_Buttons2Panel, layoutRoot.FindAnyWidget( "confirmation_buttons_2" ) );
		Class.CastTo( m_Buttons3Panel, layoutRoot.FindAnyWidget( "confirmation_buttons_3" ) );
	}

	bool IsVisible()
	{
		return layoutRoot.IsVisible();
	}

	protected void CallCallback( string callback )
	{
		m_SelectedCallback = callback;

		if ( callback != "" )
		{
			g_Game.GameScript.CallFunctionParams( m_Base, callback, NULL, new Param1<JMConfirmation>( this ) );
		}
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
		{
			return false;
		}

		if ( w == m_Button1 )
		{
			m_EditBoxValue = m_EditBox.GetText();

			Close();

			CallCallback( m_Callback1 );
			return true;
		}

		if ( w == m_Button2 )
		{
			m_EditBoxValue = m_EditBox.GetText();

			Close();

			CallCallback( m_Callback2 );

			return true;
		}

		if ( w == m_Button3 )
		{
			m_EditBoxValue = m_EditBox.GetText();

			Close();

			CallCallback( m_Callback3 );

			return true;
		}

		return false;
	}

	void CreateConfirmation_One( JMConfirmationType type, string title, string message, string button1Title, string button1Callback )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::CreateConfirmation_One" );
		#endif

		layoutRoot.Show( true );

		UpdateType( type );

		m_Buttons1Panel.Show( true );

		m_Buttons2Panel.Show( false );
		m_Buttons3Panel.Show( false );

		m_TextTitle.SetText( title );
		m_TextMessage.SetText( message );

		if ( Class.CastTo( m_Button1, m_Buttons1Panel.FindAnyWidget( "confirmation_button_1" ) ) )
		{
			m_Callback1 = button1Callback;

			Class.CastTo( m_ButtonText1, m_Button1.FindAnyWidget( "confirmation_text" ) );
			m_ButtonText1.SetText( button1Title );
		}

		OnShow();

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::CreateConfirmation_One" );
		#endif
	}

	void CreateConfirmation_Two( JMConfirmationType type, string title, string message, string button1Title, string button2Title, string button1Callback, string button2Callback )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::CreateConfirmation_Two" );
		#endif

		layoutRoot.Show( true );

		UpdateType( type );

		m_Buttons2Panel.Show( true );

		m_Buttons1Panel.Show( false );
		m_Buttons3Panel.Show( false );

		m_TextTitle.SetText( title );
		m_TextMessage.SetText( message );

		if ( Class.CastTo( m_Button1, m_Buttons2Panel.FindAnyWidget( "confirmation_button_1" ) ) )
		{
			m_Callback1 = button1Callback;

			Class.CastTo( m_ButtonText1, m_Button1.FindAnyWidget( "confirmation_text" ) );
			m_ButtonText1.SetText( button1Title );
		}

		if ( Class.CastTo( m_Button2, m_Buttons2Panel.FindAnyWidget( "confirmation_button_2" ) ) )
		{
			m_Callback2 = button2Callback;

			Class.CastTo( m_ButtonText2, m_Button2.FindAnyWidget( "confirmation_text" ) );
			m_ButtonText2.SetText( button2Title );
		}

		OnShow();

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::CreateConfirmation_Two" );
		#endif
	}

	void CreateConfirmation_Three( JMConfirmationType type, string title, string message, string button1Title, string button2Title, string button3Title, string button1Callback, string button2Callback, string button3Callback )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::CreateConfirmation_Three" );
		#endif

		layoutRoot.Show( true );

		UpdateType( type );

		m_Buttons3Panel.Show( true );

		m_Buttons1Panel.Show( false );
		m_Buttons2Panel.Show( false );

		m_TextTitle.SetText( title );
		m_TextMessage.SetText( message );

		if ( Class.CastTo( m_Button1, m_Buttons3Panel.FindAnyWidget( "confirmation_button_1" ) ) )
		{
			m_Callback1 = button1Callback;

			Class.CastTo( m_ButtonText1, m_Button1.FindAnyWidget( "confirmation_text" ) );
			m_ButtonText1.SetText( button1Title );
		}

		if ( Class.CastTo( m_Button2, m_Buttons3Panel.FindAnyWidget( "confirmation_button_2" ) ) )
		{
			m_Callback2 = button2Callback;

			Class.CastTo( m_ButtonText2, m_Button2.FindAnyWidget( "confirmation_text" ) );
			m_ButtonText2 = TextWidget.Cast( m_Button2.FindAnyWidget( "confirmation_text" ) );
			m_ButtonText2.SetText( button2Title );
		}

		if ( Class.CastTo( m_Button3, m_Buttons3Panel.FindAnyWidget( "confirmation_button_3" ) ) )
		{
			m_Callback3 = button3Callback;

			Class.CastTo( m_ButtonText3, m_Button3.FindAnyWidget( "confirmation_text" ) );
			m_ButtonText3.SetText( button3Title );
		}

		OnShow();

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::CreateConfirmation_Three" );
		#endif
	}

	void UpdateType( JMConfirmationType type )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::UpdateType" );
		#endif

		switch ( type )
		{
		case JMConfirmationType.INFO:
		case JMConfirmationType.SELECTION:
			m_TextMessage.SetSize(1, 1);
			ShowEditBox( false );
			break;
		case JMConfirmationType.EDIT:
			m_TextMessage.SetSize(1, 0.5);
			ShowEditBox( true );
			break;
		}

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::UpdateType" );
		#endif
	}

	override void Close()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Close" );
		#endif

		m_EditBox.SetText( "" );
		ShowEditBox( false );
		layoutRoot.Show( false );

		OnHide();

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Close" );
		#endif
	}

	void OnShow()
	{
		CommunityOnlineToolsBase.ForceDisableInputs(true);
	}

	void OnHide() 
	{
		CommunityOnlineToolsBase.ForceDisableInputs(false);
	}

	string GetEditBoxValue()
	{
		return m_EditBoxValue;
	}

	//bool GetEditBoxValueFloat(out float value, float min = -float.MAX, float max = float.MAX)
	bool GetEditBoxValueFloat(out float value, float min = -3.40282e+38, float max = 3.40282e+38)
	{
		if ( m_EditBoxValue == "" )
			return false;

		value = m_EditBoxValue.ToFloat();
		if (value < min || value > max)
			return false;

		return true;
	}

	//bool GetEditBoxValueInt(out int value, int min = int.MIN, int max = int.MAX)
	bool GetEditBoxValueInt(out int value, int min = -2147483648, int max = 2147483647)
	{
		if ( m_EditBoxValue == "" )
			return false;

		value = m_EditBoxValue.ToInt();
		if (value < min || value > max)
			return false;

		return true;
	}

	string GetSelectedCallback()
	{
		return m_SelectedCallback;
	}

	Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	private void ShowEditBox( bool show )
	{
		if ( m_EditBox )     m_EditBox.Show( show );
		if ( m_EditBoxFill ) m_EditBoxFill.Show( show );
		if ( m_EditBoxRing ) m_EditBoxRing.Show( show );
	}
}
